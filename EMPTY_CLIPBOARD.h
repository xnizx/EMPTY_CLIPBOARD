// EMPTY_CLIPBOARD.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

// CEMPTY_CLIPBOARDApp:
// See EMPTY_CLIPBOARD.cpp for the implementation of this class
//
extern CString	_strCurrentDir;
class CEMPTY_CLIPBOARDApp : public CWinApp
{
public:
	BOOL RemoveDirectoryFile(LPCTSTR PathDir, BOOL bFirst = FALSE);
	CStringArray m_aryRemove;
	CEMPTY_CLIPBOARDApp();
	void SetCurrentDirectoryToExistsFolder();
// Overrides
	public:
	virtual BOOL InitInstance();
// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CEMPTY_CLIPBOARDApp theApp;
