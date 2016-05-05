
// LoginProgramDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"


// CLoginProgramDlg 대화 상자
class CLoginProgramDlg : public CDialogEx
{
// 생성입니다.
public:
	CLoginProgramDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_LOGINPROGRAM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
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
