#pragma once


#include <string>


class Mutex
{
public:
    Mutex()
    {
        InitializeCriticalSection( &m_mutex );

    }


    ~Mutex()
    {
     
        DeleteCriticalSection( &m_mutex );

    }


    inline void Lock()
    {
       
        EnterCriticalSection( &m_mutex );

    }


    inline void Unlock()
    {
      
        LeaveCriticalSection( &m_mutex );

    }

protected:
// define the base mutex types

    CRITICAL_SECTION m_mutex;


};  // end class Mutex
