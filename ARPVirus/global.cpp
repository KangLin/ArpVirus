#include "stdafx.h"
#include "global.h"
//���ڻ�ȡ������Ϣ������
PIP_ADAPTER_INFO pAdapterInfo;
PIP_ADAPTER_INFO pOrgAdapterInfo;

//winpcap
pcap_t *pfp;
char error[PCAP_ERRBUF_SIZE]; 
 
//����������Ϣ
std::string localip;
std::string netmask;
std::string gatewayip;
unsigned char gatewaymac[6];
unsigned char localmac[6];

//ɨ����������
std::list< PLAN_HOST_INFO > hostList;
Mutex mutex;

//ARP 
EthernetHeader ethernet;
ArpHeader arp;
unsigned char sendBuffer[200];