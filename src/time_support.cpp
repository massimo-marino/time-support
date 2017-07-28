/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   time_support.cpp
 * Author: massimo
 *
 * Created on May 15, 2017, 12:00 PM
 */

#include "time_support.h"
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
  std::chrono::high_resolution_clock::time_point&& tstop = std::chrono::high_resolution_clock::now();
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
    m_tstop = tstop;
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
          << " ticks [ "
          << getStopLapsed_sec()
          << " sec = "
          << getStopLapsed_nsec()
          << " nsec ]"
          << std::endl;

    setTimerStatus(rdtscTimerStatus::REPORTED);

    return *this;
  }

  m_log << m_timerName << ": "
        << ": ERROR: report() called but timer is not stopped"
        << std::endl;

  return *this;
}

// extraction operator for class rdtscTimer
std::ostream& operator<<(std::ostream& os, const rdtscTimer& obj)
{
  // write obj to stream
  os << "> Timer "
     << obj.m_timerName
     << " is "
     << (int)obj.getTimerStatus()
     << "/"
     << obj.getTimerStatusString()
     << std::endl
     << "> From: "
     << obj.m_startPointLabel
     << std::endl
     << "> To:   "
     << obj.m_stopPointLabel
     << std::endl
     << "> Start Tick: "
     << obj.m_start
     << std::endl
     << "> Stop Tick:  "
     << obj.m_stop
     << std::endl
     << "> Start Time Point: "
     << obj.m_tstart.time_since_epoch().count()
     << std::endl
     << "> Stop Time Point:  "
     << obj.m_tstop.time_since_epoch().count()
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
