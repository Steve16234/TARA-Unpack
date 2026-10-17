#include <iostream>
#include <stdint.h>
#include <intrin.h>
#include <emmintrin.h>  // for __m128i
#include "unpack.h"

// 类型定义
typedef __m128i _OWORD;
typedef DWORD _DWORD;
typedef BYTE _BYTE;
typedef BOOL _BOOL4;
typedef WORD _WORD;
typedef unsigned __int64 _QWORD;

// 辅助宏
#define LODWORD(x) (*((_DWORD*)&(x)))
#define HIDWORD(x) (*((_DWORD*)&(x) + 1))
#define LOWORD(x)  (*((_WORD*)&(x)))
#define HIWORD(x)  (*((_WORD*)&(x) + 1))
#define LOBYTE(x)  (*((_BYTE*)&(x)))

// HIBYTE: 获取值的最高字节
// 使用 sizeof 自动适配不同类型：
//   - unsigned __int16 (2字节): 偏移1, 获取 bits 8-15
//   - unsigned int (4字节): 偏移3, 获取 bits 24-31
// 这与原始 ARM64 代码中的 HIBYTE 行为一致
#define HIBYTE(x) (*((_BYTE*)&(x) + sizeof(x) - 1))


uint32_t bswap32(uint32_t x) {
    return ((x & 0x000000FF) << 24) |
        ((x & 0x0000FF00) << 8) |
        ((x & 0x00FF0000) >> 8) |
        ((x & 0xFF000000) >> 24);
}

// 概率表初始化值 (每个word=1024, 即0x400)
alignas(16) static const unsigned __int64 xmmword_283B00_data[2] = {
    0x0400040004000400ULL,
    0x0400040004000400ULL
};
static const _OWORD* xmmword_283B00 = (const _OWORD*)xmmword_283B00_data;


