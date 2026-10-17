# TARA libil2cpp.so 解密解压工具

从加固过的 Android 游戏外层 `.so`（如 `libgedenedo.so` / `libhesissti.so`）中提取被加密压缩的
`libil2cpp.so`，供 Il2CppDumper / IDA 等工具做后续分析。

> 技术来源：看雪论坛 [《XX二游 消失的libil2cpp.so 提取与算法分析》](https://bbs.kanxue.com/thread-292061.html)
>（作者 Chxe，2026-07）。本仓库在原文基础上补全了 Layer2 密钥的**静态派生**，不再依赖运行时 hook。

仅供安全研究、逆向学习与兼容性分析使用，请勿用于任何侵犯他人权益的用途。

---

## 一句话原理

```
外层 SO
  └─ 资源表 entry (name="libil2cpp.so")
       ├─ key32 @ +0x2A, iv16 @ +0x18          ─┐
       ├─ offset @ +0x60, size @ +0x68          │ Layer1: AES-256-CBC
       ▼                                        ┘
  TARA 容器
       ├─ 0x20 起 128 字节 = RSA-1024 密文      →  pow(blob, 65537, n)  →  AES key
       └─ 0x20+field 起  payload                 →  Layer2: AES-256-CBC (IV=0)
                                                  →  自定义 LZMA  →  libil2cpp.so
```

## 算法细节

### 1. 资源表

在外层 SO 尾部搜索明文 `libil2cpp.so` 字符串，命中位置附近的布局：

| 偏移 | 内容 |
|---|---|
| `pos + 0x18` | Layer1 IV（16 字节） |
| `pos + 0x2A` | Layer1 Key（32 字节） |
| `pos + 0x60` | 数据偏移（u64） |
| `pos + 0x68` | 数据大小（u64） |

### 2. Layer1 —— AES-256-CBC

用上面的 key/iv 解出 **TARA 容器**。TARA 头布局：

| 偏移 | 含义 |
|---|---|
| `0x00` | magic `"TARA"`（0x41524154） |
| `0x04` | version（实测为 3） |
| `0x08` | 未确认 |
| `0x0C` | `field`：`0x20` 处密钥材料的长度，同时也是 payload 相对偏移 |
| `0x10` | **压缩后大小** |
| `0x14` | **解压后大小** |
| `0x18` | LZMA props（5 字节，如 `5d 00 40 00 00` → lc=3 lp=0 pb=2） |
| `0x20` | `field` 字节 **Layer2 密钥材料**（RSA-1024 密文） |
| `0x20+field` | Layer2 密文，长度按 16 对齐 |

### 3. Layer2 密钥派生 —— RSA-1024（本项目补全的关键）

原文只能靠 hook `aes_cbc_decrypt_wrapper` 抓密钥，换文件就失效。逆向发现：

- TARA 头 `0x20` 处那 128 字节是用 **RSA 私钥**运算过的结果
- 运行时用内置**公钥** `(n, e=65537)` 还原：`m = pow(blob, 65537, n)`
- `m` 是标准 **PKCS#1 v1.5 type-02** 填充：`00 02 || 93 字节随机 || 00 || 32 字节消息`
- 消息部分 = **目标 `libil2cpp.so` 的 ELF 头前 32 字节** = Layer2 AES-256 key
- IV 为 16 个零

因为只有公钥（`n`、`e`）在二进制里，没有私钥指数 `d`，所以这个设计对**同一个 SO 内**的
数据是安全的，但只要把公钥抠出来就能离线还原。

RSA 模数 `n`（128 字节，首字节 `0xb2`）在外层 SO 中出现 3 次，同一份值：
`0x1FE838` / `0x1FE992` / `0x1FCAE8`。`00 01 00 01`（= 65537）紧贴在这些大数旁边。

模数定位是**全自动**的，分两级：

1. 先试已知偏移（对当前版本最快）
2. 失败则走**通用启发式**：在整个 SO 里找 4 字节模式 `00 01 00 01`（即大端的 65537，
   RSA 公钥指数），在每个命中点 ±512 字节内枚举候选窗口，按"最高位为 1、末字节为奇数、
   大于密文"过滤后逐个做模幂，`pow(blob, e, n)` 结果是合法 PKCS#1 v1.5 type-02 填充即为模数

`00 01 00 01` 这种 4 字节模式在几十 MB 的文件里几乎不会有随机碰撞，且"还原出合法
PKCS#1 填充"本身是很强的自校验（概率约 2^-16），所以不会误判。实测在 45 MB 的 SO 上
两种路径都在 0.01 秒内定位到 `0x1FCAE8`。

### 4. 自定义 LZMA

`unpack.cpp` 是从原 SO 反编译还原的 LZMA 解码器（对应原文的 `sub_53258`），**不是标准 LZMA**，
所以不能用 Python 的 `lzma` / 7-Zip 解。props 仍按标准格式（`lc/lp/pb` + dict size）。

实测：输入 42,096,951 字节 → 输出 284,116,424 字节（270.95 MB），`ret=0`（`SZ_OK`）。

---

## 使用

依赖：Python 3.8+，`pycryptodome`，MSVC（或任意能编 `unpack.cpp` 的 C++17 编译器）。

```bash
pip install pycryptodome

# 1. 编译解压器（Windows / MSVC）
build.bat            # 产物 unpack.exe，必须放在 decrypt_tara.py 同目录

# 2. 解密解压
python decrypt_tara.py                          # 交互式询问输入/输出路径
python decrypt_tara.py <外层.so>                # 只询问输出路径
python decrypt_tara.py <外层.so> <输出.so>      # 全自动
```

运行示例：

```
[1] 读取 45,193,015 字节
[2] 搜索资源表...
    offset=0x2F3CD0 size=0x28259E0
[3] Layer1 AES-CBC 解密 (42,097,120 bytes)...
    TARA 解压后=284,116,424 enc=42,096,951 props=5d 00 40 00 00
[4] Layer2 密钥派生（RSA-1024, e=65537）...
    RSA 模数 @0x1FCAE8, 填充 93 字节, 消息 32 字节
    key = 7f454c460201010000000000000000000300b700010000000000f80500000000
[5] Layer2 AES-CBC 解密 (42,096,960 bytes)...
[6] 自定义 LZMA 解压...
    [+] ret=0 out=284116424 (270.95MB) in_used=42096951
    [+] Saved libil2cpp_dec.so (270.95 MB)

[OK] libil2cpp_dec.so (270.95 MB) b'\x7fELF'
```

`derive_key.py` 是独立的密钥推导验证脚本，可以单独跑来看 RSA 还原的中间结果。

## 文件说明

| 文件 | 说明 |
|---|---|
| `decrypt_tara.py` | 主脚本：资源表查找 → Layer1 → RSA 派生 → Layer2 → 调用解压器 |
| `unpack.cpp` / `unpack.h` | 反编译还原的自定义 LZMA 解码器（`sub_120750` / `sub_11EEF0`） |
| `main.cpp` | 解压器入口，读 `libil2cpp_compressed.bin` / `libil2cpp_props.bin` / `libil2cpp_size.bin` |
| `build.bat` | MSVC 构建脚本 |
| `derive_key.py` | 单独验证 RSA 密钥派生 |
| `verify_so.py` | 对解出产物做 51 项完整性/可读性校验 |

## 构建注意事项

- `unpack.cpp` 是反编译出来的巨型函数，MSVC **必须加 `/bigobj`**，否则段数超限
- 源码含中文注释且无 BOM，MSVC 需加 **`/utf-8`**，否则按 GBK 解析会把某个字节误判成
  反斜杠续行符，吞掉下一行代码，报莫名的 `C2065 未声明的标识符`
- `main.cpp` 的输出缓冲区大小来自 `libil2cpp_size.bin`（由脚本按 TARA 头 `+0x14` 写入），
  不再是硬编码 —— 不同版本的 `libil2cpp.so` 大小不一样

## 已知限制

- `unpack.cpp` 为反编译产物，可读性差，但功能与原 SO 一致
- 输出大小最大约 285 MB，`main.cpp` 用 `VirtualAlloc` 一次性分配
- RSA 模数定位依赖"模数在公钥指数 65537 附近"这一特征。若未来版本把指数换成别的值
  （如 3 或更大的数），需要把 `RSA_E` 和 `RSA_N_OFFSETS` 一起改

## 致谢

- [Chxe](https://bbs.kanxue.com/user-home-1051178.htm) —— 原始算法分析与 TARA/Layer1 逆向
- 本仓库补充：Layer2 密钥的 RSA 静态派生、自动化流水线、构建修复

## License

仅供学习研究。
