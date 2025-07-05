#include "pch.h"
#include "framework.h"
#include "RoomDlg.h"
#include "ChatManager.h"
#include "utility.h"

#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

CRoomDlg::CRoomDlg(ChatManager* cm) : chatManager(cm)
{
	mainView = NULL;
}

CRoomDlg::~CRoomDlg()
{
	delete mainView;
}

BOOL CRoomDlg::CreateWnd(CWnd* parent, CRect rect, CArray<CString, CString>& friends)
{
    CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL);

    CreateEx(0, className, _T("Rooms"), WS_POPUP | WS_VISIBLE, rect, parent, 0);
	
	const int DWMWA_WINDOW_CORNER_PREFERENCE = 33;
	const int DWMWCP_ROUND = 2;
	DwmSetWindowAttribute(m_hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &DWMWCP_ROUND, sizeof(DWMWCP_ROUND));

	MARGINS shadow = { 1,1,1,1 };
	DwmExtendFrameIntoClientArea(m_hWnd, &shadow);
	
	GetClientRect(&rect);
	ElementWnd* tmpWnd = NULL;
	mainView = new PaneWnd(DIRECTION::VERTICAL, _T(""), -1, RGB(255, 255, 255));
	mainView->Initialize(this, rect);
	mainView->AddElement(new ItemWnd(TEXTSTRUCT(_T("[Room Name]"), TEXTSTRUCT::STYLE::BOLD, TEXTSTRUCT::VALIGN::CENTER, TEXTSTRUCT::HALIGN::LEFT, _T("Segoe UI"), 10, RGB(80, 80, 80)), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(0, 10, 10, 0), 10), _T(""), 40));
	mainView->AddElement(new EditWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 10), 10), _T("NAMEEDIT"), 70, RGB(255, 255, 255), RGB(125, 125, 125)));
	mainView->AddElement(new ItemWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(15, 4, 15, 4)), _T(""), 10, RGB(200, 200, 200)));

	mainView->AddElement(new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 70));
	((PaneWnd*)mainView->GetElement(3))->AddElement(new EditWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 10, 10), 10), _T("SEARCHEDIT"), -1, RGB(255, 255, 255), RGB(125, 125, 125)));
	((PaneWnd*)mainView->GetElement(3))->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Search"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(0, 10, 10, 10), 10), _T("SEARCH"), 100, RGB(86, 124, 131)));

	mainView->AddElement(new ScrollWnd(DIRECTION::VERTICAL, _T("FRIENDLIST"), 460));

	PaneWnd* tmpPane = new PaneWnd(DIRECTION::HORIZONTAL, _T(""), 70);
	mainView->AddElement(tmpPane);
	tmpPane->AddElement(new ItemWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(), _T(""), 135, RGB(255, 255, 255)));
	tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Create"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 10, 5, 10), 10), _T("CREATE"), 120, RGB(98, 164, 188)));
	((ButtonWnd*)tmpPane->GetElement(1))->SetEnableStatus(FALSE);
	tmpPane->AddElement(new ButtonWnd(TEXTSTRUCT(_T("Close"), TEXTSTRUCT::STYLE::BOLD), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(5, 10, 10, 10), 10), _T("CLOSE"), 120, RGB(200, 200, 200)));
	tmpPane->AddElement(new ItemWnd(TEXTSTRUCT(_T("")), SHAPESTRUCT(), _T(""), 135, RGB(255, 255, 255)));
	InitFriendList(friends);
	mainView->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
	return TRUE;
}

BEGIN_MESSAGE_MAP(CRoomDlg, CWnd)
    ON_WM_PAINT()
	ON_MESSAGE(WM_BUTTON_CLICK, &CRoomDlg::OnButtonClick)
	ON_MESSAGE(WM_CREATE_ROOM_ACTION, &CRoomDlg::OnCreateRoomAction)
END_MESSAGE_MAP()

void CRoomDlg::OnPaint()
{
	CWnd::OnPaint();
}

