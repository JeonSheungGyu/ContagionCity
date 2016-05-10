#pragma comment(lib, "ws2_32")
#include "stdafx.h"
#include "User.h"
#include "Monster.h"
#include "Protocol.h"
#include "PacketDispatcher.h"


COLORREF color[10] = { RGB(200,100,150),RGB(100,20,50),RGB(150,100,20),(200,120,0),RGB(0,200,200),
RGB(200,0,200),RGB(200,200,200),RGB(200,0,0),RGB(0,200,0),RGB(0,0,200)};


std::vector<User> users(MAX_USER);
std::vector<Monster> monsters(MAX_NPC);

DWORD WINAPI ThFunc(LPVOID lpParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int recvn(SOCKET s, char *buf, int len, int flags);
void ErrorHandling(char *msg);



//전역변수
HINSTANCE g_hInst;
LPCSTR lpszClass = TEXT("게임서버프로그래밍_2반_2010180043_전승규");
HANDLE hStartupEvent;
HWND hHwnd, hDlg;
bool isSet = false;
int xPos = -RECTSIZE * 4, yPos = -RECTSIZE * 4;
char server_ip[100];
//네트워크 전역변수
WSABUF wsabuf;
CHAR buffer[BUFSIZE] = { 0, };
SOCKET hSocket;
WSAEVENT event;
WSAOVERLAPPED overlapped;
RECT clientRect;
DWORD myID;


int sendBytes = 0;
int recvBytes = 0;
int flags = 0;

void initialize() {

	wsabuf.buf = reinterpret_cast<CHAR *>(buffer);
	wsabuf.len = BUFSIZE;
}
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

			hStartupEvent = CreateEvent(0, FALSE, FALSE, 0);
			//통신스레드
			hThread = CreateThread(NULL, 0, ThFunc, NULL, 0, &dwThreadID);
			CloseHandle(hThread);
			WaitForSingleObject(hStartupEvent, INFINITE);
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

	static BOOL bMove = FALSE;
	int check = 0;
	RECT winRect;
	TCHAR str[50];
	GetWindowRect(hwnd, &winRect);
	cs_packet_dir dir_packet;
	unsigned char *packet;
	static XMFLOAT2 prePos = { 0,0 };

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

		initialize();
		
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, DlgProc);

		SetTimer(hwnd, 1, 33, NULL);//타이머1
		break;
	case WM_KEYDOWN:
		if (users[myID].is_move) return 0;
		switch (wParam)
		{
		case VK_LEFT:			
			dir_packet.type = CS_LEFT;
			break;
		case VK_RIGHT:
			dir_packet.type = CS_RIGHT;
			break;
		case VK_UP:
			dir_packet.type = CS_UP;
			break;
		case VK_DOWN:
			dir_packet.type = CS_DOWN;
			break;
		default:
			return 0;
		}
		dir_packet.size = sizeof(dir_packet);
		packet = reinterpret_cast<unsigned char *>(&dir_packet);
		memcpy(buffer, packet, packet[0]);
		wsabuf.len = dir_packet.size;

		if (WSASend(hSocket, &wsabuf, 1, (LPDWORD)&sendBytes, 0, NULL, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				ErrorHandling("WSASend() error");
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
		hdc = BeginPaint(hwnd, &ps);


		//좌표계변환
		if (isSet) {
			xPos = users[myID].getPos().x - RECTSIZE * 8;
			yPos = users[myID].getPos().y - RECTSIZE * 8;
		}
		

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
				Rectangle(hMemDC, users[i].getPos().x - xPos, users[i].getPos().y - yPos, users[i].getPos().x - xPos + RECTSIZE, users[i].getPos().y - yPos + RECTSIZE);
				wsprintf(str, TEXT("[%d] %d"), users[i].getID(), users[i].getHp());
				TextOut(hMemDC, users[i].getPos().x + 5 - xPos, users[i].getPos().y + 10 - yPos, str, lstrlen(str));
				SelectObject(hMemDC, oldPen);
				SelectObject(hMemDC, oldBrush);
				DeleteObject(myPen);
				DeleteObject(myBrush);
			}
		}

		//몬스터
		for (int i = MAX_USER; i < MAX_NPC - MAX_USER; i++) {
			if (monsters[i].is_using) {
				myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(hMemDC, myPen);
				myBrush = CreateSolidBrush(COLORREF(RGB(255, 0, 0)));
				oldBrush = (HBRUSH)SelectObject(hMemDC, myBrush);
				Ellipse(hMemDC, monsters[i].getPos().x - xPos, monsters[i].getPos().y - yPos, monsters[i].getPos().x - xPos + RECTSIZE, monsters[i].getPos().y - yPos + RECTSIZE);
				wsprintf(str, TEXT("[%d]"), monsters[i].getID());
				TextOut(hMemDC, monsters[i].getPos().x + 5 - xPos, monsters[i].getPos().y + 10 - yPos, str, lstrlen(str));
				SelectObject(hMemDC, oldPen);
				SelectObject(hMemDC, oldBrush);
				DeleteObject(myPen);
				DeleteObject(myBrush);
			}
		}


	
		wsprintf(str, TEXT("좌표 ( %d, %d ) "), users[myID].getPos().x/RECTSIZE, users[myID].getPos().y/ RECTSIZE);
		TextOut(hMemDC, winRect.right - 200, winRect.bottom-100, str, lstrlen(str));



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



DWORD WINAPI ThFunc(LPVOID lpParam)
{
	WSADATA wsaData;
	SOCKADDR_IN recvAddr;
	char packet[BUFSIZE];
	User* lpClientData;
	Monster* lpMonster;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		ErrorHandling("WSAStartup() error!");

	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = inet_addr(server_ip);
	recvAddr.sin_port = htons(atoi("2738"));

	if (connect(hSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	//이벤트 시작
	SetEvent(hStartupEvent);

	//전송할데이터
	flags = 0;
	while (true)
	{
		//사이즈 프로토콜 받아오기
		recvn(hSocket, packet, sizeof(BYTE)+sizeof(BYTE), flags);
		//나머지 데이터 받아오기
		recvn(hSocket, packet + sizeof(BYTE) + sizeof(BYTE), packet[0] - (sizeof(BYTE) + sizeof(BYTE)), flags);


		if (packet[1] == SC_MOVE_OBJECT) {
			PacketDispatcher::MoveObject(packet);

		}else if (packet[1] == SC_MONSTER_CHASE) {
			PacketDispatcher::MonsterChase(packet);
		}
		else if (packet[1] == SC_MONSTER_ATTACK) {
			PacketDispatcher::MonsterAttack(packet);
		}
		else if (packet[1] == SC_MONSTER_DIE) {
			PacketDispatcher::MonsterDie(packet);
		}
		else if (packet[1] == SC_PUT_OBJECT) {
			if (!isSet) {
				sc_packet_put_object put_packet;
				memcpy(&put_packet, packet, packet[0]);
				myID = put_packet.id;
				isSet = true;
			}
			PacketDispatcher::PutObject(packet);
		}
		else if (packet[1] == SC_REMOVE_OBJECT) {
			
			PacketDispatcher::RemoveObject(packet);
		}
	}
	closesocket(hSocket);
	WSACleanup();

	return 0;
}

void ErrorHandling(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags) {
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}
