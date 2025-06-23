#pragma once
#include "ContainerWnd.h"

class ScrollWnd : public ContainerWnd
{
public:
	ScrollWnd(DIRECTION d, CString name, int size, COLORREF bgcolor = CLR_NONE, COLORREF olcolor = CLR_NONE);
	~ScrollWnd();

	void MoveToStart();
	void MoveToEnd();
	void UsePaintEx();

	void Initialize(CWnd* parent, CRect rect) override;
	BOOL AddElement(ElementWnd* element) override;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

private:
	void MoveChildWindow();
private:
	BOOL usePaintex;
	CSize clientSize;
	CSize totalSize;
	CPoint scrollPos;
};