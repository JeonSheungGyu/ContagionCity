#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <vector>
#include <algorithm>
#include "resource.h"
#include "Protocol.h"
#define BUFSIZE 1024
#define RECTSIZE 40
#define INTERVAL 4
#define MAX_USER 10

typedef struct //소켓정보를구조체화.
{
	bool isConnected;
	DWORD id;
	DWORD x, y, z;
} CLIENT, *LPCLIENT;

typedef struct //소켓정보를구조체화.
{
	DWORD id;
	DWORD x, y, z;
} MONSTER, *LPMONSTER;

COLORREF color[10] = { RGB(200,100,150),RGB(100,20,50),RGB(150,100,20),(200,120,0),RGB(0,200,200),
RGB(200,0,200),RGB(200,200,200),RGB(200,0,0),RGB(0,200,0),RGB(0,0,200)};


CLIENT users[MAX_USER];
std::vector<LPMONSTER> monsters;

DWORD WINAPI ThFunc(LPVOID lpParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int recvn(SOCKET s, char *buf, int len, int flags);
void ErrorHandling(char *msg);



//전역변수
HINSTANCE g_hInst;
LPCSTR lpszClass = TEXT("게임서버프로그래밍_2반_2010180043_전승규");
HANDLE hStartupEvent;
HWND hHwnd, hDlg;
CLIENT MyData;
bool isSet = false;
int xPos = -RECTSIZE * 4, zPos = -RECTSIZE * 4;
char server_ip[100];
//네트워크 전역변수
WSABUF wsabuf;
CHAR buffer[BUFSIZE] = { 0, };
SOCKET hSocket;
WSAEVENT event;
WSAOVERLAPPED overlapped;
RECT clientRect;


int sendBytes = 0;
int recvBytes = 0;
int flags = 0;

void initialize() {

	wsabuf.buf = reinterpret_cast<CHAR *>(buffer);
	wsabuf.len = BUFSIZE;

	memset(&MyData, 0, sizeof(MyData));
	memset(users, 0, sizeof(users));

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

	while (GetMessage(&msg, NULL, 0, 0))
	{
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
	HDC hdc;
	HPEN myPen, oldPen;
	HBRUSH myBrush, oldBrush, wBrush, bBrush;
	static BOOL bMove = FALSE;
	int check = 0;
	RECT winRect;
	TCHAR str[50];
	GetWindowRect(hwnd, &winRect);
	cs_packet_dir dir_packet;
	unsigned char *packet;
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
		break;
	case WM_KEYDOWN:
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
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
		oldPen = (HPEN)SelectObject(hdc, myPen);
		wBrush = CreateSolidBrush(RGB(255, 255, 255));
		bBrush = CreateSolidBrush(RGB(0, 0, 0));
		oldBrush = (HBRUSH)SelectObject(hdc, wBrush);

		check = 0;
		//체스판 그리기
		//for문이 많지만 상수시간이다.
		for (int i = 0; i < 100; i++) {
			for (int j = 0; j < 100; j++) {
				//간격표시자 ( 8칸 ) 
				if ( (check + j ) %2 == 0 )
					(HBRUSH)SelectObject(hdc, bBrush);
				else
					(HBRUSH)SelectObject(hdc, wBrush);

				for (int z = 0; z < INTERVAL; z++) {
					for (int x = 0; x < INTERVAL; x++) {
						if (clientRect.left < (RECTSIZE*INTERVAL*j + x*RECTSIZE) - xPos && (RECTSIZE*INTERVAL*j + x*RECTSIZE) - xPos < clientRect.right &&
							clientRect.top < (RECTSIZE * INTERVAL * i + z*RECTSIZE) - zPos && (RECTSIZE * INTERVAL * i + z*RECTSIZE) - zPos < clientRect.bottom) {
							Rectangle(hdc, (RECTSIZE*INTERVAL*j + x*RECTSIZE) - xPos, (RECTSIZE * INTERVAL * i + z*RECTSIZE) - zPos,
								(RECTSIZE * INTERVAL * j + x*RECTSIZE) - xPos + RECTSIZE, (RECTSIZE * INTERVAL * i + z*RECTSIZE) - zPos + RECTSIZE);
						}
					
					}
				}
			}
			check = 1 - check;
		}
		SelectObject(hdc, oldPen);
		SelectObject(hdc, oldBrush);
		DeleteObject(myPen);
		DeleteObject(wBrush);
		DeleteObject(bBrush);

		//시야에 존재하는 Object만 그리므로 따로 필터링할 필요가 없다.
		//플레이어 그리기
		if (isSet) {
			myPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
			oldPen = (HPEN)SelectObject(hdc, myPen);
			myBrush = CreateSolidBrush(color[MyData.id]);
			oldBrush = (HBRUSH)SelectObject(hdc, myBrush);
			Rectangle(hdc, MyData.x - xPos, MyData.z - zPos, MyData.x - xPos + RECTSIZE, MyData.z - zPos + RECTSIZE);
			wsprintf(str, TEXT("[%d] %d"), MyData.id, MyData.y);
			TextOut(hdc, MyData.x + 5 - xPos, MyData.z + 10 - zPos, str, lstrlen(str));
			SelectObject(hdc, oldPen);
			SelectObject(hdc, oldBrush);
			DeleteObject(myPen);
			DeleteObject(myBrush);
		}
	
		
		//다른 유저 그리기
		for (int i = 0; i < MAX_USER; i++) {
			if (users[i].isConnected) {
				myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(hdc, myPen);
				myBrush = CreateSolidBrush(color[users[i].id]);
				oldBrush = (HBRUSH)SelectObject(hdc, myBrush);
				Rectangle(hdc, users[i].x - xPos, users[i].z - zPos, users[i].x - xPos + RECTSIZE, users[i].z - zPos + RECTSIZE);
				wsprintf(str, TEXT("[%d] %d"), users[i].id, users[i].y);
				TextOut(hdc, users[i].x + 5 - xPos, users[i].z + 10 - zPos, str, lstrlen(str));
				SelectObject(hdc, oldPen);
				SelectObject(hdc, oldBrush);
				DeleteObject(myPen);
				DeleteObject(myBrush);
			}
		}

		//몬스터
		for (auto iter = monsters.begin(); iter != monsters.end(); iter++) {
			
			myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
			oldPen = (HPEN)SelectObject(hdc, myPen);
			myBrush = CreateSolidBrush(COLORREF(RGB(255,0,0)));
			oldBrush = (HBRUSH)SelectObject(hdc, myBrush);
			Ellipse(hdc, (*iter)->x - xPos, (*iter)->z - zPos, (*iter)->x - xPos + RECTSIZE, (*iter)->z - zPos + RECTSIZE);
			wsprintf(str, TEXT("[%d] %d"), (*iter)->id, (*iter)->y);
			TextOut(hdc, (*iter)->x + 5 - xPos, (*iter)->z + 10 - zPos, str, lstrlen(str));
			SelectObject(hdc, oldPen);
			SelectObject(hdc, oldBrush);
			DeleteObject(myPen);
			DeleteObject(myBrush);
		}


	
		wsprintf(str, TEXT("좌표 ( %d, %d ) "), MyData.x/RECTSIZE, MyData.z/ RECTSIZE);
		TextOut(hdc, winRect.right - 200, winRect.bottom-100, str, lstrlen(str));

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
	LPCLIENT lpClientData;
	LPMONSTER lpMonster;
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


		if (packet[1] == SC_POS) {
			printf("Recv :SC_POS \n");
			sc_packet_pos pos_packet;
			memcpy(&pos_packet, packet, packet[0]);

			if (pos_packet.id == MyData.id) {
				//기준이 되는곳
				MyData.x = pos_packet.x;
				MyData.y = pos_packet.y;
				MyData.z = pos_packet.z;
				xPos = MyData.x - RECTSIZE * 10;
				zPos = MyData.z - RECTSIZE * 10;
			}
			else {
				if (pos_packet.id < MAX_USER) {
					users[pos_packet.id].x = pos_packet.x;
					users[pos_packet.id].y = pos_packet.y;
					users[pos_packet.id].z = pos_packet.z;
				}
				else {
					auto iter = find_if(monsters.begin(), monsters.end(), [=](LPMONSTER m) {
						if (m->id == pos_packet.id) {
							return true;
						}
						return false;
					});
					(*iter)->x = pos_packet.x;
					(*iter)->y = pos_packet.y;
					(*iter)->z = pos_packet.z;
				}
			
			}
			
		}
		else if (packet[1] == SC_PUT_OBJECT) {
			printf("Recv :SC_PUT_PLAYER \n");
			sc_packet_put_object put_packet;
			memcpy(&put_packet, packet, packet[0]);
			if (!isSet) {
				MyData.id = put_packet.id;
				MyData.x = put_packet.x;
				MyData.y = put_packet.y;
				MyData.z = put_packet.z;
				xPos = MyData.x - RECTSIZE * 10;
				zPos = MyData.z - RECTSIZE * 10;
				isSet = true;
			}
			else {
				if (put_packet.id < MAX_USER) {
					users[put_packet.id].isConnected = true;
					users[put_packet.id].id = put_packet.id;
					users[put_packet.id].x = put_packet.x;
					users[put_packet.id].y = put_packet.y;
					users[put_packet.id].z = put_packet.z;
				}
				else {
					lpMonster = new MONSTER();
					lpMonster->id = put_packet.id;
					lpMonster->x = put_packet.x;
					lpMonster->y = put_packet.y;
					lpMonster->z = put_packet.z;
					monsters.push_back(lpMonster);
				}
			
			}
		}
		else if (packet[1] == SC_REMOVE_OBJECT) {
			printf("Recv :SC_REMOVE_PLAYER \n");
			sc_packet_remove_object remove_packet;
			memcpy(&remove_packet, packet, packet[0]);
			if (remove_packet.id < MAX_USER) {
				users[remove_packet.id].isConnected = false;
			}
			else {
				auto iter = find_if(monsters.begin(), monsters.end(), [=](LPMONSTER m) {
						if (m->id == remove_packet.id) {
							return true;
						}
						return false;
				});

				//패킷이 중복으로 올 수 도 있다.
				if (iter != monsters.end()) {
					monsters.erase(iter);
				}
			}
		}
		InvalidateRect(hHwnd, NULL, TRUE);
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
