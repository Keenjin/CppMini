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
	float dwAverageCatonDelay = 0.0f;		// ��λ��ms
	DWORD dwMaxCatonDelay = 0;			// ��λ��ms
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
		virtual DWORD GetSampleGap() { return 1000; }		// Ĭ��1s�������
		virtual DWORD GetDelayTime() { return 100; }		// Ĭ��100ms�ͱ�ʾ���ڿ���
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