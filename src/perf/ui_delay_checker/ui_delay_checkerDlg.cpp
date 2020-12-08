
// ui_delay_checkerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "ui_delay_checker.h"
#include "ui_delay_checkerDlg.h"
#include "afxdialogex.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CuidelaycheckerDlg 对话框



CuidelaycheckerDlg::CuidelaycheckerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_UI_DELAY_CHECKER_DIALOG, pParent)
	, m_perfWatcher(new PerfWatcher)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	AfxInitRichEdit2();
}

void CuidelaycheckerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_procLists);
	DDX_Control(pDX, IDC_COMBO2, m_wndList);
	DDX_Control(pDX, IDC_MFCBUTTON2, m_btnCheck);
	DDX_Control(pDX, IDC_RICHEDIT21, m_preview);
	DDX_Control(pDX, IDC_EDIT1, m_sampleGap);
	DDX_Control(pDX, IDC_EDIT2, m_delayTime);
	DDX_Control(pDX, IDC_EDIT7, m_autoStopTime);
	DDX_Control(pDX, IDC_EDIT5, m_sampleTotalCnt);
	DDX_Control(pDX, IDC_EDIT6, m_catonCnt);
	DDX_Control(pDX, IDC_EDIT3, m_catonLevel);
	DDX_Control(pDX, IDC_MFCBUTTON1, m_btnUpdate);
	DDX_Control(pDX, IDC_EDIT4, m_avDelayTime);
	DDX_Control(pDX, IDC_EDIT8, m_catonMax);
}

#define WM_DELAY_TICK WM_USER+1

BEGIN_MESSAGE_MAP(CuidelaycheckerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_32771, &CuidelaycheckerDlg::OnAbout)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CuidelaycheckerDlg::OnCbnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CuidelaycheckerDlg::OnCbnSelchangeCombo2)
	ON_BN_CLICKED(IDC_MFCBUTTON1, &CuidelaycheckerDlg::OnBnClickedMfcbutton1)
	ON_BN_CLICKED(IDC_MFCBUTTON2, &CuidelaycheckerDlg::OnBnClickedMfcbutton2)
	ON_MESSAGE(WM_DELAY_TICK, &CuidelaycheckerDlg::OnDelayTick)
	ON_COMMAND(ID_32772, &CuidelaycheckerDlg::OnOpenExe)
END_MESSAGE_MAP()


// CuidelaycheckerDlg 消息处理程序

BOOL CuidelaycheckerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitUI();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CuidelaycheckerDlg::InitUI() {
	// 获取进程列表
	UpdateProcs();

	// 设置采样默认值
	m_sampleGap.SetWindowTextW(L"1000");
	m_delayTime.SetWindowTextW(L"100");
	m_autoStopTime.SetWindowTextW(L"0");

	m_catonCnt.SetWindowTextW(L"0");
	m_sampleTotalCnt.SetWindowTextW(L"0");
	m_catonLevel.SetWindowTextW(L"0.00");
	m_avDelayTime.SetWindowTextW(L"0");
	m_catonMax.SetWindowTextW(L"0");
}

bool ProcInfoCompare(const ProcInfo& left, const ProcInfo& right) {
	std::wstring leftName = left.name;
	std::wstring rightName = right.name;
	std::transform(leftName.begin(), leftName.end(), leftName.begin(), tolower);
	std::transform(rightName.begin(), rightName.end(), rightName.begin(), tolower);
	if (leftName.compare(rightName) < 0) return true;
	return false;
}

void CuidelaycheckerDlg::UpdateProcs() {
	m_wndList.SetCurSel(-1);
	m_procLists.SetCurSel(-1);
	m_procs.clear();
	EnumProcs(m_procs);
	// 按name排序
	std::sort(m_procs.begin(), m_procs.end(), ProcInfoCompare);

	CString selProc;
	m_procLists.GetWindowTextW(selProc);
	m_procLists.ResetContent();
	for (auto i = 0; i < m_procs.size(); i++) {
		m_procLists.AddString(m_procs[i].name.c_str());
	}
	
	SelectDefault();
}

