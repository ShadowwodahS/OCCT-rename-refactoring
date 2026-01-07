// Created on: 1995-03-28
// Created by: Yves FRICAUD
// Copyright (c) 1995-1999 Matra Datavision
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

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools_Substitution.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_NoSuchObject.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeListOfShape.hxx>

//=================================================================================================

ShapeSubstitution::ShapeSubstitution() {}

//=================================================================================================

void ShapeSubstitution::Clear()
{
  myMap.Clear();
}

//=================================================================================================

void ShapeSubstitution::Substitute(const TopoShape& OS, const ShapeList& NS)
{
  Standard_ConstructionError_Raise_if(IsCopied(OS), "BRepTools_CutClue::Substitute");
  myMap.Bind(OS, NS);
}

//=================================================================================================

void ShapeSubstitution::Build(const TopoShape& S)
{
  if (IsCopied(S))
    return;

  ShapeBuilder     B;
  TopoDS_Iterator  iteS(S.Oriented(TopAbs_FORWARD));
  Standard_Boolean IsModified  = Standard_False;
  Standard_Boolean HasSubShape = Standard_False;

  //------------------------------------------
  // look S is modified and build subshapes.
  //------------------------------------------
  for (; iteS.More(); iteS.Next())
  {
    const TopoShape& SS = iteS.Value();
    Build(SS);
    if (IsCopied(SS))
    {
      IsModified = Standard_True;
    }
  }

  TopoShape NewS = S.Oriented(TopAbs_FORWARD);
  if (IsModified)
  {
    //----------------------------------------
    // Rebuild S.
    //------------------------------------------
    NewS.EmptyCopy();

    if (NewS.ShapeType() == TopAbs_EDGE)
    {
      Standard_Real f, l;
      BRepInspector::Range(TopoDS::Edge(S), f, l);
      B.Range(TopoDS::Edge(NewS), f, l);
    }

    iteS.Initialize(S.Oriented(TopAbs_FORWARD));
    //------------------------------------------
    // Add the copy of subshapes of S to NewS.
    //------------------------------------------
    for (; iteS.More(); iteS.Next())
    {
      TopAbs_Orientation   OS = iteS.Value().Orientation();
      ShapeList L;
      L = myMap(iteS.Value());
      TopTools_ListIteratorOfListOfShape iteL(L);

      for (; iteL.More(); iteL.Next())
      {
        const TopoShape NSS = iteL.Value();
        //------------------------------------------
        // Rebuild NSS and add its copy to NewS.
        //------------------------------------------
        Build(NSS);

        const ShapeList&        NL    = myMap(NSS);
        TopAbs_Orientation                 NewOr = TopAbs1::Compose(OS, NSS.Orientation());
        TopTools_ListIteratorOfListOfShape iteNL(NL);

        for (; iteNL.More(); iteNL.Next())
        {
          B.Add(NewS, iteNL.Value().Oriented(NewOr));
          HasSubShape = Standard_True;
        }
      }
    }
    if (!HasSubShape)
    {
      if (NewS.ShapeType() == TopAbs_WIRE || NewS.ShapeType() == TopAbs_SHELL
          || NewS.ShapeType() == TopAbs_SOLID || NewS.ShapeType() == TopAbs_COMPOUND)
        //-----------------------------------------------------------------
        // Wire,Solid,Shell,Compound must have subshape else they disappear
        //-----------------------------------------------------------------
        NewS.Nullify();
    }
  }
  ShapeList L;
  //-------------------------------------------------------
  // NewS has the same orientation than S in its ancestors
  // so NewS is bound with orientation FORWARD.
  //-------------------------------------------------------
  if (!NewS.IsNull())
    L.Append(NewS.Oriented(TopAbs_FORWARD));
  Substitute(S, L);
}

//=================================================================================================

Standard_Boolean ShapeSubstitution::IsCopied(const TopoShape& S) const
{
  if (myMap.IsBound(S))
  {
    if (myMap(S).IsEmpty())
      return Standard_True;
    else
      return !S.IsSame(myMap(S).First());
  }
  else
    return Standard_False;
}

//=================================================================================================

const ShapeList& ShapeSubstitution::Copy(const TopoShape& S) const
{
  Standard_NoSuchObject_Raise_if(!IsCopied(S), "ShapeSubstitution::Copy");
  return myMap(S);
}
