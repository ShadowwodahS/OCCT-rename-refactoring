// Copyright (c) 2020 OPEN CASCADE SAS
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

#ifndef _Message_MetricType_HeaderFile
#define _Message_MetricType_HeaderFile

//! Specifies kind of report information to collect
enum Message_MetricType
{
  Message_MetricType_None,                 //!< no computation
  Message_MetricType_ThreadCPUUserTime,    //!< Chronometer::GetThreadCPU user time
  Message_MetricType_ThreadCPUSystemTime,  //!< Chronometer::GetThreadCPU system time
  Message_MetricType_ProcessCPUUserTime,   //!< Chronometer::GetProcessCPU user time
  Message_MetricType_ProcessCPUSystemTime, //!< Chronometer::GetProcessCPU system time
  Message_MetricType_WallClock,            //!< OSD_Timer elapsed time
  Message_MetricType_MemPrivate,           //!< MemoryInfo::MemPrivate
  Message_MetricType_MemVirtual,           //!< MemoryInfo::MemVirtual
  Message_MetricType_MemWorkingSet,        //!< MemoryInfo::MemWorkingSet
  Message_MetricType_MemWorkingSetPeak,    //!< MemoryInfo::MemWorkingSetPeak
  Message_MetricType_MemSwapUsage,         //!< MemoryInfo::MemSwapUsage
  Message_MetricType_MemSwapUsagePeak,     //!< MemoryInfo::MemSwapUsagePeak
  Message_MetricType_MemHeapUsage          //!< MemoryInfo::MemHeapUsage
};

#endif // _Message_MetricType_HeaderFile
