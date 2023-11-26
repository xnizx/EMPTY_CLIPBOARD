// EMPTY_CLIPBOARD.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "EMPTY_CLIPBOARD.h"
#include <tlhelp32.h>
#include <mmsystem.h>
#include "xLogWnd.h"
#include <cstdlib>
#include <windows.h>
#include <filesystem>
#include <thread>
#include <iostream>
#include <sstream>
#include <vector>
#include <signal.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <unordered_map>
#include <string>
#include <winsvc.h>

extern HWND g_CEMPTY_CLIPBOARDApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


typedef std::unordered_map < DWORD, std::string > PROCESSESMAP;


using namespace std;



BOOL GetProcessNameAndId(PROCESSESMAP* procMap) {

	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		return (FALSE);
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return (FALSE);
	}

	do {

		procMap->insert(std::make_pair(pe32.th32ProcessID, pe32.szExeFile));

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return (TRUE);
}


LPCSTR GetClipBoardService() {

	SC_HANDLE manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (manager == INVALID_HANDLE_VALUE) {
		std::cout << "Invalid Handle Value." << std::endl;
	}

	PROCESSESMAP* processesMap = new PROCESSESMAP;

	GetProcessNameAndId(processesMap);

	DWORD bytesNeeded;
	DWORD servicesNum;

	BOOL status = EnumServicesStatusEx(
		manager,
		SC_ENUM_PROCESS_INFO,
		SERVICE_WIN32,
		SERVICE_STATE_ALL,
		NULL,
		0,
		&bytesNeeded,
		&servicesNum,
		NULL,
		NULL
	);

	PBYTE lpBytes = (PBYTE)malloc(bytesNeeded);

	status = EnumServicesStatusEx(
		manager,
		SC_ENUM_PROCESS_INFO,
		SERVICE_WIN32,
		SERVICE_STATE_ALL,
		lpBytes,
		bytesNeeded,
		&bytesNeeded,
		&servicesNum,
		NULL,
		NULL
	);

	ENUM_SERVICE_STATUS_PROCESS* lpServiceStatus = (ENUM_SERVICE_STATUS_PROCESS*)lpBytes;
	const char* cstrf = NULL;
	for (DWORD i = 0; i < servicesNum; i++) {

		const char* cstr = lpServiceStatus[i].lpServiceName;

		//WpnUserService_38a78
		string AllServices = cstr;
		string serviceNameToFind = "cbdhsvc_";

		if (strstr(AllServices.c_str(), serviceNameToFind.c_str()))
		{
			cstrf = cstr;
			
			std::cout << cstr << std::endl;
		}
	}

	delete processesMap;
	free(lpBytes);
	return cstrf;
}

void ClearClipBoard() {

	// Will clear the clipboard history

	const auto hScm = OpenSCManager(nullptr, nullptr, NULL);
	const auto hSc = OpenService(hScm, GetClipBoardService(), SERVICE_QUERY_STATUS);
	// ^ The clipboard service

	SERVICE_STATUS_PROCESS ssp = {};

	DWORD bytesNeeded = 0;
	QueryServiceStatusEx(hSc, SC_STATUS_PROCESS_INFO, reinterpret_cast <LPBYTE> (&ssp), sizeof(ssp), &bytesNeeded);

	CloseServiceHandle(hSc);
	CloseServiceHandle(hScm);

	HANDLE ProcPidHANDLE;
	ProcPidHANDLE = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, ssp.dwProcessId);
	TerminateProcess(ProcPidHANDLE, 0);

	if (OpenClipboard(NULL)) {
		// Will clear the current text

		EmptyClipboard();
		CloseClipboard();
	}
}
// CEMPTY_CLIPBOARDApp

BEGIN_MESSAGE_MAP(CEMPTY_CLIPBOARDApp, CWinApp)
	//ON_COMMAND(ID_HELP, CWinApp::OnHelp)//2004.10.20 - F1키 비활성화
END_MESSAGE_MAP()

//2011.10.13

// CEMPTY_CLIPBOARDApp construction

CEMPTY_CLIPBOARDApp::CEMPTY_CLIPBOARDApp()
{
	
}


// The one and only CEMPTY_CLIPBOARDApp object

CEMPTY_CLIPBOARDApp theApp;
CString	_strCurrentDir= _T("");

// CEMPTY_CLIPBOARDApp initialization

BOOL CEMPTY_CLIPBOARDApp::InitInstance()
{

	//실행파일 경로를 현재 폴더로 설정함
	SetCurrentDirectoryToExistsFolder();
	_TCHAR	szCurPath[_MAX_PATH];
	GetCurrentDirectory(255, szCurPath);
	_strCurrentDir = szCurPath;
	if(_strCurrentDir.GetAt(_strCurrentDir.GetLength()-1)!=_T('\\'))
		_strCurrentDir+=_T('\\');
	
	ClearClipBoard();
	
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CEMPTY_CLIPBOARDApp::SetCurrentDirectoryToExistsFolder()
{
	TCHAR	tszModulePath[_MAX_PATH] = {0,};
	TCHAR	tszNewPath[_MAX_PATH] = {0,};
	TCHAR	c = TEXT('\\');
	LPTSTR	pLastOccur = NULL;
	size_t	nLen = 0;

	VERIFY(GetModuleFileName(NULL, tszModulePath, sizeof(tszModulePath)/sizeof(tszModulePath[0])));
	
	pLastOccur = _tcsrchr(tszModulePath, c);
	nLen = size_t(pLastOccur - tszModulePath + 1);
	
	_tcsncpy_s(tszNewPath, tszModulePath, nLen);

	VERIFY(SetCurrentDirectory(tszNewPath));
}
int CEMPTY_CLIPBOARDApp::ExitInstance()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 프로그램 종료 시 메모리 해제
	return CWinApp::ExitInstance();
}


BOOL CEMPTY_CLIPBOARDApp::RemoveDirectoryFile(LPCTSTR PathDir, BOOL bFirst)//2016.04.15
{
	if( PathDir== NULL )  
	{  
		return FALSE;  
	}  

	BOOL bReturnval = FALSE; 
	CString szNextDirPath   = _T(""); 
	CString szRoot = _T("");


	// 해당 디렉토리의 모든 파일을 검사하도록 한다.
	szRoot.Format(_T("%s\\*.*"), PathDir);

	CFileFind find; 

	bReturnval = find.FindFile( szRoot );

	if( bReturnval == FALSE )  
	{        
		return bReturnval ;  
	}

	while( bReturnval )  
	{  
		bReturnval = find.FindNextFile();


		if( find.IsDots() == FALSE )   
		{   
			// Directory 일 경우 재귀로 호출을 한다.
			if( find.IsDirectory() )   
			{   
				RemoveDirectoryFile(find.GetFilePath());   
			}   
			// file일 경우 삭제
			else   
			{    
				bReturnval = DeleteFile(find.GetFilePath()); 
				XLOGOUT(_T("DeleteFile[%d, %s]"), bReturnval, find.GetFilePath());
			}  
		}
	} 
	find.Close();
	if (bFirst == FALSE)
	{
		bReturnval = RemoveDirectory(PathDir);
		XLOGOUT(_T("RemoveDirectory[%d, %s]"), bReturnval, PathDir);

	}
	return bReturnval ;
}
