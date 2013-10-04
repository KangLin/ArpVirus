#include "ParaArrarySource.h"
#include "../../ARPVirus/global.h"
#include "ace/Guard_T.h"
#include "../ArpSpoofLib/ArpSpoofLib.h"

CParaArrarySource::CParaArrarySource(void)
{
	std::list<PLAN_HOST_INFO>::iterator it;
	for(it = hostList.begin(); it != hostList.end(); it++)
		m_Host.push_back(*it);
	
	m_nPos = 0;
}

CParaArrarySource::~CParaArrarySource(void)
{
}

int CParaArrarySource::GetSrc(std::string &srcIp, std::string &srcMac, USHORT &nPort)
{
	static USHORT port = 10000;
	
	ACE_Guard <ACE_Thread_Mutex> guard(m_Mutex);
	nPort = port++;
	if(port < 10000) port = 10000;
	
	if(m_Host.empty())
	{
		srcIp = GetSrcIp();
		srcMac = GetSrcMac();
		return 0;
	}
	if(m_nPos >= m_Host.size())
		m_nPos = 0;

	PLAN_HOST_INFO p = m_Host[m_nPos++];
	srcIp = p->IpAddr;
	srcMac = GetMacString(p->ucMacAddr);

	return 0;
}

int CParaArrarySource::Update()
{
	std::list<PLAN_HOST_INFO>::iterator it;
	m_Host.clear();
	for(it = hostList.begin(); it != hostList.end(); it++)
		m_Host.push_back(*it);

	m_nPos = 0;
	return 0;
}