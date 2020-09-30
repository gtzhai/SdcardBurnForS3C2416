
// SDCardBurnDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "WriteSDCardThread.h"

#define WRITE_BLOCK_SIZE			(1024 * 1024)
#define READ_FILE_SIZE			(WRITE_BLOCK_SIZE * 1)




// CSDCardBurnDlg 对话框
class CSDCardBurnDlg : public CDialogEx
{
// 构造
public:
	CSDCardBurnDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SDCARDBURN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
public:
	UINT	m_TimerID;
	CWinThread* m_TFunMainRun;
	CWinThread* m_TFunWriteData;
	static UINT WINAPI MainRun(LPVOID lParam);
	static UINT WINAPI WriteData(LPVOID lParam);

	CComboBox m_CBDisk;
	CEdit m_CEFilePath;
	CButton m_BuSelFile;
	CButton m_BuBurn;
	CProgressCtrl m_CProgress;
	CStatic m_CSState;
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void ScanAllDisk(UINT flag);
	void ViewLastError(LPTSTR lpMsg);
	void ShowMessageBoxTimeout();
	CString m_ImgSrcDir;
	CString m_curDir;
	CString m_ImgDstDir;
	int CopyFolder(LPCTSTR lpszFromPath,LPCTSTR lpszToPath);
	afx_msg void OnBnClickedButtonSelfile();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonBurn();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
