// Created on: 1997-11-25
// Created by: Jean Yves LEBEY
// Copyright (c) 1997-1999 Matra Datavision
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

#include <TopOpeBRepDS_connex.hxx>

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

static TopTools_DataMapOfShapeListOfShape* GLOBAL_elf1 = NULL; // NYI to CDLize as a tool of DS
static TopTools_DataMapOfShapeListOfShape* GLOBAL_elf2 = NULL; // NYI to CDLize as a tool of DS
static TopTools_DataMapOfShapeListOfShape* GLOBAL_fle  = NULL; // NYI to CDLize as a tool of DS
static ShapeList*               GLOBAL_los  = NULL; // NYI to CDLize as a tool of DS
static Standard_Boolean                    GLOBAL_FDSCNX_prepared = Standard_False;

// modified by NIZNHY-PKV Sun Dec 15 17:41:43 2002 f
//=================================================================================================

void FDSCNX_Close()
{
  if (GLOBAL_elf1)
  {
    delete GLOBAL_elf1;
    GLOBAL_elf1 = NULL;
  }
  //
  if (GLOBAL_elf2)
  {
    delete GLOBAL_elf2;
    GLOBAL_elf2 = NULL;
  }
  //
  if (GLOBAL_fle)
  {
    delete GLOBAL_fle;
    GLOBAL_fle = NULL;
  }
  //
  if (GLOBAL_los)
  {
    delete GLOBAL_los;
    GLOBAL_los = NULL;
  }
  //
  GLOBAL_FDSCNX_prepared = Standard_False;
}

// modified by NIZNHY-PKV Sun Dec 15 17:41:40 2002 t

Standard_EXPORT const ShapeList& FDSCNX_EdgeConnexityShapeIndex(
  const TopoShape&                        E,
  const Handle(TopOpeBRepDS_HDataStructure)& HDS,
  const Standard_Integer                     SI)
{
  if (HDS.IsNull())
    return *GLOBAL_los;
  if (!GLOBAL_FDSCNX_prepared)
    return *GLOBAL_los;
  if (SI != 1 && SI != 2)
    return *GLOBAL_los;
  const TopOpeBRepDS_DataStructure& BDS = HDS->DS();
  TopAbs_ShapeEnum                  t   = E.ShapeType();
  if (t != TopAbs_EDGE)
    return *GLOBAL_los;
  Standard_Boolean has = FDSCNX_HasConnexFace(E, HDS);
  if (!has)
    return *GLOBAL_los;
  Standard_Integer re = BDS.AncestorRank(E);
  if (re == 0)
    return *GLOBAL_los;
  TopTools_DataMapOfShapeListOfShape* pelf = (SI == 1) ? GLOBAL_elf1 : GLOBAL_elf2;
  TopTools_DataMapOfShapeListOfShape& elf  = *pelf;
  const ShapeList&         lof  = elf.Find(E);
  return lof;
}

// S = edge --> liste de faces connexes par S
// S = face --> liste d'edges E de S qui ont au moins une autre face connexe
Standard_EXPORT const ShapeList& FDSCNX_EdgeConnexitySameShape(
  const TopoShape&                        S,
  const Handle(TopOpeBRepDS_HDataStructure)& HDS)
{
  TopAbs_ShapeEnum t = S.ShapeType();
  if (t == TopAbs_EDGE)
  {
    Standard_Integer            si = HDS->DS().AncestorRank(S);
    const ShapeList& lf = FDSCNX_EdgeConnexityShapeIndex(S, HDS, si);
    return lf;
  }
  else if (t == TopAbs_FACE)
  {
    TopTools_DataMapOfShapeListOfShape& fle = *GLOBAL_fle;
    if (fle.IsBound(S))
    {
      const ShapeList& le = fle.Find(S);
      return le;
    }
  }
  return *GLOBAL_los;
}

