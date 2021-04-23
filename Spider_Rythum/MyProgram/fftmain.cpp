#include "main.h"
#include <stdlib.h>

//解析する信号の長さ(秒)
#define SIGNAL_LENGTH 0.1
#define SIGNAL_MSEC SIGNAL_LENGTH*1000
//スペクトラムの数
#define SPECTRUM_NUM 50
//スペクトラムの拡大幅
#define SPECTRUM_WIDTH 10
//予測されるスペクトラムの最大の大きさ÷2[Hz]
#define SPECTRUM_EXCEPT_MAX   2500
//スペクトラムをcharからintにするのに必要な文字数
#define numberofspectrum SPECTRUM_NUM*2*6+1
//キーボードの入力状態を出力
char Key[256];

//コンピュータでフーリエ変換
int FFTMode() {

	int pushed_f1 = 0;	//f1の現在状態
	int pushed_f4 = 0;	//f4の現在状態
	int space = 0;		//spaceの入出力
	int sound = -1;		//音楽ファイルのメモリを格納
	int pause = 0;		//現在曲を動かしているか
	int error = 0;

	int* wavdata = NULL;	//wavのデータの保存場所
	int wavdatamax = 0;	//音楽のビット数を格納
	int samplepersec = 0;	//音楽のサンプリング周波数[Hz]
	int usedata_len = 0;	//サンプル毎のサンプル数の差
	int signal[SPECTRUM_NUM * 2] = { 0 };	//信号の値を格納
	unsigned short int spectrum[SPECTRUM_NUM * 2 + 1] = { 0 };	//FFTの結果を格納
	unsigned short int spectrum_max = NULL;				//その時間のFFTの結果の最大値
	unsigned short int allspectrum_max = 0;		//全時間におけるFFTの結果の最大値
	int totalpos = 0;

	int white = GetColor(255, 255, 255);			//白のRGB
	int green = GetColor(0, 255, 0);				//緑のRGB

	int avoidloop = 0;		//2回動作が行われるのを防ぐ(フラグ)
	int alreadymath = 0;	//FFT変換が終わっているかを確認(フラグ)
	int nowpos = 0;			//現在再生(計算)しているsoundのメモリの場所を格納

	int nowmusic = 0;			//現在の音楽の再生状況(フラグ)
	int nowwantmem = 0;		//いくつ分メモリを移動させる必要があるか
	int nowwantsa = 0;		//現在の時間-前サンプルを取った時間
	int alreadymem = 0;		//今までいくつ分のメモリを読み取ったか
	int notreturn = 0;		//２回以上繰り返されないように
	int max = SPECTRUM_NUM * 2;	//予測されるスペクトラムの最大の大きさ
	double ample = 1;				//曲の倍率
	float avg = 0;				//pastavgが0の時のモータの出力値(spectrumの総和/spectrumの総和の最大値)
	float pastavg = 0;			//前回のモータへの出力値
	int souwa = 0;				//spectrumの総和
	int souwa_max = 0;			//spectrumの総和の最大値

	FILE* spectrumvalue = NULL;		//spectrum.txtの入れる為の定義
	FILE* spectrumcheck = NULL;		//spectrumcheck.txtを入れるための定義

	char out[numberofspectrum] = { NULL };	//.txtからのspectrumの出力の仮保存先
	int avoidsecond = 0;
	int howmany = 0;

	int pastmem = 0;
	int before = 0;
	char number[10] = "1234abc";
	char pre[10] = { NULL };

	remove("spectrum.txt");
	remove("spectrumcheck.txt");

	while (!ProcessMessage() && !ClearDrawScreen() && !GetHitKeyStateAll(Key) && !Key[KEY_INPUT_ESCAPE]) {
		//↑ﾒｯｾｰｼﾞ処理         ↑画面をｸﾘｱ          ↑ｷｰﾎﾞｰﾄﾞ入力状態取得       ↑ESCが押されていない

		//ココに処理を書いていく

		//音声の読み込み 
		if (Key[KEY_INPUT_F1]) {
			if (!pushed_f1) {

				OPENFILENAME ofn = { 0 };
				char filename[1000] = { 0 };
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = GetMainWindowHandle();
				ofn.lpstrFilter = "wavファイル(*.wav)\0*.wav\0\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = filename;
				ofn.nMaxFile = sizeof(filename);
				ofn.lpstrTitle = "再生するwavファイルを選択してください";
				ofn.Flags = OFN_CREATEPROMPT | OFN_FILEMUSTEXIST |
					OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
				if (GetOpenFileName(&ofn)) {
					unsigned char* wavfiledata;
					int fp;
					//ファイル読み込み
					int filesize = FileRead_size(filename);
					if (filesize == -1) {
						MessageBox(GetMainWindowHandle(),
							"ファイルサイズ取得に失敗しました。", "エラー",
							MB_OK | MB_ICONWARNING);
						goto wavopenexit;
					}
					fp = FileRead_open(filename);
					if (fp == 0) {
						MessageBox(GetMainWindowHandle(),
							"ファイルオープンに失敗しました。", "エラー",
							MB_OK | MB_ICONWARNING);
						goto wavopenexit;
					}
					wavfiledata = (unsigned char*)HeapAlloc(GetProcessHeap(), 0, filesize);
					if (wavfiledata == NULL) {
						MessageBox(GetMainWindowHandle(),
							"メモリ確保に失敗しました。", "エラー",
							MB_OK | MB_ICONWARNING);
						FileRead_close(fp);
						goto wavopenexit;
					}
					if (FileRead_read(wavfiledata, filesize, fp) == -1) {
						MessageBox(GetMainWindowHandle(),
							"ファイルロードに失敗しました。", "エラー",
							MB_OK | MB_ICONWARNING);
						FileRead_close(fp);
						HeapFree(GetProcessHeap(), 0, wavfiledata);
						goto wavopenexit;
					}
					FileRead_close(fp);
					//波形データの読み出し
					wavdatamax = getwavsamplenum(wavfiledata);
					samplepersec = getwavsamplepersecond(wavfiledata);
					if (wavdata != NULL) {
						HeapFree(GetProcessHeap(), 0, wavdata);
						wavdata = NULL;
					}
					wavdata = (int*)HeapAlloc(GetProcessHeap(), 0, wavdatamax * sizeof(int));
					if (wavdata == NULL) {
						MessageBox(GetMainWindowHandle(),
							"メモリ確保に失敗しました。", "エラー",
							MB_OK | MB_ICONWARNING);
						FileRead_close(fp);
						HeapFree(GetProcessHeap(), 0, wavfiledata);
						goto wavopenexit;
					}
					getwavdata(wavdata, wavfiledata, wavdatamax, 1000);
					//wavファイルデータの開放
					HeapFree(GetProcessHeap(), 0, wavfiledata);
					//データ情報の設定
					usedata_len = (int)(samplepersec * SIGNAL_LENGTH);
					//サウンドデータの読み込み
					if (sound != -1) {
						StopSoundMem(sound);
						DeleteSoundMem(sound);
						sound = -1;
					}
					sound = LoadSoundMem(filename);
					if (sound != -1) {
						avoidloop = 0;
						alreadymath = 0;
						pause = 0;
						nowpos = 0;
						nowmusic = 0;
						avoidsecond = 0;
						nowwantsa = 0;
						nowwantmem = 0;
						pastmem = 0;
						allspectrum_max = 1;
						alreadymem = 0;
						notreturn = 0;
						ample = 1;
						avg = 0;
						souwa = 0;
						souwa_max = 0;
						space = 0;
						pastavg = 0;
						if (before == 1) {
							fclose(spectrumvalue);
							fclose(spectrumcheck);
							remove("spectrum.txt");
							remove("spectrumcheck.txt");
						}
					}
					//終了 
				wavopenexit:
					;
				}
			}
			pushed_f1 = 1;
		}
		else pushed_f1 = 0;

		//スペクトラムと情報の描画 
		if (sound != -1 && avoidloop != 1 && alreadymath != 1) {
			if (nowpos + usedata_len <= wavdatamax) {
				fopen_s(&spectrumvalue, "spectrum.txt", "a+");
				int i;
				for (i = 0; i < max; i++) {
					signal[i] = wavdata[nowpos + usedata_len * i / max];
				}
				//スペクトラムの計算 
				calcSpectrum(signal, max, spectrum); //rfh.cppにある
				//スペクトラムの最大値の計算 
				spectrum_max = 1;
				for (i = 0; i < max; i++) {
					if (spectrum[i] > spectrum_max)spectrum_max = spectrum[i];	//現在の時間の最大値
					souwa_max = souwa_max + spectrum[i] * (SPECTRUM_NUM * 2 + 1 - i);
				}
				if (allspectrum_max < souwa_max) {
					allspectrum_max = souwa_max;		//全時間の最大値
				}
				int j;
				for (i = 0; i < max; i++) {
					fprintf(spectrumvalue, "%u ", spectrum[i]);
				}
				fprintf(spectrumvalue, "\n");
				nowpos = nowpos + usedata_len;
				fclose(spectrumvalue);
			}
			else if (nowpos + usedata_len > wavdatamax) {
				avoidloop = 1;
				alreadymath = 1;
			}

			DrawBox(640 - (int)(640 * spectrum_max / SPECTRUM_EXCEPT_MAX),
				0, 640, 25, green, TRUE);
			int nowcs = nowpos * 100 / samplepersec;
			DrawFormatString(400, 10, white, "読み込み位置：%3d分%2d.%02d秒",
				nowcs / 100 / 60, (nowcs / 100) % 60, nowcs % 100);
			DrawFormatString(140, 10, white, "最大値：%u", spectrum_max);

		}

		if ((alreadymath == 1 && Key[KEY_INPUT_SPACE]) || space == 1) {

			if (avoidsecond == 0) {
				fopen_s(&spectrumvalue, "spectrum.txt", "r");
				fopen_s(&spectrumcheck, "spectrumcheck.txt", "w");
				avoidsecond = 1;
				before = 1;
				for (int i = 0; i < 3; i++) {
					fgets(out, numberofspectrum, spectrumvalue);
				}
			}

			if (nowmusic != 1) {
				PlaySoundMem(sound, DX_PLAYTYPE_BACK, FALSE);
				SetMousePoint(0, 0);
				nowmusic = 1;
				spectrum_max = 1;
				space = 1;
			}
			if (GetSoundTotalSample(sound)<(GetCurrentPositionSoundMem(sound)+10000)) {
				goto last;
			}

			if (Key[KEY_INPUT_F4]) {
				if (!pushed_f4) {
					if (sound != -1) {
						if (pause) {
							PlaySoundMem(sound, DX_PLAYTYPE_BACK, FALSE);
							pause = 0;
						}
						else {
							StopSoundMem(sound);
							pause = 1;
						}
					}
				}
				pushed_f4 = 1;
			}
			else {
				pushed_f4 = 0;
			}

			ample = changespeed(samplepersec, sound);
			double motorsec = (nowwantmem * SIGNAL_MSEC) / ample;

			nowpos = GetCurrentPositionSoundMem(sound);
			nowwantsa = nowpos - pastmem;
			nowwantmem = nowwantsa / usedata_len;

			if (samplepersec == 1) {
				goto last;
			}

			if (nowwantsa >= usedata_len) {
				avg = 0;
				spectrum_max = 1;
				for (int j = 0; j < 10; j++) {
					pre[j] = ' ';
				}
				for (int k = 0; k < SPECTRUM_NUM * 2 + 1; k++) {
					spectrum[k] = 0;
				}
				for (int i = 0; i < numberofspectrum; i++) {
					out[i] = ' ';
				}
				for (int i = 0; i < nowwantmem; i = i + 1) {
					fgets(out, numberofspectrum, spectrumvalue);
				}
				alreadymem = alreadymem + nowwantmem;
				nowwantmem = 0;

				int j = 0;
				int k = 0;
				int max = SPECTRUM_NUM * 2;
				for (int i = 0; out[i] != '\0'; i++) {
					if (out[i] == ' ') {
						pre[j] = '\0';
						spectrum[k++] = atoi(pre);
						j = 0;
					}
					pre[j++] = out[i];
				}

				spectrum[k++] = atoi(pre);

				for (int i = 0; i < k; i++) {
					fprintf(spectrumcheck, "%u ", spectrum[i]);
				}
				fprintf(spectrumcheck, "\n");

				for (int i = 0; i < max; i++) {
					if (spectrum[i] > spectrum_max)spectrum_max = spectrum[i];
				}
				for (int i = 0; i < max; i++) {
					avg = avg + spectrum[i] * (SPECTRUM_NUM * 2 + 1 - i);
				}

				avg = avg / allspectrum_max;

				if (pastavg < avg) {
					SetDutyOnCh(avg - pastavg, 0, avg - pastavg, 0, motorsec);	//前の平均値の方が大きい時のモータ出力の値を設定
				}
				else if (pastavg >= avg) {
					SetDutyOnCh(0, pastavg - avg, 0, pastavg - avg, motorsec);	//今の平均値の方が大きい時のモータ出力値の値を設定
				}
				pastavg = avg;
				pastmem = usedata_len * alreadymem;
			}

			if (CheckSoundMem(sound) == 0 && pause == 0) {
				goto last;
			}

			int i, s;
			for (i = 0; i < SPECTRUM_NUM; i++) {
				for (s = 0; s < SPECTRUM_WIDTH; s++) {
					DrawLine(20 + i * SPECTRUM_WIDTH + s, 450,
						20 + i * SPECTRUM_WIDTH + s,
						450 - (int)(spectrum[i] * 420 / allspectrum_max), green);
				}
			}
			DrawBox(640 - (int)(640 * spectrum_max / SPECTRUM_EXCEPT_MAX),
				0, 640, 25, green, TRUE);

			if (samplepersec == 0) {
				samplepersec = 1;
				error = 1;
			}

			int nowcs = nowpos * 100 / samplepersec;
			DrawFormatString(400, 10, white, "読み込み位置：%3d分%2d.%02d秒",
				nowcs / 100 / 60, (nowcs / 100) % 60, nowcs % 100);
			if (nowmusic == 1) {
				DrawFormatString(140, 10, white, "最大値：%u", spectrum_max);
			}
		}else if (alreadymath == 1) {
			DrawString(560, 115, "SPACEで", white);
			DrawString(560, 135, "再生", white);
		}

		//説明の描画
		DrawLine(0, 30, 640, 30, white);
		DrawLine(0, 450, 640, 450, white);
		DrawLine(552, 30, 552, 450, white);
		DrawString(10, 460, "F1で音声読み込み", white);
		DrawString(300, 460, "", white);
		DrawString(560, 50, "F4で", white);
		DrawString(560, 80, pause ? "再開" : "ポーズ", white);
		DrawString(560, 170, "現在速度", white);
		DrawFormatString(560, 200, white, "%lf", ample);

		ScreenFlip();//裏画面を表画面に反映
	}

last:
	if (alreadymath == 1) {
		if (spectrumvalue != NULL) {
			fclose(spectrumvalue);
		}
		if (spectrumcheck != NULL) {
			fclose(spectrumcheck);
		}
	}
	return sound;
}