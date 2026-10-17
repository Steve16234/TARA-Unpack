#!/usr/bin/env python3
"""
TARA libil2cpp.so 解密解压工具
自动从 SO 中提取资源表 + 两层 AES 解密 + LZMA 解压

用法:
    python decrypt_tara.py                  # 交互式询问输入/输出路径
    python decrypt_tara.py <输入SO>         # 只询问输出路径
    python decrypt_tara.py <输入SO> <输出SO> # 全自动，不询问
"""
import struct, os, sys, subprocess

try:
    from Crypto.Cipher import AES
except ImportError:
    print("[!] pip install pycryptodome"); sys.exit(1)

# Layer2 密钥不再硬编码。
# 算法（逆向 libhesissti.so 的 sub_4362C / 原 sub_55F6C 得出）：
#   TARA 头 0x20 处的 128 字节密钥材料是 RSA-1024 私钥运算的结果，
#   运行时用内置公钥 (n, e=65537) 做 pow(blob, 65537, n) 还原，
#   得到 PKCS#1 v1.5 type-02 填充的明文，其消息部分(32字节) = 目标 libil2cpp.so 的 ELF 头
#   = Layer2 AES-256-CBC 的 key；IV 为 16 个零。
# RSA 模数 n 在外层 SO 的 .rodata 里出现 3 次（同一份值），当前版本在：
#   0x1FE838 / 0x1FE992 / 0x1FCAE8
# 但偏移会随版本变化，所以脚本主要靠下面的通用特征自动定位：
#   RSA 公钥指数 65537 以大端 4 字节 `00 01 00 01` 形式存在，且总在模数附近(±512B)。
#   4 字节模式在几十 MB 的文件里几乎不会有随机碰撞，搜完再逐窗验证即可。
RSA_E = 65537
RSA_N_OFFSETS = [0x1FCAE8, 0x1FE992, 0x1FE838]   # 已知偏移，仅作加速
RSA_MARKER_WINDOW = 512                           # 指数附近搜索半径
LAYER2_IV = bytes(16)


def _pkcs1_unpad(m):
    """PKCS#1 v1.5 type-02: 00 02 || PS(非零) || 00 || message，失败返回 None"""
    if m[0] != 0x00 or m[1] != 0x02:
        return None
    sep = m.find(b"\x00", 2)
    if sep < 0:
        return None
    return m[sep + 1:]


def find_rsa_modulus(so_data, blob):
    """定位 RSA-1024 模数：pow(blob, e, n) 应为合法的 PKCS#1 v1.5 type-02 填充"""
    c = int.from_bytes(blob, "big")
    cands = [o for o in RSA_N_OFFSETS if 0 <= o <= len(so_data) - 128]

    # 通用启发式：公钥指数 65537 (00 01 00 01) 附近 ±window 内
    marker = RSA_E.to_bytes(4, "big")
    pos, hits = 0, 0
    while hits < 512:
        i = so_data.find(marker, pos)
        if i < 0:
            break
        hits += 1
        lo = max(0, i - RSA_MARKER_WINDOW)
        hi = min(len(so_data) - 128, i + RSA_MARKER_WINDOW)
        cands.extend(range(lo, hi))
        pos = i + 1

    seen = set()
    for off in cands:
        if off in seen:
            continue
        seen.add(off)
        w = so_data[off:off + 128]
        # 模数特征：最高位为 1、末字节为奇数、且大于密文
        if w[0] < 0x80 or w[127] % 2 == 0:
            continue
        n = int.from_bytes(w, "big")
        if n <= c:
            continue
        m = pow(c, RSA_E, n).to_bytes(128, "big")
        msg = _pkcs1_unpad(m)
        if msg:
            return off, msg
    return None, None


def derive_layer2_key(so_data, blob):
    """由 128 字节密钥材料推出 Layer2 AES-256 key（32 字节）"""
    off, msg = find_rsa_modulus(so_data, blob)
    if msg is None:
        return None, None
    print(f"    RSA 模数 @0x{off:X}, 消息 {len(msg)} 字节")
    return msg[:32], msg


