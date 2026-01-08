// Created on: 1993-06-24
// Created by: Jean Yves LEBEY
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

#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TopOpeBRepDS_IndexedDataMapOfVertexPoint.hxx>
#include <TopOpeBRepDS_CurveExplorer.hxx>
#include <TopOpeBRepDS_Point.hxx>
#include <TopOpeBRepDS.hxx>

#include <TopOpeBRep_define.hxx>
#include <TopOpeBRepDS_CurvePointInterference.hxx>

Standard_Integer BREP_findPDSamongIDMOVP(const Point1&                       PDS,
                                         const TopOpeBRepDS_IndexedDataMapOfVertexPoint& IDMOVP)
{
  Standard_Integer iIDMOVP = 0;
  Standard_Integer i = 1, n = IDMOVP.Extent();
  for (; i <= n; i++)
  {
    const Point1& PM = IDMOVP.FindFromIndex(i);
    if (PDS.IsEqual(PM))
    {
      iIDMOVP = i;
      break;
    }
  }
  return iIDMOVP;
}

void BREP_makeIDMOVP(const TopoShape& S, TopOpeBRepDS_IndexedDataMapOfVertexPoint& IDMOVP)
{
  ShapeExplorer Ex;
  for (Ex.Init(S, TopAbs_VERTEX); Ex.More(); Ex.Next())
  {
    const TopoVertex& v = TopoDS::Vertex(Ex.Current());
    Point1   PDS(v);
    IDMOVP.Add(v, PDS);
  }
}

void BREP_mergePDS(const Handle(TopOpeBRepDS_HDataStructure)& HDS)
{
  TopOpeBRepDS_DataStructure& BDS = HDS->ChangeDS();
  CurveExplorer  cex(BDS);
  if (!cex.More())
    return;

  TopOpeBRepDS_IndexedDataMapOfVertexPoint Mvp1;
  TopOpeBRepDS_IndexedDataMapOfVertexPoint Mvp2;

  for (; cex.More(); cex.Next())
  {

    const TopOpeBRepDS_Curve&                     c  = cex.Curve();
    const Standard_Integer                        ic = cex.Index();
    TopOpeBRepDS_ListIteratorOfListOfInterference itI;
    itI.Initialize(BDS.ChangeCurveInterferences(ic));
    if (!itI.More())
      continue;

    const TopoFace& f1 = TopoDS::Face(c.Shape1());
#ifdef OCCT_DEBUG
    Standard_Integer if1 =
#endif
      BDS.Shape(f1);
    const TopoFace& f2 = TopoDS::Face(c.Shape2());
#ifdef OCCT_DEBUG
    Standard_Integer if2 =
#endif
      BDS.Shape(f2);

    Mvp1.Clear();
    BREP_makeIDMOVP(f1, Mvp1);
    Mvp2.Clear();
    BREP_makeIDMOVP(f2, Mvp2);

    for (; itI.More(); itI.Next())
    {
      Handle(TopOpeBRepDS_Interference)           ITF = itI.Value();
      Handle(TopOpeBRepDS_CurvePointInterference) CPI =
        Handle(TopOpeBRepDS_CurvePointInterference)::DownCast(ITF);
      if (CPI.IsNull())
        continue;
      TopOpeBRepDS_Kind GK = CPI->GeometryType();
      if (GK != TopOpeBRepDS_POINT)
        continue;
      Standard_Integer GI = CPI->Geometry1();
      //**!
      if (GI > BDS.NbPoints())
        continue;
      //**!
      const Point1& PDS = BDS.Point(GI);

      Standard_Integer  ivp1;
      TopoShape      v1;
      TopOpeBRepDS_Kind k1  = TopOpeBRepDS_UNKNOWN;
      Standard_Integer  iv1 = 0;

      ivp1 = BREP_findPDSamongIDMOVP(PDS, Mvp1);
      if (ivp1)
      {
        v1  = Mvp1.FindKey(ivp1);
        iv1 = BDS.AddShape(v1);
        k1  = TopOpeBRepDS_VERTEX;
      }

      Standard_Integer  ivp2;
      TopoShape      v2;
      TopOpeBRepDS_Kind k2  = TopOpeBRepDS_UNKNOWN;
      Standard_Integer  iv2 = 0;

      ivp2 = BREP_findPDSamongIDMOVP(PDS, Mvp2);
      if (ivp2)
      {
        v2  = Mvp2.FindKey(ivp2);
        iv2 = BDS.AddShape(v2);
        k2  = TopOpeBRepDS_VERTEX;
      }

      if (ivp1 && ivp2)
        BDS.FillShapesSameDomain(v1, v2);

      Standard_Boolean editITF = (ivp1 || ivp2);
      if (editITF)
      {
        if (ivp1)
        {
          CPI->GeometryType(k1);
          CPI->Geometry1(iv1);
        }
        else if (ivp2)
        {
          CPI->GeometryType(k2);
          CPI->Geometry1(iv2);
        }
      }

#ifdef OCCT_DEBUG
      if (editITF)
      {
        if (ivp1 != 0)
        {
          std::cout << TopOpeBRepDS1::SPrint(TopOpeBRepDS_CURVE, ic, "# BREP_mergePDS ", " : ");
          std::cout << TopOpeBRepDS1::SPrint(GK, GI, "", " = ");
          AsciiString1 str(BDS.HasShape(v1) ? "old" : "new ");
          std::cout << TopOpeBRepDS1::SPrint(k1, iv1, str);
          std::cout << TopOpeBRepDS1::SPrint(TopOpeBRepDS1::ShapeToKind(f1.ShapeType()), if1, " de ")
                    << std::endl;
        }
        if (ivp2 != 0)
        {
          std::cout << TopOpeBRepDS1::SPrint(TopOpeBRepDS_CURVE, ic, "# BREP_mergePDS ", " : ");
          std::cout << TopOpeBRepDS1::SPrint(GK, GI, "", " = ");
          AsciiString1 str(BDS.HasShape(v2) ? "old" : "new ");
          std::cout << TopOpeBRepDS1::SPrint(k2, iv2, str);
          std::cout << TopOpeBRepDS1::SPrint(TopOpeBRepDS1::ShapeToKind(f2.ShapeType()), if2, " de ")
                    << std::endl;
        }
      }
#endif

    } // itI.More()
  }
} // BREP_mergePDS
