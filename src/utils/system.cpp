#include "include\system.h"
#include "include\macros.h"

namespace utils {

	bool IsSystem64Bit()
	{
#ifdef _WIN64
		return true;
#else
		BOOL x86 = false;
		bool success = !!IsWow64Process(GetCurrentProcess(), &x86);
		return success && !x86;
#endif
	}

	WinHandle::WinHandle(HANDLE handle/* = NULL*/)
		: m_handle(handle)
	{

	}

	WinHandle::~WinHandle()
	{
		Close();
	}

	WinHandle::operator HANDLE() const
	{
		return m_handle;
	}

	WinHandle& WinHandle::operator = (const HANDLE& handle)
	{
		m_handle = handle;
		return *this;
	}

	bool WinHandle::Invalid() const
	{
		if (m_handle == NULL || m_handle == INVALID_HANDLE_VALUE)
		{
			return true;
		}

		return false;
	}

	void WinHandle::Close()
	{
		if (!Invalid())
		{
			CloseHandle(m_handle);
			m_handle = NULL;
		}
	}

	// The values under the CurrentVersion registry hive are mirrored under
	// the corresponding Wow6432 hive.
	constexpr wchar_t kRegKeyWindowsNTCurrentVersion[] =
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

	// Returns the "UBR" (Windows 10 patch number) and "ReleaseId" (Windows 10
	// release number) from the registry. "UBR" is an undocumented value and will be
	// 0 if the value was not found. "ReleaseId" will be an empty string if the
	// value is not found.
	std::pair<int, std::string> GetVersionData() {
		DWORD ubr = 0;
		std::wstring release_id;
		RegKey key;

		if (key.Open(HKEY_LOCAL_MACHINE, kRegKeyWindowsNTCurrentVersion,
			KEY_QUERY_VALUE) == ERROR_SUCCESS) {
			key.ReadValueDW(L"UBR", &ubr);
			key.ReadValue(L"ReleaseId", &release_id);
		}

		return std::make_pair(static_cast<int>(ubr), WideToUTF8(release_id));
	}

	class OSInfo {
	public:
		struct VersionNumber {
			int major;
			int minor;
			int build;
			int patch;
		};

		struct ServicePack {
			int major;
			int minor;
		};

		// The processor architecture this copy of Windows natively uses.  For
		// example, given an x64-capable processor, we have three possibilities:
		//   32-bit Chrome running on 32-bit Windows:           X86_ARCHITECTURE
		//   32-bit Chrome running on 64-bit Windows via WOW64: X64_ARCHITECTURE
		//   64-bit Chrome running on 64-bit Windows:           X64_ARCHITECTURE
		enum WindowsArchitecture {
			X86_ARCHITECTURE,
			X64_ARCHITECTURE,
			IA64_ARCHITECTURE,
			ARM64_ARCHITECTURE,
			OTHER_ARCHITECTURE,
		};

		// Whether a process is running under WOW64 (the wrapper that allows 32-bit
		// processes to run on 64-bit versions of Windows).  This will return
		// WOW64_DISABLED for both "32-bit Chrome on 32-bit Windows" and "64-bit
		// Chrome on 64-bit Windows".  WOW64_UNKNOWN means "an error occurred", e.g.
		// the process does not have sufficient access rights to determine this.
		enum WOW64Status {
			WOW64_DISABLED,
			WOW64_ENABLED,
			WOW64_UNKNOWN,
		};

		static OSInfo* GetInstance() {
			return *GetInstanceStorage();
		}

		static const _SYSTEM_INFO& GetSystemInfoStorage() {
			static const NoDestructor<_SYSTEM_INFO> system_info([] {
				_SYSTEM_INFO info = {};
				::GetNativeSystemInfo(&info);
				return info;
			}());
			return *system_info;
		}

		// Separate from the rest of OSInfo so it can be used during early process
		// initialization.
		static WindowsArchitecture GetArchitecture() {
			switch (GetSystemInfoStorage().wProcessorArchitecture) {
			case PROCESSOR_ARCHITECTURE_INTEL:
				return X86_ARCHITECTURE;
			case PROCESSOR_ARCHITECTURE_AMD64:
				return X64_ARCHITECTURE;
			case PROCESSOR_ARCHITECTURE_IA64:
				return IA64_ARCHITECTURE;
			case PROCESSOR_ARCHITECTURE_ARM64:
				return ARM64_ARCHITECTURE;
			default:
				return OTHER_ARCHITECTURE;
			}
		}

		// Like wow64_status(), but for the supplied handle instead of the current
		// process.  This doesn't touch member state, so you can bypass the singleton.
		static WOW64Status GetWOW64StatusForProcess(HANDLE process_handle);

		const OSVersion& version() const { return version_; }

		OSVersion Kernel32Version() const;
		std::string Kernel32BaseVersion() const;
		// The next two functions return arrays of values, [major, minor(, build)].
		const VersionNumber& version_number() const { return version_number_; }
		const VersionType& version_type() const { return version_type_; }
		const ServicePack& service_pack() const { return service_pack_; }
		const std::string& service_pack_str() const { return service_pack_str_; }
		const int& processors() const { return processors_; }
		const size_t& allocation_granularity() const {
			return allocation_granularity_;
		}
		const WOW64Status& wow64_status() const { return wow64_status_; }
		std::string processor_model_name();
		const std::string& release_id() const { return release_id_; }

