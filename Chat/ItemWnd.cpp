#include "pch.h"
#include "framework.h"
#include "ItemWnd.h"
#include "ContainerWnd.h"

ItemWnd::ItemWnd(TEXTSTRUCT text, SHAPESTRUCT shape, CString name, int size, COLORREF bgcolor, COLORREF olcolor) : ElementWnd(name, size, bgcolor, olcolor)
{
	type = ELETYPE::ITEM;
	this->text = text;
	this->shape = shape;
}

ItemWnd::~ItemWnd()
{

}

void ItemWnd::Initialize(CWnd* parent, CRect rect)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL);
	CWnd::Create(className, _T("ItemWnd"), WS_CHILD | WS_VISIBLE, rect, parent, 1);
}

void ItemWnd::PaintExtern(CDC* pdc, CRect rect)
{
	Gdiplus::Graphics graphics(pdc->GetSafeHdc()); // 직접 사용

    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    ElementWnd* parent = dynamic_cast<ElementWnd*>(GetParent());
    if (parent != NULL)
    {
        COLORREF bg = parent->GetBackgroundColor();
        pdc->FillSolidRect(&rect, bg);

        COLORREF ol = parent->GetOutlineColor();
        if (ol != CLR_NONE)
        {
            CPen penBorder(PS_SOLID, 1, RGB(255, 0, 0));
            CPen* pOldPen = pdc->SelectObject(&penBorder);
            pdc->SelectStockObject(NULL_BRUSH);
            pdc->Rectangle(&rect);
            pdc->SelectObject(pOldPen);
        }
    }

    rect.DeflateRect(shape.margin.left, shape.margin.top, shape.margin.right, shape.margin.bottom);

    switch (shape.shape)
    {
    case SHAPESTRUCT::SHAPE::CIRCLE:
        PaintCircle(&graphics, rect);
        break;
    case SHAPESTRUCT::SHAPE::SQUARE:
        PaintSquare(&graphics, rect);
        break;
    case SHAPESTRUCT::SHAPE::TRIANGLE:
        PaintTriangle(&graphics, rect);
        break;
    case SHAPESTRUCT::SHAPE::PLUS:
        PaintPlus(&graphics, rect);
        break;
    default:
        break;
    }

    PaintText(&graphics, rect);
    ShowWindow(SW_HIDE);
}

