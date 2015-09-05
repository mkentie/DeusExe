#pragma once

#include <list>

//Pretty much default file manager, but with .int file overrides
class FFileManagerDeusExe : public FFileManagerWindows
{
public:
    static const wchar_t* const sm_pszIntPaths;

    explicit FFileManagerDeusExe();
    virtual ~FFileManagerDeusExe();

    virtual void AfterCoreInit();
    void OnGameStart();

    //Used by startup dialog
    virtual bool ToModernFileName(wchar_t(&szNewName)[MAX_PATH], const wchar_t* const pszOldName, const char op = 'r');

    //We new() this class before the Unreal core is started (instead of creating it on the stack) so we can use derived classes.
    //As Unreal overrides global operator new with something that requires the core to be running, that's a catch-22 situation and we require our own new operator.
    void* operator new(const size_t s)
    {
        return malloc(s);
    }

    void operator delete(void* const p)
    {
        free(p);
    }

protected:
    /**
    For some reason, the game (also tested Unreal 1) crashes when a *.int is added as an override path; so we use our own mechanism.
    
    Returns false of the original file isn't a (localized) .int file, or if no suitable replacement was found.
    */  
    bool IntOverride(wchar_t(&szNewName)[MAX_PATH], const wchar_t* const pszOldName);

private:
    std::unique_ptr<std::vector<std::wstring>> m_pIntPaths; //Pointer because we can't allocate on startup

//From FFileManagerWindows
public:
    virtual FArchive* CreateFileReader(const wchar_t* Filename, DWORD Flags, FOutputDevice* Error) override;
};

//File manager that uses user documents directory
class FFileManagerDeusExeUserDocs: public FFileManagerDeusExe
{
public:
    explicit FFileManagerDeusExeUserDocs();
private:
    void Test();

    wchar_t m_szUserDataPath[MAX_PATH];
    wchar_t m_szSystemPath[MAX_PATH];
    wchar_t m_szGamePath[MAX_PATH];

    //From FFileManagerDeusExe
public:
    virtual void AfterCoreInit() override;

    /**
    Converts relative paths to modern (i.e. Documents-based) filename.
    */
    virtual bool ToModernFileName(wchar_t(&szNewName)[MAX_PATH], const wchar_t* const pszOldName, const char op = 'r') override;

    //From FFileManagerWindows
public:
    virtual FArchive* CreateFileReader(const wchar_t* Filename, DWORD Flags, FOutputDevice* Error) override;
    virtual FArchive* CreateFileWriter(const wchar_t* Filename, DWORD Flags, FOutputDevice* Error) override;
    virtual INT FileSize(const wchar_t* Filename) override;
    virtual UBOOL Copy(const wchar_t* DestFile, const wchar_t* SrcFile, UBOOL ReplaceExisting, UBOOL EvenIfReadOnly, UBOOL Attributes, void(*Progress)(FLOAT Fraction)) override;
    virtual UBOOL Delete(const wchar_t* Filename, UBOOL RequireExists = 0, UBOOL EvenReadOnly = 0) override;
    virtual UBOOL Move(const wchar_t* Dest, const wchar_t* Src, UBOOL Replace = 1, UBOOL EvenIfReadOnly = 0, UBOOL Attributes = 0) override;
    virtual UBOOL MakeDirectory(const wchar_t* Path, UBOOL Tree = 0) override;
    virtual UBOOL DeleteDirectory(const wchar_t* Path, UBOOL RequireExists = 0, UBOOL Tree = 0) override;
    virtual TArray<FString> FindFiles(const wchar_t* Filename, UBOOL Files, UBOOL Directories) override;
};