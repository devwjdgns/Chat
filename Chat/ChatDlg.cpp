
// ChatDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Chat.h"
#include "ChatDlg.h"
#include "afxdialogex.h"

#include "utility.h"
#include "FriendDlg.h"
#include "RoomDlg.h"

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
int g_nDeleteFriend = -1;
int g_nSelectFriend = -1;
int g_nDeleteRoom = -1;
int g_nSelectRoom = -1;

CChatDlg::CChatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	loginView = nullptr;
	registerView = nullptr;
	mainChatView = nullptr;
	chatManager = std::make_shared<ChatManager>();
	friendDlg = std::make_unique<CFriendDlg>(chatManager);
	roomDlg = std::make_unique<CRoomDlg>(chatManager);
	gdiplusToken = 0;
	roomId = -1;
	setlocale(LC_TIME, "ko-KR");
}

CChatDlg::~CChatDlg()
{
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
	return friendDlg.get();
}

CRoomDlg* CChatDlg::GetRoomDlg()
{
	return roomDlg.get();
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
	ON_MESSAGE(WM_BUTTON_MENU, &CChatDlg::OnButtonMenu)
	ON_MESSAGE(WM_BUTTON_CLICK, &CChatDlg::OnButtonClick)
	ON_MESSAGE(WM_EDIT_COMPLETE, &CChatDlg::OnEditComplete)
	ON_MESSAGE(WM_LOGIN_ACTION, &CChatDlg::OnLoginAction)
	ON_MESSAGE(WM_REGISTER_ACTION, &CChatDlg::OnRegisterAction)
	ON_MESSAGE(WM_LOGOUT_ACTION, &CChatDlg::OnLogoutAction)
	ON_MESSAGE(WM_ADD_FRIEND_ACTION, &CChatDlg::OnAddFriendAction)
	ON_MESSAGE(WM_DELETE_FRIEND_ACTION, &CChatDlg::OnDeleteFriendAction)
	ON_MESSAGE(WM_SEARCH_FRIEND_ACTION, &CChatDlg::OnSearchFriendAction)
	ON_MESSAGE(WM_OPEN_FRIEND_ACTION, &CChatDlg::OnOpenFriendAction)
	ON_MESSAGE(WM_CREATE_ROOM_ACTION, &CChatDlg::OnCreateRoomAction)
	ON_MESSAGE(WM_DELETE_ROOM_ACTION, &CChatDlg::OnDeleteRoomAction)
	ON_MESSAGE(WM_SEARCH_ROOM_ACTION, &CChatDlg::OnSearchRoomAction)
	ON_MESSAGE(WM_SEARCH_MESSAGE_ACTION, &CChatDlg::OnSearchMessageAction)
	ON_MESSAGE(WM_UPDATE_MESSAGE_ACTION, &CChatDlg::OnUpdateMessageAction)
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

	InitEventHandler();
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
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

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
		PaneWnd* mainView = mainChatView->GetElement<PaneWnd>(1);
		if (!mainView) return;
		ScrollWnd* chatView = mainView->GetElement<ScrollWnd>(0);
		if (!chatView) return;
		PaneWnd* message = nullptr;
		int cnt = chatView->GetElementCount();
		for (int i = 0; i < cnt; i++)
		{
			message = (chatView)->GetElement<PaneWnd>(i);
			if (!message) continue;

			if (auto text = message->GetElement<ItemWnd>(1))
			{
				text->SetSize(min(max(200, (cx - 400) * 0.6), text->GetTextWidth()));
			}
		}
		mainChatView->MoveWindow(0, 0, cx, cy);
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
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

void CChatDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 1500;
	lpMMI->ptMinTrackSize.y = 1200;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

LRESULT CChatDlg::OnButtonMenu(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = (CString*)wParam;
	if (pStr)
	{
		CString str = *pStr;
		delete pStr;

		if (str.Find(_T("FRIEND")) >= 0)
		{
			if (str.Find(_T("Delete")) >= 0)
			{
				g_nDeleteFriend = _ttoi(trimFromAffix(str, _T("FRIEND"), _T("^")));
				if (auto sideView = mainChatView->GetElement<PaneWnd>(0))
				{
					if (auto friendView = sideView->GetElement<ScrollWnd>(1))
					{
						if (auto pBtn = friendView->GetElement<ButtonWnd>(g_nDeleteFriend))
						{
							chatManager->deleteFriend(convertString(trimFromAffix(pBtn->GetItemText(), _T("("), _T(")"))));
						}
					}
				}
				ClearMainChatView();
			}
		}
		else if (str.Find(_T("ROOM")) >= 0)
		{
			if (str.Find(_T("Delete")) >= 0)
			{
				g_nDeleteRoom = _ttoi(trimFromAffix(str, _T("ROOM"), _T("^")));
				chatManager->deleteRoom(_ttoi(trimFromAffix(str, _T("("), _T(")"))));
				ClearMainChatView();
			}
		}
	}
	return 0;
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
			CString account, password;
			if (auto edit = loginView->GetElement<EditWnd>(1))
			{
				account = edit->GetItemText();
			}
			if (auto edit = loginView->GetElement<EditWnd>(3))
			{
				password = edit->GetItemText();
			}
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
			chatManager->loginAccount(convertString(account), convertString(password));
		}
		else if (str.Compare(_T("LOGOUT")) == 0)
		{
			MovePage(PAGE_NAME::LOGIN);
			chatManager->logoutAccount();
			ClearMainChatView();
		}
		else if (str.Compare(_T("MOVEREGISTER")) == 0)
		{
			MovePage(PAGE_NAME::REGISTER);
		}
		else if (str.Compare(_T("REGISTER")) == 0)
		{
			CString name, account, password;
			if (auto edit = registerView->GetElement<EditWnd>(1))
			{
				name = edit->GetItemText();
			}
			if (auto edit = registerView->GetElement<EditWnd>(3))
			{
				account = edit->GetItemText();
			}
			if (auto edit = registerView->GetElement<EditWnd>(5))
			{
				password = edit->GetItemText();
			}
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
			chatManager->registerAccount(convertString(name), convertString(account), convertString(password));
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
			SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		else if (str.Compare(_T("CREATEROOM")) == 0)
		{
			CRect rect;
			GetWindowRect(&rect);
			int centerX = (rect.left + rect.right) / 2;
			int centerY = (rect.top + rect.bottom) / 2;

			CArray<CString, CString> friends;
			PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
			if (!sideView)return 0;
			ScrollWnd* friendView = sideView->GetElement<ScrollWnd>(1);
			if (!friendView)return 0;
			int count = friendView->GetElementCount();
			for (int i = 0; i < count; i++)
			{
				if (auto item = friendView->GetElement<ItemWnd>(i))
				{
					friends.Add(item->GetItemText());
				}
			}
			EnableWindow(FALSE);
			if (roomDlg->CreateWnd(this, CRect(centerX - 250, centerY - 360, centerX + 250, centerY + 360), friends))
			{
				roomDlg->ShowWindow(SW_SHOW);
				roomDlg->UpdateWindow();
				roomDlg->RunModalLoop();
				roomDlg->DestroyWindow();
			}
			EnableWindow(TRUE);
			SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		else if (str.Find(_T("FRIEND")) >= 0)
		{
			PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
			if (!sideView)return 0;
			ScrollWnd* friendView = sideView->GetElement<ScrollWnd>(1);
			if (!friendView)return 0;
			ScrollWnd* roomView = sideView->GetElement<ScrollWnd>(3);
			if (!roomView)return 0;
			ButtonWnd* pBtn = nullptr;
			if (g_nSelectFriend >= 0)
			{
				pBtn = friendView->GetElement<ButtonWnd>(g_nSelectFriend);
				if (pBtn)
				{
					pBtn->SetPressedStatus(FALSE);
					pBtn->Invalidate();
				}
				g_nSelectFriend = -1;
			}
			if (g_nSelectRoom >= 0)
			{
				pBtn = roomView->GetElement<ButtonWnd>(g_nSelectRoom);
				if (pBtn)
				{
					pBtn->SetPressedStatus(FALSE);
					pBtn->Invalidate();
				}
				g_nSelectRoom = -1;
			}
			g_nSelectFriend = _ttoi(trimFromAffix(str, _T("FRIEND")));
			pBtn = friendView->GetElement<ButtonWnd>(g_nSelectFriend);
			if (!pBtn)return 0;
			pBtn->SetPressedStatus(TRUE);
			pBtn->Invalidate();

			if (auto* element = mainChatView->FindElement<ElementWnd>(_T("MESSAGE")))
			{
				element->ShowWindow(SW_SHOW);
			}
			if (auto* element = mainChatView->FindElement<ElementWnd>(_T("SENDMESSAGE")))
			{
				element->ShowWindow(SW_SHOW);
			}

			CString account = trimFromAffix(pBtn->GetItemText(), _T("("), _T(")"));
			chatManager->searchMessage(convertString(account));
		}
		else if (str.Find(_T("ROOM")) >= 0)
		{
			PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
			if (!sideView)return 0;
			ScrollWnd* friendView = sideView->GetElement<ScrollWnd>(1);
			if (!friendView)return 0;
			ScrollWnd* roomView = sideView->GetElement<ScrollWnd>(3);
			if (!roomView)return 0;
			ButtonWnd* pBtn = nullptr;
			if (g_nSelectFriend >= 0)
			{
				pBtn = friendView->GetElement<ButtonWnd>(g_nSelectFriend);
				if (pBtn)
				{
					pBtn->SetPressedStatus(FALSE);
					pBtn->Invalidate();
				}
				g_nSelectFriend = -1;
			}
			if (g_nSelectRoom >= 0)
			{
				pBtn = roomView->GetElement<ButtonWnd>(g_nSelectRoom);
				if (pBtn)
				{
					pBtn->SetPressedStatus(FALSE);
					pBtn->Invalidate();
				}
				g_nSelectRoom = -1;
			}
			g_nSelectRoom = _ttoi(trimFromAffix(str, _T("ROOM")));
			pBtn = roomView->GetElement<ButtonWnd>(g_nSelectRoom);
			if (!pBtn)return 0;
			pBtn->SetPressedStatus(TRUE);
			pBtn->Invalidate();

			if (auto* element = mainChatView->FindElement<ElementWnd>(_T("MESSAGE")))
			{
				element->ShowWindow(SW_SHOW);
			}
			if (auto* element = mainChatView->FindElement<ElementWnd>(_T("SENDMESSAGE")))
			{
				element->ShowWindow(SW_SHOW);
			}

			chatManager->searchMessage(_ttoi(trimFromAffix(str, _T("("), _T(")"))));
		}
		else if (str.Compare(_T("SENDMESSAGE")) == 0)
		{
			if(auto mainView = mainChatView->GetElement<PaneWnd>(1))
			{
				if (auto* edit = mainView->FindElement<EditWnd>(_T("MESSAGE")))
				{
					if (edit->GetItemText().IsEmpty())
					{
						return 0;
					}
					if (roomId >= 0)
					{
						chatManager->sendMessage(roomId, convertString(edit->GetItemText()), convertString(getCurrentDateTimeString()));
					}
				}
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
			if (auto mainView = mainChatView->GetElement<PaneWnd>(1))
			{
				if (auto edit = mainView->FindElement<EditWnd>(_T("MESSAGE")))
				{
					if (edit->GetItemText().IsEmpty())
						return 0;

					if (roomId >= 0)
					{
						chatManager->sendMessage(roomId, convertString(edit->GetItemText()), convertString(getCurrentDateTimeString()));
					}
				}
			}
			
		}
	}
	return 0;
}

LRESULT CChatDlg::OnLoginAction(WPARAM wParam, LPARAM lParam)
{
	CString* result = reinterpret_cast<CString*>(lParam);
	if (static_cast<BOOL>(wParam))
	{
		CString account, password;
		if (auto edit = loginView->GetElement<EditWnd>(1))
		{
			account = edit->GetItemText();
		}
		if (auto edit = loginView->GetElement<EditWnd>(3))
		{
			password = edit->GetItemText();
		}
		saveToRegistry(_T("Account"), account);
		saveToRegistry(_T("Password"), password);
		chatManager->searchFriend();
		chatManager->searchRoom();
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
		if (auto edit = registerView->GetElement<EditWnd>(1))
		{
			edit->ClearText();
		}
		if (auto edit = registerView->GetElement<EditWnd>(3))
		{
			edit->ClearText();
		}
		if (auto edit = registerView->GetElement<EditWnd>(5))
		{
			edit->ClearText();
		}
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

LRESULT CChatDlg::OnAddFriendAction(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR pStr = (LPCTSTR)lParam;
	CString text(pStr);

	PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
	if (!sideView)return 0;
	if (auto friendView = sideView->GetElement<ScrollWnd>(1))
	{
		CString name;
		name.Format(_T("FRIEND%d"), friendView->GetElementCount());
		ButtonWnd* pBtn = new ButtonWnd(TEXTSTRUCT(text), SHAPESTRUCT(), name, 70);
		pBtn->AddContextMenu(_T("Delete"));
		friendView->AddElement(pBtn);

		CRect rect;
		friendView->GetClientRect(rect);
		friendView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
	}
	
	return 0;
}

LRESULT CChatDlg::OnDeleteFriendAction(WPARAM wParam, LPARAM lParam)
{
	CString* result = reinterpret_cast<CString*>(lParam);
	if (static_cast<BOOL>(wParam))
	{
		PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
		if (!sideView)return 0;
		if (auto friendView = sideView->GetElement<ScrollWnd>(1))
		{
			friendView->DeleteElement(g_nDeleteFriend);
			if (g_nDeleteFriend == g_nSelectFriend) g_nSelectFriend = -1;
			g_nDeleteFriend = -1;

			CRect rect;
			friendView->GetClientRect(rect);
			friendView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
		}
	}
	else
	{
		MessageBox(*result, _T("Notice"), MB_OK | MB_ICONINFORMATION);
	}
	delete result;
	return 0;
}

LRESULT CChatDlg::OnSearchFriendAction(WPARAM wParam, LPARAM lParam)
{
	int count = (int)wParam;
	CString* result = reinterpret_cast<CString*>(lParam);
	PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
	if (!sideView)
	{
		delete[] result;
		return 0;
	}
	if (auto friendView = sideView->GetElement<ScrollWnd>(1))
	{
		CString selFriendName;
		if (g_nSelectFriend >= 0)
		{
			if (auto item = friendView->GetElement<ItemWnd>(g_nSelectFriend))
			{
				selFriendName = item->GetItemText();
			}
		}
		friendView->ClearElement();
		BOOL selFriend = FALSE;
		for (int i = 0; i < count; ++i)
		{
			CString name;
			name.Format(_T("FRIEND%d"), i);
			ButtonWnd* pBtn = new ButtonWnd(TEXTSTRUCT(result[i]), SHAPESTRUCT(), name, 70);
			pBtn->AddContextMenu(_T("Delete"));
			friendView->AddElement(pBtn);
			if (selFriendName.Compare(result[i]) == 0)
			{
				g_nSelectFriend = i;
				pBtn->SetPressedStatus(TRUE);
				selFriend = TRUE;
			}
		}
		if (!selFriend)
		{
			ClearMainChatView();
		}

		CRect rect;
		friendView->GetClientRect(rect);
		friendView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
	}
	delete[] result;
		
	return 0;
}

LRESULT CChatDlg::OnOpenFriendAction(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR pId = (LPCTSTR)wParam;
	CString id(pId);

	PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
	if (!sideView)return 0;

	if (auto friendView = sideView->GetElement<ScrollWnd>(1))
	{
		int cnt = friendView->GetElementCount();
		for (int i = 0; i < cnt; i++)
		{
			if (ButtonWnd* pBtn = friendView->GetElement<ButtonWnd>(i))
			{
				CString text = pBtn->GetItemText();
				if (text.Compare(id) == 0)
				{
					OnButtonClick((WPARAM)new CString(pBtn->GetName()), 0);
					return 0;
				}
			}
		}
	}
	
	return 0;
}

LRESULT CChatDlg::OnCreateRoomAction(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR pId = (LPCTSTR)wParam;
	LPCTSTR pStr = (LPCTSTR)lParam;
	CString id(pId);
	CString text(pStr);

	PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
	if (!sideView)return 0;

	if (auto roomView = sideView->GetElement<ScrollWnd>(3))
	{
		CString name;
		name.Format(_T("(%s)ROOM%d"), id, roomView->GetElementCount());
		ButtonWnd* pBtn = new ButtonWnd(TEXTSTRUCT(text), SHAPESTRUCT(), name, 70);
		pBtn->AddContextMenu(_T("Delete"));
		roomView->AddElement(pBtn);

		CRect rect;
		roomView->GetClientRect(rect);
		roomView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));

		OnButtonClick((WPARAM)new CString(name), 0);
	}
	
	return 0;
}

LRESULT CChatDlg::OnDeleteRoomAction(WPARAM wParam, LPARAM lParam)
{
	CString* result = reinterpret_cast<CString*>(lParam);
	if (static_cast<BOOL>(wParam))
	{
		PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
		if (!sideView)return 0;

		if (auto roomView = sideView->GetElement<ScrollWnd>(3))
		{
			roomView->DeleteElement(g_nDeleteRoom);
			if (g_nDeleteRoom == g_nSelectRoom) g_nSelectRoom = -1;
			g_nDeleteRoom = -1;

			CRect rect;
			roomView->GetClientRect(rect);
			roomView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
		}
	}
	else
	{
		MessageBox(*result, _T("Notice"), MB_OK | MB_ICONINFORMATION);
	}
	delete result;
	return 0;
}

LRESULT CChatDlg::OnSearchRoomAction(WPARAM wParam, LPARAM lParam)
{
	int count = (int)wParam;
	CString* result = reinterpret_cast<CString*>(lParam);

	PaneWnd* sideView = mainChatView->GetElement<PaneWnd>(0);
	if (!sideView)
	{
		delete[] result;
		return 0;
	}

	if (auto roomView = sideView->GetElement<ScrollWnd>(3))
	{
		roomView->ClearElement();
		for (int i = 0; i < count; ++i)
		{
			CString id = trimFromAffix(result[i], _T("("), _T(")"));
			CString text = trimFromAffix(result[i], _T(")"));
			CString name;
			name.Format(_T("(%s)ROOM%d"), id, i);
			ButtonWnd* pBtn = new ButtonWnd(TEXTSTRUCT(text), SHAPESTRUCT(), name, 70);
			pBtn->AddContextMenu(_T("Delete"));
			roomView->AddElement(pBtn);
		}

		CRect rect;
		roomView->GetClientRect(rect);
		roomView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
	}
	delete[] result;
	
	return 0;
}

LRESULT CChatDlg::OnSearchMessageAction(WPARAM wParam, LPARAM lParam)
{
	MessageData* result = reinterpret_cast<MessageData*>(lParam);
	PaneWnd* mainView = mainChatView->GetElement<PaneWnd>(1);
	if (!mainView)
	{
		delete[] result;
		return 0;
	}

	if (auto chatView = mainView->GetElement<ScrollWnd>(0))
	{
		chatView->ClearElement();
		messageDate.Empty();

		int count = (int)wParam;
		for (int i = 0; i < count; ++i)
		{
			if (result[i].name.empty())
			{
				SendChatMessage(convertString(result[i].message), convertString(result[i].timestamp), FALSE);
			}
			else
			{
				ReceiveChatMessage(convertString(result[i].name), convertString(result[i].message), convertString(result[i].timestamp), FALSE);
			}
		}
		CRect rect;
		chatView->GetClientRect(&rect);
		chatView->SetRedraw(FALSE);
		chatView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
		chatView->SetRedraw(TRUE);
		chatView->MoveToEnd();
	}
	delete[] result;

	return 0;
}

LRESULT CChatDlg::OnUpdateMessageAction(WPARAM wParam, LPARAM lParam)
{
	MessageData* result = reinterpret_cast<MessageData*>(lParam);
	if (result->name.empty())
	{
		SendChatMessage(convertString(result->message), convertString(result->timestamp));
	}
	else
	{
		ReceiveChatMessage(convertString(result->name), convertString(result->message), convertString(result->timestamp));
	}
	delete result;
	return 0;
}

void CChatDlg::InitEventHandler()
{
	std::unordered_map<std::string, std::function<void(const int&, const IData*)>> handlers;

	handlers["room_info"] = [this](const int& state, const IData* data) {
		roomId = state;
		};

	handlers["register"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<StringData>(data))
		{
			CString* result = new CString(CA2T(tmp->str.c_str()));
			::PostMessage(GetSafeHwnd(), WM_REGISTER_ACTION, state, reinterpret_cast<LPARAM>(result));
		}
		};

	handlers["login"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<StringData>(data))
		{
			CString* result = new CString(CA2T(tmp->str.c_str()));
			::PostMessage(GetSafeHwnd(), WM_LOGIN_ACTION, state, reinterpret_cast<LPARAM>(result));
		}
		};

	handlers["logout"] = [this](const int& state, const IData* data) {
		roomId = state;
		};

	handlers["search_user"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<ListData>(data))
		{
			int count = static_cast<int>(tmp->list.size());
			CString* result = new CString[count];

			for (int i = 0; i < count; ++i)
			{
				result[i] = convertString(tmp->list[i]);
			}

			::PostMessage(friendDlg->GetSafeHwnd(), WM_SEARCH_USER_ACTION, (WPARAM)count, reinterpret_cast<LPARAM>(result));
		}
		};

	handlers["add_friend"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<StringData>(data))
		{
			CString* result = new CString(CA2T(tmp->str.c_str()));
			::PostMessage(friendDlg->GetSafeHwnd(), WM_ADD_FRIEND_ACTION, state, reinterpret_cast<LPARAM>(result));
		}
		};

	handlers["delete_friend"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<StringData>(data))
		{
			CString* result = new CString(CA2T(tmp->str.c_str()));
			::PostMessage(GetSafeHwnd(), WM_DELETE_FRIEND_ACTION, state, reinterpret_cast<LPARAM>(result));
		}
		};

	handlers["search_friend"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<ListData>(data))
		{
			int count = static_cast<int>(tmp->list.size());
			CString* result = new CString[count];

			for (int i = 0; i < count; ++i)
			{
				result[i] = convertString(tmp->list[i]);
			}

			::PostMessage(GetSafeHwnd(), WM_SEARCH_FRIEND_ACTION, (WPARAM)count, reinterpret_cast<LPARAM>(result));
		}
		};

	handlers["create_room"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<StringData>(data))
		{
			CString* result = new CString(CA2T(tmp->str.c_str()));
			::PostMessage(roomDlg->GetSafeHwnd(), WM_CREATE_ROOM_ACTION, state, reinterpret_cast<LPARAM>(result));
		}
		};

	handlers["delete_room"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<StringData>(data))
		{
			CString* result = new CString(CA2T(tmp->str.c_str()));
			::PostMessage(GetSafeHwnd(), WM_DELETE_ROOM_ACTION, state, reinterpret_cast<LPARAM>(result));
		}
		};

	handlers["search_room"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<ListData>(data))
		{
			int count = static_cast<int>(tmp->list.size());
			CString* result = new CString[count];

			for (int i = 0; i < count; ++i)
			{
				result[i] = convertString(tmp->list[i]);
			}

			::PostMessage(GetSafeHwnd(), WM_SEARCH_ROOM_ACTION, (WPARAM)count, reinterpret_cast<LPARAM>(result));
		}
		};

	handlers["search_message"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<MessageData>(data))
		{
			::PostMessage(GetSafeHwnd(), WM_SEARCH_MESSAGE_ACTION, (WPARAM)state, reinterpret_cast<LPARAM>(data));
		}
		};
	
	handlers["update_message"] = [this](const int& state, const IData* data) {
		if (auto tmp = IData::GetData<MessageData>(data))
		{
			if(roomId == state) 
			{
				::PostMessage(GetSafeHwnd(), WM_UPDATE_MESSAGE_ACTION, (WPARAM)0, reinterpret_cast<LPARAM>(data));
			}
			else
			{
				delete data;
			}
		}
		};

	chatManager->setEventHandler([this, handlers](const std::string& type, const int& state, const IData* msg) mutable {
		auto it = handlers.find(type);
		if (it != handlers.end()) {
			it->second(state, msg);
		}
		});
}