Standard_EXPORT void FDSCNX_Prepare(const TopoShape& /*S1*/,
                                    const TopoShape& /*S2*/,
                                    const Handle(TopOpeBRepDS_HDataStructure)& HDS)
{
  if (HDS.IsNull())
  {
    GLOBAL_FDSCNX_prepared = Standard_False;
    return;
  }
  const TopOpeBRepDS_DataStructure& BDS = HDS->DS();
  if (GLOBAL_elf1 == NULL)
    GLOBAL_elf1 = (TopTools_DataMapOfShapeListOfShape*)new TopTools_DataMapOfShapeListOfShape();
  if (GLOBAL_elf2 == NULL)
    GLOBAL_elf2 = (TopTools_DataMapOfShapeListOfShape*)new TopTools_DataMapOfShapeListOfShape();
  if (GLOBAL_fle == NULL)
    GLOBAL_fle = (TopTools_DataMapOfShapeListOfShape*)new TopTools_DataMapOfShapeListOfShape();
  if (GLOBAL_los == NULL)
    GLOBAL_los = (ShapeList*)new ShapeList();
  GLOBAL_elf1->Clear();
  GLOBAL_elf2->Clear();
  GLOBAL_fle->Clear();
  GLOBAL_los->Clear();

  Standard_Integer i = 0, n = BDS.NbShapes1();
  for (i = 1; i <= n; i++)
  {
    const TopoShape& f = BDS.Shape(i);
    if (f.ShapeType() != TopAbs_FACE)
      continue;
    Standard_Integer rf = BDS.AncestorRank(f);
    if (rf == 0)
      continue;
    //    BDS.Shape(f);
    TopTools_DataMapOfShapeListOfShape& fle = *GLOBAL_fle;
    TopTools_DataMapOfShapeListOfShape& elf = (rf == 1) ? *GLOBAL_elf1 : *GLOBAL_elf2;
    ShapeExplorer                     exe;
    for (exe.Init(f, TopAbs_EDGE); exe.More(); exe.Next())
    {
      //    for (ShapeExplorer exe(f,TopAbs_EDGE);exe.More();exe.Next()) {
      const TopoShape& e = exe.Current();
      //               BDS.Shape(e);
      //      Standard_Boolean se = BDS.IsSectionEdge(TopoDS::Edge(e)); if (!se) continue;
      Standard_Boolean hs = BDS.HasShape(TopoDS::Edge(e));
      if (!hs)
        continue;

      ShapeList* aListFle = fle.ChangeSeek(f);
      if (aListFle == NULL)
      {
        aListFle = fle.Bound(f, ShapeList());
      }
      aListFle->Append(e);

      ShapeList* aListElf = elf.ChangeSeek(e);
      if (aListElf == NULL)
      {
        aListElf = elf.Bound(e, ShapeList());
      }
      aListElf->Append(f);
    }
  }
  GLOBAL_FDSCNX_prepared = Standard_True;
}

Standard_EXPORT Standard_Boolean
  FDSCNX_HasConnexFace(const TopoShape& S, const Handle(TopOpeBRepDS_HDataStructure)& HDS)
{
  if (HDS.IsNull())
  {
    return Standard_False;
  }

  const TopOpeBRepDS_DataStructure& BDS = HDS->DS();
  const TopAbs_ShapeEnum            t   = S.ShapeType();
  if (t != TopAbs_FACE && t != TopAbs_EDGE)
  {
    return Standard_False;
  }
  const Standard_Integer rs = BDS.AncestorRank(S);
  if (rs == 0)
  {
    return Standard_False;
  }

  TopTools_DataMapOfShapeListOfShape* pelf = (rs == 1) ? GLOBAL_elf1 : GLOBAL_elf2;
  if (pelf == NULL)
  {
    return Standard_False;
  }

  Standard_Boolean has = (t == TopAbs_EDGE ? pelf : GLOBAL_fle)->IsBound(S);
  return has;
}

Standard_EXPORT void FDSCNX_FaceEdgeConnexFaces(const TopoShape&                        F,
                                                const TopoShape&                        E,
                                                const Handle(TopOpeBRepDS_HDataStructure)& HDS,
                                                ShapeList&                      LF)
{
  LF.Clear();
  // verifier que E est une arete de connexite de F
  Standard_Boolean            EofF = Standard_False;
  const ShapeList& loe  = FDSCNX_EdgeConnexitySameShape(F, HDS);
  if (loe.IsEmpty())
  {
    return;
  }
  for (TopTools_ListIteratorOfListOfShape i(loe); i.More(); i.Next())
  {
    const TopoShape& e = i.Value();
    //             HDS->Shape(e);
    Standard_Boolean b = e.IsSame(E);
    if (b)
    {
      EofF = Standard_True;
      break;
    }
  }
  if (!EofF)
  {
    return;
  }

  const ShapeList& lof = FDSCNX_EdgeConnexitySameShape(E, HDS);
  if (lof.IsEmpty())
  {
    return;
  }
  for (TopTools_ListIteratorOfListOfShape it(lof); it.More(); it.Next())
  {
    const TopoShape& f = it.Value();
    Standard_Boolean    b = f.IsSame(F);
    if (!b)
    {
      LF.Append(f);
    }
  }
}

