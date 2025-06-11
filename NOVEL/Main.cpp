#include <stdio.h>
#include <windows.h> 
//ライブラリ読込
#pragma comment(lib, "winmm.lib")		//音声再生で使用する

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


//BGM 関数
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

	static int sceneNo = -1;		//Scene Number

	static bool dispSel = false;	//選択肢表切替
	static int selNo = 1;			//選択肢番号

	static int BGMNo = 1; //BGM番号

	static char BGMStatus[256]; //BGMステータスメッセージ取得用
	
	static char menuSel[3][50 * 2];	//メニュー選択肢
	static char sel[2][50 * 2]; //選択
	static char story[11][50 * 2]; //ストーリー用文字列

	static HBITMAP hBack[5]; 	//背景画像ハンドル
	static HBITMAP hMenuBack;	//背景画像ハンドル（メニュー用）


	
	//メッセージ別の処理
	switch (uMsg)
	{
	case WM_DESTROY:
		//Destroying has to be done in the opposite order to how it was created!!

		//mp3のクローズ
		mciSendString("close BGM4", nullptr, 0, hWnd);
		mciSendString("close BGM3", nullptr, 0, hWnd);
		mciSendString("close BGM2", nullptr, 0, hWnd);
		mciSendString("close BGM1", nullptr, 0, hWnd);

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
			40,							//文字高
			30,							//文字幅
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
			"Data\\BMP\\menu.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む
		//画像が読み込んだかどうか確認
		if (!hMenuBack) {
			DWORD err = GetLastError();
			char msg[256];
			FormatMessageA(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, err, 0, msg, sizeof(msg), nullptr);
			MessageBoxA(hWnd, msg, "menu.bmpの読み込みに失敗しました", MB_OK | MB_ICONERROR);
		}

		hBack[0] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\street.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0,0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む

		hBack[1] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\mansion.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);

		hBack[2] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\torii.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップx
			0, 0,						//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む

		hBack[10] = (HBITMAP)LoadImage(
			nullptr,					//インスタンス
			"Data\\BMP\\heya.bmp",	//ファイル名
			IMAGE_BITMAP,				//ビットマップ
			0, 0,					//画像のはば、たかさ（０で自動設定）
			LR_LOADFROMFILE);			//ファイルから読み込む

		//文字列のコピー
		strcpy_s(disptext, "昔々。。。");

		//メニュー選択
		strcpy_s(menuSel[0], "プレイ");
		strcpy_s(menuSel[1], "コンティニュー");
		//strcpy_s(menuSel[2], "普通のジャイアン");
		//strcpy_s(menuSel[3], "普通のジャイアン");

		//選択肢のテキスト
		strcpy_s(sel[0], "シーン３に進む");
		strcpy_s(sel[1], "シーン４に進む");


		//ストーリーのテキスト
		strcpy_s(story[0], "シーン１");
		strcpy_s(story[1], "シーン２");
		strcpy_s(story[2], "シーン３");
		strcpy_s(story[10],"シーン４");


		//MIDIまたはmp3ファイルのオプション
		mciSendString("open Data\\BGM\\Holiday.mp3 alias BGM1", nullptr, 0, hWnd);
		mciSendString("open Data\\BGM\\Last.mp3 alias BGM2", nullptr, 0, hWnd);
		mciSendString("open Data\\BGM\\Pinch.mp3 alias BGM3", nullptr, 0, hWnd);
		mciSendString("open Data\\BGM\\ThemeOfGian.mp3 alias BGM4",nullptr,0,hWnd);

		//BGMの再生
		//BGM番号設定
		BGMNo = playBGM(1, hWnd);
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
		case VK_LEFT:
		case VK_RIGHT:
		case VK_ESCAPE:
			if (menuShow)
			{
				menuShow = false;	//メニューを非表示にする
			}
			else
			{
				menuShow = true;	//メニューを表示する
				sceneNo = -1;	//メニューのシーンにする
				dispSel = false;			//選択肢番号を初期化
			}
			break;
		case 'A':
		case 'x':
			break;
		
		
		case VK_RETURN:			//ENTER KEY PRESSED
			switch (sceneNo)
			{

			//メニューのシーン
			case -1:
				//メニューの表示
				switch (selNo){

					//プレイ
					case 1:
						sceneNo = 0;
						break;
					//コンティニュー
					case 2:
						sceneNo = 0;
						break;
				}
				menuShow = false;	//メニューを非表示にする
				break;

			case 0:

				sceneNo = 1;
				dispSel = true;

				//BGMの停止
				stopBGM(BGMNo, hWnd);

				//BGMの再生
				BGMNo = playBGM(2, hWnd);
				break;

			case 1:
				if (selNo == 1)
				{
					sceneNo = 2;
					//waveファイルの再生
					PlaySound("Data\\WAV\\GianVoice.wav",
						nullptr,
						SND_ASYNC | SND_FILENAME | SND_RING);
						//SND_ASYNC			:
						//SND_FILENAME		:
						//SND_LOOP			:
						// 
					//BGMの停止
					stopBGM(BGMNo, hWnd);
					//BGMの再生
					BGMNo = playBGM(BGMNo,hWnd);
				}
				else
				{
					sceneNo = 10;

					//BGMの停止
					stopBGM(BGMNo, hWnd);
					//BGMの再生
					playBGM(4, hWnd);
					BGMNo = 4;
				}
				
				dispSel = false;
				break;

			case 2:
				sceneNo = 0;

				//wave再生停止
				PlaySound(nullptr,nullptr,SND_PURGE);
				
				//BGMの停止
				stopBGM(BGMNo, hWnd);
				//BGMの再生
				BGMNo = playBGM(1,hWnd);
				break;
			case 10:
				sceneNo = 0;
				//BGMの停止
				stopBGM(BGMNo, hWnd);
				//BGMの再生
				BGMNo = playBGM(1,hWnd);

				break;		
			
			}

		}

		InvalidateRect(hWnd, nullptr, FALSE);
		return 0;

	case WM_PAINT :		//ウィンドウが更新された時
		
		//Check if menu has to be shown
		if (menuShow && hMenuBack)
			SelectObject(hMemDC, hMenuBack);
		else
			SelectObject(hMemDC, hBack[sceneNo]);

		//描画開始
		hDC = BeginPaint(hWnd,&ps);

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
		dialogRect.bottom = rect.bottom - 10; dialogRect.top = rect.top + currHeight*80/100;
		
		//When Menu is going to be displayed 
		if (!menuShow)
		{
			//Draw dialog display region
			DrawRectangle(dialogRect,hDC, RGB(10,10,10),RGB(255,255,255));
		
			//Set text color
			SetTextColor(hDC, RGB(0xFF, 0xFF, 0xFF));
			//Select font for text
			SelectObject(hDC, hFont);
		
			//文字列の表示
	
			dialogRect.top += 20;
			dialogRect.left += 20;

			DrawText(hDC,
				story[sceneNo],		//表示する文字列
				-1,				//文字数を指定（-1で全部）
				&dialogRect,			//表示範囲
				DT_WORDBREAK);	//折り返し

			//選択肢の表示
			if (dispSel)
			{
				SetTextColor(hDC, RGB(0xFF, 0xFF, 0xFF));
				for (int i = 0; i < 2; i++)
				{

					if (selNo == i + 1)
					{
						SetBkMode(hDC, OPAQUE);
						SetBkColor(hDC, RGB(0x9A, 0x5A, 0x9A));
					}
					else
						SetBkMode(hDC, TRANSPARENT);

					TextOut(hDC,
						//Tried to center the select dialog
						rect.left + (rect.right - rect.left) * 15 / 100,	//代表ｘ座標 
						120 + 60 * i,		//代表ｙ座標
						sel[i],				//代表する文字列
						lstrlen(sel[i]));	//代表する文字列のサイズ
				}
			}

		}
		else
		{
			//Set game title text color
			SetTextColor(hDC, RGB(0x9A, 0x5A, 0x9A));
			//Select font for game title text
			SelectObject(hDC, hMenuFont);

			//Show game title 
			DrawText(hDC,
				"物語",		//表示する文字列
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
					SetBkColor(hDC, RGB(0x9A, 0x5A, 0x9A));
				}
				else
					SetBkMode(hDC, TRANSPARENT);

				TextOut(hDC,
					rect.left + (rect.right - rect.left) * 5 / 100,	//代表ｘ座標 
					220 + 60 * i,		//代表ｙ座標
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
		"基本ウィンドウ",		//ウィンドウタイトル
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

