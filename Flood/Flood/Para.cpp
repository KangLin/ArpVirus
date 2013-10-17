
#include "ParaArrarySource.h"
#include <pcap.h> 
#include <remote-ext.h>
#include <conio.h> 
#include <packet32.h> 
#include <ntddndis.h>
#include <Iphlpapi.h>
#include "../ArpSpoofLib/ArpSpoofLib.h"
//#include <winsock2.h>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "Ws2_32.lib")

CPara::CPara(void)
{
	OnInit();
	m_nDstPort = CBase::GetConfigInt("section", "dst_port", 80);
	m_szDstIp = CBase::GetConfigString("section", "dst_ip", "61.187.204.17");
}

CPara::~CPara(void)
{
}

CPara* CPara::GetInstance()
{
	static CPara* p = NULL;
	if(p)
		return p;
	std::string buf = CBase::GetConfigString("section", "para_instance", "normal");
	if(!stricmp(buf.c_str(), "src_mac"))
		p = new CParaArrarySource();
	if(!stricmp(buf.c_str(), "normal"))
		p = new CPara();

	if(!p)
		p = new CPara();

	return p;
}

int CPara::OnInit()
{
	std::string name;
	unsigned char gatewaymac[6], localmac[6];
	char error[PCAP_ERRBUF_SIZE]; 
	//用于获取网卡信息的链表
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pOrgAdapterInfo;

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
		pcap_t *pfp = pcap_open_live( pOrgAdapterInfo->AdapterName ,  100, 0, 1, error );
		ULONG macAddLen=6;
		//如果能正常打开网卡，且能正确连入网关，则判断为活动网卡
		if(pfp != NULL && (SendARP(inet_addr((char*)pOrgAdapterInfo->GatewayList.IpAddress.String), (IPAddr) NULL,(PULONG) gatewaymac, &macAddLen) == NO_ERROR))
			break;
		else
			pOrgAdapterInfo = pOrgAdapterInfo->Next;
	}

	//没有满足要求的网卡则说明主机不能正常上网，程序退出。
	if(pOrgAdapterInfo == NULL)
	{
		printf("无上网网卡");
		GlobalFree(pAdapterInfo);
		return - 1;
	} // 结束 if(pOrgAdapterInfo == NULL)

	m_szInterfaceName = pOrgAdapterInfo->AdapterName;
	m_DstMac = GetMacString(gatewaymac);

	m_szSrcIp = pOrgAdapterInfo->IpAddressList.IpAddress.String;
	ULONG macLen=6;
	if(!(SendARP(inet_addr((char*)pOrgAdapterInfo->IpAddressList.IpAddress.String), (IPAddr) NULL,(PULONG) localmac, &macLen) == NO_ERROR))
	{
		printf("得到本地MAC失败");
		GlobalFree(pAdapterInfo);
		return - 2;
	} // 结束 if(!(SendARP(inet_addr((char*)pOrgAdapterInfo->IpAddressList.IpAddress.String), (IPAddr) NULL,(PULONG) localmac, &macLen) == NO_ERROR))	
	m_szSrcMac = GetMacString(localmac);
	
	GlobalFree(pAdapterInfo);
	return 0;
}

int CPara::Update()
{
	return 0;
}

int CPara::TimePolic()
{
	return CBase::TimePolic();
}

std::string CPara::GetInterfaceName()
{
	return m_szInterfaceName;
}

USHORT CPara::GetDstPort()
{
	return m_nDstPort;
}

std::string CPara::GetDstIp()
{
	return m_szDstIp;
}

std::string CPara::GetDstMac()
{
	return m_DstMac;
}

std::string CPara::GetSrcIp()
{
	return CBase::GetConfigString("section", "src_ip", m_szSrcIp.c_str());
}

USHORT CPara::GetSrcPort()
{
	static int nPort = 1001;
	return nPort++;
}

std::string  CPara::GetSrcMac()
{
	return CBase::GetConfigString("section", "src_mac", m_szSrcMac.c_str());
}

int CPara::GetSrc(std::string &srcIp, std::string &srcMac, USHORT &nPort)
{
	srcIp = GetSrcIp();
	srcMac = GetSrcMac();
	nPort = GetSrcPort();
	return 0;
}

int CPara::GetThreadNumber()
{
	int nNum = 10;
	nNum = CBase::GetConfigInt("section", "thread_number", nNum);
	return nNum;
}
