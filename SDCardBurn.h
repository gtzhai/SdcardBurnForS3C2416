
// SDCardBurn.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSDCardBurnApp: 
// �йش����ʵ�֣������ SDCardBurn.cpp
//

class CSDCardBurnApp : public CWinApp
{
public:
	CSDCardBurnApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSDCardBurnApp theApp;