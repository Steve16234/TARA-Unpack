#pragma once
#include <Windows.h>
#include <intrin.h>
#include <minwindef.h>
#include <stdint.h>
typedef unsigned __int64 _QWORD;
#define HIBYTE(x) (*((_BYTE*)&(x) + sizeof(x) - 1))
#pragma pack(push, 1)
struct LZMA_Ctx96 {
    uint32_t lc;              // 0x00
    uint32_t lp;              // 0x04
    uint32_t pb;              // 0x08
    uint32_t dict_size;       // 0x0C
    uint16_t* prob_table;     // 0x10
    uint8_t* output_buf;      // 0x18
    uint8_t* input_ptr;       // 0x20
    uint32_t range;           // 0x28
    uint32_t code;            // 0x2C
    uint64_t dict_offset;     // 0x30
    uint8_t* dict_ring;       // 0x38
    uint32_t processed;       // 0x40
    uint32_t remain_len;      // 0x44
    uint32_t state;           // 0x48
    uint32_t rep0;            // 0x4C
    uint32_t rep1;            // 0x50
    uint32_t rep2;            // 0x54
    uint32_t rep3;            // 0x58
    uint32_t temp_buf_size;   // 0x5C
    uint32_t is_finished;     // 0x60
    uint32_t need_init;       // 0x64
    uint32_t temp_buf_count;  // 0x68
    uint32_t padding_6C;      // 0x6C
    uint8_t  temp_buf[20];    // 0x70
};
#pragma pack(pop)
__int64 __fastcall sub_120750(__int64 a1, __int64* a2, __int64 a3,
    unsigned __int64* a4, unsigned __int8* a5, unsigned int a6,
    unsigned int a7, unsigned __int64* a8, __int64 a9);
