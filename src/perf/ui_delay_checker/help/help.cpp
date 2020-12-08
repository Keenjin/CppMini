#include "help.h"

#include <TlHelp32.h>
#include <Psapi.h>

typedef LONG   NTSTATUS;
typedef LONG    KPRIORITY;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define STATUS_SUCCESS              ((NTSTATUS) 0x00000000)
#define SystemProcessesAndThreadsInformation    5 // ���ܺ�
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

// �߳���Ϣ�ṹ��
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
	LONG            State;// ״̬,��THREAD_STATEö�������е�һ��ֵ
	LONG            WaitReason;//�ȴ�ԭ��, KWAIT_REASON�е�һ��ֵ
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION
{
	ULONG            NextEntryDelta; // ָ����һ���ṹ���ָ��
	ULONG            ThreadCount; // �����̵����߳���
	ULONG            Reserved1[6]; // ����
	LARGE_INTEGER    CreateTime; // ���̵Ĵ���ʱ��
	LARGE_INTEGER    UserTime; // ���û����ʹ��ʱ��
	LARGE_INTEGER    KernelTime; // ���ں˲��ʹ��ʱ��
	UNICODE_STRING   ProcessName; // ������
	KPRIORITY        BasePriority; // 
	ULONG            ProcessId; // ����ID
	ULONG            InheritedFromProcessId;
	ULONG            HandleCount; // ���̵ľ������
	ULONG            Reserved2[2]; // ����
	VM_COUNTERS      VmCounters;
	IO_COUNTERS      IoCounters;
	SYSTEM_THREAD_INFORMATION Threads[5]; // ���߳���Ϣ����
}SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

void _EnumAllProcs(std::vector<ProcInfo>& procs)
{
	NTSTATUS Status = 0;


	PQUERYSYSTEM NtQuerySystemInformation = NULL;
	PSYSTEM_PROCESS_INFORMATION pInfo = { 0 };

	// ��ȡ������ַ
	NtQuerySystemInformation = (PQUERYSYSTEM)
		GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQuerySystemInformation");


	DWORD   dwSize = 0;
	// ��ȡ��Ϣ����Ļ�������С
	Status = NtQuerySystemInformation(SystemProcessesAndThreadsInformation,//Ҫ��ȡ����Ϣ������
		NULL, // ���ڽ�����Ϣ�Ļ�����
		0,  // ��������С
		&dwSize
	);
	// ���뻺����
	char* pBuff = new char[dwSize];
	pInfo = (PSYSTEM_PROCESS_INFORMATION)pBuff;
	if (pInfo == NULL)
		return;
	// �ٴε��ú���, ��ȡ��Ϣ
	Status = NtQuerySystemInformation(SystemProcessesAndThreadsInformation,//Ҫ��ȡ����Ϣ������
		pInfo, // ���ڽ�����Ϣ�Ļ�����
		dwSize,  // ��������С
		&dwSize
	);
	if (!NT_SUCCESS(Status)) {/*�������ִ��ʧ��*/
		delete[] pInfo;
		return;
	}

	// �����ṹ��,�ҵ���Ӧ�Ľ���
	while (1) {
		// �ж��Ƿ�����һ������
		if (pInfo->NextEntryDelta == 0)
			break;

		if (pInfo->ProcessName.Buffer) {
			// �ж��Ƿ��ҵ���ID
			ProcInfo procInfo;
			procInfo.name.assign(pInfo->ProcessName.Buffer, pInfo->ProcessName.Length);
			procInfo.pid = pInfo->ProcessId;
			procInfo.parrentPid = pInfo->InheritedFromProcessId;
			// �ҵ��ý����µĶ�Ӧ���߳�,Ҳ���Ǳ��������߳�
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

		// ��������һ���ڵ�
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