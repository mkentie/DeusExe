#include "stdafx.h"
#include "FixApp.h"
#include "Misc.h"
#include "GUIScalingFix.h"
#include "SubTitleFix.h"
#include "resource.h"

CFixApp::CFixApp()
{

}

CFixApp::~CFixApp()
{

}

bool CFixApp::Show(const HWND hWndParent) const
{
    return DialogBoxParam(GetModuleHandle(0),MAKEINTRESOURCE(IDD_DIALOG2),hWndParent,FixAppDialogProc,reinterpret_cast<LPARAM>(this)) == 1;
}

void CFixApp::ReadSettings()
{
    assert(GConfig);

    //Full-screen
    BOOL bFullScreen = TRUE;
    GConfig->GetBool(L"WinDrv.WindowsClient", L"StartupFullScreen", bFullScreen);
    CheckDlgButton(m_hWnd, CHK_FULLSCREEN, bFullScreen);

    //Resolution
    int iResX = 1024;
    GConfig->GetInt(L"WinDrv.WindowsClient", bFullScreen ? L"FullscreenViewportX" : L"WindowedViewportX", iResX);
    int iResY = 768;
    GConfig->GetInt(L"WinDrv.WindowsClient", bFullScreen ? L"FullscreenViewportY" : L"WindowedViewportY", iResY);

    wchar_t szBuffer[20];
    _snwprintf_s(szBuffer, _TRUNCATE, L"%dx%d", iResX, iResY);
    const int iComboRes = ComboBox_FindStringExact(m_hWndCBResolutions, -1, szBuffer);

    if(iComboRes != CB_ERR) //Can fail if res not supported
    {
        ComboBox_SetCurSel(m_hWndCBResolutions, iComboRes);
        EnableWindow(m_hWndCBResolutions, TRUE);
        EnableWindow(m_hWndTxtResX, FALSE);
        EnableWindow(m_hWndTxtResY, FALSE);
        CheckRadioButton(m_hWnd, RADIO_RESCOMMON, RADIO_RESCUSTOM, RADIO_RESCOMMON);
    }
    else
    {
        CheckRadioButton(m_hWnd,RADIO_RESCOMMON,RADIO_RESCUSTOM,RADIO_RESCUSTOM);
        EnableWindow(m_hWndCBResolutions, FALSE);
        EnableWindow(m_hWndTxtResX, TRUE);
        EnableWindow(m_hWndTxtResY, TRUE);
        SetDlgItemInt(m_hWnd,TXT_RESX,iResX,FALSE);
        SetDlgItemInt(m_hWnd,TXT_RESY,iResY,FALSE);
    }

    //Bit depth
    int iBitDepth = sm_iBPP_32;
    GConfig->GetInt(L"WinDrv.WindowsClient", L"FullscreenColorBits", iBitDepth);

    if(iBitDepth == sm_iBPP_16)
    {
        CheckRadioButton(m_hWnd,RADIO_16BIT,RADIO_32BIT,RADIO_16BIT);
    }
    else
    {
        CheckRadioButton(m_hWnd,RADIO_16BIT,RADIO_32BIT,RADIO_32BIT);
    }

    //FOV
    const float fDefaultFOV = Misc::GetDefaultFOV();
    float fFOV = fDefaultFOV;
    GConfig->GetFloat(L"Engine.PlayerPawn", L"DefaultFOV", fFOV, *static_cast<FConfigCacheIni*>(GConfig)->UserIni);
    UBOOL bUseAutoFOV = TRUE;
    GConfig->GetBool(PROJECTNAME, L"UseAutoFOV", bUseAutoFOV);

    if(bUseAutoFOV)
    {
        CheckRadioButton(m_hWnd, RADIO_FOVDEFAULT, RADIO_FOVCUSTOM, RADIO_FOVAUTO);
        EnableWindow(m_hWndTxtFOV, FALSE);
    }
    else if (fFOV == fDefaultFOV)
    {
        CheckRadioButton(m_hWnd, RADIO_FOVDEFAULT, RADIO_FOVCUSTOM, RADIO_FOVDEFAULT);
        EnableWindow(m_hWndTxtFOV, FALSE);
    }
    else
    {
        CheckRadioButton(m_hWnd, RADIO_FOVDEFAULT, RADIO_FOVCUSTOM, RADIO_FOVCUSTOM);
        EnableWindow(m_hWndTxtFOV, TRUE);
    }
    SetDlgItemInt(m_hWnd, TXT_FOV, static_cast<UINT>(fFOV), FALSE);

    //GUI scaling fix
    int iGuiScale = 0;
    GConfig->GetInt(PROJECTNAME, CGUIScalingFix::sm_pszConfigString, iGuiScale);
    CheckDlgButton(m_hWnd, CHK_GUIFIX, iGuiScale!=0);
    if (iGuiScale > 0)
    {
        ComboBox_SetCurSel(m_hWndCBGUIScales, iGuiScale - 1);
        EnableWindow(m_hWndCBGUIScales, TRUE);
    }
    else
    {
        EnableWindow(m_hWndCBGUIScales, FALSE);
    }

    //Subtitle fix
    BOOL bSubtitleFix = TRUE;
    GConfig->GetBool(PROJECTNAME, L"SubtitleFix", bSubtitleFix);
    CheckDlgButton(m_hWnd, CHK_SUBTITLEFIX, bSubtitleFix);

    //Renderer
    const wchar_t* pszRenderer = GConfig->GetStr(L"Engine.Engine", L"GameRenderDevice");
    if(pszRenderer[0]=='\0')
    {
        pszRenderer = L"SoftDrv.SoftwareRenderDevice";
    }

    const auto renderIt = std::find(m_Renderers.cbegin(), m_Renderers.cend(), pszRenderer);
    if(renderIt != m_Renderers.cend())
    {
        ComboBox_SetCurSel(m_hWndCBRenderers, renderIt - m_Renderers.begin());
    }

    //Detail textures
    BOOL bDetailTextures = TRUE;
    GConfig->GetBool(pszRenderer, L"DetailTextures", bDetailTextures);
    CheckDlgButton(m_hWnd,CHK_DETAILTEX, bDetailTextures);

    //Mouse acceleration
    BOOL bNoMouseAccel = TRUE;
    GConfig->GetBool(PROJECTNAME, L"RawInput", bNoMouseAccel);
    CheckDlgButton(m_hWnd,CHK_NOMOUSEACCEL, bNoMouseAccel);

    //DirectSound
    BOOL bDirectSound = TRUE;
    GConfig->GetBool(L"Galaxy.GalaxyAudioSubsystem", L"UseDirectSound", bDirectSound);
    CheckDlgButton(m_hWnd,CHK_DIRECTSOUND, bDirectSound);

    //Audio latency
    int iSndLatency = 40;
    GConfig->GetInt(L"Galaxy.GalaxyAudioSubsystem", L"Latency", iSndLatency);
    SetDlgItemInt(m_hWnd,TXT_LATENCY,iSndLatency,FALSE);

    //FPS limit
    int iFPSLimit = sm_iDefaultFPSLimit;
    GConfig->GetInt(PROJECTNAME, L"FPSLimit", iFPSLimit);
    SetDlgItemInt(m_hWnd, TXT_FPSLIMIT, iFPSLimit, FALSE);

    //Single CPU
    BOOL bUseSingleCPU = FALSE;
    GConfig->GetBool(PROJECTNAME, L"UseSingleCPU", bUseSingleCPU);
    CheckDlgButton(m_hWnd, CHK_USESINGLECPU, bUseSingleCPU);
}

