#include "perf_watcher.h"

bool PerfWatcher::Start(std::shared_ptr<PerfOption> option, FPerfWatcherReply reply) {
	if (!option || !reply) return false;
	std::atomic_exchange(&m_quit, false);
	m_perfChecker.reset(new PerfChecker(option->GetTarget()));

	// 启动一个新线程，负责数据的采样收集
	if (!m_watchThread) {
		m_watchThread.reset(new std::thread(&PerfWatcher::ThreadMain, this, option, reply));
	}
	return true;
}

void PerfWatcher::Stop() {
	if (m_watchThread) {
		std::atomic_exchange(&m_quit, true);
		m_watchThread->join();
		m_watchThread.reset();
	}
}

void PerfWatcher::ThreadMain(std::shared_ptr<PerfWatcher::PerfOption> option, FPerfWatcherReply reply) {
	while (!m_quit) {
		// 根据外层设置的时间间隔，来采样
		// 采样间隔需要减去卡顿消耗的时间
		DWORD dwTick = GetTickCount();
		// 这里回包，包括：是否卡顿、卡顿峰值、平均卡顿时长
		bool bCaton = m_perfChecker->IsWinCaton(option->GetDelayTime());
		bool bCatonDead = m_perfChecker->IsWinHung();
		reply(option->CaclPerfValue(bCaton, bCatonDead));
		DWORD dwCaton = GetTickCount() - dwTick;
		int sleepGap = option->GetSampleGap() - dwCaton;
		if (sleepGap > 0)
			Sleep(sleepGap);
	}
}
