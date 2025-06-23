#include "pch.h"
#include "framework.h"
#include "EditWnd.h"

BOOL SingleEdit::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		ElementWnd* pParent = dynamic_cast<ElementWnd*>(GetParent());
		if (pParent)
		{
			pParent->SendMessageParent(WM_EDIT_COMPLETE, pParent->GetName());
		}
		return TRUE;
	}

	return CEdit::PreTranslateMessage(pMsg);
}

BOOL MultiEdit::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        if (GetKeyState(VK_SHIFT) & 0x8000)
        {
			ElementWnd* pParent = dynamic_cast<ElementWnd*>(GetParent());
			if (pParent)
			{
				pParent->InvalidateParent();
			}
            return FALSE;
        }
        else
        {
			ElementWnd* pParent = dynamic_cast<ElementWnd*>(GetParent());
			if (pParent)
			{
				pParent->SendMessageParent(WM_EDIT_COMPLETE, pParent->GetName());
			}
            return TRUE;
        }
    }

    return CRichEditCtrl::PreTranslateMessage(pMsg);
}

EditWnd::EditWnd(TEXTSTRUCT text, SHAPESTRUCT shape, CString name, int size, COLORREF bgcolor, COLORREF olcolor) : ItemWnd(text, shape, name, size, bgcolor, olcolor)
{
	type = ELETYPE::EDIT;
	password = FALSE;
	multiline = FALSE;
}

EditWnd::~EditWnd()
{

}

void EditWnd::SetStylePassword()
{
	password = TRUE;
	if (::IsWindow(singleEdit.GetSafeHwnd()))
	{
		singleEdit.SetPasswordChar(_T('*'));
		singleEdit.Invalidate();
	}
}

void EditWnd::SetStyleMultiLine()
{
	multiline = TRUE;
	if (::IsWindow(singleEdit.GetSafeHwnd()))
	{
		singleEdit.ShowWindow(SW_HIDE);
		multiEdit.ShowWindow(SW_SHOW);
	}
}

void EditWnd::ClearText()
{
	singleEdit.SetWindowText(_T(""));
	multiEdit.SetWindowText(_T(""));
}

CString EditWnd::GetItemText()
{
	if (!multiline)
	{
		CString str;
		singleEdit.GetWindowText(str);
		return str;
	}
	else
	{
		CString str;
		multiEdit.GetWindowText(str);
		if (str.IsEmpty())
			str += _T("null");
		else
			str += _T("\r\n\r\n");
		return str;
	}
}

BEGIN_MESSAGE_MAP(EditWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int EditWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (ItemWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	singleEdit.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_WANTRETURN | ES_AUTOHSCROLL, CRect(10, 10, 200, 30), this, 1000);
	multiEdit.Create(WS_CHILD | WS_VISIBLE| ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | ES_WANTRETURN, CRect(10, 10, 200, 30), this, 1001);
	
	if (password)
	{
		SetStylePassword();
	}

	if (multiline)
	{
		SetStyleMultiLine();
	}
	else
	{
		multiEdit.ShowWindow(SW_HIDE);
	}
	font.CreatePointFont(text.fontsize*10, text.font);
	singleEdit.SetFont(&font);
	multiEdit.SetFont(&font);
	singleEdit.SetWindowText(text.text);
	multiEdit.SetWindowText(text.text);
	return 0;
}

void EditWnd::OnSize(UINT nType, int cx, int cy)
{
	ItemWnd::OnSize(nType, cx, cy);
	singleEdit.MoveWindow(0 + shape.margin.left + 10
		, 0 + shape.margin.top + 10
		, cx - shape.margin.left - shape.margin.right - 20
		, cy - shape.margin.top - shape.margin.bottom - 20);
	multiEdit.MoveWindow(0 + shape.margin.left + 10
		, 0 + shape.margin.top + 10
		, cx - shape.margin.left - shape.margin.right - 20
		, cy - shape.margin.top - shape.margin.bottom - 20);
}

void EditWnd::OnPaint()
{
	ItemWnd::OnPaint();
}