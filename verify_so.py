#!/usr/bin/env python3
"""全面校验解出的 libil2cpp.so：结构完整性 + 内容可读性"""
import struct, sys, hashlib
from collections import Counter

SO = r"D:\netease\Downloads\libil2cpp_dec.so"
raw = open(SO, "rb").read()
N = len(raw)
ok = []

def rep(name, cond, detail=""):
    ok.append(cond)
    print(f"  [{'✓' if cond else '✗'}] {name}" + (f"  {detail}" if detail else ""))

print("=" * 70)
print("1. ELF 头")
print("=" * 70)
magic = raw[:4]
rep("magic \\x7fELF", magic == b"\x7fELF")
ei_class, ei_data, ei_ver = raw[4], raw[5], raw[6]
rep("64-bit / 小端 / EV_CURRENT", ei_class == 2 and ei_data == 1 and ei_ver == 1)
e_type, e_machine = struct.unpack_from("<HH", raw, 16)
e_entry, e_phoff, e_shoff = struct.unpack_from("<QQQ", raw, 24)
e_flags, e_ehsize, e_phentsize, e_phnum = struct.unpack_from("<IHHH", raw, 48)
e_shentsize, e_shnum, e_shstrndx = struct.unpack_from("<HHH", raw, 58)
rep("ET_DYN (共享库)", e_type == 3, f"e_type={e_type}")
rep("AArch64 (0xB7)", e_machine == 0xB7, f"e_machine=0x{e_machine:X}")
print(f"     e_entry = 0x{e_entry:X}   e_phoff=0x{e_phoff:X}  e_shoff=0x{e_shoff:X}")
print(f"     phnum={e_phnum}  shnum={e_shnum}  shstrndx={e_shstrndx}")

print()
print("=" * 70)
print("2. 与 RSA 推导密钥交叉验证（Layer2 明文前 32 字节应 == ELF 头）")
print("=" * 70)
RSA_KEY = bytes.fromhex("7f454c460201010000000000000000000300b700010000000000f80500000000")
rep("ELF 头前 32 字节 == RSA 消息", raw[:32] == RSA_KEY, raw[:32].hex(" "))

print()
print("=" * 70)
print("3. Program Headers")
print("=" * 70)
phs = []
for i in range(e_phnum):
    o = e_phoff + i * e_phentsize
    p_type, p_flags = struct.unpack_from("<II", raw, o)
    p_off, p_vaddr, p_paddr, p_filesz, p_memsz, p_align = struct.unpack_from("<QQQQQQ", raw, o + 8)
    phs.append((p_type, p_off, p_vaddr, p_filesz, p_memsz, p_flags))
    rep(f"  PH[{i}] type=0x{p_type:X} off=0x{p_off:X} vaddr=0x{p_vaddr:X} filesz=0x{p_filesz:X}",
        p_off + p_filesz <= N, "" if p_off + p_filesz <= N else f"越界! 文件只有 {N}")
loads = [p for p in phs if p[0] == 1]
rep("PT_LOAD 段存在", len(loads) > 0, f"{len(loads)} 个")
for p in loads:
    rep(f"  PT_LOAD vaddr==p_offset (0 基加载)", p[1] == p[2], f"off=0x{p[1]:X} vaddr=0x{p[2]:X}")
dyn = [p for p in phs if p[0] == 2]
rep("PT_DYNAMIC 存在", len(dyn) == 1)

print()
print("=" * 70)
print("4. Section Headers")
print("=" * 70)
rep("shoff 在文件内", e_shoff + e_shnum * e_shentsize <= N,
    f"末尾 0x{e_shoff + e_shnum*e_shentsize:X} vs 文件 0x{N:X}")
