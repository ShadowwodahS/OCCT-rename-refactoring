// Created on: 1996-09-16
// Created by: Jacques GOUSSARD
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
#include <BRep_Tool.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <gp_Pnt.hxx>
#include <LocOpe_BuildShape.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

static void Add(const Standard_Integer,
                TColStd_MapOfInteger&,
                TopTools_IndexedMapOfShape&,
                const TopTools_IndexedDataMapOfShapeListOfShape&);

static void Propagate(const TopoShape&, // face
                      TopoShape&,       // shell
                      const TopTools_IndexedMapOfShape&,
                      TColStd_MapOfInteger&);

static Standard_Boolean IsInside(const TopoShape&, const TopoShape&);

//=================================================================================================

void LocOpe_BuildShape::Perform(const ShapeList& L)
{
  Standard_Integer i;
  Standard_Integer j;
  Standard_Integer k;
  myRes.Nullify();

  TopoCompound C;
  ShapeBuilder    B;
  B.MakeCompound(C);

  TopTools_IndexedMapOfShape         mapF;
  TopTools_ListIteratorOfListOfShape itl;

  for (itl.Initialize(L); itl.More(); itl.Next())
  {
    if (itl.Value().ShapeType() == TopAbs_FACE && !mapF.Contains(itl.Value()))
    {
      mapF.Add(itl.Value());
      B.Add(C, itl.Value());
    }
  }

  if (mapF.Extent() == 0)
  {
    return; // no face
  }

  TopTools_IndexedDataMapOfShapeListOfShape theMapEF;
  TopExp1::MapShapesAndAncestors(C, TopAbs_EDGE, TopAbs_FACE, theMapEF);

  TopTools_DataMapOfShapeListOfShape mapSh;
  TColStd_MapOfInteger               mapI, mapIf;
  Standard_Integer                   Nbedges = theMapEF.Extent();

  ShapeList lshell;
  ShapeList lresult;

  do
  {
    // Recherche du premier edge non traite
    for (i = 1; i <= Nbedges; i++)
    {
      if (!mapI.Contains(i))
      {
        break;
      }
    }
    if (i <= Nbedges)
    {
      mapF.Clear();
      mapIf.Clear();
      Add(i, mapI, mapF, theMapEF);
      Standard_Boolean   Manifold = Standard_True;
      TopoShape       FaceRef;
      TopAbs_Orientation orient;

      for (j = 1; j <= mapF.Extent(); j++)
      {
        orient = mapF(j).Orientation();
        if (orient == TopAbs_INTERNAL || orient == TopAbs_EXTERNAL)
        {
          Manifold = Standard_False;
        }
        else if (FaceRef.IsNull())
        {
          FaceRef = mapF(j);
        }
        mapIf.Add(j);
      }

      TopoShell newSh;
      B.MakeShell(newSh);
      if (!Manifold && FaceRef.IsNull())
      {
        // on a un paquet de faces. pas d'orientation possible ?
        for (j = 1; j <= mapF.Extent(); j++)
        {
          B.Add(newSh, mapF(j));
        }
      }
      else
      {
        // orienter ce qu`on peut
        if (!Manifold)
        {
          for (j = 1; j <= mapF.Extent(); j++)
          {
            if (mapF(j).Orientation() == TopAbs_INTERNAL
                || mapF(j).Orientation() == TopAbs_EXTERNAL)
            {
              B.Add(newSh, mapF(j));
              mapIf.Remove(j);
            }
          }
        }

        B.Add(newSh, FaceRef);
        Propagate(FaceRef, newSh, mapF, mapIf);
      }
      newSh.Closed(BRepInspector::IsClosed(newSh));
      if (!Manifold)
      {
        lshell.Append(newSh.Oriented(TopAbs_INTERNAL));
      }
      else
      {
        TopTools_IndexedDataMapOfShapeListOfShape theMapEFbis;
        TopExp1::MapShapesAndAncestors(newSh, TopAbs_EDGE, TopAbs_FACE, theMapEFbis);
        for (k = 1; k <= theMapEFbis.Extent(); k++)
        {
          const TopoEdge& Ed    = TopoDS::Edge(theMapEFbis.FindKey(k));
          TopAbs_Orientation OriEd = Ed.Orientation();
          if (OriEd != TopAbs_INTERNAL && OriEd != TopAbs_EXTERNAL)
          {
            Standard_Integer Nbfac = theMapEFbis(k).Extent();
            if (Nbfac > 2)
            { // peu probable
              break;
            }
            else if (Nbfac == 1)
            {
              if (!BRepInspector::Degenerated(Ed))
              {
                break;
              }
            }
          }
        }
        if (k > theMapEFbis.Extent())
        {
          TopoSolid newSo;
          B.MakeSolid(newSo);
          B.Add(newSo, newSh); // newSh est FORWARD
          BRepClass3d_SolidClassifier Class(newSo);
          Class.PerformInfinitePoint(Precision::Confusion());
          if (Class.State() == TopAbs_IN)
          {
            lresult.Append(newSh.Oriented(TopAbs_REVERSED));
          }
          else
          {
            lresult.Append(newSh);
          }
        }
        else
        {
          lshell.Append(newSh.Oriented(TopAbs_INTERNAL));
        }
      }
    }
  } while (mapI.Extent() < Nbedges);

  // on a une list de shells dans lresult. on suppose qu`ils ne s`intersectent pas.
  // il faut classifier les shells orientes pour en faire des solides...
  // on n`accepte qu`1 niveau d'imbrication

  TopTools_DataMapOfShapeListOfShape imbSh;
  ShapeList               LIntern;

  for (itl.Initialize(lresult); itl.More(); itl.Next())
  {
    const TopoShape& sh = itl.Value();
    TopoSolid        tempo;
    B.MakeSolid(tempo);
    B.Add(tempo, sh);

    ShapeList thelist;
    imbSh.Bind(sh, thelist);
    TopTools_ListIteratorOfListOfShape itl2;
    for (itl2.Initialize(lresult);
         //    for (TopTools_ListIteratorOfListOfShape itl2(lresult);
         itl2.More();
         itl2.Next())
    {
      const TopoShape& sh2 = itl2.Value();
      if (!sh2.IsSame(sh))
      {
        if (IsInside(sh2, tempo))
        {
          LIntern.Append(sh2);
          imbSh(sh).Append(sh2);
        }
      }
    }
  }

  // LPA 07/10/98: on vire les shells imbriques comme
  // etant aussi des solides a part entiere.
  for (itl.Initialize(LIntern); itl.More(); itl.Next())
  {
    const TopoShape& sh = itl.Value();
    if (imbSh.IsBound(sh))
    {
      imbSh.UnBind(sh);
    }
  }

  ShapeList lsolid;
  do
  {
    //    for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itdm(imbSh);
    TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itdm(imbSh);
    for (; itdm.More(); itdm.Next())
    {
      if (itdm.Value().Extent() != 0)
      {
        break;
      }
    }
    if (itdm.More())
    {
      TopoSolid newSo;
      B.MakeSolid(newSo);
      B.Add(newSo, itdm.Key1());
      for (itl.Initialize(itdm.Value()); itl.More(); itl.Next())
      {
        B.Add(newSo, itl.Value().Reversed());
      }
      lsolid.Append(newSo);
      imbSh.UnBind(itdm.Key1());
    }
    else
    {
      for (itdm.Initialize(imbSh); itdm.More(); itdm.Next())
      {
        TopoSolid newSo;
        B.MakeSolid(newSo);
        B.Add(newSo, itdm.Key1());
        lsolid.Append(newSo);
      }
      imbSh.Clear();
    }
  } while (imbSh.Extent() != 0);

  Standard_Integer nbsol = lsolid.Extent();
  Standard_Integer nbshl = lshell.Extent();

  if (nbsol == 1 && nbshl == 0)
  {
    myRes = lsolid.First();
  }
  else if (nbsol == 0 && nbshl == 1)
  {
    myRes = lshell.First();
  }
  else
  {
    B.MakeCompound(TopoDS::Compound(myRes));
    for (itl.Initialize(lsolid); itl.More(); itl.Next())
    {
      B.Add(myRes, itl.Value());
    }
    for (itl.Initialize(lshell); itl.More(); itl.Next())
    {
      B.Add(myRes, itl.Value());
    }
  }
}

