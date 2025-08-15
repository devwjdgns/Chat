#pragma once

#include "PaneWnd.h"
#include "ScrollWnd.h"
#include "ButtonWnd.h"
#include "EditWnd.h"
#include <memory>

#define WM_SEARCH_USER_ACTION (WM_USER + 201)
#define WM_ADD_FRIEND_ACTION (WM_USER + 202)

class ChatManager;

class CFriendDlg : public CWnd
{
public:
	CFriendDlg(std::shared_ptr<ChatManager> cm);
	~CFriendDlg();

	BOOL CreateWnd(CWnd* parent, CRect rect);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg LRESULT OnButtonClick(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnSearchUserAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddFriendAction(WPARAM wParam, LPARAM lParam);

private:
	std::shared_ptr<ChatManager> chatManager;
	std::unique_ptr<PaneWnd> mainView;
	int sel;
};