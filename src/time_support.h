/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

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
namespace timeSupport
{
inline uint_fast64_t rdtsc() noexcept
{
  uint_fast32_t tickl {};
  uint_fast32_t tickh {};

  __asm__ __volatile__("rdtsc":"=a"(tickl), "=d"(tickh));

  return ((static_cast<uint_fast64_t>(tickh) << 32) | tickl);
}

inline void rdtsc(uint_fast64_t& r) noexcept
{
  uint_fast32_t tickl {};
  uint_fast32_t tickh {};

  __asm__ __volatile__("rdtsc":"=a"(tickl), "=d"(tickh));

  r = (static_cast<uint_fast64_t>(tickh) << 32) | tickl;
}
////////////////////////////////////////////////////////////////////////////////
class rdtscTimer
{
 public:
  enum class rdtscTimerStatus { INACTIVE, STARTED, STOPPED, REPORTED };

  explicit rdtscTimer(const std::string& timerName = "rdtscTimer",
                      std::ostream& log = std::cout);

  ~rdtscTimer();

  inline rdtscTimer& start(const std::string& startPoint = "") noexcept
  {
    auto s = getTimerStatus();

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
      m_tstart = std::chrono::high_resolution_clock::now(),
      m_start = rdtsc();
      return *this;
    }
    
    m_log << m_timerName << ": "
          << startPoint
          << ": ERROR: start() called but timer is already started"
          << std::endl;

    return *this;
  }

  inline rdtscTimer& stop(const std::string& stopPoint) noexcept
  {
    if ( rdtscTimerStatus::STARTED == getTimerStatus() )
    {
      m_stop = rdtsc(),
      m_tstop = std::chrono::high_resolution_clock::now();
      setTimerStatus(rdtscTimerStatus::STOPPED);
      m_stopPointLabel = std::move(stopPoint);
      return *this;
    }
    
    m_log << m_timerName << ": "
          << stopPoint
          << ": ERROR: stop() called but timer is not started"
          << std::endl;

    return *this;
  }

  inline void stopAndReport(const std::string& stopPoint) noexcept
  {
    stop(stopPoint).report();
  }

  rdtscTimer& report() noexcept;

  inline uint_fast64_t getStartTSC() noexcept
  {
    return m_start;
  }

  inline uint_fast64_t getStopTSC() noexcept
  {
    return m_stop;
  }

  inline uint_fast64_t getLapsedTSC() noexcept
  {
    if ( rdtscTimerStatus::STARTED == getTimerStatus() )
    {
      return (rdtsc() - m_start);
    }
    return 0;
  }

  inline uint_fast64_t getStopLapsedTSC() noexcept
  {
    auto s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return (m_stop - m_start);
    }
    return 0;
  }

  inline double getStopLapsed_sec() noexcept
  {
    auto s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return std::chrono::duration_cast<std::chrono::duration<double>>(m_tstop - m_tstart).count();
    }
    return 0;
  }

  inline uint_fast64_t getStopLapsed_msec() noexcept
  {
    auto s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return (uint_fast64_t)(std::chrono::duration_cast<std::chrono::milliseconds>(m_tstop - m_tstart).count());
    }
    return 0;
  }

  inline uint_fast64_t getStopLapsed_usec() noexcept
  {
    auto s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return (uint_fast64_t)(std::chrono::duration_cast<std::chrono::microseconds>(m_tstop - m_tstart).count());
    }
    return 0;
  }

  inline uint_fast64_t getStopLapsed_nsec() noexcept
  {
    auto s = getTimerStatus();

    if ( (rdtscTimerStatus::STOPPED == s) ||
         (rdtscTimerStatus::REPORTED == s) )
    {
      return (uint_fast64_t)(std::chrono::duration_cast<std::chrono::nanoseconds>(m_tstop - m_tstart).count());
    }
    return 0;
  }

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
  uint_fast64_t m_start{};
  std::chrono::high_resolution_clock::time_point m_tstart{};
  uint_fast64_t m_stop{};
  std::chrono::high_resolution_clock::time_point m_tstop{};
  std::ostream& m_log{std::cout};

  inline void setTimerStatus (const rdtscTimerStatus& s) noexcept
  {
    m_rdtscTimerStatus = s;
  }
};  // class rdtscTimer
////////////////////////////////////////////////////////////////////////////////
}  // namespace timeSupport
#endif /* TIME_SUPPORT_H */