void CFixApp::PopulateDialog()
{
    //Populate the GUI scale combobox
    for (size_t i = 1; i < 6; i++)
    {
        wchar_t szBuf[3];
        swprintf_s(szBuf, L"x%Iu", i);
        ComboBox_AddString(m_hWndCBGUIScales, szBuf);
    }
    ComboBox_SetCurSel(m_hWndCBGUIScales, 0);

    //Populate the resolution combobox
    DEVMODE dm = {};
    dm.dmSize = sizeof(dm);
    Resolution r= {};
    for(DWORD iModeNum = 0; EnumDisplaySettings(NULL, iModeNum, &dm) != FALSE; iModeNum++)
    {
        if((dm.dmPelsWidth != r.iX || dm.dmPelsHeight != r.iY) && dm.dmBitsPerPel == 32) //Only add each res once, but don't actually check if it matches the current color depth/refresh rate
        {
            r.iX = dm.dmPelsWidth;
            r.iY = dm.dmPelsHeight;
            m_Resolutions.push_back(r);

            wchar_t szBuffer[20];
            _snwprintf_s(szBuffer, _TRUNCATE, L"%Iux%Iu", r.iX, r.iY);
            int iIndex = ComboBox_AddString(m_hWndCBResolutions, szBuffer);
            ComboBox_SetItemData(m_hWndCBResolutions, iIndex, &(m_Resolutions.back()));
        }
    }
    ComboBox_SetCurSel(m_hWndCBResolutions, 0);


    //Renderers (based on UnEngineWin.h), requires appInit() to have been called
    TArray<FRegistryObjectInfo> Classes;
    Classes.Empty();

    UObject::GetRegistryObjects( Classes, UClass::StaticClass(), URenderDevice::StaticClass(), 0 );
    for( TArray<FRegistryObjectInfo>::TIterator It(Classes); It; ++It )
    {
        FString Path = It->Object, Left, Right;
        if( Path.Split(L".",&Left,&Right)  )
        {
            const wchar_t* pszDesc = Localize(*Right,L"ClassCaption",*Left);
            assert(pszDesc);
            if(ComboBox_FindStringExact(m_hWndCBRenderers, -1, pszDesc) == CB_ERR)
            {
                ComboBox_AddString(m_hWndCBRenderers, pszDesc);
                m_Renderers.emplace_back(static_cast<wchar_t*>(Path.GetCharArray().GetData()));
            }
        }
    }

}