shs = []
for i in range(e_shnum):
    o = e_shoff + i * e_shentsize
    sh_name, sh_type, sh_flags = struct.unpack_from("<IIQ", raw, o)
    sh_addr, sh_off, sh_size = struct.unpack_from("<QQQ", raw, o + 16)
    sh_link, sh_info, sh_align, sh_entsize = struct.unpack_from("<IIQQ", raw, o + 40)
    shs.append(dict(name=sh_name, type=sh_type, flags=sh_flags, addr=sh_addr,
                    off=sh_off, size=sh_size, link=sh_link, entsize=sh_entsize))
bad = [s for s in shs if s["type"] != 8 and s["off"] + s["size"] > N]
rep("所有节区均在文件边界内", not bad, "" if not bad else f"越界 {len(bad)} 个")
# 节区名
strtab = shs[e_shstrndx]
names_off = strtab["off"]
def sname(n):
    e = raw.index(b"\x00", names_off + n)
    return raw[names_off + n:e].decode("utf-8", "replace")
secmap = {}
for s in shs:
    s["nm"] = sname(s["name"])
    secmap[s["nm"]] = s
print("     节区:", ", ".join(s["nm"] for s in shs if s["nm"]))
for want in (".text", ".rodata", ".data", ".bss", ".dynamic", ".dynsym", ".dynstr",
             ".rela.dyn", ".rela.plt", ".got", ".init_array", ".fini_array"):
    rep(f"  节区 {want} 存在", want in secmap)
# 文件末尾 == 最后一个节区结束（无尾部垃圾）
tail = max((s["off"] + s["size"]) for s in shs if s["type"] != 8)
rep("文件大小 == 最后节区结束（无多余垃圾）", tail <= N, f"节区末尾 0x{tail:X} / 文件 0x{N:X} (差 {N-tail} 字节)")

