
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
}

CPara::~CPara(void)
{
}

CPara* CPara::GetInstance()
{
	char buf[100];
	static CPara* p = NULL;
	if(p)
		return p;
	GetPrivateProfileString("section", "para_instance", "src_mac", buf, 100, GetModuleFile().c_str());
	if(!stricmp(buf, "src_mac"))
		p = new CParaArrarySource();
	if(!stricmp(buf, "normal"))
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

std::string CPara::GetInterfaceName()
{
	return m_szInterfaceName;
}

USHORT CPara::GetDstPort()
{
	return 80;
}

std::string CPara::GetDstIp()
{
	char buf[100];
	GetPrivateProfileString("section", "dst_ip", "61.187.204.17", buf, 100, GetModuleFile().c_str());
	return buf; 
}

std::string CPara::GetDstMac()
{
	return m_DstMac;
}

std::string CPara::GetSrcIp()
{
	char buf[100];
	GetPrivateProfileString("section", "src_ip", m_szSrcIp.c_str(), buf, 100, GetModuleFile().c_str());
	m_szSrcIp = buf;
	return m_szSrcIp;
}

USHORT CPara::GetSrcPort()
{
	static int nPort = 1001;
	return nPort++;
}

std::string  CPara::GetSrcMac()
{
	char buf[100];
	GetPrivateProfileString("section", "src_mac", m_szSrcMac.c_str(), buf, 100, GetModuleFile().c_str());
	m_szSrcMac = buf;
	return m_szSrcMac;
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
	nNum = GetPrivateProfileInt("section", "thread_number", nNum, 
		CPara::GetModuleFile().c_str());
	return nNum;
}

std::string CPara::GetModuleFile()
{
	TCHAR szFileName[1024];
	int iFileLen = 1024;
	int iLen = ::GetModuleFileName(NULL, szFileName, iFileLen);
	if(!iLen)
	{
		printf(("得到当前执行文件名错误.\n"));
		return "";
	}

	std::string szFile = szFileName, szFilePath;
	int i = szFile.find_last_of(('\\'));
	memset(szFileName, 0, 1024);
	memcpy(szFileName, szFile.c_str(), i + 1);
	szFilePath = szFileName;
	return szFilePath + "config.ini";
}

int CPara::Rand(int min, int max)
{
	//srand((unsigned) time(NULL)); //为了提高不重复的概率
	return rand()%(max - min + 1) + min;                //使用时将m和n换为具体数即可
}