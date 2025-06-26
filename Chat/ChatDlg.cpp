
// ChatDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Chat.h"
#include "ChatDlg.h"
#include "afxdialogex.h"

#include "utility.h"
#include "FriendDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CChatDlg 대화 상자



CChatDlg::CChatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	loginView = NULL;
	registerView = NULL;
	mainChatView = NULL;
	chatManager = new ChatManager(this);
	friendDlg = new CFriendDlg(chatManager);
}

CChatDlg::~CChatDlg()
{
	if (loginView)
	{
		delete loginView;
	}
	
	if (registerView)
	{
		delete registerView;
	}
	
	if (mainChatView)
	{
		delete mainChatView;
	}

	if (chatManager)
	{
		delete chatManager;
	}

	if (friendDlg)
	{
		delete friendDlg;
	}
}

void CChatDlg::MovePage(PAGE_NAME name)
{
	CRect rcScreen;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
	switch (name)
	{
	case PAGE_NAME::LOGIN:
	{
		int nWidth = 600;
		int nHeight = 450;
		int x = rcScreen.left + (rcScreen.Width() - nWidth) / 2;
		int y = rcScreen.top + (rcScreen.Height() - nHeight) / 2;
		loginView->ShowWindow(SW_SHOW);
		registerView->ShowWindow(SW_HIDE);
		mainChatView->ShowWindow(SW_HIDE);
		UpdateWindowStyle(FALSE, FALSE, FALSE);
		MoveWindow(x, y, nWidth, nHeight);
		break;
	}
	case PAGE_NAME::REGISTER:
	{
		int nWidth = 600;
		int nHeight = 560;
		int x = rcScreen.left + (rcScreen.Width() - nWidth) / 2;
		int y = rcScreen.top + (rcScreen.Height() - nHeight) / 2;
		loginView->ShowWindow(SW_HIDE);
		registerView->ShowWindow(SW_SHOW);
		mainChatView->ShowWindow(SW_HIDE);
		UpdateWindowStyle(FALSE, FALSE, FALSE);
		MoveWindow(x, y, nWidth, nHeight);
		break;
	}
	case PAGE_NAME::MAINCHAT:
	{
		int nWidth = 1500;
		int nHeight = 1200;
		int x = rcScreen.left + (rcScreen.Width() - nWidth) / 2;
		int y = rcScreen.top + (rcScreen.Height() - nHeight) / 2;
		loginView->ShowWindow(SW_HIDE);
		registerView->ShowWindow(SW_HIDE);
		mainChatView->ShowWindow(SW_SHOW);
		UpdateWindowStyle(TRUE, TRUE, TRUE);
		MoveWindow(x, y, nWidth, nHeight);
		break;
	}
	default:
		break;
	}
}

CFriendDlg* CChatDlg::GetFriendDlg()
{
	return friendDlg;
}

void CChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CChatDlg, CDialogEx)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_BUTTON_CLICK, &CChatDlg::OnButtonClick)
	ON_MESSAGE(WM_EDIT_COMPLETE, &CChatDlg::OnEditComplete)
	ON_MESSAGE(WM_LOGIN_ACTION, &CChatDlg::OnLoginAction)
	ON_MESSAGE(WM_REGISTER_ACTION, &CChatDlg::OnRegisterAction)
	ON_MESSAGE(WM_LOGOUT_ACTION, &CChatDlg::OnLogoutAction)
	ON_MESSAGE(WM_MESSAGE_RECEIVED, &CChatDlg::OnMessageReceived)
END_MESSAGE_MAP()


// CChatDlg 메시지 처리기

BOOL CChatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	InitLoginView();
	InitRegisterView();
	InitMainChatView();
	MovePage(PAGE_NAME::LOGIN);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

int CChatDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

	return 0;
}


void CChatDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (loginView && ::IsWindow(loginView->GetSafeHwnd()))
	{
		loginView->MoveWindow(0, 0, cx, cy);
	}
	if (registerView && ::IsWindow(registerView->GetSafeHwnd()))
	{
		registerView->MoveWindow(0, 0, cx, cy);
	}
	if (mainChatView && ::IsWindow(mainChatView->GetSafeHwnd()))
	{
		mainChatView->MoveWindow(0, 0, cx, cy);

		PaneWnd* mainView = (PaneWnd*)mainChatView->GetElement(1);
		PaneWnd* message = NULL;
		ItemWnd* text = NULL;
		int cnt = ((ScrollWnd*)mainView->GetElement(0))->GetElementCount();
		CRect rect;
		((ScrollWnd*)mainView->GetElement(0))->GetClientRect(&rect);
		for (int i = 0; i < cnt; i++)
		{
			message = (PaneWnd*)((ScrollWnd*)mainView->GetElement(0))->GetElement(i);
			if (message->GetElement(0)->GetName().Compare(_T("TEXT")) == 0)
			{
				text = (ItemWnd*)message->GetElement(0);
				text->SetSize(min(max(200, rect.Width() * 0.6), text->GetTextWidth()));
			}
			if (message->GetElement(1)->GetName().Compare(_T("TEXT")) == 0)
			{
				text = (ItemWnd*)message->GetElement(1);
				text->SetSize(min(max(200, rect.Width() * 0.6), text->GetTextWidth()));
			}
		}
	}
}

void CChatDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CChatDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CChatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

void CChatDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 1500;
	lpMMI->ptMinTrackSize.y = 1200;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

CString GetCurrentDateTimeString()
{
	time_t now = time(nullptr);
	struct tm localTime;
	localtime_s(&localTime, &now);

	CString strTime;
	strTime.Format(_T("%04d/%02d/%02d %02d:%02d:%02d"),
		localTime.tm_year + 1900,
		localTime.tm_mon + 1,
		localTime.tm_mday,
		localTime.tm_hour,
		localTime.tm_min,
		localTime.tm_sec);

	return strTime;
}