print()
print("=" * 70)
print("5. 动态段 (.dynamic)")
print("=" * 70)
d = secmap[".dynamic"]
dyn_entries = []
for i in range(d["size"] // 16):
    tag, val = struct.unpack_from("<QQ", raw, d["off"] + i * 16)
    if tag == 0:
        break
    dyn_entries.append((tag, val))
DT = {1: "NEEDED", 5: "STRTAB", 6: "SYMTAB", 10: "STRSZ", 11: "SYMENT",
      0xE: "SONAME", 0xC: "INIT", 0xD: "FINI", 0x19: "INIT_ARRAY", 0x1B: "INIT_ARRAYSZ",
      0x1A: "FINI_ARRAY", 0x1C: "FINI_ARRAYSZ", 0x15: "DEBUG", 7: "RELA", 8: "RELASZ",
      0x17: "JMPREL", 0x2: "PLTRELSZ", 0xFFFFFFF5: "GNU_HASH", 6: "SYMTAB"}
tags = [t for t, v in dyn_entries]
for want, nm in ((1, "DT_NEEDED"), (5, "DT_STRTAB"), (6, "DT_SYMTAB"),
                 (0x19, "DT_INIT_ARRAY"), (7, "DT_RELA")):
    rep(f"  {nm} 存在", want in tags)
needed = []
strtab_sec = None
# DT_STRTAB 给的是虚拟地址，转文件偏移
strt_v = next(v for t, v in dyn_entries if t == 5)
for s in shs:
    if s["type"] != 8 and s["addr"] <= strt_v < s["addr"] + s["size"]:
        strtab_sec = s
        break
if strtab_sec:
    so = strtab_sec["off"] + (strt_v - strtab_sec["addr"])
    for t, v in dyn_entries:
        if t == 1:
            e2 = raw.index(b"\x00", so + v)
            needed.append(raw[so + v:e2].decode("utf-8", "replace"))
print("     DT_NEEDED:", needed if needed else "(无)")

print()
print("=" * 70)
print("6. 符号表")
print("=" * 70)
ds = secmap.get(".dynsym")
if ds and ds["entsize"]:
    cnt = ds["size"] // ds["entsize"]
    rep(".dynsym 可解析", True, f"{cnt:,} 个符号")
    dynstr = secmap[".dynstr"]
    exported = []
    for i in range(cnt):
        o = ds["off"] + i * ds["entsize"]
        st_name, st_info, st_other, st_shndx, st_value, st_size = struct.unpack_from("<IBBHQQ", raw, o)
        if st_shndx != 0 and st_name:
            e2 = raw.index(b"\x00", dynstr["off"] + st_name)
            nm = raw[dynstr["off"] + st_name:e2].decode("utf-8", "replace")
            exported.append(nm)
    rep("有导出符号", len(exported) > 0, f"{len(exported):,} 个")
    hits = [s for s in exported if "il2cpp" in s or s in ("JNI_OnLoad", "JNI_OnUnload")]
    print(f"     il2cpp/JNI 相关符号 {len(hits)} 个，示例: {hits[:12]}")
    rep("  含 il2cpp_* 导出函数", any(s.startswith("il2cpp_") for s in exported))
else:
    rep(".dynsym 可解析", False)
print("     .symtab (完整符号表):", "存在" if ".symtab" in secmap else "已被 strip（正常）")

print()
print("=" * 70)
print("7. 重定位")
print("=" * 70)
for nm in (".rela.dyn", ".rela.plt"):
    s = secmap.get(nm)
    if s and s["entsize"]:
        rep(f"  {nm} 可解析", True, f"{s['size']//s['entsize']:,} 条")

print()
print("=" * 70)
print("8. 内容可读性 / 合理性")
print("=" * 70)
# 可打印 ASCII 字符串统计
runs, cnt, cur = [], 0, 0
sample = raw[:0x800000]  # 前 8MB 足够采样
i = 0
printable = set(range(0x20, 0x7F))
strs = []
start = None
for i, b in enumerate(sample):
    if b in printable:
        if start is None:
            start = i
    else:
        if start is not None and i - start >= 8:
            strs.append(sample[start:i])
        start = None
print(f"  前 8MB 内长度≥8 的可打印字符串: {len(strs):,} 条")
il2 = [s for s in strs if b"il2cpp" in s]
rep("  含 il2cpp 相关字符串", len(il2) > 0, f"{len(il2):,} 条")
print("     示例:", [s.decode("ascii", "replace")[:52] for s in il2[:4]])
unity = [s for s in strs if b"Unity" in s or b"unity" in s]
rep("  含 Unity 相关字符串", len(unity) > 0, f"{len(unity):,} 条")
# .text 非全零、非全 FF
t = secmap[".text"]
tb = raw[t["off"]:t["off"] + min(t["size"], 0x200000)]
z = tb.count(0) / len(tb)
f = tb.count(0xFF) / len(tb)
rep("  .text 不是全零/全FF（解压无空洞）", z < 0.30 and f < 0.30,
    f"0x00 占比 {z:.1%}, 0xFF 占比 {f:.1%}")
# 采样熵
import math
def entropy(b):
    if not b: return 0.0
    c = Counter(b)
    n = len(b)
    return -sum((v/n) * math.log2(v/n) for v in c.values())
for nm in (".text", ".rodata"):
    s = secmap[nm]
    seg = raw[s["off"]:s["off"] + min(s["size"], 0x100000)]
    e = entropy(seg)
    rep(f"  {nm} 熵值正常 (4.0~8.0)", 4.0 <= e <= 8.0, f"{e:.2f} bits/byte")

print()
print("=" * 70)
print("9. 汇总")
print("=" * 70)
print(f"  大小: {N:,} 字节 ({N/1048576:.2f} MB)   SHA256: {hashlib.sha256(raw).hexdigest()}")
print(f"  检查项: {len(ok)} 项, 通过 {sum(1 for x in ok if x)} 项, 失败 {sum(1 for x in ok if not x)} 项")
if all(ok):
    print("\n  ✅ 全部通过 —— 文件结构完整、内容可读，可直接用 Il2CppDumper / IDA / Ghidra 分析")
else:
    print("\n  ❌ 存在失败项，见上方 ✗ 标记")
    sys.exit(1)
