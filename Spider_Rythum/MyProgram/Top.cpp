#include <stdio.h>
#include <math.h>
#include "DxLib.h"
#include "HitKey.h"
#include"SpidarMouse.h"

#pragma comment(lib,"SpidarMouse.lib")

int FFTMode();

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow ){
	if( ChangeWindowMode(TRUE) != DX_CHANGESCREEN_OK || DxLib_Init() == -1 ) return -1; //初期化処理
	SetDrawScreen( DX_SCREEN_BACK );        //裏画面に設定
	SetAlwaysRunFlag(TRUE);

	if(OpenSpidarMouse() !=1){
		printf("SPIDAR-mouseの接続が確認できませんでした\n");
		//return -1;
	}


	int sound =	0;
	int Youso = 2;
	int Enter = 0;
	int White = GetColor(255, 255, 255);
	MenuElement TOPMENU[1] = {
		{ 80 , 100 , "Enterでスタート"}
	};

	int NowSelect = 0;

	while (ScreenFlip() == 0 &&
		ProcessMessage() == 0 &&
		ClearDrawScreen() == 0 &&
		gpUpdateKey() == 0 &&
		!Key[KEY_INPUT_F3]) {

		if (Key[KEY_INPUT_RETURN] == 0 || Enter == 0) {


			if (Key[KEY_INPUT_DOWN] == 1) {

				NowSelect = (NowSelect + 1) % Youso;

			}

			if (Key[KEY_INPUT_UP] == 1) {

				NowSelect = (NowSelect + Youso - 1) % Youso;

			} //現在選択の変数指定完了

			if (Key[KEY_INPUT_DOWN] == 1 || Key[KEY_INPUT_UP] == 1) {
				for (int i = 0; i < Youso; i++) {
					if (i == NowSelect) {
						TOPMENU[i].x = 80;
					}
					else {
						TOPMENU[i].x = 100;
					}
				}
			}//表示場所指定完了

			for (int i = 0; i < Youso; i++) {
				DrawFormatString(TOPMENU[i].x, TOPMENU[i].y, GetColor(255, 255, 255), TOPMENU[i].name);
			} // 描画終了

		}

			if (Key[KEY_INPUT_RETURN] == 1 || Enter != 0) {
				if (NowSelect == 0) {
					//SetDutyOnCh(0,0,0,0,10000);
					//Sleep(10000);
					//SetDutyOnCh(1,1,1,1,10000);
					//Sleep(10000);
					sound=FFTMode();
					//StopSoundMem(sound);
					//InitSoundMem();
					Enter = 0;
				}
			}
			DrawString(230, 240, "Escapeキーを押すと終了します。", White);
			if (Key[KEY_INPUT_ESCAPE] == 1) {
				Enter = 0;
				return 0;
				CloseSpidarMouse();
				DxLib_End();
			}

	}

	//終了
	CloseSpidarMouse();
	DxLib_End();

	return 0;
}

