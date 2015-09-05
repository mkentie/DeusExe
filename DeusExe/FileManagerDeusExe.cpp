#include "stdafx.h"
#include "FileManagerDeusExe.h"
#include "Misc.h"

const wchar_t* const FFileManagerDeusExe::sm_pszIntPaths = L"IntPaths";


FFileManagerDeusExe::FFileManagerDeusExe()
{

}

FFileManagerDeusExe::~FFileManagerDeusExe()
{

}

void FFileManagerDeusExe::AfterCoreInit()
{

}


void FFileManagerDeusExe::OnGameStart()
{
    m_pIntPaths = std::make_unique<std::vector<std::wstring>>();
    //Prepare int overrides
    TMultiMap<FString, FString>* const pSectionInt = GConfig->GetSectionPrivate(PROJECTNAME, FALSE, FALSE);
    if(pSectionInt)
    {
        TArray<FString> IntPaths;
        pSectionInt->MultiFind(sm_pszIntPaths, IntPaths); //Returns in reverse order
        for(int i = IntPaths.Num() - 1; i >= 0; i--)
        {
            //Convert format like "..\Shifter\*.int" to "..\Shifter\"
            wchar_t szBuf[MAX_PATH];
            wcscpy_s(szBuf, *IntPaths(i));
            PathRemoveFileSpec(szBuf);
            PathAddBackslash(szBuf);
            m_pIntPaths->emplace_back(szBuf);
        }
    }
}

