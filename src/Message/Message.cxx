// Created on: 1999-11-23
// Created by: data exchange team
// Copyright (c) 1999-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Message_Report.hxx>
#include <TCollection_AsciiString.hxx>

#include <stdio.h>

namespace
{
static Standard_CString Message_Table_PrintMetricTypeEnum[13] = {"NONE",
                                                                 "ThreadCPUUserTime",
                                                                 "ThreadCPUSystemTime",
                                                                 "ProcessCPUUserTime",
                                                                 "ProcessCPUSystemTime",
                                                                 "WallClock",
                                                                 "MemPrivate",
                                                                 "MemVirtual",
                                                                 "MemWorkingSet",
                                                                 "MemWorkingSetPeak",
                                                                 "MemSwapUsage",
                                                                 "MemSwapUsagePeak",
                                                                 "MemHeapUsage"};
}

//=================================================================================================

const Handle(Message_Messenger)& Message1::DefaultMessenger()
{
  static Handle(Message_Messenger) aMessenger = new Message_Messenger;
  return aMessenger;
}

//=================================================================================================

AsciiString1 Message1::FillTime(const Standard_Integer hour,
                                          const Standard_Integer minute,
                                          const Standard_Real    second)
{
  char t[30];
  if (hour > 0)
    Sprintf(t, "%02dh:%02dm:%.2fs", hour, minute, second);
  else if (minute > 0)
    Sprintf(t, "%02dm:%.2fs", minute, second);
  else
    Sprintf(t, "%.2fs", second);
  return AsciiString1(t);
}

//=================================================================================================

const Handle(Message_Report)& Message1::DefaultReport(const Standard_Boolean theToCreate)
{
  static Handle(Message_Report) MyReport;
  if (MyReport.IsNull() && theToCreate)
  {
    MyReport = new Message_Report();
  }
  return MyReport;
}

//=================================================================================================

Standard_CString Message1::MetricToString(const Message_MetricType theType)
{
  return Message_Table_PrintMetricTypeEnum[theType];
}

//=================================================================================================

Standard_Boolean Message1::MetricFromString(const Standard_CString theString,
                                           Message_MetricType&    theGravity)
{
  AsciiString1 aName(theString);
  for (Standard_Integer aMetricIter = 0; aMetricIter <= Message_MetricType_MemHeapUsage;
       ++aMetricIter)
  {
    Standard_CString aMetricName = Message_Table_PrintMetricTypeEnum[aMetricIter];
    if (aName == aMetricName)
    {
      theGravity = Message_MetricType(aMetricIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean Message1::ToOSDMetric(const Message_MetricType theMetric,
                                      MemoryInfo::Counter&    theMemInfo)
{
  switch (theMetric)
  {
    case Message_MetricType_MemPrivate:
      theMemInfo = MemoryInfo::MemPrivate;
      break;
    case Message_MetricType_MemVirtual:
      theMemInfo = MemoryInfo::MemVirtual;
      break;
    case Message_MetricType_MemWorkingSet:
      theMemInfo = MemoryInfo::MemWorkingSet;
      break;
    case Message_MetricType_MemWorkingSetPeak:
      theMemInfo = MemoryInfo::MemWorkingSetPeak;
      break;
    case Message_MetricType_MemSwapUsage:
      theMemInfo = MemoryInfo::MemSwapUsage;
      break;
    case Message_MetricType_MemSwapUsagePeak:
      theMemInfo = MemoryInfo::MemSwapUsagePeak;
      break;
    case Message_MetricType_MemHeapUsage:
      theMemInfo = MemoryInfo::MemHeapUsage;
      break;
    default:
      return Standard_False;
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean Message1::ToMessageMetric(const MemoryInfo::Counter theMemInfo,
                                          Message_MetricType&        theMetric)
{
  switch (theMemInfo)
  {
    case MemoryInfo::MemPrivate:
      theMetric = Message_MetricType_MemPrivate;
      break;
    case MemoryInfo::MemVirtual:
      theMetric = Message_MetricType_MemVirtual;
      break;
    case MemoryInfo::MemWorkingSet:
      theMetric = Message_MetricType_MemWorkingSet;
      break;
    case MemoryInfo::MemWorkingSetPeak:
      theMetric = Message_MetricType_MemWorkingSetPeak;
      break;
    case MemoryInfo::MemSwapUsage:
      theMetric = Message_MetricType_MemSwapUsage;
      break;
    case MemoryInfo::MemSwapUsagePeak:
      theMetric = Message_MetricType_MemSwapUsagePeak;
      break;
    case MemoryInfo::MemHeapUsage:
      theMetric = Message_MetricType_MemHeapUsage;
      break;
    default:
      return Standard_False;
  }
  return Standard_True;
}
