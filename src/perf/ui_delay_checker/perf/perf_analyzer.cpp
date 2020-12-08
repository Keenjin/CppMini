#include "perf_analyzer.h"

HWND PerfAnalyzer::GetTarget() {
	return m_hWnd;
}

// 一次采样
std::shared_ptr<PerfValue> PerfAnalyzer::CaclPerfValue(bool caton, bool hung) {

	if (caton) {
		m_dwCurrentDelay += m_dwDelayTime;
		m_dwTotalCaton += m_dwCurrentDelay;
		if (m_dwMaxCaton < m_dwCurrentDelay) m_dwMaxCaton = m_dwCurrentDelay;

		int len = m_vecDelayRecord.size() + 1;
		m_dwAverageCaton = m_dwTotalCaton * 1.0f / (len * 1.0f);
	}
	else {
		if (m_bLastCaton) {
			m_vecDelayRecord.push_back(PerfRecord{ _time64(nullptr), m_dwCurrentDelay});
			m_dwCurrentDelay = 0;
		}
	}

	m_bLastCaton = caton;

	return std::make_shared<PerfValue>(_time64(nullptr), caton, m_dwAverageCaton, m_dwMaxCaton, m_dwDelayTime, hung);
}