void CFixApp::ApplySettings() const
{
    assert(GConfig);

    //GUI scaling fix
    GConfig->SetInt(PROJECTNAME, L"GUIScalingFix", IsDlgButtonChecked(m_hWnd, CHK_GUIFIX)==0 ? 0 : ComboBox_GetCurSel(m_hWndCBGUIScales)+1);

    //Subtitle fix
    GConfig->SetBool(PROJECTNAME, L"SubtitleFix", IsDlgButtonChecked(m_hWnd, CHK_SUBTITLEFIX) != 0);

    //Bit depth
    const unsigned char iBitDepth = IsDlgButtonChecked(m_hWnd,RADIO_16BIT) ? sm_iBPP_16 : sm_iBPP_32;   
    GConfig->SetInt(L"WinDrv.WindowsClient",L"FullscreenColorBits",iBitDepth);

    //Resolution
    size_t iResX, iResY;
    if(IsDlgButtonChecked(m_hWnd,RADIO_RESCOMMON))
    {
        const int i = ComboBox_GetCurSel(m_hWndCBResolutions);
        const Resolution* pRes = reinterpret_cast<Resolution*>(ComboBox_GetItemData(m_hWndCBResolutions,i));
        iResX = pRes->iX;
        iResY = pRes->iY;
    }
    else
    {
        iResX = GetDlgItemInt(m_hWnd, TXT_RESX, nullptr, FALSE);
        iResY = GetDlgItemInt(m_hWnd, TXT_RESY, nullptr, FALSE);
    }
    GConfig->SetInt(L"WinDrv.WindowsClient",L"FullscreenViewportX",iResX);
    GConfig->SetInt(L"WinDrv.WindowsClient", L"WindowedViewportX", iResX);
    GConfig->SetInt(L"WinDrv.WindowsClient",L"FullscreenViewportY",iResY);
    GConfig->SetInt(L"WinDrv.WindowsClient", L"WindowedViewportY", iResY);

    //FOV
    float fFOV;
    if(IsDlgButtonChecked(m_hWnd, RADIO_FOVAUTO))
    {
        fFOV = Misc::CalcFOV(iResX, iResY);
        GConfig->SetBool(PROJECTNAME, L"UseAutoFOV", TRUE);
    }
    else
    {
        if(IsDlgButtonChecked(m_hWnd, RADIO_FOVDEFAULT))
        {
            fFOV = Misc::GetDefaultFOV();
            GConfig->SetBool(PROJECTNAME, L"UseAutoFOV", FALSE);
        }
        else
        {
            fFOV = static_cast<float>(GetDlgItemInt(m_hWnd, TXT_FOV, nullptr, FALSE));
            GConfig->SetBool(PROJECTNAME, L"UseAutoFOV", FALSE);
        }

        const wchar_t* pszUserIni = *static_cast<FConfigCacheIni*>(GConfig)->UserIni;
        GConfig->SetFloat(L"Engine.PlayerPawn", L"DesiredFOV", fFOV, pszUserIni);
        GConfig->SetFloat(L"Engine.PlayerPawn", L"DefaultFOV", fFOV, pszUserIni);
    }
    
    //Disable mouse scaling
    GConfig->SetBool(PROJECTNAME,L"RawInput",IsDlgButtonChecked(m_hWnd,CHK_NOMOUSEACCEL)!=0);
    //DirectSound
    GConfig->SetBool(L"Galaxy.GalaxyAudioSubsystem",L"UseDirectSound",IsDlgButtonChecked(m_hWnd,CHK_DIRECTSOUND)!=0);
    //Audio latency
    GConfig->SetInt(L"Galaxy.GalaxyAudioSubsystem", L"Latency", GetDlgItemInt(m_hWnd, TXT_LATENCY, nullptr, FALSE));
    //Full-screen
    GConfig->SetBool(L"WinDrv.WindowsClient",L"StartupFullSCreen",IsDlgButtonChecked(m_hWnd,CHK_FULLSCREEN)!=0);
    //FPS Limit
    GConfig->SetInt(PROJECTNAME, L"FPSLimit", GetDlgItemInt(m_hWnd, TXT_FPSLIMIT, nullptr, FALSE));
    //Single CPU
    GConfig->SetBool(PROJECTNAME, L"UseSingleCPU", IsDlgButtonChecked(m_hWnd, CHK_USESINGLECPU) != 0);
        
    //Renderer
    const int iRendererIndex = ComboBox_GetCurSel(m_hWndCBRenderers);
    GConfig->SetString(L"Engine.Engine",L"GameRenderDevice",m_Renderers[iRendererIndex].c_str());
    //Detail textures
    GConfig->SetBool(m_Renderers[iRendererIndex].c_str(),L"DetailTextures",IsDlgButtonChecked(m_hWnd,CHK_DETAILTEX)!=0);
}