void CChatDlg::InitLoginView()
{
	CRect rect;
	GetClientRect(&rect);
	ElementWnd* tmpWnd = nullptr;
	loginView = std::make_unique<PaneWnd>(DIRECTION::VERTICAL, _T(""), -1, RGB(255,255,255));
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
	ElementWnd* tmpWnd = nullptr;
	registerView = std::make_unique<PaneWnd>(DIRECTION::VERTICAL, _T(""), -1, RGB(255, 255, 255));
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
	ElementWnd* tmpWnd = nullptr;
	PaneWnd* tmpPane = nullptr;

	mainChatView = std::make_unique<PaneWnd>(DIRECTION::HORIZONTAL, _T(""), -1, RGB(255, 255, 255));
	mainChatView->Initialize(this, rect);

	PaneWnd* sideView = new PaneWnd(DIRECTION::VERTICAL, _T("SIDEVIEW"), 400, RGB(242, 242, 249));
	PaneWnd* chatView = new PaneWnd(DIRECTION::VERTICAL, _T("CHATVIEW"), -1, RGB(255, 255, 255));
	mainChatView->AddElement(sideView);
	mainChatView->AddElement(chatView);

	tmpPane = dynamic_cast<PaneWnd*>(sideView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 60)));
	if (tmpPane)
	{
		tmpPane->AddElement(new ItemWnd(TEXTSTRUCT(_T("Friends"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(10, 10, 10, 0)), _T(""), -1));
		tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::PLUS, CRect(10, 20, 20, 10)), _T("ADDFRIEND"), 60, sideView->GetBackgroundColor(), 0));
	}
	sideView->AddElement(new ScrollWnd(DIRECTION::VERTICAL, _T("FRIENDS"), 500));

	tmpPane = dynamic_cast<PaneWnd*>(sideView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 60)));
	if (tmpPane)
	{
		tmpPane->AddElement(new ItemWnd(TEXTSTRUCT(_T("Rooms"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(10, 10, 10, 0)), _T(""), -1));
		tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::PLUS, CRect(10, 20, 20, 10)), _T("CREATEROOM"), 60, sideView->GetBackgroundColor(), 0));
	}
	sideView->AddElement(new ScrollWnd(DIRECTION::VERTICAL, _T("ROOMS"), -1));
	sideView->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Logout")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(20, 10, 20, 10), 12), _T("LOGOUT"), 70, RGB(232, 106, 106)));

	ScrollWnd* chatWnd = new ScrollWnd(DIRECTION::VERTICAL, _T("CHAT"), -1);
	chatWnd->UsePaintEx();
	chatView->AddElement(chatWnd);
	tmpPane = dynamic_cast<PaneWnd*>(chatView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T("INPUT"), 70)));

	if (tmpPane)
	{
		tmpPane->AddElement(new EditWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 10), 10), _T("MESSAGE"), -1, RGB(255, 255, 255), RGB(125, 125, 125)));
		tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Send"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(0, 10, 10, 10), 10), _T("SENDMESSAGE"), 80, RGB(86, 124, 131)));
	}
	if (auto* element = mainChatView->FindElement<ElementWnd>(_T("MESSAGE")))
	{
		element->ShowWindow(SW_HIDE);
	}
	if (auto* element = mainChatView->FindElement<ElementWnd>(_T("SENDMESSAGE")))
	{
		element->ShowWindow(SW_HIDE);
	}
}

