#include <stdio.h>
#include <windows.h> 
//ライブラリ読込
#pragma comment(lib, "winmm.lib")		//音声再生で使用する

static char gameTitle[16] = "ONe cHAncE";
static bool closeWindowFlag = false;

//Tool Functions


//===================================================================================

void DrawTransparentRectangle(RECT rc, HDC hDC, COLORREF outline)
{
	//Create brush and pen
	HPEN hPen = CreatePen(PS_SOLID, 5, outline);

	//Select brush and pen
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);

	Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);

	// Select the original objects back into the DC
	SelectObject(hDC, hOldBrush);
	SelectObject(hDC, hOldPen);

	// Delete the custom objects to free memory
	DeleteObject(hPen);
}

void DrawRectangle(RECT rc, HDC hDC, COLORREF bg, COLORREF outline)
{

	//Create brush and pen
	HBRUSH hBrush = CreateSolidBrush(bg);
	HPEN hPen = CreatePen(PS_SOLID, 5, outline);

	//Select brush and pen
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);

	Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);

	// Select the original objects back into the DC
	SelectObject(hDC, hOldBrush);
	SelectObject(hDC, hOldPen);

	// Delete the custom objects to free memory
	DeleteObject(hBrush);
	DeleteObject(hPen);
}


void ResetStoryLine(int* storyLine, static char story)
{

}


//BGM 管理関数
const char* checkBGMStatus(int BGMNo, char* BGMStatus, HWND hWnd)
{
	char tmp[32];
	sprintf_s(tmp, "%s%d%s", "status BGM", BGMNo, " mode");
	mciSendStringA(tmp, BGMStatus, 255, hWnd);
	return BGMStatus;
}
int playBGM(int BGMNo, HWND hWnd)
{
	char tmp[32];
	sprintf_s(tmp, "%s%d%s", "play BGM", BGMNo, " notify");
	mciSendStringA(tmp, nullptr, 0, hWnd);
	return BGMNo;
}
void restartBGM(int BGMNo, HWND hWnd)
{
	char tmp[32];
	sprintf_s(tmp, "%s%d%s", "seek BGM", BGMNo, " to start");
	mciSendStringA(tmp, nullptr, 0, hWnd);
	playBGM(BGMNo, hWnd);
}
void stopBGM(int BGMNo, HWND hWnd)
{
	char tmp[32];
	sprintf_s(tmp, "%s%d%s", "stop BGM", BGMNo, "");
	restartBGM(BGMNo, hWnd);
	mciSendStringA(tmp, nullptr, 0, hWnd);
}

void NextScene(HWND& hWnd, int& sceneNo, int& dispNo, bool& menuShow, bool& questionFlag, int& BGMNo);


