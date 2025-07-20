
// ChatDlg.h: 헤더 파일
//

#pragma once
#include "PaneWnd.h"
#include "ScrollWnd.h"
#include "ButtonWnd.h"
#include "EditWnd.h"
#include "ChatManager.h"

#define WM_LOGIN_ACTION (WM_USER + 100)
#define WM_REGISTER_ACTION (WM_USER + 101)
#define WM_LOGOUT_ACTION (WM_USER + 102)
#define WM_DELETE_FRIEND_ACTION (WM_USER + 103)
#define WM_SEARCH_FRIEND_ACTION (WM_USER + 104)
#define WM_DELETE_ROOM_ACTION (WM_USER + 105)
#define WM_SEARCH_ROOM_ACTION (WM_USER + 106)
#define WM_SEARCH_MESSAGE_ACTION (WM_USER + 107)

#define WM_MESSAGE_RECEIVED (WM_USER + 120)

enum PAGE_NAME
{
	LOGIN = 0,
	REGISTER,
	MAINCHAT
};

struct MessageData
{
	CString name;
	CString message;
	CString timestamp;
};

class CFriendDlg;
class CRoomDlg;
// CChatDlg 대화 상자
class CChatDlg : public CDialogEx
{
// 생성입니다.
public:
	CChatDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	~CChatDlg();
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_DIALOG };
#endif

	void MovePage(PAGE_NAME name);
	CFriendDlg* GetFriendDlg();
	CRoomDlg* GetRoomDlg();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnButtonMenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnButtonClick(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEditComplete(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnLoginAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRegisterAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLogoutAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddFriendAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeleteFriendAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSearchFriendAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenFriendAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCreateRoomAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeleteRoomAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSearchRoomAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSearchMessageAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageReceived(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	void InitLoginView();
	void InitRegisterView();
	void InitMainChatView();
	void UpdateWindowStyle(BOOL resize, BOOL minsize, BOOL maxsize);

	void SendChatMessage(CString message, CString timestamp, BOOL update = TRUE);
	void ReceiveChatMessage(CString name, CString message, CString timestamp, BOOL update = TRUE);

public:
	int roomId;

private:
	ULONG_PTR gdiplusToken;
	PaneWnd* loginView;
	PaneWnd* registerView;
	PaneWnd* mainChatView;
	ChatManager* chatManager;

private:
	CFriendDlg* friendDlg;
	CRoomDlg* roomDlg;
};