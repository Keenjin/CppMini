
// UnPressQMPacketDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "UnPressQMPacket.h"
#include "UnPressQMPacketDlg.h"
#include "afxdialogex.h"
#include <thread>
#include <string>
#include <filesystem>

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


// CUnPressQMPacketDlg 对话框



CUnPressQMPacketDlg::CUnPressQMPacketDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_UNPRESSQMPACKET_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUnPressQMPacketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_instFile);
	DDX_Control(pDX, IDC_EDIT2, m_dstDir);
	DDX_Control(pDX, IDOK2, m_unpress);
}

BEGIN_MESSAGE_MAP(CUnPressQMPacketDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BROWSE, &CUnPressQMPacketDlg::OnBnClickedBrowseFile)
	ON_BN_CLICKED(IDCANCEL2, &CUnPressQMPacketDlg::OnBnClickedBrowseDst)
	ON_BN_CLICKED(IDOK2, &CUnPressQMPacketDlg::OnBnClickedUnpress)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CUnPressQMPacketDlg 消息处理程序

BOOL CUnPressQMPacketDlg::OnInitDialog()
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

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUnPressQMPacketDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		if ((nID & 0xFFF0) == SC_CLOSE) {
			// 点击了关闭按钮，如果当前正在解压，则提示一下，不要立刻退出
			if (m_unpressing) {
				auto ret = this->MessageBoxW(L"当前正在解压安装包，是否立刻终止？", 0, MB_YESNO);
				if (ret != IDYES) {
					return;
				}

				// 强杀子进程
				if (m_hProc != NULL) {
					TerminateProcess(m_hProc, 1);
				}
			}
		}
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUnPressQMPacketDlg::OnPaint()
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
HCURSOR CUnPressQMPacketDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CUnPressQMPacketDlg::OnBnClickedBrowseFile()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, nullptr, nullptr, OFN_HIDEREADONLY | OFN_READONLY, L"文件（*.exe）|*.exe||");
	auto ret = dlg.DoModal();
	if (ret == IDOK)
	{
		auto file = dlg.GetPathName();
		m_instFile.SetWindowTextW(file.GetString());

		if (!file.IsEmpty()) {
			std::filesystem::path p(file.GetString());
			auto parent = p.parent_path();

			// 添加文件名，作为目录路径
			// 用文件名作为新目录
			std::filesystem::path newDir(parent);
			newDir /= p.filename().replace_extension();
			std::filesystem::remove_all(newDir);

			m_dstDir.SetWindowTextW(newDir.wstring().c_str());
		}

		EnableUnpress();
	}
}


void CUnPressQMPacketDlg::EnableUnpress()
{
	CString instFile, dstPath;
	m_instFile.GetWindowTextW(instFile);
	m_dstDir.GetWindowTextW(dstPath);

	if (!instFile.IsEmpty() && !dstPath.IsEmpty()) {
		m_unpress.EnableWindow(TRUE);
	}
}

void CUnPressQMPacketDlg::OnBnClickedBrowseDst()
{
	// TODO: 在此添加控件通知处理程序代码

	CString    strFolderPath;
	BROWSEINFO broInfo = { 0 };
	TCHAR       szDisName[MAX_PATH] = { 0 };

	broInfo.hwndOwner = this->m_hWnd;
	broInfo.pidlRoot = NULL;
	broInfo.pszDisplayName = szDisName;
	broInfo.lpszTitle = _T("选择保存路径");
	broInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_DONTGOBELOWDOMAIN
		| BIF_BROWSEFORCOMPUTER | BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;
	broInfo.lpfn = NULL;
	broInfo.lParam = NULL;
	broInfo.iImage = IDR_MAINFRAME;

	LPITEMIDLIST pIDList = SHBrowseForFolder(&broInfo);
	if (pIDList != NULL)
	{
		memset(szDisName, 0, sizeof(szDisName));
		SHGetPathFromIDListW(pIDList, szDisName);
		strFolderPath = szDisName;
	}

	if (!strFolderPath.IsEmpty())
	{
		// 如果这个目录不为空，并且不是前面创建的目录，则需要新建一个子目录
		std::filesystem::path dstDir(strFolderPath.GetString());
		if (!std::filesystem::is_empty(dstDir)) {
			CString instFile;
			m_instFile.GetWindowTextW(instFile);
			std::filesystem::path p(instFile.GetString());
			std::filesystem::path newDir(p.parent_path());
			newDir /= p.filename().replace_extension();
			if (dstDir.compare(newDir) != 0 && dstDir.lexically_relative(newDir).empty()) {
				// 创建一个新的子目录
				dstDir /= p.filename().replace_extension();
			}
		}

		m_dstDir.SetWindowTextW(dstDir.wstring().c_str());

		EnableUnpress();
	}
}

bool StartProcess(const std::wstring& exePath, const std::wstring& commandline, bool wait/* = false*/, HANDLE& proc)
{
	if (exePath.empty() && commandline.empty()) return false;

	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };
	bool ret = !!CreateProcessW(exePath.c_str(), (LPWSTR)commandline.c_str(), NULL, NULL, false, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	if (ret)
	{
		proc = pi.hProcess;
		if (wait) WaitForSingleObject(pi.hProcess, INFINITE);
		proc = NULL;
		if (pi.hThread) CloseHandle(pi.hThread);
		if (pi.hProcess) CloseHandle(pi.hProcess);
	}
	return ret;
}

void CUnPressQMPacketDlg::OnBnClickedUnpress()
{
	// TODO: 在此添加控件通知处理程序代码
	m_unpress.EnableWindow(FALSE);
	m_unpress.SetWindowTextW(L"解压中...");

	CString instFile, dstPath;
	m_instFile.GetWindowTextW(instFile);
	m_dstDir.GetWindowTextW(dstPath);

	std::filesystem::path pi(instFile.GetString());
	std::filesystem::path po(dstPath.GetString());
	if (!std::filesystem::exists(pi)) {
		CString msg;
		msg.Format(L"安装包（%s）不存在，重新选择", instFile.GetString());
		CDialogEx::MessageBoxW(msg.GetString());
		m_instFile.SetWindowTextW(L"");
		m_unpress.EnableWindow(TRUE);
	}
	else 
	{
		if (std::filesystem::exists(po)) {
			std::filesystem::remove_all(po);
		}

		std::filesystem::create_directories(po);
		
		// 启动线程，执行解压
		std::thread([this, pi, po]() {

			m_unpressing = true;

			CString cmdline;
			cmdline.Format(L"\"%s\" ##DbgExtract=1^&DbgExtractpath=\"%s\"", pi.wstring().c_str(), po.wstring().c_str());
			StartProcess(pi.wstring(), cmdline.GetString(), true, m_hProc);

			m_unpressing = false;

			// 打开输出目录
			ShellExecute(nullptr, L"open", L"explorer.exe", std::format(L"{}", po.wstring().c_str()).c_str(), nullptr, SW_SHOWNORMAL);
			// 执行完毕，发消息给对话框，关闭对话框
			this->PostMessageW(WM_QUIT);
			}).detach();
	}
}


void CUnPressQMPacketDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码

}
