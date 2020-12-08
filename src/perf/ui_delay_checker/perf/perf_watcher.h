#pragma once
#include <memory>
#include <functional>
#include <thread>
#include <atomic>

#include "perf_checker.h"

class PerfValue {
public:
	PerfValue(__time64_t time_, bool caton_, float avCaton_, DWORD maxCaton_, DWORD delay_, bool catonDead_)
		: timeNow(time_)
		, bCaton(caton_)
		, dwAverageCatonDelay(avCaton_)
		, dwMaxCatonDelay(maxCaton_)
		, dwDelayTime(delay_)
		, bCatonDead(catonDead_) {

	}
	virtual ~PerfValue() = default;

	__time64_t timeNow = 0;
	bool bCaton = false;
	float dwAverageCatonDelay = 0.0f;		// 单位：ms
	DWORD dwMaxCatonDelay = 0;			// 单位：ms
	DWORD dwDelayTime = 0;
	bool bCatonDead = false;
};

using FPerfWatcherReply = std::function<void(std::shared_ptr<PerfValue>)>;

class PerfWatcher {
public:
	class PerfOption {
	public:
		virtual ~PerfOption() = default;
		virtual HWND GetTarget() = 0;
		virtual DWORD GetSampleGap() { return 1000; }		// 默认1s采样间隔
		virtual DWORD GetDelayTime() { return 100; }		// 默认100ms就表示存在卡顿
		virtual std::shared_ptr<PerfValue> CaclPerfValue(bool caton, bool hung) = 0;
	};

	bool Start(std::shared_ptr<PerfOption> option, FPerfWatcherReply reply);
	void Stop();

private:
	void ThreadMain(std::shared_ptr<PerfWatcher::PerfOption> option, FPerfWatcherReply reply);

	std::unique_ptr<std::thread> m_watchThread;
	std::atomic_bool m_quit = false;
	std::unique_ptr<PerfChecker> m_perfChecker;
};