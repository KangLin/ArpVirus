/*!
\warning Copyright (C) 2007 KangLin Studio;All rights reserved.
\file    ServiceManage.cpp
\brief   服务管理类实现文件
\version 1.0
\author  KangLin (kl222@126.com)
\date    2007-6-5 13:40:14
\par     Compiler:
Microsoft Visual Studio 8.0 —— C/C++ 8.0
*/

#include "StdAfx.h"
#include "ServiceManage.h"
#include <tchar.h>

#ifndef ASSERT
#define ASSERT _ASSERT
#endif

CServiceManage::CServiceManage(LPCTSTR lpszName)
{
	m_hSchSCManager = NULL;
	SetServiceName(lpszName);
	SetManager();
}

CServiceManage::~CServiceManage(void)
{
	if(m_hSchSCManager)
	{
		CloseServiceHandle(m_hSchSCManager);
	} // 结束 if(m_hSchSCManager)
}

/*!
\brief   设置服务名
\param   LPCTSTR lpszName：服务名
\return  类型为 BOOL 。成功返回TRUE,否则返回FALSE
\version 1.0
\author  KangLin
\date    2007-6-5 13:41:22
*/
BOOL CServiceManage::SetServiceName(LPCTSTR lpszName)
{
	ASSERT(lpszName);
	if(NULL == lpszName)
	{
		return FALSE;
	} // 结束 if(NULL == lpszName)
	m_szServiceName = lpszName;
	return TRUE;
}

/*!
\brief   设置服务管理器
\param   LPCTSTR pszMachine：管理器的主机IP
\return  类型为 BOOL 。成功返回TURE,否则返回FALSE
\version 1.0
\author  KangLin
\date    2007-6-5 13:42:00
*/
BOOL CServiceManage::SetManager(LPCTSTR pszMachine)
{
	BOOL bRet = TRUE;
	if(m_hSchSCManager)
	{
		CloseServiceHandle(m_hSchSCManager);
	} // 结束 if(m_hSchSCManager)
	m_hSchSCManager = OpenSCManager(pszMachine, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_hSchSCManager == NULL)
	{
		bRet = FALSE;
	} // 结束 if(m_hSchSCManager == NULL)
	return bRet;
}


/*!
\brief   开始服务
\param   无
\return  类型为 BOOL 。成功返回TRUE,否则返回FALSE
\version 1.0
\author  KangLin
\date    2007-6-5 13:46:20
*/
BOOL CServiceManage::Start()
{
	SERVICE_STATUS_PROCESS ssStatus; 
	DWORD dwOldCheckPoint; 
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;
	SC_HANDLE schService;


	// Get a handle to the SCM database. 

	if(NULL == m_hSchSCManager)
	{
		m_hSchSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == m_hSchSCManager) 
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return FALSE;
	}

	}

	// Get a handle to the service.

	schService = OpenService( 
		m_hSchSCManager,         // SCM database 
		m_szServiceName.c_str(),            // name of service 
		SERVICE_ALL_ACCESS);  // full access 

	if (schService == NULL)
	{ 
		printf("OpenService failed (%d)\n", GetLastError()); 
		
		return FALSE;
	}    

	// Attempt to start the service.

	if (!StartService(
		schService,  // handle to service 
		0,           // number of arguments 
		NULL) )      // no arguments 
	{
		printf("StartService failed (%d)\n", GetLastError());
		CloseServiceHandle(schService); 
		return FALSE; 
	}
	else printf("Service start pending...\n"); 

	// Check the status until the service is no longer start pending. 

	if (!QueryServiceStatusEx( 
		schService,             // handle to service 
		SC_STATUS_PROCESS_INFO, // info level
		(LPBYTE) &ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded ) )              // if buffer too small
	{
		return FALSE; 
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
	{ 
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;

		Sleep( dwWaitTime );

		// Check the status again. 

		if (!QueryServiceStatusEx( 
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE) &ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded ) )              // if buffer too small
			break; 

		if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
		{
			// The service is making progress.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
			{
				// No progress made within the wait hint.
				break;
			}
		}
	} 

	// Determine whether the service is running

	if (ssStatus.dwCurrentState == SERVICE_RUNNING) 
	{
		printf("Service started successfully.\n"); 
	}
	else 
	{ 
		printf("Service not started. \n");
		printf("  Current State: %d\n", ssStatus.dwCurrentState); 
		printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode); 
		printf("  Check Point: %d\n", ssStatus.dwCheckPoint); 
		printf("  Wait Hint: %d\n", ssStatus.dwWaitHint); 
	} 

	CloseServiceHandle(schService);
	return TRUE;

}

