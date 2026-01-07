// Created on: 1993-10-14
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

// Modified by skv - Fri Mar  4 15:50:09 2005
// Add methods for supporting history.

#include <BRepLib.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepSweep_Prism.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Shape.hxx>

// perform checks on the argument
static const TopoShape& check(const TopoShape& S)
{
  BRepLib::BuildCurves3d(S);
  return S;
}

//=================================================================================================

BRepPrimAPI_MakePrism::BRepPrimAPI_MakePrism(const TopoShape&    S,
                                             const Vector3d&          V,
                                             const Standard_Boolean Copy,
                                             const Standard_Boolean Canonize)
    : myPrism(check(S), V, Copy, Canonize)
{
  Build();
}

//=================================================================================================

BRepPrimAPI_MakePrism::BRepPrimAPI_MakePrism(const TopoShape&    S,
                                             const Dir3d&          D,
                                             const Standard_Boolean Inf,
                                             const Standard_Boolean Copy,
                                             const Standard_Boolean Canonize)
    : myPrism(check(S), D, Inf, Copy, Canonize)
{
  Build();
}

//=================================================================================================

const BRepSweep_Prism& BRepPrimAPI_MakePrism::Prism() const
{
  return myPrism;
}

//=================================================================================================

void BRepPrimAPI_MakePrism::Build(const Message_ProgressRange& /*theRange*/)
{
  myShape = myPrism.Shape();
  Done();
}

//=================================================================================================

TopoShape BRepPrimAPI_MakePrism::FirstShape()
{
  return myPrism.FirstShape();
}

//=================================================================================================

TopoShape BRepPrimAPI_MakePrism::LastShape()
{
  return myPrism.LastShape();
}

//=================================================================================================

const ShapeList& BRepPrimAPI_MakePrism::Generated(const TopoShape& S)
{
  myGenerated.Clear();
  if (myPrism.IsUsed(S) && myPrism.GenIsUsed(S))
  {
    myGenerated.Append(myPrism.Shape(S));
  }
  return myGenerated;
}

// Modified by skv - Fri Mar  4 15:50:09 2005 Begin

//=======================================================================
// function : FirstShape
// purpose  : This method returns the bottom shape of the prism, generated
//           with theShape (subShape of the generating shape)
//=======================================================================

TopoShape BRepPrimAPI_MakePrism::FirstShape(const TopoShape& theShape)
{
  return myPrism.FirstShape(theShape);
}

//=======================================================================
// function : LastShape
// purpose  : This method returns the top shape of the prism, generated
//           with theShape (subShape of the generating shape)
//=======================================================================

TopoShape BRepPrimAPI_MakePrism::LastShape(const TopoShape& theShape)
{
  return myPrism.LastShape(theShape);
}

// Modified by skv - Fri Mar  4 15:50:09 2005 End

//=================================================================================================

Standard_Boolean BRepPrimAPI_MakePrism::IsDeleted(const TopoShape& S)
{
  return !myPrism.IsUsed(S);
}
