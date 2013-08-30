#include "StdAfx.h"
#include "Service.h"


// - Internal service implementation functions

CService* service = 0;

VOID WINAPI serviceHandler(DWORD control) {
	switch (control) {
  case SERVICE_CONTROL_INTERROGATE:
	  TRACE(_T("cmd: report status\n"));
	  service->setStatus();
	  return;
  case SERVICE_CONTROL_PARAMCHANGE:
	  TRACE(_T("cmd: param change\n"));
	  service->readParams();
	  return;
  case SERVICE_CONTROL_SHUTDOWN:
	  TRACE(_T("cmd: OS shutdown\n"));
	  service->osShuttingDown();
	  return;
  case SERVICE_CONTROL_STOP:
	  TRACE(_T("cmd: stop\n"));
	  service->setStatus(SERVICE_STOP_PENDING);
	  service->stop();
	  return;
	};
	TRACE(_T("cmd: unknown %lu\n"), control);
}

// -=- Service main procedure, used under WinNT/2K/XP by the SCM

VOID WINAPI serviceProc(DWORD dwArgc, LPTSTR* lpszArgv) {
	TRACE(_T("entering %s serviceProc\n"), service->getName());
	TRACE(_T("registering handler...\n"));
	service->status_handle = RegisterServiceCtrlHandler(service->getName(), serviceHandler);
	if (!service->status_handle) {
		DWORD err = GetLastError();
		TRACE(_T("failed to register handler: %lu\n"), err);
		ExitProcess(err);
	}
	TRACE(_T("registered handler (%lx)\n"), service->status_handle);
	service->setStatus(SERVICE_START_PENDING);
	TRACE(_T("entering %s serviceMain\n"), service->getName());
	service->status.dwWin32ExitCode = service->serviceMain(dwArgc, lpszArgv);
	TRACE(_T("leaving %s serviceMain\n"), service->getName());
	service->setStatus(SERVICE_STOPPED);
}

// -=- CService

CService::CService(const TCHAR* name_) : name(name_) {
	TRACE(_T("CService\n"));
	status_handle = 0;
	status.dwControlsAccepted = SERVICE_CONTROL_INTERROGATE | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
	status.dwWin32ExitCode = NO_ERROR;
	status.dwServiceSpecificExitCode = 0;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 30000;
	status.dwCurrentState = SERVICE_STOPPED;
}

void
CService::start() {
	
		SERVICE_TABLE_ENTRY entry[2];
		entry[0].lpServiceName = (TCHAR*)name;
		entry[0].lpServiceProc = serviceProc;
		entry[1].lpServiceName = NULL;
		entry[1].lpServiceProc = NULL;
		TRACE(_T("entering dispatcher\n"));
		if (!SetProcessShutdownParameters(0x100, 0))
			TRACE(_T("unable to set shutdown parameters: %d\n"), GetLastError());
		service = this;
		if (!StartServiceCtrlDispatcher(entry))
			return;
}

void
CService::setStatus() {
	setStatus(status.dwCurrentState);
}

void
CService::setStatus(DWORD state) {
	
	if (status_handle == 0) {
		TRACE(_T("warning - cannot setStatus\n"));
		return;
	}
	status.dwCurrentState = state;
	status.dwCheckPoint++;
	if (!SetServiceStatus(status_handle, &status)) {
		status.dwCurrentState = SERVICE_STOPPED;
		status.dwWin32ExitCode = GetLastError();
		TRACE(_T("unable to set service status:%u\n"), status.dwWin32ExitCode);
	}
	TRACE(_T("set status to %u(%u)\n"), state, status.dwCheckPoint);
}

CService::~CService() {
	TRACE(_T("~CService\n"));
	service = 0;
}


