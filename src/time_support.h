/* 
 * File:   time_support.h
 * Author: massimo
 *
 * Created on May 16, 2017, 2:12 PM
 */
#pragma once

#include <iostream>
#include <chrono>
#include <unordered_map>
#include <functional>
////////////////////////////////////////////////////////////////////////////////
#ifndef CHRONO_TIME
#define CHRONO_TIME
#endif
////////////////////////////////////////////////////////////////////////////////
namespace timeSupport
{
inline
uint_fast64_t
rdtscp() noexcept
{
  volatile uint_fast32_t tickl {};
  volatile uint_fast32_t tickh {};

  __asm__ __volatile__("rdtscp" : "=a"(tickl), "=d"(tickh) ::);

  return ((static_cast<uint_fast64_t>(tickh) << 32) | tickl);
}

inline
void
rdtscp(uint_fast64_t& r) noexcept
{
  volatile uint_fast32_t tickl {};
  volatile uint_fast32_t tickh {};

  __asm__ __volatile__("rdtscp" : "=a"(tickl), "=d"(tickh) ::);

  r = (static_cast<uint_fast64_t>(tickh) << 32) | tickl;
}
////////////////////////////////////////////////////////////////////////////////
class rdtscTimer final
{
  using mapKey = unsigned int;

 public:
  enum class rdtscTimerStatus { INACTIVE, STARTED, STOPPED, REPORTED };

  explicit rdtscTimer(const std::string& timerName = "rdtscTimer",
                      std::ostream& log = std::cout) noexcept;

  ~rdtscTimer() noexcept;

  constexpr
  rdtscTimer&
  start(const std::string& startPoint = "") noexcept
  {
    auto&& s = getTimerStatus();

    if ( (rdtscTimerStatus::INACTIVE == s) ||
         (rdtscTimerStatus::REPORTED == s) ||
         (rdtscTimerStatus::STOPPED == s)
       )
    {
      setTimerStatus(rdtscTimerStatus::STARTED);
      if ( "" != startPoint )
      {
        m_startPointLabel = startPoint;
      }
#ifdef CHRONO_TIME
      m_tstart = std::chrono::high_resolution_clock::now();
#endif
      m_start = rdtscp();
      return *this;
    }
    
    m_log << m_timerName << ": "
          << startPoint
          << ": ERROR: start() called but timer is already started"
          << '\n';

    return *this;
  }

  constexpr
  rdtscTimer&
  stop(const std::string& stopPoint) noexcept
  {
    if ( rdtscTimerStatus::STARTED == getTimerStatus() )
    {
      m_stop = rdtscp();
#ifdef CHRONO_TIME
      m_tstop = std::chrono::high_resolution_clock::now();
#endif
      setTimerStatus(rdtscTimerStatus::STOPPED);
      m_stopPointLabel = std::move(stopPoint);
      return *this;
    }
    
    m_log << m_timerName << ": "
          << stopPoint
          << ": ERROR: stop() called but timer is not started"
          << '\n';

    return *this;
  }

  void
  stopAndReport(const std::string& stopPoint) noexcept
  {
    stop(stopPoint).report();
  }

  rdtscTimer& report() noexcept;

  constexpr
  uint_fast64_t
  getStartTSC() const noexcept
  {
    return m_start;
  }

  constexpr
  uint_fast64_t
  getStopTSC() const noexcept
  {
    return m_stop;
  }

  constexpr
  uint_fast64_t
  getLapsedTSC() const noexcept
  {
    if ( rdtscTimerStatus::STARTED == getTimerStatus() )
    {
      return (rdtscp() - m_start);
    }
    return 0;
  }

  constexpr
  uint_fast64_t
  getStopLapsedTSC() const noexcept
  {
    auto&& s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return (m_stop - m_start);
    }
    return 0;
  }

#ifdef CHRONO_TIME
  constexpr
  double
  getStopLapsed_sec() const noexcept
  {
    auto&& s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return std::chrono::duration_cast<std::chrono::duration<double>>(m_tstop - m_tstart).count();
    }
    return 0;
  }
#endif

#ifdef CHRONO_TIME
  constexpr
  uint_fast64_t
  getStopLapsed_msec() const noexcept
  {
    auto&& s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return static_cast<uint_fast64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(m_tstop - m_tstart).count());
    }
    return 0;
  }
#endif

#ifdef CHRONO_TIME
  constexpr
  uint_fast64_t
  getStopLapsed_usec() const noexcept
  {
    auto&& s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return static_cast<uint_fast64_t>(std::chrono::duration_cast<std::chrono::microseconds>(m_tstop - m_tstart).count());
    }
    return 0;
  }
#endif

#ifdef CHRONO_TIME
  constexpr
  uint_fast64_t
  getStopLapsed_nsec() const noexcept
  {
    auto&& s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return static_cast<uint_fast64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(m_tstop - m_tstart).count());
    }
    return 0;
  }
#endif

  const std::string&
  getTimerStatusString() const noexcept
  {
    return timerStatusStringMap[static_cast<mapKey>(getTimerStatus())];
  }

  constexpr
  rdtscTimerStatus
  getTimerStatus() const noexcept
  {
    return m_rdtscTimerStatus;
  }

  void
  operator()() const noexcept
  {
    m_log << "\n<------------------------------------------------------------------->\n"
          << *this
          << "\n<------------------------------------------------------------------->\n";
  }

  friend std::ostream& operator<<(std::ostream& os, const rdtscTimer& obj);
//  friend std::istream& operator>>(std::istream& is, rdtscTimer& obj);

 private:
  static std::unordered_map<mapKey, std::string> timerStatusStringMap;
  static const std::string m_startPointLabelDefault;
  static const std::string m_stopPointLabelDefault;
  const std::string m_timerName{};
  mutable std::string m_startPointLabel{};
  mutable std::string m_stopPointLabel{};
  mutable rdtscTimerStatus m_rdtscTimerStatus{rdtscTimerStatus::INACTIVE};
  uint_fast64_t m_start{};
  uint_fast64_t m_stop{};
#ifdef CHRONO_TIME
  std::chrono::high_resolution_clock::time_point m_tstart{};
  std::chrono::high_resolution_clock::time_point m_tstop{};
#endif
  std::ostream& m_log{std::cout};

  void
  setTimerStatus (const rdtscTimerStatus& s) const noexcept
  {
    m_rdtscTimerStatus = s;
  }
};  // class rdtscTimer

// generic lambda (C++14 onwards)
decltype(auto)
profileFunction = [] (rdtscTimer& rdtsct,
                      const std::string&& startPoint,
                      const std::string&& stopPoint,
                      auto&& func, auto&&... params) noexcept(false) -> void // C++14's universal references aka forwarding references
{
  // start timer
  rdtsct.start(startPoint);

  // invoke function to profile on forwarded params
#ifdef CALL_STD_FORWARD // define this macro in this case (used in the unit tests)
  // std::forward() until C++17
  std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
#else
  // std::invoke() from C++17
  std::invoke(std::forward<decltype(func)>(func), std::forward<decltype(params)>(params)...);
#endif
  // stop timer and report elapsed time
  rdtsct.stop(stopPoint).report();
};
////////////////////////////////////////////////////////////////////////////////
}  // namespace timeSupport
