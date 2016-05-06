
// LoginProgramDlg.h : ��� ����
//

#pragma once
#include "afxwin.h"


// CLoginProgramDlg ��ȭ ����
class CLoginProgramDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CLoginProgramDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_LOGINPROGRAM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editID;
	CEdit m_editPW;
	afx_msg void OnBnClickedPlay( );
	afx_msg void OnBnClickedExit( );
	afx_msg void OnBnClickedButtonLogin( );
	bool m_bLogined;
};
