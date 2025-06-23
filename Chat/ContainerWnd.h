#pragma once
#include "ElementWnd.h"

#include <vector>

class ContainerWnd : public ElementWnd
{
public:
	ContainerWnd(DIRECTION d, CString name, int size, COLORREF bgcolor = CLR_NONE, COLORREF olcolor = CLR_NONE);
	~ContainerWnd();

	DIRECTION GetDirection() { return direction; }

	ElementWnd* GetElement(int idx);
	ElementWnd* FindElement(CString name);
	void ClearElement();
	int GetElementCount();

	void Initialize(CWnd* parent, CRect rect) override;
	void PaintExtern(CDC* pdc, CRect rect) override;
	int GetMinSize(DIRECTION d) override;

	virtual BOOL AddElement(ElementWnd* element);

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

protected:
	DIRECTION direction;
	std::vector<ElementWnd*> elements;
};