LRESULT CChatDlg::OnButtonClick(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = (CString*)wParam;
	if (pStr)
	{
		CString str = *pStr;
		delete pStr;

		if (str.Compare(_T("LOGIN")) == 0)
		{
			CString account = ((EditWnd*)loginView->GetElement(1))->GetItemText();
			CString password = ((EditWnd*)loginView->GetElement(3))->GetItemText();
			if (account.IsEmpty())
			{
				MessageBox(_T("Enter account!"), _T("Notice"), MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			else if (password.IsEmpty())
			{
				MessageBox(_T("Enter password!"), _T("Notice"), MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			chatManager->loginAccount(account, password);
		}
		else if (str.Compare(_T("MOVEREGISTER")) == 0)
		{
			MovePage(PAGE_NAME::REGISTER);
		}
		else if (str.Compare(_T("REGISTER")) == 0)
		{
			CString name = ((EditWnd*)registerView->GetElement(1))->GetItemText();
			CString account = ((EditWnd*)registerView->GetElement(3))->GetItemText();
			CString password = ((EditWnd*)registerView->GetElement(5))->GetItemText();
			if (name.IsEmpty())
			{
				MessageBox(_T("Enter name!"), _T("Notice"), MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			else if (account.IsEmpty())
			{
				MessageBox(_T("Enter account!"), _T("Notice"), MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			else if (password.IsEmpty())
			{
				MessageBox(_T("Enter password!"), _T("Notice"), MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			chatManager->registerAccount(name, account, password);
		}
		else if (str.Compare(_T("MOVELOGIN")) == 0)
		{
			MovePage(PAGE_NAME::LOGIN);
		}
		else if (str.Compare(_T("ADDFRIEND")) == 0)
		{
			CRect rect;
			GetWindowRect(&rect);
			int centerX = (rect.left + rect.right) / 2;
			int centerY = (rect.top + rect.bottom) / 2;
			
			EnableWindow(FALSE);
			if(friendDlg->CreateWnd(this, CRect(centerX - 250, centerY - 300, centerX + 250, centerY + 300)))
			{
				friendDlg->ShowWindow(SW_SHOW);
				friendDlg->UpdateWindow();
				friendDlg->RunModalLoop();
				friendDlg->DestroyWindow();
			}
			EnableWindow(TRUE);
			/*
			PaneWnd* sideView = (PaneWnd*)mainChatView->GetElement(0);
			((ScrollWnd*)sideView->GetElement(1))->AddElement(new ButtonWnd(TEXTSTRUCT(_T("friend")), SHAPESTRUCT(), _T("FRIEND"), 70));
			
			CRect rect;
			((ScrollWnd*)sideView->GetElement(1))->GetClientRect(rect);
			((ScrollWnd*)sideView->GetElement(1))->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
			*/
		}
		else if (str.Compare(_T("ADDROOM")) == 0)
		{
			PaneWnd* sideView = dynamic_cast<PaneWnd*>(mainChatView->GetElement(0));
			if(sideView)
			{
				((ScrollWnd*)sideView->GetElement(3))->AddElement(new ButtonWnd(TEXTSTRUCT(_T("room")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(0, 0, 0, 0), 10), _T("ROOM"), 70));
			
				CRect rect;
				((ScrollWnd*)sideView->GetElement(3))->GetClientRect(rect);
				((ScrollWnd*)sideView->GetElement(3))->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
			}
		}
		else if (str.Compare(_T("SENDMESSAGE")) == 0)
		{
			PaneWnd* mainView = dynamic_cast<PaneWnd*>(mainChatView->GetElement(1));
			if(mainView)
			{
				EditWnd* edit = (EditWnd*)mainView->FindElement(_T("MESSAGE"));
				if (edit->GetItemText().IsEmpty())
				{
					return 0;
				}
				chatManager->sendMessage(GetCurrentDateTimeString(), edit->GetItemText());
			}
		}
	}
	return 0;
}

LRESULT CChatDlg::OnEditComplete(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = (CString*)wParam;
	if (pStr)
	{
		CString str = *pStr;
		delete pStr;

		if (str.Compare(_T("MESSAGE")) == 0)
		{
			PaneWnd* mainView = (PaneWnd*)mainChatView->GetElement(1);
			EditWnd* edit = (EditWnd*)mainView->FindElement(_T("MESSAGE"));
			if (edit->GetItemText().IsEmpty())
				return 0;

			chatManager->sendMessage(GetCurrentDateTimeString(), edit->GetItemText());
		}
	}
	return 0;
}

LRESULT CChatDlg::OnLoginAction(WPARAM wParam, LPARAM lParam)
{
	CString* result = reinterpret_cast<CString*>(lParam);
	if (static_cast<BOOL>(wParam))
	{
		saveToRegistry(_T("Account"), ((EditWnd*)loginView->GetElement(1))->GetItemText());
		saveToRegistry(_T("Password"), ((EditWnd*)loginView->GetElement(3))->GetItemText());
		MovePage(PAGE_NAME::MAINCHAT);
	}
	else
	{
		MessageBox(*result, _T("Notice"), MB_OK | MB_ICONINFORMATION);
	}
	delete result;
	return 0;
}

LRESULT CChatDlg::OnRegisterAction(WPARAM wParam, LPARAM lParam)
{
	CString* result = reinterpret_cast<CString*>(lParam);
	MessageBox(*result, _T("Notice"), MB_OK | MB_ICONINFORMATION);
	if (static_cast<BOOL>(wParam))
	{
		((EditWnd*)registerView->GetElement(1))->ClearText();
		((EditWnd*)registerView->GetElement(3))->ClearText();
		((EditWnd*)registerView->GetElement(5))->ClearText();
		MovePage(PAGE_NAME::LOGIN);
	}
	delete result;
	return 0;
}

LRESULT CChatDlg::OnLogoutAction(WPARAM wParam, LPARAM lParam)
{
	CString* result = reinterpret_cast<CString*>(lParam);
	MovePage(PAGE_NAME::LOGIN);
	delete result;
	return 0;
}

LRESULT CChatDlg::OnMessageReceived(WPARAM wParam, LPARAM lParam)
{
	CString* pTime = reinterpret_cast<CString*>(wParam);
	CString* pChat = reinterpret_cast<CString*>(lParam);
	ReceiveChatMessage(*pTime, *pChat);
	delete pTime;
	delete pChat;
	return 0;
}

void CChatDlg::InitLoginView()
{
	CRect rect;
	GetClientRect(&rect);
	ElementWnd* tmpWnd = NULL;
	loginView = new PaneWnd(DIRECTION::VERTICAL, _T(""), -1, RGB(255,255,255));
	loginView->Initialize(this, rect);
	
	loginView->AddElement(new ItemWnd(TEXTSTRUCT(_T("ACCOUNT"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::BOTTOM, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(10, 30, 10, 0)), _T(""), 60));
	loginView->AddElement(new EditWnd(TEXTSTRUCT(loadFromRegistry(_T("Account"))), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 0, 10, 20), 30), _T("ACCOUNT"), 70, RGB(255, 255, 255), RGB(125, 125, 125)));
	loginView->AddElement(new ItemWnd(TEXTSTRUCT(_T("PASSWORD"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::BOTTOM, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(10, 20, 10, 0)), _T(""), 50));
	tmpWnd = new EditWnd(TEXTSTRUCT(loadFromRegistry(_T("Password"))), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 0, 10, 20), 30), _T("PASSWORD"), 70, RGB(255, 255, 255), RGB(125, 125, 125));
	((EditWnd*)tmpWnd)->SetStylePassword();
	loginView->AddElement(tmpWnd);

	loginView->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Login"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 0), 30), _T("LOGIN"), 70, RGB(86, 124, 131)));
	loginView->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Register"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 10), 30), _T("MOVEREGISTER"), 70, RGB(98, 164, 188)));
}

void CChatDlg::InitRegisterView()
{
	CRect rect;
	GetClientRect(&rect);
	ElementWnd* tmpWnd = NULL;
	registerView = new PaneWnd(DIRECTION::VERTICAL, _T(""), -1, RGB(255, 255, 255));
	registerView->Initialize(this, rect);

	registerView->AddElement(new ItemWnd(TEXTSTRUCT(_T("NAME"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::BOTTOM, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(10, 30, 10, 0)), _T(""), 60));
	registerView->AddElement(new EditWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 0, 10, 20), 30), _T("NAME"), 70, RGB(255, 255, 255), RGB(125, 125, 125)));
	registerView->AddElement(new ItemWnd(TEXTSTRUCT(_T("ACCOUNT"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::BOTTOM, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(10, 20, 10, 0)), _T(""), 50));
	registerView->AddElement(new EditWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 0, 10, 20), 30), _T("ACCOUNT"), 70, RGB(255, 255, 255), RGB(125, 125, 125)));
	registerView->AddElement(new ItemWnd(TEXTSTRUCT(_T("PASSWORD"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::BOTTOM, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(10, 20, 10, 0)), _T(""), 50));
	
	tmpWnd = new EditWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 0, 10, 20), 30), _T("PASSWORD"), 70, RGB(255, 255, 255), RGB(125, 125, 125));
	((EditWnd*)tmpWnd)->SetStylePassword();
	registerView->AddElement(tmpWnd);
	registerView->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Register"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 10), 30), _T("REGISTER"), 70, RGB(98, 164, 188)));
	registerView->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Cancel")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 0, 10, 10), 30), _T("MOVELOGIN"), 60, RGB(200, 200, 200)));
}

void CChatDlg::InitMainChatView()
{
	CRect rect;
	GetClientRect(&rect);
	ElementWnd* tmpWnd = NULL;

	mainChatView = new PaneWnd(DIRECTION::HORIZONTAL, _T(""), -1, RGB(255, 255, 255));
	mainChatView->Initialize(this, rect);

	PaneWnd* sideView = new PaneWnd(DIRECTION::VERTICAL, _T("SIDEVIEW"), 400, RGB(242, 242, 249));
	PaneWnd* chatView = new PaneWnd(DIRECTION::VERTICAL, _T("CHATVIEW"), -1, RGB(255, 255, 255));
	mainChatView->AddElement(sideView);
	mainChatView->AddElement(chatView);

	sideView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 60));
	((PaneWnd*)sideView->GetElement(0))->AddElement(new ItemWnd(TEXTSTRUCT(_T("Friends"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(10, 10, 10, 0)), _T(""), -1));
	((PaneWnd*)sideView->GetElement(0))->AddElement(new ButtonWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::PLUS, CRect(10, 20, 20, 10)), _T("ADDFRIEND"), 60, sideView->GetBackgroundColor(), 0));
	sideView->AddElement(new ScrollWnd(DIRECTION::VERTICAL, _T("FRIENDS"), 500));

	sideView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 60));
	((PaneWnd*)sideView->GetElement(2))->AddElement(new ItemWnd(TEXTSTRUCT(_T("Rooms"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(10, 10, 10, 0)), _T(""), -1));
	((PaneWnd*)sideView->GetElement(2))->AddElement(new ButtonWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::PLUS, CRect(10, 20, 20, 10)), _T("ADDROOM"), 60, sideView->GetBackgroundColor(), 0));
	sideView->AddElement(new ScrollWnd(DIRECTION::VERTICAL, _T("ROOMS"), -1));

	ScrollWnd* chatWnd = new ScrollWnd(DIRECTION::VERTICAL, _T("CHAT"), -1);
	chatWnd->UsePaintEx();
	chatView->AddElement(chatWnd);
	chatView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T("INPUT"), 70));

	((PaneWnd*)chatView->GetElement(1))->AddElement(new EditWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 10), 10), _T("MESSAGE"), -1, RGB(255, 255, 255), RGB(125, 125, 125)));
	((PaneWnd*)chatView->GetElement(1))->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Send"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(0, 10, 10, 10), 10), _T("SENDMESSAGE"), 80, RGB(86, 124, 131)));
}

