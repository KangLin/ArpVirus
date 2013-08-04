
#pragma once
#include "stdafx.h"
#include "Mutex.h"
#include <list>
#include "IPHeaders.h"
//用于获取网卡信息的链表
extern PIP_ADAPTER_INFO pAdapterInfo;
extern PIP_ADAPTER_INFO pOrgAdapterInfo;

//winpcap
extern pcap_t *pfp;
extern char error[PCAP_ERRBUF_SIZE]; 

//基本网络信息
extern std::string localip;
extern std::string netmask;
extern std::string gatewayip;
extern unsigned char gatewaymac[6];
extern unsigned char localmac[6];

//扫描在线主机
typedef struct _LAN_HOST_INFO {
	char IpAddr[4 * 4]; //主机IP地址
	unsigned char ucMacAddr[6]; //主机网卡地址
}LAN_HOST_INFO,*PLAN_HOST_INFO;

extern std::list < PLAN_HOST_INFO > hostList;
extern std::list < PLAN_HOST_INFO > removeHostList;
extern std::list < PLAN_HOST_INFO > retureHostList;

extern Mutex mutex;

//ARP
extern EthernetHeader ethernet;
extern ArpHeader arp;
extern unsigned char sendBuffer[200];