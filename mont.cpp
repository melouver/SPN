// ConsoleApplication1.cpp : Defines the entry point for the console application.
//


#include <time.h>
#include "gmp.h"

//测试次数
#define TESTCOUNT 500
//模的位数
#define TESTBITS 1024
//GMP中，每个单精度整数占用多少比特
#define MONTBITS (8*sizeof(mp_limb_t))
//假设蒙哥马利中的T=XY的最大位数不超过MONT_MAX个整数
#define MONT_MAX 64
//随机数状态
gmp_randstate_t state;

void MontMulti(mpz_t T, const mpz_t x, const mpz_t y, const  mpz_t N, const mp_limb_t N_1)
{
	/*
	功能：计算x和y的蒙哥马利乘积，结果保存在T中，其中 0<=x，y<N [ 输入x y ，给出x * y * r^-1的值 ]
	N：模
	N_1:满足N*N_1=-1(mod 2^32)的整数
	*/

	unsigned int i;
	mp_limb_t num, carry, res[MONT_MAX] = { 0 };
	mp_limb_t *temp,t[MONT_MAX] = { 0 };

	//计算x和y的乘积，保存在t中，这里假设x和y均为蒙哥马利数
	if (x->_mp_size > y->_mp_size)
	   mpn_mul(t, x->_mp_d, x->_mp_size, y->_mp_d, y->_mp_size);
	else
	   mpn_mul(t, y->_mp_d, y->_mp_size, x->_mp_d, x->_mp_size);

	temp = t;
	for (i = 0; i < N->_mp_size; i++){
 		num = temp[0]*N_1;//num=t[i]*N_1
		res[i]=mpn_addmul_1(temp, N->_mp_d,N->_mp_size,num);//t=t+N*num,但是加法只做了N->_mp_size次，超出N->_mp_size长度的保存在res[i]中
		temp++;//相当于整除2^32
	}
	
	carry = mpn_add_n(temp, temp, res, N->_mp_size);//将上面步骤中所有没有处理的进位res[i]一次性地加到t上
	if (carry != 0 || mpn_cmp(temp, N->_mp_d, N->_mp_size) >= 0)//判断是否需要-N
		mpn_sub_n(temp, temp, N->_mp_d, N->_mp_size);

	mpz_import(T, N->_mp_size, -1, sizeof(mp_limb_t), 0, 0,temp);//将得到的结果保存在T中
}

void Mont_Exp(mpz_t rop, const mpz_t base, const mpz_t exp, const mpz_t N)
{
	/*
	功能：利用蒙哥马利模乘，计算base^exp(mod N)，结果保存在rop中
	*/
	mp_limb_t N_1;
	mpz_t K, P, R, temp, N_inv, b;
	mp_bitcnt_t index;
	unsigned int bitnum = mpz_sizeinbase(exp, 2);
	unsigned int rbit = N->_mp_size*MONTBITS;//蒙哥马利中r的选择是和模的位数相关的，r=2^rbit。此处并不要求N一定是1024比特，1023，1022等，得到的rbit值都是一样的
	mpz_inits(K, P, R, temp, N_inv, b, NULL);

	mpz_setbit(b, MONTBITS);	//b = 2^32
	mpz_invert(N_inv, N, b);//
	mpz_sub(N_inv, b, N_inv);
	N_1 = *(N_inv->_mp_d);	//N*N_1 =-1 (mod 2^32) [ r = 2^32 ]

	mpz_set_ui(temp, 1);
	mpz_setbit(K, 2 * rbit);//K=r^2
	mpz_mod(K, K, N);//保证0<=K<N

	MontMulti(P, K, base, N, N_1);//将base变成蒙哥马利数，P=K*base*r^-1=r^2*base*r^-1=base*r(mod N)，所以大家直接计算base<<rbit(modN)会更快一点
	MontMulti(R, K, temp, N, N_1);//将1变成蒙哥马利数，R=r^2*1*r^-1=r(mod N),所以大家直接计算1<<rbit(modN)会更快一点

	for (index = 0; index < bitnum; index++) {
		if (mpz_tstbit(exp, index) == 1)
			MontMulti(R, R, P, N, N_1);
		MontMulti(P, P, P, N, N_1);
	}

	MontMulti(rop, temp, R, N, N_1);//将R还原成普通整数，rop=1*R*r^-1=base^exp(mod N)
	
	//test code,将结果和库函数进行对比
	/*mpz_powm(temp, base, exp, N);
	if (mpz_cmp(temp, rop) != 0) {
		gmp_printf("r1 = %Zx\n", rop);
		gmp_printf("r2 = %Zx\n", temp);
		gmp_printf("mod = %Zx\n", N);
		gmp_printf("base = %Zx\n", base);
		gmp_printf("exp = %Zx\n", exp);
		getchar();
	}
	*/
	
	//test code
	mpz_clears(K, P, R, temp, N_inv, b, NULL);
}


void Mont_Test()
{
	gmp_randinit_lc_2exp_size(state, 128);	//设置随机数状态state
	gmp_randseed_ui(state, (unsigned long)time(NULL));
	int i;
	time_t t1, t2;
	mpz_t base, exp, mod, r1, r2;
	mpz_inits(base, exp, mod, r1, r2, NULL);

	mpz_rrandomb(exp, state, TESTBITS);
	mpz_rrandomb(mod, state, TESTBITS);
	mpz_rrandomb(base, state, TESTBITS);
	mpz_setbit(mod, 0);	//设置mod为奇数
	mpz_mod(base, base, mod);


	t1 = clock();
	for (i = 0; i < TESTCOUNT; i++) {
		Mont_Exp(r1, base, exp, mod);
	}
	t2 = clock();
	printf("蒙哥马利算法用时 : \t%lf\n", (double)(t2 - t1) / TESTCOUNT);


	mpz_clears(base, exp, mod, r1, r2, NULL);
}



int main(int argc,char* argv[])
{
	Mont_Test();

	return 0;
}