// [范围编码器/Range Coder]
// 算术编码解码核心
// 处理literal和match符号
// 使用11-bit概率精度(0-2047)
__int64 __fastcall sub_11F380(__int64 a1, unsigned __int8* a2, __int64 a3)
{
    __int64 v3; // x15
    __int64 v4; // x10
    int v5; // w16
    __int64 v6; // x17
    unsigned int v7; // w3
    unsigned int v8; // w13
    __int64 v9; // x14
    unsigned __int8* v10; // x8
    unsigned int v11; // w11
    unsigned int v12; // w12
    unsigned __int8* v13; // x9
    __int64 v14; // x10
    __int64 v15; // x1
    unsigned int i; // w12
    unsigned __int8 v17; // t1
    unsigned int v18; // w14
    unsigned int v19; // w13
    unsigned __int8 v20; // t1
    unsigned int v21; // w16
    unsigned int v22; // w11
    int v23; // w15
    unsigned int v24; // w13
    __int64 v25; // x11
    unsigned __int64 v26; // x14
    unsigned __int64 v27; // x15
    bool v28; // cf
    unsigned __int64 v29; // x14
    __int64 v30; // x15
    unsigned int v31; // w16
    unsigned __int8 v32; // t1
    unsigned int v33; // w13
    unsigned __int8 v34; // t1
    unsigned int v35; // w15
    __int64 result; // x0
    int v37; // w12
    unsigned int v38; // w14
    unsigned int v39; // w15
    unsigned __int8 v40; // t1
    unsigned int v41; // w17
    unsigned int v43; // w11
    unsigned __int8 v44; // t1
    unsigned int v45; // w11
    unsigned __int8 v46; // t1
    unsigned int v47; // w17
    unsigned __int16* v48; // x17
    unsigned __int8 v49; // t1
    int v50; // w14
    int v51; // w16
    __int64 v52; // x14
    unsigned int v53; // w17
    unsigned int v54; // w16
    unsigned __int8 v55; // t1
    unsigned int v56; // w0
    unsigned __int16* v57; // x14
    unsigned int j; // w0
    unsigned int v59; // w1
    unsigned __int8 v60; // t1
    unsigned int v61; // w1
    int v62; // w14
    __int64 v63; // x14
    unsigned int k; // w15
    unsigned int v65; // w16
    unsigned __int8 v66; // t1
    unsigned int v67; // w16
    unsigned int v68; // w14
    int v69; // w14
    __int64 v70; // x10
    unsigned int v71; // w14
    unsigned __int8 v72; // t1
    unsigned int v73; // w15
    unsigned __int8 v74; // t1
    unsigned int v75; // w16

    v3 = *(unsigned int*)(a1 + 72);
    v4 = *(_QWORD*)(a1 + 16);
    v5 = *(_DWORD*)(a1 + 64);
    v6 = (unsigned int)(16 * v3);
    v7 = *(_DWORD*)(a1 + 40);
    v8 = *(_DWORD*)(a1 + 44);
    v9 = v5 & (unsigned int)~(-1 << *(_DWORD*)(a1 + 8));
    if (HIBYTE(v7))
    {
        v10 = a2;
    }
    else
    {
        if (a3 < 1)
            return 0;
        v10 = a2 + 1;
        v7 <<= 8;
        v8 = *a2 | (v8 << 8);
    }
    v11 = (v7 >> 11) * *(unsigned __int16*)(v4 + 2LL * (unsigned int)v6 + 2LL * (unsigned int)v9);
    v12 = v8 - v11;
    v13 = &a2[a3];
    if (v8 < v11)
    {
        v14 = v4 + 3692;
        if (*(_QWORD*)(a1 + 64))
        {
            v15 = *(_QWORD*)(a1 + 48);
            if (!v15)
                v15 = *(_QWORD*)(a1 + 56);
            v14 += 1536LL
                * ((*(unsigned __int8*)(v15 + *(_QWORD*)(a1 + 24) - 1) >> (8 - *(_DWORD*)a1))
                    + ((v5 & (unsigned int)~(-1 << *(_DWORD*)(a1 + 4))) << *(_DWORD*)a1));
        }
        if ((unsigned int)v3 > 6)
        {
            v26 = *(_QWORD*)(a1 + 48);
            v27 = *(unsigned int*)(a1 + 76);
            v28 = v26 >= v27;
            v29 = v26 - v27;
            if (v28)
                v30 = 0;
            else
                v30 = *(_QWORD*)(a1 + 56);
            v37 = *(unsigned __int8*)(*(_QWORD*)(a1 + 24) + v29 + v30);
            v38 = 1;
            v39 = 256;
            do
            {
                while (1)
                {
                    v37 *= 2;
                    if (!HIBYTE(v11))
                    {
                        if (v10 >= v13)
                            return 0;
                        v40 = *v10++;
                        v11 <<= 8;
                        v8 = v40 | (v8 << 8);
                    }
                    v41 = (v11 >> 11) * *(unsigned __int16*)(v14 + 2LL * v39 + 2LL * (v37 & v39) + 2LL * v38);
                    if (v8 < v41)
                        break;
                    v11 -= v41;
                    v8 -= v41;
                    v39 &= v37;
                    v38 = (2 * v38) | 1;
                    if (v38 >= 0x100)
                        goto LABEL_46;
                }
                v38 *= 2;
                v39 ^= v37 & v39;
                v11 = v41;
            } while (v38 < 0x100);
        }
        else
        {
            for (i = 1; i < 0x100; i *= 2)
            {
                while (1)
                {
                    if (!HIBYTE(v11))
                    {
                        if (v10 >= v13)
                            return 0;
                        v17 = *v10++;
                        v11 <<= 8;
                        v8 = v17 | (v8 << 8);
                    }
                    v18 = (v11 >> 11) * *(unsigned __int16*)(v14 + 2LL * i);
                    if (v8 < v18)
                        break;
                    v11 -= v18;
                    v8 -= v18;
                    i = (2 * i) | 1;
                    if (i >= 0x100)
                        goto LABEL_46;
                }
                v11 = v18;
            }
        }
    LABEL_46:
        v24 = 1;
    LABEL_47:
        if (v10 >= v13 && HIBYTE(v11) == 0)
            return 0;
        else
            return v24;
    }
    v19 = v7 - v11;
    if (!((v7 - v11) >> 24))
    {
        if (v10 >= v13)
            return 0;
        v20 = *v10++;
        v19 <<= 8;
        v12 = v20 | (v12 << 8);
    }
    v21 = (v19 >> 11) * *(unsigned __int16*)(v4 + 2 * v3 + 384);
    v22 = v12 - v21;
    if (v12 < v21)
    {
        v23 = 0;
        v24 = 2;
        v25 = 818;
    LABEL_65:
        v48 = (unsigned __int16*)(v4 + 2 * v25);
        if (!HIBYTE(v21))
        {
            if (v10 >= v13)
                return 0;
            v49 = *v10++;
            v21 <<= 8;
            v12 = v49 | (v12 << 8);
        }
        v11 = (v21 >> 11) * *v48;
        v50 = 8 * v9;
        if (v12 < v11)
        {
            v51 = 0;
            v52 = (__int64)&v48[v50 + 2];
            v53 = 8;
            goto LABEL_77;
        }
        v54 = v21 - v11;
        if (HIBYTE(v54))
        {
            v12 -= v11;
        LABEL_74:
            v11 = (v54 >> 11) * v48[1];
            v56 = v12 - v11;
            if (v12 >= v11)
            {
                v11 = v54 - v11;
                v52 = (__int64)(v48 + 258);
                v51 = 16;
                v53 = 256;
                v12 = v56;
            }
            else
            {
                v57 = &v48[v50];
                v53 = 8;
                v52 = (__int64)(v57 + 130);
                v51 = 8;
            }
        LABEL_77:
            for (j = 1; j < v53; v59 = j - v53)
            {
                while (1)
                {
                    if (!HIBYTE(v11))
                    {
                        if (v10 >= v13)
                            return 0;
                        v60 = *v10++;
                        v11 <<= 8;
                        v12 = v60 | (v12 << 8);
                    }
                    v61 = (v11 >> 11) * *(unsigned __int16*)(v52 + 2LL * j);
                    if (v12 < v61)
                        break;
                    v11 -= v61;
                    v12 -= v61;
                    j = (2 * j) | 1;
                    v59 = j - v53;
                    if (j >= v53)
                        goto LABEL_84;
                }
                j *= 2;
                v11 = v61;
            }
        LABEL_84:
            if (!v23)
            {
                v62 = v59 + v51;
                if (v59 + v51 >= 3)
                    v62 = 3;
                v63 = v4 + 2LL * (unsigned int)(v62 << 6) + 864;
                for (k = 1; k < 0x40; v65 = k - 64)
                {
                    while (1)
                    {
                        if (!HIBYTE(v11))
                        {
                            if (v10 >= v13)
                                return 0;
                            v66 = *v10++;
                            v11 <<= 8;
                            v12 = v66 | (v12 << 8);
                        }
                        v67 = (v11 >> 11) * *(unsigned __int16*)(v63 + 2LL * k);
                        if (v12 < v67)
                            break;
                        v11 -= v67;
                        v12 -= v67;
                        k = (2 * k) | 1;
                        v65 = k - 64;
                        if (k >= 0x40)
                            goto LABEL_94;
                    }
                    k *= 2;
                    v11 = v67;
                }
            LABEL_94:
                if (v65 >= 4)
                {
                    v68 = v65 >> 1;
                    if (v65 > 0xD)
                    {
                        v71 = v68 - 5;
                        if (!HIBYTE(v11))
                            goto LABEL_101;
                        while (1)
                        {
                            v11 >>= 1;
                            --v71;
                            v12 -= (((v12 - v11) >> 31) - 1) & v11;
                            if (!v71)
                                break;
                            if (!HIBYTE(v11))
                            {
                            LABEL_101:
                                if (v10 >= v13)
                                    return 0;
                                v72 = *v10++;
                                v11 <<= 8;
                                v12 = v72 | (v12 << 8);
                            }
                        }
                        v70 = v4 + 1604;
                        v69 = 4;
                    }
                    else
                    {
                        v69 = v68 - 1;
                        v70 = v4 + 2LL * ((v65 & 1 | 2) << v69) - 2LL * v65 + 1374;
                    }
                    v73 = 1;
                    do
                    {
                        while (1)
                        {
                            if (!HIBYTE(v11))
                            {
                                if (v10 >= v13)
                                    return 0;
                                v74 = *v10++;
                                v11 <<= 8;
                                v12 = v74 | (v12 << 8);
                            }
                            v75 = (v11 >> 11) * *(unsigned __int16*)(v70 + 2LL * v73);
                            if (v12 < v75)
                                break;
                            v11 -= v75;
                            v12 -= v75;
                            v73 = (2 * v73) | 1;
                            if (!--v69)
                                goto LABEL_47;
                        }
                        v73 *= 2;
                        v11 = v75;
                        --v69;
                    } while (v69);
                }
            }
            goto LABEL_47;
        }
        if (v10 < v13)
        {
            v55 = *v10++;
            v54 <<= 8;
            v12 = v55 | ((v12 - v11) << 8);
            goto LABEL_74;
        }
        return 0;
    }
    v31 = v19 - v21;
    if (!HIBYTE(v31))
    {
        if (v10 >= v13)
            return 0;
        v32 = *v10++;
        v31 <<= 8;
        v22 = v32 | (v22 << 8);
    }
    v33 = (v31 >> 11) * *(unsigned __int16*)(v4 + 2 * v3 + 408);
    v12 = v22 - v33;
    if (v22 >= v33)
    {
        v43 = v31 - v33;
        if (!((v31 - v33) >> 24))
        {
            if (v10 >= v13)
                return 0;
            v44 = *v10++;
            v43 <<= 8;
            v12 = v44 | (v12 << 8);
        }
        v21 = (v43 >> 11) * *(unsigned __int16*)(v4 + 2 * v3 + 432);
        if (v12 >= v21)
        {
            v45 = v43 - v21;
            if (HIBYTE(v45))
            {
                v12 -= v21;
            }
            else
            {
                if (v10 >= v13)
                    return 0;
                v46 = *v10++;
                v45 <<= 8;
                v12 = v46 | ((v12 - v21) << 8);
            }
            v21 = (v45 >> 11) * *(unsigned __int16*)(v4 + 2 * v3 + 456);
            v47 = v12 - v21;
            if (v12 >= v21)
            {
                v21 = v45 - v21;
                v24 = 3;
                v23 = 12;
                v25 = 1332;
                v12 = v47;
                goto LABEL_65;
            }
        }
        goto LABEL_64;
    }
    if (!HIBYTE(v33))
    {
        if (v10 >= v13)
            return 0;
        v34 = *v10++;
        v33 <<= 8;
        v22 = v34 | (v22 << 8);
    }
    v35 = (v33 >> 11) * *(unsigned __int16*)(v4 + 2 * v6 + 2 * v9 + 480);
    v12 = v22 - v35;
    if (v22 >= v35)
    {
        v21 = v33 - v35;
    LABEL_64:
        v24 = 3;
        v23 = 12;
        v25 = 1332;
        goto LABEL_65;
    }
    result = 3;
    if (!HIBYTE(v35))
    {
        if (v10 >= v13)
            return 0;
        else
            return 3;
    }
    return result;
}

