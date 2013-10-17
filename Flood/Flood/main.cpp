#include "ace/ACE.h"
#include "ace/Thread_Manager.h"
int lib_flood_start();
#ifdef _EXE_
int ACE_TMAIN(int argc, ACE_TCHAR * argv[])
{
	lib_flood_start();
	ACE_Thread_Manager::instance()->wait();
	return 0;
	
}
#endif