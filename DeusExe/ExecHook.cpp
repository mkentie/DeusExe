#include "stdafx.h"
#include "ExecHook.h"

FExecHook::FExecHook()
{

}


FExecHook::~FExecHook()
{

}

void FExecHook::NotifyDestroy( void* Src )
{
    if( Src==m_pPreferences )
    {
        m_pPreferences = nullptr;
    }
}

UBOOL FExecHook::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
    if( ParseCommand(&Cmd,TEXT("ShowLog")) )
    {
        if( GLogWindow )
        {
            GLogWindow->Show(1);
            SetFocus( *GLogWindow );
            GLogWindow->Display.ScrollCaret();
        }
        return TRUE;
    }
    else if( ParseCommand(&Cmd,TEXT("TakeFocus")) )
    {
        TObjectIterator<UEngine> EngineIt;
        if( EngineIt && EngineIt->Client && EngineIt->Client->Viewports.Num() )
        {
            SetForegroundWindow( (HWND)EngineIt->Client->Viewports(0)->GetWindow() );
        }
        return TRUE;
    }
    else if( ParseCommand(&Cmd,TEXT("EditActor")) )
    {
        UClass* Class;
        TObjectIterator<UEngine> EngineIt;
        if( EngineIt && ParseObject<UClass>( Cmd, TEXT("Class="), Class, ANY_PACKAGE ) )
        {
            const AActor* Player  = EngineIt->Client ? EngineIt->Client->Viewports(0)->Actor : NULL;
            const AActor* Found   = NULL;
            FLOAT   MinDist = 999999.0f;
            for( TObjectIterator<AActor> It; It; ++It )
            {
                FLOAT Dist = Player ? FDist(It->Location,Player->Location) : 0.0f;
                if( (!Player || It->GetLevel()==Player->GetLevel()) &&  (!It->bDeleteMe) && (It->IsA( Class) ) && (Dist<MinDist) )
                {
                    MinDist = Dist;
                    Found   = *It;
                }
            }
            if( Found )
            {
                WObjectProperties* P = new WObjectProperties( TEXT("EditActor"), 0, TEXT(""), NULL, 1 );
                P->OpenWindow( (HWND)EngineIt->Client->Viewports(0)->GetWindow() );
                P->Root.SetObjects( (UObject**)&Found, 1 );
                P->Show(1);
            }
            else Ar.Logf( TEXT("Actor not found") );
        }
        else Ar.Logf( TEXT("Missing class") );
        return TRUE;
    }
    else if( ParseCommand(&Cmd,TEXT("HideLog")) )
    {
        if( GLogWindow )
        {
            GLogWindow->Show(0);
        }
        return TRUE;
    }
    else if( ParseCommand(&Cmd,TEXT("Preferences")) && !GIsClient )
    {
        if( !m_pPreferences )
        {
            m_pPreferences = new WConfigProperties( TEXT("Preferences"), LocalizeGeneral("AdvancedOptionsTitle",TEXT("Window")) );
            m_pPreferences->SetNotifyHook( this );
            m_pPreferences->OpenWindow( GLogWindow ? GLogWindow->hWnd : NULL );
            m_pPreferences->ForceRefresh();
        }
        assert(m_pPreferences);
        m_pPreferences->Show(TRUE);
        SetFocus( *m_pPreferences );
        return TRUE;
    }
    return FALSE;
}