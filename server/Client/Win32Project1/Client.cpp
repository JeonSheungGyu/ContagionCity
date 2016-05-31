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

//��������
HINSTANCE g_hInst;
LPCSTR lpszClass = TEXT("���Ӽ������α׷���_2��_2010180043_���±�");
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

			//���Ž����� 
			Server::DispatcherInit();
			Server::receiveThread = new thread{ Server::ReceiveThread };
			
			//�α��μ�������
			if (!Server::LoginServerConnection(id, password, server_ip)) {
				closesocket(Server::sock);
				EndDialog(hDlg, IDCANCEL);
				hDlg = NULL;
				PostQuitMessage(0);
				return TRUE;
			}
			//���Ӽ�������
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

		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, DlgProc);
		
		SetTimer(hwnd, 1, 33, NULL);//Ÿ�̸�1
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
		//������ �ȵǾ��� ��� 
		if (Server::getClientID() == -1) {
			//not connected
			hdc = BeginPaint(hwnd, &ps);
			wsprintf(str, TEXT("���Ӽ��� (%s)  ������ ���� �ʾҽ��ϴ�."), server_ip);
			TextOut(hdc, clientRect.right / 2 - lstrlen(str)*4, clientRect.bottom / 2, str, lstrlen(str));
			EndPaint(hwnd, &ps);
			return 0;
		}

		xPos = users[Server::getClientID()].getPos().x - RECTSIZE * 8;
		yPos = users[Server::getClientID()].getPos().y - RECTSIZE * 8;
		
		hdc = BeginPaint(hwnd, &ps);
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
								
				Rectangle(hMemDC, users[i].getPos().x - xPos - RECTSIZE / 2, users[i].getPos().y - yPos - RECTSIZE / 2, 
					users[i].getPos().x - xPos + RECTSIZE/2, users[i].getPos().y - yPos + RECTSIZE/2);

				SelectObject(hMemDC, oldPen);
				SelectObject(hMemDC, oldBrush);
				DeleteObject(myPen);
				DeleteObject(myBrush);


				myPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
				oldPen = (HPEN)SelectObject(hMemDC, myPen);

				//����׸���
				MoveToEx(hMemDC, users[i].getPos().x - xPos, users[i].getPos().y - yPos, NULL);
				LineTo(hMemDC, users[i].getPos().x - xPos + users[i].getDir().x*RECTSIZE,
					users[i].getPos().y - yPos + users[i].getDir().y*RECTSIZE);

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
				Ellipse(hMemDC, monsters[i].getPos().x - xPos - RECTSIZE / 2, monsters[i].getPos().y - yPos - RECTSIZE / 2, 
					monsters[i].getPos().x - xPos + RECTSIZE / 2, monsters[i].getPos().y - yPos + RECTSIZE / 2);
				wsprintf(str, TEXT("[%d] %d"), monsters[i].getID(), monsters[i].getHp());
				TextOut(hMemDC, monsters[i].getPos().x + 5 - xPos, monsters[i].getPos().y + 10 - yPos, str, lstrlen(str));
				SelectObject(hMemDC, oldPen);
				SelectObject(hMemDC, oldBrush);
				DeleteObject(myPen);
				DeleteObject(myBrush);

				
				if (isDrawTarget) {
					//Ÿ�ٱ׸���
					myPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 255));
					oldPen = (HPEN)SelectObject(hMemDC, myPen);

					
					MoveToEx(hMemDC, monsters[i].getPos().x - xPos, monsters[i].getPos().y - yPos, NULL);
					LineTo(hMemDC, monsters[i].getTargetPos().x - xPos, monsters[i].getTargetPos().y - yPos);

					SelectObject(hMemDC, oldPen);
					DeleteObject(myPen);
				}
				if (isDrawServerPos) {
					//������ġ �׸���
					myPen = CreatePen(PS_SOLID, 5, RGB(0, 255, 0));
					oldPen = (HPEN)SelectObject(hMemDC, myPen);

					
					MoveToEx(hMemDC, monsters[i].getPos().x - xPos, monsters[i].getPos().y - yPos, NULL);
					LineTo(hMemDC, monsters[i].getServerPos().x - xPos, monsters[i].getServerPos().y - yPos);

					SelectObject(hMemDC, oldPen);
					DeleteObject(myPen);
				}
			}
		}


	
		wsprintf(str, TEXT("��ǥ ( %d, %d ) "),(DWORD)(users[Server::getClientID()].getPos().x), (DWORD)(users[Server::getClientID()].getPos().y ));
		TextOut(hMemDC, clientRect.right - 200, clientRect.bottom-100, str, lstrlen(str));



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