#pragma once
#include "para.h"
#include "ace/Thread_Mutex.h"
#include "../../ARPVirus/global.h"
#include <vector>

class CParaArrarySource : public CPara
{
public:
	CParaArrarySource(void);
	virtual ~CParaArrarySource(void);

	virtual int GetSrc(std::string &srcIp, std::string &srcMac, USHORT &nPort);
	virtual int Update();

private:
	ACE_Thread_Mutex m_Mutex;
	std::vector<PLAN_HOST_INFO> m_Host;
	int m_nPos;

};
