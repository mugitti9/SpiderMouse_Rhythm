#include "main.h"

double changespeed(int samplepersec,int sound){
	double ample;
	int NOW_POINT_X;
	int NOW_POINT_Y;

	GetMousePoint(&NOW_POINT_X,&NOW_POINT_Y);
	ample=(NOW_POINT_X/200)*0.1+1;

	SetFrequencySoundMem(ample*samplepersec,sound);

	return ample;
}