void CChatDlg::ClearMainChatView()
{
	g_nSelectFriend = -1;
	g_nSelectRoom = -1;

	messageDate.Empty();

	PaneWnd* mainView = mainChatView->GetElement<PaneWnd>(1);
	if (!mainView)return;
	if (auto chatView = mainView->GetElement<ScrollWnd>(0))
	{
		chatView->ClearElement();
		CRect rect;
		chatView->GetClientRect(&rect);
		chatView->SetRedraw(FALSE);
		chatView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
		chatView->SetRedraw(TRUE);
		chatView->MoveToEnd();
	}
	
	if (auto* element = mainChatView->FindElement<ElementWnd>(_T("MESSAGE")))
	{
		element->ShowWindow(SW_HIDE);
	}
	if (auto* element = mainChatView->FindElement<ElementWnd>(_T("SENDMESSAGE")))
	{
		element->ShowWindow(SW_HIDE);
	}
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

void CChatDlg::SendChatMessage(CString message, CString timestamp, BOOL update)
{
	PaneWnd* mainView = mainChatView->GetElement<PaneWnd>(1);
	if (!mainView)return;

	if (auto chatView = mainView->GetElement<ScrollWnd>(0))
	{
		CRect rect;
		chatView->GetClientRect(&rect);

		if (timestamp.Left(10).Compare(messageDate) != 0)
		{
			CRect rect;
			mainView->GetWindowRect(&rect);
			messageDate = timestamp.Left(10);
			chatView->AddElement(new ItemWnd(TEXTSTRUCT(), SHAPESTRUCT(SHAPESTRUCT::SHAPE::LINE, CRect(30, 15, 30, 0)), _T(""), 25, chatView->GetBackgroundColor(), RGB(120, 120, 120)));

			int year, month, day;
			_stscanf_s(messageDate, _T("%d-%d-%d"), &year, &month, &day);
			CTime date(year, month, day, 0, 0, 0);

			CString dateFormat = date.Format(_T("%Y년 %m월 %d일 %A"));
			chatView->AddElement(new ItemWnd(TEXTSTRUCT(dateFormat, TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::CENTER, _T("Segoe UI"), 10, RGB(120, 120, 120)), SHAPESTRUCT(SHAPESTRUCT::SHAPE::LINE, CRect(0, 0, 0, 5)), _T(""), 25));
		}

		int messageCnt = chatView->GetElementCount();
		PaneWnd* messageWnd = new PaneWnd(DIRECTION::HORIZONTAL, _T("ONE"), -1);
		chatView->AddElement(messageWnd);
		messageWnd->AddElement(new ItemWnd(TEXTSTRUCT(timestamp.Mid(11, 5), TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::BOTTOM, TEXTSTRUCT::HALIGN::RIGHT, _T("Segoe UI"), 8), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(0, 16, -8, 0)), _T(""), -1));

		ItemWnd* itemWnd = new ItemWnd(TEXTSTRUCT(message, TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 0), 30), _T("TEXT"), 100, RGB(58, 174, 169));
		messageWnd->AddElement(itemWnd);
		itemWnd->SetSize(min(max(200, rect.Width() * 0.6), itemWnd->GetTextWidth()));

		if (PaneWnd* prevMessageWnd = chatView->GetElement<PaneWnd>(messageCnt - 1))
		{
			CString prevName = prevMessageWnd->GetName();
			if (prevName.Compare(_T("ONE")) == 0)
			{
				if (auto item = prevMessageWnd->GetElement<ItemWnd>(0))
				{
					CString prevTimestamp = item->GetItemText();
					if (prevTimestamp.Compare(timestamp.Mid(11, 5)) == 0)
					{
						item->ShowWindowExtern(FALSE);
					}
				}
			}
		}

		if (update)
		{
			if (auto edit = mainView->FindElement<EditWnd>(_T("MESSAGE")))
			{
				edit->ClearText();
			}
			chatView->SetRedraw(FALSE);
			chatView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
			chatView->SetRedraw(TRUE);
			chatView->MoveToEnd();
		}
	}
}

