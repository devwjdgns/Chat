#pragma once

#include "PaneWnd.h"
#include "ScrollWnd.h"
#include "ButtonWnd.h"
#include "EditWnd.h"

#define WM_CREATE_ROOM_ACTION (WM_USER + 301)

class ChatManager;

class CRoomDlg : public CWnd
{
public:
	CRoomDlg(ChatManager* cm);
	~CRoomDlg();

	BOOL CreateWnd(CWnd* parent, CRect rect, CArray<CString, CString>& friends);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg LRESULT OnButtonClick(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnCreateRoomAction(WPARAM wParam, LPARAM lParam);

private:
	void InitFriendList(CArray<CString, CString>& friends);

private:
	ChatManager* chatManager;
	PaneWnd* mainView;
	CArray<BOOL, BOOL> sel;
	CArray<CString, CString> friendList;
};