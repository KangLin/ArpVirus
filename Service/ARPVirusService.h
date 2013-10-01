#pragma once
#include "service.h"

class CARPVirusService :
	public CService
{
public:
	CARPVirusService(void);
	virtual ~CARPVirusService(void);

	virtual DWORD serviceMain(int argc, TCHAR * argv[]);
	virtual void stop();

private:
	static CString m_szName;
};
