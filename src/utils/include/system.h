#pragma once
#include <windows.h>
#include <string>

namespace utils {

	class WinHandle
	{
	public:
		WinHandle(HANDLE handle = NULL);
		~WinHandle();

		operator HANDLE() const;
		WinHandle& operator = (const HANDLE& handle);
		bool Invalid() const;
		void Close();

	private:
		HANDLE m_handle = NULL;
	};

	enum OSVersion {
		PRE_XP = 0,  // Not supported.
		XP = 1,
		SERVER_2003 = 2,  // Also includes XP Pro x64 and Server 2003 R2.
		VISTA = 3,        // Also includes Windows Server 2008.
		WIN7 = 4,         // Also includes Windows Server 2008 R2.
		WIN8 = 5,         // Also includes Windows Server 2012.
		WIN8_1 = 6,       // Also includes Windows Server 2012 R2.
		WIN10 = 7,        // Threshold 1: Version 1507, Build 10240.
		WIN10_TH2 = 8,    // Threshold 2: Version 1511, Build 10586.
		WIN10_RS1 = 9,    // Redstone 1: Version 1607, Build 14393.
		WIN10_RS2 = 10,   // Redstone 2: Version 1703, Build 15063.
		WIN10_RS3 = 11,   // Redstone 3: Version 1709, Build 16299.
		WIN10_RS4 = 12,   // Redstone 4: Version 1803, Build 17134.
		WIN10_RS5 = 13,   // Redstone 5: Version 1809, Build 17763.
		WIN10_19H1 = 14,  // 19H1: Version 1903, Build 18362.
		// On edit, update tools\metrics\histograms\enums.xml "WindowsVersion" and
		// "GpuBlacklistFeatureTestResultsWindows2".
		WIN_LAST,  // Indicates error condition.
	};

	// A rough bucketing of the available types of versions of Windows. This is used
	// to distinguish enterprise enabled versions from home versions and potentially
	// server versions. Keep these values in the same order, since they are used as
	// is for metrics histogram ids.
	enum VersionType {
		SUITE_HOME = 0,
		SUITE_PROFESSIONAL,
		SUITE_SERVER,
		SUITE_ENTERPRISE,
		SUITE_EDUCATION,
		SUITE_LAST,
	};

	bool IsSystem64Bit();
	OSVersion GetOSVersion();
}