def find_resource_entry(data, name=b"libil2cpp.so"):
    """从 SO 文件中搜索资源 entry"""
    idx = 0x300000
    while True:
        pos = data.find(name, idx)
        if pos < 0:
            return None
        iv_off = pos + 0x18
        key_off = pos + 0x2A
        off_off = pos + 0x60
        sz_off = pos + 0x68
        if sz_off + 8 <= len(data):
            off_val = struct.unpack_from("<Q", data, off_off)[0]
            sz_val = struct.unpack_from("<Q", data, sz_off)[0]
            if 0 < sz_val < 200000000 and off_val < len(data):
                return {
                    "offset": off_val,
                    "size": sz_val,
                    "key": data[key_off:key_off+32],
                    "iv": data[iv_off:iv_off+16],
                }
        idx = pos + 1


def _clean_path(raw):
    """去掉拖拽文件时带来的引号与空白，并展开 ~ 与相对路径"""
    return os.path.abspath(os.path.expanduser(raw.strip().strip('"').strip("'")))


def ask_input_so():
    """询问待解密的 SO 路径，直到拿到一个真实存在的文件"""
    while True:
        try:
            raw = input("请输入待解密的 SO 文件路径 (可直接把文件拖进来): ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\n[!] 已取消"); sys.exit(1)
        if not raw:
            continue
        p = _clean_path(raw)
        if os.path.isfile(p):
            return p
        print(f"[!] 文件不存在: {p}")


def ask_output_so(in_path):
    """询问输出 SO 路径，回车使用默认值"""
    default = os.path.join(os.path.dirname(in_path), "libil2cpp_dec.so")
    while True:
        try:
            raw = input(f"请输入输出 SO 路径 (回车默认 {default}): ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\n[!] 已取消"); sys.exit(1)
        p = _clean_path(raw) if raw else default
        d = os.path.dirname(p)
        if os.path.isdir(d):
            return p
        try:
            yn = input(f"[?] 目录不存在: {d}  是否创建? [y/N]: ").strip().lower()
        except (EOFError, KeyboardInterrupt):
            print("\n[!] 已取消"); sys.exit(1)
        if yn in ("y", "yes"):
            try:
                os.makedirs(d, exist_ok=True)
                return p
            except OSError as e:
                print(f"[!] 创建目录失败: {e}")
        print("    请重新输入输出路径")


def ask_exe():
    """定位 unpack.exe：脚本目录下找不到就询问"""
    exe = os.path.join(os.path.dirname(os.path.abspath(__file__)), "unpack.exe")
    if os.path.exists(exe):
        return exe, os.path.dirname(exe)
    print(f"[!] 默认位置找不到 unpack.exe: {exe}")
    while True:
        try:
            raw = input("请输入 unpack.exe 的路径 (可直接拖进来): ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\n[!] 已取消"); sys.exit(1)
        if not raw:
            continue
        p = _clean_path(raw)
        if os.path.isfile(p):
            return p, os.path.dirname(p)
        print(f"[!] 文件不存在: {p}")


