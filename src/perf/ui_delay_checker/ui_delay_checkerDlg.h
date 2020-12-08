
// ui_delay_checkerDlg.h: 头文件
//

#pragma once
#include "help/help.h"
#include "perf/perf_analyzer.h"

// CuidelaycheckerDlg 对话框
class CuidelaycheckerDlg : public CDialogEx
{
// 构造
public:
	CuidelaycheckerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UI_DELAY_CHECKER_DIALOG };
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

private:
	void InitUI();
	void UpdateProcs();
	void SelectDefault();

	void OnTickCheckReply(std::shared_ptr<PerfValue> perfValue);
	void ChangeState();

public:
	afx_msg void OnAbout();
	CComboBox m_procLists;
	afx_msg void OnCbnSelchangeCombo1();
	std::vector<ProcInfo> m_procs;
	CComboBox m_wndList;
	std::vector<WndInfo> m_wnds;
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnBnClickedMfcbutton1();
	afx_msg void OnBnClickedMfcbutton2();
	afx_msg LRESULT OnDelayTick(WPARAM, LPARAM);
	CMFCButton m_btnCheck;
	bool m_bStartOrStop = false;
	CRichEditCtrl m_preview;

	std::unique_ptr<PerfWatcher> m_perfWatcher;
	CEdit m_sampleGap;
	CEdit m_delayTime;
	CEdit m_autoStopTime;
	CEdit m_sampleTotalCnt;
	CEdit m_catonCnt;
	CEdit m_catonLevel;

	DWORD m_dwTotalSampleTotalCnt = 0;
	DWORD m_dwCurrentSampleCnt = 0;
	DWORD m_dwCurrentCatonCnt = 0;
	CMFCButton m_btnUpdate;
	CEdit m_avDelayTime;
	CEdit m_catonMax;
	afx_msg void OnOpenExe();
};
