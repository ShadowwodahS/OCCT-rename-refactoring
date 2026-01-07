// Created on: 1995-10-26
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

#include <BRepAlgo_AsDes.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_Type.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_MapOfOrientedShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepAlgo_AsDes, RefObject)

//=================================================================================================

BRepAlgo_AsDes::BRepAlgo_AsDes() {}

//=================================================================================================

void BRepAlgo_AsDes::Add(const TopoShape& S, const TopoShape& SS)
{
  if (!down.IsBound(S))
  {
    ShapeList L;
    down.Bind(S, L);
  }
  down(S).Append(SS);

  if (!up.IsBound(SS))
  {
    ShapeList L;
    up.Bind(SS, L);
  }
  up(SS).Append(S);
}

//=================================================================================================

void BRepAlgo_AsDes::Add(const TopoShape& S, const ShapeList& SS)
{
  TopTools_ListIteratorOfListOfShape it(SS);
  for (; it.More(); it.Next())
  {
    Add(S, it.Value());
  }
}

//=================================================================================================

void BRepAlgo_AsDes::Clear()
{
  up.Clear();
  down.Clear();
}

//=================================================================================================

Standard_Boolean BRepAlgo_AsDes::HasAscendant(const TopoShape& S) const
{
  return up.IsBound(S);
}

//=================================================================================================

Standard_Boolean BRepAlgo_AsDes::HasDescendant(const TopoShape& S) const
{
  return down.IsBound(S);
}

//=================================================================================================

const ShapeList& BRepAlgo_AsDes::Ascendant(const TopoShape& S) const
{
  if (up.IsBound(S))
    return up(S);
  static ShapeList empty;
  return empty;
}

//=================================================================================================

const ShapeList& BRepAlgo_AsDes::Descendant(const TopoShape& S) const
{
  if (down.IsBound(S))
    return down(S);
  static ShapeList empty;
  return empty;
}

//=================================================================================================

ShapeList& BRepAlgo_AsDes::ChangeDescendant(const TopoShape& S)
{
  if (down.IsBound(S))
    return down.ChangeFind(S);
  static ShapeList empty;
  return empty;
}

//=================================================================================================

static void ReplaceInList(const TopoShape&   OldS,
                          const TopoShape&   NewS,
                          ShapeList& L)
{
  TopTools_MapOfOrientedShape        aMS;
  TopTools_ListIteratorOfListOfShape it(L);
  for (; it.More(); it.Next())
  {
    aMS.Add(it.Value());
  }
  it.Initialize(L);
  while (it.More())
  {
    if (it.Value().IsSame(OldS))
    {
      TopAbs_Orientation O = it.Value().Orientation();
      if (aMS.Add(NewS.Oriented(O)))
      {
        L.InsertBefore(NewS.Oriented(O), it);
      }
      L.Remove(it);
    }
    else
      it.Next();
  }
}

//=================================================================================================

static void RemoveInList(const TopoShape& S, ShapeList& L)
{
  TopTools_ListIteratorOfListOfShape it(L);
  while (it.More())
  {
    if (it.Value().IsSame(S))
    {
      L.Remove(it);
      break;
    }
    it.Next();
  }
}

//=================================================================================================

Standard_Boolean BRepAlgo_AsDes::HasCommonDescendant(const TopoShape&   S1,
                                                     const TopoShape&   S2,
                                                     ShapeList& LC) const
{
  LC.Clear();
  if (HasDescendant(S1) && HasDescendant(S2))
  {
    TopTools_ListIteratorOfListOfShape it1(Descendant(S1));
    for (; it1.More(); it1.Next())
    {
      const TopoShape&                DS1 = it1.Value();
      TopTools_ListIteratorOfListOfShape it2(Ascendant(DS1));
      for (; it2.More(); it2.Next())
      {
        const TopoShape& ADS1 = it2.Value();
        if (ADS1.IsSame(S2))
        {
          LC.Append(DS1);
        }
      }
    }
  }
  return (!LC.IsEmpty());
}

//=================================================================================================

void BRepAlgo_AsDes::BackReplace(const TopoShape&         OldS,
                                 const TopoShape&         NewS,
                                 const ShapeList& L,
                                 const Standard_Boolean      InUp)
{
  TopTools_ListIteratorOfListOfShape it(L);
  for (; it.More(); it.Next())
  {
    const TopoShape& S = it.Value();
    if (InUp)
    {
      if (up.IsBound(S))
      {
        ReplaceInList(OldS, NewS, up.ChangeFind(S));
      }
    }
    else
    {
      if (down.IsBound(S))
      {
        ReplaceInList(OldS, NewS, down.ChangeFind(S));
      }
    }
  }
}

//=================================================================================================

void BRepAlgo_AsDes::Replace(const TopoShape& OldS, const TopoShape& NewS)
{
  for (Standard_Integer i = 0; i < 2; ++i)
  {
    TopTools_DataMapOfShapeListOfShape& aMap   = !i ? up : down;
    ShapeList*               pLSOld = aMap.ChangeSeek(OldS);
    if (!pLSOld)
    {
      continue;
    }
    //
    Standard_Boolean InUp = !i ? Standard_False : Standard_True;
    BackReplace(OldS, NewS, *pLSOld, InUp);
    //
    ShapeList* pLSNew = aMap.ChangeSeek(NewS);
    if (!pLSNew)
    {
      // filter the list
      TopTools_MapOfOrientedShape        aMS;
      TopTools_ListIteratorOfListOfShape aIt(*pLSOld);
      for (; aIt.More();)
      {
        if (!aMS.Add(aIt.Value()))
        {
          pLSOld->Remove(aIt);
        }
        else
        {
          aIt.Next();
        }
      }
      aMap.Bind(NewS, *pLSOld);
    }
    else
    {
      // avoid duplicates
      TopTools_MapOfOrientedShape        aMS;
      TopTools_ListIteratorOfListOfShape aIt(*pLSNew);
      for (; aIt.More(); aIt.Next())
      {
        aMS.Add(aIt.Value());
      }
      //
      aIt.Initialize(*pLSOld);
      for (; aIt.More(); aIt.Next())
      {
        const TopoShape& aS = aIt.Value();
        if (aMS.Add(aS))
        {
          pLSNew->Append(aS);
        }
      }
    }
    //
    aMap.UnBind(OldS);
  }
}

//=================================================================================================

void BRepAlgo_AsDes::Remove(const TopoShape& SS)
{
  if (down.IsBound(SS))
  {
    throw Standard_ConstructionError(" BRepAlgo_AsDes::Remove");
  }
  if (!up.IsBound(SS))
  {
    throw Standard_ConstructionError(" BRepAlgo_AsDes::Remove");
  }
  TopTools_ListIteratorOfListOfShape it(up(SS));
  for (; it.More(); it.Next())
  {
    RemoveInList(SS, down.ChangeFind((it.Value())));
  }
  up.UnBind(SS);
}
