
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

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	ACE::init();
	int nRetCode = 0;

	if(nRetCode < 0)
		return nRetCode;

	do 
	{
		if(2 == argc)
		{
			if( _tcsicmp( argv[1], TEXT("-install")) == 0 )
			{
				CServiceManage sm(_T("Service"));
				TCHAR szFileName[1024];
				int iFileLen = 1024;
				int iLen = ::GetModuleFileName(NULL, szFileName, iFileLen);
				if(!iLen)
				{
					printf(("得到当前执行文件名错误.\n"));
					nRetCode = -1;
					break;
				}
				sm.Install(szFileName, _T("Service"), _T("Windows Service"));
				nRetCode = 0;
				CARPVirusService s;
				s.start();
				break;
			}
			else if(_tcsicmp(argv[1], TEXT("-uninstall")) == 0)
			{
				CServiceManage sm(_T("Service"));
				sm.Remove();
				nRetCode = 0;
				break;
			}
			else if(_tcsicmp(argv[1], TEXT("-help")) == 0)
			{
				_tprintf(_T("%s [option]:\n"), argv[0]);
				_tprintf(_T("   option:\n"));
				_tprintf(_T("         -install:install server\n"));
				_tprintf(_T("         -delete:delete server\n"));
				_tprintf(_T("         -help:help\n"));
				nRetCode = 0;
				break;
			}// 结束 if(lstrcmpi(argv[1], TEXT("-help")) == 0) 
		}
		else
		{
			CARPVirusService s;
			s.start();
		}	
	} while (0);

	ACE::fini();
	return nRetCode;
}