/*!
\brief   停止服务
\param   无
\return  类型为 BOOL 。成功返回TRUE,否则返回FALSE
\version 1.0
\author  KangLin
\date    2007-6-5 13:46:42
*/
BOOL CServiceManage::Stop()
{
	BOOL bRet = FALSE;

	return bRet;
}

BOOL CServiceManage::QueryStatus(LPSERVICE_STATUS pStatus)
{
	BOOL bRet = FALSE;
	SC_HANDLE hService = ::OpenService(m_hSchSCManager, m_szServiceName.c_str(), SERVICE_ALL_ACCESS);
	if(NULL == hService)
	{
		return FALSE;
	} // 结束 if(NULL == hService)
	bRet = ::QueryServiceStatus(m_hSchSCManager, pStatus);
	::CloseServiceHandle(hService);
	return bRet;
}


/*!
\brief   安装指定的服务到服务管理器中
\param   LPCTSTR pszPath    ：服务执行文件的全路径
\param   DWORD dwServiceType：服务类型
\li SERVICE_FILE_SYSTEM_DRIVER
\li SERVICE_KERNEL_DRIVER
\li SERVICE_WIN32_OWN_PROCESS
\li SERVICE_WIN32_SHARE_PROCESS
\li SERVICE_INTERACTIVE_PROCESS
\param   DWORD dwStartType  ：开始类型
\li SERVICE_AUTO_START
\li SERVICE_BOOT_START
\li SERVICE_DEMAND_START
\li SERVICE_DISABLED
\li SERVICE_SYSTEM_START
\return  类型为 BOOL 。成功返回TRUE,否则返回FALSE.
\version 1.0
\author  康  林
\date    2007-6-12 21:03:45
*/
BOOL CServiceManage::Install(LPCTSTR pszPath /* = NULL */, LPTSTR pszServerName /* = NULL */, LPCTSTR pszDisplayName /* = NULL */)
{

	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

	if(NULL == pszPath)
	{
		if(!GetModuleFileName(NULL, szPath, MAX_PATH))
		{
			printf("Cannot install service (%d)\n", GetLastError());
			return FALSE;
		}
	}
	else
	{
		_tcscpy(szPath, pszPath);
	}
	// Get a handle to the SCM database. 

	if(NULL == m_hSchSCManager)
	{
		m_hSchSCManager = OpenSCManager(
			NULL,                    // local computer
			NULL,                    // ServicesActive database 
			SC_MANAGER_ALL_ACCESS);  // full access rights 

		if (NULL == m_hSchSCManager) 
		{
			printf("OpenSCManager failed (%d)\n", GetLastError());
			return FALSE;
		}
	}

	if(NULL == pszServerName)
	{
		pszServerName =(LPTSTR) m_szServiceName.c_str();
	} // 结束 if(NULL == pszServerName)

	// Create the service.

	schService = CreateService( 
		m_hSchSCManager,              // SCM database 
		pszServerName,                   // name of service 
		pszDisplayName,                   // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_AUTO_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		szPath,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL) 
	{
		printf("CreateService failed (%d)\n", GetLastError()); 
		return FALSE;
	}
	else printf("Service installed successfully\n"); 

	CloseServiceHandle(schService); 

}

/*!
\brief   从服务管理器中移除服务
\param   无
\return  类型为 BOOL 。成功返回TRUE,否则返回FALSE
\version 1.0
\author  KangLin
\date    2007-6-5 13:43:31
*/
BOOL CServiceManage::Remove(LPCTSTR pszServerName)
{

	SC_HANDLE schService;
	SERVICE_STATUS ssStatus; 

	// Get a handle to the SCM database. 

	if(NULL == m_hSchSCManager)
	{
		m_hSchSCManager = OpenSCManager(
			NULL,                    // local computer
			NULL,                    // ServicesActive database 
			SC_MANAGER_ALL_ACCESS);  // full access rights 

		if (NULL == m_hSchSCManager) 
		{
			printf("OpenSCManager failed (%d)\n", GetLastError());
			return FALSE;
		}
	}

	if(NULL == pszServerName)
	{
		pszServerName = m_szServiceName.c_str();
	} // 结束 if(NULL == pszServerName)

	// Get a handle to the service.

	schService = OpenService( 
		m_hSchSCManager,       // SCM database 
		pszServerName,          // name of service 
		DELETE);            // need delete access 

	if (schService == NULL)
	{ 
		printf("OpenService failed (%d)\n", GetLastError()); 
		//CloseServiceHandle(m_hSchSCManager);
		return FALSE;
	}

	// Delete the service.

	if (! DeleteService(schService) ) 
	{
		printf("DeleteService failed (%d)\n", GetLastError()); 
	}
	else printf("Service deleted successfully\n"); 

	CloseServiceHandle(schService); 

}
