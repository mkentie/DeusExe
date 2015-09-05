#pragma once

class CNativeHooks
{
public:
    explicit CNativeHooks(const wchar_t* const pszIniSection);

    class CFixBase //So instances can share container
    {
    public:
        virtual ~CFixBase() {}
    };

    template <class FixerClass, class UnrealClass, size_t iNativeId> //We use CRTP as the replacement function is called with another object's 'this' pointer, i.e. virtual functions plain won't work.
    class CFixBaseT : public CFixBase
    {
    public:
        ~CFixBaseT()
        {
            //Restore original behavior
            GNatives[iNativeId] = m_OrigFunc;
        }

    protected:
        explicit CFixBaseT(const wchar_t* const pszName)
        :m_OrigFunc(GNatives[iNativeId])
        {
            GNatives[iNativeId] = reinterpret_cast<Native>(&CFixBaseT<FixerClass, UnrealClass, iNativeId>::ReplacementFuncInternal);
            GLog->Logf(L"Installing hook '%s'.", pszName);

            //We add ourselves to the parent, that way the native function id doesn't have to be exposed
            GetSingleton().m_ActiveHooks.emplace(iNativeId, std::unique_ptr<CFixBase>(this));
        }

    private:
        //The actual native function that's called
        void ReplacementFuncInternal(FFrame& Stack, RESULT_DECL)
        {
            //At this point our 'this' pointer points to an Unreal object, not the fix object, so get its pointer.
            assert(CNativeHooks::GetSingleton().m_ActiveHooks.find(iNativeId) != std::cend(GetSingleton().sm_pSingleton->m_ActiveHooks));
            CFixBase* const pContext = CNativeHooks::GetSingleton().m_ActiveHooks.at(iNativeId).get();
            assert(pContext);

            static_cast<FixerClass&>(*this).ReplacementFunc(reinterpret_cast<UnrealClass&>(*this), static_cast<FixerClass&>(*pContext), Stack, Result);
        }

        const Native m_OrigFunc;
    };

private:
    static CNativeHooks& GetSingleton()
    {
        assert(sm_pSingleton);
        return *sm_pSingleton;
    }

    static CNativeHooks* sm_pSingleton; //Need a singleton as classes with a wrong 'this' pointer must be able to find context.
    std::unordered_map<size_t, std::unique_ptr<CFixBase>> m_ActiveHooks;
};
