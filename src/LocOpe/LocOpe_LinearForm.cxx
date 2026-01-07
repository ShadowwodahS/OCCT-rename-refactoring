// Created on: 1996-09-04
// Created by: Olga PILLOT
// Copyright (c) 1996-1999 Matra Datavision
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
#include <BRepSweep_Prism.hxx>
#include <BRepTools_Modifier.hxx>
#include <BRepTools_TrsfModification.hxx>
#include <Geom_Line.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <LocOpe_BuildShape.hxx>
#include <LocOpe_LinearForm.hxx>
#include <StdFail_NotDone.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

//=================================================================================================

void LocOpe_LinearForm::Perform(const TopoShape& Base,
                                const Vector3d&       V,
                                const Point3d&       Pnt1,
                                const Point3d&       Pnt2)

{
  myIsTrans = Standard_False;
  myMap.Clear();
  myFirstShape.Nullify();
  myLastShape.Nullify();
  myBase.Nullify();
  myRes.Nullify();

  myBase = Base;
  myVec  = V;

  // myEdge = E;
  myPnt1 = Pnt1;
  myPnt2 = Pnt2;

  IntPerf();
}

//=================================================================================================

void LocOpe_LinearForm::Perform(const TopoShape& Base,
                                const Vector3d&       V,
                                const Vector3d&       Vectra,
                                const Point3d&       Pnt1,
                                const Point3d&       Pnt2)

{
  myIsTrans = Standard_True;
  myTra     = Vectra;
  myMap.Clear();
  myFirstShape.Nullify();
  myLastShape.Nullify();
  myBase.Nullify();
  myRes.Nullify();

  myBase = Base;
  myVec  = V;

  // myEdge = E;
  myPnt1 = Pnt1;
  myPnt2 = Pnt2;

  IntPerf();
}

//=================================================================================================

void LocOpe_LinearForm::IntPerf()
{
  TopoShape       theBase = myBase;
  ShapeModifier Modif;

  if (myIsTrans)
  {
    Transform3d T;
    T.SetTranslation(myTra);
    Handle(BRepTools_TrsfModification) modbase = new BRepTools_TrsfModification(T);
    Modif.Init(theBase);
    Modif.Perform(modbase);
    theBase = Modif.ModifiedShape(theBase);
  }

  BRepSweep_Prism myPrism(theBase, myVec);

  myFirstShape = myPrism.FirstShape();
  myLastShape  = myPrism.LastShape();

  ShapeExplorer exp;
  if (theBase.ShapeType() == TopAbs_FACE)
  {
    for (exp.Init(theBase, TopAbs_EDGE); exp.More(); exp.Next())
    {
      const TopoEdge& edg = TopoDS::Edge(exp.Current());
      if (!myMap.IsBound(edg))
      {
        ShapeList thelist;
        myMap.Bind(edg, thelist);
        TopoShape desc = myPrism.Shape(edg);
        if (!desc.IsNull())
        {
          myMap(edg).Append(desc);
        }
      }
    }
    myRes = myPrism.Shape();
  }

  else
  {
    // Cas base != FACE
    TopTools_IndexedDataMapOfShapeListOfShape theEFMap;
    TopExp1::MapShapesAndAncestors(theBase, TopAbs_EDGE, TopAbs_FACE, theEFMap);
    ShapeList lfaces;
    Standard_Boolean     toremove = Standard_False;
    for (Standard_Integer i = 1; i <= theEFMap.Extent(); i++)
    {
      const TopoShape&  edg = theEFMap.FindKey(i);
      ShapeList thelist1;
      myMap.Bind(edg, thelist1);
      TopoShape desc = myPrism.Shape(edg);
      if (!desc.IsNull())
      {
        if (theEFMap(i).Extent() >= 2)
        {
          toremove = Standard_True;
        }
        else
        {
          myMap(edg).Append(desc);
          lfaces.Append(desc);
        }
      }
    }
    if (toremove)
    {
      // Rajouter les faces de FirstShape et LastShape
      for (exp.Init(myFirstShape, TopAbs_FACE); exp.More(); exp.Next())
      {
        lfaces.Append(exp.Current());
      }
      for (exp.Init(myLastShape, TopAbs_FACE); exp.More(); exp.Next())
      {
        lfaces.Append(exp.Current());
      }

      LocOpe_BuildShape BS(lfaces);
      myRes = BS.Shape();
    }
    else
    {
      for (exp.Init(theBase, TopAbs_EDGE); exp.More(); exp.Next())
      {
        const TopoEdge& edg = TopoDS::Edge(exp.Current());
        if (!myMap.IsBound(edg))
        {
          ShapeList thelist2;
          myMap.Bind(edg, thelist2);
          TopoShape desc = myPrism.Shape(edg);
          if (!desc.IsNull())
          {
            myMap(edg).Append(desc);
          }
        }
      }
      myRes = myPrism.Shape();
    }
  }

  if (myIsTrans)
  {
    // m-a-j des descendants
    ShapeExplorer anExp;
    for (anExp.Init(myBase, TopAbs_EDGE); anExp.More(); anExp.Next())
    {
      const TopoEdge& edg    = TopoDS::Edge(anExp.Current());
      const TopoEdge& edgbis = TopoDS::Edge(Modif.ModifiedShape(edg));
      if (!edgbis.IsSame(edg) && myMap.IsBound(edgbis))
      {
        myMap.Bind(edg, myMap(edgbis));
        myMap.UnBind(edgbis);
      }
    }
  }

  myDone = Standard_True;
}

//=================================================================================================

const TopoShape& LocOpe_LinearForm::Shape() const
{
  if (!myDone)
  {
    throw StdFail_NotDone();
  }
  return myRes;
}

//=================================================================================================

const TopoShape& LocOpe_LinearForm::FirstShape() const
{
  return myFirstShape;
}

//=================================================================================================

const TopoShape& LocOpe_LinearForm::LastShape() const
{
  return myLastShape;
}

//=================================================================================================

const ShapeList& LocOpe_LinearForm::Shapes(const TopoShape& S) const
{
  return myMap(S);
}
