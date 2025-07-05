#pragma once
#include "ElementWnd.h"

struct TEXTSTRUCT
{
	enum class STYLE
	{
		NORMAL = 0,
		BOLD = 1,
		ITALIC = 2
	};
	enum class VALIGN
	{
		TOP = 0,
		CENTER = 1,
		BOTTOM = 2
	};
	enum class HALIGN
	{
		LEFT = 0,
		CENTER = 1,
		RIGHT = 2
	};
	TEXTSTRUCT(CString t = _T(""), STYLE s = STYLE::NORMAL, VALIGN v = VALIGN::CENTER, HALIGN h = HALIGN::CENTER, CString f = _T("Segoe UI"), int fs = 10, COLORREF c = RGB(0, 0, 0))
		:text(t), style(s), valign(v), halign(h), font(f), fontsize(fs), color(c) {}
	CString text;
	STYLE style;
	VALIGN valign;
	HALIGN halign;
	CString font;
	int fontsize;
	COLORREF color;
};

struct SHAPESTRUCT
{
	enum class SHAPE
	{
		NONE = 0,
		CIRCLE = 1,
		SQUARE = 2,
		TRIANGLE = 3,
		PLUS = 4
	};
	SHAPESTRUCT(SHAPE s = SHAPE::SQUARE, CRect m = CRect(0, 0, 0, 0), int r = 0, int a = 0)
		: shape(s), margin(m), radius(r), angle(a) {}
	SHAPE shape;
	CRect margin;
	int radius;
	int angle;
};

class ItemWnd : public ElementWnd
{
public:
	ItemWnd(TEXTSTRUCT text, SHAPESTRUCT shape, CString name, int size, COLORREF bgcolor = CLR_NONE, COLORREF olcolor = CLR_NONE);
	~ItemWnd();

	void Initialize(CWnd* parent, CRect rect) override;
	void PaintExtern(CDC* pdc, CRect rect) override;
	int GetMinSize(DIRECTION d) override;
	
	int GetTextWidth();
	int GetTextHeight();
	
	//virtual void SetItemText(CString text);
	virtual CString GetItemText();

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

private:
	void PaintText(Gdiplus::Graphics* gdc, CRect rect);
	void PaintCircle(Gdiplus::Graphics* gdc, CRect rect);
	void PaintSquare(Gdiplus::Graphics* gdc, CRect rect);
	void PaintTriangle(Gdiplus::Graphics* gdc, CRect rect);
	void PaintPlus(Gdiplus::Graphics* gdc, CRect rect);

protected:
	TEXTSTRUCT text;
	SHAPESTRUCT shape;
};