	private:
		static OSInfo** GetInstanceStorage();

		OSInfo(const _OSVERSIONINFOEXW& version_info,
			const _SYSTEM_INFO& system_info,
			int os_type)
			: version_(OSVersion::PRE_XP)
			, wow64_status_(GetWOW64StatusForProcess(GetCurrentProcess())) {
			version_number_.major = version_info.dwMajorVersion;
			version_number_.minor = version_info.dwMinorVersion;
			version_number_.build = version_info.dwBuildNumber;
			std::tie(version_number_.patch, release_id_) = GetVersionData();
			version_ = MajorMinorBuildToVersion(
				version_number_.major, version_number_.minor, version_number_.build);
			service_pack_.major = version_info.wServicePackMajor;
			service_pack_.minor = version_info.wServicePackMinor;
			service_pack_str_ = WideToUTF8(version_info.szCSDVersion);

			processors_ = system_info.dwNumberOfProcessors;
			allocation_granularity_ = system_info.dwAllocationGranularity;

			if (version_info.dwMajorVersion == 6 || version_info.dwMajorVersion == 10) {
				// Only present on Vista+.
				switch (os_type) {
				case PRODUCT_CLUSTER_SERVER:
				case PRODUCT_DATACENTER_SERVER:
				case PRODUCT_DATACENTER_SERVER_CORE:
				case PRODUCT_ENTERPRISE_SERVER:
				case PRODUCT_ENTERPRISE_SERVER_CORE:
				case PRODUCT_ENTERPRISE_SERVER_IA64:
				case PRODUCT_SMALLBUSINESS_SERVER:
				case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				case PRODUCT_STANDARD_SERVER:
				case PRODUCT_STANDARD_SERVER_CORE:
				case PRODUCT_WEB_SERVER:
					version_type_ = SUITE_SERVER;
					break;
				case PRODUCT_PROFESSIONAL:
				case PRODUCT_ULTIMATE:
					version_type_ = SUITE_PROFESSIONAL;
					break;
				case PRODUCT_ENTERPRISE:
				case PRODUCT_ENTERPRISE_E:
				case PRODUCT_ENTERPRISE_EVALUATION:
				case PRODUCT_ENTERPRISE_N:
				case PRODUCT_ENTERPRISE_N_EVALUATION:
				case PRODUCT_ENTERPRISE_S:
				case PRODUCT_ENTERPRISE_S_EVALUATION:
				case PRODUCT_ENTERPRISE_S_N:
				case PRODUCT_ENTERPRISE_S_N_EVALUATION:
				case PRODUCT_BUSINESS:
				case PRODUCT_BUSINESS_N:
					version_type_ = SUITE_ENTERPRISE;
					break;
				case PRODUCT_EDUCATION:
				case PRODUCT_EDUCATION_N:
					version_type_ = SUITE_EDUCATION;
					break;
				case PRODUCT_HOME_BASIC:
				case PRODUCT_HOME_PREMIUM:
				case PRODUCT_STARTER:
				default:
					version_type_ = SUITE_HOME;
					break;
				}
			}
			else if (version_info.dwMajorVersion == 5 &&
				version_info.dwMinorVersion == 2) {
				if (version_info.wProductType == VER_NT_WORKSTATION &&
					system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
					version_type_ = SUITE_PROFESSIONAL;
				}
				else if (version_info.wSuiteMask & VER_SUITE_WH_SERVER) {
					version_type_ = SUITE_HOME;
				}
				else {
					version_type_ = SUITE_SERVER;
				}
			}
			else if (version_info.dwMajorVersion == 5 &&
				version_info.dwMinorVersion == 1) {
				if (version_info.wSuiteMask & VER_SUITE_PERSONAL)
					version_type_ = SUITE_HOME;
				else
					version_type_ = SUITE_PROFESSIONAL;
			}
			else {
				// Windows is pre XP so we don't care but pick a safe default.
				version_type_ = SUITE_HOME;
			}
		}
		~OSInfo();

		// Returns a Version value for a given OS version tuple.
		static OSVersion MajorMinorBuildToVersion(int major, int minor, int build);

		OSVersion version_;
		VersionNumber version_number_;
		VersionType version_type_;
		ServicePack service_pack_;

		// Represents the version of the OS associated to a release of
		// Windows 10. Each version may have different releases (such as patch
		// updates). This is the identifier of the release.
		// Example:
		//    Windows 10 Version 1809 (OS build 17763) has multiple releases
		//    (i.e. build 17763.1, build 17763.195, build 17763.379, ...).
		// See https://docs.microsoft.com/en-us/windows/windows-10/release-information
		// for more information.
		std::string release_id_;

		// A string, such as "Service Pack 3", that indicates the latest Service Pack
		// installed on the system. If no Service Pack has been installed, the string
		// is empty.
		std::string service_pack_str_;
		int processors_;
		size_t allocation_granularity_;
		WOW64Status wow64_status_;
		std::string processor_model_name_;

		DISALLOW_COPY_AND_ASSIGN(OSInfo);
	};

	OSVersion GetOSVersion()
	{
		return OSInfo::GetInstance()->version();
	}
}