def main():
    # 命令行给了参数就不问，没给就交互式询问
    so_in = _clean_path(sys.argv[1]) if len(sys.argv) > 1 else ask_input_so()
    if not os.path.isfile(so_in):
        print(f"[!] 输入文件不存在: {so_in}")
        so_in = ask_input_so()

    if len(sys.argv) > 2:
        so_out = _clean_path(sys.argv[2])
        d = os.path.dirname(so_out)
        if d and not os.path.isdir(d):
            os.makedirs(d, exist_ok=True)
    else:
        so_out = ask_output_so(so_in)

    exe, work = ask_exe()

    print(f"\n输入: {so_in}")
    print(f"输出: {so_out}")
    print(f"解压器: {exe}\n")

    with open(so_in, "rb") as f:
        data = f.read()
    print(f"[1] 读取 {len(data):,} 字节")

    # 搜索资源表
    print("[2] 搜索资源表...")
    entry = find_resource_entry(data)
    if entry is None:
        print("[!] 找不到资源 entry"); sys.exit(1)
    print(f"    offset=0x{entry['offset']:X} size=0x{entry['size']:X}")

    # Layer1 解密
    raw = data[entry["offset"]:entry["offset"]+entry["size"]]
    print(f"[3] Layer1 AES-CBC 解密 ({len(raw):,} bytes)...")
    dec1 = AES.new(entry["key"], AES.MODE_CBC, entry["iv"]).decrypt(raw)
    magic = struct.pack("<I", struct.unpack_from("<I", dec1, 0)[0]).decode()
    field = struct.unpack_from("<I", dec1, 0x0C)[0]
    enc_sz = struct.unpack_from("<I", dec1, 0x10)[0]
    props = dec1[0x18:0x1D]
    # TARA 头布局（依据原作者对 sub_84A48 的反编译）：
    #   0x00 magic "TARA"  0x04 version  0x08 ?   0x0C field(0x20 处密钥材料长度+payload偏移)
    #   0x10 压缩大小       0x14 解压后大小         0x18 props[5]
    #   0x20 起 field 字节是 Layer2 密钥材料，之后才是 payload；payload 长度按 16 对齐
    unc_sz = struct.unpack_from("<I", dec1, 0x14)[0]
    print(f"    {magic} 解压后={unc_sz:,} enc={enc_sz:,} props={props.hex(' ')}")

    # 把解压后大小告知 unpack.exe（它原来把它硬编码成 366192184）
    with open(os.path.join(work, "libil2cpp_size.bin"), "wb") as f:
        f.write(struct.pack("<Q", unc_sz))

    # Layer2 密钥：RSA-1024 公钥还原（详见文件头注释）
    key_blob = dec1[0x20:0x20 + field]
    print(f"[4] Layer2 密钥派生（RSA-{len(key_blob)*8}, e={RSA_E}）...")
    l2key, msg = derive_layer2_key(data, key_blob)
    if l2key is None:
        print("[!] 无法定位 RSA 模数 / 还原失败，密钥材料: " + key_blob[:16].hex(" "))
        sys.exit(1)
    print(f"    key = {l2key.hex()}")
    if msg[:4] != b"\x7fELF":
        print(f"    [!] 还原消息不是 ELF 头: {msg[:8].hex(' ')}")

    # Layer2 解密
    off = 0x20 + field
    payload = dec1[off:off + ((enc_sz + 15) & ~15)]   # 原文: 长度按 16 对齐
    pad = (16 - len(payload) % 16) % 16
    print(f"[5] Layer2 AES-CBC 解密 ({len(payload):,} bytes)...")
    dec2 = AES.new(l2key, AES.MODE_CBC, LAYER2_IV).decrypt(payload + b'\x00' * pad)[:enc_sz]

    # LZMA 解压
    comp_path = os.path.join(work, "libil2cpp_compressed.bin")
    props_path = os.path.join(work, "libil2cpp_props.bin")
    with open(comp_path, "wb") as f: f.write(dec2)
    with open(props_path, "wb") as f: f.write(props)

    print(f"[6] 自定义 LZMA 解压...")
    r = subprocess.run([exe], capture_output=True, text=True, cwd=work, timeout=600)
    for line in r.stdout.strip().split("\n"):
        if "ret=" in line or "Saved" in line:
            print(f"    {line.strip()}")

    dec_path = os.path.join(work, "libil2cpp_dec.so")
    if not os.path.exists(dec_path):
        print("[!] 解压失败"); sys.exit(1)

    if so_out != dec_path:
        import shutil
        shutil.copy2(dec_path, so_out)

    sz = os.path.getsize(so_out)
    with open(so_out, "rb") as f:
        hdr = f.read(4)
    print(f"\n[OK] {so_out} ({sz/1024/1024:.2f} MB) {hdr}")

    for p in [comp_path, props_path]:
        os.remove(p)


if __name__ == "__main__":
    main()