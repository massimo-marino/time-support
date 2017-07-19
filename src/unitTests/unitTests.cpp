/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "../time_support.h"
#include <sys/resource.h>
#include <vector>
#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;
////////////////////////////////////////////////////////////////////////////////
void nanosleep() noexcept
{
  struct timespec req = { 5, 0 };
  struct timespec rem{};
  int ret{};

  retry:
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
    goto retry;
  }
  std::cerr << "ERROR: nanosleep()"
            << std::endl;
}

void absnanosleep(const time_t& seconds = 0,
                  const long& nanoseconds = 0) noexcept
{
  static clockid_t clock{CLOCK_REALTIME};
  struct timespec ts{};

  clock_gettime(clock, &ts);

  ts.tv_sec = ts.tv_sec + seconds;
  ts.tv_nsec = ts.tv_nsec + nanoseconds;
  clock_nanosleep(clock, TIMER_ABSTIME, &ts, NULL);
}

////////////////////////////////////////////////////////////////////////////////
TEST(timeSupport, TimeSourceResolution)
{
  std::vector<std::pair<std::string,clockid_t>> clockv{ {"CLOCK_REALTIME", CLOCK_REALTIME},
                                                        {"CLOCK_MONOTONIC", CLOCK_MONOTONIC},
                                                        {"CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID},
                                                        {"CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID},
                                                        {"CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW}
  };
  for (const auto& p : clockv)
  {
    struct timespec res{};
    int ret{};

    ret = clock_getres(p.second, &res);
    if ( ret )
    {
      std::cerr << "ERROR: clock_getres() error"
                << std::endl;
    }
    else
    {
      std::cout << p.first
                << " Time resolution: sec = "
                << res.tv_sec
                << " nsec = "
                << res.tv_nsec
                << std::endl;
    }
  }
}

TEST(timeSupport, TimeSourceCurrentTime)
{
  std::vector<std::pair<std::string,clockid_t>> clockv{ {"CLOCK_REALTIME", CLOCK_REALTIME},
                                                        {"CLOCK_MONOTONIC", CLOCK_MONOTONIC},
                                                        {"CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID},
                                                        {"CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID},
                                                        {"CLOCK_MONOTONIC_RAW", CLOCK_MONOTONIC_RAW}
  };
  
  for (const auto& p : clockv)
  {
    struct timespec res{};
    int ret{};

    ret = clock_gettime(p.second, &res);
    if ( ret )
    {
      std::cerr << "ERROR: clock_gettime() error"
                << std::endl;
    }
    else
    {
      std::cout << p.first
                << " Current time: sec = "
                << res.tv_sec
                << " nsec = "
                << res.tv_nsec
                << std::endl;
    }
  }
}

TEST(timeSupport, rdtscTest_1)
{
  // Only declared. Never used, so no time is measured and no logs are generated
  timeSupport::rdtscTimer rdtsct_1{"T1"};
  timeSupport::rdtscTimer rdtsct_2{"T2"};
  timeSupport::rdtscTimer rdtsct_3{"T3"};
}

TEST(timeSupport, rdtscTest_2)
{
  // all logs to std::cout by default
  timeSupport::rdtscTimer rdtsct_1{"T1"};
  timeSupport::rdtscTimer rdtsct_2{"T2"};
  timeSupport::rdtscTimer rdtsct_3{"T3"};
  timeSupport::rdtscTimer rdtsct_4{"T4"};
  timeSupport::rdtscTimer rdtsct_5{"T5"};

  rdtsct_1.start("START-POINT-A")->stop("STOP-POINT-A");

  rdtsct_2.start("START-POINT-B");
  rdtsct_2.stop("STOP-POINT-B");

  rdtsct_3.start("START-POINT-C")->stop("STOP-POINT-C");

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
  timeSupport::rdtscTimer rdtsct{"T1"};
  uint_fast64_t waitTicks{10'000};
  uint_fast64_t loopsNumber{1'000'000};
  uint_fast64_t overruns{0};
  uint_fast64_t doNotCount{0};
  uint_fast64_t loopStop{};

  for (uint_fast64_t i{1}; i <= loopsNumber; ++i)
  {
    // start counting and set the stop tick
    loopStop = waitTicks + rdtsct.start("START-POINT-A")->getStartTSC();

    do
    {}
    while ( timeSupport::rdtsc() < loopStop );

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
      //std::cout << i << ": overrun of " << overrun << " ticks" << std::endl;
    }
  }

  auto overrunAverage{overruns/(loopsNumber - doNotCount)};
  std::cout << "overrun average: " << overrunAverage << " ticks over " << loopsNumber - doNotCount << " loops" << std::endl;

  EXPECT_LT(overrunAverage,55);
}

TEST(timeSupport, rdtscTest_4)
{
  // store all the logs generated by the class in a stringstream
  std::stringstream ss{};
  timeSupport::rdtscTimer rdtsct{"T4", ss};

  // NOTE: report() MUST be called after stop(), otherwise nothing is written in
  //       the stringstream
  rdtsct.start("START-POINT-A")->stop("STOP-POINT-A")->report();

  // print the logs generated so far
  std::cout << "--------------"
            << std::endl
            << ss.str()
            << "--------------"
            << std::endl;

  rdtsct.start("START-POINT-B")->stopAndReport("STOP-POINT-B");

  // print the logs generated so far
  std::cout << "--------------"
            << std::endl
            << ss.str()
            << "--------------"
            << std::endl;

  rdtsct.start("START-POINT-C")->stopAndReport("STOP-POINT-C");

  // print the logs generated so far
  std::cout << "--------------"
            << std::endl
            << ss.str()
            << "--------------"
            << std::endl;
}

TEST(timeSupport, rdtscTest_5)
{
  // store all the logs generated by the class in a stringstream
  std::stringstream ss{};
  timeSupport::rdtscTimer rdtsct{"T5", ss};

  rdtsct.start("START-POINT-A");

  absnanosleep(1, 0);

  rdtsct.stop("STOP-POINT-A");
  rdtsct.report();

  // print the logs generated so far
  std::cout << "--------------"
            << std::endl
            << ss.str();

  std::cout << "Ticks counted: " << rdtsct.getStopLapsedTSC() << std::endl;
  std::cout << "sec   counted: " << rdtsct.getStopLapsed_sec() << std::endl;
  std::cout << "msec  counted: " << rdtsct.getStopLapsed_msec() << std::endl;
  std::cout << "usec  counted: " << rdtsct.getStopLapsed_usec() << std::endl;
  std::cout << "nsec  counted: " << rdtsct.getStopLapsed_nsec() << std::endl;
  std::cout << "--------------"
            << std::endl;

  EXPECT_GE(rdtsct.getStopLapsed_sec(),  1.0);
  EXPECT_EQ(rdtsct.getStopLapsed_msec(), 1'000);
  EXPECT_GE(rdtsct.getStopLapsed_usec(), 1'000'000);
  EXPECT_GE(rdtsct.getStopLapsed_nsec(), 1'000'000'000);
}

////////////////////////////////////////////////////////////////////////////////
// the following tests need su rights

int setRealtimePriority(const int algo = SCHED_FIFO) noexcept
{
  int ret{};

  // work on the currently running thread.
  pthread_t this_thread{pthread_self()};
  // struct sched_param is used to store the scheduling priority
  struct sched_param params{};

  // set the priority to the maximum.
  params.sched_priority = sched_get_priority_max(algo);

//  std::cout << "Trying to set thread realtime prio = " << params.sched_priority << std::endl;

  // attempt to set thread real-time priority to the algo policy
  ret = pthread_setschedparam(this_thread, algo, &params);
  if ( 0 != ret )
  {
    // print the error
    std::cout << "Unsuccessful in setting thread realtime prio" << std::endl;
    return -1;
  }
  // verify the change in thread priority
  int policy{0};
  ret = pthread_getschedparam(this_thread, &policy, &params);
  if ( 0 != ret )
  {
    std::cout << "Couldn't retrieve real-time scheduling paramers" << std::endl;
    return -1;
  }

  // check the correct policy was applied
  if ( policy != algo )
  {
    std::cout << "Scheduling is NOT " << algo << std::endl;
  }

  // print thread scheduling priority
  //std::cout << "Thread priority is " << params.sched_priority << std::endl;

  return policy;
}

TEST(timeSupport, rdtscTest_6)
{
  int which{PRIO_PROCESS};
  id_t pid{};
  int priority{-20};
  int ret{};

  pid = getpid();
  ret = setpriority(which, pid, priority);
  auto errNo = errno;

  ASSERT_EQ(ret,0);
  ASSERT_EQ(errNo,0);

  ASSERT_EQ(setRealtimePriority(SCHED_FIFO),SCHED_FIFO);
  //ASSERT_EQ(setRealtimePriority(SCHED_RR),SCHED_RR);

  // logs to std::cout by default
  timeSupport::rdtscTimer rdtsct{"T6"};
  uint_fast64_t waitTicks{10'000};
  uint_fast64_t loopsNumber{2'000'000};
  uint_fast64_t overruns{0};
  uint_fast64_t doNotCount{0};
  uint_fast64_t loopStop{};

  for (uint_fast64_t i{1}; i <= loopsNumber; ++i)
  {
    // start counting and set the stop tick
    loopStop = waitTicks + rdtsct.start("START-POINT-A")->getStartTSC();

    do
    {}
    while ( timeSupport::rdtsc() < loopStop );

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
      //std::cout << i << ": overrun of " << overrun << " ticks" << std::endl;
    }
  }

  auto overrunAverage{overruns/(loopsNumber - doNotCount)};
  std::cout << "overrun average: " << overrunAverage << " ticks over " << loopsNumber - doNotCount << " loops" << std::endl;

  EXPECT_LT(overrunAverage,55);
}

//int main(int argc, char **argv) {
//  ::testing::InitGoogleTest(&argc, argv);
//  return RUN_ALL_TESTS();
//}
