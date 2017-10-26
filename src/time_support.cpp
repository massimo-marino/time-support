/* 
 * File:   time_support.cpp
 * Author: massimo
 *
 * Created on May 15, 2017, 12:00 PM
 */
// BEGIN: ignore the warnings listed below when compiled with clang from here
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
////////////////////////////////////////////////////////////////////////////////
#include "time_support.h"
#include <iomanip>
////////////////////////////////////////////////////////////////////////////////
namespace timeSupport
{
const std::string rdtscTimer::m_startPointLabelDefault{"-CTOR-START"};
const std::string rdtscTimer::m_stopPointLabelDefault{"-DTOR-STOP"};

std::map<rdtscTimer::rdtscTimerStatus,std::string> rdtscTimer::timerStatusStringMap{
    {rdtscTimer::rdtscTimerStatus::INACTIVE, "INACTIVE"},
    {rdtscTimer::rdtscTimerStatus::STARTED,  "STARTED"},
    {rdtscTimer::rdtscTimerStatus::STOPPED,  "STOPPED"},
    {rdtscTimer::rdtscTimerStatus::REPORTED, "REPORTED"}
  };

rdtscTimer::rdtscTimer(const std::string& timerName,
                       std::ostream& log)
:
m_timerName{timerName},
m_startPointLabel(m_timerName + m_startPointLabelDefault),
m_stopPointLabel(m_timerName + m_stopPointLabelDefault),
m_rdtscTimerStatus(rdtscTimerStatus::INACTIVE),
m_log(log)
{}

rdtscTimer::~rdtscTimer()
{
  // take the stop time and store it in temporary vars, in case it is needed later
  uint_fast64_t&& stop = rdtscp();
#ifdef CHRONO_TIME
  std::chrono::high_resolution_clock::time_point&& tstop = std::chrono::high_resolution_clock::now();
#endif
  auto&& s = getTimerStatus();

  // if inactive then leave
  if ( rdtscTimerStatus::INACTIVE == s )
  {
    return;
  }
  // if started then store the stop temporary values in the class attributes
  if ( rdtscTimerStatus::STARTED == s )
  {
    m_stop  = stop;
#ifdef CHRONO_TIME
    m_tstop = tstop;
#endif
    s = rdtscTimerStatus::STOPPED;
    setTimerStatus(s);
  }
  if ( rdtscTimerStatus::STOPPED == s )
  {
    // if the timer was stopped we have to report the stop before destruction
    report();
  }
}

rdtscTimer& rdtscTimer::report() noexcept
{
  if ( rdtscTimerStatus::STOPPED == getTimerStatus() )
  {
    m_log << m_timerName << ": " << m_startPointLabel << " -> " << m_stopPointLabel
          << ": Timer started at "
          << m_start
          <<  " and stopped at "
          << m_stop
          << " taking "
          << m_stop - m_start
          << " ticks"
#ifdef CHRONO_TIME
          << " [ "
          <<  std::setprecision(16)
          << getStopLapsed_sec()
          << " sec = "
          << getStopLapsed_nsec()
          << std::setprecision(5)
          << " nsec ]"
#endif
          << '\n';

    setTimerStatus(rdtscTimerStatus::REPORTED);

    return *this;
  }

  m_log << m_timerName << ": "
        << ": ERROR: report() called but timer is not stopped"
        << '\n';

  return *this;
}

// extraction operator for class rdtscTimer
std::ostream& operator<<(std::ostream& os, const rdtscTimer& obj)
{
  // write obj to stream
  os << "> Timer "
     << obj.m_timerName
     << " is "
     << static_cast<int>(obj.getTimerStatus())
     << "/"
     << obj.getTimerStatusString()
     << '\n'
     << "> From: "
     << obj.m_startPointLabel
     << '\n'
     << "> To:   "
     << obj.m_stopPointLabel
     << '\n'
     << "> Start Tick: "
     << obj.m_start
     << '\n'
     << "> Stop Tick:  "
     << obj.m_stop
     << '\n'
#ifdef CHRONO_TIME
     << "> Start Time Point: "
     << obj.m_tstart.time_since_epoch().count()
     << '\n'
     << "> Stop Time Point:  "
     << obj.m_tstop.time_since_epoch().count()
#endif
     ;

  return os;
}

// insertion operator for class rdtscTimer [not implemented]
//std::istream& operator>>(std::istream& is, rdtscTimer& obj)
//{
//  // read obj from stream
//  if ( /* rdtscTimer could not be constructed */ )
//    is.setstate(std::ios::failbit);
//  return is;
//}
}  // namespace timeSupport
////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
