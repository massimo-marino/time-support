/* 
 * File:   time_support.h
 * Author: massimo
 *
 * Created on May 16, 2017, 2:12 PM
 */

#ifndef TIME_SUPPORT_H
#define TIME_SUPPORT_H

#include <iostream>
#include <chrono>
#include <map>
////////////////////////////////////////////////////////////////////////////////
#ifndef CHRONO_TIME
#define CHRONO_TIME
#endif
////////////////////////////////////////////////////////////////////////////////
namespace timeSupport
{
static inline
uint_fast64_t rdtscp() noexcept
{
  uint_fast32_t tickl {};
  uint_fast32_t tickh {};

  __asm__ __volatile__("rdtscp":"=a"(tickl), "=d"(tickh) ::);

  return ((static_cast<uint_fast64_t>(tickh) << 32) | tickl);
}

static inline
void rdtscp(uint_fast64_t& r) noexcept
{
  uint_fast32_t tickl {};
  uint_fast32_t tickh {};

  __asm__ __volatile__("rdtscp":"=a"(tickl), "=d"(tickh) ::);

  r = (static_cast<uint_fast64_t>(tickh) << 32) | tickl;
}
////////////////////////////////////////////////////////////////////////////////
class rdtscTimer final
{
 public:
  enum class rdtscTimerStatus { INACTIVE, STARTED, STOPPED, REPORTED };

  explicit rdtscTimer(const std::string& timerName = "rdtscTimer",
                      std::ostream& log = std::cout);

  ~rdtscTimer();

  inline rdtscTimer& start(const std::string& startPoint = "") noexcept
  {
    auto&& s = getTimerStatus();

    if ( (rdtscTimerStatus::INACTIVE == s) ||
         (rdtscTimerStatus::REPORTED == s) ||
         (rdtscTimerStatus::STOPPED == s)
       )
    {
      setTimerStatus(rdtscTimerStatus::STARTED);
      if ( "" == startPoint )
      {
        m_startPointLabel = m_timerName + m_startPointLabelDefault;
      }
      else
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

  inline rdtscTimer& stop(const std::string& stopPoint) noexcept
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

  inline void stopAndReport(const std::string& stopPoint) noexcept
  {
    stop(stopPoint).report();
  }

  rdtscTimer& report() noexcept;

  inline uint_fast64_t getStartTSC() const noexcept
  {
    return m_start;
  }

  inline uint_fast64_t getStopTSC() const noexcept
  {
    return m_stop;
  }

  inline uint_fast64_t getLapsedTSC() const noexcept
  {
    if ( rdtscTimerStatus::STARTED == getTimerStatus() )
    {
      return (rdtscp() - m_start);
    }
    return 0;
  }

  inline uint_fast64_t getStopLapsedTSC() const noexcept
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
  inline double getStopLapsed_sec() const noexcept
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
  inline uint_fast64_t getStopLapsed_msec() const noexcept
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
  inline uint_fast64_t getStopLapsed_usec() const noexcept
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
  inline uint_fast64_t getStopLapsed_nsec() const noexcept
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

  inline const std::string getTimerStatusString() const noexcept
  {
    return timerStatusStringMap[getTimerStatus()];
  }

  inline rdtscTimerStatus getTimerStatus() const noexcept
  {
    return m_rdtscTimerStatus;
  }

  friend std::ostream& operator<<(std::ostream& os, const rdtscTimer& obj);
//  friend std::istream& operator>>(std::istream& is, rdtscTimer& obj);

 private:
  static std::map<rdtscTimerStatus,std::string> timerStatusStringMap;
  static const std::string m_startPointLabelDefault;
  static const std::string m_stopPointLabelDefault;
  const std::string m_timerName{};
  mutable std::string m_startPointLabel{};
  mutable std::string m_stopPointLabel{};
  rdtscTimerStatus m_rdtscTimerStatus{rdtscTimerStatus::INACTIVE};
  [[maybe_unused]] char padding[4] {0,0,0,0};
  uint_fast64_t m_start{};
  uint_fast64_t m_stop{};
#ifdef CHRONO_TIME
  std::chrono::high_resolution_clock::time_point m_tstart{};
  std::chrono::high_resolution_clock::time_point m_tstop{};
#endif
  std::ostream& m_log{std::cout};

  inline void setTimerStatus (const rdtscTimerStatus& s) noexcept
  {
    m_rdtscTimerStatus = s;
  }
};  // class rdtscTimer

// generic lambda (C++14 onwards)
decltype(auto) profileFunction =
  [](rdtscTimer& rdtsct,
     const std::string&& startPoint,
     const std::string&& stopPoint,
     auto&& func, auto&&... params) // C++14's universal references aka forwarding references
  {
    // start timer
    rdtsct.start(startPoint);

    // invoke function to profile on forwarded params
    // C++17: not yet available
    //std::invoke(std::forward<decltype(func)>(func), std::forward<decltype(params)>(params)...);
    std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);

    // stop timer and report elapsed time
    rdtsct.stop(stopPoint).report();
  };
////////////////////////////////////////////////////////////////////////////////
}  // namespace timeSupport
#endif /* TIME_SUPPORT_H */