void CuidelaycheckerDlg::SelectDefault() {
	int pos = m_procLists.FindString(0, L"DYToolEx.exe");
	if (pos == -1) {
		pos = m_procLists.FindString(0, L"DYTool.exe");
	}
	if (pos != -1) {
		m_procLists.SetCurSel(pos);
		OnCbnSelchangeCombo1();
	}
}

void CuidelaycheckerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CuidelaycheckerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CuidelaycheckerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CuidelaycheckerDlg::OnAbout()
{
	// TODO: 在此添加命令处理程序代码
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CuidelaycheckerDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	int pos = m_procLists.GetCurSel();
	if (pos != -1) {
		auto procItem = m_procs[pos];

		// 枚举当前窗口
		m_wnds.clear();
		for (auto tid : procItem.tids) {
			EnumWnd(tid, m_wnds);
		}

		// 设置窗口
		m_wndList.ResetContent();
		for (auto i = 0; i < m_wnds.size(); i++) {
			CString wndShowInfo;
			wndShowInfo.Format(L"0x%x - %s - %s", m_wnds[i].hWnd, m_wnds[i].title.c_str(), m_wnds[i].className.c_str());
			m_wndList.AddString(wndShowInfo.GetString());

			if (m_wnds[i].title.compare(L"斗鱼直播伴侣") == 0) {
				m_wndList.SetCurSel(i);
			}
		}
		
		OnCbnSelchangeCombo2();
	}
}


void CuidelaycheckerDlg::OnCbnSelchangeCombo2()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CuidelaycheckerDlg::OnBnClickedMfcbutton1()
{
	// TODO: 在此添加控件通知处理程序代码
	// 点击刷新，重新获取一下进程列表，然后重新
	UpdateProcs();
}

void CuidelaycheckerDlg::OnTickCheckReply(std::shared_ptr<PerfValue> perfValue) {
	// 发到UI线程，刷新
	PerfValue* value = new PerfValue(*perfValue.get());
	PostMessage(WM_DELAY_TICK, (WPARAM)value);
}

LRESULT CuidelaycheckerDlg::OnDelayTick(WPARAM perfV, LPARAM) {
	// 采样一次
	std::unique_ptr<PerfValue> perfValue((PerfValue*)perfV);
	m_dwCurrentSampleCnt++;
	CString sampleTotalCnt;
	sampleTotalCnt.Format(L"%d", m_dwCurrentSampleCnt);
	m_sampleTotalCnt.SetWindowTextW(sampleTotalCnt.GetString());

	if (perfValue->bCaton) {
		m_dwCurrentCatonCnt++;
		CString catonCnt;
		catonCnt.Format(L"%d", m_dwCurrentCatonCnt);
		m_catonCnt.SetWindowTextW(catonCnt.GetString());
	}

	// 卡顿率
	CString catonLevel;
	catonLevel.Format(L"%0.2f", m_dwCurrentCatonCnt * 100.0f / (m_dwCurrentSampleCnt * 1.0f));
	m_catonLevel.SetWindowTextW(catonLevel.GetString());

	// 平均卡顿时长和峰值
	CString avDelayTime, maxDelayTime;
	avDelayTime.Format(L"%0.2f", perfValue->dwAverageCatonDelay);
	m_avDelayTime.SetWindowTextW(avDelayTime.GetString());
	maxDelayTime.Format(L"%d", perfValue->dwMaxCatonDelay);
	m_catonMax.SetWindowTextW(maxDelayTime.GetString());

	int timeout = perfValue->bCaton ? 1 : 0;
	m_preview.SetSel(-1, -1);
	CTime time(_time64(nullptr));
	CString msg;
	msg.Format(L"%04d-%02d-%02d %02d:%02d:%02d   %d   %dms\n",
		time.GetYear(), time.GetMonth(), time.GetDay(),
		time.GetHour(), time.GetMinute(), time.GetSecond(),
		timeout, timeout * perfValue->dwDelayTime);
	m_preview.ReplaceSel(msg.GetString());

	// 自动滚动到最后一行
	m_preview.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	if (perfValue->bCatonDead) {
		GetDlgItem(IDC_STATIC_HUNG)->ShowWindow(SW_SHOW);
		OnBnClickedMfcbutton2();
	}

	// 判断是否需要停止
	if (m_dwTotalSampleTotalCnt != 0 && m_dwTotalSampleTotalCnt == m_dwCurrentSampleCnt) {
		OnBnClickedMfcbutton2();
	}

	return S_OK;
}

