#include "pch.h"
#include "framework.h"
#include "FriendDlg.h"
#include "ChatManager.h"

#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

CFriendDlg::CFriendDlg(ChatManager* cm) : chatManager(cm)
{
	mainView = NULL;
	sel = -1;
}

CFriendDlg::~CFriendDlg()
{
	delete mainView;
}

BOOL CFriendDlg::CreateWnd(CWnd* parent, CRect rect)
{
    CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL);

    CreateEx(0, className, _T("Friends"), WS_POPUP | WS_VISIBLE, rect, parent, 0);
	
	const int DWMWA_WINDOW_CORNER_PREFERENCE = 33;
	const int DWMWCP_ROUND = 2;
	DwmSetWindowAttribute(m_hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &DWMWCP_ROUND, sizeof(DWMWCP_ROUND));

	MARGINS shadow = { 1,1,1,1 };
	DwmExtendFrameIntoClientArea(m_hWnd, &shadow);
	
	GetClientRect(&rect);
	ElementWnd* tmpWnd = NULL;
	mainView = new PaneWnd(DIRECTION::VERTICAL, _T(""), -1, RGB(255, 255, 255));
	mainView->Initialize(this, rect);
	mainView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 70));
	((PaneWnd*)mainView->GetElement(0))->AddElement(new EditWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 10), 10), _T("SEARCHEDIT"), -1, RGB(255, 255, 255), RGB(125, 125, 125)));
	((PaneWnd*)mainView->GetElement(0))->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Search"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(0, 10, 10, 10), 10), _T("SEARCH"), 100, RGB(86, 124, 131)));

	mainView->AddElement(new ScrollWnd(DIRECTION::VERTICAL, _T("FRIENDLIST"), 460));

	PaneWnd* tmpPane = new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 70);
	mainView->AddElement(tmpPane);
	tmpPane->AddElement(new ItemWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(), _T(""), 135, RGB(255, 255, 255)));
	tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Add"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 5, 10), 10), _T("ADD"), 120, RGB(98, 164, 188)));
	((ButtonWnd*)tmpPane->GetElement(1))->SetEnableStatus(FALSE);
	tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Close"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(5, 10, 10, 10), 10), _T("CLOSE"), 120, RGB(200, 200, 200)));
	tmpPane->AddElement(new ItemWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(), _T(""), 135, RGB(255, 255, 255)));
	mainView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
	return TRUE;
}

BEGIN_MESSAGE_MAP(CFriendDlg, CWnd)
    ON_WM_PAINT()
	ON_MESSAGE(WM_BUTTON_CLICK, &CFriendDlg::OnButtonClick)
	ON_MESSAGE(WM_SEARCH_FRIEND_ACTION, &CFriendDlg::OnSearchFriendAction)
END_MESSAGE_MAP()

void CFriendDlg::OnPaint()
{
	CWnd::OnPaint();
}

LRESULT CFriendDlg::OnButtonClick(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = (CString*)wParam;
	if (pStr)
	{
		CString str = *pStr;
		delete pStr;

		if (str.Compare(_T("SEARCH")) == 0)
		{
			PaneWnd* paneWnd = dynamic_cast<PaneWnd*>(mainView->GetElement(0));
			if (paneWnd)
			{
				CString text = ((EditWnd*)paneWnd->GetElement(0))->GetItemText();
				chatManager->searchFriend(text);
				sel = -1;
			}
		}
		else if (str.Find(_T("Item")) >= 0)
		{
			ButtonWnd* pBtn = NULL;
			if (sel >= 0)
			{
				pBtn = dynamic_cast<ButtonWnd*>(((ScrollWnd*)mainView->GetElement(1))->GetElement(sel));
				if (pBtn)
				{
					pBtn->SetPressedStatus(FALSE);
					pBtn->Invalidate();
				}
			}
			sel = _ttoi(str.Mid(4));
			pBtn = dynamic_cast<ButtonWnd*>(((ScrollWnd*)mainView->GetElement(1))->GetElement(sel));
			if (pBtn)
			{
				pBtn->SetPressedStatus(TRUE);
			}
			pBtn = dynamic_cast<ButtonWnd*>(mainView->FindElement(_T("ADD")));
			if (pBtn)
			{
				pBtn->SetEnableStatus(TRUE);
			}
		}
		else if (str.Compare(_T("ADD")) == 0)
		{
			if (sel >= 0)
			{
				ButtonWnd* pBtn = dynamic_cast<ButtonWnd*>(((ScrollWnd*)mainView->GetElement(1))->GetElement(sel));
				CString account = pBtn->GetItemText();
				account = account.Mid(account.Find(_T("(")) + 1, account.GetLength() - account.Find(_T("(")) - 2);
				EndModalLoop(IDOK);
			}
		}
		else if (str.Compare(_T("CLOSE")) == 0)
		{
			EndModalLoop(IDOK);
		}
	}
	return 0;
}

LRESULT CFriendDlg::OnSearchFriendAction(WPARAM wParam, LPARAM lParam)
{
	((ScrollWnd*)mainView->GetElement(1))->ClearElement();
	int count = (int)wParam;
	CString* result = reinterpret_cast<CString*>(lParam);

	for (int i = 0; i < count; ++i)
	{
		CString name;
		name.Format(_T("Item%d"), i);
		((ScrollWnd*)mainView->GetElement(1))->AddElement(new ButtonWnd(TEXTSTRUCT(result[i]), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 5, 10, 5), 10), name, 70, RGB(245, 245, 245)));
	}
	CRect rect;
	((ScrollWnd*)mainView->GetElement(1))->GetClientRect(&rect);
	mainView->GetElement(1)->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));

	delete[] result;
	return 0;
}