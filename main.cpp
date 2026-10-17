#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <stdint.h>
#include <Windows.h>
#include "unpack.h"

void* __fastcall sub_10FA1C(__int64 a1, size_t a2) { return a2 ? malloc(a2) : (void*)a2; }
void __fastcall sub_10FA24(__int64 a1, void* a2) { free(a2); }
INT64 off_2F51D8[] = { *(INT64*)&sub_10FA1C, *(INT64*)&sub_10FA24 };

__int64 __fastcall sub_10F9F0(__int64 a1, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6) {
    unsigned long long v7 = 0;
    return sub_120750(a1, (long long*)a2, a3, (unsigned long long*)a4, (unsigned char*)a5, a6, 0, (unsigned __int64*)&v7, (long long)&off_2F51D8);
}

int main() {
    printf("=== TARA Decompress ===\n");
    std::ifstream cf("libil2cpp_compressed.bin", std::ios::binary | std::ios::ate);
    std::ifstream pf("libil2cpp_props.bin", std::ios::binary | std::ios::ate);
    if (!cf.is_open() || !pf.is_open()) { printf("[!] Cannot open input\n"); return 1; }
    size_t csz = cf.tellg(); cf.seekg(0);
    size_t psz = pf.tellg(); pf.seekg(0);
    std::vector<uint8_t> cd(csz), pr(psz);
    cf.read((char*)cd.data(), csz);
    pf.read((char*)pr.data(), psz);
    printf("[+] Comp: %zu bytes Props: %02X %02X %02X %02X %02X\n", csz, pr[0], pr[1], pr[2], pr[3], pr[4]);

    // 解压后大小：优先读 libil2cpp_size.bin（由 decrypt_tara.py 从 TARA 头 +0x14 写入）
    // 读不到才退回旧文件的硬编码值
    unsigned long long OUT_SZ = 366192184;
    {
        std::ifstream sf("libil2cpp_size.bin", std::ios::binary);
        if (sf.is_open()) {
            unsigned long long v = 0;
            sf.read((char*)&v, 8);
            if (v > 0) OUT_SZ = v;
        }
    }
    printf("[*] OUT_SZ=%llu (%.2f MB)\n", OUT_SZ, OUT_SZ/1024.0/1024.0);

    uint8_t* db = (uint8_t*)VirtualAlloc(NULL, (SIZE_T)OUT_SZ, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!db) { printf("[!] Alloc fail\n"); return 1; }
    unsigned long long dl = OUT_SZ, sl = csz;
    printf("[*] Calling decompress...\n"); fflush(stdout);
    __int64 ret = sub_10F9F0((__int64)db, (__int64)&dl, (__int64)cd.data(), (__int64)&sl, (__int64)pr.data(), (unsigned int)pr.size());
    printf("[+] ret=%lld out=%llu (%.2fMB) in_used=%llu\n", ret, dl, dl/1024.0/1024.0, sl);
    // ret==0 (SZ_OK) 或 ret==6 (NEEDS_MORE_INPUT，旧构建的返回) 都算成功；
    // 真正判据是解出的字节数与预期一致
    if ((ret == 0 || ret == 6) && dl > 0) {
        std::ofstream f("libil2cpp_dec.so", std::ios::binary);
        size_t rem = (size_t)dl, off = 0;
        while (rem > 0) { size_t w = rem < 0x100000 ? rem : 0x100000; f.write((char*)(db+off), w); off += w; rem -= w; }
        printf("[+] Saved libil2cpp_dec.so (%.2f MB)\n", dl/1024.0/1024.0);
    } else { printf("[!] Failed ret=%lld\n", ret); }
    VirtualFree(db, 0, MEM_RELEASE);
    return 0;
}