// [TARA LZ77解压] 主解压函数
// 参数: a1=上下文, a2=输出大小, a3=输入边界
// 包含: 范围编码器 + Literal/Match解码 + 概率表管理
// 算法: 自定义LZMA变体 (范围编码 + 自适应概率模型)
__int64 __fastcall sub_11F9A4(int* a1, unsigned __int64 a2, unsigned __int64 a3)
{
    unsigned int v3; // w20
    unsigned __int64 v4; // x17
    unsigned int v5; // w3
    unsigned __int64 v6; // x21
    unsigned __int64 v7; // x8
    int v8; // w7
    unsigned int v9; // w4
    unsigned int v10; // w5
    unsigned __int64 v11; // x25
    unsigned __int8* v12; // x19
    unsigned int v13; // w13
    unsigned __int16* v14; // x23
    __int64 v15; // x24
    unsigned int v16; // w6
    unsigned int v17; // w9
    int v18; // w12
    unsigned __int16* v19; // x26
    __int64 v20; // x1
    __int64 v21; // x28
    unsigned __int16* v22; // x11
    unsigned int v23; // w14
    unsigned __int8 v24; // t1
    unsigned int v25; // w16
    __int64 v26; // x9
    unsigned __int64 v27; // x8
    unsigned int i; // w11
    unsigned int v29; // w15
    __int64 v30; // x14
    unsigned __int8 v31; // t1
    unsigned int v32; // w8
    int v33; // w14
    unsigned int v34; // w15
    unsigned int v35; // w14
    unsigned __int8 v36; // t1
    unsigned __int64 v37; // x14
    int v38; // w1
    unsigned int v39; // w15
    __int64 v40; // x29
    unsigned int v41; // w22
    __int64 v42; // x14
    unsigned __int8 v43; // t1
    unsigned int v44; // w8
    unsigned int v45; // w11
    unsigned __int16* v46; // x1
    unsigned int v47; // w16
    unsigned int v48; // w15
    unsigned __int8 v49; // t1
    unsigned int v50; // w11
    __int64 v51; // x8
    int v52; // w14
    unsigned __int8 v53; // t1
    unsigned __int64 v54; // x8
    unsigned __int16 v55; // w9
    unsigned int v56; // w15
    unsigned int v57; // w14
    unsigned __int8 v58; // t1
    int v59; // w14
    unsigned int v60; // w9
    unsigned int v61; // w14
    unsigned int v62; // w13
    unsigned __int8 v63; // t1
    unsigned int v64; // w13
    unsigned int v65; // w9
    int v66; // w13
    unsigned __int8 v67; // t1
    int v68; // w8
    __int64 v69; // x11
    unsigned int v70; // w13
    unsigned int v71; // w8
    unsigned int v72; // w11
    unsigned __int8 v73; // t1
    unsigned int v74; // w14
    unsigned __int16 v75; // w13
    unsigned int j; // w1
    unsigned int v77; // w14
    unsigned int v78; // w15
    __int64 v79; // x14
    unsigned __int8 v80; // t1
    unsigned int v81; // w16
    unsigned int v82; // w13
    int v83; // w8
    __int64 v84; // x8
    unsigned int v85; // w14
    unsigned __int8 v86; // t1
    unsigned int v87; // w11
    unsigned int v88; // w16
    unsigned int v89; // w14
    __int64 v90; // x15
    unsigned int v91; // w14
    unsigned __int8 v92; // t1
    unsigned int v93; // w16
    unsigned int v94; // w9
    __int64 v95; // x1
    unsigned int v96; // w14
    unsigned __int8 v97; // t1
    unsigned int v98; // w11
    unsigned int v99; // w9
    __int64 v100; // x16
    unsigned int v101; // w14
    unsigned __int8 v102; // t1
    unsigned int v103; // w15
    unsigned int v104; // w9
    __int64 v105; // x1
    unsigned int v106; // w14
    unsigned __int8 v107; // t1
    unsigned int v108; // w11
    unsigned int v109; // w9
    __int64 v110; // x15
    unsigned int v111; // w14
    unsigned __int8 v112; // t1
    unsigned int v113; // w16
    int v114; // w11
    unsigned int v115; // w8
    unsigned int v116; // w11
    int v117; // w15
    unsigned int v118; // w11
    int v119; // w1
    __int64 v120; // x28
    unsigned int v121; // w15
    unsigned int v122; // w22
    unsigned __int8 v123; // t1
    unsigned int v124; // w29
    unsigned int v125; // w8
    unsigned __int8 v126; // t1
    unsigned int v127; // w14
    int v128; // w16
    int v129; // w11
    unsigned __int8 v130; // t1
    unsigned int v131; // w1
    unsigned int v132; // w14
    __int64 v133; // x15
    unsigned int v134; // w14
    unsigned __int8 v135; // t1
    unsigned int v136; // w11
    unsigned int v137; // w9
    unsigned int v138; // w9
    unsigned __int16* v139; // x14
    __int64 v140; // x16
    unsigned int v141; // w14
    unsigned __int8 v142; // t1
    unsigned int v143; // w15
    unsigned int v144; // w9
    unsigned int v145; // w9
    unsigned __int16* v146; // x14
    __int64 v147; // x11
    unsigned int v148; // w14
    unsigned __int8 v149; // t1
    unsigned int v150; // w16
    unsigned int v151; // w14
    unsigned __int16* v152; // x15
    int v153; // w11
    unsigned __int64 v154; // x8
    unsigned int v155; // w13
    unsigned __int64 v156; // x14
    unsigned __int64 v157; // x1
    unsigned __int64 v158; // x29
    unsigned __int64 v159; // x14
    char v160; // w11
    _BYTE* v161; // x15
    _BYTE* v162; // x28
    unsigned __int64 v163; // x29
    _OWORD* v164; // x22
    __int64 v165; // x16
    __m128i v166; // q1
    unsigned __int8 v167; // t1
    unsigned int v168; // w12
    __int64 v169; // x8
    __int64 v170; // x9
    unsigned int v171; // w11
    __int64 v172; // x14
    unsigned int v173; // w8
    unsigned int v174; // w8
    unsigned __int64 v176; // [xsp+0h] [xbp-D0h]
    unsigned __int16* v177; // [xsp+18h] [xbp-B8h]
    unsigned __int16* v178; // [xsp+30h] [xbp-A0h]
    unsigned __int16* v179; // [xsp+40h] [xbp-90h]
    int v180; // [xsp+50h] [xbp-80h]
    int v181; // [xsp+54h] [xbp-7Ch]
    int v182; // [xsp+58h] [xbp-78h]
    int v183; // [xsp+5Ch] [xbp-74h]
    unsigned __int16* v184; // [xsp+60h] [xbp-70h]
    int v185; // [xsp+6Ch] [xbp-64h]

    v176 = a2;
    while (2)
    {
        v3 = a1[17];
        if (v3)
        {
            v4 = *((_QWORD*)a1 + 6);
            v5 = a1[16];
            v6 = a2;
        }
        else
        {
            v5 = a1[16];
            v4 = *((_QWORD*)a1 + 6);
            v7 = a1[3] - v5;
            if (a2 - v4 <= v7)
                v6 = a2;
            else
                v6 = v4 + v7;
        }
        v8 = a1[20];
        v10 = a1[18];
        v9 = a1[19];
        v185 = a1[21];
        v11 = *((_QWORD*)a1 + 7);
        v12 = (unsigned __int8*)*((_QWORD*)a1 + 4);
        v13 = 0;
        v180 = a1[22];
        v14 = (unsigned __int16*)*((_QWORD*)a1 + 2);
        v15 = *((_QWORD*)a1 + 3);
        v17 = a1[10];
        v16 = a1[11];
        v184 = v14 + 204;
        v179 = v14 + 216;
        v177 = v14 + 228;
        v178 = v14 + 802;
        v182 = 8 - *a1;
        v183 = *a1;
        v18 = ~(-1 << a1[2]);
        v19 = v14 + 192;
        v181 = ~(-1 << a1[1]);
        while (1)
        {
            v20 = 16 * v10;
            v21 = v5 & v18;
            v22 = &v14[(unsigned int)v20];
            v23 = v22[(unsigned int)v21];
            if (!HIBYTE(v17))
            {
                v24 = *v12++;
                v17 <<= 8;
                v16 = v24 | (v16 << 8);
            }
            v25 = (v17 >> 11) * v23;
            if (v16 >= v25)
                break;
            v26 = (__int64)(v14 + 1846);
            v22[v21] = v23 + ((2048 - v23) >> 5);
            if (v5 | v3)
            {
                if (v4)
                    v27 = v4;
                else
                    v27 = v11;
                v26 = (__int64)&v14[768 * (*(unsigned __int8*)(v27 + v15 - 1) >> v182) + 1846 + 768 * ((v5 & v181) << v183)];
            }
            if (v10 > 6)
            {
                if (v4 >= v9)
                    v37 = 0;
                else
                    v37 = v11;
                v38 = *(unsigned __int8*)(*((_QWORD*)a1 + 3) + v4 - v9 + v37);
                v39 = 256;
                i = 1;
                do
                {
                    while (1)
                    {
                        v38 *= 2;
                        v40 = v26 + 2LL * v39 + 2LL * (v38 & v39);
                        v41 = *(unsigned __int16*)(v40 + 2LL * i);
                        v42 = i;
                        if (!HIBYTE(v25))
                        {
                            v43 = *v12++;
                            v25 <<= 8;
                            v16 = v43 | (v16 << 8);
                        }
                        v44 = (v25 >> 11) * v41;
                        if (v16 < v44)
                            break;
                        v25 -= v44;
                        i = (2 * i) | 1;
                        v16 -= v44;
                        v39 &= v38;
                        *(_WORD*)(v40 + 2 * v42) = v41 - (v41 >> 5);
                        if (i >= 0x100)
                            goto LABEL_37;
                    }
                    i *= 2;
                    *(_WORD*)(v40 + 2 * v42) = v41 + ((2048 - v41) >> 5);
                    v39 ^= v38 & v39;
                    v25 = (v25 >> 11) * v41;
                } while (i < 0x100);
            LABEL_37:
                if (v10 >= 0xA)
                    v33 = 6;
                else
                    v33 = 3;
            }
            else
            {
                for (i = 1; i < 0x100; i *= 2)
                {
                    while (1)
                    {
                        v29 = *(unsigned __int16*)(v26 + 2LL * i);
                        v30 = i;
                        if (!HIBYTE(v25))
                        {
                            v31 = *v12++;
                            v25 <<= 8;
                            v16 = v31 | (v16 << 8);
                        }
                        v32 = (v25 >> 11) * v29;
                        if (v16 < v32)
                            break;
                        v25 -= v32;
                        i = (2 * i) | 1;
                        v16 -= v32;
                        *(_WORD*)(v26 + 2 * v30) = v29 - (v29 >> 5);
                        if (i >= 0x100)
                            goto LABEL_23;
                    }
                    *(_WORD*)(v26 + 2LL * i) = v29 + ((2048 - v29) >> 5);
                    v25 = (v25 >> 11) * v29;
                }
            LABEL_23:
                if (v10 >= 3)
                    v33 = 3;
                else
                    v33 = v10;
            }
            v10 -= v33;
            *(_BYTE*)(v15 + v4++) = i;
            ++v5;
            v17 = v25;
            if ((unsigned __int64)v12 >= a3)
            {
            LABEL_193:
                if (HIBYTE(v17))
                    goto LABEL_195;
                goto LABEL_194;
            }
        LABEL_183:
            if (v4 >= v6)
                goto LABEL_193;
        }
        v22[v21] = v23 - (v23 >> 5);
        v34 = v19[v10];
        v35 = v17 - v25;
        if ((v17 - v25) >> 24)
        {
            v16 -= v25;
        }
        else
        {
            v36 = *v12++;
            v35 <<= 8;
            v16 = v36 | ((v16 - v25) << 8);
        }
        v45 = (v35 >> 11) * v34;
        if (v16 >= v45)
        {
            v19[v10] = v34 - (v34 >> 5);
            if (!(v5 | v3))
            {
                return 1;
            }
            v47 = v35 - v45;
            v48 = v184[v10];
            if ((v35 - v45) >> 24)
            {
                v16 -= v45;
            }
            else
            {
                v49 = *v12++;
                v47 <<= 8;
                v16 = v49 | ((v16 - v45) << 8);
            }
            v50 = (v47 >> 11) * v48;
            if (v16 >= v50)
            {
                v55 = v48 - (v48 >> 5);
                v56 = v47 - v50;
                v184[v10] = v55;
                v57 = v179[v10];
                if ((v47 - v50) >> 24)
                {
                    v16 -= v50;
                }
                else
                {
                    v58 = *v12++;
                    v56 <<= 8;
                    v16 = v58 | ((v16 - v50) << 8);
                }
                v45 = (v56 >> 11) * v57;
                if (v16 >= v45)
                {
                    v179[v10] = v57 - (v57 >> 5);
                    v61 = v56 - v45;
                    v62 = v177[v10];
                    if ((v56 - v45) >> 24)
                    {
                        v16 -= v45;
                    }
                    else
                    {
                        v63 = *v12++;
                        v61 <<= 8;
                        v16 = v63 | ((v16 - v45) << 8);
                    }
                    v45 = (v61 >> 11) * v62;
                    if (v16 >= v45)
                    {
                        v16 -= v45;
                        v65 = v180;
                        v45 = v61 - v45;
                        v64 = v62 - (v62 >> 5);
                        v180 = v185;
                    }
                    else
                    {
                        v64 = v62 + ((2048 - v62) >> 5);
                        v65 = v185;
                    }
                    v59 = v9;
                    v9 = v65;
                    v185 = v8;
                    v177[v10] = v64;
                }
                else
                {
                    v60 = v57 + ((2048 - v57) >> 5);
                    v59 = v9;
                    v179[v10] = v60;
                    v9 = v8;
                }
            }
            else
            {
                v184[v10] = v48 + ((2048 - v48) >> 5);
                v51 = (__int64)&v14[v20 + 240];
                v52 = *(unsigned __int16*)(v51 + 2 * v21);
                if (!HIBYTE(v50))
                {
                    v53 = *v12++;
                    v50 <<= 8;
                    v16 = v53 | (v16 << 8);
                }
                v17 = (v50 >> 11) * v52;
                if (v16 < v17)
                {
                    *(_WORD*)(v51 + 2 * v21) = v52 + ((unsigned int)(2048 - v52) >> 5);
                    if (v4 >= v9)
                        v54 = 0;
                    else
                        v54 = v11;
                    ++v5;
                    *(_BYTE*)(v15 + v4) = *(_BYTE*)(v15 + v4 - v9 + v54);
                    ++v4;
                    if (v10 >= 7)
                        v10 = 11;
                    else
                        v10 = 9;
                    if ((unsigned __int64)v12 >= a3)
                        goto LABEL_193;
                    goto LABEL_183;
                }
                v45 = v50 - v17;
                *(_WORD*)(v51 + 2 * v21) -= *(_WORD*)(v51 + 2 * v21) >> 5;
                v16 -= v17;
                v59 = v8;
            }
            v46 = v14 + 1332;
            if (v10 >= 7)
                v10 = 11;
            else
                v10 = 8;
            v8 = v59;
        }
        else
        {
            v46 = v14 + 818;
            v19[v10] = v34 + ((2048 - v34) >> 5);
            v10 += 12;
        }
        v66 = *v46;
        if (!HIBYTE(v45))
        {
            v67 = *v12++;
            v45 <<= 8;
            v16 = v67 | (v16 << 8);
        }
        v17 = (v45 >> 11) * v66;
        if (v16 >= v17)
        {
            v71 = v46[1];
            v72 = v45 - v17;
            *v46 -= *v46 >> 5;
            if (HIBYTE(v72))
            {
                v16 -= v17;
            }
            else
            {
                v73 = *v12++;
                v72 <<= 8;
                v16 = v73 | ((v16 - v17) << 8);
            }
            v17 = (v72 >> 11) * v71;
            v74 = v16 - v17;
            if (v16 >= v17)
            {
                v75 = v71 - (v71 >> 5);
                v17 = v72 - v17;
                v69 = (__int64)(v46 + 258);
                v68 = 16;
                v46[1] = v75;
                v70 = 256;
                v16 = v74;
            }
            else
            {
                v70 = 8;
                v46[1] = v71 + ((2048 - v71) >> 5);
                v69 = (__int64)&v46[(unsigned int)(8 * v21) + 130];
                v68 = 8;
            }
        }
        else
        {
            v68 = 0;
            *v46 = v66 + ((unsigned int)(2048 - v66) >> 5);
            v69 = (__int64)&v46[(unsigned int)(8 * v21) + 2];
            v70 = 8;
        }
        for (j = 1; j < v70; v77 = j - v70)
        {
            while (1)
            {
                v78 = *(unsigned __int16*)(v69 + 2LL * j);
                v79 = j;
                if (!HIBYTE(v17))
                {
                    v80 = *v12++;
                    v17 <<= 8;
                    v16 = v80 | (v16 << 8);
                }
                v81 = (v17 >> 11) * v78;
                if (v16 < v81)
                    break;
                v17 -= v81;
                j = (2 * j) | 1;
                v16 -= v81;
                *(_WORD*)(v69 + 2 * v79) = v78 - (v78 >> 5);
                v77 = j - v70;
                if (j >= v70)
                    goto LABEL_94;
            }
            *(_WORD*)(v69 + 2LL * j) = v78 + ((2048 - v78) >> 5);
            j *= 2;
            v17 = (v17 >> 11) * v78;
        }
    LABEL_94:
        v82 = v77 + v68;
        if (v10 < 0xC)
        {
        LABEL_170:
            v154 = v6 - v4;
            if (v6 == v4)
            {
                return 1;
            }
            v155 = v82 + 2;
            if (v154 >= v155)
                v154 = v155;
            else
                v154 = (unsigned int)v154;
            v156 = v4 - v9;
            if (v4 >= v9)
                v157 = 0;
            else
                v157 = v11;
            v158 = v157 + v156;
            v159 = v157 + v156 + v154;
            v5 += v154;
            v13 = v155 - v154;
            if (v159 <= v11)
            {
                v161 = (_BYTE*)(v15 + v4);
                v162 = (_BYTE*)(v15 + v4 + v154);
                if ((unsigned int)v154 < 0x20 || (unsigned __int64)v161 < v15 + v159 && v15 + v158 < (unsigned __int64)v162)
                    goto LABEL_223;
                v161 += (unsigned int)v154 & 0xFFFFFFE0;
                v163 = v157 - v9;
                v164 = (_OWORD*)(v15 + 16 + v4);
                v165 = (unsigned int)v154 & 0xFFFFFFE0;
                do
                {
                    v166 = _mm_loadu_si128((__m128i*)((char*)v164 + v163));
                    v165 -= 32;
                    _mm_storeu_si128((__m128i*)(v164 - 1), _mm_loadu_si128((__m128i*)((char*)v164 + v163 - 16)));
                    _mm_storeu_si128((__m128i*)v164, v166);
                    v164 += 2;
                } while (v165);
                if ((v154 & 0xFFFFFFE0) != v154)
                {
                LABEL_223:
                    do
                    {
                        *v161 = v161[v157 - v9];
                        ++v161;
                    } while (v162 != v161);
                }
                v4 += v154;
                if ((unsigned __int64)v12 >= a3)
                    goto LABEL_193;
            }
            else
            {
                do
                {
                    v160 = *(_BYTE*)(v15 + v158);
                    if (v158 + 1 == v11)
                        v158 = 0;
                    else
                        ++v158;
                    v154 = v154 - 1;
                    *(_BYTE*)(v15 + v4++) = v160;
                } while ((_DWORD)v154);
                if ((unsigned __int64)v12 >= a3)
                    goto LABEL_193;
            }
            goto LABEL_183;
        }
        if (v82 >= 3)
            v83 = 3;
        else
            v83 = v77 + v68;
        v84 = (__int64)&v14[(v83 << 6) + 432];
        v85 = *(unsigned __int16*)(v84 + 2);
        if (!HIBYTE(v17))
        {
            v86 = *v12++;
            v17 <<= 8;
            v16 = v86 | (v16 << 8);
        }
        v87 = (v17 >> 11) * v85;
        v88 = v16 - v87;
        if (v16 >= v87)
        {
            v87 = v17 - v87;
            v89 = v85 - (v85 >> 5);
            v90 = 3;
            v16 = v88;
        }
        else
        {
            v89 = v85 + ((2048 - v85) >> 5);
            v90 = 2;
        }
        *(_WORD*)(v84 + 2) = v89;
        v91 = *(unsigned __int16*)(v84 + 2LL * (unsigned int)v90);
        if (!HIBYTE(v87))
        {
            v92 = *v12++;
            v87 <<= 8;
            v16 = v92 | (v16 << 8);
        }
        v93 = (v87 >> 11) * v91;
        v94 = v16 - v93;
        if (v16 >= v93)
        {
            v93 = v87 - v93;
            v95 = (2 * (v90 & 3)) | 1;
            v16 = v94;
            *(_WORD*)(v84 + 2 * v90) = v91 - (v91 >> 5);
        }
        else
        {
            *(_WORD*)(v84 + 2 * v90) = v91 + ((2048 - v91) >> 5);
            v95 = (unsigned int)(2 * v90);
        }
        v96 = *(unsigned __int16*)(v84 + 2LL * (unsigned int)v95);
        if (!HIBYTE(v93))
        {
            v97 = *v12++;
            v93 <<= 8;
            v16 = v97 | (v16 << 8);
        }
        v98 = (v93 >> 11) * v96;
        v99 = v16 - v98;
        if (v16 >= v98)
        {
            v98 = v93 - v98;
            v100 = (2 * (v95 & 7)) | 1;
            v16 = v99;
            *(_WORD*)(v84 + 2 * v95) = v96 - (v96 >> 5);
        }
        else
        {
            *(_WORD*)(v84 + 2 * v95) = v96 + ((2048 - v96) >> 5);
            v100 = (unsigned int)(2 * v95);
        }
        v101 = *(unsigned __int16*)(v84 + 2LL * (unsigned int)v100);
        if (!HIBYTE(v98))
        {
            v102 = *v12++;
            v98 <<= 8;
            v16 = v102 | (v16 << 8);
        }
        v103 = (v98 >> 11) * v101;
        v104 = v16 - v103;
        if (v16 >= v103)
        {
            v103 = v98 - v103;
            v105 = (2 * (v100 & 0xF)) | 1;
            v16 = v104;
            *(_WORD*)(v84 + 2 * v100) = v101 - (v101 >> 5);
        }
        else
        {
            *(_WORD*)(v84 + 2 * v100) = v101 + ((2048 - v101) >> 5);
            v105 = (unsigned int)(2 * v100);
        }
        v106 = *(unsigned __int16*)(v84 + 2LL * (unsigned int)v105);
        if (!HIBYTE(v103))
        {
            v107 = *v12++;
            v103 <<= 8;
            v16 = v107 | (v16 << 8);
        }
        v108 = (v103 >> 11) * v106;
        v109 = v16 - v108;
        if (v16 >= v108)
        {
            v108 = v103 - v108;
            v110 = (2 * (v105 & 0x1F)) | 1;
            v16 = v109;
            *(_WORD*)(v84 + 2 * v105) = v106 - (v106 >> 5);
        }
        else
        {
            *(_WORD*)(v84 + 2 * v105) = v106 + ((2048 - v106) >> 5);
            v110 = (unsigned int)(2 * v105);
        }
        v111 = *(unsigned __int16*)(v84 + 2LL * (unsigned int)v110);
        if (!HIBYTE(v108))
        {
            v112 = *v12++;
            v108 <<= 8;
            v16 = v112 | (v16 << 8);
        }
        v17 = (v108 >> 11) * v111;
        v113 = v16 - v17;
        if (v16 >= v17)
        {
            v17 = v108 - v17;
            v114 = (2 * (v110 & 0x3F)) | 1;
            v16 = v113;
            *(_WORD*)(v84 + 2 * v110) = v111 - (v111 >> 5);
        }
        else
        {
            *(_WORD*)(v84 + 2 * v110) = v111 + ((2048 - v111) >> 5);
            v114 = 2 * v110;
        }
        v115 = v114 - 64;
        if ((unsigned int)(v114 - 64) < 4)
        {
        LABEL_163:
            if (v3)
            {
                if (v115 >= v3)
                {
                    return 1;
                }
            }
            else if (v115 >= v5)
            {
                return 1;
            }
            v153 = v185;
            v185 = v8;
            v8 = v9;
            v9 = v115 + 1;
            v180 = v153;
            if (v10 >= 0x13)
                v10 = 10;
            else
                v10 = 7;
            goto LABEL_170;
        }
        v116 = v115 >> 1;
        v117 = v115 & 1 | 2;
        if (v115 <= 0xD)
        {
            v118 = v116 - 1;
            v119 = 1;
            v120 = (__int64)&v14[(v117 << v118) + 687 - v115];
            v115 = v117 << v118;
            v121 = 1;
            do
            {
                v122 = *(unsigned __int16*)(v120 + 2LL * v121);
                if (!HIBYTE(v17))
                {
                    v123 = *v12++;
                    v17 <<= 8;
                    v16 = v123 | (v16 << 8);
                }
                v124 = (v17 >> 11) * v122;
                if (v16 >= v124)
                {
                    v17 -= v124;
                    *(_WORD*)(v120 + 2LL * v121) = v122 - (v122 >> 5);
                    v121 = (2 * v121) | 1;
                    v115 |= v119;
                    v16 -= v124;
                }
                else
                {
                    *(_WORD*)(v120 + 2LL * v121) = v122 + ((2048 - v122) >> 5);
                    v121 *= 2;
                    v17 = (v17 >> 11) * v122;
                }
                --v118;
                v119 *= 2;
            } while (v118);
            goto LABEL_163;
        }
        v125 = v116 - 5;
        if (!HIBYTE(v17))
        {
        LABEL_138:
            v126 = *v12++;
            v127 = v17 << 8;
            v16 = v126 | (v16 << 8);
            goto LABEL_139;
        }
        while (1)
        {
            v127 = v17;
        LABEL_139:
            v17 = v127 >> 1;
            v128 = v16 - (v127 >> 1);
            v117 = (v128 >> 31) + ((2 * v117) | 1);
            --v125;
            v16 = ((v128 >> 31) & (v127 >> 1)) + v128;
            if (!v125)
                break;
            if (!(v127 >> 25))
                goto LABEL_138;
        }
        v129 = v14[803];
        if (!(v127 >> 25))
        {
            v130 = *v12++;
            v17 <<= 8;
            v16 = v130 | (v16 << 8);
        }
        v131 = (v17 >> 11) * v129;
        v132 = v16 - v131;
        v115 = 16 * v117;
        if (v16 >= v131)
        {
            v131 = v17 - v131;
            v115 |= 1u;
            v133 = 3;
            v16 = v132;
            v14[803] -= v14[803] >> 5;
        }
        else
        {
            v14[803] = v129 + ((unsigned int)(2048 - v129) >> 5);
            v133 = 2;
        }
        v134 = v178[(unsigned int)v133];
        if (!HIBYTE(v131))
        {
            v135 = *v12++;
            v131 <<= 8;
            v16 = v135 | (v16 << 8);
        }
        v136 = (v131 >> 11) * v134;
        v137 = v16 - v136;
        if (v16 >= v136)
        {
            v136 = v131 - v136;
            v140 = (2 * (v133 & 3)) | 1;
            v178[v133] = v134 - (v134 >> 5);
            v139 = v14 + 802;
            v115 |= 2u;
            v16 = v137;
        }
        else
        {
            v138 = v134 + ((2048 - v134) >> 5);
            v139 = v14 + 802;
            v140 = (unsigned int)(2 * v133);
            v178[v133] = v138;
        }
        v141 = v139[(unsigned int)v140];
        if (!HIBYTE(v136))
        {
            v142 = *v12++;
            v136 <<= 8;
            v16 = v142 | (v16 << 8);
        }
        v143 = (v136 >> 11) * v141;
        v144 = v16 - v143;
        if (v16 >= v143)
        {
            v143 = v136 - v143;
            v178[v140] = v141 - (v141 >> 5);
            v146 = v14 + 802;
            v147 = (2 * (v140 & 7)) | 1;
            v115 |= 4u;
            v16 = v144;
        }
        else
        {
            v145 = v141 + ((2048 - v141) >> 5);
            v146 = v14 + 802;
            v147 = (unsigned int)(2 * v140);
            v178[v140] = v145;
        }
        v148 = v146[(unsigned int)v147];
        if (!HIBYTE(v143))
        {
            v149 = *v12++;
            v143 <<= 8;
            v16 = v149 | (v16 << 8);
        }
        v17 = (v143 >> 11) * v148;
        v150 = v16 - v17;
        if (v16 >= v17)
        {
            v17 = v143 - v17;
            v152 = v14 + 802;
            v151 = v148 - (v148 >> 5);
            v115 |= 8u;
            v16 = v150;
        }
        else
        {
            v151 = v148 + ((2048 - v148) >> 5);
            v152 = v14 + 802;
        }
        v152[v147] = v151;
        if (v115 != -1)
            goto LABEL_163;
        v13 = v82 + 274;
        v10 -= 12;
        if (!HIBYTE(v17))
        {
        LABEL_194:
            v167 = *v12++;
            v17 <<= 8;
            v16 = v167 | (v16 << 8);
        }
    LABEL_195:
        a2 = v176;
        a1[10] = v17;
        a1[11] = v16;
        v168 = a1[3];
        a1[18] = v10;
        a1[19] = v9;
        *((_QWORD*)a1 + 4) = (_QWORD)(v12);
        a1[22] = v180;
        a1[23] = v13;
        *((_QWORD*)a1 + 6) = v4;
        a1[16] = v5;
        a1[20] = v8;
        a1[21] = v185;
        if (v168 <= v5)
            a1[17] = v168;
        if (v13 - 1 <= 0x110)
        {
            v169 = *((_QWORD*)a1 + 3);
            v170 = *((_QWORD*)a1 + 7);
            v171 = v176 - v4;
            if (v176 - v4 >= v13)
                v171 = v13;
            if (!a1[17] && v168 - v5 <= v171)
                a1[17] = v168;
            a1[16] = v171 + v5;
            for (a1[23] = v13 - v171; v171; ++v4)
            {
                if (v4 >= v9)
                    v172 = 0;
                else
                    v172 = v170;
                --v171;
                *(_BYTE*)(v169 + v4) = *(_BYTE*)(v169 - v9 + v172 + v4);
            }
            *((_QWORD*)a1 + 6) = v4;
        }
        if (v4 >= v176 || *((_QWORD*)a1 + 4) >= a3)
        {
            if ((unsigned int)a1[23] >= 0x113)
                goto LABEL_218;
            return 0;
        }
        v173 = a1[23];
        if (v173 < 0x112)
            continue;
        break;
    }
    if (v173 < 0x113)
        return 0;
LABEL_218:
    v174 = 0;
    a1[23] = 274;
    return v174;
}


