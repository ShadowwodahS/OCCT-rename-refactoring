// Created on: 2000-05-18
// Created by: Peter KURNEV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#ifndef _BOPTest_HeaderFile
#define _BOPTest_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>
#include <BOPAlgo_Operation.hxx>

#include <Draw_Interpretor.hxx>
class Message_Report;

class BOPTest
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT static void AllCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void BOPCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void CheckCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void TolerCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void LowCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void ObjCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void PartitionCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void APICommands(DrawInterpreter& aDI);

  Standard_EXPORT static void OptionCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void Factory(DrawInterpreter& aDI);

  Standard_EXPORT static void DebugCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void CellsCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void UtilityCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void RemoveFeaturesCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void PeriodicityCommands(DrawInterpreter& aDI);

  Standard_EXPORT static void MkConnectedCommands(DrawInterpreter& aDI);

  //! Prints errors and warnings if any and draws attached shapes
  //! if flag BOPTest_Objects::DrawWarnShapes() is set
  Standard_EXPORT static void ReportAlerts(const Handle(Message_Report)& theReport);

  //! Returns operation type according to the given string.
  //! For numeric values, the number correspond to the order in enum.
  Standard_EXPORT static BOPAlgo_Operation GetOperationType(const Standard_CString theOp);
};

#endif // _BOPTest_HeaderFile
