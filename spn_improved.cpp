//
// Created by 李玉哲 on 14/09/2017.
//



#include <cstdint>
#include <cstdio>
#include <cmath>
#include <inttypes.h>

unsigned s_m[] = {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7};
unsigned inv_s_m[] = {14,3,4,8,1,12,10,15,7,13,9,6,11,2,0,5};

unsigned p_m[64] = {
        0, 8, 16, 24, 32, 40, 48, 56,
        2, 10,18, 26, 34, 42, 50, 58,
        1, 9, 17, 25, 33, 41, 49, 57,
        3, 11,19, 27, 35, 43, 51, 59,
        5, 13,21, 29, 37, 45, 53, 61,
        4, 12,20, 28, 36, 44, 52, 60,
        6, 14,22, 30, 38, 46, 54, 62,
        7, 15,23, 31, 39, 47, 55, 63
};


unsigned inv_p_m[64] = {0};



uint64_t sTrans64(uint64_t u, unsigned s[]) {
    uint64_t res = 0;
    uint64_t tmp = 0;

    for (int i = 0; i < 8; i++) {
        res |= (s[(u>>(i*4))&0xF]) << (i * 4);
    }
    for (int i = 0; i < 8; i++) {
        tmp |= (s[(u>>((i+8)*4))&0XF]) << (i*4);
    }
    for (int i = 0; i < 32; i++) {
        tmp *= 2;
    }
    tmp += res;
    return tmp;
}


uint64_t pTrans64(uint64_t v, unsigned p[]) {
    uint64_t res = 0;
    for (int i = 0; i < 64; i++) {
        res |= ((v>>(63-p[i]))&0x1) << (63-i);
    }
    return res;
}

const int Nr64 = 12;

void geneRoundKeys64(uint64_t k, uint64_t K_all[Nr64+1]) {
    uint64_t x;
    for (int i = 0; i < Nr64+1; i++) {
        uint64_t tmp = 0, tmp2 = 0;
        tmp |= k >> (4*i);
        x = (uint64_t)(pow(2, 4*i))-1;
        tmp2 |= k & x;

        //tmp2 <<= 4 * (16 - i);
        tmp2 = tmp2 * pow(2,4*(16-i));
        tmp2 |= tmp;

        K_all[i] = tmp2;

    }

}

uint64_t spn_encryption64(uint64_t x, uint64_t *K_all) {
    uint64_t w = x, u, v;

    for (int r = 1; r < Nr64; r++) {
        u = w ^ K_all[r-1];
        v = sTrans64(u, s_m);
        w = pTrans64(v, p_m);

        printf("w=%" PRIx64 "\n", w);
        printf("K=%" PRIx64 "\n", K_all[r-1]);
        printf("u=%" PRIx64 "\n", u);
        printf("v=%" PRIx64 "\n", v);


    }
    u = w ^ K_all[Nr64-1];
    v = sTrans64(u, s_m);
    return  v ^ K_all[Nr64];
}

uint64_t spn_decryption64(uint64_t y, uint64_t *K_all) {
    uint64_t w, u, v;
    v = y ^ K_all[Nr64];
    u = sTrans64(v, inv_s_m);
    w = u ^ K_all[Nr64-1];
    for (int r = Nr64-1; r > 0; r--) {
        v = pTrans64(w, inv_p_m);
        u = sTrans64(v, inv_s_m);
        w = u ^ K_all[r-1];
    }
    return w;
}


int main () {
    // init inv_p_m
    for (int i = 0; i < 64; i++) {
        inv_p_m[p_m[i]] = i;
    }

    uint64_t K0 = 0x3A94D63F3A94D63F;
    uint64_t K_all[Nr64+1];
    geneRoundKeys64(K0, K_all);
    uint64_t x = 0x12FFFF0000000000, y;
    y = spn_encryption64(x, K_all);
    printf("%" PRIx64 "\n", spn_decryption64(y, K_all));

}