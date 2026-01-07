// Created on: 1994-08-04
// Created by: Jean Yves LEBEY
// Copyright (c) 1994-1999 Matra Datavision
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

#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <GeomTools_Curve2dSet.hxx>
#include <GeomTools_CurveSet.hxx>
#include <Standard_Stream.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopOpeBRepDS.hxx>
#include <TopOpeBRepDS_CurveExplorer.hxx>
#include <TopOpeBRepDS_define.hxx>
#include <TopOpeBRepDS_Dumper.hxx>
#include <TopOpeBRepDS_HDataStructure.hxx>

//=================================================================================================

TopOpeBRepDS_Dumper::TopOpeBRepDS_Dumper(const Handle(TopOpeBRepDS_HDataStructure)& HDS)
{
  myHDS = HDS;
}

//=================================================================================================

AsciiString1 TopOpeBRepDS_Dumper::SDumpRefOri(const TopOpeBRepDS_Kind K,
                                                         const Standard_Integer  I) const
{
  AsciiString1           SS;
  Standard_Boolean                  fk = Standard_False;
  const TopOpeBRepDS_DataStructure& DS = myHDS->DS();
  if (!TopOpeBRepDS1::IsTopology(K))
    return SS;
  TopAbs_ShapeEnum t = TopOpeBRepDS1::KindToShape(K);
  if (DS.Shape(I, fk).ShapeType() != t)
    return SS;
  const TopoShape& S = myHDS->Shape(I, fk);
  Standard_Integer    r = myHDS->SameDomainReference(S);
  TopOpeBRepDS_Config o = myHDS->SameDomainOrientation(S);
  SS                    = SS + "(" + SPrintShape(r) + "," + TopOpeBRepDS1::SPrint(o) + ")";
  return SS;
}

//=================================================================================================

AsciiString1 TopOpeBRepDS_Dumper::SDumpRefOri(const TopoShape& S) const
{
  AsciiString1 SS;
  TopOpeBRepDS_Kind       k  = TopOpeBRepDS1::ShapeToKind(S.ShapeType());
  Standard_Boolean        fk = Standard_False;
  Standard_Integer        i  = myHDS->Shape(S, fk);
  SS                         = SDumpRefOri(k, i);
  return SS;
}

//=================================================================================================

AsciiString1 TopOpeBRepDS_Dumper::SPrintShape(const Standard_Integer IS) const
{
  AsciiString1           SS;
  const TopOpeBRepDS_DataStructure& BDS = myHDS->DS();
  if (IS < 1 || IS > BDS.NbShapes1())
    return SS;
  SS = SPrintShape(BDS.Shape(IS));
  return SS;
}

//=================================================================================================

AsciiString1 TopOpeBRepDS_Dumper::SPrintShape(const TopoShape& S) const
{
  const TopOpeBRepDS_DataStructure& BDS    = myHDS->DS();
  const Standard_Integer            IS     = myHDS->DS().Shape(S);
  Standard_Integer                  rankIS = BDS.AncestorRank(IS);
  // JR/Hp  AsciiString1 s1,s2;
  Standard_CString s1, s2;
  if (BDS.KeepShape(IS))
  {
    s1 = (Standard_CString)((rankIS == 1) ? "*" : "");
    s2 = (Standard_CString)((rankIS == 2) ? "*" : "");
  }
  else
  {
    s1 = (Standard_CString)((rankIS == 1) ? "~" : "");
    s2 = (Standard_CString)((rankIS == 2) ? "~" : "");
  }
  AsciiString1 sse =
    TopOpeBRepDS1::SPrint(TopOpeBRepDS1::ShapeToKind(S.ShapeType()), IS, s1, s2);
  return sse;
}

//=================================================================================================

AsciiString1 TopOpeBRepDS_Dumper::SPrintShapeRefOri(
  const TopoShape&            S,
  const AsciiString1& astr) const
{
  AsciiString1 SS = astr + SPrintShape(S) + " " + SDumpRefOri(S);
  return SS;
}

//=================================================================================================

AsciiString1 TopOpeBRepDS_Dumper::SPrintShapeRefOri(
  const ShapeList&    L,
  const AsciiString1& astr) const
{
  AsciiString1            SS;
  TopTools_ListIteratorOfListOfShape it(L);
  if (!it.More())
    return SS;
  SS = SS + astr;
  AsciiString1 bst(astr.Length(), ' ');
  for (Standard_Integer il = 0; it.More(); it.Next(), il++)
  {
    AsciiString1 ss = SPrintShapeRefOri(it.Value());
    if (il)
      ss = bst + ss;
    SS = SS + ss + "\n";
  }
  return SS;
}
