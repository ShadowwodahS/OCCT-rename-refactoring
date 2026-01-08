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

// szv#4 S4163

#include <BRep_Builder.hxx>
#include <ShapeExtend_Explorer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

//=================================================================================================

Explorer::Explorer() {}

//=================================================================================================

TopoShape Explorer::CompoundFromSeq(
  const Handle(TopTools_HSequenceOfShape)& seqval) const
{
  ShapeBuilder    B;
  TopoCompound C;
  B.MakeCompound(C);
  Standard_Integer i, n = seqval->Length();
  for (i = 1; i <= n; i++)
    B.Add(C, seqval->Value(i));
  return C;
}

//=================================================================================================

static void FillList(const Handle(TopTools_HSequenceOfShape)& list,
                     const TopoShape&                      comp,
                     const Standard_Boolean                   expcomp)
{
  for (TopoDS_Iterator it(comp); it.More(); it.Next())
  {
    const TopoShape& sub = it.Value();
    if (sub.ShapeType() != TopAbs_COMPOUND)
      list->Append(sub);
    else if (!expcomp)
      list->Append(sub);
    else
      FillList(list, sub, expcomp);
  }
}

Handle(TopTools_HSequenceOfShape) Explorer::SeqFromCompound(
  const TopoShape&    comp,
  const Standard_Boolean expcomp) const
{
  Handle(TopTools_HSequenceOfShape) list = new TopTools_HSequenceOfShape();
  if (comp.IsNull())
    return list;
  if (comp.ShapeType() != TopAbs_COMPOUND)
  {
    list->Append(comp);
    return list;
  }
  FillList(list, comp, expcomp);
  return list;
}

//=================================================================================================

void Explorer::ListFromSeq(const Handle(TopTools_HSequenceOfShape)& seqval,
                                       ShapeList&                    lisval,
                                       const Standard_Boolean                   clear) const
{
  if (clear)
    lisval.Clear();
  if (seqval.IsNull())
    return;
  Standard_Integer i, nb = seqval->Length();
  for (i = 1; i <= nb; i++)
    lisval.Append(seqval->Value(i));
}

//=================================================================================================

Handle(TopTools_HSequenceOfShape) Explorer::SeqFromList(
  const ShapeList& lisval) const
{
  Handle(TopTools_HSequenceOfShape)  seqval = new TopTools_HSequenceOfShape();
  TopTools_ListIteratorOfListOfShape it;
  for (it.Initialize(lisval); it.More(); it.Next())
    seqval->Append(it.Value());
  return seqval;
}

//=================================================================================================

TopAbs_ShapeEnum Explorer::ShapeType(const TopoShape&    shape,
                                                 const Standard_Boolean compound) const
{
  if (shape.IsNull())
    return TopAbs_SHAPE;
  TopAbs_ShapeEnum res = shape.ShapeType();
  if (!compound || res != TopAbs_COMPOUND)
    return res;
  res = TopAbs_SHAPE;
  for (TopoDS_Iterator iter(shape); iter.More(); iter.Next())
  {
    const TopoShape& sh = iter.Value();
    if (sh.IsNull())
      continue;
    TopAbs_ShapeEnum typ = sh.ShapeType();
    if (typ == TopAbs_COMPOUND)
      typ = ShapeType(sh, compound);
    if (res == TopAbs_SHAPE)
      res = typ;
    //   Egalite : OK;  Pseudo-Egalite : EDGE/WIRE ou FACE/SHELL
    else if (res == TopAbs_EDGE && typ == TopAbs_WIRE)
      res = typ;
    else if (res == TopAbs_WIRE && typ == TopAbs_EDGE)
      continue;
    else if (res == TopAbs_FACE && typ == TopAbs_SHELL)
      res = typ;
    else if (res == TopAbs_SHELL && typ == TopAbs_FACE)
      continue;
    else if (res != typ)
      return TopAbs_COMPOUND;
  }
  return res;
}

//=================================================================================================

