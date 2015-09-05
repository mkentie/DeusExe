#pragma once

class FDeusExeExecHook:  public FExec, public FNotifyHook
{
public:
	explicit FDeusExeExecHook();
	virtual ~FDeusExeExecHook();

private:
	WConfigProperties* m_pPreferences;

//From FNotifyHook
public:
	virtual void NotifyDestroy( void* Src ) override;

//From FExec
public:
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar ) override;
};