//=======================================================================
// function : Add
// purpose  : static function
//=======================================================================

static void Add(const Standard_Integer                           ind,
                TColStd_MapOfInteger&                            mapI,
                TopTools_IndexedMapOfShape&                      mapF,
                const TopTools_IndexedDataMapOfShapeListOfShape& mapEF)

{
  if (!mapI.Add(ind))
  {
    throw Standard_ConstructionError();
  }

  TopTools_ListIteratorOfListOfShape itl(mapEF(ind));
  for (; itl.More(); itl.Next())
  {
    if (!mapF.Contains(itl.Value()))
    {
      mapF.Add(itl.Value());
      ShapeExplorer exp;
      for (exp.Init(itl.Value(), TopAbs_EDGE);
           //      for (ShapeExplorer exp(itl.Value(),TopAbs_EDGE);
           exp.More();
           exp.Next())
      {
        const TopoShape& edg    = exp.Current();
        Standard_Integer    indedg = mapEF.FindIndex(edg);
        if (indedg == 0)
        {
          throw Standard_ConstructionError();
        }
        if (!mapI.Contains(indedg))
        {
          Add(indedg, mapI, mapF, mapEF);
        }
      }
    }
  }
}

//=======================================================================
// function : Propagate
// purpose  : static function
//=======================================================================

