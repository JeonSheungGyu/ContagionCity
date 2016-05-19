#pragma comment(lib, "ws2_32")
#include "stdafx.h"
#include "User.h"
#include "Monster.h"
#include "Protocol.h"
#include "PacketDispatcher.h"
#include "PacketSender.h"


COLORREF color[10] = { RGB(200,100,150),RGB(100,20,50),RGB(150,100,20),(200,120,0),RGB(0,200,200),
RGB(200,0,200),RGB(200,200,200),RGB(200,0,0),RGB(0,200,0),RGB(0,0,200)};

FuncProcess funcDispatcher[PACKET_TYPE];

std::vector<User> users(MAX_USER);
std::vector<Monster> monsters(MAX_NPC);

DWORD WINAPI ThFunc(LPVOID lpParam);
DWORD WINAPI UpdateThread(LPVOID lpParam); // ������ ������Ʈ
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int recvn(SOCKET s, char *buf, int len, int flags);
void ErrorHandling(char *msg);



//��������
HINSTANCE g_hInst;
LPCSTR lpszClass = TEXT("���Ӽ������α׷���_2��_2010180043_���±�");
HANDLE hStartupEvent;
HWND hHwnd, hDlg;
bool isSet = false;
int xPos = -RECTSIZE * 4, yPos = -RECTSIZE * 4;
char server_ip[100];
char id[ID_LEN];
char password[PASSWORD_LEN];

//��Ʈ��ũ ��������
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





BYTE LoginPermit();

void FuncInit()
{
	funcDispatcher[SC_PUT_OBJECT].Func = PacketDispatcher::PutObject;
	funcDispatcher[SC_REMOVE_OBJECT].Func = PacketDispatcher::RemoveObject;
	//�̱���
	//funcDispatcher[SC_LOGIN].Func = PacketDispatcher::Login;
	funcDispatcher[SC_MOVE_OBJECT].Func = PacketDispatcher::MoveObject;
	funcDispatcher[SC_COMBAT_OBJECT].Func = PacketDispatcher::ObjectCombat;
	funcDispatcher[SC_MONSTER_ATTACK].Func = PacketDispatcher::MonsterAttack;
	funcDispatcher[SC_MONSTER_CHASE].Func = PacketDispatcher::MonsterChase;
	funcDispatcher[SC_MONSTER_DIE].Func = PacketDispatcher::MonsterDie;
	//���λ���Ѵ�.
	//funcDispatcher[LC_PERMISION_LOGIN].Func = PacketDispatcher::PermisionLogin;
}

