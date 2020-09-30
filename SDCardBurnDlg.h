
// SDCardBurnDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "WriteSDCardThread.h"

#define WRITE_BLOCK_SIZE			(1024 * 1024)
#define READ_FILE_SIZE			(WRITE_BLOCK_SIZE * 1)




// CSDCardBurnDlg �Ի���
class CSDCardBurnDlg : public CDialogEx
{
// ����
public:
	CSDCardBurnDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SDCARDBURN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
