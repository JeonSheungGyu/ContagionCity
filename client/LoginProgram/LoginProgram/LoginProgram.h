
// LoginProgram.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CLoginProgramApp:
// �� Ŭ������ ������ ���ؼ��� LoginProgram.cpp�� �����Ͻʽÿ�.
//

class CLoginProgramApp : public CWinApp
{
public:
	CLoginProgramApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CLoginProgramApp theApp;