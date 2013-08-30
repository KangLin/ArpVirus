/*!
\warning Copyright (C) 2007 KangLin Studio;All rights reserved.
\file    ServiceManage.h
\brief   服务管理类头文件
\version 1.0
\author  KangLin (kl222@126.com)
\date    2007-6-5 13:40:37
\par     Compiler:
         Microsoft Visual Studio 8.0 —— C/C++ 8.0
*/

#pragma once

#include <Windows.h>
#include <Winsvc.h>
#include <string>
using namespace std;

#ifdef UNICODE
#define _tstring wstring
#else
#define _tstring string
#endif

class CServiceManage
{
public:
	CServiceManage(LPCTSTR lpszName);
	~CServiceManage(void);

	BOOL SetServiceName(LPCTSTR lpszName);
	BOOL SetManager(LPCTSTR pszMachine = NULL);

	BOOL Install(LPCTSTR pszPath = NULL, LPTSTR pszServerName = NULL, LPCTSTR pszDisplayName = NULL);
	BOOL Remove(LPCTSTR pszServerName = NULL);

	BOOL Start();
	BOOL Stop();

	BOOL QueryStatus(LPSERVICE_STATUS pStatus);

private:
	SC_HANDLE m_hSchSCManager;
	_tstring m_szServiceName;
};