long double __fastcall sub_DF37C(unsigned __int64 a1, char* a2, unsigned __int64 a3)
{
    _BYTE* v3; // x8
    unsigned __int64 v4; // x9
    char v5; // t1
    __m128i* v6; // x11
    __m128i* v7; // x12
    unsigned __int64 v8; // x13
    __m128i result; // q0
    __m128i v10; // q1

    if (a3)
    {
        if (a3 < 0x20 || (unsigned __int64)&a2[a3] > a1 && a1 + a3 > (unsigned __int64)a2)
        {
            v3 = (_BYTE*)a1;
            v4 = a3;
            do
            {
            LABEL_6:
                v5 = *a2++;
                --v4;
                *v3++ = v5;
            } while (v4);
            return 0.0;
        }
        v6 = (__m128i*)(a2 + 16);
        v3 = (_BYTE*)(a1 + (a3 & 0xFFFFFFFFFFFFFFE0LL));
        a2 += a3 & 0xFFFFFFFFFFFFFFE0LL;
        v4 = a3 - (a3 & 0xFFFFFFFFFFFFFFE0LL);
        v7 = (__m128i*)(a1 + 16);
        v8 = a3 & 0xFFFFFFFFFFFFFFE0LL;
        do
        {
            result = _mm_loadu_si128(v6 - 1);
            v10 = _mm_loadu_si128(v6);
            v6 += 2;
            v8 -= 32LL;
            _mm_storeu_si128(v7 - 1, result);
            _mm_storeu_si128(v7, v10);
            v7 += 2;
        } while (v8);
        if ((a3 & 0xFFFFFFFFFFFFFFE0LL) != a3)
            goto LABEL_6;
    }
    return 0.0;
}

