#include "main.h"

/*
http://www.kurims.kyoto-u.ac.jp/~ooura/fftman/ftmn1_2.html
リスト1.2.1-3. 基数 2 の周波数間引き FFT (修正版)
*/

void fft(int n, double theta, double ar[], double ai[])
{
	int m, mh, i, j, k, irev;
	double wr, wi, xr, xi;

	/* ---- scrambler ---- */
	i = 0;
	for (j = 1; j < n - 1; j++) {
		for (k = n >> 1; k > (i ^= k); k >>= 1);
		if (j < i) {
			xr = ar[j];
			xi = ai[j];
			ar[j] = ar[i];
			ai[j] = ai[i];
			ar[i] = xr;
			ai[i] = xi;
		}
	}
	for (mh = 1; (m = mh << 1) <= n; mh = m) {
		irev = 0;
		for (i = 0; i < n; i += m) {
			wr = cos(theta * irev);
			wi = sin(theta * irev);
			for (k = n >> 2; k > (irev ^= k); k >>= 1);
			for (j = i; j < mh + i; j++) {
				k = j + mh;
				xr = ar[j] - ar[k];
				xi = ai[j] - ai[k];
				ar[j] += ar[k];
				ai[j] += ai[k];
				ar[k] = wr * xr - wi * xi;
				ai[k] = wr * xi + wi * xr;
			}
		}
	}
}

/*nが2のn乗であるかを判定*/
static int n_check(int n) {
	if(n==0)return 1;
	while(n%2==0)n/=2;
	return n==1?1:0;
}

/*この関数を使用する*/
void calcSpectrum_fft(const int *signal, int length, long long *spectrum) {
	double* x;
	double* y;
	int i;
	/*不正*/
	if(!n_check(length)) {
		for(i=0;i<length;i++)spectrum[i]=0;
		return;
	}
	x=(double*)HeapAlloc(GetProcessHeap(),0,length*sizeof(double));
	y=(double*)HeapAlloc(GetProcessHeap(),0,length*sizeof(double));
	if(x==NULL || y==NULL) {
		if(x)HeapFree(GetProcessHeap(),0,x);
		if(y)HeapFree(GetProcessHeap(),0,y);
		return;
	}
	for(i=0;i<length;i++) {
		x[i]=signal[i];
		y[i]=0;
	}
	fft(length,2*DX_PI/length,x,y);
	for(i=0;i<length;i++) {
		spectrum[i] = (long long)(x[i] * x[i] + y[i] * y[i]);
	}
	HeapFree(GetProcessHeap(),0,x);
	HeapFree(GetProcessHeap(),0,y);
}
