#include "stdafx.h"
#include "DeusExeExecHook.h"

FDeusExeExecHook::FDeusExeExecHook()
{

}

FDeusExeExecHook::~FDeusExeExecHook()
{

}

void FDeusExeExecHook::NotifyDestroy( void* Src )
{
	if(Src==m_pPreferences)
	{
		m_pPreferences = nullptr; //Engine deletes it
	}
}

UBOOL FDeusExeExecHook::Exec( const TCHAR* Cmd, FOutputDevice& /*Ar*/ )
{	
	if( ParseCommand(&Cmd,TEXT("ShowLog")) )
	{
		if( GLogWindow )
		{
			GLogWindow->Show(TRUE);
			SetFocus( *GLogWindow );
			GLogWindow->Display.ScrollCaret();
		}
		return TRUE;
	}	
	else if( ParseCommand(&Cmd,TEXT("HideLog")) )
	{
		if( GLogWindow )
		{
			GLogWindow->Show(FALSE);
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("Preferences")) && !GIsClient )
	{
		if( !m_pPreferences )
		{
			m_pPreferences = new WConfigProperties( TEXT("Preferences"), LocalizeGeneral("AdvancedOptionsTitle",TEXT("Window")));
			m_pPreferences->SetNotifyHook( this );
			m_pPreferences->OpenWindow( GLogWindow ? GLogWindow->hWnd : NULL );
			m_pPreferences->ForceRefresh();
		}
		m_pPreferences->Show(1);
		SetFocus( *m_pPreferences );
		return TRUE;
	}
	
	return FALSE;
}