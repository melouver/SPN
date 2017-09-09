//
// Created by 李玉哲 on 09/09/2017.
//

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>


typedef unsigned short (*pBoxTrans)(unsigned short);
typedef unsigned short (*sBoxTrans)(unsigned short, unsigned[]);

const int Nr = 4;
const int l = 4;
const int m = 4;
const int pairs_cnt = 8000;
const int da_pairs_cnt = 80;
char input_or = 0xB;

unsigned s_m[] = {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7};
unsigned p_m[] = {1,5,9,13,2,6,10,14,3,7,11,15,4,8,12,16};
unsigned inv_s_m[] = {14,3,4,8,1,12,10,15,7,13,9,6,11,2,0,5};

unsigned short sTrans(unsigned short u, unsigned s[]) {
    unsigned short res = 0;
    for (int i = 0; i < 4; i++) {
        res |= (s[(u>>(i*4))&0xF]) << (i*4);
    }
    return res;
}

unsigned short pTrans(unsigned short v) {
    unsigned short res = 0;
    for (int i = 1; i <= 16; i++) {
        res |= ((v>>(16-p_m[i-1]))&0x1) << (16-i);
    }
    return res;
}

void geneRoundKeys(uint32_t k, unsigned short K_all[Nr+1]) {
    for (int i = 0; i < Nr + 1; i++) {
        K_all[i] = (k >> ((4 - i) * 4)) & 0xFFFF;
    }
}

unsigned  short spn_encryption(unsigned short x, sBoxTrans pi_s, pBoxTrans pi_p, unsigned short *K_all) {
    unsigned short w = x, u, v;

    for (int r = 1; r < Nr; r++) {
        u = w ^ K_all[r-1];
        v = pi_s(u, s_m);
        w = pi_p(v);
    }
    u = w ^ K_all[Nr-1];
    v = pi_s(u, s_m);
    return  v ^ K_all[Nr];
}

void linear_attack(unsigned short X[pairs_cnt], unsigned short Y[pairs_cnt]) {
    int count[16][16] = {0};

    char v4 = 0, u4 = 0;
    unsigned short x, y;

    for (int cnt = 0; cnt < pairs_cnt; cnt++) {
        x = X[cnt];
        y = Y[cnt];

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {

                v4 = u4 = 0;

                v4 |= ((i ^ (y>>8))&0xF)<<4;
                v4 |= (j ^ y)&0xF;

                u4 |= inv_s_m[v4>>4&0xF]<<4;
                u4 |= inv_s_m[v4&0xF];

                char z = ((x>>11) ^ (x>>9) ^ (x>>8) ^ (u4>>6) ^ (u4>>4) ^ (u4>>2)^ u4)&0x1;
                if (z == 0) {
                    count[i][j]++;
                }
            }
        }
    }

    int max = 0;
    char k1 = 0, k2 = 0;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            count[i][j] = abs(count[i][j]-pairs_cnt/2);
            if (count[i][j] > max) {
                max = count[i][j];
                k1 = i;
                k2 = j;
            }
        }
    }

    x = X[0];
    y = Y[0];
    unsigned short test = 0;

    int CNT = 0;
    for (int i = 0; i < 0xFFFFFF; i++) {
        unsigned short roundKey[Nr+1] = {0};

        uint32_t K = 0;
        K |= k1 << 8;
        K |= k2;
        for (int j = 0; j < 5; j++) {
            K |= (i >> 4*(5-j)) << 4*(7-j);
        }

        K |= (i&0xF)<<4;

        geneRoundKeys(K, roundKey);test = spn_encryption(x, sTrans, pTrans, roundKey);

        if (test == y) {
            // Got the 32bit Key needed
        }
    }
}

void differential_cryptanalysis(unsigned short X[], unsigned short X_astrk[], unsigned short Y[], unsigned short Y_astrk[], int T) {
    int count[16][16] = {0};
    unsigned short x, x_ast, y, y_ast;
    char v4, u4, v4_ast, u4_ast, u4_prm;

    for (int i = 0; i < T; i++) {
        x = X[i]; x_ast = X_astrk[i]; y = Y[i]; y_ast = Y_astrk[i];

        if (((y >> 12) == (y_ast>>12)) &&
                (((y >> 4)&0xF) == ((y_ast>>4)&0xF))) {
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 16; j++) {
                    u4 = v4 = 0;

                    v4 |= ((i ^ (y>>8))&0xF)<<4;
                    v4 |= (j ^ y)&0xF;

                    u4 |= inv_s_m[v4>>4&0xF]<<4;
                    u4 |= inv_s_m[v4&0xF];

                    v4_ast = u4_ast = 0;

                    v4_ast |= ((i ^ (y>>8))&0xF)<<4;
                    v4_ast |= (j ^ y_ast)&0xF;

                    u4_ast |= inv_s_m[v4_ast>>4&0xF]<<4;
                    u4_ast |= inv_s_m[v4_ast&0xF];

                    u4_prm = 0;
                    u4_prm |= ((u4>>4) ^ (u4_ast>>4))<<4;
                    u4_prm |= (u4&0xF)^(u4_ast&0xF);

                    if (((u4_prm>>4) == 6) && ((u4_prm&0xF) == 6)) {
                        count[i][j]++;
                    }
                }
            }
        }
    }

    int max = -1;
    int k1 = 0, k2 = 0;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (count[i][j] > max) {
                max = count[i][j];
                k1 = i;
                k2 = j;
            }
        }
    }
    printf("%d, %d\n", k1, k2);
}

unsigned short LAX[pairs_cnt] = {0};
unsigned short LAY[pairs_cnt] = {0};

unsigned short DAX[da_pairs_cnt] = {0};
unsigned short DAY[da_pairs_cnt] = {0};
unsigned short DAXART[da_pairs_cnt] = {0};
unsigned short DAYART[da_pairs_cnt] = {0};

int main() {
#ifdef LA
    uint32_t K0 = 0x3A94D63F;
    unsigned short K_all[Nr+1];
    geneRoundKeys(K0, K_all);

    srand(time(NULL));
    for (int i = 0; i < pairs_cnt; i++) {
        LAX[i] = rand()%UINT16_MAX;
        LAY[i] = spn_encryption(LAX[i], sTrans, pTrans, K_all);
    }

    clock_t start, end;
    start = clock();
    linear_attack(LAX, LAY);
    end = clock();
    double during = (double)(end - start)/CLOCKS_PER_SEC;
    printf("%f\n", during);
#else

    uint32_t K0 = 0x3A94D63F;
    unsigned short K_all[Nr+1];
    geneRoundKeys(K0, K_all);

    srand(time(NULL));

    unsigned short x, x_ast;
    for (int i = 0; i < da_pairs_cnt; i++) {
        DAX[i] = x = rand()%UINT16_MAX;
        x_ast = 0;
        for (int j = 0; j < 4; j++) {
            x_ast |= (((x >> (4*(3-j)))&0xF) ^ input_or) << (4*(3-j));
        }
        DAXART[i] = x_ast;
        DAY[i]  = spn_encryption(x, sTrans, pTrans, K_all);
        DAYART[i] = sTrans(x_ast, s_m);
    }
    differential_cryptanalysis(DAX, DAXART, DAY, DAYART, da_pairs_cnt);

#endif

    return 0;
}