void CChatDlg::ReceiveChatMessage(CString name, CString message, CString timestamp, BOOL update)
{
	PaneWnd* mainView = mainChatView->GetElement<PaneWnd>(1);
	if (!mainView)return;

	if (auto chatView = mainView->GetElement<ScrollWnd>(0))
	{
		CRect rect;
		chatView->GetClientRect(&rect);

		if (timestamp.Left(10).Compare(messageDate) != 0)
		{
			CRect rect;
			mainView->GetWindowRect(&rect);
			messageDate = timestamp.Left(10);
			chatView->AddElement(new ItemWnd(TEXTSTRUCT(), SHAPESTRUCT(SHAPESTRUCT::SHAPE::LINE, CRect(30, 15, 30, 0)), _T(""), 25, chatView->GetBackgroundColor(), RGB(120, 120, 120)));

			int year, month, day;
			_stscanf_s(messageDate, _T("%d-%d-%d"), &year, &month, &day);
			CTime date(year, month, day, 0, 0, 0);

			CString dateFormat = date.Format(_T("%Y년 %m월 %d일 %A"));
			chatView->AddElement(new ItemWnd(TEXTSTRUCT(dateFormat, TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::CENTER, _T("Segoe UI"), 10, RGB(120, 120, 120)), SHAPESTRUCT(SHAPESTRUCT::SHAPE::LINE, CRect(0, 0, 0, 5)), _T(""), 25));
		}

		int messageCnt = chatView->GetElementCount();
		PaneWnd* messageWnd = new PaneWnd(DIRECTION::HORIZONTAL, name, -1);
		chatView->AddElement(messageWnd);

		messageWnd->AddElement(new PaneWnd(DIRECTION::VERTICAL, _T(""), 60));
		messageWnd->GetElement<PaneWnd>(0)->AddElement(new ItemWnd(TEXTSTRUCT(name.Left(1), TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::CENTER, _T("Segoe UI"), 12), SHAPESTRUCT(SHAPESTRUCT::SHAPE::CIRCLE, CRect(5, 0, 5, 5)), _T(""), 60, RGB(220, 220, 220)));

		ItemWnd* itemWnd = new ItemWnd(TEXTSTRUCT(message, TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::LEFT), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 0), 30), _T("TEXT"), 300, RGB(229, 239, 193));
		messageWnd->AddElement(itemWnd);
		messageWnd->AddElement(new ItemWnd(TEXTSTRUCT(timestamp.Mid(11, 5), TEXTSTRUCT::STYLE::NORMAL, TEXTSTRUCT::VALIGN::BOTTOM, TEXTSTRUCT::HALIGN::LEFT, _T("Segoe UI"), 8), SHAPESTRUCT(SHAPESTRUCT::SHAPE::NONE, CRect(-8, 16, 0, 0)), _T(""), -1));
		itemWnd->SetSize(min(max(200, rect.Width() * 0.6), itemWnd->GetTextWidth()));

		if (PaneWnd* prevMessageWnd = chatView->GetElement<PaneWnd>(messageCnt - 1))
		{
			CString prevName = prevMessageWnd->GetName();
			if (prevName.Compare(name) == 0)
			{
				messageWnd->GetElement<PaneWnd>(0)->ShowWindowExtern(FALSE);
				if (auto item = prevMessageWnd->GetElement<ItemWnd>(2))
				{
					CString prevTimestamp = item->GetItemText();
					if (prevTimestamp.Compare(timestamp.Mid(11, 5)) == 0)
					{
						item->ShowWindowExtern(FALSE);
					}
				}
			}
		}

		if (update)
		{
			chatView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
		}
	}
}