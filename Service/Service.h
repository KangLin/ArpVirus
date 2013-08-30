/*!
\warning ��Ȩ���� (C) 2007 ���ֹ����ң���������Ȩ����
\file    Service.h
\brief   ����������
\version 1.0
\author  ��  �� (kl222@126.com)
\date    2007��5��31��  15:27:05
\par     ������:
         Microsoft Visual Studio 8.0 ���� C/C++ 8.0
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
