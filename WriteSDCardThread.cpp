// WriteSDCardThread.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SDCardBurn.h"
#include "WriteSDCardThread.h"
#include "SDCardBurnDlg.h"

#include <Windows.h>
#include <winioctl.h>

// WriteSDCardThread

IMPLEMENT_DYNCREATE(WriteSDCardThread, CWinThread)

WriteSDCardThread::WriteSDCardThread()
{
	TRACE(L"abcq1\r\n");
}

WriteSDCardThread::~WriteSDCardThread()
{
	TRACE(L"abcq2\r\n");
}

BOOL WriteSDCardThread::InitInstance()
{
	// TODO:    �ڴ�ִ���������̳߳�ʼ��
	TRACE(L"abc3\r\n");
	return TRUE;
}

int WriteSDCardThread::ExitInstance()
{
	// TODO:    �ڴ�ִ���������߳�����
	TRACE(L"abcq4\r\n");
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(WriteSDCardThread, CWinThread)
END_MESSAGE_MAP()


// WriteSDCardThread ��Ϣ�������


int WriteSDCardThread::Run()
{
	// TODO: �ڴ����ר�ô����/����û���
	
	CString sdCardPath;
	BOOL bRtn;
	CHAR inBuffer[512];
	DWORD outSize;
	
	//�������̾��
	sdCardPath = L"\\\\.\\H:";
	m_SDCardHandle = CreateFile(sdCardPath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (m_SDCardHandle == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"open sd error", MB_OK | MB_ICONERROR);
		return 0;
	}
	//ɾ������������Ϣ(����0��һ��ƫ����Ϣ�������������ᵼ��д��ĵ�ַ����Ҫ�Ĳ�һ��)
	bRtn = DeviceIoControl(m_SDCardHandle, IOCTL_DISK_DELETE_DRIVE_LAYOUT, NULL, 0, NULL, 0, &outSize, NULL);
	if (bRtn==FALSE)
	{
		AfxMessageBox(L"ɾ��������Ϣʧ��");
		CloseHandle(m_SDCardHandle);
		return 0;
	}
	//�������0
	memset(inBuffer, 0, sizeof(inBuffer));
	SetFilePointer(m_SDCardHandle, 0, 0, FILE_BEGIN);
	bRtn = WriteFile(m_SDCardHandle, inBuffer, WRITE_BLOCK_SIZE, &outSize, NULL);
	if (bRtn == FALSE)
	{
		AfxMessageBox(L"�������0ʧ��");
	}

	CloseHandle(m_SDCardHandle);
	AfxEndThread(0);
	return 0;
}
