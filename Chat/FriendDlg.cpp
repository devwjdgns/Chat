#include "pch.h"
#include "framework.h"
#include "FriendDlg.h"
#include "../ChatCore/ChatManager.h"
#include "utility.h"

#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

CFriendDlg::CFriendDlg(std::shared_ptr<ChatManager> cm) : chatManager(cm)
{
	mainView = nullptr;
	sel = -1;
}

CFriendDlg::~CFriendDlg()
{
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
	ElementWnd* tmpWnd = nullptr;
	PaneWnd* tmpPane = nullptr;
	
	mainView = std::make_unique<PaneWnd>(DIRECTION::VERTICAL, _T(""), -1, RGB(255, 255, 255));
	mainView->Initialize(this, rect);
	tmpPane = dynamic_cast<PaneWnd*>(mainView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 70)));
	if (tmpPane)
	{
		tmpPane->AddElement(new EditWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 10), 10), _T("SEARCHEDIT"), -1, RGB(255, 255, 255), RGB(125, 125, 125)));
		tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Search"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(0, 10, 10, 10), 10), _T("SEARCH"), 100, RGB(86, 124, 131)));
	}
	mainView->AddElement(new ScrollWnd(DIRECTION::VERTICAL, _T("FRIENDLIST"), 460));

	tmpPane = dynamic_cast<PaneWnd*>(mainView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 70)));
	tmpPane->AddElement(new ItemWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(), _T(""), 135, RGB(255, 255, 255)));
	tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Add"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 5, 10), 10), _T("ADD"), 120, RGB(98, 164, 188)));
	tmpPane->GetElement<ButtonWnd>(1)->SetEnableStatus(FALSE);
	tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Close"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(5, 10, 10, 10), 10), _T("CLOSE"), 120, RGB(200, 200, 200)));
	tmpPane->AddElement(new ItemWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(), _T(""), 135, RGB(255, 255, 255)));
	mainView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
	return TRUE;
}

BEGIN_MESSAGE_MAP(CFriendDlg, CWnd)
    ON_WM_PAINT()
	ON_MESSAGE(WM_BUTTON_CLICK, &CFriendDlg::OnButtonClick)
	ON_MESSAGE(WM_SEARCH_USER_ACTION, &CFriendDlg::OnSearchUserAction)
	ON_MESSAGE(WM_ADD_FRIEND_ACTION, &CFriendDlg::OnAddFriendAction)
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
			PaneWnd* paneWnd = mainView->GetElement<PaneWnd>(0);
			if (paneWnd)
			{
				if (auto edit = paneWnd->GetElement<EditWnd>(0))
				{
					CString text = edit->GetItemText();
					chatManager->searchUser(convertString(text));
					sel = -1;
				}
			}
		}
		else if (str.Find(_T("USER")) >= 0)
		{
			ButtonWnd* pBtn = nullptr;
			if (sel >= 0)
			{
				if (auto scroll = mainView->GetElement<ScrollWnd>(1))
				{
					pBtn = scroll->GetElement<ButtonWnd>(sel);
					if (pBtn)
					{
						pBtn->SetPressedStatus(FALSE);
						pBtn->Invalidate();
					}
				}
				
			}
			sel = _ttoi(trimFromAffix(str, _T("USER")));


			if (auto scroll = mainView->GetElement<ScrollWnd>(1))
			{
				pBtn = scroll->GetElement<ButtonWnd>(sel);
				if (pBtn)
				{
					pBtn->SetPressedStatus(TRUE);
				}
				pBtn = mainView->FindElement<ButtonWnd>(_T("ADD"));
				if (pBtn)
				{
					pBtn->SetEnableStatus(TRUE);
				}
			}
		}
		else if (str.Compare(_T("ADD")) == 0)
		{
			if (sel >= 0)
			{
				if (auto scroll = mainView->GetElement<ScrollWnd>(1))
				{
					ButtonWnd* pBtn = scroll->GetElement<ButtonWnd>(sel);
					CString account = trimFromAffix(pBtn->GetItemText(), _T("("), _T(")"));
					chatManager->addFriend(convertString(account));
				}
			}
		}
		else if (str.Compare(_T("CLOSE")) == 0)
		{
			EndModalLoop(IDOK);
		}
	}
	return 0;
}

LRESULT CFriendDlg::OnSearchUserAction(WPARAM wParam, LPARAM lParam)
{
	CString* result = reinterpret_cast<CString*>(lParam);
	int count = (int)wParam;
	ScrollWnd* scrollWnd = mainView->GetElement<ScrollWnd>(1);
	if (!scrollWnd)
	{
		delete[] result;
		return 0;
	}
	scrollWnd->ClearElement();

	for (int i = 0; i < count; ++i)
	{
		CString name;
		name.Format(_T("USER%d"), i);
		scrollWnd->AddElement(new ButtonWnd(TEXTSTRUCT(result[i]), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 5, 10, 5), 10), name, 70, RGB(245, 245, 245)));
	}
	CRect rect;
	scrollWnd->GetClientRect(&rect);
	scrollWnd->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));

	delete[] result;
	return 0;
}

LRESULT CFriendDlg::OnAddFriendAction(WPARAM wParam, LPARAM lParam)
{
	CString* result = reinterpret_cast<CString*>(lParam);
	if (static_cast<BOOL>(wParam))
	{
		delete result;
		ScrollWnd* scrollWnd = mainView->GetElement<ScrollWnd>(1);
		if (!scrollWnd)return 0;
		if (ButtonWnd* pBtn = scrollWnd->GetElement<ButtonWnd>(sel))
		{
			::SendMessage(GetParent()->GetSafeHwnd(), WM_ADD_FRIEND_ACTION, TRUE, (LPARAM)(LPCTSTR)pBtn->GetItemText());
		}
		EndModalLoop(IDOK);
	}
	else
	{
		MessageBox(*result, _T("Notice"), MB_OK | MB_ICONINFORMATION);
		delete result;
	}
	return 0;
}