Standard_EXPORT void FDSCNX_DumpIndex(const Handle(TopOpeBRepDS_HDataStructure)& HDS,
                                      const Standard_Integer                     I)
{
  if (HDS.IsNull())
  {
    return;
  }

  const TopOpeBRepDS_DataStructure& BDS = HDS->DS();
  Standard_Integer                  ns  = BDS.NbShapes1();
  if (I < 1 || I > ns)
  {
    return;
  }

  Standard_Integer            i  = I;
  const TopoShape&         s  = BDS.Shape(i);
  TopAbs_ShapeEnum            ts = s.ShapeType();
  const ShapeList& ls = FDSCNX_EdgeConnexitySameShape(s, HDS);
  if (ts == TopAbs_EDGE)
  {
    TopTools_ListIteratorOfListOfShape ils(ls);
    if (!ils.More())
    {
      return;
    }

    for (; ils.More(); ils.Next())
    {
      std::cout << BDS.Shape(ils.Value()) << " ";
    }
  }
  else if (ts == TopAbs_FACE)
  {
    TopTools_ListIteratorOfListOfShape ils(ls);
    if (!ils.More())
    {
      return;
    }

    for (; ils.More(); ils.Next())
    {
      const TopoShape&  e = ils.Value();
      ShapeList lf;
      FDSCNX_FaceEdgeConnexFaces(s, e, HDS, lf);
      TopTools_ListIteratorOfListOfShape ilf(lf);
      if (!ilf.More())
      {
        continue;
      }
      for (; ilf.More(); ilf.Next())
      {
        std::cout << BDS.Shape(ilf.Value()) << " ";
      }
    }
  }
}

Standard_EXPORT void FDSCNX_Dump(const Handle(TopOpeBRepDS_HDataStructure)& HDS,
                                 const Standard_Integer                     I)
{
  if (HDS.IsNull())
  {
    return;
  }

  const TopOpeBRepDS_DataStructure& BDS = HDS->DS();
  Standard_Integer                  ns  = BDS.NbShapes1();
  if (I < 1 || I > ns)
  {
    return;
  }

  Standard_Integer            i  = I;
  const TopoShape&         s  = BDS.Shape(i);
  Standard_Integer            is = BDS.Shape(s);
  TopAbs_ShapeEnum            ts = s.ShapeType();
  const ShapeList& ls = FDSCNX_EdgeConnexitySameShape(s, HDS);
  if (ts == TopAbs_EDGE)
  {
    TopTools_ListIteratorOfListOfShape ils(ls);
    if (!ils.More())
    {
      return;
    }

    std::cout << "clear;";
    for (; ils.More(); ils.Next())
    {
      std::cout << "tsee f " << BDS.Shape(ils.Value()) << ";";
    }
    std::cout << "tsee e " << is << ";### edge " << is << " connexity" << std::endl;
  }
  else if (ts == TopAbs_FACE)
  {
    TopTools_ListIteratorOfListOfShape ils(ls);
    if (!ils.More())
    {
      return;
    }
    for (; ils.More(); ils.Next())
    {
      const TopoShape&  e  = ils.Value();
      Standard_Integer     ie = BDS.Shape(e);
      ShapeList lf;
      FDSCNX_FaceEdgeConnexFaces(s, e, HDS, lf);
      TopTools_ListIteratorOfListOfShape ilf(lf);
      if (!ilf.More())
      {
        continue;
      }

      std::cout << "clear;";
      std::cout << "tsee f " << is << ";";
      for (; ilf.More(); ilf.Next())
      {
        std::cout << "tsee f " << BDS.Shape(ilf.Value()) << ";";
      }
      std::cout << "tsee e " << ie << ";### face " << is << " connexity" << std::endl;
    }
  }
}

Standard_EXPORT void FDSCNX_Dump(const Handle(TopOpeBRepDS_HDataStructure)& HDS)
{
  if (HDS.IsNull())
  {
    return;
  }

  const TopOpeBRepDS_DataStructure& BDS = HDS->DS();
  Standard_Integer                  ns  = BDS.NbShapes1();
  for (Standard_Integer i = 1; i <= ns; i++)
  {
    FDSCNX_Dump(HDS, i);
  }
}
