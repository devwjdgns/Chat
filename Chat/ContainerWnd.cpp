#include "pch.h"
#include "framework.h"
#include "ContainerWnd.h"

ContainerWnd::ContainerWnd(DIRECTION d, CString name, int size, COLORREF bgcolor, COLORREF olcolor) : ElementWnd(name, size, bgcolor, olcolor)
{
	type = ELETYPE::CONTAINER;
	direction = d;
}

ContainerWnd::~ContainerWnd()
{
	ClearElement();
}

BOOL ContainerWnd::AddElement(ElementWnd* element)
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

ElementWnd* ContainerWnd::GetElement(int idx)
{
	if (idx >= elements.size()) return NULL;
	return elements[idx];
}

ElementWnd* ContainerWnd::FindElement(CString name)
{
	if (GetName().Compare(name) == 0) return this;
	for (int i = 0; i < elements.size(); i++)
	{
		if (elements[i]->GetName().Compare(name) == 0) return elements[i];
		if (dynamic_cast<ContainerWnd*>(elements[i]) != NULL)
		{
			ElementWnd* tmp = dynamic_cast<ContainerWnd*>(elements[i])->FindElement(name);
			if (tmp != NULL)
			{
				return tmp;
			}
		}
	}
	return NULL;
}

void ContainerWnd::ClearElement()
{
	for (int i = 0; i < elements.size(); i++)
	{
		delete elements[i];
	}
	elements.clear();
}

int ContainerWnd::GetElementCount()
{
	return elements.size();
}

void ContainerWnd::Initialize(CWnd* parent, CRect rect)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL);
	CWnd::Create(className, _T("ContainerWnd"), WS_CHILD | WS_VISIBLE, rect, parent, 1);
}

void ContainerWnd::PaintExtern(CDC* pdc, CRect rect)
{
	COLORREF bg = GetBackgroundColor();
	pdc->FillSolidRect(&rect, bg);

	COLORREF ol = GetOutlineColor();
	if (ol != CLR_NONE)
	{
		CPen penBorder(PS_SOLID, 1, RGB(255, 0, 0));
		CPen* pOldPen = pdc->SelectObject(&penBorder);
		pdc->SelectStockObject(NULL_BRUSH);
		pdc->Rectangle(&rect);
		pdc->SelectObject(pOldPen);
	}

	int left = rect.left;
	int top = rect.top;
	for (int i = 0; i < elements.size(); i++)
	{
		elements[i]->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.left += left; rect.right += left;
		rect.top += top; rect.bottom += top;
		elements[i]->PaintExtern(pdc, rect);
	}
	ShowWindow(SW_HIDE);
}

int ContainerWnd::GetMinSize(DIRECTION d)
{
	if (size >= 0) return size;
	CRect rect;
	GetClientRect(&rect);
	if (d == DIRECTION::HORIZONTAL)
	{
		ContainerWnd* pParent = dynamic_cast<ContainerWnd*>(GetParent());
		if (pParent != NULL && pParent->GetDirection() == DIRECTION::VERTICAL)
		{
			return rect.Width();
		}
		else if (pParent != NULL && pParent->GetDirection() == DIRECTION::HORIZONTAL)
		{
			if (GetSize() >= 0)
			{
				return GetSize();
			}
			else
			{
				int width = 0;
				int cnt = elements.size();
				for (int i = 0; i < cnt; i++)
				{
					width += elements[i]->GetMinSize(d);
				}
				return width;
			}
		}
	}
	else if (d == DIRECTION::VERTICAL)
	{
		ContainerWnd* pParent = dynamic_cast<ContainerWnd*>(GetParent());
		if (pParent != NULL && pParent->GetDirection() == DIRECTION::HORIZONTAL)
		{
			return rect.Height();
		}
		else if (pParent != NULL && pParent->GetDirection() == DIRECTION::VERTICAL)
		{
			if (GetSize() >= 0)
			{
				return GetSize();
			}
			else
			{
				int height = 0;
				int cnt = elements.size();
				for (int i = 0; i < cnt; i++)
				{
					height += elements[i]->GetMinSize(d);
				}
				return height;
			}
		}
	}
}

BEGIN_MESSAGE_MAP(ContainerWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void ContainerWnd::OnPaint()
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

	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBitmap);
}

BOOL ContainerWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}