#include "stdafx.h"
#include "global.h"
//用于获取网卡信息的链表
PIP_ADAPTER_INFO pAdapterInfo;
PIP_ADAPTER_INFO pOrgAdapterInfo;

//winpcap
pcap_t *pfp;
char error[PCAP_ERRBUF_SIZE]; 
 
//基本网络信息
std::string localip;
std::string netmask;
std::string gatewayip;
unsigned char gatewaymac[6];
unsigned char localmac[6];

//扫描在线主机
std::list < PLAN_HOST_INFO > hostList;
std::list < PLAN_HOST_INFO > allHostList;
std::list < PLAN_HOST_INFO > spoofHostList;


Mutex mutex;

//ARP 
EthernetHeader ethernet;
ArpHeader arp;
unsigned char sendBuffer[200];