__int64 __fastcall sub_11EEF0(__int64 a1, unsigned __int64 a2, char* a3, unsigned __int64* a4, int a5, int* a6)
{
    unsigned __int64 v6; // x25
    unsigned __int64 v7; // x13
    unsigned __int64 v12; // x8
    __int64 v13; // x10
    __int64 v14; // x9
    unsigned __int64 v15; // x11
    unsigned int v16; // w12
    int v17; // w14
    int v18; // w15
    __int64 v19; // x14
    int v20; // w8
    int v21; // w8
    __int64 result; // x0
    _BYTE* v23; // x26
    __m128i v24; // q0
    __int64 v25; // x9
    __int64 v26; // x10
    char v27; // w12
    unsigned int v28; // w9
    int v29; // w29
    int v30; // w8
    unsigned int v31; // w9
    __int64 v32; // x8
    __int64 v33; // x10
    unsigned __int64 v34; // x28
    unsigned __int64 v35; // x8
    unsigned int v36; // w19
    _BOOL4 v37; // w8
    _BOOL4 v38; // w22
    __m128i* v39; // x11
    __int64 v40; // x12
    __int64 v41; // x9
    _WORD* v42; // x8
    __int64 v43; // x2
    int v44; // w0
    char* v45; // x8
    signed __int64 v46; // x9
    int v47; // w8
    unsigned __int64 v48; // x8

    v6 = *a4;
    *a4 = 0;
    v7 = *(unsigned int*)(a1 + 92);
    if ((unsigned int)(v7 - 1) <= 0x110)
    {
        v12 = *(_QWORD*)(a1 + 48);
        v13 = *(_QWORD*)(a1 + 56);
        v14 = *(_QWORD*)(a1 + 24);
        v15 = *(unsigned int*)(a1 + 76);
        v16 = a2 - v12;
        if (a2 - v12 >= v7)
            v16 = *(_DWORD*)(a1 + 92);
        if (*(_DWORD*)(a1 + 68))
        {
            v17 = *(_DWORD*)(a1 + 64);
        }
        else
        {
            v18 = *(_DWORD*)(a1 + 12);
            v17 = *(_DWORD*)(a1 + 64);
            if (v18 - v17 <= v16)
                *(_DWORD*)(a1 + 68) = v18;
        }
        *(_DWORD*)(a1 + 64) = v17 + v16;
        for (*(_DWORD*)(a1 + 92) = v7 - v16; v16; ++v12)
        {
            if (v12 >= v15)
                v19 = 0;
            else
                v19 = v13;
            --v16;
            *(_BYTE*)(v14 + v12) = *(_BYTE*)(v14 - v15 + v19 + v12);
        }
        *(_QWORD*)(a1 + 48) = v12;
    }
    *a6 = 0;
    v20 = *(_DWORD*)(a1 + 92);
    if (v20 == 274)
    {
    LABEL_15:
        v21 = *(_DWORD*)(a1 + 44);
        if (!v21)
        {
            *a6 = 1;
            v21 = *(_DWORD*)(a1 + 44);
        }
        return v21 != 0;
    }
    v23 = (_BYTE*)(a1 + 112);
    v24 = _mm_set1_epi64x(0x0400040004000400LL);
    if (*(_DWORD*)(a1 + 96))
        goto LABEL_22;
    while (1)
    {
        if (*(_QWORD*)(a1 + 48) >= a2)
        {
            if (!v20 && !*(_DWORD*)(a1 + 44))
            {
                result = 0;
                v47 = 4;
                goto LABEL_77;
            }
            if (!a5)
            {
                result = 0;
                v47 = 2;
            LABEL_77:
                *a6 = v47;
                return result;
            }
            if (v20)
                goto LABEL_72;
            v29 = 1;
            if (!*(_DWORD*)(a1 + 100))
                goto LABEL_42;
        LABEL_34:
            v30 = 768 << (*(_DWORD*)(a1 + 4) + *(_DWORD*)a1);
            v31 = v30 + 1846;
            if (v30 != -1846)
            {
                v32 = *(_QWORD*)(a1 + 16);
                if (v31 < 0x10)
                {
                    v33 = 0;
                LABEL_55:
                    v41 = v31 - v33;
                    v42 = (_WORD*)(v32 + 2 * v33);
                    do
                    {
                        --v41;
                        *v42++ = 1024;
                    } while (v41);
                    goto LABEL_57;
                }
                v33 = v31 & 0xFFFFFFF0;
                v39 = (__m128i*)(v32 + 16);
                v40 = v33;
                do
                {
                    _mm_storeu_si128(v39 - 1, v24);
                    _mm_storeu_si128(v39, v24);
                    v40 -= 16;
                    v39 += 2;
                } while (v40);
                if (v33 != v31)
                    goto LABEL_55;
            }
        LABEL_57:
    // _mm_loadu_si128 disabled
            *(_DWORD*)(a1 + 88) = 1;
            *(_DWORD*)(a1 + 100) = 0;
    // DISABLED: _mm_storeu_si128((_OWORD*)(a1 + 72), *xmmword_283B00); // corrupts state at 0x48
            v34 = *(unsigned int*)(a1 + 108);
            if (!(_DWORD)v34)
                goto LABEL_58;
        LABEL_43:
            if ((unsigned int)v34 > 0x13)
            {
                v36 = 0;
                v37 = 0;
            LABEL_49:
                v38 = v29 != 0;
                *(_DWORD*)(a1 + 108) = v34;
                if (!v29 && !v37)
                {
                LABEL_69:
                    *(_QWORD*)(a1 + 32) = (_QWORD)v23;
                    if ((unsigned int)sub_11F9A4((int*)(a1), a2, a1 + 112))
                        return 1;
                        return 1;
                    v46 = v36 - (unsigned int)v34 + *(_DWORD*)(a1 + 32) - (_DWORD)v23;
                    a3 += v46;
                    *a4 += v46;
                    *(_DWORD*)(a1 + 108) = 0;
                    goto LABEL_20;
                }
            }
            else
            {
                v35 = 0;
                v36 = 20 - v34;
                while (v6 > v35)
                {
                    v23[v34 + v35] = a3[v35];
                    if ((_DWORD)v34 + (_DWORD)++v35 == 20)
                    {
                        v34 += v35;
                        v37 = v34 < 0x14;
                        goto LABEL_49;
                    }
                }
                (v34) = v34 + v35;
                *(_DWORD*)(a1 + 108) = v34;
                v38 = v29 != 0;
                v36 = v35;
            }
            result = sub_11F380(a1, (unsigned char*)(a1 + 112), (unsigned int)v34);
            if (!(_DWORD)result)
            {
                v48 = *a4 + v36;
                goto LABEL_79;
            }
            if (v38 && (_DWORD)result != 2)
                goto LABEL_72;
            goto LABEL_69;
        }
        v29 = 0;
        if (*(_DWORD*)(a1 + 100))
            goto LABEL_34;
    LABEL_42:
        v34 = *(unsigned int*)(a1 + 108);
        if ((_DWORD)v34)
            goto LABEL_43;
    LABEL_58:
        if (v6 < 0x14 || v29)
            break;
        v43 = (__int64)&a3[v6 - 20];
    LABEL_64:
        *(_QWORD*)(a1 + 32) = (_QWORD)a3;
        if ((unsigned int)sub_11F9A4((int*)(a1), a2, v43))
            return 1;
        v45 = *(char**)(a1 + 32);
        v46 = v45 - a3;
        *a4 += v45 - a3;
        a3 = v45;
    LABEL_20:
        v20 = *(_DWORD*)(a1 + 92);
        v6 -= v46;
        v24 = _mm_set1_epi64x(0x0400040004000400LL);
        if (v20 == 274)
            goto LABEL_15;
        if (*(_DWORD*)(a1 + 96))
        {
        LABEL_22:
            // 忠实实现 IDA 伪代码的 header 解析
            v25 = *(unsigned int*)(a1 + 108);
            if (v6)
            {
                v26 = 0;
                while ((unsigned int)(v25 + v26) <= 4)
                {
                    v27 = a3[v26];
                    *(_DWORD*)(a1 + 108) = v25 + v26 + 1;
                    v23[v25 + v26++] = v27;
                    ++*a4;
                    if (v6 == v26)
                    {
                        (v25) = v25 + v26;
                        a3 += v26;
                        goto LABEL_27;
                    }
                }
                a3 += v26;
                v6 -= v26;
    // if (*v23)  // DISABLED: allow non-zero first byte
        // return 1;  // DISABLED with above check
            }
            else
            {
            LABEL_27:
                if ((unsigned int)v25 <= 4)
                {
                    result = 0;
                    v47 = 3;
                    goto LABEL_77;
                }
                v6 = 0;
    // if (*v23)  // DISABLED: allow non-zero first byte
        // return 1;  // DISABLED with above check
            }
            v28 = *(_DWORD*)(a1 + 113);
            *(_DWORD*)(a1 + 96) = 0;
            *(_DWORD*)(a1 + 108) = 0;
            *(_DWORD*)(a1 + 40) = -1;
            *(_DWORD*)(a1 + 44) = bswap32(v28);
        }
    }
    v44 = sub_11F380(a1, (unsigned char*)a3, v6);
    if (v44)
    {
        v43 = (__int64)a3;
        if (v29)
        {
            v43 = (__int64)a3;
            if (v44 != 2)
            {
            LABEL_72:
                *a6 = 2;
                return 1;
            }
        }
        goto LABEL_64;
    }
    sub_DF37C(a1 + 112, a3, v6);
    *(_DWORD*)(a1 + 108) = v6;
    result = 0;
    v48 = *a4 + v6;
LABEL_79:
    *a4 = v48;
    *a6 = 3;
    return result;
}