void initialize() {

	wsabuf.buf = reinterpret_cast<CHAR *>(buffer);
	wsabuf.len = BUFSIZE;

	FuncInit();
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
			GetDlgItemText(hDlg, IDC_EDIT2, id, 20);
			GetDlgItemText(hDlg, IDC_EDIT3, password, 20);

			//�α��� �������ؾ� �ȴ�.
			/*if ( LoginPermit() == 0) {
				EndDialog(hDlg, IDCANCEL);
				hDlg = NULL;
				PostQuitMessage(0);
				return TRUE;
			}*/


			hStartupEvent = CreateEvent(0, FALSE, FALSE, 0);
			//��Ž�����
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
	HDC hdc, hMemDC; // HDC�� �ϳ��� �������ش�. HDC�� '�׸��� �۾�' �̴�.
	HBITMAP hBitmap, OldBitmap; // HBITMAP�� �뷫 ���̸� �ǹ��Ѵ�. ���� 2�� ����
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
		SetRect(&clientRect, 0, 0, 800, 800 + RECTSIZE); //���ϴ� Ŭ���̾�Ʈ ũ�⸦ �����Ѵ�.
		AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE); //������ ũ�⸦ ���
		MoveWindow(hwnd, 0, 0,
			clientRect.right - clientRect.left,
			clientRect.bottom - clientRect.top,
			TRUE); //������ ũ�⸦ �ٲ��ְ� Ŭ���̾�Ʈ ������ ���� �׷��ش�.

		initialize();
		
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, DlgProc);
		
		SetTimer(hwnd, 1, 33, NULL);//Ÿ�̸�1
		break;
	case WM_KEYDOWN:
		if (users[myID].is_move) return 0;
		switch (wParam)
		{
		case VK_LEFT:
			dir = LEFT;
			users[myID].setDir(XMFLOAT2(-1, 0));
			users[myID].setDist(users[myID].getSpeed() / 33);
			users[myID].is_move = true;
			break;
		case VK_RIGHT:
			dir = RIGHT;
			users[myID].setDir(XMFLOAT2(1, 0));
			users[myID].setDist(users[myID].getSpeed() / 33);
			users[myID].is_move = true;
			break;
		case VK_UP:
			dir = UP;
			users[myID].setDir(XMFLOAT2(0, -1));
			users[myID].setDist(users[myID].getSpeed() / 33);
			users[myID].is_move = true;
			break;
		case VK_DOWN:
			dir = DOWN;
			users[myID].setDir(XMFLOAT2(0, 1));
			users[myID].setDist(users[myID].getSpeed() / 33);
			users[myID].is_move = true;
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
			
		return 0;
	case WM_TIMER:
		//������Ʈ
		for (auto& data : users)
			if (data.is_using) 
				data.update();
		for (auto& data : monsters)
			if (data.is_using) data.update();
		//������ �۾�
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);


		//��ǥ�躯ȯ
		if (isSet) {
			xPos = users[myID].getPos().x - RECTSIZE * 8;
			yPos = users[myID].getPos().y - RECTSIZE * 8;
		}
		

		hMemDC = CreateCompatibleDC(hdc); // hMemDC �� ���� DC (hdc)�� �°� �� DC ����
		hBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom); // crt �԰ݴ�� ���� ����
		OldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap); // ���� ��ü
		FillRect(hMemDC, &clientRect, (HBRUSH)GetStockObject(WHITE_BRUSH)); //��ȭ�� �� ����

		myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
		oldPen = (HPEN)SelectObject(hMemDC, myPen);
		wBrush = CreateSolidBrush(RGB(255, 255, 255));
		bBrush = CreateSolidBrush(RGB(0, 0, 0));
		oldBrush = (HBRUSH)SelectObject(hMemDC, wBrush);

		check = 0;
		//ü���� �׸���
		//for���� ������ ����ð��̴�.
		for (int i = 0; i < 100; i++) {
			for (int j = 0; j < 100; j++) {
				//����ǥ���� ( 8ĭ ) 
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

	
		
		//����
		for (int i = 0; i < MAX_USER; i++) {
			if (users[i].is_using) {
				myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(hMemDC, myPen);
				myBrush = CreateSolidBrush(color[users[i].getID()]);
				oldBrush = (HBRUSH)SelectObject(hdc, myBrush);
								
				Rectangle(hMemDC, users[i].getPos().x - xPos, users[i].getPos().y - yPos, users[i].getPos().x - xPos + RECTSIZE, users[i].getPos().y - yPos + RECTSIZE);

				SelectObject(hMemDC, oldPen);
				SelectObject(hMemDC, oldBrush);
				DeleteObject(myPen);
				DeleteObject(myBrush);


				myPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
				oldPen = (HPEN)SelectObject(hMemDC, myPen);

				//����׸���
				MoveToEx(hMemDC, users[i].getPos().x - xPos + RECTSIZE / 2, users[i].getPos().y - yPos + RECTSIZE / 2, NULL);
				LineTo(hMemDC, users[i].getPos().x - xPos + RECTSIZE / 2 + users[i].getDir().x*RECTSIZE,
					users[i].getPos().y - yPos + RECTSIZE / 2 + users[i].getDir().y*RECTSIZE);

				SelectObject(hMemDC, oldPen);
				DeleteObject(myPen);


				wsprintf(str, TEXT("[%d] %d"), users[i].getID(), users[i].getHp());
				TextOut(hMemDC, users[i].getPos().x  - xPos, users[i].getPos().y + 10 - yPos, str, lstrlen(str));
		
			}
		}

		//����
		for (int i = MAX_USER; i < MAX_NPC - MAX_USER; i++) {
			if (monsters[i].is_using) {
				myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(hMemDC, myPen);
				myBrush = CreateSolidBrush(COLORREF(RGB(255, 0, 0)));
				oldBrush = (HBRUSH)SelectObject(hMemDC, myBrush);
				Ellipse(hMemDC, monsters[i].getPos().x - xPos, monsters[i].getPos().y - yPos, monsters[i].getPos().x - xPos + RECTSIZE, monsters[i].getPos().y - yPos + RECTSIZE);
				wsprintf(str, TEXT("[%d] %d"), monsters[i].getID(), monsters[i].getHp());
				TextOut(hMemDC, monsters[i].getPos().x + 5 - xPos, monsters[i].getPos().y + 10 - yPos, str, lstrlen(str));
				SelectObject(hMemDC, oldPen);
				SelectObject(hMemDC, oldBrush);
				DeleteObject(myPen);
				DeleteObject(myBrush);
			}
		}


	
		wsprintf(str, TEXT("��ǥ ( %d, %d ) "),(DWORD)(users[myID].getPos().x/RECTSIZE), (DWORD)(users[myID].getPos().y / RECTSIZE));
		TextOut(hMemDC, winRect.right - 200, winRect.bottom-100, str, lstrlen(str));



		BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hMemDC, 0, 0, SRCCOPY); // �貸�׸���
																		 // hdc �� 0,0 ��ġ�� hMemDC�� 0,0��ġ���� crt.right,crt.bottom������ ����, �� crt������ �׸��� ��� �����ε�.

		DeleteObject(SelectObject(hMemDC, OldBitmap)); // ���� ������� �� �� ����
		DeleteDC(hMemDC); // hMemDC ����



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

BYTE LoginPermit() {
	WSADATA wsaData;
	SOCKADDR_IN recvAddr;
	char packet[BUFSIZE];

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		ErrorHandling("WSAStartup() error!");

	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = inet_addr(server_ip);
	recvAddr.sin_port = htons(LOGIN_PORT);

	if (connect(hSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	PacketSender::instance().requestLogin(id, password);
	//������Ŷ �޾ƿ���
	recvn(hSocket, packet, sizeof(BYTE) + sizeof(BYTE), flags);
	//������ ������ �޾ƿ���
	recvn(hSocket, packet + sizeof(BYTE) + sizeof(BYTE), packet[0] - (sizeof(BYTE) + sizeof(BYTE)), flags);

	closesocket(hSocket);
	WSACleanup();

	return PacketDispatcher::PermisionLogin(packet);
}
DWORD WINAPI ThFunc(LPVOID lpParam)
{
	WSADATA wsaData;
	SOCKADDR_IN recvAddr;
	HANDLE hThread;
	DWORD dwThreadID;

	char packet[BUFSIZE];

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		ErrorHandling("WSAStartup() error!");

	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = inet_addr(server_ip);
	recvAddr.sin_port = htons(GAME_PORT);

	if (connect(hSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	//�̺�Ʈ ����
	SetEvent(hStartupEvent);

	//�����ҵ�����
	flags = 0;
	while (true)
	{
		//������ �������� �޾ƿ���
		recvn(hSocket, packet, sizeof(BYTE)+sizeof(BYTE), flags);
		//������ ������ �޾ƿ���
		recvn(hSocket, packet + sizeof(BYTE) + sizeof(BYTE), packet[0] - (sizeof(BYTE) + sizeof(BYTE)), flags);


		if(packet[1] == SC_PUT_OBJECT) {
			if (!isSet) {
				sc_packet_put_object put_packet;
				memcpy(&put_packet, packet, packet[0]);
				myID = put_packet.id;
				isSet = true;

				//��ġ ������Ʈ ������
				hThread = CreateThread(NULL, 0, UpdateThread, NULL, 0, &dwThreadID);
				CloseHandle(hThread);
			}
		}
		funcDispatcher[*(packet + 1)].Func(packet);
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

// ����� ���� ������ ���� �Լ�
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
DWORD WINAPI UpdateThread(LPVOID lpParam)
{
	auto& owner = users[myID];
	auto oldPos = owner.getPos();
	auto start_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<long long, std::milli> count;
	while (1) {
		if (count.count() > UPDATE_TIME)
		{
			if (oldPos.x == owner.getPos().x && oldPos.y == owner.getPos().y) {
				Sleep(UPDATE_TIME);
			}
			else {
				oldPos = owner.getPos();
				PacketSender::instance().PlayerMove();
				start_time = std::chrono::high_resolution_clock::now();
				Sleep(UPDATE_TIME);
			}
		}
		count = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time);
	}
}