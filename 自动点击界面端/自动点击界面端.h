
// �Զ���������.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// C�Զ���������App: 
// �йش����ʵ�֣������ �Զ���������.cpp
//

class C�Զ���������App : public CWinApp
{
public:
	C�Զ���������App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern C�Զ���������App theApp;