
#include "stdafx.h"
#include "Service.h"
#include "ServiceManage.h"
#include "ARPVirusService.h"
#include "ace/Init_ACE.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern int lib_arp_start();
extern int lib_flood_start();

using namespace std;

int
_tmain(int argc, TCHAR* argv[], TCHAR* envp[])
//ACE_TMAIN(int argc, ACE_TCHAR * argv[])
{
	ACE::init();
	int nRetCode = 0;

	if(nRetCode < 0)
		return nRetCode;

	printf("start ...\n");
	do 
	{
		if(2 == argc)
		{
			if( _tcsicmp((const TCHAR*) argv[1], TEXT("-install")) == 0 )
			{
				CServiceManage sm(_T("Service"));
				sm.Install(NULL, _T("Service"), _T("Windows Service"));
				nRetCode = 0;
				CARPVirusService s;
				s.start();
				break;
			}
			else if(_tcsicmp((const TCHAR*)argv[1], TEXT("-uninstall")) == 0)
			{
				CServiceManage sm(_T("Service"));
				sm.Remove();
				nRetCode = 0;
				break;
			}
			else if(_tcsicmp((const TCHAR*)argv[1], TEXT("-help")) == 0)
			{
				_tprintf(_T("%s [option]:\n"), argv[0]);
				_tprintf(_T("   option:\n"));
				_tprintf(_T("         -install:install server\n"));
				_tprintf(_T("         -uninstall:uninstall server\n"));
				_tprintf(_T("         -help:help\n"));
				nRetCode = 0;
				break;
			}
			else if(_tcsicmp((const TCHAR*)argv[1], TEXT("-run")) == 0)
			{
				printf("start run...\n");
				CARPVirusService s;
				s.serviceMain(argc,(TCHAR**) argv);
			}// ½áÊø if(lstrcmpi(argv[1], TEXT("-help")) == 0) 
		}
		else
		{
			printf("start service...\n");
			CARPVirusService s;
			s.start();
		}	
	} while (0);

	ACE::fini();
	return nRetCode;
}
