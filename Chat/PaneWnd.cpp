#include "pch.h"
#include "framework.h"
#include "PaneWnd.h"

PaneWnd::PaneWnd(DIRECTION d, CString name, int size, COLORREF bgcolor, COLORREF olcolor) : ContainerWnd(d, name, size, bgcolor, olcolor)
{
	type = ELETYPE::PANE;
    direction = d;
}

PaneWnd::~PaneWnd()
{
}

void PaneWnd::Initialize(CWnd* parent, CRect rect)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL);
	CWnd::Create(className, _T("PaneWnd"), WS_CHILD | WS_VISIBLE, rect, parent, 1);
}

BOOL PaneWnd::AddElement(ElementWnd* element)
{
	if (element == NULL)
	{
		delete element;
		return FALSE;
	}

	CRect rect;
	GetClientRect(&rect);

	if (element->GetSize() == -1)
	{
		for (int i = 0; i < elements.size(); i++)
		{
			if (elements[i]->GetSize() == -1)
			{
				delete element;
				return FALSE;
			}
		}
		element->Initialize(this, rect);
	}
	else
	{
		CRect rect;
		GetClientRect(rect);
		int totalsize = direction == DIRECTION::HORIZONTAL ? rect.Width() : rect.Height();
		for (int i = 0; i < elements.size(); i++)
		{
			if (elements[i]->GetSize() >= 0)
			{
				totalsize -= elements[i]->GetSize();
			}
			if (totalsize < 0)
			{
				delete element;
				return FALSE;
			}
		}
		element->Initialize(this, rect);
	}
	elements.push_back(element);
	return TRUE;
}

BEGIN_MESSAGE_MAP(PaneWnd, CWnd)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void PaneWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (direction == DIRECTION::HORIZONTAL)
	{
		int start = 0;
		int end = cx;
		int cnt = elements.size();
		ElementWnd* resizableElement = NULL;
		for (int i = 0; i < cnt; i++)
		{
			if (elements[i]->GetSize() >= 0)
			{
				elements[i]->MoveWindow(start, 0, elements[i]->GetSize(), cy);
				start += elements[i]->GetSize();
			}
			else
			{
				resizableElement = elements[i];
				for (int j = cnt - 1; j > i; j--)
				{
					elements[j]->MoveWindow(end - elements[j]->GetSize(), 0, elements[j]->GetSize(), cy);
					end -= elements[j]->GetSize();
				}
				break;
			}
		}
		if (resizableElement != NULL)
		{
			resizableElement->MoveWindow(start, 0, end-start, cy);
		}
	}
	else if (direction == DIRECTION::VERTICAL)
	{
		int start = 0;
		int end = cy;
		int cnt = elements.size();
		ElementWnd* resizableElement = NULL;
		for (int i = 0; i < cnt; i++)
		{
			if (elements[i]->GetSize() >= 0)
			{
				elements[i]->MoveWindow(0, start, cx, elements[i]->GetSize());
				start += elements[i]->GetSize();
			}
			else
			{
				resizableElement = elements[i];
				for (int j = cnt - 1; j > i; j--)
				{
					elements[j]->MoveWindow(0, end - elements[j]->GetSize(), cx, elements[j]->GetSize());
					end -= elements[j]->GetSize();
				}
				break;
			}
		}
		if (resizableElement != NULL)
		{
			resizableElement->MoveWindow(0, start, cx, end - start);
		}
	}
}

void PaneWnd::OnPaint()
{
	ContainerWnd::OnPaint();
}

BOOL PaneWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}