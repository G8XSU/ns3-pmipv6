/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 * Copyright (c) 2007 Emmanuelle Laprise
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * TimeStep support by Emmanuelle Laprise <emmanuelle.laprise@bluekazoo.ca>
 */
#include "ns3/nstime.h"
#include "ns3/test.h"

using namespace ns3;

class TimeSimpleTestCase : public TestCase
{
public:
  TimeSimpleTestCase ();
private:
  virtual void DoSetup (void);
  virtual void DoRun (void);
  virtual void DoTeardown (void);
};

TimeSimpleTestCase::TimeSimpleTestCase ()
  : TestCase ("Sanity check of common time operations")
{
}

void
TimeSimpleTestCase::DoSetup (void)
{
}

void
TimeSimpleTestCase::DoRun (void)
{
  NS_TEST_ASSERT_MSG_EQ_TOL (Years (1.0).GetYears (), 1.0, Years (1).GetYears (),
                             "is 1 really 1 ?");
  NS_TEST_ASSERT_MSG_EQ_TOL (Years (10.0).GetYears (), 10.0, Years (1).GetYears (),
                             "is 10 really 10 ?");
  NS_TEST_ASSERT_MSG_EQ_TOL (Days (1.0).GetDays (), 1.0, Days (1).GetDays (),
                             "is 1 really 1 ?");
  NS_TEST_ASSERT_MSG_EQ_TOL (Days (10.0).GetDays (), 10.0, Days (1).GetDays (),
                             "is 10 really 10 ?");
  NS_TEST_ASSERT_MSG_EQ_TOL (Hours (1.0).GetHours (), 1.0, Hours (1).GetHours (),
                             "is 1 really 1 ?");
  NS_TEST_ASSERT_MSG_EQ_TOL (Hours (10.0).GetHours (), 10.0, Hours (1).GetHours (),
                             "is 10 really 10 ?");
  NS_TEST_ASSERT_MSG_EQ_TOL (Minutes (1.0).GetMinutes (), 1.0, Minutes (1).GetMinutes (),
                             "is 1 really 1 ?");
  NS_TEST_ASSERT_MSG_EQ_TOL (Minutes (10.0).GetMinutes (), 10.0, Minutes (1).GetMinutes (),
                             "is 10 really 10 ?");
  NS_TEST_ASSERT_MSG_EQ_TOL (Seconds (1.0).GetSeconds (), 1.0, TimeStep (1).GetSeconds (), 
                             "is 1 really 1 ?");
  NS_TEST_ASSERT_MSG_EQ_TOL (Seconds (10.0).GetSeconds (), 10.0, TimeStep (1).GetSeconds (), 
                             "is 10 really 10 ?");
  NS_TEST_ASSERT_MSG_EQ (MilliSeconds (1).GetMilliSeconds (), 1, 
                         "is 1ms really 1ms ?");
  NS_TEST_ASSERT_MSG_EQ (MicroSeconds (1).GetMicroSeconds (), 1, 
                         "is 1us really 1us ?");
#if 0
  Time ns = NanoSeconds (1);
  ns.GetNanoSeconds ();
  NS_TEST_ASSERT_MSG_EQ (NanoSeconds (1).GetNanoSeconds (), 1, 
                         "is 1ns really 1ns ?");
  NS_TEST_ASSERT_MSG_EQ (PicoSeconds (1).GetPicoSeconds (), 1, 
                         "is 1ps really 1ps ?");
  NS_TEST_ASSERT_MSG_EQ (FemtoSeconds (1).GetFemtoSeconds (), 1, 
                         "is 1fs really 1fs ?");
#endif

  Time ten = NanoSeconds (10);
  int64_t tenValue = ten.GetInteger ();
  Time::SetResolution (Time::PS);
  int64_t tenKValue = ten.GetInteger ();
  NS_TEST_ASSERT_MSG_EQ (tenValue * 1000, tenKValue,
                         "change resolution to PS");
}

void 
TimeSimpleTestCase::DoTeardown (void)
{
}

class TimesWithSignsTestCase : public TestCase
{
public:
  TimesWithSignsTestCase ();
private:
  virtual void DoSetup (void);
  virtual void DoRun (void);
  virtual void DoTeardown (void);
};

TimesWithSignsTestCase::TimesWithSignsTestCase ()
  : TestCase ("Checks times that have plus or minus signs")
{
}

void
TimesWithSignsTestCase::DoSetup (void)
{
}

void
TimesWithSignsTestCase::DoRun (void)
{
  Time timePositive          ("+1000.0");
  Time timePositiveWithUnits ("+1000.0ms");

  Time timeNegative          ("-1000.0");
  Time timeNegativeWithUnits ("-1000.0ms");

  NS_TEST_ASSERT_MSG_EQ_TOL (timePositive.GetSeconds (),
                             +1000.0,
                             1.0e-8,
                             "Positive time not parsed correctly.");

  NS_TEST_ASSERT_MSG_EQ_TOL (timePositiveWithUnits.GetSeconds (),
                             +1.0,
                             1.0e-8,
                             "Positive time with units not parsed correctly.");

  NS_TEST_ASSERT_MSG_EQ_TOL (timeNegative.GetSeconds (),
                             -1000.0,
                             1.0e-8,
                             "Negative time not parsed correctly.");

  NS_TEST_ASSERT_MSG_EQ_TOL (timeNegativeWithUnits.GetSeconds (),
                             -1.0,
                             1.0e-8,
                             "Negative time with units not parsed correctly.");
}

void 
TimesWithSignsTestCase::DoTeardown (void)
{
}

static class TimeTestSuite : public TestSuite
{
public:
  TimeTestSuite ()
    : TestSuite ("time", UNIT)
  {
    AddTestCase (new TimeSimpleTestCase (), TestCase::QUICK);
    AddTestCase (new TimesWithSignsTestCase (), TestCase::QUICK);
  }
} g_timeTestSuite;