INT_PTR CALLBACK CFixApp::FixAppDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{   
    CFixApp* pThis = reinterpret_cast<CFixApp*>(GetProp(hwndDlg,L"this"));

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            //Get all object oriented like
            SetProp(hwndDlg,L"this",reinterpret_cast<HANDLE>(lParam));
            pThis =  reinterpret_cast<CFixApp*>(lParam);
            pThis->m_hWnd = hwndDlg;
            pThis->m_hWndCBGUIScales = GetDlgItem(hwndDlg, COMBO_GUISCALING);
            pThis->m_hWndCBRenderers = GetDlgItem(hwndDlg, COMBO_RENDERER);
            pThis->m_hWndCBResolutions = GetDlgItem(hwndDlg, COMBO_RESOLUTION);
            pThis->m_hWndTxtResX = GetDlgItem(hwndDlg, TXT_RESX);
            pThis->m_hWndTxtResY = GetDlgItem(hwndDlg, TXT_RESY);
            pThis->m_hWndTxtFOV = GetDlgItem(hwndDlg, TXT_FOV);
            SendMessage(hwndDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(reinterpret_cast<HINSTANCE>(GetWindowLong(hwndDlg, GWL_HINSTANCE)), MAKEINTRESOURCE(IDI_ICON))));

            pThis->PopulateDialog();
            pThis->ReadSettings();
        }
    return TRUE;
    
    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case RADIO_RESCOMMON:
            case RADIO_RESCUSTOM:
            {
                const bool bCustom = LOWORD(wParam) == RADIO_RESCUSTOM;
                EnableWindow(pThis->m_hWndTxtResX, bCustom);
                EnableWindow(pThis->m_hWndTxtResY, bCustom);
                EnableWindow(pThis->m_hWndCBResolutions, !bCustom);
            }
            return TRUE;

            case RADIO_FOVAUTO:
            case RADIO_FOVDEFAULT:
            case RADIO_FOVCUSTOM:
            {
                const bool bCustom = LOWORD(wParam) == RADIO_FOVCUSTOM;
                EnableWindow(pThis->m_hWndTxtFOV, bCustom);
            }
            return TRUE;

            case CHK_GUIFIX:
                EnableWindow(pThis->m_hWndCBGUIScales, IsDlgButtonChecked(pThis->m_hWnd, CHK_GUIFIX));
                return TRUE;

            case IDOK:
                pThis->ApplySettings();
            case IDCANCEL:
                EndDialog(hwndDlg, 0);
                return TRUE;

            }
            break;
        }
        break;

    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        return TRUE;

    }

    return FALSE;
}