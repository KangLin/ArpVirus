
#pragma once
#include "stdafx.h"
#include "Mutex.h"
#include <list>
#include "IPHeaders.h"
//���ڻ�ȡ������Ϣ������
extern PIP_ADAPTER_INFO pAdapterInfo;
extern PIP_ADAPTER_INFO pOrgAdapterInfo;

//winpcap
extern pcap_t *pfp;
extern char error[PCAP_ERRBUF_SIZE]; 

//����������Ϣ
extern std::string localip;
extern std::string netmask;
extern std::string gatewayip;
extern unsigned char gatewaymac[6];
extern unsigned char localmac[6];

//ɨ����������
typedef struct _LAN_HOST_INFO {
	char IpAddr[4 * 4]; //����IP��ַ
	unsigned char ucMacAddr[6]; //����������ַ
}LAN_HOST_INFO,*PLAN_HOST_INFO;

extern std::list < PLAN_HOST_INFO > hostList;
extern std::list < PLAN_HOST_INFO > removeHostList;
extern std::list < PLAN_HOST_INFO > retureHostList;

extern Mutex mutex;

//ARP
extern EthernetHeader ethernet;
extern ArpHeader arp;
extern unsigned char sendBuffer[200];