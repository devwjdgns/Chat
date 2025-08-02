#include "pch.h"
#include "framework.h"
#include "ScrollWnd.h"

#define VISIBLE_MARGIN 80

ScrollWnd::ScrollWnd(DIRECTION d, CString name, int size, COLORREF bgcolor, COLORREF olcolor) : ContainerWnd(d, name, size, bgcolor, olcolor)
{
	type = ELETYPE::SCROLL;
	direction = d;
	usePaintex = FALSE;
}

ScrollWnd::~ScrollWnd()
{
}

void ScrollWnd::MoveToStart()
{
	if (direction == DIRECTION::HORIZONTAL)
	{
		CRect rect;
		GetClientRect(&rect);
		scrollPos.x = 0;

		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE | SIF_POS };
		si.nMin = 0;
		si.nMax = totalSize.cx - 1;
		si.nPage = rect.Width();
		si.nPos = scrollPos.x;
		SetScrollInfo(SB_HORZ, &si, TRUE);
	}
	else if (direction == DIRECTION::VERTICAL)
	{
		CRect rect;
		GetClientRect(&rect);
		scrollPos.y = 0;

		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE | SIF_POS };
		si.nMin = 0;
		si.nMax = totalSize.cy - 1;
		si.nPage = rect.Height();
		si.nPos = scrollPos.y;
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
}

void ScrollWnd::MoveToEnd()
{
	if (direction == DIRECTION::HORIZONTAL)
	{
		CRect rect;
		GetClientRect(&rect);
		scrollPos.x = totalSize.cx - rect.Width();
		if (scrollPos.x < 0)
			scrollPos.x = 0;

		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE | SIF_POS };
		si.nMin = 0;
		si.nMax = totalSize.cx - 1;
		si.nPage = rect.Width();
		si.nPos = scrollPos.x;
		SetScrollInfo(SB_HORZ, &si, TRUE);
	}
	else if (direction == DIRECTION::VERTICAL)
	{
		CRect rect;
		GetClientRect(&rect);
		scrollPos.y = totalSize.cy - rect.Height();
		if (scrollPos.y < 0)
			scrollPos.y = 0;

		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE | SIF_POS };
		si.nMin = 0;
		si.nMax = totalSize.cy - 1;
		si.nPage = rect.Height();
		si.nPos = scrollPos.y;
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
	MoveChildWindow();
}

void ScrollWnd::UsePaintEx()
{
	usePaintex = TRUE;
}

void ScrollWnd::Initialize(CWnd* parent, CRect rect)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL);
	if (direction == DIRECTION::HORIZONTAL)
	{
		CWnd::Create(className, _T("ScrollWnd"), WS_CHILD | WS_VISIBLE | WS_HSCROLL, rect, parent, 1);
	}
	else if (direction == DIRECTION::VERTICAL)
	{
		CWnd::Create(className, _T("ScrollWnd"), WS_CHILD | WS_VISIBLE | WS_VSCROLL, rect, parent, 1);
	}
}

BOOL ScrollWnd::AddElement(ElementWnd* element)
{
	if (element == NULL)
	{
		delete element;
		return FALSE;
	}

	CRect rect;
	GetClientRect(&rect);

	element->Initialize(this, rect);
	elements.push_back(element);
	return TRUE;
}

BEGIN_MESSAGE_MAP(ScrollWnd, CWnd)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

void ScrollWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (direction == DIRECTION::HORIZONTAL)
	{
		int width = 0;
		int cnt = elements.size();
		for (int i = 0; i < cnt; i++)
		{
			if (elements[i]->GetSize() >= 0)
			{
				width += elements[i]->GetSize();
			}
			else
			{
				width += elements[i]->GetMinSize(direction);
			}
		}
		totalSize = CPoint(width, cy);
		int prevScrollX = scrollPos.x;
		int maxScrollX = max(0, totalSize.cx - cx);
		int newScrollX = min(prevScrollX, maxScrollX);
		scrollPos.x = newScrollX;

		CRect rect;
		GetClientRect(&rect);
		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE | SIF_POS };
		si.nMin = 0;
		si.nMax = totalSize.cx - 1;
		si.nPage = rect.Width();
		si.nPos = scrollPos.x;
		SetScrollInfo(SB_HORZ, &si, TRUE);
	}
	else if (direction == DIRECTION::VERTICAL)
	{
		int height = 0;
		int cnt = elements.size();
		for (int i = 0; i < cnt; i++)
		{
			if (elements[i]->GetSize() >= 0)
			{
				height += elements[i]->GetSize();
			}
			else
			{
				height += elements[i]->GetMinSize(direction);
			}
		}
		totalSize = CPoint(cx, height);
		int prevScrollY = scrollPos.y;
		int maxScrollY = max(0, totalSize.cy - cy);
		int newScrollY = min(prevScrollY, maxScrollY);
		scrollPos.y = newScrollY;

		CRect rect;
		GetClientRect(&rect);
		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE | SIF_POS };
		si.nMin = 0;
		si.nMax = totalSize.cy - 1;
		si.nPage = rect.Height();
		si.nPos = scrollPos.y;
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
	clientSize = CPoint(cx, cy);
	MoveChildWindow();
}