int ItemWnd::GetMinSize(DIRECTION d)
{
	ContainerWnd* pParent = dynamic_cast<ContainerWnd*>(GetParent());
	if (pParent != NULL)
	{
		CRect rect;
		pParent->GetClientRect(&rect);
		Gdiplus::StringFormat format;
		switch (text.valign)
		{
		case TEXTSTRUCT::VALIGN::TOP:
			format.SetLineAlignment(Gdiplus::StringAlignmentNear);
			break;
		case TEXTSTRUCT::VALIGN::CENTER:
			format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
			break;
		case TEXTSTRUCT::VALIGN::BOTTOM:
			format.SetLineAlignment(Gdiplus::StringAlignmentFar);
			break;
		default:
			break;
		}
		switch (text.halign)
		{
		case TEXTSTRUCT::HALIGN::LEFT:
			format.SetAlignment(Gdiplus::StringAlignmentNear);
			break;
		case TEXTSTRUCT::HALIGN::CENTER:
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			break;
		case TEXTSTRUCT::HALIGN::RIGHT:
			format.SetAlignment(Gdiplus::StringAlignmentFar);
			break;
		default:
			break;
		}

		INT fontStyle = Gdiplus::FontStyleRegular;
		switch (text.style)
		{
		case TEXTSTRUCT::STYLE::BOLD:
			fontStyle != Gdiplus::FontStyleBold;
			break;
		case TEXTSTRUCT::STYLE::ITALIC:
			fontStyle != Gdiplus::FontStyleItalic;
			break;
		default:
			break;
		}
		Gdiplus::Font font(text.font, text.fontsize, fontStyle);
		rect.top += shape.margin.top;
		rect.left += shape.margin.left;
		rect.bottom -= shape.margin.bottom;
		rect.right -= shape.margin.right;
		Gdiplus::RectF boundingBox;
		if (pParent->GetDirection() == DIRECTION::HORIZONTAL)
		{
			int width = GetSize() >= 0 ? GetSize() : rect.Width();
			int height = rect.Height();
			if (d == DIRECTION::VERTICAL)
			{
				if (pParent->GetSize() >= 0)
				{
					return pParent->GetSize();
				}
				else
				{
					if (GetTextWidth() > width)
					{
						width -= text.fontsize * 4;
					}
					HWND hwnd = GetSafeHwnd();
					HDC hdc = ::GetDC(hwnd);
					Gdiplus::Graphics graphics(hdc);
					Gdiplus::RectF layoutRect(0, 0, width, height);
					graphics.MeasureString(GetItemText(), -1, &font, layoutRect, &format, &boundingBox);
					::ReleaseDC(hwnd, hdc);
					return boundingBox.Height + shape.margin.top + shape.margin.bottom + text.fontsize * 2;
				}
			}
			else if (d == DIRECTION::HORIZONTAL)
			{
				if (GetSize() >= 0)
				{
					return GetSize();
				}
				else
				{
					HWND hwnd = GetSafeHwnd();
					HDC hdc = ::GetDC(hwnd);
					Gdiplus::Graphics graphics(hdc);
					Gdiplus::RectF layoutRect(0, 0, width, height);
					graphics.MeasureString(GetItemText(), -1, &font, layoutRect, &format, &boundingBox);
					::ReleaseDC(hwnd, hdc);
					return boundingBox.Width + shape.margin.left + shape.margin.right + text.fontsize * 2;
				}
			}
		}
		else if (pParent->GetDirection() == DIRECTION::VERTICAL)
		{
			int width = rect.Width();
			int height = GetSize() >= 0 ? GetSize() : rect.Height();
			if (d == DIRECTION::HORIZONTAL)
			{
				if (pParent->GetSize() >= 0)
				{
					return pParent->GetSize();
				}
				else
				{
					if (GetTextWidth() > width)
					{
						width -= text.fontsize * 4;
					}
					HWND hwnd = GetSafeHwnd();
					HDC hdc = ::GetDC(hwnd);
					Gdiplus::Graphics graphics(hdc);
					Gdiplus::RectF layoutRect(0, 0, width, height);
					graphics.MeasureString(GetItemText(), -1, &font, layoutRect, &format, &boundingBox);
					::ReleaseDC(hwnd, hdc);
					return boundingBox.Width + shape.margin.left + shape.margin.right + text.fontsize * 2;
				}
			}
			else if (d == DIRECTION::VERTICAL)
			{
				if (GetSize() >= 0)
				{
					return GetSize();
				}
				else
				{
					HWND hwnd = GetSafeHwnd();
					HDC hdc = ::GetDC(hwnd);
					Gdiplus::Graphics graphics(hdc);
					Gdiplus::RectF layoutRect(0, 0, width, height);
					graphics.MeasureString(GetItemText(), -1, &font, layoutRect, &format, &boundingBox);
					::ReleaseDC(hwnd, hdc);
					return boundingBox.Height + shape.margin.top + shape.margin.bottom + text.fontsize * 2;
				}
			}
		}
	}
	return 0;
}

int ItemWnd::GetTextWidth()
{
	ContainerWnd* pParent = dynamic_cast<ContainerWnd*>(GetParent());
	if (pParent != NULL)
	{
		int margin = 0;
		Gdiplus::StringFormat format;
		switch (text.valign)
		{
		case TEXTSTRUCT::VALIGN::TOP:
			format.SetLineAlignment(Gdiplus::StringAlignmentNear);
			break;
		case TEXTSTRUCT::VALIGN::CENTER:
			format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
			break;
		case TEXTSTRUCT::VALIGN::BOTTOM:
			format.SetLineAlignment(Gdiplus::StringAlignmentFar);
			break;
		default:
			break;
		}
		switch (text.halign)
		{
		case TEXTSTRUCT::HALIGN::LEFT:
			format.SetAlignment(Gdiplus::StringAlignmentNear);
			margin = 8;
			break;
		case TEXTSTRUCT::HALIGN::CENTER:
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			break;
		case TEXTSTRUCT::HALIGN::RIGHT:
			format.SetAlignment(Gdiplus::StringAlignmentFar);
			margin = 8;
			break;
		default:
			break;
		}

		INT fontStyle = Gdiplus::FontStyleRegular;
		switch (text.style)
		{
		case TEXTSTRUCT::STYLE::BOLD:
			fontStyle != Gdiplus::FontStyleBold;
			break;
		case TEXTSTRUCT::STYLE::ITALIC:
			fontStyle != Gdiplus::FontStyleItalic;
			break;
		default:
			break;
		}
		Gdiplus::Font font(text.font, text.fontsize, fontStyle);
		Gdiplus::RectF boundingBox;
		HWND hwnd = GetSafeHwnd();
		HDC hdc = ::GetDC(hwnd);
		Gdiplus::Graphics graphics(hdc);
		Gdiplus::RectF layoutRect(0, 0, 1000, 1000);
		graphics.MeasureString(GetItemText(), -1, &font, layoutRect, &format, &boundingBox);
		::ReleaseDC(hwnd, hdc);
		return boundingBox.Width + shape.margin.left + shape.margin.right + text.fontsize*2 - margin;
	}
	return 0;
}

