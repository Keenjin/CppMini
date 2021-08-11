
// UnPressQMPacketDlg.h: 头文件
//

#pragma once
#include <memory>


// CUnPressQMPacketDlg 对话框
class CUnPressQMPacketDlg : public CDialogEx
{
// 构造
public:
	CUnPressQMPacketDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UNPRESSQMPACKET_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_instFile;
	CEdit m_dstDir;
	afx_msg void OnBnClickedBrowseFile();
	afx_msg void OnBnClickedBrowseDst();
	afx_msg void OnBnClickedUnpress();
	CButton m_unpress;
	void EnableUnpress();
	afx_msg void OnDestroy();

	std::atomic_bool m_unpressing = false;
	HANDLE m_hProc = NULL;
};
