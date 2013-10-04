// Flood.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Flood.h"
#include "SynFlood.h"


CFlood::CFlood(void)
{
}

CFlood::~CFlood(void)
{
}

int CFlood::open (void *para)
{
	if(para)
		m_Para = (CPara*) para;
	else
	{
		m_Para = CPara::GetInstance();
	}

	if(m_Para)
		activate(0, m_Para->GetThreadNumber());
	return 0;
}

int CFlood::svc()
{
	int nRet = 0;
	printf("ok\n");
	return nRet;
}

CSynFlood g_Flood;

int
lib_flood_start()
//ACE_TMAIN(int argc, ACE_TCHAR * argv[])
{
	g_Flood.open(NULL);

	return 0;
}

void lib_flood_stop()
{
	g_Flood.close();
}