void CChatDlg::UpdateWindowStyle(BOOL resize, BOOL minsize, BOOL maxsize)
{
	LONG_PTR style = ::GetWindowLongPtr(this->GetSafeHwnd(), GWL_STYLE);

	if (resize)
		style |= WS_THICKFRAME;
	else
		style &= ~WS_THICKFRAME;

	if (minsize)
		style |= WS_MINIMIZEBOX;
	else
		style &= ~WS_MINIMIZEBOX;

	if (maxsize)
		style |= WS_MAXIMIZEBOX;
	else
		style &= ~WS_MAXIMIZEBOX;

	::SetWindowLongPtr(this->GetSafeHwnd(), GWL_STYLE, style);

	::SetWindowPos(this->GetSafeHwnd(), NULL, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void CChatDlg::SendChatMessage(CString time, CString chat)
{
	time = time.Mid(11, 5);
	PaneWnd* mainView = (PaneWnd*)mainChatView->GetElement(1);
	EditWnd* edit = (EditWnd*)mainView->FindElement(_T("MESSAGE"));

	PaneWnd* message = new PaneWnd(DIRECTION::HORIZONTAL, _T(""), -1);
	((ScrollWnd*)mainView->GetElement(0))->AddElement(message);
	message->AddElement(new ItemWnd(TEXTSTRUCT(time, TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::BOTTOM, TEXTSTRUCT::HALIGN::RIGHT, _T("Segoe UI"), 8), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(0, 12, -8, 4)), _T(""), -1));

	ItemWnd* item = new ItemWnd(TEXTSTRUCT(chat, TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 0), 30), _T("TEXT"), 100, RGB(58, 174, 169));
	message->AddElement(item);

	CRect rect;
	((ScrollWnd*)mainView->GetElement(0))->GetClientRect(&rect);
	item->SetSize(min(max(200, rect.Width() * 0.6), item->GetTextWidth()));
	edit->ClearText();

	mainView->GetElement(0)->SetRedraw(FALSE);
	mainView->GetElement(0)->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
	mainView->GetElement(0)->SetRedraw(TRUE);
	((ScrollWnd*)mainView->GetElement(0))->MoveToEnd();
}

void CChatDlg::ReceiveChatMessage(CString time, CString chat)
{
	time = time.Mid(11, 5);
	PaneWnd* mainView = (PaneWnd*)mainChatView->GetElement(1);
	PaneWnd* message = new PaneWnd(DIRECTION::HORIZONTAL, _T(""), -1);
	((ScrollWnd*)mainView->GetElement(0))->AddElement(message);

	ItemWnd* item = new ItemWnd(TEXTSTRUCT(chat, TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 0), 30), _T("TEXT"), 100, RGB(229, 239, 193));
	message->AddElement(item);
	message->AddElement(new ItemWnd(TEXTSTRUCT(time, TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::BOTTOM, TEXTSTRUCT::HALIGN::LEFT, _T("Segoe UI"), 8), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(-8, 12, 0, 4)), _T(""), -1));

	CRect rect;
	((ScrollWnd*)mainView->GetElement(0))->GetClientRect(&rect);
	item->SetSize(min(max(200, rect.Width() * 0.6), item->GetTextWidth()));

	mainView->GetElement(0)->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
}