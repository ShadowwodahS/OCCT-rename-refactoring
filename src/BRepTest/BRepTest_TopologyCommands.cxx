// Created on: 1993-07-22
// Created by: Remi LEQUETTE
// Copyright (c) 1993-1999 Matra Datavision
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

#include <BRepTest.hxx>
#include <DBRep.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_Appli.hxx>

#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <TopoDS.hxx>
#include <TopOpeBRepBuild_HBuilder.hxx>
#include <TopOpeBRepDS_HDataStructure.hxx>

static Standard_Integer halfspace(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 6)
    return 1;

  // Le point indiquant le cote "matiere".
  Point3d RefPnt = Point3d(Draw1::Atof(a[3]), Draw1::Atof(a[4]), Draw1::Atof(a[5]));

  TopoShape Face = DBRep1::Get(a[2], TopAbs_FACE);
  if (Face.IsNull())
  {
    TopoShape Shell = DBRep1::Get(a[2], TopAbs_SHELL);
    if (Shell.IsNull())
    {
      di << a[2] << " must be a face or a shell\n";
      return 1;
    }
    else
    {
      BRepPrimAPI_MakeHalfSpace Half(TopoDS::Shell(Shell), RefPnt);
      if (Half.IsDone())
      {
        DBRep1::Set(a[1], Half.Solid());
      }
      else
      {
        di << " HalfSpace NotDone\n";
        return 1;
      }
    }
  }
  else
  {
    BRepPrimAPI_MakeHalfSpace Half(TopoDS::Face(Face), RefPnt);
    if (Half.IsDone())
    {
      DBRep1::Set(a[1], Half.Solid());
    }
    else
    {
      di << " HalfSpace NotDone\n";
      return 1;
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer buildfaces(DrawInterpreter&, Standard_Integer narg, const char** a)
{
  if (narg < 4)
    return 1;

  TopoShape            InputShape(DBRep1::Get(a[2], TopAbs_FACE));
  TopoFace             F = TopoDS::Face(InputShape);
  BRepAlgo_FaceRestrictor FR;
  FR.Init(F);

  for (Standard_Integer i = 3; i < narg; i++)
  {
    TopoShape InputWire(DBRep1::Get(a[i], TopAbs_WIRE));
    TopoWire  W = TopoDS::Wire(InputWire);
    FR.Add(W);
  }
  FR.Perform();
  if (!FR.IsDone())
    return 1;

  TopoCompound Res;
  ShapeBuilder    BB;
  BB.MakeCompound(Res);

  for (; FR.More(); FR.Next())
  {
    TopoFace FF = FR.Current();
    BB.Add(Res, FF);
    DBRep1::Set(a[1], Res);
  }
  return 0;
}

//=================================================================================================

void BRepTest::TopologyCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  DBRep1::BasicCommands(theCommands);

  const char* g = "TOPOLOGY Topological operation commands";

  theCommands.Add("halfspace", "halfspace result face/shell x y z", __FILE__, halfspace, g);
  theCommands.Add("buildfaces",
                  "buildfaces result faceReference wire1 wire2 ...",
                  __FILE__,
                  buildfaces,
                  g);
}
