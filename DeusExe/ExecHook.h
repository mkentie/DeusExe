#pragma once

/**
Basically copied from UnEngineWin.h and cleaned up a little
*/

class FExecHook : public FExec, public FNotifyHook
{
public:
    explicit FExecHook();
    ~FExecHook();
private:
    WConfigProperties* m_pPreferences = nullptr; //Cleaned up by engine

//From FExec
protected:
    UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );

//From FNotifyHook
private:
    void NotifyDestroy( void* Src );
};