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

#include <BRepAlgo_Image.hxx>
#include <Standard_ConstructionError.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

//=================================================================================================

ShapeImage::ShapeImage() {}

//=================================================================================================

void ShapeImage::SetRoot(const TopoShape& S)
{
  roots.Append(S);
}

//=================================================================================================

void ShapeImage::Bind(const TopoShape& OldS, const TopoShape& NewS)
{
  if (down.IsBound(OldS))
  {
    throw Standard_ConstructionError(" ShapeImage::Bind");
    return;
  }
  ShapeList L;
  down.Bind(OldS, L);
  down(OldS).Append(NewS);
  up.Bind(NewS, OldS);
}

//=================================================================================================

void ShapeImage::Bind(const TopoShape& OldS, const ShapeList& L)
{
  if (HasImage(OldS))
  {
    throw Standard_ConstructionError(" ShapeImage::Bind");
    return;
  }
  TopTools_ListIteratorOfListOfShape it(L);
  for (; it.More(); it.Next())
  {
    if (!HasImage(OldS))
      Bind(OldS, it.Value());
    else
      Add(OldS, it.Value());
  }
}

//=================================================================================================

void ShapeImage::Clear()
{
  roots.Clear();
  up.Clear();
  down.Clear();
}

//=================================================================================================

void ShapeImage::Add(const TopoShape& OldS, const TopoShape& NewS)
{
  if (!HasImage(OldS))
  {
    throw Standard_ConstructionError(" ShapeImage::Add");
  }
  down(OldS).Append(NewS);
  up.Bind(NewS, OldS);
}

//=================================================================================================

void ShapeImage::Add(const TopoShape& OldS, const ShapeList& L)
{
  TopTools_ListIteratorOfListOfShape it(L);
  for (; it.More(); it.Next())
  {
    Add(OldS, it.Value());
  }
}

//=================================================================================================

void ShapeImage::Remove(const TopoShape& S)
{
  if (!up.IsBound(S))
  {
    throw Standard_ConstructionError(" ShapeImage::Remove");
  }
  const TopoShape&                OldS = up(S);
  ShapeList&              L    = down(OldS);
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
  if (L.IsEmpty())
    down.UnBind(OldS);
  up.UnBind(S);
}

//=======================================================================
// function : ShapeList&
// purpose  :
//=======================================================================

const ShapeList& ShapeImage::Roots() const
{
  return roots;
}

//=================================================================================================

Standard_Boolean ShapeImage::IsImage(const TopoShape& S) const
{
  return up.IsBound(S);
}

//=================================================================================================

const TopoShape& ShapeImage::ImageFrom(const TopoShape& S) const
{
  if (!up.IsBound(S))
  {
    throw Standard_ConstructionError(" ShapeImage::ImageFrom");
  }
  return up(S);
}

//=================================================================================================

const TopoShape& ShapeImage::Root(const TopoShape& S) const
{
  if (!up.IsBound(S))
  {
    throw Standard_ConstructionError(" ShapeImage::FirstImageFrom");
  }

  TopoShape S1 = up(S);
  TopoShape S2 = S;

  if (S1.IsSame(S2))
    return up(S);

  while (up.IsBound(S1))
  {
    S2 = S1;
    S1 = up(S1);
    if (S1.IsSame(S2))
      break;
  }
  return up(S2);
}

//=================================================================================================

Standard_Boolean ShapeImage::HasImage(const TopoShape& S) const
{
  return down.IsBound(S);
}

//=======================================================================
// function : ShapeList&
// purpose  :
//=======================================================================

const ShapeList& ShapeImage::Image(const TopoShape& S) const
{
  if (!HasImage(S))
  {
    static ShapeList L;
    L.Append(S);
    return L;
  }
  return down(S);
}

//=======================================================================
// function : ShapeList&
// purpose  :
//=======================================================================
void ShapeImage::LastImage(const TopoShape& S, ShapeList& L) const
{
  if (!down.IsBound(S))
  {
    L.Append(S);
  }
  else
  {
    TopTools_ListIteratorOfListOfShape it(down(S));
    for (; it.More(); it.Next())
    {
      if (it.Value().IsSame(S))
      {
        L.Append(S);
      }
      else
      {
        LastImage(it.Value(), L);
      }
    }
  }
}

//=================================================================================================

void ShapeImage::Compact()
{
  TopTools_DataMapOfShapeListOfShape M;
  TopTools_ListIteratorOfListOfShape it(roots);
  for (; it.More(); it.Next())
  {
    const TopoShape&  S = it.Value();
    ShapeList LI;
    if (HasImage(S))
      LastImage(S, LI);
    M.Bind(S, LI);
  }
  up.Clear();
  down.Clear();
  for (it.Initialize(roots); it.More(); it.Next())
  {
    if (M.IsBound(it.Value()))
    {
      Bind(it.Value(), M(it.Value()));
    }
  }
}

//=================================================================================================

void ShapeImage::Filter(const TopoShape& S, const TopAbs_ShapeEnum T)

{
  ShapeExplorer     exp(S, T);
  TopTools_MapOfShape M;
  for (; exp.More(); exp.Next())
  {
    M.Add(exp.Current());
  }
  Standard_Boolean Change = Standard_True;
  while (Change)
  {
    Change = Standard_False;
    TopTools_DataMapIteratorOfDataMapOfShapeShape mit(up);
    for (; mit.More(); mit.Next())
    {
      const TopoShape& aS = mit.Key1();
      if (aS.ShapeType() == T && !M.Contains(aS))
      {
        Remove(aS);
        Change = Standard_True;
        break;
      }
    }
  }
}

//=================================================================================================

void ShapeImage::RemoveRoot(const TopoShape& Root)
{
  Standard_Boolean isRemoved = Standard_False;
  for (ShapeList::Iterator it(roots); it.More(); it.Next())
  {
    if (Root.IsSame(it.Value()))
    {
      roots.Remove(it);
      isRemoved = Standard_True;
      break;
    }
  }

  if (!isRemoved)
    return;

  const ShapeList* pNewS = down.Seek(Root);
  if (pNewS)
  {
    for (ShapeList::Iterator it(*pNewS); it.More(); it.Next())
    {
      const TopoShape* pOldS = up.Seek(it.Value());
      if (pOldS && pOldS->IsSame(Root))
        up.UnBind(it.Value());
    }
    down.UnBind(Root);
  }
}

//=================================================================================================

void ShapeImage::ReplaceRoot(const TopoShape& OldRoot, const TopoShape& NewRoot)
{
  if (!HasImage(OldRoot))
    return;

  const ShapeList& aLImage = Image(OldRoot);
  if (HasImage(NewRoot))
    Add(NewRoot, aLImage);
  else
    Bind(NewRoot, aLImage);

  SetRoot(NewRoot);
  RemoveRoot(OldRoot);
}
