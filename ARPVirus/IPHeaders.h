



#pragma once
#pragma pack( push ,1 )          // ȡ��WINDOWS�ڴ��Զ����룬����Ҫ
typedef struct _EthernetHeader
{ 
    u_char DestMAC[6];
    u_char SourMAC[6];
    u_short EthType;
}EthernetHeader;
typedef struct _ArpHeader
{
    u_short HardwareType;
    u_short ProtocolType;
    u_char MACLength;
    u_char IpAddLength;
    u_short OperationCode;
    u_char SourceMAC[6];
    unsigned long SourceIP;
    u_char DestinationMAC[6];
    unsigned long DestinationIP;
}ArpHeader;
#pragma pack(  pop  )