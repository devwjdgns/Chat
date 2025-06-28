#include "pch.h"
#include "framework.h"
#include "ButtonWnd.h"

ButtonWnd::ButtonWnd(TEXTSTRUCT text, SHAPESTRUCT shape, CString name, int size, COLORREF bgcolor, COLORREF olcolor) : ItemWnd(text, shape, name, size, bgcolor, olcolor)
{
	type = ELETYPE::BUTTON;
	focus = FALSE;
	click = FALSE;
	enable = TRUE;
	pressed = FALSE;
}

ButtonWnd::~ButtonWnd()
{

}

COLORREF ButtonWnd::GetBackgroundColor()
{
	if ((focus && click && enable) || pressed)
	{
		COLORREF bg = ElementWnd::GetBackgroundColor();
		return RGB(GetRValue(bg) * 0.8, GetGValue(bg) * 0.8, GetBValue(bg) * 0.8);
	}
	else if (focus && enable)
	{
		COLORREF bg = ElementWnd::GetBackgroundColor();
		return RGB(GetRValue(bg) * 0.9, GetGValue(bg) * 0.9, GetBValue(bg) * 0.9);
	}
	else
	{
		return ElementWnd::GetBackgroundColor();
	}
}


BEGIN_MESSAGE_MAP(ButtonWnd, CWnd)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND_RANGE(1000, 1234, OnDynamicMenuCommand)
END_MESSAGE_MAP()

void ButtonWnd::OnSize(UINT nType, int cx, int cy)
{
	ItemWnd::OnSize(nType, cx, cy);
}

void ButtonWnd::OnPaint()
{
	ItemWnd::OnPaint();
}

void ButtonWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	click = TRUE;
	Invalidate();

	ItemWnd::OnLButtonDown(nFlags, point);
}

void ButtonWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (click)
	{
		SendMessageParent(WM_BUTTON_CLICK, GetName());
	}
	click = FALSE;
	Invalidate();

	ItemWnd::OnLButtonUp(nFlags, point);
}

void ButtonWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!focus)
	{
		focus = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = 0;
		::TrackMouseEvent(&tme);
		Invalidate();
	}

	ItemWnd::OnMouseMove(nFlags, point);
}

void ButtonWnd::OnMouseLeave()
{
	if (focus)
	{
		focus = FALSE;
		click = FALSE;
		Invalidate();
	}
	
	ItemWnd::OnMouseLeave();
}

void ButtonWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.CreatePopupMenu();

	const UINT BASE_COMMAND_ID = 1000;
	for (size_t i = 0; i < menus.GetCount(); ++i)
	{
		menu.AppendMenu(MF_STRING, BASE_COMMAND_ID + static_cast<UINT>(i), menus.GetAt(i));
	}
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

void ButtonWnd::OnDynamicMenuCommand(UINT nID)
{
	const UINT BASE_COMMAND_ID = 1000;
	UINT index = nID - BASE_COMMAND_ID;
	SendMessageParent(WM_BUTTON_MENU, GetName() + _T("^") + menus.GetAt(index));
}
