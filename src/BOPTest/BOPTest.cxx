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

#include <BOPTest.hxx>
#include <BRepTest.hxx>
#include <DBRep.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_PluginMacro.hxx>
#include <GeometryTest.hxx>
#include <GeomliteTest.hxx>
#include <HLRTest.hxx>
#include <NCollection_Map.hxx>
#include <MeshTest.hxx>
#include <Message_Msg.hxx>
#include <SWDRAW.hxx>

#include <BOPAlgo_Alerts.hxx>
#include <BOPTest_Objects.hxx>

//=================================================================================================

void BOPTest1::AllCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;
  //
  BOPTest1::BOPCommands(theCommands);
  BOPTest1::CheckCommands(theCommands);
  BOPTest1::LowCommands(theCommands);
  BOPTest1::TolerCommands(theCommands);
  BOPTest1::ObjCommands(theCommands);
  BOPTest1::PartitionCommands(theCommands);
  BOPTest1::APICommands(theCommands);
  BOPTest1::OptionCommands(theCommands);
  BOPTest1::DebugCommands(theCommands);
  BOPTest1::CellsCommands(theCommands);
  BOPTest1::UtilityCommands(theCommands);
  BOPTest1::RemoveFeaturesCommands(theCommands);
  BOPTest1::PeriodicityCommands(theCommands);
  BOPTest1::MkConnectedCommands(theCommands);
}

//=================================================================================================

void BOPTest1::Factory(DrawInterpreter& theCommands)
{
  static Standard_Boolean FactoryDone = Standard_False;
  if (FactoryDone)
    return;

  FactoryDone = Standard_True;

  DBRep1::BasicCommands(theCommands);
  GeomliteTest1::AllCommands(theCommands);
  GeometryTest1::AllCommands(theCommands);
  BRepTest1::AllCommands(theCommands);
  MeshTest1::Commands(theCommands);
  HLRTest1::Commands(theCommands);
  BOPTest1::AllCommands(theCommands);
  SWDRAW1::Init(theCommands);
}
// Declare entry point PLUGINFACTORY
DPLUGIN(BOPTest1)

//=================================================================================================

void BOPTest1::ReportAlerts(const Handle(Message_Report)& theReport)
{
  // first report warnings, then errors
  Message_Gravity            anAlertTypes[2] = {Message_Warning, Message_Fail};
  UtfString aMsgType[2]     = {"Warning: ", "Error: "};
  for (int iGravity = 0; iGravity < 2; iGravity++)
  {
    // report shapes for the same type of alert together
    NCollection_Map<Handle(RefObject)> aPassedTypes;
    const Message_ListOfAlert& aList = theReport->GetAlerts(anAlertTypes[iGravity]);
    for (Message_ListOfAlert::Iterator aIt(aList); aIt.More(); aIt.Next())
    {
      // check that this type of warnings has not yet been processed
      const Handle(TypeInfo)& aType = aIt.Value()->DynamicType();
      if (!aPassedTypes.Add(aType))
        continue;

      // get alert message
      Message_Msg                aMsg(aIt.Value()->GetMessageKey());
      UtfString aText = aMsgType[iGravity] + aMsg.Get();

      // collect all shapes if any attached to this alert
      if (Objects::DrawWarnShapes())
      {
        AsciiString1 aShapeList;
        Standard_Integer        aNbShapes = 0;
        for (Message_ListOfAlert::Iterator aIt2(aIt); aIt2.More(); aIt2.Next())
        {
          Handle(TopoDS_AlertWithShape) aShapeAlert =
            Handle(TopoDS_AlertWithShape)::DownCast(aIt2.Value());

          if (!aShapeAlert.IsNull() && (aType == aShapeAlert->DynamicType())
              && !aShapeAlert->GetShape().IsNull())
          {
            //
            char aName[80];
            Sprintf(aName, "%ss_%d_%d", (iGravity ? "e" : "w"), aPassedTypes.Extent(), ++aNbShapes);
            DBRep1::Set(aName, aShapeAlert->GetShape());
            //
            aShapeList += " ";
            aShapeList += aName;
          }
        }
        aText += (aNbShapes ? ": " : "(no shapes attached)");
        aText += aShapeList;
      }

      // output message with list of shapes
      DrawInterpreter& aDrawInterpretor = Draw1::GetInterpretor();
      aDrawInterpretor << aText << "\n";
    }
  }
}

//=================================================================================================

BOPAlgo_Operation BOPTest1::GetOperationType(const Standard_CString theOp)
{
  AsciiString1 anOp(theOp);
  anOp.LowerCase();

  if (anOp.IsIntegerValue())
  {
    // Check if the given value satisfies the enumeration.
    Standard_Integer iOp = anOp.IntegerValue();
    if (iOp >= 0 && iOp <= 4)
    {
      return static_cast<BOPAlgo_Operation>(iOp);
    }
    return BOPAlgo_UNKNOWN;
  }

  // Check for the meaningful symbolic operation parameter
  if (anOp == "common")
  {
    return BOPAlgo_COMMON;
  }
  else if (anOp == "fuse")
  {
    return BOPAlgo_FUSE;
  }
  else if (anOp == "cut")
  {
    return BOPAlgo_CUT;
  }
  else if (anOp == "tuc" || anOp == "cut21")
  {
    return BOPAlgo_CUT21;
  }
  else if (anOp == "section")
  {
    return BOPAlgo_SECTION;
  }

  return BOPAlgo_UNKNOWN;
}
