#pragma once
#include "ItemWnd.h"

class SingleEdit : public CEdit
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

class MultiEdit : public CRichEditCtrl
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

class EditWnd : public ItemWnd
{
public:
	EditWnd(TEXTSTRUCT text, SHAPESTRUCT shape, CString name, int size, COLORREF bgcolor = CLR_NONE, COLORREF olcolor = CLR_NONE);
	~EditWnd();

	void SetStylePassword();
	void SetStyleMultiLine();
	void ClearText();

	CString GetItemText() override;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();

private:
	BOOL password;
	BOOL multiline;
	SingleEdit singleEdit;
	MultiEdit multiEdit;
	CFont font;
};