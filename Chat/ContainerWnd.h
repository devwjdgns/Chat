#pragma once
#include "ElementWnd.h"

#include <vector>

class ContainerWnd : public ElementWnd
{
public:
	ContainerWnd(DIRECTION d, CString name, int size, COLORREF bgcolor = CLR_NONE, COLORREF olcolor = CLR_NONE);
	~ContainerWnd();

	DIRECTION GetDirection() { return direction; }

	template<typename T>
	T* GetElement(int idx)
	{
		if (idx >= elements.size()) return nullptr;
		return dynamic_cast<T*>(elements[idx]);
	}
	
	template<typename T>
	T* FindElement(CString name)
	{
		if (GetName().Compare(name) == 0) return dynamic_cast<T*>(this);
		for (int i = 0; i < elements.size(); i++)
		{
			if (elements[i]->GetName().Compare(name) == 0) return dynamic_cast<T*>(elements[i]);
			if (dynamic_cast<ContainerWnd*>(elements[i]) != nullptr)
			{
				if (ElementWnd* tmp = dynamic_cast<ContainerWnd*>(elements[i])->FindElement<ElementWnd>(name))
				{
					return dynamic_cast<T*>(tmp);
				}
			}
		}
		return nullptr;
	}
	
	void DeleteElement(int idx);
	void ClearElement();
	int GetElementCount();

	void Initialize(CWnd* parent, CRect rect) override;
	void PaintExtern(CDC* pdc, CRect rect) override;
	int GetMinSize(DIRECTION d) override;

	virtual ElementWnd* AddElement(ElementWnd* element);

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

protected:
	DIRECTION direction;
	std::vector<ElementWnd*> elements;
};