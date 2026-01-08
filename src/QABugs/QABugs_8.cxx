// Created on: 2002-03-19
// Created by: QA Admin
// Copyright (c) 2002-2014 OPEN CASCADE SAS
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

#include <QABugs.hxx>

#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <DBRep.hxx>
#include <AIS_InteractiveContext.hxx>
#include <ViewerTest.hxx>
#include <AIS_Shape.hxx>
#include <TopoDS_Shape.hxx>

#include <V3d_View.hxx>

#include <BRepOffsetAPI_Sewing.hxx>

#include <AIS_ListOfInteractive.hxx>

#include <BRepPrimAPI_MakeBox.hxx>

static Standard_Integer OCC162(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " name\n";
    return 1;
  }

  TopoShape aShape = DBRep1::Get(argv[1]);
  if (aShape.IsNull())
    return 0;

  Standard_Real        tolValue = 0.0001;
  BRepOffsetAPI_Sewing sew(tolValue);
  sew.Add(aShape);
  sew.Perform();
  TopoShape aSewed = sew.SewedShape();

  return 0;
}

static Standard_Integer OCC172(DrawInterpreter& di, Standard_Integer /*argc*/, const char** argv)
{
  Handle(VisualContext) aContext = ViewerTest1::GetAISContext();
  if (aContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return 1;
  }

  AIS_ListOfInteractive aListOfIO;
  aContext->DisplayedObjects(aListOfIO);
  AIS_ListIteratorOfListOfInteractive It;
  for (It.Initialize(aListOfIO); It.More(); It.Next())
  {
    aContext->AddOrRemoveSelected(It.Value(), Standard_False);
  }
  aContext->UpdateCurrentViewer();
  return 0;
}

static Standard_Integer OCC204(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " updateviewer=0/1\n";
    return 1;
  }

  Handle(VisualContext) aContext = ViewerTest1::GetAISContext();
  if (aContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return 1;
  }
  Standard_Boolean UpdateViewer        = Standard_True;
  Standard_Integer IntegerUpdateViewer = Draw1::Atoi(argv[1]);
  if (IntegerUpdateViewer == 0)
  {
    UpdateViewer = Standard_False;
  }

  Standard_Integer    deltaY = -500;
  BoxMaker box1(Point3d(0, 0 + deltaY, 0), Point3d(100, 100 + deltaY, 100));
  BoxMaker box2(Point3d(120, 120 + deltaY, 120), Point3d(300, 300 + deltaY, 300));
  BoxMaker box3(Point3d(320, 320 + deltaY, 320), Point3d(500, 500 + deltaY, 500));

  Handle(VisualEntity) ais1 = new VisualShape(box1.Shape());
  Handle(VisualEntity) ais2 = new VisualShape(box2.Shape());
  Handle(VisualEntity) ais3 = new VisualShape(box3.Shape());

  aContext->Display(ais1, Standard_False);
  aContext->Display(ais2, Standard_False);
  aContext->Display(ais3, Standard_False);

  aContext->AddOrRemoveSelected(ais1, Standard_False);
  aContext->AddOrRemoveSelected(ais2, Standard_False);
  aContext->AddOrRemoveSelected(ais3, Standard_False);

  aContext->UpdateCurrentViewer();

  // printf("\n No of currents = %d", aContext->NbCurrents());

  aContext->InitSelected();

  // int count = 1;
  while (aContext->MoreSelected())
  {
    // printf("\n count is = %d",  count++);
    Handle(VisualEntity) ais = aContext->SelectedInteractive();
    aContext->Remove(ais, UpdateViewer);
    aContext->InitSelected();
  }

  return 0;
}

#include <gp_Lin.hxx>
#include <BRepClass3d_Intersector3d.hxx>
#include <TopoDS.hxx>

static Standard_Integer OCC1651(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 8)
  {
    di << "Usage : " << argv[0] << " Shape PntX PntY PntZ DirX DirY DirZ\n";
    return 1;
  }

  TopoShape aShape = DBRep1::Get(argv[1]);
  if (aShape.IsNull())
    return 0;

  Point3d                    aP1(Draw1::Atof(argv[2]), Draw1::Atof(argv[3]), Draw1::Atof(argv[4]));
  Dir3d                    aD1(Draw1::Atof(argv[5]), Draw1::Atof(argv[6]), Draw1::Atof(argv[7]));
  gp_Lin                    aL1(aP1, aD1);
  BRepClass3d_Intersector3d aI1;
  aI1.Perform(aL1, -250, 1e-7, TopoDS::Face(aShape));
  if (aI1.IsDone() && aI1.HasAPoint())
  {
    Point3d aR1 = aI1.Pnt();
    di << aR1.X() << " " << aR1.Y() << " " << aR1.Z() << "\n";
  }

  return 0;
}

void QABugs1::Commands_8(DrawInterpreter& theCommands)
{
  const char* group = "QABugs1";

  theCommands.Add("OCC162", "OCC162 name", __FILE__, OCC162, group);
  theCommands.Add("OCC172", "OCC172", __FILE__, OCC172, group);
  theCommands.Add("OCC204", "OCC204 updateviewer=0/1", __FILE__, OCC204, group);
  theCommands.Add("OCC1651",
                  "OCC1651 Shape PntX PntY PntZ DirX DirY DirZ",
                  __FILE__,
                  OCC1651,
                  group);

  return;
}
