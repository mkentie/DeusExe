#include "stdafx.h"
#include "GUIScalingFix.h"

const wchar_t* const CGUIScalingFix::sm_pszConfigString = L"GUIScalingFix";

CGUIScalingFix::CGUIScalingFix(const decltype(XRootWindow::hMultiplier) iScaleAmount)
:CFixBaseT(L"GUI scaling fix")
,m_iScaleAmount(iScaleAmount)
{
    assert(m_iScaleAmount > 0);
}

void CGUIScalingFix::ReplacementFunc(const APlayerPawnExt& PlayerPawnThis, CGUIScalingFix& Context, FFrame& Stack, RESULT_DECL)
{
    UNREFERENCED_PARAMETER(Result);

    P_GET_OBJECT(UCanvas, pCanvas);
    P_FINISH;

    //Hacky derived class so we can access protected members
    class RootHack : public XRootWindow
    {
    public:
        void ApplyScaling(UCanvas* const pCanvas, CGUIScalingFix& Context)
        {
            //Checking the size fixes OTP scaling fix issue of UI popping into corner when e.g. resizing window from 2x to 1x scaling range
            if (hMultiplier != Context.m_iScaleAmount || vMultiplier != Context.m_iScaleAmount || Context.m_iSizeX != pCanvas->X || Context.m_iSizeY != pCanvas->Y )
            {
                ResizeRoot(pCanvas);

                //Change our size; children will align themselves properly
                const float fScaleAmount = static_cast<float>(Context.m_iScaleAmount);
                width *= hMultiplier/fScaleAmount;
                height *= vMultiplier/fScaleAmount;

                clipRect.clipWidth = width;
                clipRect.clipHeight = height;

                winGC->SetClipRect(clipRect); //Otherwise cursor is hidden

                //Prevent actual scaling
                hMultiplier = Context.m_iScaleAmount;
                vMultiplier = Context.m_iScaleAmount;

                //Apply changes
                for (XWindow *pChild = GetBottomChild(); pChild != nullptr; pChild = pChild->GetHigherSibling())
                {
                    pChild->Hide();
                    pChild->Show();
                }

                Context.m_iSizeX = pCanvas->X;
                Context.m_iSizeY = pCanvas->Y;
            }
        }
    };

    XRootWindow* const pRoot = PlayerPawnThis.rootWindow;
    assert(pRoot);
    RootHack* const pHack = static_cast<RootHack*>(pRoot);
    pHack->ApplyScaling(pCanvas, Context);
}
