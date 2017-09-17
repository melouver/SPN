//
// Created by yuzheli on 9/12/17.
//
#include <stdio.h>
#include <stdarg.h>
#include <gmp.h>
#include <ctime>
#include "mont.h"



void parameter_generator(mpz_t n, mpz_t p, mpz_t q, mpz_t a, mpz_t b) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    mpz_t init, phi, res;
    mpz_inits(p, q, init, phi, n, b, res, a, NULL);

    mpz_urandomb(init, state, 1024);
    mpz_nextprime(p, init);
    mpz_urandomb(init, state, 1024);
    mpz_nextprime(q, init);

    mpz_mul(n, p, q);

    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);

    mpz_mul(phi, p, q);

    do {
        mpz_urandomm(b, state, phi);
        mpz_gcd(res, b, phi);
    }while (mpz_cmp_ui(res, 1) != 0);

    mpz_invert(a, b, phi);

    mpz_clears(init, phi, res, NULL);
}

#define KeyBits 1024
#define TSTCNT 500
/*
void Multiplicative_invert(mpz_t a, mpz_t b, mpz_t t) {
    mpz_t a0, b0, t0, t, q, r, tmp;
    mpz_inits(t,a,b,t0,q,r,tmp, NULL);

    mpz_set(a0, a);
    mpz_set(b0, b);
    mpz_set_ui(t0, 0);
    mpz_set_ui(t, 1);
    mpz_fdiv_q(q, a0, b0);
    mpz_submul(a0,q,b0);
    mpz_set(r,a0);

    while (mpz_cmp_ui(r, 0) > 0) {
        mpz_submul(t0, q, t);
        mpz_mod(tmp,t0,a);
        mpz_set(t0,t);
        mpz_set(t,tmp);
        mpz_set(a0,b0);
        mpz_set(b0,r);




    }
}
*/

void expoentionBysquaring(mpz_t rop, const mpz_t base, const mpz_t exp, const mpz_t N) {
    mp_bitcnt_t index;
    mpz_t R, P;
    mpz_inits(R, P, NULL);
    mpz_set_ui(R, 1);
    mpz_set(P, base);

    unsigned int bitnum = mpz_sizeinbase(exp, 2);
    for (index = 0; index < bitnum; index++) {
        if (mpz_tstbit(exp, index) == 1) {
            mpz_mul(R, R, P);
            mpz_mod(R, R, N);
        }
        mpz_mul(P, P, P);
        mpz_mod(P, P, N);
    }
    mpz_mod(R, R, N);
    mpz_set(rop, R);
}



int main() {
    mpz_t n, p, q, a, b;
    mpz_inits(n,p,q,a,b,NULL);
    parameter_generator(n, p, q, a, b);

    mpz_t x, y, dp, dq, qinv, h, m1, m2;
    mpz_inits(x, y, dp, dq, qinv, h, m1, m2, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    time_t t1, t2;

    mpz_mod(dp, a, p);
    mpz_mod(dq, a, q);
    mpz_add_ui(q, q, 1);
    mpz_add_ui(p, p, 1);
    mpz_invert(qinv, q, p);
    t1 = clock();
    for (int i = 0; i < TSTCNT; i++) {
        mpz_urandomb(x, state, KeyBits);
        gmp_printf("x=%Zx\n", x);
        Mont_Exp(y, x, b, n);
        //(y, x, b, n);
        //expoentionBysquaring(y,x,b,n);
        //gmp_printf("y=%Zx\n", y);

        //Mont_Exp(m1, y, dp, p);
        //Mont_Exp(m2, y, dq, q);
        mpz_powm(m1, y, dp, p);
        mpz_powm(m2, y, dq, q);

        mpz_sub(m1, m1, m2);
        mpz_mul(h, qinv, m1);
        mpz_mod(h, h, p);
        mpz_addmul(m2, h, q);
        //expoentionBysquaring(x,y,a,n);
        Mont_Exp(x,y,a,n);
        gmp_printf("x'=%Zx\n\n", x);
    }

    t2 = clock();

    mpz_clears(n,p,q,a,b,x,y,dp, dq, qinv, h, m1, m2, NULL);

    printf("encryption and decryption take %f per 200 times.", (double)(t2-t1)/(TESTCOUNT/200)/CLOCKS_PER_SEC);
}