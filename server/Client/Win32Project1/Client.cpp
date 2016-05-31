#pragma comment(lib, "ws2_32")
#include "stdafx.h"
#include "User.h"
#include "Monster.h"
#include "Protocol.h"
#include "PacketDispatcher.h"
#include "PacketSender.h"
#include "Server.h"

COLORREF color[10] = { RGB(200,100,150),RGB(100,20,50),RGB(150,100,20),(200,120,0),RGB(0,200,200),
RGB(200,0,200),RGB(200,200,200),RGB(200,0,0),RGB(0,200,0),RGB(0,0,200)};

std::vector<User> users(MAX_USER);
std::vector<Monster> monsters(MAX_NPC);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//전역변수
HINSTANCE g_hInst;
LPCSTR lpszClass = TEXT("게임서버프로그래밍_2반_2010180043_전승규");
HWND hHwnd, hDlg;
int xPos = -RECTSIZE * 4, yPos = -RECTSIZE * 4;
char server_ip[100];
char id[ID_LEN];
char password[PASSWORD_LEN];
RECT clientRect;

bool isDrawTarget = false , isDrawServerPos = false;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG msg;
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = lpszClass;
	wndclass.lpszMenuName = NULL;

	RegisterClass(&wndclass);



	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	g_hInst = hInstance;


	// Check to see if any messages are waiting in the queue
	while (GetMessage(&msg, NULL, 0, 0))
	{
		// Translate the message and dispatch it to WindowProc()
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	

	return (int)msg.wParam;
}
LRESULT CALLBACK DlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	DWORD dwThreadID;
	HANDLE hThread;
	switch (iMessage) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, server_ip, 128);
			GetDlgItemText(hDlg, IDC_EDIT2, id, 20);
			GetDlgItemText(hDlg, IDC_EDIT3, password, 20);

			//수신스레드 
			Server::DispatcherInit();
			Server::receiveThread = new thread{ Server::ReceiveThread };
			
			//로그인서버접속
			if (!Server::LoginServerConnection(id, password, server_ip)) {
				closesocket(Server::sock);
				EndDialog(hDlg, IDCANCEL);
				hDlg = NULL;
				PostQuitMessage(0);
				return TRUE;
			}
			//게임서버접속
			closesocket(Server::sock);
			Server::GameServerConnection();
			PacketSender::instance().requestLogin2(id);

			EndDialog(hDlg, IDCANCEL);
			hDlg = NULL;
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			hDlg = NULL;
			PostQuitMessage(0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HPEN myPen, oldPen;
	HBRUSH myBrush, oldBrush, wBrush, bBrush;
	HDC hdc, hMemDC; // HDC를 하나더 선언해준다. HDC는 '그리는 작업' 이다.
	HBITMAP hBitmap, OldBitmap; // HBITMAP은 대략 종이를 의미한다. 종이 2장 선언
	BYTE dir;
	static BOOL bMove = FALSE;
	int check = 0;
	RECT winRect;
	TCHAR str[50];
	char key;
	GetWindowRect(hwnd, &winRect);
	
	unsigned char *packet;
	static XMFLOAT2 prePos = { 0,0 };
	POINT p, userPoint[4][3];
	
	hHwnd = hwnd;
	switch (msg)
	{
	case WM_CREATE:
		SetRect(&clientRect, 0, 0, 800, 800 + RECTSIZE); //원하는 클라이언트 크기를 저장한다.
		AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE); //윈도우 크기를 계산
		MoveWindow(hwnd, 0, 0,
			clientRect.right - clientRect.left,
			clientRect.bottom - clientRect.top,
			TRUE); //윈도우 크기를 바꿔주고 클라이언트 영역을 새로 그려준다.

		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, DlgProc);
		
		SetTimer(hwnd, 1, 33, NULL);//타이머1
		break;
	case WM_KEYDOWN:
		if (Server::getClientID() == -1) return 0;
		if (users[Server::getClientID()].is_move) return 0;
		switch (wParam)
		{
		case VK_LEFT:
			users[Server::getClientID()].setDir(XMFLOAT2(-1, 0));
			users[Server::getClientID()].setDist(users[Server::getClientID()].getSpeed() / 33);
			users[Server::getClientID()].is_move = true;
			break;
		case VK_RIGHT:
			users[Server::getClientID()].setDir(XMFLOAT2(1, 0));
			users[Server::getClientID()].setDist(users[Server::getClientID()].getSpeed() / 33);
			users[Server::getClientID()].is_move = true;
			break;
		case VK_UP:
			users[Server::getClientID()].setDir(XMFLOAT2(0, -1));
			users[Server::getClientID()].setDist(users[Server::getClientID()].getSpeed() / 33);
			users[Server::getClientID()].is_move = true;
			break;
		case VK_DOWN:
			users[Server::getClientID()].setDir(XMFLOAT2(0, 1));
			users[Server::getClientID()].setDist(users[Server::getClientID()].getSpeed() / 33);
			users[Server::getClientID()].is_move = true;
			break;
		default:
			return 0;
		}
		return 0;
	case WM_CHAR:
		key = (TCHAR)wParam;
		key = tolower(key);
		if (key == 'q')
			PacketSender::instance().PlayerCombat(CC_CircleAround, 0, 0);
		else if (key == 'w')
			PacketSender::instance().PlayerCombat(CC_CircleFront, 0, 0);
		else if (key == 'e')
			PacketSender::instance().PlayerCombat(CC_Eraser, 0, 0);
		else if (key == 'r') {
			GetCursorPos(&p);
			ScreenToClient(hwnd, &p);
			PacketSender::instance().PlayerCombat(CC_PointCircle, p.x+xPos, p.y+yPos);
		}
		else if (key == '1') {
			isDrawTarget = !isDrawTarget;
		}
		else if (key == '2') {
			isDrawServerPos = !isDrawServerPos;
		}
			
		return 0;
	case WM_TIMER:
		//업데이트
		for (auto& data : users)
			if (data.is_using) 
				data.update();
		for (auto& data : monsters)
			if (data.is_using) data.update();
		//렌더링 작업
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	case WM_PAINT:
		//접속이 안되었을 경우 
		if (Server::getClientID() == -1) {
			//not connected
			hdc = BeginPaint(hwnd, &ps);
			wsprintf(str, TEXT("게임서버 (%s)  연결이 되지 않았습니다."), server_ip);
			TextOut(hdc, clientRect.right / 2 - lstrlen(str)*4, clientRect.bottom / 2, str, lstrlen(str));
			EndPaint(hwnd, &ps);
			return 0;
		}

		xPos = users[Server::getClientID()].getPos().x - RECTSIZE * 8;
		yPos = users[Server::getClientID()].getPos().y - RECTSIZE * 8;
		
		hdc = BeginPaint(hwnd, &ps);
		hMemDC = CreateCompatibleDC(hdc); // hMemDC 에 기존 DC (hdc)에 맞게 새 DC 생성
		hBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom); // crt 규격대로 종이 생성
		OldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap); // 종이 교체
		FillRect(hMemDC, &clientRect, (HBRUSH)GetStockObject(WHITE_BRUSH)); //도화지 색 변경

		myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
		oldPen = (HPEN)SelectObject(hMemDC, myPen);
		wBrush = CreateSolidBrush(RGB(255, 255, 255));
		bBrush = CreateSolidBrush(RGB(0, 0, 0));
		oldBrush = (HBRUSH)SelectObject(hMemDC, wBrush);

		check = 0;
		//체스판 그리기
		//for문이 많지만 상수시간이다.
		for (int i = 0; i < 100; i++) {
			for (int j = 0; j < 100; j++) {
				//간격표시자 ( 8칸 ) 
				if ( (check + j ) %2 == 0 )
					(HBRUSH)SelectObject(hMemDC, bBrush);
				else
					(HBRUSH)SelectObject(hMemDC, wBrush);

				for (int z = 0; z < INTERVAL; z++) {
					for (int x = 0; x < INTERVAL; x++) {
						if (clientRect.left < (RECTSIZE*INTERVAL*j + x*RECTSIZE) - xPos && (RECTSIZE*INTERVAL*j + x*RECTSIZE) - xPos < clientRect.right &&
							clientRect.top < (RECTSIZE * INTERVAL * i + z*RECTSIZE) - yPos && (RECTSIZE * INTERVAL * i + z*RECTSIZE) - yPos < clientRect.bottom) {
							Rectangle(hMemDC, (RECTSIZE*INTERVAL*j + x*RECTSIZE) - xPos, (RECTSIZE * INTERVAL * i + z*RECTSIZE) - yPos,
								(RECTSIZE * INTERVAL * j + x*RECTSIZE) - xPos + RECTSIZE, (RECTSIZE * INTERVAL * i + z*RECTSIZE) - yPos + RECTSIZE);
						}
					
					}
				}
			}
			check = 1 - check;
		}
		SelectObject(hMemDC, oldPen);
		SelectObject(hMemDC, oldBrush);
		DeleteObject(myPen);
		DeleteObject(wBrush);
		DeleteObject(bBrush);

	
		
		//유저
		for (int i = 0; i < MAX_USER; i++) {
			if (users[i].is_using) {
				myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(hMemDC, myPen);
				myBrush = CreateSolidBrush(color[users[i].getID()]);
				oldBrush = (HBRUSH)SelectObject(hdc, myBrush);
								
				Rectangle(hMemDC, users[i].getPos().x - xPos - RECTSIZE / 2, users[i].getPos().y - yPos - RECTSIZE / 2, 
					users[i].getPos().x - xPos + RECTSIZE/2, users[i].getPos().y - yPos + RECTSIZE/2);

				SelectObject(hMemDC, oldPen);
				SelectObject(hMemDC, oldBrush);
				DeleteObject(myPen);
				DeleteObject(myBrush);


				myPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
				oldPen = (HPEN)SelectObject(hMemDC, myPen);

				//방향그리기
				MoveToEx(hMemDC, users[i].getPos().x - xPos, users[i].getPos().y - yPos, NULL);
				LineTo(hMemDC, users[i].getPos().x - xPos + users[i].getDir().x*RECTSIZE,
					users[i].getPos().y - yPos + users[i].getDir().y*RECTSIZE);

				SelectObject(hMemDC, oldPen);
				DeleteObject(myPen);


				wsprintf(str, TEXT("[%d] %d"), users[i].getID(), users[i].getHp());
				TextOut(hMemDC, users[i].getPos().x  - xPos, users[i].getPos().y + 10 - yPos, str, lstrlen(str));
		
			}
		}

		//몬스터
		for (int i = MAX_USER; i < MAX_NPC - MAX_USER; i++) {
			if (monsters[i].is_using) {
				myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(hMemDC, myPen);
				myBrush = CreateSolidBrush(COLORREF(RGB(255, 0, 0)));
				oldBrush = (HBRUSH)SelectObject(hMemDC, myBrush);
				Ellipse(hMemDC, monsters[i].getPos().x - xPos - RECTSIZE / 2, monsters[i].getPos().y - yPos - RECTSIZE / 2, 
					monsters[i].getPos().x - xPos + RECTSIZE / 2, monsters[i].getPos().y - yPos + RECTSIZE / 2);
				wsprintf(str, TEXT("[%d] %d"), monsters[i].getID(), monsters[i].getHp());
				TextOut(hMemDC, monsters[i].getPos().x + 5 - xPos, monsters[i].getPos().y + 10 - yPos, str, lstrlen(str));
				SelectObject(hMemDC, oldPen);
				SelectObject(hMemDC, oldBrush);
				DeleteObject(myPen);
				DeleteObject(myBrush);

				
				if (isDrawTarget) {
					//타겟그리기
					myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 255));
					oldPen = (HPEN)SelectObject(hMemDC, myPen);

					
					MoveToEx(hMemDC, monsters[i].getPos().x - xPos, monsters[i].getPos().y - yPos, NULL);
					LineTo(hMemDC, monsters[i].getTargetPos().x - xPos, monsters[i].getTargetPos().y - yPos);

					SelectObject(hMemDC, oldPen);
					DeleteObject(myPen);
				}
				if (isDrawServerPos) {
					//서버위치 그리기
					myPen = CreatePen(PS_SOLID, 5, RGB(0, 255, 0));
					oldPen = (HPEN)SelectObject(hMemDC, myPen);

					
					MoveToEx(hMemDC, monsters[i].getPos().x - xPos, monsters[i].getPos().y - yPos, NULL);
					LineTo(hMemDC, monsters[i].getServerPos().x - xPos, monsters[i].getServerPos().y - yPos);

					SelectObject(hMemDC, oldPen);
					DeleteObject(myPen);
				}
			}
		}


	
		wsprintf(str, TEXT("좌표 ( %d, %d ) "),(DWORD)(users[Server::getClientID()].getPos().x), (DWORD)(users[Server::getClientID()].getPos().y ));
		TextOut(hMemDC, clientRect.right - 200, clientRect.bottom-100, str, lstrlen(str));



		BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hMemDC, 0, 0, SRCCOPY); // 배껴그리기
																		 // hdc 의 0,0 위치에 hMemDC의 0,0위치부터 crt.right,crt.bottom까지의 영역, 즉 crt범위를 그린다 라는 설정인듯.

		DeleteObject(SelectObject(hMemDC, OldBitmap)); // 종이 원래대로 한 후 제거
		DeleteDC(hMemDC); // hMemDC 제거



		EndPaint(hwnd, &ps);
		return 0;
	case WM_GETMINMAXINFO:

		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = clientRect.right;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = clientRect.bottom;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = clientRect.right;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = clientRect.bottom;
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}