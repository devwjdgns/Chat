#pragma once
#include "ContainerWnd.h"

class PaneWnd : public ContainerWnd
{
public:
	PaneWnd(DIRECTION d, CString name, int size, COLORREF bgcolor = CLR_NONE, COLORREF olcolor = CLR_NONE);
	~PaneWnd();

	void Initialize(CWnd* parent, CRect rect) override;
	ElementWnd* AddElement(ElementWnd* element) override;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};