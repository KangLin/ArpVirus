/*!
\warning 版权所有 (C) 2007 康林工作室；保留所有权利。
\file    Service.h
\brief   服务程序基类
\version 1.0
\author  康  林 (kl222@126.com)
\date    2007年5月31日  15:27:05
\par     编译器:
         Microsoft Visual Studio 8.0 —— C/C++ 8.0
*/
#pragma once

#include <WinSvc.h>

class CService
{
public:
	CService(const TCHAR* name_);
public:
	~CService(void);

	const TCHAR* getName() {return name;}
	SERVICE_STATUS& getStatus() {return status;}

	void setStatus(DWORD status);
	void setStatus();

	// - Start the service, having initialised it
	void start();

	// - Service main procedure - override to implement a service
	virtual DWORD serviceMain(int argc, TCHAR* argv[]) = 0;

	// - Service control notifications

	// To get notified when the OS is shutting down
	virtual void osShuttingDown() {};

	// To get notified when the service parameters change
	virtual void readParams() {};

	// To cause the serviceMain() routine to return
	virtual void stop() {};

public:
	SERVICE_STATUS_HANDLE status_handle;
	SERVICE_STATUS status;
protected:
	const TCHAR* name;
};