int ItemWnd::GetTextHeight()
{
	ContainerWnd* pParent = dynamic_cast<ContainerWnd*>(GetParent());
	if (pParent != NULL)
	{
		int margin = 0;
		Gdiplus::StringFormat format;
		switch (text.valign)
		{
		case TEXTSTRUCT::VALIGN::TOP:
			format.SetLineAlignment(Gdiplus::StringAlignmentNear);
			margin = 8;
			break;
		case TEXTSTRUCT::VALIGN::CENTER:
			format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
			break;
		case TEXTSTRUCT::VALIGN::BOTTOM:
			format.SetLineAlignment(Gdiplus::StringAlignmentFar);
			margin = 8;
			break;
		default:
			break;
		}
		switch (text.halign)
		{
		case TEXTSTRUCT::HALIGN::LEFT:
			format.SetAlignment(Gdiplus::StringAlignmentNear);
			break;
		case TEXTSTRUCT::HALIGN::CENTER:
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			break;
		case TEXTSTRUCT::HALIGN::RIGHT:
			format.SetAlignment(Gdiplus::StringAlignmentFar);
			break;
		default:
			break;
		}

		INT fontStyle = Gdiplus::FontStyleRegular;
		switch (text.style)
		{
		case TEXTSTRUCT::STYLE::BOLD:
			fontStyle != Gdiplus::FontStyleBold;
			break;
		case TEXTSTRUCT::STYLE::ITALIC:
			fontStyle != Gdiplus::FontStyleItalic;
			break;
		default:
			break;
		}
		Gdiplus::Font font(text.font, text.fontsize, fontStyle);
		Gdiplus::RectF boundingBox;
		HWND hwnd = GetSafeHwnd();
		HDC hdc = ::GetDC(hwnd);
		Gdiplus::Graphics graphics(hdc);
		Gdiplus::RectF layoutRect(0, 0, 1000, 1000);
		graphics.MeasureString(GetItemText(), -1, &font, layoutRect, &format, &boundingBox);
		::ReleaseDC(hwnd, hdc);
		return boundingBox.Height + shape.margin.top + shape.margin.bottom - margin;
	}
	return 0;
}

CString ItemWnd::GetItemText()
{
	return text.text;
}

BEGIN_MESSAGE_MAP(ItemWnd, CWnd)
    ON_WM_SIZE()
    ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void ItemWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}

