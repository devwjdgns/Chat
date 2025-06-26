
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

#define WM_MESSAGE_RECEIVED (WM_USER + 200)

enum PAGE_NAME
{
	LOGIN = 0,
	REGISTER,
	MAINCHAT
};

class CFriendDlg;
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
	afx_msg LRESULT OnButtonClick(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEditComplete(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnLoginAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRegisterAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLogoutAction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageReceived(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	void InitLoginView();
	void InitRegisterView();
	void InitMainChatView();
	void UpdateWindowStyle(BOOL resize, BOOL minsize, BOOL maxsize);

	void SendChatMessage(CString time, CString chat);
	void ReceiveChatMessage(CString time, CString chat);

private:
	ULONG_PTR m_gdiplusToken;
	PaneWnd* loginView;
	PaneWnd* registerView;
	PaneWnd* mainChatView;
	ChatManager* chatManager;

private:
	CFriendDlg* friendDlg;
};