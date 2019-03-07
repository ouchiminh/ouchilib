#pragma once
#include <optional>
#include <utility>
#include <chrono>

namespace ouchi {
	/// <summary>
	/// Clock must have static `now` method that returns current time point.
	/// </summary>
	template<class Clock = std::chrono::high_resolution_clock, class TimePoint = decltype(Clock::now())>
	class fps_counter{
		unsigned long long frame_cnt_ = 0;
		std::optional<TimePoint> beg_, end_;
	public:
		/// <summary>
		/// call more than 2 times. calculate avg framerate
		/// </summary>
		void on_frame() noexcept {
			if (!beg_)  beg_ = Clock::now();
			else end_ = Clock::now();
			frame_cnt_++;
		}
		/// <summary>
		/// calc avg framerate. if frame count is less than 2, returns 0.
		/// </summary>
		/// <returns>framerate</returns>
		auto get_framerate() const noexcept->double {
			if (frame_cnt_ < 2) return 0;
			// f/sec
			return frame_cnt_ / (double)std::chrono::duration_cast<std::chrono::milliseconds>(end_.value() - beg_.value()).count() * 1000;
		}
        auto get_avg_duration() const noexcept
            -> decltype(std::declval<TimePoint>() - std::declval<TimePoint>())
        {
            return beg_ && end_
                ? (beg_.value() - end_.value()) / frame_cnt_
                : 0;
        }
		void reset() noexcept {
			frame_cnt_ = 0;
			beg_.reset(); end_.reset();
		}

		/// <summary>
		/// measure time while drawing and returns framerate.
		/// </summary>
		/// <param name="f">drow function</param>
		/// <param name="...args">draw func's argument</param>
		/// <returns>framerate</returns>
		template<class DrawFunc, class ...Args>
		double draw_and_measure(DrawFunc && f, Args && ...args) const noexcept(noexcept(std::declval<DrawFunc>()(std::declval<Args>()...))) {
			fps_counter<Clock, TimePoint> cnter;
			cnter.on_frame();
			f(args...);
			cnter.on_frame();
			return cnter.get_framerate();
		}
	};
}