void ItemWnd::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	int width = rect.Width();
	int height = rect.Height();

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, width, height);
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);

	Gdiplus::Graphics graphics(memDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	ElementWnd* parent = dynamic_cast<ElementWnd*>(GetParent());
	if (parent != NULL)
	{
		COLORREF bg = parent->GetBackgroundColor();
		memDC.FillSolidRect(&rect, bg);

		COLORREF ol = parent->GetOutlineColor();
		if (ol != CLR_NONE)
		{
			CPen penBorder(PS_SOLID, 1, RGB(255, 0, 0));
			CPen* pOldPen = memDC.SelectObject(&penBorder);
			memDC.SelectStockObject(NULL_BRUSH);
			memDC.Rectangle(&rect);
			memDC.SelectObject(pOldPen);
		}
	}

	rect.DeflateRect(shape.margin.left, shape.margin.top, shape.margin.right, shape.margin.bottom);

	switch (shape.shape)
	{
	case SHAPESTRUCT::SHAPE::CIRCLE:
		PaintCircle(&graphics, rect);
		break;
	case SHAPESTRUCT::SHAPE::SQUARE:
		PaintSquare(&graphics, rect);
		break;
	case SHAPESTRUCT::SHAPE::TRIANGLE:
		PaintTriangle(&graphics, rect);
		break;
	case SHAPESTRUCT::SHAPE::PLUS:
		PaintPlus(&graphics, rect);
		break;
	default:
		break;
	}
	PaintText(&graphics, rect);

	dc.BitBlt(0, 0, width, height, &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

BOOL ItemWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void ItemWnd::PaintText(Gdiplus::Graphics* gdc, CRect rect)
{
	Gdiplus::RectF textRect((float)rect.left, (float)rect.top, (float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
	Gdiplus::StringFormat format;
	switch (text.valign)
	{
	case TEXTSTRUCT::VALIGN::TOP:
		format.SetLineAlignment(Gdiplus::StringAlignmentNear);
		textRect.Y += 8;
		textRect.Height -= 8;
		break;
	case TEXTSTRUCT::VALIGN::CENTER:
		format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		break;
	case TEXTSTRUCT::VALIGN::BOTTOM:
		format.SetLineAlignment(Gdiplus::StringAlignmentFar);
		textRect.Height -= 8;
		break;
	default:
		break;
	}
	switch (text.halign)
	{
	case TEXTSTRUCT::HALIGN::LEFT:
		format.SetAlignment(Gdiplus::StringAlignmentNear);
		textRect.X += 8;
		textRect.Width -= 8;
		break;
	case TEXTSTRUCT::HALIGN::CENTER:
		format.SetAlignment(Gdiplus::StringAlignmentCenter);
		break;
	case TEXTSTRUCT::HALIGN::RIGHT:
		format.SetAlignment(Gdiplus::StringAlignmentFar);
		textRect.Width -= 8;
		break;
	default:
		break;
	}

	INT fontStyle = Gdiplus::FontStyleRegular;
	switch (text.style)
	{
	case TEXTSTRUCT::STYLE::BOLD:
		fontStyle |= Gdiplus::FontStyleBold;
		break;
	case TEXTSTRUCT::STYLE::ITALIC:
		fontStyle |= Gdiplus::FontStyleItalic;
		break;
	default:
		break;
	}
	Gdiplus::Font font(text.font, text.fontsize, fontStyle);
	Gdiplus::SolidBrush textBrush(Gdiplus::Color(GetRValue(text.color), GetGValue(text.color), GetBValue(text.color)));
	gdc->DrawString(text.text, -1, &font, textRect, &format, &textBrush);
}

void ItemWnd::PaintCircle(Gdiplus::Graphics* gdc, CRect rect)
{
	COLORREF bg = GetBackgroundColor();
	Gdiplus::SolidBrush brush(Gdiplus::Color(GetRValue(bg), GetGValue(bg), GetBValue(bg)));
	gdc->FillEllipse(&brush, rect.left, rect.top, rect.Width(), rect.Height());

	COLORREF ol = GetOutlineColor();
	Gdiplus::Pen pen(Gdiplus::Color(GetRValue(ol), GetGValue(ol), GetBValue(ol)));
	gdc->DrawEllipse(&pen, rect.left, rect.top, rect.Width(), rect.Height());
}

void ItemWnd::PaintSquare(Gdiplus::Graphics* gdc, CRect rect)
{
	int radius = shape.radius;
	if (radius == 0)
	{
		COLORREF bg = GetBackgroundColor();
		Gdiplus::SolidBrush brush(Gdiplus::Color(GetRValue(bg), GetGValue(bg), GetBValue(bg)));
		gdc->FillRectangle(&brush, Gdiplus::Rect(rect.left, rect.top, rect.Width(), rect.Height()));

		COLORREF ol = GetOutlineColor();
		Gdiplus::Pen pen(Gdiplus::Color(GetRValue(ol), GetGValue(ol), GetBValue(ol)));
		gdc->DrawRectangle(&pen, Gdiplus::Rect(rect.left, rect.top, rect.Width(), rect.Height()));
	}
	else
	{
		ContainerWnd* pParent = dynamic_cast<ContainerWnd*>(GetParent());
		if (pParent != NULL)
		{
			if (pParent->GetDirection() == DIRECTION::HORIZONTAL && rect.Width() / 2 < radius)
			{
				radius = rect.Width() / 2;
			}
			else if (pParent->GetDirection() == DIRECTION::VERTICAL && rect.Height() / 2 < radius)
			{
				radius = rect.Height() / 2;
			}
		}
		Gdiplus::GraphicsPath path;
		path.AddArc(rect.left, rect.top, radius, radius, 180, 90);
		path.AddArc(rect.right - radius, rect.top, radius, radius, 270, 90);
		path.AddArc(rect.right - radius, rect.bottom - radius, radius, radius, 0, 90);
		path.AddArc(rect.left, rect.bottom - radius, radius, radius, 90, 90);
		path.CloseFigure();

		COLORREF bg = GetBackgroundColor();
		Gdiplus::SolidBrush brush(Gdiplus::Color(GetRValue(bg), GetGValue(bg), GetBValue(bg)));
		gdc->FillPath(&brush, &path);

		COLORREF ol = GetOutlineColor();
		Gdiplus::Pen pen(Gdiplus::Color(GetRValue(ol), GetGValue(ol), GetBValue(ol)));
		gdc->DrawPath(&pen, &path);
	}
}

void ItemWnd::PaintTriangle(Gdiplus::Graphics* gdc, CRect rect)
{
	Gdiplus::Point points[3] = {
	Gdiplus::Point((rect.left + rect.right) / 2, rect.top),
	Gdiplus::Point(rect.left, rect.bottom),
	Gdiplus::Point(rect.right, rect.bottom)
	};

	Gdiplus::PointF center((rect.left + rect.right) / 2.0f, (rect.top + rect.bottom) / 2.0f);
	float angleRad = shape.angle * (3.14159265f / 180.0f);
	for (int i = 0; i < 3; ++i)
	{
		float dx = points[i].X - center.X;
		float dy = points[i].Y - center.Y;

		float rotatedX = dx * cos(angleRad) - dy * sin(angleRad);
		float rotatedY = dx * sin(angleRad) + dy * cos(angleRad);

		points[i].X = center.X + rotatedX;
		points[i].Y = center.Y + rotatedY;
	}

	COLORREF bg = GetBackgroundColor();
	Gdiplus::SolidBrush brush(Gdiplus::Color(GetRValue(bg), GetGValue(bg), GetBValue(bg)));
	gdc->FillPolygon(&brush, points, 3);

	COLORREF ol = GetOutlineColor();
	Gdiplus::Pen pen(Gdiplus::Color(GetRValue(ol), GetGValue(ol), GetBValue(ol)));
	gdc->DrawPolygon(&pen, points, 3);
}

void ItemWnd::PaintPlus(Gdiplus::Graphics* gdc, CRect rect)
{
	Gdiplus::PointF center((rect.left + rect.right) / 2.0f, (rect.top + rect.bottom) / 2.0f);
	float halfLength = min(rect.Width(), rect.Height()) / 2.5f;
	float angleRad = shape.angle * (3.14159265f / 180.0f);

	Gdiplus::PointF verticalStart(center.X, center.Y - halfLength);
	Gdiplus::PointF verticalEnd(center.X, center.Y + halfLength);

	Gdiplus::PointF horizontalStart(center.X - halfLength, center.Y);
	Gdiplus::PointF horizontalEnd(center.X + halfLength, center.Y);

	auto RotatePoint = [&](Gdiplus::PointF pt) -> Gdiplus::PointF {
		float dx = pt.X - center.X;
		float dy = pt.Y - center.Y;

		float rotatedX = dx * cos(angleRad) - dy * sin(angleRad);
		float rotatedY = dx * sin(angleRad) + dy * cos(angleRad);

		return Gdiplus::PointF(center.X + rotatedX, center.Y + rotatedY);
		};

	Gdiplus::PointF v1 = RotatePoint(verticalStart);
	Gdiplus::PointF v2 = RotatePoint(verticalEnd);
	Gdiplus::PointF h1 = RotatePoint(horizontalStart);
	Gdiplus::PointF h2 = RotatePoint(horizontalEnd);

	COLORREF ol = GetOutlineColor();
	Gdiplus::Pen pen(Gdiplus::Color(GetRValue(ol), GetGValue(ol), GetBValue(ol)), 1.6f);

	gdc->DrawLine(&pen, v1, v2);
	gdc->DrawLine(&pen, h1, h2);
}