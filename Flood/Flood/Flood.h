#pragma once

#include "ace/Task.h"
#include "Para.h"

class CFlood : public ACE_Task<ACE_MT_SYNCH>
{
public:
	CFlood(void);
	~CFlood(void);

	virtual int open (void *para);
	virtual int svc ();

protected:
	CPara* m_Para;
};

int lib_flood();
