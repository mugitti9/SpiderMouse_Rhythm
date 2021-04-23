#include "main.h"


/*指定されたポインタの4バイトの整数を得る*/
int getint(const unsigned char* data) {
	return (int)data[0]     | (int)data[1]<<8 |
	       (int)data[2]<<16 | (int)data[3]<<24;
}

/*指定されたポインタの2バイトの整数を得る*/
short getshort(const unsigned char* data) {
	return (short)data[0]     | (short)data[1]<<8;
}

/*wavファイルのデータ数を得る*/
int getwavsamplenum(const unsigned char* wav) {
	int pos;
	int max;
	int datasize=0;
	int bps=8;
	max=getint(&wav[4])+8;
	pos=0x0C;
	while(pos<max) {
		if(wav[pos  ]=='f' && wav[pos+1]=='m' &&
		   wav[pos+2]=='t' && wav[pos+3]==' ') {
			/*fmtチャンク*/
			bps=getshort(&wav[pos+20]);
		} else if(wav[pos  ]=='d' && wav[pos+1]=='a' &&
		          wav[pos+2]=='t' && wav[pos+3]=='a') {
			/*dataチャンク*/
			datasize=getint(&wav[pos+4]);
		}
		pos+=getint(&wav[pos+4])+8;
	}
	return datasize/bps;
}

/*wavファイルの1秒あたりのサンプル数を得る*/
int getwavsamplepersecond(const unsigned char* wav) {
	int pos;
	int max;
	max=getint(&wav[4])+8;
	pos=0x0C;
	while(pos<max) {
		if(wav[pos  ]=='f' && wav[pos+1]=='m' &&
		   wav[pos+2]=='t' && wav[pos+3]==' ') {
			/*fmtチャンク*/
			return getint(&wav[pos+12]);
		}
		pos+=getint(&wav[pos+4])+8;
	}
	return 1;
}

/*wavファイルのデータを得る*/
void getwavdata(int* data,const unsigned char* wav,int num,int smax) {
	int pos;
	int max;
	int datasize=0;
	unsigned char* datastart=0;
	int datastartpos;
	int channel=1;
	int block=1;
	int bps=8;
	int i;
	max=getint(&wav[4])+8;
	pos=0x0C;
	/*ヘッダ解析*/
	while(pos<max) {
		if(wav[pos  ]=='f' && wav[pos+1]=='m' &&
		   wav[pos+2]=='t' && wav[pos+3]==' ') {
			/*fmtチャンク*/
			channel=getshort(&wav[pos+10]);
			block=getshort(&wav[pos+22])/8;
			bps=getshort(&wav[pos+20]);
		} else if(wav[pos  ]=='d' && wav[pos+1]=='a' &&
		          wav[pos+2]=='t' && wav[pos+3]=='a') {
			/*dataチャンク*/
			datasize=getshort(&wav[pos+4]);
			datastart=(unsigned char*)&wav[pos+8];
			datastartpos=pos+8;
		}
		pos+=getint(&wav[pos+4])+8;
	}
	/*データ取得*/
	for(i=0;i<num;i++) {
		if(datastartpos+i*bps>max)break;
		if(channel==1) {
			if(block==1) {
				data[i]=(datastart[i]-128)*smax/128;
			} else {
				data[i]=(int)getshort(&datastart[i*2])*smax/32768;
			}
		} else {
			if(block==1) {
				data[i]=((datastart[i*2]-128)+(datastart[i*2+1]-128))/2*smax/128;
			} else {
				data[i]=((int)getshort(&datastart[i*4])+(int)getshort(&datastart[i*4+2]))/2*smax/32768;
			}
		}
	}
}