bool FFileManagerDeusExe::IntOverride(wchar_t(&szNewName)[MAX_PATH], const wchar_t* const pszOldName)
{
    if(!m_pIntPaths) //Not initialized yet
    {
        return false;
    }
    wchar_t* pszExtension = PathFindExtension(pszOldName);
    assert(pszExtension);
    if(*pszExtension == '\0')
    {
        return false;
    }
    pszExtension++; //After period

    const bool bIntFile = _wcsicmp(pszExtension, L"int") == 0;
    const bool bLocalizedFile = !bIntFile && _wcsicmp(pszExtension, UObject::GetLanguage()) == 0;

    if(!bIntFile && !bLocalizedFile)
    {
        return false;
    }

    for(const std::wstring& IntPath : *m_pIntPaths)
    {
        //PathCombine() doens't work with relative paths...
        wcscpy_s(szNewName, IntPath.c_str());
        wcscpy_s(szNewName + IntPath.length(), _countof(szNewName) - IntPath.length(), pszOldName);
        const DWORD dwAttrib = GetFileAttributes(szNewName); //PathFileExists also returns true for directories
        if(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
        {
            return true;
        }

        if(bLocalizedFile) //If localized, try again with .int
        {
            wchar_t* const pszNewExtension = PathFindExtension(szNewName);
            wcscpy_s(pszNewExtension, _countof(szNewName) - (pszNewExtension - szNewName), L".int");
            const DWORD dwAttrib2 = GetFileAttributes(szNewName); //PathFileExists also returns true for directories
            if(dwAttrib2 != INVALID_FILE_ATTRIBUTES && !(dwAttrib2 & FILE_ATTRIBUTE_DIRECTORY))
            {
                return true;
            }
        }
    }

    return false;
}

FArchive* FFileManagerDeusExe::CreateFileReader(const wchar_t* Filename, DWORD Flags, FOutputDevice* Error)
{
    wchar_t szFilename[MAX_PATH];
    return FFileManagerWindows::CreateFileReader(IntOverride(szFilename, Filename) ? szFilename : Filename, Flags, Error);
}

bool FFileManagerDeusExe::ToModernFileName(wchar_t(&szNewName)[MAX_PATH], const wchar_t* const pszOldName, const char /*op*/ /*= 'r'*/)
{
	wcscpy_s(szNewName, pszOldName);
    return true;
}

FFileManagerDeusExeUserDocs::FFileManagerDeusExeUserDocs()
{
    Misc::GetUserDocsDir(m_szUserDataPath);
    PathAppend(m_szUserDataPath, L"System"); //The games use paths relative to system, so we're doing that too.

    //Get game and system directory
    Misc::GetGameSystemDir(m_szSystemPath);
    PathCombine(m_szGamePath, m_szSystemPath, L".."); //Move up from system directory

#ifdef _DEBUG
    Test();
#endif
}

void FFileManagerDeusExeUserDocs::AfterCoreInit()
{
    assert(GLog);
    GLog->Log(L"Deus Exe: Using User Documents File Manager.");
    FFileManagerDeusExe::AfterCoreInit();
}

void FFileManagerDeusExeUserDocs::Test()
{
    wchar_t szNewPath[MAX_PATH];
    ToModernFileName(szNewPath,L"bla.txt",'w');
    ToModernFileName(szNewPath,L".\\bla.txt",'w');
    ToModernFileName(szNewPath,L"..\\bla.txt",'w');
    ToModernFileName(szNewPath,L"..\\system\\bla.txt",'w');
    ToModernFileName(szNewPath,L"..\\textures\\bla.txt",'w');
    ToModernFileName(szNewPath,L"..\\..\\bla.txt",'w');
    ToModernFileName(szNewPath,L"D:\\Steam\\steamapps\\common\\Deus Ex\\..\\bla.txt");
    ToModernFileName(szNewPath,L"D:\\Steam\\steamapps\\common\\Deus Ex\\textures\\bla.txt");
    ToModernFileName(szNewPath,L"D:\\Steam\\steamapps\\common\\Deus Ex\\system\\bla.txt");
    ToModernFileName(szNewPath,L"c:\\bla.txt",'w');
    ToModernFileName(szNewPath,L"C:\\Users\\Marijn Kentie\\Documents\\Deus Ex\\System\\bla.txt",'w');
    ToModernFileName(szNewPath, L"C:\\Users\\Marijn Kentie\\Documents\\Deus Ex\\boe\\..\\System\\bla2.txt", 'w');
    ToModernFileName(szNewPath,L"C:\\Users\\Marijn Kentie\\Documents\\Deus Ex\\",'w');
}

bool FFileManagerDeusExeUserDocs::ToModernFileName(wchar_t(&szNewName)[MAX_PATH], const wchar_t* const pszOldName, const char op)
{
    assert(pszOldName);
    assert(szNewName != pszOldName);

    //Make all paths relative to game directory
    if(PathIsRelative(pszOldName))
    {
        wcscpy_s(szNewName, pszOldName);
    }
    else
    {
        //If not in game directory, abort. This facilitates how MakeDirectory() recursively creates directories
        wchar_t szCommonPrefix[MAX_PATH];
        PathCanonicalize(szNewName,pszOldName); //Resolve relative parts of path
        PathCommonPrefix(szNewName,m_szGamePath,szCommonPrefix);
        if(_wcsicmp(szCommonPrefix,m_szGamePath)!=0) //If not in game directory, don't touch path and return
        {
            return false;
        }

        //Make path relative to System, like the games' own paths
        PathRelativePathTo(szNewName,m_szSystemPath,FILE_ATTRIBUTE_DIRECTORY,pszOldName,0);
    }

    //Rebase to Documents
    PathCombine(szNewName,m_szUserDataPath,szNewName);
    
    if((op == 'r' || op == 'd') && !PathFileExists(szNewName) && PathFileExists(pszOldName)) //If opening a file that already exists, return original.
    {
        return false;
    }

    //Create directory if needed
    if(op=='w' && !PathIsDirectory(pszOldName)) //PathIsDirectory needed as PathRemoveFileSpec would strip stuff like 'Save040' to just 'Save'
    {
        wchar_t* pszFileSpec = PathFindFileName(szNewName);
        PathRemoveFileSpec(szNewName);
        if(!PathFileExists(szNewName))
        {
            MakeDirectory(szNewName,1);
        }
        PathAppend(szNewName,pszFileSpec);
    }
    return true;
}

FArchive* FFileManagerDeusExeUserDocs::CreateFileReader(const wchar_t* Filename, DWORD Flags, FOutputDevice* Error)
{
    assert(Filename);
    wchar_t szNewFilename[MAX_PATH];
    return FFileManagerWindows::CreateFileReader(IntOverride(szNewFilename, Filename) ? szNewFilename : ToModernFileName(szNewFilename, Filename) ? szNewFilename : Filename, Flags, Error);
}

FArchive* FFileManagerDeusExeUserDocs::CreateFileWriter(const wchar_t* Filename, DWORD Flags, FOutputDevice* Error)
{
    assert(Filename);
    wchar_t szNewFilename[MAX_PATH];
    return FFileManagerWindows::CreateFileWriter(ToModernFileName(szNewFilename, Filename, 'w') ? szNewFilename : Filename, Flags, Error);
}

INT FFileManagerDeusExeUserDocs::FileSize(const wchar_t* Filename)
{
    assert(Filename);
    wchar_t szNewFilename[MAX_PATH];
    return FFileManagerWindows::FileSize(ToModernFileName(szNewFilename, Filename) ? szNewFilename : Filename);
}

UBOOL FFileManagerDeusExeUserDocs::Copy(const wchar_t* DestFile, const wchar_t* SrcFile, UBOOL ReplaceExisting, UBOOL EvenIfReadOnly, UBOOL Attributes, void(*Progress)(FLOAT Fraction))
{
    assert(DestFile);
    assert(SrcFile);
    wchar_t szNewDestFile[MAX_PATH];
    wchar_t szNewSrcFile[MAX_PATH];
    return FFileManagerWindows::Copy(ToModernFileName(szNewDestFile, DestFile, 'w') ? szNewDestFile : DestFile, ToModernFileName(szNewSrcFile, SrcFile)  ? szNewSrcFile : SrcFile, ReplaceExisting, EvenIfReadOnly, Attributes, Progress);
}

UBOOL FFileManagerDeusExeUserDocs::Delete(const wchar_t* Filename, UBOOL RequireExists, UBOOL EvenReadOnly)
{
    assert(Filename);
    wchar_t szNewFilename[MAX_PATH];
    return FFileManagerWindows::Delete(ToModernFileName(szNewFilename, Filename, 'd') ? szNewFilename : Filename, RequireExists, EvenReadOnly);
}

UBOOL FFileManagerDeusExeUserDocs::Move(const wchar_t* Dest, const wchar_t* Src, UBOOL Replace, UBOOL EvenIfReadOnly, UBOOL Attributes)
{
    assert(Dest);
    assert(Src);
    wchar_t szNewDest[MAX_PATH];
    wchar_t szNewSrc[MAX_PATH];
    return FFileManagerWindows::Move(ToModernFileName(szNewDest, Dest, 'w') ? szNewDest : Dest, ToModernFileName(szNewSrc, Src, 'd') ? szNewSrc : Src, Replace, EvenIfReadOnly, Attributes);
}
    
UBOOL FFileManagerDeusExeUserDocs::MakeDirectory(const wchar_t* Path, UBOOL Tree)
{
    assert(Path);
    wchar_t szNewPath[MAX_PATH];
    return FFileManagerWindows::MakeDirectory(ToModernFileName(szNewPath, Path, 'w')  ? szNewPath : Path, Tree);
}

UBOOL FFileManagerDeusExeUserDocs::DeleteDirectory(const wchar_t* Path, UBOOL RequireExists, UBOOL Tree)
{
    assert(Path);
    wchar_t szNewPath[MAX_PATH];
    return FFileManagerWindows::DeleteDirectory(ToModernFileName(szNewPath, Path, 'd') ? szNewPath : Path, RequireExists, Tree);
}

TArray<FString> FFileManagerDeusExeUserDocs::FindFiles(const wchar_t* Filename, UBOOL Files, UBOOL Directories)
{
    assert(Filename);
    
    //Look for old style filenames
    auto Result = FFileManagerWindows::FindFiles(Filename,Files,Directories);

    wchar_t szConvertedFilename[MAX_PATH];
    if(!ToModernFileName(szConvertedFilename, Filename)) //Already started out with a new style file
    {
        return Result;
    }

    //Look for new style filenames
    WIN32_FIND_DATAW Data;
    const HANDLE hHandle = FindFirstFileW(szConvertedFilename, &Data);
    if(hHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if
                (appStricmp(Data.cFileName, L".")
                && appStricmp(Data.cFileName, L"..")
                && ((Data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ? Directories : Files)
                && Result.FindItemIndex(Data.cFileName) == INDEX_NONE) //Don't list things twice if the game won't be able to tell them apart (i.e. a directory like 'Save001' with no further path will result in the new version being listed twice)
            {
                #pragma warning(disable:4291) //no matching operator delete found; memory will not be freed if initialization throws an exception
                new(Result)FString(Data.cFileName);
            }
        } while(FindNextFileW(hHandle, &Data));
    }

    if( hHandle )
    {
        FindClose( hHandle );
    }
    return Result;
}
