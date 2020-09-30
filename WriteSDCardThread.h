#pragma once



// WriteSDCardThread



class WriteSDCardThread : public CWinThread
{
	DECLARE_DYNCREATE(WriteSDCardThread)

protected:
	WriteSDCardThread();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~WriteSDCardThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual int Run();

public:
	CString m_OutFilePath;
	int m_Progress;
	CFile m_SrcFile;
	HANDLE m_SDCardHandle;
};


