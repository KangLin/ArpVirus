
#include "stdafx.h"
#include "Service.h"
#include "ServiceManage.h"
#include "ARPVirusService.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ψһ��Ӧ�ó������

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: ���Ĵ�������Է���������Ҫ
		_tprintf(_T("����: MFC ��ʼ��ʧ��\n"));
		nRetCode = 1;
	}
	else
	{
		if( lstrcmpi( argv[1], TEXT("-install")) == 0 )
		{
			CServiceManage sm(_T("Service"));
			sm.Install(NULL, _T("Service"), _T("Windows Service"));
			return 0;;
		}
		else if(lstrcmpi(argv[1], TEXT("-delete")) == 0)
		{
			CServiceManage sm(_T("Service"));
			sm.Remove();
			return 0;
		}
		else if(lstrcmpi(argv[1], TEXT("-help")) == 0)
		{
			_tprintf(_T("%s [option]:\n"), argv[0]);
			_tprintf(_T("   option:\n"));
			_tprintf(_T("         -install:install server\n"));
			_tprintf(_T("         -delete:delete server\n"));
			_tprintf(_T("         -help:help\n"));
			return 0;
		}// ���� if(lstrcmpi(argv[1], TEXT("-help")) == 0) 

		CARPVirusService s;
		s.start();
	}

	return nRetCode;
}
