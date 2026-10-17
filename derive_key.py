#!/usr/bin/env python3
"""解析 PKCS#1 v1.5：m = 00 02 || PS(非零随机) || 00 || message，message 就是 Layer2 密钥

用法: python derive_key.py <外层.so> [RSA模数偏移(默认 0x1FCAE8)]
"""
import struct, sys
from Crypto.Cipher import AES

if len(sys.argv) < 2:
    print(__doc__); sys.exit(1)
SO = sys.argv[1]
N_OFF = int(sys.argv[2], 0) if len(sys.argv) > 2 else 0x1FCAE8
raw = open(SO, "rb").read()
E = 65537
N = int.from_bytes(raw[N_OFF:N_OFF+128], "big")


def find_resource_entry(data, name=b"libil2cpp.so"):
    idx = 0x300000
    while True:
        pos = data.find(name, idx)
        if pos < 0:
            return None
        iv_off, key_off, off_off, sz_off = pos + 0x18, pos + 0x2A, pos + 0x60, pos + 0x68
        if sz_off + 8 <= len(data):
            off_val = struct.unpack_from("<Q", data, off_off)[0]
            sz_val = struct.unpack_from("<Q", data, sz_off)[0]
            if 0 < sz_val < 200000000 and off_val < len(data):
                return {"offset": off_val, "size": sz_val,
                        "key": data[key_off:key_off+32], "iv": data[iv_off:iv_off+16]}
        idx = pos + 1


data = open(SO, "rb").read()
e = find_resource_entry(data)
dec1 = AES.new(e["key"], AES.MODE_CBC, e["iv"]).decrypt(data[e["offset"]:e["offset"]+e["size"]])
field = struct.unpack_from("<I", dec1, 0x0C)[0]
enc_sz = struct.unpack_from("<I", dec1, 0x10)[0]
unc_sz = struct.unpack_from("<I", dec1, 0x14)[0]
props = dec1[0x18:0x1D]
blob = dec1[0x20:0x20 + field]
payload = dec1[0x20+field: 0x20+field + ((enc_sz+15) & ~15)]

c = int.from_bytes(blob, "big")
m = pow(c, E, N).to_bytes(128, "big")
print("RSA 还原结果 (128 字节):")
print(" ", m.hex(" "))
print(f"  头: {m[0]:02X} {m[1]:02X}  (PKCS#1 v1.5 应为 00 02)")

assert m[0] == 0x00 and m[1] == 0x02, "不是 PKCS#1 v1.5 type 02"
sep = m.index(b"\x00", 2)
msg = m[sep+1:]
print(f"\n  填充长度 {sep-2}, 消息长度 {len(msg)}")
print(f"  消息 = {msg.hex(' ')}")

key = msg[:32]
iv = bytes(16)
print(f"\n>>> AES-256 key = {key.hex()}")
pad = (16 - len(payload) % 16) % 16
dec2 = AES.new(key, AES.MODE_CBC, iv).decrypt(payload + b"\x00"*pad)[:enc_sz]
print(f"    Layer2 解密后前 32 = {dec2[:32].hex(' ')}")

pb = props[0]
lc, rem = pb % 9, pb // 9
lp, pbs = rem % 5, rem // 5
print(f"\n    props=0x{pb:02X} lc={lc} lp={lp} pb={pbs}  (自定义 LZMA，标准库解不了属正常)")

# 落盘给 unpack.exe 用
import os
work = os.path.dirname(os.path.abspath(__file__))
open(os.path.join(work, "libil2cpp_compressed.bin"), "wb").write(dec2)
open(os.path.join(work, "libil2cpp_props.bin"), "wb").write(props)
open(os.path.join(work, "libil2cpp_size.bin"), "wb").write(struct.pack("<Q", unc_sz))
print(f"\n[OK] 已写出 libil2cpp_compressed.bin ({len(dec2):,}) / props / size={unc_sz:,}")
print("     现在运行 unpack.exe 即可用自定义 LZMA 解压")