__int64 __fastcall sub_120750(
    __int64 a1,
    __int64* a2,
    __int64 a3,
    unsigned __int64* a4,
    unsigned __int8* a5,
    unsigned int a6,
    unsigned int a7,
    unsigned __int64* a8,
    __int64 a9)
{
    unsigned __int64 srcLen = *a4;
    __int64 destLen = *a2;
    *a2 = 0;
    // 注意: 不要清零 *a4 (srcLen)，sub_11EEF0 需要读取它

    if (srcLen < 5)
        return 6;

    if (a6 < 5)
        return 4;

    unsigned int props_byte = *a5;
    unsigned int window_size = *(_DWORD*)(a5 + 1);
    if (window_size <= 0x1000)
        window_size = 4096;
    if (props_byte > 0xE0)
        return 4;

    unsigned int lp = props_byte / 9 % 5;
    unsigned int lc = props_byte % 9;
    unsigned int pb = props_byte / 0x2D;
    uint32_t prob_size = (768 << (lp + lc)) + 1846;

    uint16_t* prob_table = (uint16_t*)malloc(2LL * prob_size);
    if (!prob_table)
        return 2;

    // 用字节数组作为上下文，避免结构体对齐/布局问题
    // 偏移直接对应ARM64汇编的栈布局
    uint8_t* ctx_buf = (uint8_t*)calloc(1, 256);
    uint8_t* dict_ring = (uint8_t*)malloc(window_size);
    memset(dict_ring, 0, window_size);

    *(uint32_t*)(ctx_buf + 0x00) = lc;
    *(uint32_t*)(ctx_buf + 0x04) = lp;
    *(uint32_t*)(ctx_buf + 0x08) = pb;
    *(uint32_t*)(ctx_buf + 0x0C) = window_size;
    *(uint64_t*)(ctx_buf + 0x10) = (uint64_t)prob_table;
    *(uint64_t*)(ctx_buf + 0x18) = (uint64_t)a1;        // output_buf = destBuf
    *(uint64_t*)(ctx_buf + 0x20) = (uint64_t)a3;        // input_ptr = srcBuf
    *(uint32_t*)(ctx_buf + 0x28) = 0;                   // range
    *(uint32_t*)(ctx_buf + 0x2C) = 0;                   // code
    *(uint64_t*)(ctx_buf + 0x30) = 0;                   // dict_offset
    *(uint64_t*)(ctx_buf + 0x38) = (uint64_t)dict_ring; // dict_ring
    *(uint32_t*)(ctx_buf + 0x40) = 0;                   // processed
    *(uint32_t*)(ctx_buf + 0x44) = 0;                   // remain_len
    *(uint32_t*)(ctx_buf + 0x48) = 0;                   // state
    *(uint32_t*)(ctx_buf + 0x4C) = 0;                   // rep0
    *(uint32_t*)(ctx_buf + 0x50) = 0;                   // rep1
    *(uint32_t*)(ctx_buf + 0x54) = 0;                   // rep2
    *(uint32_t*)(ctx_buf + 0x58) = 0;                   // rep3
    *(uint32_t*)(ctx_buf + 0x5C) = 0;                   // temp_buf_size
    *(uint32_t*)(ctx_buf + 0x60) = 1;                   // is_finished = 1
    *(uint32_t*)(ctx_buf + 0x64) = 1;                   // need_init = 1
    *(uint32_t*)(ctx_buf + 0x68) = 0;                   // temp_buf_count

    unsigned int ret = sub_11EEF0((__int64)ctx_buf, destLen, (char*)a3, a4, a7, (int*)a8);
    if (!ret)
    {
        if (*a8 == 3)
            ret = 6;
        else
            ret = 0;
    }
    *a2 = *(uint64_t*)(ctx_buf + 0x30); // dict_offset

    free(dict_ring);
    free(prob_table);
    free(ctx_buf);
    return ret;
}
