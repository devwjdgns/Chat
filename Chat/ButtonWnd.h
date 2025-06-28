#pragma once
#include "ItemWnd.h"

class ButtonWnd : public ItemWnd
{
public:
	ButtonWnd(TEXTSTRUCT text, SHAPESTRUCT shape, CString name, int size, COLORREF bgcolor = CLR_NONE, COLORREF olcolor = CLR_NONE);
	~ButtonWnd();

	COLORREF GetBackgroundColor() override;

	void SetEnableStatus(BOOL e) { enable = e; }
	void SetPressedStatus(BOOL p) { pressed = p; }
	void AddContextMenu(CString item) { menus.Add(item); }

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);
	afx_msg void OnDynamicMenuCommand(UINT nID);

private:
	BOOL focus;
	BOOL click;
	BOOL enable;
	BOOL pressed;
	CArray<CString, CString&> menus;
};