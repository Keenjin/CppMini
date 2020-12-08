#pragma once
#include "perf_watcher.h"
#include <vector>

class PerfAnalyzer : public PerfWatcher::PerfOption {
public:
	class PerfRecord {
	public:
		__time64_t time = 0;
		DWORD delay = 0;
	};

	PerfAnalyzer(HWND hWnd, DWORD sampleGap, DWORD delayTime) 
		: m_hWnd(hWnd)
		, m_dwSampleGap(sampleGap)
		, m_dwDelayTime(delayTime) {
	}

	// PerfWatcher::PerfOption
	virtual HWND GetTarget() override;
	virtual DWORD GetSampleGap() override { return m_dwSampleGap; }
	virtual DWORD GetDelayTime() override { return m_dwDelayTime; }
	virtual std::shared_ptr<PerfValue> CaclPerfValue(bool caton, bool hung) override;

private:
	HWND m_hWnd = nullptr;
	DWORD m_dwSampleGap = 1000;
	DWORD m_dwDelayTime = 100;

	// 保存原始值
	std::vector<PerfRecord> m_vecDelayRecord;		// 只记录历史每一次的delay时间

	bool m_bLastCaton = false;
	DWORD m_dwCurrentDelay = 0;
	DWORD m_dwMaxCaton = 0;
	DWORD m_dwTotalCaton = 0;
	float m_dwAverageCaton = 0.0f;
};