TopoShape Explorer::SortedCompound(const TopoShape&    shape,
                                                  const TopAbs_ShapeEnum type,
                                                  const Standard_Boolean explore,
                                                  const Standard_Boolean compound) const
{
  if (shape.IsNull())
    return shape;
  TopAbs_ShapeEnum typ = shape.ShapeType();
  TopoShape     sh, sh0;
  Standard_Integer nb = 0;

  //  Compound : on le prend, soit tel quel, soit son contenu
  if (typ == TopAbs_COMPOUND || typ == TopAbs_COMPSOLID)
  {
    TopoCompound C;
    ShapeBuilder    B;
    B.MakeCompound(C);
    for (TopoDS_Iterator it(shape); it.More(); it.Next())
    {
      sh0 = SortedCompound(it.Value(), type, explore, compound);
      if (sh0.IsNull())
        continue;
      sh  = sh0;
      typ = sh.ShapeType();
      if (typ == TopAbs_COMPOUND && !compound)
      {
        for (TopoDS_Iterator it2(sh); it2.More(); it2.Next())
        {
          nb++;
          sh = it2.Value();
          B.Add(C, sh);
        }
      }
      else
      {
        nb++;
        B.Add(C, sh);
      }
    }
    if (nb == 0)
      C.Nullify();
    else if (nb == 1)
      return sh;
    return C;
  }

  //   Egalite : OK;  Pseudo-Egalite : EDGE/WIRE ou FACE/SHELL
  if (typ == type)
    return shape;
  if (typ == TopAbs_EDGE && type == TopAbs_WIRE)
  {
    ShapeBuilder B;
    TopoWire  W;
    B.MakeWire(W);
    B.Add(W, shape);
    return W;
  }
  if (typ == TopAbs_FACE && type == TopAbs_SHELL)
  {
    ShapeBuilder B;
    TopoShell S;
    B.MakeShell(S);
    B.Add(S, shape);
    return S;
  }

  //   Le reste : selon exploration
  if (!explore)
  {
    TopoShape nulsh;
    return nulsh;
  }

  //  Ici, on doit explorer
  //  SOLID + mode COMPOUND : reconduire les SHELLs
  if (typ == TopAbs_SOLID && compound)
  {
    TopoCompound C;
    ShapeBuilder    B;
    B.MakeCompound(C);
    for (TopoDS_Iterator it(shape); it.More(); it.Next())
    {
      sh0 = SortedCompound(it.Value(), type, explore, compound);
      if (sh0.IsNull())
        continue;
      sh = sh0;
      nb++;
      B.Add(C, sh);
    }
    if (nb == 0)
      C.Nullify();
    else if (nb == 1)
      return sh;
    return C;
  }

  //  Exploration classique
  TopoCompound CC;
  ShapeBuilder    BB;
  BB.MakeCompound(CC);
  // Standard_Integer iena = Standard_False; //szv#4:S4163:12Mar99 unused
  for (ShapeExplorer aExp(shape, type); aExp.More(); aExp.Next())
  {
    nb++;
    sh = aExp.Current();
    BB.Add(CC, sh);
  }
  if (nb == 0)
    CC.Nullify();
  else if (nb == 1)
    return sh;
  return CC;
}

//=================================================================================================

void Explorer::DispatchList(const Handle(TopTools_HSequenceOfShape)& list,
                                        Handle(TopTools_HSequenceOfShape)&       vertices,
                                        Handle(TopTools_HSequenceOfShape)&       edges,
                                        Handle(TopTools_HSequenceOfShape)&       wires,
                                        Handle(TopTools_HSequenceOfShape)&       faces,
                                        Handle(TopTools_HSequenceOfShape)&       shells,
                                        Handle(TopTools_HSequenceOfShape)&       solids,
                                        Handle(TopTools_HSequenceOfShape)&       compsols,
                                        Handle(TopTools_HSequenceOfShape)&       compounds) const
{
  if (list.IsNull())
    return;
  if (vertices.IsNull())
    vertices = new TopTools_HSequenceOfShape();
  if (edges.IsNull())
    edges = new TopTools_HSequenceOfShape();
  if (wires.IsNull())
    wires = new TopTools_HSequenceOfShape();
  if (faces.IsNull())
    faces = new TopTools_HSequenceOfShape();
  if (shells.IsNull())
    shells = new TopTools_HSequenceOfShape();
  if (solids.IsNull())
    solids = new TopTools_HSequenceOfShape();
  if (compsols.IsNull())
    compsols = new TopTools_HSequenceOfShape();
  if (compounds.IsNull())
    compounds = new TopTools_HSequenceOfShape();

  Standard_Integer i, nb = list->Length();
  for (i = 1; i <= nb; i++)
  {
    TopoShape sh = list->Value(i);
    if (sh.IsNull())
      continue;
    switch (sh.ShapeType())
    {
      case TopAbs_VERTEX:
        vertices->Append(sh);
        break;
      case TopAbs_EDGE:
        edges->Append(sh);
        break;
      case TopAbs_WIRE:
        wires->Append(sh);
        break;
      case TopAbs_FACE:
        faces->Append(sh);
        break;
      case TopAbs_SHELL:
        shells->Append(sh);
        break;
      case TopAbs_SOLID:
        solids->Append(sh);
        break;
      case TopAbs_COMPSOLID:
        compsols->Append(sh);
        break;
      case TopAbs_COMPOUND:
        compounds->Append(sh);
        break;
      default:
        break;
    }
  }
}
