// drawcli.cpp : Defines the class behaviors for the application.
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "drawcli.h"

#include "mainfrm.h"
#include "drawobj.h"
#include "drawdoc.h"
#include "drawvw.h"
#include "splitfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDrawApp

BEGIN_MESSAGE_MAP(CDrawApp, CWinApp)
	//BEGIN_MESSAGE_MAP: 메시지 맵의 정의를 시작합니다.
	//{{AFX_MSG_MAP(CDrawApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//ON_COMMAND: 지정된 명령 메시지를 처리할 함수를 나타냅니다.
	//OnAppAbout: 다이로그 실행에 관한 앱 코멘트 
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	//ON_COMMAND: 지정된 명령 메시지를 처리할 함수를 나타냅니다.
	//CWinApp::OnFileNew(): ID_FILE_NEW 명령을 구현합니다.
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	//ON_COMMAND: 지정된 명령 메시지를 처리할 함수를 나타냅니다.
	//ID_FILE_OPEN 명령을 구현합니다.
	//CWinApp::OnFileOpen():  Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	//ON_COMMAND: 지정된 명령 메시지를 처리할 함수를 나타냅니다.
	// CWinApp::OnFilePrintSetup(): ID_FILE_PRINT_SETUP 명령을 구현합니다.

END_MESSAGE_MAP() //메시지 맵의 정의를 종료합니다.

/////////////////////////////////////////////////////////////////////////////
// CDrawApp construction

CDrawApp::CDrawApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	//InitInstance에 모든 중요한 초기화 배치
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDrawApp object

CDrawApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDrawApp initialization

BOOL CDrawApp::InitInstance()
{
		SetRegistryKey(_T("YourCompany"));
		//SetRegistryKey(): 애플리케이션 설정이 INI 파일 대신 레지스트리에 저장되도록 합니다.
	// Initialize OLE libraries
	if (!AfxOleInit())//OLE 라이브러리를 초기화
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		//AfxMessageBox(): 알림창 띄우기
		//IDP_OLE_INIT_FAILED: OLE 초기화에 실패했습니다. OLE 라이브러리가 올바른 버전인지 확인합니다.
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
	//LoadStdProfileSettings():
	//InitInstance 멤버 함수 내에서 이 멤버 
	//함수를 호출하여 가장 최근에 사용한(MRU) 파일 및
	//마지막 미리 보기 상태 목록을 사용하도록 설정하고 로드합니다

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_DRAWCLTYPE,
		RUNTIME_CLASS(CDrawDoc),
		RUNTIME_CLASS(CSplitFrame),
		RUNTIME_CLASS(CDrawView));
	pDocTemplate->SetContainerInfo(IDR_DRAWCLTYPE_CNTR_IP);
	//SetContainerInfo(): 현재 위치 OLE 항목을 편집할 때 OLE 컨테이너에 대한 리소스를 결정합니다.
	AddDocTemplate(pDocTemplate);
	//AddDocTemplate(): 이 멤버 함수를 호출하여 응용 프로그램에서 유지 관리하는 사용 가능한 문서 서식 파일 목록에 문서 서식 파일을 추가합니다.

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;

	if(!pMainFrame)
		return FALSE;

	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	// Enable DDE Execute open
	EnableShellOpen();
	//클릭으로 파일을 열 수 있게 하는 함수.
	RegisterShellFileTypes(TRUE);
	//도큐먼트 템플릿을 찾아내 도큐먼트 타입을 레지스트리에 등록하는 함수

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo); 
	//이 멤버 함수를 호출하여 명령줄을 구문 분석하고
	//매개 변수를 한 번에 하나씩 cmdInfo로 보냅니다.

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
//ProcessShellCommand(): 이 멤버 함수는 rCmdInfo로 식별된 개체에서 CCommandLineInfo 전달된 매개 변수를 수락하고 표시된 작업을 수행하기 위해 InitInstance에서 호출됩니다.
		return FALSE;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	//ProcessShellCommand(): 응용 프로그램의 CWinApp::InitInstance 함수에서 포인터를 CWnd 사용하여 창 내에서 이 멤버 함수를 호출하여 창이 Windows 파일 관리자 또는 파일 탐색기 삭제된 파일을 허용함을 나타냅니다.

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	//ShowWindow(): 개체를 숨기거나 표시하려고 할 때 CWnd 프레임워크에서 이 멤버 함수를 호출합니다.
	pMainFrame->UpdateWindow();
	//UpdateWindow(): 업데이트 지역이 비어 있지 않은 경우 메시지를 보내 WM_PAINT 클라이언트 영역을 업데이트합니다.
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX }; //IDD_ABOUTBOX: 이 프로그램 정보 창
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//DoDataExchange(): 대화 상자 데이터를 교환하고 유효성을 검사하기 위해 프레임워크에서 호출됩니다.
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	//DECLARE_MESSAGE_MAP(): 클래스가 메시지 맵을 정의한다고 선언합니다. 프로그램의 각 CCmdTarget파생 클래스는 메시지를 처리할 메시지 맵을 제공해야 합니다.
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//대화 상자 데이터를 교환하고 유효성을 검사하기 위해 프레임워크에서 호출됩니다.
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CDrawApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
	//DoModal(): 이 멤버 함수를 호출하여 모달 대화 상자를 호출하고 완료되면 대화 상자 결과를 반환합니다.
}

/////////////////////////////////////////////////////////////////////////////
// CDrawApp commands