void CuidelaycheckerDlg::ChangeState() {
	BOOL bEnable = m_bStartOrStop ? FALSE : TRUE;
	m_sampleGap.EnableWindow(bEnable);
	m_delayTime.EnableWindow(bEnable);
	m_autoStopTime.EnableWindow(bEnable);
	m_procLists.EnableWindow(bEnable);
	m_wndList.EnableWindow(bEnable);
	m_btnUpdate.EnableWindow(bEnable);

	if (m_bStartOrStop)
		GetDlgItem(IDC_STATIC_HUNG)->ShowWindow(SW_HIDE);
}

void CuidelaycheckerDlg::OnBnClickedMfcbutton2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bStartOrStop = !m_bStartOrStop;
	ChangeState();
	if (m_bStartOrStop) {
		m_btnCheck.SetWindowTextW(L"停止检测");

		m_catonCnt.SetWindowTextW(L"0");
		m_sampleTotalCnt.SetWindowTextW(L"0");
		m_catonLevel.SetWindowTextW(L"0.00");
		m_avDelayTime.SetWindowTextW(L"0");
		m_catonMax.SetWindowTextW(L"0");

		// 当前开始检测，需要做一些校验动作
		int pos = m_wndList.GetCurSel();
		if (pos == -1) {
			MessageBox(L"当前未选择目标窗口！", L"错误", MB_OK | MB_ICONERROR);
			OnBnClickedMfcbutton2();
			return;
		}

		HWND hTargetWnd = m_wnds[pos].hWnd;
		if (!IsWindow(hTargetWnd)) {
			MessageBox(L"当前选择窗口已失效，请点击刷新，尝试重新获取目标窗口！", L"错误", MB_OK | MB_ICONERROR);
			OnBnClickedMfcbutton2();
			return;
		}

		// 采样多少秒自动停止
		CString autoStopTimeTxt;
		m_autoStopTime.GetWindowTextW(autoStopTimeTxt);
		int autoStopTime = _wtoi(autoStopTimeTxt.GetString());

		// 获取采样数，delay数
		CString sampleGapTxt, delayTimeTxt;
		m_sampleGap.GetWindowTextW(sampleGapTxt);
		m_delayTime.GetWindowTextW(delayTimeTxt);
		int sampleGap = _wtoi(sampleGapTxt.GetString());
		int delayTime = _wtoi(delayTimeTxt.GetString());

		// 计算出总采样次数
		if (sampleGap > 0)
			m_dwTotalSampleTotalCnt = autoStopTime / sampleGap;
		else
			m_dwTotalSampleTotalCnt = 0;
		m_dwCurrentSampleCnt = 0;
		m_dwCurrentCatonCnt = 0;

		m_perfWatcher->Stop();
		std::shared_ptr<PerfAnalyzer> perfAnalyzer(new PerfAnalyzer(hTargetWnd, sampleGap, delayTime));
		m_perfWatcher->Start(std::static_pointer_cast<PerfWatcher::PerfOption>(perfAnalyzer),
			std::bind(&CuidelaycheckerDlg::OnTickCheckReply, this, std::placeholders::_1));
		
	}
	else {
		m_btnCheck.SetWindowTextW(L"开始检测");
		m_perfWatcher->Stop();
	}
}

void CuidelaycheckerDlg::OnOpenExe()
{
	// TODO: 在此添加命令处理程序代码
	// 打开文件对话框，选择exe。默认打开斗鱼的安装目录
	CFileDialog fileDlg(TRUE, L"可执行文件(*.exe)|*.exe", L"DYToolEx.exe",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"可执行文件(*.exe)|*.exe||", this);
	fileDlg.DoModal();
	CString exePath = fileDlg.GetPathName();
	// 打开进程
}
