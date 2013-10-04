#pragma once

#include "flood.h"
#include "Para.h"
#include "../ArpSpoofLib/ArpSpoofLib.h"
#pragma pack (1) 

typedef struct tcphdr
{
	USHORT th_sport;
	USHORT th_dport;
	unsigned int th_seq;
	unsigned int th_ack;
	unsigned char th_lenres;
	unsigned char th_flag;
	USHORT th_win;
	USHORT th_sum;
	USHORT th_urp;
}TCP_HEADER; 

typedef struct iphdr
{
	unsigned char h_verlen;
	unsigned char tos;
	unsigned short total_len;
	unsigned short ident;
	unsigned short frag_and_flags;
	unsigned char  ttl;
	unsigned char proto;
	unsigned short checksum;
	unsigned int sourceIP;
	unsigned int destIP;
}IP_HEADER;

#pragma pack()

USHORT checksum(USHORT *buffer, int size);

class CSynFlood : public CFlood
{
public:
	CSynFlood(void);
	virtual ~CSynFlood(void);

	virtual int close(){m_bExit = true; return 0;};

private:
	int ConstructTcpHead(TCP_HEADER *pHead, const char* srcIp, USHORT srcPort, const char* desIp, USHORT desPort);
	int ConstructIpHead(IP_HEADER *pHead, const char* srcIp, const char* desIp);
	int ConstructMacHead(ETHERNET_HEAD *pHead, const char* srcMac,  const char* dstMac);
	int ConstructMacHead(ETHERNET_HEAD *head);
	int Construct(char *pBuffer, const char* srcMac, const char* srcIp, USHORT srcPort, const char* dstMac, const char* desIp, USHORT desPort);
	int Construct(char *pBuffer);

	virtual int svc();

	BOOL m_bExit;
	
};
