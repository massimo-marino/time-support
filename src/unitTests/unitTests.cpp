//
//  unitTests.cpp
//
#include "../time_support.h"

#include <typeinfo>
#include <sys/resource.h>
#include <vector>
#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

using tspec = struct timespec;
// BEGIN: ignore the warnings listed below when compiled with clang from here
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
////////////////////////////////////////////////////////////////////////////////
[[maybe_unused]]
static inline
void
nanoSleep(const time_t& seconds = 0,
          const long& nanoseconds = 0) noexcept
{
  tspec&& req = { seconds, nanoseconds };
  tspec&& rem {};
  int ret {};

  RETRY:
  ret = nanosleep (&req, &rem);
  if ( 0 == ret )
  {
    return;
  }
  if ( errno == EINTR )
  {
    /* retry, with the provided time remaining */
    req.tv_sec = rem.tv_sec;
    req.tv_nsec = rem.tv_nsec;
    goto RETRY;
  }
  std::cerr << "ERROR: nanosleep()"
            << '\n';
}

static inline
void
absnanosleep(const time_t& seconds = 0,
             const long& nanoseconds = 0) noexcept
{
  static const clockid_t&& clock{CLOCK_REALTIME};
  tspec&& ts {};

  clock_gettime(clock, &ts);

  ts.tv_sec = ts.tv_sec + seconds;
  ts.tv_nsec = ts.tv_nsec + nanoseconds;
  clock_nanosleep(clock, TIMER_ABSTIME, &ts, NULL);
}

