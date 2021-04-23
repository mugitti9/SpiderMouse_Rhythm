#include "main.h"

/*—£Uƒt[ƒŠƒG•ÏŠ·*/
void calcSpectrum(const int *signal, int length, unsigned short int *spectrum) {
	int i,j;
	long long kari;

	int N = length;

	for (i = 0; i < N; i++) {

		double a = 0, b = 0;

		for (j = 0; j < N; j++) {
			int m = j * i;

			a += cos( 2 * DX_PI * m / N ) * signal[j];
			b += -sin( 2 * DX_PI * m / N ) * signal[j];
		}

		kari = (long long)(a * a + b * b);
		spectrum[i] = kari / 10000;

	}
}
