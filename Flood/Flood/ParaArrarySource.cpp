#include "ParaArrarySource.h"
#include "../../ARPVirus/global.h"
#include "ace/Guard_T.h"
#include "../ArpSpoofLib/ArpSpoofLib.h"

CParaArrarySource::CParaArrarySource(void)
{
}

CParaArrarySource::~CParaArrarySource(void)
{
}

int CParaArrarySource::GetSrc(std::string &srcIp, std::string &srcMac, USHORT &nPort)
{
	static USHORT port = 10000;
	
	//ACE_Guard <ACE_Thread_Mutex> guard(m_Mutex);
	mutex.Lock();
	nPort = port++;
	if(port < 10000) port = 10000;
	
	if(hostList.empty())
	{
		srcIp = GetSrcIp();
		srcMac = GetSrcMac();
		mutex.Unlock();
		return 0;
	}
	
	if(!(m_nPos <= hostList.size() && m_nPos > 0))
		m_nPos = 1;

	std::list<PLAN_HOST_INFO>::iterator it;
	int i = 1;
	for(it = hostList.begin(); it != hostList.end(); it++)
	{
		if(i++ == m_nPos)
			break;
	}
	if(it != hostList.end())
	{
		PLAN_HOST_INFO p = *it;
		srcIp = p->IpAddr;
		srcMac = GetMacString(p->ucMacAddr);
		m_nPos++;
	}	
	else
		m_nPos = 1;

	mutex.Unlock();
	return 0;
}

int CParaArrarySource::Update()
{
	return 0;
}