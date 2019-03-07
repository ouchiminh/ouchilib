#pragma once
#include <thread>
#include <chrono>
#include <type_traits>

namespace ouchi{
template<class Clock = std::chrono::steady_clock>
class time_keeper {

	using TimePoint = decltype(Clock::now());

	typename TimePoint m_lastCalled;
	typename Clock::duration m_duration;

	typename Clock::duration remaining_time() const {
		auto difference = (Clock::now() - m_lastCalled);
		return m_duration - difference;
	}
public:
	template<class Duration>
	time_keeper(Duration d = std::chrono::microseconds(1'000'000) / 60){
		m_duration = std::chrono::duration_cast<Clock::duration>(d);
	}
	~time_keeper() = default;

	void start() {
		m_lastCalled = Clock::now();
	}
	bool is_time() const {
		return remaining_time().count() <= 0;
	}
	void sleep(){
		auto remaining = remaining_time();
		if(remaining.count() > 0) std::this_thread::sleep_for(remaining);
		start();
	}
	template <typename Clock2, typename Period>
	void set_duration(std::chrono::duration<Clock2, Period> d) {
		m_duration = std::chrono::duration_cast<decltype(m_duration)>(d);
	}
};
} // namespace ouchi
