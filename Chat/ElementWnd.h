#pragma once
#include "gdiplus.h"

#define WM_BUTTON_MENU WM_USER + 1
#define WM_BUTTON_CLICK WM_USER + 2
#define WM_EDIT_COMPLETE WM_USER + 3

enum class ELETYPE
{
	NONE = 0,
	CONTAINER,
	PANE,
	SCROLL,
	ITEM,
	EDIT,
	BUTTON
};

enum class DIRECTION
{
	VERTICAL = 0,
	HORIZONTAL
};

class ElementWnd : public CWnd
{
public:
	ElementWnd(CString name, int size, COLORREF bgcolor = CLR_NONE, COLORREF olcolor = CLR_NONE);
	~ElementWnd();

	void InvalidateParent();
	void SendMessageParent(int flag, CString message);

	void SetSize(int s) { size = s; }

	ELETYPE GetType() { return type; }
	CString GetName() { return name; }
	int GetSize() { return size; }

	void ShowWindowExtern(BOOL b) { show = b; }
	BOOL IsShowWindowExtern() { return show; }

	virtual COLORREF GetBackgroundColor();
	virtual COLORREF GetOutlineColor();

	virtual void Initialize(CWnd* parent, CRect rect) = 0;
	virtual void PaintExtern(CDC* pdc, CRect rect) = 0;
	virtual int GetMinSize(DIRECTION d) = 0;

protected:
	DECLARE_MESSAGE_MAP()

protected:
	ELETYPE type;
	CString name;
	int size;
	COLORREF bgcolor;
	COLORREF olcolor;

private:
	BOOL show;
};