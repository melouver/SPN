//
// Created by yuzheli on 9/12/17.
//
#include <stdio.h>
#include <stdarg.h>
#include <gmp.h>
#include <ctime>
const unsigned int two_1024 = 1 << 10;

void parameter_generator(mpz_t n, mpz_t p, mpz_t q, mpz_t a, mpz_t b) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    mpz_t init, phi, res;
    mpz_inits(p, q, init, phi, n, b, res, a, NULL);
    mpz_set_ui(init, 100);
    mpz_urandomm(p, state, init);
    mpz_urandomm(q, state, init);

    mpz_add_ui(p, p, two_1024);
    mpz_add_ui(q, q, two_1024);

    mpz_nextprime(p, p);
    mpz_nextprime(q, q);

    mpz_mul(n, p, q);

    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);

    mpz_mul(phi, p, q);

    mpz_add_ui(p, p, 1);
    mpz_add_ui(q, q, 1);

    do {
        mpz_urandomm(b, state, phi);
        mpz_gcd(res, b, phi);
    }while (mpz_cmp_ui(res, 1) != 0);

    mpz_invert(a, b, phi);

    mpz_clears(init, phi, res, NULL);
}


int main() {
    mpz_t n, p, q, a, b;
    mpz_inits(n,p,q,a,b,NULL);
    parameter_generator(n, p, q, a, b);

    gmp_printf("public key: %Zd %Zd \t private key: %Zd %Zd %Zd\n", n, b, p, q, a);

    mpz_t X[5], Y[5];

    for (int i = 0; i < 5; i++) {
        mpz_init_set_si(X[i], i);
        mpz_init(Y[i]);
    }

    /* naive encrypt and decrypt
    for (int i = 0; i < 5; i++) {
        mpz_powm(Y[i], X[i], b, n);
        gmp_printf("%Zd ", Y[i]);
    }
    printf("\n");

    for (int i = 0; i < 5; i++) {
        mpz_powm(X[i], Y[i], a, n);
        gmp_printf("%Zd ", X[i]);
    }
    */














}