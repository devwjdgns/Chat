#include "pch.h"
#include "framework.h"
#include "ElementWnd.h"

ElementWnd::ElementWnd(CString name, int size, COLORREF bgcolor, COLORREF olcolor) : CWnd(), name(name), size(size), bgcolor(bgcolor), olcolor(olcolor)
{
	type = ELETYPE::NONE;
	show = TRUE;
}

ElementWnd::~ElementWnd()
{

}
void ElementWnd::InvalidateParent()
{
	ElementWnd* pParent = dynamic_cast<ElementWnd*>(GetParent());
	if (pParent)
	{
		pParent->InvalidateParent();
	}
	else
	{
		CRect rect;
		GetWindowRect(&rect);
		SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.Width(), rect.Height()));
	}
}

void ElementWnd::SendMessageParent(int flag, CString message)
{
	ElementWnd* pParent = dynamic_cast<ElementWnd*>(GetParent());
	if (pParent)
	{
		pParent->SendMessageParent(flag, message);
	}
	else
	{
		CString* pStr = new CString(message);
		GetParent()->SendMessage(flag, (WPARAM)pStr);
	}
}

COLORREF ElementWnd::GetBackgroundColor()
{
    if (bgcolor != CLR_NONE) return bgcolor;
    ElementWnd* parent = dynamic_cast<ElementWnd*>(GetParent());
    while (parent != nullptr)
    {
        if (parent->bgcolor != CLR_NONE) return parent->bgcolor;
        parent = dynamic_cast<ElementWnd*>(parent->GetParent());
    }
}

COLORREF ElementWnd::GetOutlineColor()
{
    return olcolor;
}

BEGIN_MESSAGE_MAP(ElementWnd, CWnd)
END_MESSAGE_MAP()