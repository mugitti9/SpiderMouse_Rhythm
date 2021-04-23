#include "main.h"
#include <stdlib.h>

//��͂���M���̒���(�b)
#define SIGNAL_LENGTH 0.1
#define SIGNAL_MSEC SIGNAL_LENGTH*1000
//�X�y�N�g�����̐�
#define SPECTRUM_NUM 50
//�X�y�N�g�����̊g�啝
#define SPECTRUM_WIDTH 10
//�\�������X�y�N�g�����̍ő�̑傫����2[Hz]
#define SPECTRUM_EXCEPT_MAX   2500
//�X�y�N�g������char����int�ɂ���̂ɕK�v�ȕ�����
#define numberofspectrum SPECTRUM_NUM*2*6+1
//�L�[�{�[�h�̓��͏�Ԃ��o��
char Key[256];

//�R���s���[�^�Ńt�[���G�ϊ�
int FFTMode() {

	int pushed_f1 = 0;	//f1�̌��ݏ��
	int pushed_f4 = 0;	//f4�̌��ݏ��
	int space = 0;		//space�̓��o��
	int sound = -1;		//���y�t�@�C���̃��������i�[
	int pause = 0;		//���݋Ȃ𓮂����Ă��邩
	int error = 0;

	int* wavdata = NULL;	//wav�̃f�[�^�̕ۑ��ꏊ
	int wavdatamax = 0;	//���y�̃r�b�g�����i�[
	int samplepersec = 0;	//���y�̃T���v�����O���g��[Hz]
	int usedata_len = 0;	//�T���v�����̃T���v�����̍�
	int signal[SPECTRUM_NUM * 2] = { 0 };	//�M���̒l���i�[
	unsigned short int spectrum[SPECTRUM_NUM * 2 + 1] = { 0 };	//FFT�̌��ʂ��i�[
	unsigned short int spectrum_max = NULL;				//���̎��Ԃ�FFT�̌��ʂ̍ő�l
	unsigned short int allspectrum_max = 0;		//�S���Ԃɂ�����FFT�̌��ʂ̍ő�l
	int totalpos = 0;

	int white = GetColor(255, 255, 255);			//����RGB
	int green = GetColor(0, 255, 0);				//�΂�RGB

	int avoidloop = 0;		//2�񓮍삪�s����̂�h��(�t���O)
	int alreadymath = 0;	//FFT�ϊ����I����Ă��邩���m�F(�t���O)
	int nowpos = 0;			//���ݍĐ�(�v�Z)���Ă���sound�̃������̏ꏊ���i�[

	int nowmusic = 0;			//���݂̉��y�̍Đ���(�t���O)
	int nowwantmem = 0;		//���������������ړ�������K�v�����邩
	int nowwantsa = 0;		//���݂̎���-�O�T���v�������������
	int alreadymem = 0;		//���܂ł������̃�������ǂݎ������
	int notreturn = 0;		//�Q��ȏ�J��Ԃ���Ȃ��悤��
	int max = SPECTRUM_NUM * 2;	//�\�������X�y�N�g�����̍ő�̑傫��
	double ample = 1;				//�Ȃ̔{��
	float avg = 0;				//pastavg��0�̎��̃��[�^�̏o�͒l(spectrum�̑��a/spectrum�̑��a�̍ő�l)
	float pastavg = 0;			//�O��̃��[�^�ւ̏o�͒l
	int souwa = 0;				//spectrum�̑��a
	int souwa_max = 0;			//spectrum�̑��a�̍ő�l

	FILE* spectrumvalue = NULL;		//spectrum.txt�̓����ׂ̒�`
	FILE* spectrumcheck = NULL;		//spectrumcheck.txt�����邽�߂̒�`

	char out[numberofspectrum] = { NULL };	//.txt�����spectrum�̏o�͂̉��ۑ���
	int avoidsecond = 0;
	int howmany = 0;

	int pastmem = 0;
	int before = 0;
	char number[10] = "1234abc";
	char pre[10] = { NULL };

	remove("spectrum.txt");
	remove("spectrumcheck.txt");

	while (!ProcessMessage() && !ClearDrawScreen() && !GetHitKeyStateAll(Key) && !Key[KEY_INPUT_ESCAPE]) {
		//��ү���ޏ���         ����ʂ�ر          �����ް�ޓ��͏�Ԏ擾       ��ESC��������Ă��Ȃ�

		//�R�R�ɏ����������Ă���

		//�����̓ǂݍ��� 
		if (Key[KEY_INPUT_F1]) {
			if (!pushed_f1) {

				OPENFILENAME ofn = { 0 };
				char filename[1000] = { 0 };
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = GetMainWindowHandle();
				ofn.lpstrFilter = "wav�t�@�C��(*.wav)\0*.wav\0\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = filename;
				ofn.nMaxFile = sizeof(filename);
				ofn.lpstrTitle = "�Đ�����wav�t�@�C����I�����Ă�������";
				ofn.Flags = OFN_CREATEPROMPT | OFN_FILEMUSTEXIST |
					OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
				if (GetOpenFileName(&ofn)) {
					unsigned char* wavfiledata;
					int fp;
					//�t�@�C���ǂݍ���
					int filesize = FileRead_size(filename);
					if (filesize == -1) {
						MessageBox(GetMainWindowHandle(),
							"�t�@�C���T�C�Y�擾�Ɏ��s���܂����B", "�G���[",
							MB_OK | MB_ICONWARNING);
						goto wavopenexit;
					}
					fp = FileRead_open(filename);
					if (fp == 0) {
						MessageBox(GetMainWindowHandle(),
							"�t�@�C���I�[�v���Ɏ��s���܂����B", "�G���[",
							MB_OK | MB_ICONWARNING);
						goto wavopenexit;
					}
					wavfiledata = (unsigned char*)HeapAlloc(GetProcessHeap(), 0, filesize);
					if (wavfiledata == NULL) {
						MessageBox(GetMainWindowHandle(),
							"�������m�ۂɎ��s���܂����B", "�G���[",
							MB_OK | MB_ICONWARNING);
						FileRead_close(fp);
						goto wavopenexit;
					}
					if (FileRead_read(wavfiledata, filesize, fp) == -1) {
						MessageBox(GetMainWindowHandle(),
							"�t�@�C�����[�h�Ɏ��s���܂����B", "�G���[",
							MB_OK | MB_ICONWARNING);
						FileRead_close(fp);
						HeapFree(GetProcessHeap(), 0, wavfiledata);
						goto wavopenexit;
					}
					FileRead_close(fp);
					//�g�`�f�[�^�̓ǂݏo��
					wavdatamax = getwavsamplenum(wavfiledata);
					samplepersec = getwavsamplepersecond(wavfiledata);
					if (wavdata != NULL) {
						HeapFree(GetProcessHeap(), 0, wavdata);
						wavdata = NULL;
					}
					wavdata = (int*)HeapAlloc(GetProcessHeap(), 0, wavdatamax * sizeof(int));
					if (wavdata == NULL) {
						MessageBox(GetMainWindowHandle(),
							"�������m�ۂɎ��s���܂����B", "�G���[",
							MB_OK | MB_ICONWARNING);
						FileRead_close(fp);
						HeapFree(GetProcessHeap(), 0, wavfiledata);
						goto wavopenexit;
					}
					getwavdata(wavdata, wavfiledata, wavdatamax, 1000);
					//wav�t�@�C���f�[�^�̊J��
					HeapFree(GetProcessHeap(), 0, wavfiledata);
					//�f�[�^���̐ݒ�
					usedata_len = (int)(samplepersec * SIGNAL_LENGTH);
					//�T�E���h�f�[�^�̓ǂݍ���
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
					//�I�� 
				wavopenexit:
					;
				}
			}
			pushed_f1 = 1;
		}
		else pushed_f1 = 0;

		//�X�y�N�g�����Ə��̕`�� 
		if (sound != -1 && avoidloop != 1 && alreadymath != 1) {
			if (nowpos + usedata_len <= wavdatamax) {
				fopen_s(&spectrumvalue, "spectrum.txt", "a+");
				int i;
				for (i = 0; i < max; i++) {
					signal[i] = wavdata[nowpos + usedata_len * i / max];
				}
				//�X�y�N�g�����̌v�Z 
				calcSpectrum(signal, max, spectrum); //rfh.cpp�ɂ���
				//�X�y�N�g�����̍ő�l�̌v�Z 
				spectrum_max = 1;
				for (i = 0; i < max; i++) {
					if (spectrum[i] > spectrum_max)spectrum_max = spectrum[i];	//���݂̎��Ԃ̍ő�l
					souwa_max = souwa_max + spectrum[i] * (SPECTRUM_NUM * 2 + 1 - i);
				}
				if (allspectrum_max < souwa_max) {
					allspectrum_max = souwa_max;		//�S���Ԃ̍ő�l
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
			DrawFormatString(400, 10, white, "�ǂݍ��݈ʒu�F%3d��%2d.%02d�b",
				nowcs / 100 / 60, (nowcs / 100) % 60, nowcs % 100);
			DrawFormatString(140, 10, white, "�ő�l�F%u", spectrum_max);

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
					SetDutyOnCh(avg - pastavg, 0, avg - pastavg, 0, motorsec);	//�O�̕��ϒl�̕����傫�����̃��[�^�o�͂̒l��ݒ�
				}
				else if (pastavg >= avg) {
					SetDutyOnCh(0, pastavg - avg, 0, pastavg - avg, motorsec);	//���̕��ϒl�̕����傫�����̃��[�^�o�͒l�̒l��ݒ�
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
			DrawFormatString(400, 10, white, "�ǂݍ��݈ʒu�F%3d��%2d.%02d�b",
				nowcs / 100 / 60, (nowcs / 100) % 60, nowcs % 100);
			if (nowmusic == 1) {
				DrawFormatString(140, 10, white, "�ő�l�F%u", spectrum_max);
			}
		}else if (alreadymath == 1) {
			DrawString(560, 115, "SPACE��", white);
			DrawString(560, 135, "�Đ�", white);
		}

		//�����̕`��
		DrawLine(0, 30, 640, 30, white);
		DrawLine(0, 450, 640, 450, white);
		DrawLine(552, 30, 552, 450, white);
		DrawString(10, 460, "F1�ŉ����ǂݍ���", white);
		DrawString(300, 460, "", white);
		DrawString(560, 50, "F4��", white);
		DrawString(560, 80, pause ? "�ĊJ" : "�|�[�Y", white);
		DrawString(560, 170, "���ݑ��x", white);
		DrawFormatString(560, 200, white, "%lf", ample);

		ScreenFlip();//����ʂ�\��ʂɔ��f
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