#include "perf_watcher.h"

bool PerfWatcher::Start(std::shared_ptr<PerfOption> option, FPerfWatcherReply reply) {
	if (!option || !reply) return false;
	std::atomic_exchange(&m_quit, false);
	m_perfChecker.reset(new PerfChecker(option->GetTarget()));

	// ����һ�����̣߳��������ݵĲ����ռ�
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
		// ����������õ�ʱ������������
		// ���������Ҫ��ȥ�������ĵ�ʱ��
		DWORD dwTick = GetTickCount();
		// ����ذ����������Ƿ񿨶١����ٷ�ֵ��ƽ������ʱ��
		bool bCaton = m_perfChecker->IsWinCaton(option->GetDelayTime());
		bool bCatonDead = m_perfChecker->IsWinHung();
		reply(option->CaclPerfValue(bCaton, bCatonDead));
		DWORD dwCaton = GetTickCount() - dwTick;
		int sleepGap = option->GetSampleGap() - dwCaton;
		if (sleepGap > 0)
			Sleep(sleepGap);
	}
}
