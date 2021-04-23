#include "DxLib.h"
#include <math.h>
#include "SpidarMouse.h"
//#pragma comment(lib,"SpidarMouse.lib")


//getwavdata.cpp
int getwavsamplenum(const unsigned char* wav);
int getwavsamplepersecond(const unsigned char* wav);
void getwavdata(int* data,const unsigned char* wav,int num,int smax);

//rfh.cpp
void calcSpectrum(const int *signal, int length, unsigned short int *spectrum);

//fft.cpp
void calcSpectrum_fft(const int *signal, int length, long long *spectrum);
double changespeed(int samplepersec,int sound);