////////////////////////////////////////////////////////////////////////////////
TEST(timeSupport, TimeSourceResolution)
{
  const std::vector<std::pair<std::string,clockid_t>> clockv {
    {"CLOCK_REALTIME", CLOCK_REALTIME},
    {"CLOCK_MONOTONIC", CLOCK_MONOTONIC},
    {"CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID},
    {"CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID},
    {"CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW}
  };

  for (auto&& p : clockv)
  {
    tspec&& res {};
    int ret {};

    ret = clock_getres(p.second, &res);
    if ( ret )
    {
      std::cerr << "ERROR: clock_getres() error"
                << '\n';
    }
    else
    {
      std::cout << p.first
                << " Time resolution: sec = "
                << res.tv_sec
                << " nsec = "
                << res.tv_nsec
                << '\n';
    }
  }
}

TEST(timeSupport, TimeSourceCurrentTime)
{
  const std::vector<std::pair<std::string,clockid_t>> clockv {
    {"CLOCK_REALTIME", CLOCK_REALTIME},
    {"CLOCK_MONOTONIC", CLOCK_MONOTONIC},
    {"CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID},
    {"CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID},
    {"CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW}
  };
  
  for (auto&& p : clockv)
  {
    tspec&& res {};
    int ret {};

    ret = clock_gettime(p.second, &res);
    if ( ret )
    {
      std::cerr << "ERROR: clock_gettime() error"
                << '\n';
    }
    else
    {
      std::cout << p.first
                << " Current time: sec = "
                << res.tv_sec
                << " nsec = "
                << res.tv_nsec
                << '\n';
    }
  }
}

TEST(timeSupport, stopCalledBeforeStart)
{
  timeSupport::rdtscTimer rdtsct {"TIMER"};

  rdtsct();

  auto s1 = rdtsct.getTimerStatus();
  rdtsct.stop("STOP-BEFORE-START");
  auto s2 = rdtsct.getTimerStatus();

  rdtsct();

  ASSERT_EQ(s1, s2);
}

TEST(timeSupport, reportCalledWhenNotStopped)
{
  timeSupport::rdtscTimer rdtsct {"TIMER"};

  rdtsct();

  auto s1 = rdtsct.getTimerStatus();
  rdtsct.report();
  auto s2 = rdtsct.getTimerStatus();

  rdtsct();

  ASSERT_EQ(s1, s2);
}

TEST(timeSupport, reStartAttempt)
{
  timeSupport::rdtscTimer rdtsct {"TIMER"};

  rdtsct();

  rdtsct.start("START-POINT-1");
  auto s1 = rdtsct.getTimerStatus();
  rdtsct.start("START-POINT-2");
  rdtsct.start("START-POINT-3");
  auto s2 = rdtsct.getTimerStatus();

  rdtsct();

  ASSERT_EQ(s1, s2);
}

TEST(timeSupport, reStopAttempt)
{
  timeSupport::rdtscTimer rdtsct {"TIMER"};

  rdtsct();

  rdtsct.start("START-POINT");
  rdtsct.stop("STOP-POINT-1");
  auto s1 = rdtsct.getTimerStatus();
  rdtsct.stop("STOP-POINT-2");
  rdtsct.stop("STOP-POINT-3");
  auto s2 = rdtsct.getTimerStatus();

  rdtsct();

  ASSERT_EQ(s1, s2);
}

TEST(timeSupport, reReportAttempt)
{
  timeSupport::rdtscTimer rdtsct {"TIMER"};

  rdtsct();

  rdtsct.start("START-POINT");
  rdtsct.stop("STOP-POINT-1").report();
  auto s1 = rdtsct.getTimerStatus();
  rdtsct.report();
  rdtsct.report().report();
  auto s2 = rdtsct.getTimerStatus();

  rdtsct();

  ASSERT_EQ(s1, s2);
}

TEST(timeSupport, correctFullCycle)
{
  timeSupport::rdtscTimer rdtsct {"TIMER"};

  rdtsct();

  rdtsct.start("START-POINT").stop("STOP-POINT").report();
  auto s1 = rdtsct.getTimerStatus();

  rdtsct();

  ASSERT_EQ(s1, timeSupport::rdtscTimer::rdtscTimerStatus::REPORTED);
}

TEST(timeSupport, wrongFullCycle)
{
  timeSupport::rdtscTimer rdtsct {"TIMER"};

  rdtsct();

  rdtsct.report().stop("STOP-POINT").start("START-POINT");
  auto s1 = rdtsct.getTimerStatus();

  rdtsct();

  ASSERT_EQ(s1, timeSupport::rdtscTimer::rdtscTimerStatus::STARTED);
}

TEST(timeSupport, rdtscTest_1)
{
  // Only declared. Never used, so no time is measured and no logs are generated
  timeSupport::rdtscTimer rdtsct_1 {"T1"};
  timeSupport::rdtscTimer rdtsct_2 {"T2"};
  timeSupport::rdtscTimer rdtsct_3 {"T3"};

  auto s1 = rdtsct_1.getTimerStatus();
  auto s2 = rdtsct_2.getTimerStatus();
  auto s3 = rdtsct_3.getTimerStatus();

  ASSERT_EQ(s1, timeSupport::rdtscTimer::rdtscTimerStatus::INACTIVE);
  ASSERT_EQ(s2, timeSupport::rdtscTimer::rdtscTimerStatus::INACTIVE);
  ASSERT_EQ(s3, timeSupport::rdtscTimer::rdtscTimerStatus::INACTIVE);
}

TEST(timeSupport, rdtscTest_2)
{
  // all logs to std::cout by default
  timeSupport::rdtscTimer rdtsct_1 {"T1"};
  timeSupport::rdtscTimer rdtsct_2 {"T2"};
  timeSupport::rdtscTimer rdtsct_3 {"T3"};
  timeSupport::rdtscTimer rdtsct_4 {"T4"};
  timeSupport::rdtscTimer rdtsct_5 {"T5"};

  rdtsct_1.start("START-POINT-A").stop("STOP-POINT-A");

  rdtsct_2.start("START-POINT-B");
  rdtsct_2.stop("STOP-POINT-B");

  rdtsct_3.start("START-POINT-C").stop("STOP-POINT-C");

  // stopped by rdtsct_4's dtor at the end of the block
  // this way of stopping is highly inefficient due to the overruns at the end of
  // the block when all the clean up is made
  rdtsct_4.start("START-POINT-D");

  // stopped by rdtsct_5's dtor at the end of the block
  // this way of stopping is inefficient due to the overruns at the end of
  // the block when all the clean up is made
  rdtsct_5.start();
}

TEST(timeSupport, rdtscTest_3)
{
  // logs to std::cout by default
  timeSupport::rdtscTimer rdtsct {"T1"};
  constexpr uint_fast64_t waitTicks {10'000};
  constexpr uint_fast64_t loopsNumber {1'000'000};
  uint_fast64_t overruns {0};
  uint_fast64_t doNotCount {0};
  uint_fast64_t loopStop {0};

  for (uint_fast64_t&& i {1}; i <= loopsNumber; ++i)
  {
    // start counting and set the stop tick
    loopStop = waitTicks + rdtsct.start("START-POINT-A").getStartTSC();

    do
    {}
    while ( timeSupport::rdtscp() < loopStop );

    // stop counting
    rdtsct.stop("STOP-POINT-A");

    // check if overrun occurred
    if ( rdtsct.getStopTSC() > loopStop)
    {
      auto overrun = rdtsct.getStopTSC() - loopStop;
      auto currentAverage = (overruns/(i - doNotCount));
      if ( (i > 1) && (overrun > (currentAverage * 2)) )
      {
        ++doNotCount;
      }
      else
      {
        overruns = overruns + overrun;
      }
      //std::cout << i << ": overrun of " << overrun << " ticks" << '\n';
    }
  }

  auto overrunAverage{overruns/(loopsNumber - doNotCount)};
  std::cout << "average overrun: "
            << overrunAverage
            << " ticks over "
            << loopsNumber - doNotCount
            << " loops"
            << '\n';

  EXPECT_LE(overrunAverage,80);
}

TEST(timeSupport, rdtscTest_4)
{
  // store all the logs generated by the class in a stringstream
  std::stringstream ss {};
  timeSupport::rdtscTimer rdtsct {"T4", ss};

  // NOTE: report() MUST be called after stop(), otherwise nothing is written in
  //       the stringstream
  rdtsct.start("START-POINT-A").stop("STOP-POINT-A").report();

  // print the logs generated so far
  std::cout << "--------------"
            << '\n'
            << ss.str()
            << "--------------"
            << '\n';

  rdtsct.start("START-POINT-B").stopAndReport("STOP-POINT-B");

  // print the logs generated so far
  std::cout << "--------------"
            << '\n'
            << ss.str()
            << "--------------"
            << '\n';

  rdtsct.start("START-POINT-C").stopAndReport("STOP-POINT-C");

  // print the logs generated so far
  std::cout << "--------------"
            << '\n'
            << ss.str()
            << "--------------"
            << '\n';
}

TEST(timeSupport, rdtscTest_5)
{
  // store all the logs generated by the class in a stringstream
  std::stringstream ss {};
  timeSupport::rdtscTimer rdtsct {"T5", ss};

  rdtsct.start("START-POINT-A");

  absnanosleep(1, 0);

  rdtsct.stop("STOP-POINT-A");
  rdtsct.report();

  // print the logs generated so far
  std::cout << "-------rdtscTest_5-------"
            << '\n'
            << ss.str();

  std::cout << "Ticks counted: " << rdtsct.getStopLapsedTSC() << '\n';
#ifdef CHRONO_TIME
  std::cout << "sec   counted: " << rdtsct.getStopLapsed_sec() << '\n';
  std::cout << "msec  counted: " << rdtsct.getStopLapsed_msec() << '\n';
  std::cout << "usec  counted: " << rdtsct.getStopLapsed_usec() << '\n';
  std::cout << "nsec  counted: " << rdtsct.getStopLapsed_nsec() << '\n';
#endif
  std::cout << "-------------------------"
            << '\n';

#ifdef CHRONO_TIME
  EXPECT_GE(rdtsct.getStopLapsed_sec(),  1.0);
  EXPECT_EQ(rdtsct.getStopLapsed_msec(), 1'000);
  EXPECT_GE(rdtsct.getStopLapsed_usec(), 1'000'000);
  EXPECT_GE(rdtsct.getStopLapsed_nsec(), 1'000'000'000);
#endif
}

TEST(timeSupport, rdtscTest_6)
{
  // store all the logs generated by the class in a stringstream
  std::stringstream ss {};

  {
    timeSupport::rdtscTimer rdtsct {"T6", ss};

    rdtsct.start("START-POINT-A");

    absnanosleep(10, 0);

    rdtsct();
  } // rdtsct's dtor called: stop() and report() called

  // print the logs generated in the stringstream
  std::cout << "-------rdtscTest_6-------"
            << '\n'
            << ss.str()
            << "-------------------------"
            << '\n';

  ASSERT_NE(ss.str(), "");
}

TEST(timeSupport, rdtscTest_7)
{
  // store all the logs generated by the class in a stringstream
  std::stringstream ss {};

  {
    // function to profile
    decltype(auto) nofun = [](){};

    // function to profile
    decltype(auto) f = [](const time_t& seconds){ absnanosleep(seconds,0); };

    timeSupport::rdtscTimer rdtsct {"T7", ss};

    for (unsigned int&& i {1}; i < 100; ++i)
    {
      timeSupport::profileFunction(rdtsct, "START-PROFILE-NOFUN", "STOP-PROFILE-NOFUN", nofun);
    }
    
    for (unsigned int&& i {1}; i < 20; ++i)
    {
      timeSupport::profileFunction(rdtsct, "START-PROFILE-F", "STOP-PROFILE-F", f, 1);
    }

    rdtsct();
  } // rdtsct's dtor called

  // print the logs generated in the stringstream
  std::cout << "-------rdtscTest_7-------"
            << '\n'
            << ss.str()
            << "-------------------------"
            << '\n';

  ASSERT_NE(ss.str(), "");
}

////////////////////////////////////////////////////////////////////////////////
// the following tests need super user rights
// they fail when run as a user with standard privileges

static int setRealtimePriority(const int algo = SCHED_FIFO) noexcept
{
  int ret {};

  // work on the currently running thread.
  pthread_t this_thread {pthread_self()};
  // struct sched_param is used to store the scheduling priority
  struct sched_param params {};

  // set the priority to the maximum.
  params.sched_priority = sched_get_priority_max(algo);

//  std::cout << "Trying to set thread realtime prio = " << params.sched_priority << '\n';

  // attempt to set thread real-time priority to the algo policy
  ret = pthread_setschedparam(this_thread, algo, &params);
  if ( 0 != ret )
  {
    // print the error
    std::cout << "Unsuccessful in setting thread realtime prio" << '\n';
    return -1;
  }
  // verify the change in thread priority
  int policy{0};
  ret = pthread_getschedparam(this_thread, &policy, &params);
  if ( 0 != ret )
  {
    std::cout << "Couldn't retrieve real-time scheduling paramers" << '\n';
    return -1;
  }

  // check the correct policy was applied
  if ( policy != algo )
  {
    std::cout << "Scheduling is NOT " << algo << '\n';
  }

  // print thread scheduling priority
  //std::cout << "Thread priority is " << params.sched_priority << '\n';

  return policy;
}

TEST(timeSupport, rdtscTest_8)
{
  constexpr int which {PRIO_PROCESS};
  constexpr int priority {-20};
  int ret {};
  decltype(errno) errNo = errno;

  auto pid = getpid();
  ret = setpriority(which, static_cast<id_t>(pid), priority);
  errNo = errno;

  EXPECT_EQ(ret,0);
  EXPECT_EQ(errNo,0);

  // use SCHED_FIFO or SCHED_RR
  decltype(SCHED_FIFO) algo = SCHED_FIFO;

  ASSERT_EQ(setRealtimePriority(algo),algo);

  // logs to std::cout by default
  timeSupport::rdtscTimer rdtsct {"T8"};
  constexpr uint_fast64_t waitTicks {10'000};
  constexpr uint_fast64_t loopsNumber {2'000'000};
  uint_fast64_t overruns {0};
  uint_fast64_t doNotCount {0};
  uint_fast64_t loopStop {0};

  for (uint_fast64_t&& i {1}; i <= loopsNumber; ++i)
  {
    // start counting and set the stop tick
    loopStop = waitTicks + rdtsct.start("START-POINT-A").getStartTSC();

    do
    {}
    while ( timeSupport::rdtscp() < loopStop );

    // stop counting
    rdtsct.stop("STOP-POINT-A");

    // check if overrun occurred
    if ( rdtsct.getStopTSC() > loopStop)
    {
      auto overrun = rdtsct.getStopTSC() - loopStop;
      auto currentAverage = (overruns/(i - doNotCount));
      if ( (i > 1) && (overrun > (currentAverage * 2)) )
      {
        ++doNotCount;
      }
      else
      {
        overruns = overruns + overrun;
      }
      //std::cout << i << ": overrun of " << overrun << " ticks" << '\n';
    }
  }

  auto overrunAverage{overruns/(loopsNumber - doNotCount)};
  std::cout << "average overrun: "
            << overrunAverage
            << " ticks over "
            << loopsNumber - doNotCount
            << " loops"
            << '\n';

  rdtsct();

  EXPECT_LE(overrunAverage,80);
}
////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
