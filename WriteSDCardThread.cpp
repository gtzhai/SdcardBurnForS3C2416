// WriteSDCardThread.cpp : 实现文件
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
	// TODO:    在此执行任意逐线程初始化
	TRACE(L"abc3\r\n");
	return TRUE;
}

int WriteSDCardThread::ExitInstance()
{
	// TODO:    在此执行任意逐线程清理
	TRACE(L"abcq4\r\n");
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(WriteSDCardThread, CWinThread)
END_MESSAGE_MAP()


// WriteSDCardThread 消息处理程序


int WriteSDCardThread::Run()
{
	// TODO: 在此添加专用代码和/或调用基类
	
	CString sdCardPath;
	BOOL bRtn;
	CHAR inBuffer[512];
	DWORD outSize;
	
	//创建磁盘句柄
	sdCardPath = L"\\\\.\\H:";
	m_SDCardHandle = CreateFile(sdCardPath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (m_SDCardHandle == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"open sd error", MB_OK | MB_ICONERROR);
		return 0;
	}
	//删除磁盘引导信息(扇区0有一个偏移信息，如果不清除，会导致写入的地址与想要的不一样)
	bRtn = DeviceIoControl(m_SDCardHandle, IOCTL_DISK_DELETE_DRIVE_LAYOUT, NULL, 0, NULL, 0, &outSize, NULL);
	if (bRtn==FALSE)
	{
		AfxMessageBox(L"删除引导信息失败");
		CloseHandle(m_SDCardHandle);
		return 0;
	}
	//清除扇区0
	memset(inBuffer, 0, sizeof(inBuffer));
	SetFilePointer(m_SDCardHandle, 0, 0, FILE_BEGIN);
	bRtn = WriteFile(m_SDCardHandle, inBuffer, WRITE_BLOCK_SIZE, &outSize, NULL);
	if (bRtn == FALSE)
	{
		AfxMessageBox(L"清除扇区0失败");
	}

	CloseHandle(m_SDCardHandle);
	AfxEndThread(0);
	return 0;
}