LRESULT CALLBACK WindowProc(
	HWND hWnd,		//Window Handler
	UINT uMsg,		//Message
	WPARAM wParam,	//Width Parameter
	LPARAM lParam)	//Length Parameter
{
	HDC hDC;		//テバイスコンテキスト
	PAINTSTRUCT ps; //描画構造体

	static char disptext[128];	//表示確認用

	static HDC hMemDC;		//メモリーデバイスコンテキスト

	static HFONT hFont;		//フォントハンドル
	static HFONT hMenuFont;

	static int currWidth = 0;	//ウィンドウの幅
	static int currHeight = 0;	//ウィンドウの高さ

	static bool menuShow = true;	//メニュー表示フラグ

	static int lastScene = -1;
	static int sceneNo = -1;		//Scene Number

	static bool dispSelFlag = false;	
	static bool dispSel[51];	//選択肢表切替
	static int selNo = 1;			//選択肢番号

	static int BGMNo = 1; //BGM番号

	static char BGMStatus[256]; //BGMステータスメッセージ取得用

	static char menuSel[3][50 * 2];	//メニュー選択肢
	static char sel[100][50 * 2]; //選択

	static char inScreenText[200 * 3] = ".......";	//画面に表示する文字列
	static bool clearText = false;

	static bool questionFlag = false; //重要な質問かどうか

	static char story[52][200 * 3]; //ストーリー用文字列
	static int storyLine[50]; //ストーリー用行番号
	static int lastLine = 0;

	static HBITMAP hBack[52]; 	//背景画像ハンドル
	static HBITMAP hMenuBack;	//背景画像ハンドル（メニュー用）



	//メッセージ別の処理
	switch (uMsg)
	{
	case WM_DESTROY:
		//Destroying has to be done in the opposite order to how it was created!!

		//mp3のクローズ
		mciSendString("close BGM10", nullptr, 0, hWnd);
		mciSendString("close BGM11", nullptr, 0, hWnd);
		mciSendString("close BGM5", nullptr, 0, hWnd);
		mciSendString("close BGM1", nullptr, 0, hWnd);
		mciSendString("close BGM0", nullptr, 0, hWnd);

		//Delete Background Bitmap	
		DeleteObject(hBack);
		DeleteObject(hMenuBack);

		//Delete Font
		DeleteObject(hFont);
		DeleteObject(hMenuFont);

		//Delete Font Resource
		RemoveFontResourceEx("Data\\FONT\\魔導太丸ゴシック.ttf", FR_PRIVATE, 0);

		//Delete Memory Device Context Handler
		DeleteDC(hMemDC);

		//Informs Window that the application is closing
		PostQuitMessage(0);
		return 0;

	case WM_CREATE:		//ウィンドウが生成される時

		//フォントの登録
		// アプリ終了時は RemoveFontResourceEx で解除


		//メモリDCの作成
		hMemDC = CreateCompatibleDC(nullptr);

		AddFontResourceEx("Data\\FONT\\MadouFutoMaruGothic.ttf", FR_PRIVATE, 0);
		//フォントの作成
		hMenuFont = CreateFont(
			120,							//文字高
			100,							//文字幅
			0,							//角度
			0,							//ベースライン角度
			FW_THIN,					//太さ
			FALSE,						//
			FALSE,						//下線
			FALSE,						//打消し
			SHIFTJIS_CHARSET,			//文字セット
			OUT_DEFAULT_PRECIS,			//出力精度
			CLIP_DEFAULT_PRECIS,		//クリップング精度
			DEFAULT_QUALITY,			//出力品質
			VARIABLE_PITCH | FF_ROMAN,	//	可愛ピッチとフォントファミリ
			"魔導太丸ゴシック");			//書体(nullptr:現在使われている書体)

		hFont = CreateFont(
			60,							//文字高
			40,							//文字幅
			0,							//角度
			0,							//ベースライン角度
			FW_REGULAR,					//太さ
			FALSE,						//
			FALSE,						//下線
			FALSE,						//打消し
			SHIFTJIS_CHARSET,			//文字セット
			OUT_DEFAULT_PRECIS,			//出力精度
			CLIP_DEFAULT_PRECIS,		//クリップング精度
			DEFAULT_QUALITY,			//出力品質
			VARIABLE_PITCH | FF_ROMAN,	//	可愛ピッチとフォントファミリ
			"魔導太丸ゴシック");			//書体(nullptr:現在使われている書体)

		//背景の読み込む
		hMenuBack = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\menu.bmp",		//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む

		////画像が読み込んだかどうか確認
		//if (!hMenuBack) {
		//	DWORD err = GetLastError();
		//	char msg[256];
		//	FormatMessageA(
		//		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		//		nullptr, err, 0, msg, sizeof(msg), nullptr);
		//	MessageBoxA(hWnd, msg, "menu.bmpの読み込みに失敗しました", MB_OK | MB_ICONERROR);
		//}

		hBack[0] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\2doors.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む

		hBack[1] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\guards1.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);

		hBack[6] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\leftguard.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);

		hBack[9] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\rightguard.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);

		hBack[11] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\pointToDoor.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップx
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む
		hBack[14] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\pointToDoor.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップx
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む
		hBack[16] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\pointToDoor.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップx
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む
		hBack[19] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\pointToDoor.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップx
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む

		hBack[49] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\heaven.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,					//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む
		hBack[50] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\gian.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,					//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む

		hBack[47] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\hell.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,					//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む
		hBack[48] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\evilGian.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,					//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む

		//メニュー選択
		strcpy_s(menuSel[0], "開始");
		strcpy_s(menuSel[1], "終了");
		//strcpy_s(menuSel[2], "普通のジャイアン");
		//strcpy_s(menuSel[3], "普通のジャイアン");

		//選択肢のテキスト
		//SCENE 1
		strcpy_s(sel[1], "左の門番に近づく");
		strcpy_s(sel[2], "右の門番に近づく");

		//SCENE 6 LEFT GUARD
		strcpy_s(sel[6], "どの道を進むのか？");
		strcpy_s(sel[7], "向こうの番人に正しい道を問えば、何と答えるだろう？");
		
		//SCENE 9 RIGHT GUARD
		strcpy_s(sel[9], "どの道を進むのか？");
		strcpy_s(sel[10], "もう一人の番人に道尋ねれば、何と応えるだろうか。");

		//SCENE 11 LEFT GUARD	DIRECT QUESTION
		strcpy_s(sel[11], "番人が指し示した道を進む。");
		strcpy_s(sel[12], "あえて逆の道を選ぶ。");

		//SCENE 14 RIGHT GUARD	DIRECT QUESTION
		strcpy_s(sel[14], "番人が指し示した道を進む。");
		strcpy_s(sel[15], "あえて逆の道を選ぶ。");

		//SCENE 16 LEFT GUARD	INDIRECT QUESTION	
		strcpy_s(sel[16], "番人が告げた道を進む。");
		strcpy_s(sel[17], "番人が告げし道とは異なる方を選ぶ");

		//SCENE 19 RIGHT GUARD INDIRECT QUESTION	
		strcpy_s(sel[19], "番人が告げた道を進む。");
		strcpy_s(sel[20], "番人が告げし道とは異なる方を選ぶ");
		
		//ストーリーのテキスト
		strcpy_s(story[0], "あなたは冷たいそよ風で目を覚まし、人生の最後の響きが夢のように…\n消えていく。気がつくと、そこはリンボーの世界だった。\n目の前には、煙のように姿を変える2人の謎めいた人物が立っている。\n一人は真実のみを語り、もう一人は嘘のみを語る。\nあなたの目標は、自分の価値を証明し、進むべき道を見つけることだ。");
		strcpy_s(story[1], "重苦しい沈黙があなたを襲い、過去の行いの重みが空気にまとわりついている。\nあなたは伝説を知っている：門番の1人に1つだけ質問することができる。\nあなたの運命は天秤にかかっている。");

		strcpy_s(story[4], "あなたは左手の人物に向かって足を踏み出す。その姿に光の揺らめきが舞う。");
		strcpy_s(story[8], "あなたは右手の人影に向かって足を踏み出す。まるであなたを観察するかのように、その影が濃くなる。");

		strcpy_s(story[6], "ご質問どうぞ");
		strcpy_s(story[9], "ご質問どうぞ");
		//DIRECT QUESTION TRUTH TELLER
		strcpy_s(story[11], "選ばれし番人は、あなたへ首を傾げた。\n左の番人： その道は、先へ通ず\n冷たい不安に心が縛られる。\nまだどの道が正しいのかも分からない。\n	唯一の問いは、もう使ってしまった。"); // THIS IS THE CORRECT PATH	
		
		//DIRECT QUESTION LIAR
		strcpy_s(story[14], "番人があなたの方へ顔を向けた。\n 右の番人：　その道こそが、先へ続く。\n冷たい不安に心が縛られる。\nまだどの道が正しいのかも分からない。\n	唯一の問いは、もう使ってしまった。");	
		

		//INDERECT QUESTION TRUTH TELLER
		strcpy_s(story[16], "左の番人：左の道こそ、先へ続くと告げるだろう。\nその答えを慎重に吟味する。そして、選び取る。");
		
		//INDERECT QUESTION LIAR
		strcpy_s(story[19], "右の番人：左の道こそ、先へ続くと告げるだろう。\nその答えを慎重に吟味する。そして、選び取る。");

		
		strcpy_s(story[49], "その道を進むにつれ、温かな光が強まり、影を押し退けていく。\n番人たちは、その役目を終え、背後に消え去った。\nお前は、自らの価値を証明したのだ。\n天国へようこそ。\n今、創造主とまみえる時。"); //GOOD ENDING GOOD GIAN PICTURE
		strcpy_s(story[50], "...");

		strcpy_s(story[47], "扉をくぐると、途端に全てが霧散し、熱が全身を包み込む。\nもう引き返せぬと悟った時には遅かった。\n地獄へと落ちたのだ。\n今、彼とまみえねばならぬ。"); //BAD ENDING EVIL GIAN PICTURE
		strcpy_s(story[48], "..."); //BAD ENDING EVIL GIAN PICTURE


		memset(dispSel, 0, sizeof(dispSel)); //ストーリーの行番号を初期化

		dispSel[1] = true;
		dispSel[6] = true;
		dispSel[9] = true;
		dispSel[11] = true;
		dispSel[14] = true;
		dispSel[16] = true;
		dispSel[19] = true;

		memset(storyLine, 0, sizeof(storyLine)); //ストーリーの行番号を初期化

		for (int i = 0; i < 50; i++)
		{
			for (int j = 0; j < 200 * 3; j++)
			{
				if (story[i][j] == '\n')
					storyLine[i]++;	//ストーリーの行番カウンター
			}
		}

		//MIDIまたはmp3ファイルのオプション
		mciSendString("open Data\\BGM\\Menu.wav alias BGM0", nullptr, 0, hWnd);
		mciSendString("open Data\\BGM\\Loop.mp3 alias BGM1", nullptr, 0, hWnd);
		mciSendString("open Data\\BGM\\DecisionMaking.mp3 alias BGM5", nullptr, 0, hWnd);
		mciSendString("open Data\\BGM\\EvilGian.mp3 alias BGM11", nullptr, 0, hWnd);
		mciSendString("open Data\\BGM\\GoodEnd.mp3 alias BGM10", nullptr, 0, hWnd);


		//BGMの再生
		//BGM番号設定
		BGMNo = playBGM(0, hWnd);
		//タイマー設定
		SetTimer(hWnd,
			1,			//タイマーNo
			100,		//時間
			nullptr);	//一定時間に動かす関数名

	case WM_TIMER:	//タイマーで動く

		//曲の状態を
		checkBGMStatus(BGMNo, BGMStatus, hWnd);

		//曲が停まっているか
		if (strcmp(BGMStatus, "stopped") == 0)
		{
			restartBGM(BGMNo, hWnd);
		}

		return 0;

	case WM_LBUTTONDOWN:		//MOUSE LEFT CLICK DOWN
	case WM_RBUTTONDOWN:		//MOUSE RIGHT CLICK DOWN
		//if (sceneNo + 1 == 1)dispSel = true;
		//else dispSel = false;
		//sceneNo++;			//Show Scene 1
		//がめんの再描画
		InvalidateRect(hWnd, nullptr, FALSE);
		return 0;

	case WM_KEYDOWN:

		switch (wParam)
		{
		case VK_UP:
			selNo--;
			if (selNo < 1)
				selNo = 1;
			break;
		case VK_DOWN:
			selNo++;
			if (selNo > 2)
				selNo--;
			break;
		case VK_ESCAPE:
			//ESCAPEキーが押されたらメニューを表示
			if (sceneNo != -1)
			{
				sceneNo = -1;
				menuShow = true;
				selNo = 1;	//選択肢の初期化
			}
			else
			{
				menuShow = false;

			}
			break;
		case 'A':
		case 'x':
			break;


		case VK_RETURN:			//ENTER KEY PRESSED

			if (sceneNo == -1)
			{

				memset(storyLine, 0, sizeof(storyLine)); //ストーリーの行番号を初期化
				memset(inScreenText, ' ', sizeof(inScreenText));

				for (int i = 0; i < 50; i++)
				{
					for (int j = 0; j < 200 * 3; j++)
					{
						if (story[i][j] == '\n')
							storyLine[i]++;	//ストーリーの行番カウンター
					}
				}
				menuShow = true;
			}

			if ((sceneNo > -1) && (storyLine[sceneNo] >= 0))
			{
				dispSelFlag = false;	//選択肢表示フラグを立てる	
				--storyLine[sceneNo];	//ストーリーの行番号を減らす
				clearText = true;	//テキストクリアフラグを立てる

				memset(inScreenText, ' ', sizeof(inScreenText));

				for (int i = lastLine; i < 200 * 3; i++)
				{
					if (story[sceneNo][i] == '\n')
					{
						lastLine = i + 1;	//ストーリーの行番号を更新
						break;
					}
					inScreenText[i - lastLine] = story[sceneNo][i];
				}

				break;
			}

			clearText = false;
			lastLine = 0;
			NextScene(hWnd,sceneNo, selNo, menuShow, questionFlag, BGMNo);

		//switch (sceneNo)
		//{

		//	case 0:
		//		//BGMの停止
		//		stopBGM(BGMNo, hWnd);

		//		//BGMの再生
		//		BGMNo = playBGM(2, hWnd);
		//		break;

		//		//case 1:
		//		//	if (selNo == 1)
		//		//	{
		//		//		sceneNo = 2;
		//		//		//waveファイルの再生
		//		//		PlaySound("Data\\WAV\\GianVoice.wav",
		//		//			nullptr,
		//		//			SND_ASYNC | SND_FILENAME | SND_RING);
		//		//			//SND_ASYNC			:
		//		//			//SND_FILENAME		:
		//		//			//SND_LOOP			:
		//		//			// 
		//		//		//BGMの停止
		//		//		stopBGM(BGMNo, hWnd);
		//		//		//BGMの再生
		//		//		BGMNo = playBGM(BGMNo,hWnd);
		//		//	}
		//		//	else
		//		//	{
		//		//		sceneNo = 10;

		//		//		//BGMの停止
		//		//		stopBGM(BGMNo, hWnd);
		//		//		//BGMの再生
		//		//		playBGM(4, hWnd);
		//		//		BGMNo = 4;
		//		//	}
		//		//	
		//		//	dispSel = false;
		//		//	break;

		//		//case 2:
		//		//	sceneNo = -1;

		//		//	//wave再生停止
		//		//	PlaySound(nullptr,nullptr,SND_PURGE);
		//		//	
		//		//	//BGMの停止
		//		//	stopBGM(BGMNo, hWnd);
		//		//	//BGMの再生
		//		//	BGMNo = playBGM(1,hWnd);
		//		//	break;
		//		//case 10:
		//		//	sceneNo = -1;
		//		//	//BGMの停止
		//		//	stopBGM(BGMNo, hWnd);
		//		//	//BGMの再生
		//		//	BGMNo = playBGM(1,hWnd);

		//		//	break;	
		//		// 	
		//	}
		}

		InvalidateRect(hWnd, nullptr, FALSE);
		return 0;

	case WM_PAINT:		//ウィンドウが更新された時


		if (closeWindowFlag)
		{
			DestroyWindow(hWnd);
		}

		//Check if menu has to be shown
		if (menuShow && hMenuBack)
			SelectObject(hMemDC, hMenuBack);
		else
			SelectObject(hMemDC, hBack[sceneNo]);

		//描画開始
		hDC = BeginPaint(hWnd, &ps);

		//描画の準備
		RECT refreshRect;
		GetClientRect(hWnd, &refreshRect)
			;
		//ウィンドウの大きさを取得
		currWidth = refreshRect.right - refreshRect.left;
		currHeight = refreshRect.bottom - refreshRect.top;

		// StretchBltで拡大・縮小描画
		StretchBlt(
			hDC, 0, 0, currWidth, currHeight, // 描画先（拡大後のサイズ）
			hMemDC, 0, 0, 860, 640,   // 元画像サイズ（例: 860x640）
			SRCCOPY
		);

		////背景の画像をメモリDCへコピー
		////背景の表示
		//BitBlt(hDC,			//デバイスコンテキスト
		//	0, 0,			//表示位置ｘ、ｙ座標
		//	currWidth, currHeight,		//画像幅、たかさ
		//	hMemDC,			//メモリDC
		//	0, 0,			//元画像x,y,座標
		//	SRCCOPY);		//コピーする

		//SetBkMode(hDC, OPAQUE);
		//SetBkColor(hDC, RGB(0x00, 0x00, 0x00));
		SetBkMode(hDC, TRANSPARENT);


		RECT rect;		//矩形工事構造体		
		//文字列の表示範囲
		//ウィンドウの大きさを取得
		//RECT構造体のleftとtop
		GetClientRect(hWnd, &rect);
		//表示開始位置設定
		rect.left = 10;
		rect.top = 20;

		//Set the dialog display region
		RECT dialogRect;
		dialogRect.left = rect.left + 10; dialogRect.right = rect.right - 10;
		dialogRect.bottom = rect.bottom - 10; dialogRect.top = rect.top + currHeight * 70 / 100;

		//When Menu is going to be displayed 
		if (!menuShow)
		{
			//Draw dialog display region
			DrawRectangle(dialogRect, hDC, RGB(0x0C, 0x0C, 0x0C), RGB(255, 255, 255));

			//Set text color
			SetTextColor(hDC, RGB(0xFF, 0xFF, 0xFF));
			//Select font for text
			SelectObject(hDC, hFont);

			//文字列の表示
			dialogRect.top += 20;
			dialogRect.left += 20;

			if (clearText)
			{
				clearText = false;
				InvalidateRect(hWnd, nullptr, FALSE); // Request redraw
			}




			DrawText(hDC,
				inScreenText,		//表示する文字列
				-1,				//文字数を指定（-1で全部）
				&dialogRect,			//表示範囲
				DT_WORDBREAK);	//折り返し

			//strcpy_s(story[sceneNo], sizeof(story[sceneNo]), "");
			//InvalidateRect(hWnd, nullptr, FALSE); // Request redraw

			//選択肢の表示
			if (dispSel[sceneNo] && (storyLine[sceneNo] < 0))
			{
				int cnt = 1;
				SetTextColor(hDC, RGB(0xFF, 0xFF, 0xFF));
				for (int i = sceneNo; i < sceneNo + 2; i++)
				{

					if (selNo == cnt)
					{
						SetBkMode(hDC, OPAQUE);
						if(questionFlag)
							SetBkColor(hDC, RGB(199, 2, 2));
						else 
							SetBkColor(hDC, RGB(0x8B, 0x8B, 0x8B));
					}
					else
						SetBkMode(hDC, TRANSPARENT);

					RECT drawRect;
					drawRect.left = rect.left + (rect.right - rect.left) * 20 / 100;
					drawRect.top = 30 + 60 * cnt;
					drawRect.right = rect.right - (rect.right - rect.left) * 10 / 100;
					drawRect.bottom = currHeight;

					DrawText(hDC,
						sel[i],				//代表する文字列
						-1,
						&drawRect,
						DT_WORDBREAK);
						

					cnt++;
				}
			}

		}
		else
		{
			//When Menu is going to be displayed
			//Set game title text color
			SetTextColor(hDC, RGB(0xEC, 0xEC, 0xEC));
			//Select font for game title text
			SelectObject(hDC, hMenuFont);

			//Show game title 
			DrawText(hDC,
				gameTitle,		//表示する文字列
				-1,				//文字数を指定（-1で全部）
				&rect,			//表示範囲
				DT_WORDBREAK);	//折り返し

			//Set menu text color
			SetTextColor(hDC, RGB(0xFF, 0xFF, 0xFF));
			SelectObject(hDC, hFont);

			//Show menu options
			for (int i = 0; i < 2; i++)
			{

				if (selNo == i + 1)
				{
					SetBkMode(hDC, OPAQUE);
					SetBkColor(hDC, RGB(0x8B, 0x8B, 0x8B));
				}
				else
					SetBkMode(hDC, TRANSPARENT);

				TextOut(hDC,
					rect.left + currWidth * 5 / 100,	//代表ｘ座標 
					(rect.top + currHeight * 30 / 100) + 60 * i,		//代表ｙ座標
					menuSel[i],				//代表する文字列
					lstrlen(menuSel[i]));	//代表する文字列のサイズ
			}
		}

		EndPaint(hWnd, &ps);

		return 0;

	}
	//Give back information to MAIN
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,	//インスタンス番号(ウィンドウの番号)
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PSTR lpCmdLine,
	_In_ int nCmdShow)
{
	WNDCLASS wc;				//Window Class ウィンドウクラス構造体(Struct	ure)
	HWND hWnd;					//Window Handler ウィンドウハンドル
	MSG msg;					//メッセージ

	//--------------------------------------------
	//ウィンドウクラス登録
	//--------------------------------------------
	//スタイルの登録
	//　CS_HREDRAW Horizontal Redraw	：	水平方向の再描画
	//	CS_VREDRAW Vertical Redraw	：	垂直方向の再描画
	//

	wc.style = CS_HREDRAW | CS_VREDRAW;
	//ウィンドウ関数の登録
	//DefWindowProc : デフォルトウィンドウ関数(後で自分で作る)
	wc.lpfnWndProc = WindowProc;
	//使わない(0固定)
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	//インスタンス番号の登録
	wc.hInstance = hInstance;
	//アイコン登録
	//nullptr : デフォルト
	wc.hIcon = nullptr;
	//カーソル登録
	//nullptr : デフォルト
	wc.hCursor = nullptr;
	//ウィンドウの背景色
	// LTGRAY_BRUSH : 明るい灰色
	wc.hbrBackground = (HBRUSH)GetStockObject(1);
	//メニューの登録
	//nullptr : デフォルト
	wc.lpszMenuName = nullptr;
	//アプリケーション名
	wc.lpszClassName = "MainWindow";

	//ウィンドウをWindows登録
	if (RegisterClass(&wc) == 0)
	{
		//エラーメッセージの表示
		//MB_OK : OKボタンのみ
		//MB_YESNO : OK・キャンセル
		//MB_YESNOCANCEL : ハイ・いいえ・キャンセル
		MessageBoxA(nullptr,
			"ウィンドウクラス登録失敗",
			"エラーメッセージ",
			MB_YESNOCANCEL);
		return 0;
	}

	//--------------------------------------------
	//ウィンドウの作成
	//--------------------------------------------
	hWnd = CreateWindow(
		"MainWindow",		//アプリケーション名
		gameTitle,		//ウィンドウタイトル
		WS_OVERLAPPEDWINDOW,//普通のウィンドウ
		100, 100,			//ウィンドウの表示位置(x,y)
		860, 640,			//ウィンドウの幅、高さ
		nullptr,			//親ウィンドウのハンドル
		nullptr,			//メニューの
		hInstance,			//インスタンス番号
		nullptr);			//ウィンドウ作成時には発生するイベントに渡すデータ

	if (hWnd == nullptr)
	{
		MessageBox(nullptr,
			"ウィンドウ作成失敗",
			"エラーメッセージ", MB_OK);
		return 0;
	}

	//ウィンドウの表示
	//SW_SHOW : 表示する
	//SW_HIDE :　隠す（非表示）
	ShowWindow(hWnd, SW_SHOW);



	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		//ウィンドウ関数メッセージを送る
		DispatchMessage(&msg);
	}

	return 0;
}
//シーンの切り替え関数
void NextScene(HWND& hWnd, int& sceneNo, int& selNo, bool& menuShow, bool& questionFlag, int& BGMNo)
{
	switch (sceneNo)
	{
	case -1:
		menuShow = true;
		switch (selNo) {

		case 1:
			sceneNo = 0;
			break;
		case 2:
			closeWindowFlag = true;	//ウィンドウを破棄
			break;
		}
		menuShow = false;
		stopBGM(0, hWnd); //BGMを停止
		BGMNo = playBGM(1, hWnd); //BGMを再生
		break;
	case 0:

		sceneNo = 1;
		break;

	case 1:
		switch (selNo) {
		case 1:
			sceneNo = 4;
			break;
		case 2:
			sceneNo = 8;
			break;
		}
		break;
	
	case 4:
		sceneNo = 6;
		stopBGM(BGMNo, hWnd); //BGMを停止
		BGMNo = playBGM(5, hWnd); //BGMを再生
		break;
	case 8:
		sceneNo = 9;
		stopBGM(BGMNo, hWnd); //BGMを停止
		BGMNo = playBGM(5, hWnd); //BGMを再生
		break;

	case 6:
		switch (selNo) {
		PlaySound("Data\\WAV\\OpenDoor.wav",
			nullptr, 
			SND_ASYNC | SND_FILENAME | SND_PURGE); //ドアを開く音を再生
		case 1:
			sceneNo = 11;
			questionFlag = true; //重要な質問フラグを立てる
			break;
		case 2:
			sceneNo = 16;
			questionFlag = true; //重要な質問フラグを立てる
			break;
		}
		break;

	case 9:
		switch (selNo) {
		case 1:
			sceneNo = 14;
			questionFlag = true; //重要な質問フラグを立てる
			break;
		case 2:
			sceneNo = 19;
			questionFlag = true; //重要な質問フラグを立てる
			break;

		}
		break;

	case 11:
		PlaySound("Data\\WAV\\OpenDoor.wav",
			nullptr, 
			SND_ASYNC | SND_FILENAME | SND_PURGE);
		switch (selNo) {

		case 1:
			sceneNo = 49;
			questionFlag = true; //重要な質問フラグを立てる
			stopBGM(BGMNo, hWnd); //BGMを停止
			BGMNo = playBGM(10, hWnd); //BGMを再生
			break;
		case 2:
			sceneNo = 47;
			questionFlag = true; //重要な質問フラグを立てる
			stopBGM(BGMNo, hWnd); //BGMを停止
			BGMNo = playBGM(11, hWnd); //BGMを再生
			break;

		}
		break;
	case 14:
		PlaySound("Data\\WAV\\OpenDoor.wav",
			nullptr,
			SND_ASYNC | SND_FILENAME | SND_PURGE);
		switch (selNo) {

		case 1:
			sceneNo = 47;
			questionFlag = true; //重要な質問フラグを立てる
			stopBGM(BGMNo, hWnd); //BGMを停止
			BGMNo = playBGM(11, hWnd); //BGMを再生
			break;
		case 2:
			sceneNo = 49;
			questionFlag = true; //重要な質問フラグを立てる
			stopBGM(BGMNo, hWnd); //BGMを停止
			BGMNo = playBGM(10, hWnd); //BGMを再生
			break;

		}
		break;
	case 16:
		PlaySound("Data\\WAV\\OpenDoor.wav",
			nullptr,
			SND_ASYNC | SND_FILENAME | SND_PURGE);
		switch (selNo) {

		case 1:
			sceneNo = 47;
			questionFlag = true; //重要な質問フラグを立てる
			stopBGM(BGMNo, hWnd); //BGMを停止
			BGMNo = playBGM(11, hWnd); //BGMを再生
			break;
		case 2:
			sceneNo = 49;
			questionFlag = true; //重要な質問フラグを立てる
			stopBGM(BGMNo, hWnd); //BGMを停止
			BGMNo = playBGM(10, hWnd); //BGMを再生
			break;

		}
		break;
	case 19:
		PlaySound("Data\\WAV\\OpenDoor.wav",
			nullptr, 
			SND_ASYNC | SND_FILENAME | SND_PURGE);
		switch (selNo) {

		case 1:
			sceneNo = 47;
			questionFlag = true; //重要な質問フラグを立てる
			stopBGM(BGMNo, hWnd); //BGMを停止
			BGMNo = playBGM(11, hWnd); //BGMを再生
			break;
		case 2:
			sceneNo = 49;
			questionFlag = true; //重要な質問フラグを立てる
			stopBGM(BGMNo, hWnd); //BGMを停止
			BGMNo = playBGM(10, hWnd); //BGMを再生
			break;

		}
		break;

	case 47:
		//BAD ENDING
		PlaySound("Data\\WAV\\Scream.wav", //DISTORTED VOICE
			nullptr,
			SND_ASYNC | SND_FILENAME | SND_PURGE);
		stopBGM(BGMNo, hWnd); //BGMを停止
		BGMNo = playBGM(11, hWnd); //BGMを再生
		sceneNo = 48; //EVIL GIAN PICTURE
		break;
	case 49:
		//GOOD ENDING
		PlaySound("Data\\WAV\\GianVoice.wav",
			nullptr,
			SND_ASYNC | SND_FILENAME | SND_PURGE);
		sceneNo = 50; //GOOD GIAN PICTURE
		break;

	default:
		sceneNo = -1;	//初期化
		menuShow = true;	//メニューを表示
		stopBGM(BGMNo, hWnd); //BGMを停止
		BGMNo = playBGM(0, hWnd); //BGMを再生
		break;

	}
}