void ScrollWnd::OnPaint()
{
	if (!usePaintex)
	{
		ContainerWnd::OnPaint();
	}
	else
	{
		CPaintDC dc(this);

		CRect rect;
		GetClientRect(&rect);

		CDC memDC;
		memDC.CreateCompatibleDC(&dc);

		CBitmap memBitmap;
		memBitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
		CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

		COLORREF bg = GetBackgroundColor();
		memDC.FillSolidRect(&rect, bg);

		COLORREF ol = GetOutlineColor();
		if (ol != CLR_NONE)
		{
			CPen penBorder(PS_SOLID, 1, RGB(255, 0, 0));
			CPen* pOldPen = memDC.SelectObject(&penBorder);
			memDC.SelectStockObject(NULL_BRUSH);
			memDC.Rectangle(&rect);
			memDC.SelectObject(pOldPen);
		}

		CRect childRect;
		for (int i = 0; i < elements.size(); i++)
		{
			if (!elements[i]->IsShowWindowExtern()) continue;
			elements[i]->GetWindowRect(&childRect);
			ScreenToClient(&childRect);
			elements[i]->PaintExtern(&memDC, childRect);
		}

		dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(pOldBitmap);
	}
}

BOOL ScrollWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void ScrollWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect clientRect;
	GetClientRect(&clientRect);

	int pos = scrollPos.x;
	int maxPos = totalSize.cx - clientRect.Width();

	switch (nSBCode)
	{
	case SB_LINELEFT:   pos -= 20; break;
	case SB_LINERIGHT:  pos += 20; break;
	case SB_PAGELEFT:   pos -= clientRect.Width(); break;
	case SB_PAGERIGHT:  pos += clientRect.Width(); break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: pos = nPos; break;
	case SB_LEFT:       pos = 0; break;
	case SB_RIGHT:      pos = maxPos; break;
	}

	if (pos < 0) pos = 0;
	if (pos > maxPos) pos = maxPos;

	if (pos != scrollPos.x)
	{
		scrollPos.x = pos;
		SetScrollPos(SB_HORZ, pos);
		MoveChildWindow();
	}
}

void ScrollWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect clientRect;
	GetClientRect(&clientRect);

	int pos = scrollPos.y;
	int maxPos = totalSize.cy - clientRect.Height();

	switch (nSBCode)
	{
	case SB_LINEUP:     pos -= 20; break;
	case SB_LINEDOWN:   pos += 20; break;
	case SB_PAGEUP:     pos -= clientRect.Height(); break;
	case SB_PAGEDOWN:   pos += clientRect.Height(); break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: pos = nPos; break;
	case SB_TOP:        pos = 0; break;
	case SB_BOTTOM:     pos = maxPos; break;
	}

	if (pos < 0) pos = 0;
	if (pos > maxPos) pos = maxPos;

	if (pos > 0 && pos != scrollPos.y)
	{
		scrollPos.y = pos;
		SetScrollPos(SB_VERT, pos);
		MoveChildWindow();
	}
}

BOOL ScrollWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int scrollStep = 30;
	int pos = scrollPos.y;

	if (zDelta > 0)
		pos -= scrollStep;
	else
		pos += scrollStep;

	CRect clientRect;
	GetClientRect(&clientRect);
	int maxPos = totalSize.cy - clientRect.Height();

	if (pos < 0) pos = 0;
	if (pos > maxPos) pos = maxPos;

	if (pos >= 0 && pos != scrollPos.y)
	{
		scrollPos.y = pos;
		SetScrollPos(SB_VERT, pos);
		MoveChildWindow();
	}

	return TRUE;
}

void ScrollWnd::MoveChildWindow()
{
	CRect rect;
	GetClientRect(&rect);
	if (direction == DIRECTION::HORIZONTAL)
	{
		int width = 0;
		int cnt = elements.size();
		for (int i = 0; i < cnt; i++)
		{
			if (elements[i]->GetSize() >= 0)
			{
				if (width - scrollPos.x >= -VISIBLE_MARGIN && width - scrollPos.x + elements[i]->GetSize() <= rect.Width() + VISIBLE_MARGIN)
				{
					elements[i]->MoveWindow(width - scrollPos.x, 0, elements[i]->GetSize(), rect.Height());
				}
				width += elements[i]->GetSize();
			}
			else
			{
				int minwidth = elements[i]->GetMinSize(direction);
				if (width - scrollPos.x >= -VISIBLE_MARGIN && width - scrollPos.x + minwidth <= rect.Width() + VISIBLE_MARGIN)
				{
					elements[i]->MoveWindow(width - scrollPos.x, 0, minwidth, rect.Height());
				}
				width += minwidth;
			}
		}
	}
	else if (direction == DIRECTION::VERTICAL)
	{
		int height = 0;
		int cnt = elements.size();
		for (int i = 0; i < cnt; i++)
		{
			if (elements[i]->GetSize() >= 0)
			{
				if (height - scrollPos.y >= -VISIBLE_MARGIN && height - scrollPos.y + elements[i]->GetSize() <= rect.Height() + VISIBLE_MARGIN)
				{
					elements[i]->MoveWindow(0, height - scrollPos.y, rect.Width(), elements[i]->GetSize());
				}
				height += elements[i]->GetSize();
			}
			else
			{
				int minheight = elements[i]->GetMinSize(direction);
				if (height - scrollPos.y >= -VISIBLE_MARGIN && height - scrollPos.y + minheight <= rect.Height() + VISIBLE_MARGIN)
				{
					elements[i]->MoveWindow(0, height - scrollPos.y, rect.Width(), minheight);
				}
				height += minheight;
			}
		}
	}
	Invalidate();
	UpdateWindow();
}