static void Propagate(const TopoShape&               F,
                      TopoShape&                     Sh,
                      const TopTools_IndexedMapOfShape& mapF,
                      TColStd_MapOfInteger&             mapIf)
{
  ShapeBuilder     B;
  Standard_Integer indf = mapF.FindIndex(F);
  if (!mapIf.Contains(indf))
  {
    return;
  }
  mapIf.Remove(indf);
  if (mapIf.Extent() == 0)
  {
    return;
  }

  ShapeExplorer exp;
  for (exp.Init(F, TopAbs_EDGE); exp.More(); exp.Next())
  {
    //  for (ShapeExplorer exp(F,TopAbs_EDGE); exp.More(); exp.Next()) {
    const TopoShape& edg = exp.Current();

    TopAbs_Orientation ored1 = edg.Orientation(), ored2 = TopAbs_FORWARD;

    if (ored1 == TopAbs_INTERNAL || ored1 == TopAbs_EXTERNAL)
    {
      continue;
    }
    //    for (TColStd_MapIteratorOfMapOfInteger itm(mapIf);
    TColStd_MapIteratorOfMapOfInteger itm(mapIf);
    for (; itm.More(); itm.Next())
    {
      const TopoShape& newF = mapF(itm.Key1());
      //      for (ShapeExplorer exp2(newF,TopAbs_EDGE);exp2.More(); exp2.Next()) {
      ShapeExplorer exp2(newF, TopAbs_EDGE);
      for (; exp2.More(); exp2.Next())
      {
        if (exp2.Current().IsSame(edg))
        {
          break;
        }
      }
      if (exp2.More())
      {
        ored2 = exp2.Current().Orientation();
        break;
      }
    }
    if (itm.More())
    {
      TopoShape     FtoAdd = mapF(itm.Key1());
      Standard_Boolean added  = Standard_False;
      if (ored2 == ored1)
      {
        FtoAdd.Reverse();
        B.Add(Sh, FtoAdd);
        added = Standard_True;
      }
      else if (ored2 == TopAbs1::Reverse(ored1))
      {
        B.Add(Sh, FtoAdd);
        added = Standard_True;
      }
      if (added)
      {
        Propagate(FtoAdd, Sh, mapF, mapIf);
      }
    }
  }
}

//=======================================================================
// function : IsInside
// purpose  : static function
//=======================================================================

static Standard_Boolean IsInside(const TopoShape& S1, const TopoShape& S2)
{
  BRepClass3d_SolidClassifier Class(S2);
  ShapeExplorer             exp;
  for (exp.Init(S1, TopAbs_VERTEX); exp.More(); exp.Next())
  {
    //  for (ShapeExplorer exp(S1,TopAbs_VERTEX);exp.More(); exp.Next()) {
    const TopoVertex& vtx    = TopoDS::Vertex(exp.Current());
    Point3d               Pttest = BRepInspector::Pnt(vtx);
    Standard_Real        Tol    = BRepInspector::Tolerance(vtx);
    Class.Perform(Pttest, Tol);
    if (Class.State() == TopAbs_IN)
    {
      return Standard_True;
    }
    else if (Class.State() == TopAbs_OUT)
    {
      return Standard_False;
    }
  }
#ifdef OCCT_DEBUG
  std::cout << "Classification impossible sur vertex " << std::endl;
#endif

  return Standard_True;
}
