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

rdtscTimer::rdtscTimer(const std::string& timerName,
                       std::ostream& log)
:
m_timerName{timerName},
m_startPointLabel(m_timerName + m_startPointLabelDefault),
m_stopPointLabel(m_timerName + m_stopPointLabelDefault),
m_rdtscTimerStatus(rdtscTimerStatus::INACTIVE),
m_log(log),
m_start(0),
m_tstart(std::chrono::high_resolution_clock::now()),
m_stop(0),
m_tstop(std::chrono::high_resolution_clock::now())
{}

rdtscTimer::~rdtscTimer()
{
  // take the stop time and store it in temporary vars, in case it is needed
  uint_fast64_t stop = rdtsc();
  std::chrono::high_resolution_clock::time_point tstop = std::chrono::high_resolution_clock::now();
  // if inactive then leave
  if ( rdtscTimerStatus::INACTIVE == getRDTSCTimerStatus() )
  {
    return;
  }
  // if started then store the stop temporary values in the class attributes
  if ( rdtscTimerStatus::STARTED == getRDTSCTimerStatus() )
  {
    m_stop  = stop;
    m_tstop = tstop;
    setRDTSCTimerStatus(rdtscTimerStatus::STOPPED);
  }
  // we are here when the timer was stopped and we have to report the stop
  // before destruction
  report();
}

void rdtscTimer::report() noexcept
{
  m_log << m_timerName << ": " << m_startPointLabel << " -> " << m_stopPointLabel
        << ": Timer started at "
        << m_start
        <<  " and stopped at "
        << m_stop
        << " taking "
        << m_stop - m_start
        << " ticks - "
        << getStopLapsed_sec()
        << " sec ["
        << getStopLapsed_nsec()
        << " nsec]"
        << std::endl;
}
}  // namespace timeSupport