LRESULT CRoomDlg::OnButtonClick(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = (CString*)wParam;
	if (pStr)
	{
		CString str = *pStr;
		delete pStr;

		if (str.Compare(_T("SEARCH")) == 0)
		{
			PaneWnd* paneWnd = dynamic_cast<PaneWnd*>(mainView->GetElement(3));
			if (paneWnd)
			{
				CString text = ((EditWnd*)paneWnd->GetElement(0))->GetItemText();
				((ScrollWnd*)mainView->GetElement(4))->ClearElement();
				for (int i = 0; i < friendList.GetCount(); ++i)
				{
					if (friendList.GetAt(i).Find(text) >= 0)
					{
						CString name;
						name.Format(_T("FRIEND%d"), i);
						ButtonWnd* pBtn = new ButtonWnd(TEXTSTRUCT(friendList.GetAt(i)), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 5, 10, 5), 10), name, 70, RGB(245, 245, 245));
						pBtn->SetPressedStatus(sel.GetAt(i));
						((ScrollWnd*)mainView->GetElement(4))->AddElement(pBtn);
					}
				}

				CRect rect;
				((ScrollWnd*)mainView->GetElement(4))->GetClientRect(&rect);
				mainView->GetElement(4)->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
			}
		}
		else if (str.Find(_T("FRIEND")) >= 0)
		{
			int idx = _ttoi(trimFromAffix(str, _T("FRIEND")));
			ButtonWnd* pBtn = NULL;
			if (idx >= 0)
			{
				pBtn = dynamic_cast<ButtonWnd*>(((ScrollWnd*)mainView->GetElement(4))->FindElement(str));
				if (pBtn)
				{
					BOOL b = !sel.GetAt(idx);
					sel.SetAt(idx, b);
					pBtn->SetPressedStatus(b);
					pBtn->Invalidate();
				}
			}
			pBtn = dynamic_cast<ButtonWnd*>(mainView->FindElement(_T("CREATE")));
			if (pBtn)
			{
				BOOL b = FALSE;
				for (int i = 0; i < sel.GetCount(); i++)
				{
					if (sel.GetAt(i) == TRUE) b = TRUE;
				}
				pBtn->SetEnableStatus(b);
			}
		}
		else if (str.Compare(_T("CREATE")) == 0)
		{
			EditWnd* pEdit = dynamic_cast<EditWnd*>(mainView->GetElement(1));
			if (pEdit && pEdit->GetItemText().IsEmpty())
			{
				MessageBox(_T("Enter room name."), _T("Notice"), MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			ButtonWnd* pBtn = dynamic_cast<ButtonWnd*>(mainView->FindElement(_T("CREATE")));
			if (pBtn && pBtn->GetEnableStatus() == TRUE)
			{
				CArray<CString, CString> friends;
				for (int i = 0; i < friendList.GetCount(); i++)
				{
					if (sel.GetAt(i) == TRUE)
					{
						friends.Add(trimFromAffix(friendList.GetAt(i), _T("("), _T(")")));
					}
				}
				chatManager->createRoom(pEdit->GetItemText(), friends);
			}
		}
		else if (str.Compare(_T("CLOSE")) == 0)
		{
			EndModalLoop(IDOK);
		}
	}
	return 0;
}

LRESULT CRoomDlg::OnCreateRoomAction(WPARAM wParam, LPARAM lParam)
{
	CString* result = reinterpret_cast<CString*>(lParam);
	if (static_cast<BOOL>(wParam))
	{
		CString id = *result;
		EditWnd* pEdit = dynamic_cast<EditWnd*>(mainView->GetElement(1));
		::SendMessage(GetParent()->GetSafeHwnd(), WM_CREATE_ROOM_ACTION, (LPARAM)(LPCTSTR)id, (LPARAM)(LPCTSTR)pEdit->GetItemText());
		delete result;
		EndModalLoop(IDOK);
	}
	else
	{
		MessageBox(*result, _T("Notice"), MB_OK | MB_ICONINFORMATION);
		delete result;
	}
	return 0;
}

void CRoomDlg::InitFriendList(CArray<CString, CString>& friends)
{
	sel.RemoveAll();
	friendList.RemoveAll();
	((ScrollWnd*)mainView->GetElement(4))->ClearElement();
	for (int i = 0; i < friends.GetCount(); i++)
	{
		sel.Add(FALSE);
		friendList.Add(friends.GetAt(i));
		CString name;
		name.Format(_T("FRIEND%d"), i);
		((ScrollWnd*)mainView->GetElement(4))->AddElement(new ButtonWnd(TEXTSTRUCT(friends.GetAt(i)), SHAPESTRUCT(SHAPESTRUCT::SHAPE::SQUARE, CRect(10, 5, 10, 5), 10), name, 70, RGB(245, 245, 245)));
	}
}
