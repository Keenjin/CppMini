#include "help.h"

#include <TlHelp32.h>
#include <Psapi.h>

typedef LONG   NTSTATUS;
typedef LONG    KPRIORITY;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define STATUS_SUCCESS              ((NTSTATUS) 0x00000000)
#define SystemProcessesAndThreadsInformation    5 // 功能号
#define NTAPI    __stdcall

typedef DWORD(WINAPI* PQUERYSYSTEM)(UINT, PVOID, DWORD, PDWORD);

typedef struct _CLIENT_ID
{
	DWORD        UniqueProcess;
	DWORD        UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _VM_COUNTERS
{
	SIZE_T        PeakVirtualSize;
	SIZE_T        VirtualSize;
	ULONG         PageFaultCount;
	SIZE_T        PeakWorkingSetSize;
	SIZE_T        WorkingSetSize;
	SIZE_T        QuotaPeakPagedPoolUsage;
	SIZE_T        QuotaPagedPoolUsage;
	SIZE_T        QuotaPeakNonPagedPoolUsage;
	SIZE_T        QuotaNonPagedPoolUsage;
	SIZE_T        PagefileUsage;
	SIZE_T        PeakPagefileUsage;
} VM_COUNTERS;

// 线程信息结构体
typedef struct _SYSTEM_THREAD_INFORMATION
{
	LARGE_INTEGER   KernelTime;
	LARGE_INTEGER   UserTime;
	LARGE_INTEGER   CreateTime;
	ULONG           WaitTime;
	PVOID           StartAddress;
	CLIENT_ID       ClientId;
	KPRIORITY       Priority;
	KPRIORITY       BasePriority;
	ULONG           ContextSwitchCount;
	LONG            State;// 状态,是THREAD_STATE枚举类型中的一个值
	LONG            WaitReason;//等待原因, KWAIT_REASON中的一个值
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION
{
	ULONG            NextEntryDelta; // 指向下一个结构体的指针
	ULONG            ThreadCount; // 本进程的总线程数
	ULONG            Reserved1[6]; // 保留
	LARGE_INTEGER    CreateTime; // 进程的创建时间
	LARGE_INTEGER    UserTime; // 在用户层的使用时间
	LARGE_INTEGER    KernelTime; // 在内核层的使用时间
	UNICODE_STRING   ProcessName; // 进程名
	KPRIORITY        BasePriority; // 
	ULONG            ProcessId; // 进程ID
	ULONG            InheritedFromProcessId;
	ULONG            HandleCount; // 进程的句柄总数
	ULONG            Reserved2[2]; // 保留
	VM_COUNTERS      VmCounters;
	IO_COUNTERS      IoCounters;
	SYSTEM_THREAD_INFORMATION Threads[5]; // 子线程信息数组
}SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

void _EnumAllProcs(std::vector<ProcInfo>& procs)
{
	NTSTATUS Status = 0;


	PQUERYSYSTEM NtQuerySystemInformation = NULL;
	PSYSTEM_PROCESS_INFORMATION pInfo = { 0 };

	// 获取函数地址
	NtQuerySystemInformation = (PQUERYSYSTEM)
		GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQuerySystemInformation");


	DWORD   dwSize = 0;
	// 获取信息所需的缓冲区大小
	Status = NtQuerySystemInformation(SystemProcessesAndThreadsInformation,//要获取的信息的类型
		NULL, // 用于接收信息的缓冲区
		0,  // 缓冲区大小
		&dwSize
	);
	// 申请缓冲区
	char* pBuff = new char[dwSize];
	pInfo = (PSYSTEM_PROCESS_INFORMATION)pBuff;
	if (pInfo == NULL)
		return;
	// 再次调用函数, 获取信息
	Status = NtQuerySystemInformation(SystemProcessesAndThreadsInformation,//要获取的信息的类型
		pInfo, // 用于接收信息的缓冲区
		dwSize,  // 缓冲区大小
		&dwSize
	);
	if (!NT_SUCCESS(Status)) {/*如果函数执行失败*/
		delete[] pInfo;
		return;
	}

	// 遍历结构体,找到对应的进程
	while (1) {
		// 判断是否还有下一个进程
		if (pInfo->NextEntryDelta == 0)
			break;

		if (pInfo->ProcessName.Buffer) {
			// 判断是否找到了ID
			ProcInfo procInfo;
			procInfo.name.assign(pInfo->ProcessName.Buffer, pInfo->ProcessName.Length);
			procInfo.pid = pInfo->ProcessId;
			procInfo.parrentPid = pInfo->InheritedFromProcessId;
			// 找到该进程下的对应的线程,也就是遍历所有线程
			for (DWORD i = 0; i < pInfo->ThreadCount; i++) {
				procInfo.tids.push_back(pInfo->Threads[i].ClientId.UniqueThread);
			}
			HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION |
				PROCESS_VM_READ,
				FALSE, procInfo.pid);
			if (INVALID_HANDLE_VALUE != hProc && NULL != hProc)
			{
				WCHAR szPath[MAX_PATH + 1] = { 0 };
				GetModuleFileNameEx((HMODULE)hProc, NULL, szPath, MAX_PATH);
				procInfo.path = szPath;
				CloseHandle(hProc);
			}

			procs.push_back(procInfo);
		}

		// 迭代到下一个节点
		pInfo = (PSYSTEM_PROCESS_INFORMATION)(((PUCHAR)pInfo) + pInfo->NextEntryDelta);
	}

	delete[] pBuff;
}

bool EnumProcs(std::vector<ProcInfo>& procs) {
	__try {
		_EnumAllProcs(procs);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
	return true;
}

class EnumWndHelper {
public:
	std::vector<WndInfo>* m_pWndLists = nullptr;
	bool m_onlyTop = true;
};

BOOL CALLBACK EnumChildWndProc(HWND hwnd, LPARAM lParam) {
	EnumWndHelper* pThis = (EnumWndHelper*)lParam;
	WndInfo wndInfo;
	wndInfo.hWnd = hwnd;
	WCHAR szTitle[1024] = { 0 };
	GetWindowTextW(hwnd, szTitle, 1023);
	wndInfo.title = szTitle;
	WCHAR szClass[1024] = { 0 };
	GetClassNameW(hwnd, szClass, 1023);
	wndInfo.className = szClass;
	pThis->m_pWndLists->push_back(wndInfo);
	return TRUE;
}

BOOL CALLBACK EnumThreadWndProc(
	_In_ HWND   hwnd,
	_In_ LPARAM lParam
) {
	EnumWndHelper* pThis = (EnumWndHelper*)lParam;
	WndInfo wndInfo;
	wndInfo.hWnd = hwnd;
	WCHAR szTitle[1024] = { 0 };
	GetWindowTextW(hwnd, szTitle, 1023);
	wndInfo.title = szTitle;
	WCHAR szClass[1024] = { 0 };
	GetClassNameW(hwnd, szClass, 1023);
	wndInfo.className = szClass;
	pThis->m_pWndLists->push_back(wndInfo);

	if (!pThis->m_onlyTop) {
		EnumChildWindows(hwnd, EnumChildWndProc, (LPARAM)pThis);
	}

	return TRUE;
}

bool EnumWnd(int32_t tid, std::vector<WndInfo>& wnds, bool onlyTop) {
	EnumWndHelper helper;
	helper.m_pWndLists = &wnds;
	helper.m_onlyTop = onlyTop;
	return !!EnumThreadWindows(tid, EnumThreadWndProc, (LPARAM)&helper);
}