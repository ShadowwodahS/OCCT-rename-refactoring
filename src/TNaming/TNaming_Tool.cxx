// Created on: 1997-01-06
// Created by: Yves FRICAUD
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

#include <BRep_Builder.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TNaming_Iterator.hxx>
#include <TNaming_Localizer.hxx>
#include <TNaming_NamedShape.hxx>
#include <TNaming_Naming.hxx>
#include <TNaming_NewShapeIterator.hxx>
#include <TNaming_OldShapeIterator.hxx>
#include <TNaming_Tool.hxx>
#include <TNaming_UsedShapes.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

//=================================================================================================

static void LastModif(NewShapeIterator&   it,
                      const TopoShape&         S,
                      TopTools_IndexedMapOfShape& MS,
                      const TDF_LabelMap&         Updated,
                      TDF_LabelList&              Deleted)
{
  Standard_Boolean YaModif = Standard_False;
  for (; it.More(); it.Next())
  {
    const DataLabel& Lab = it.Label();
    if (!Updated.Contains(Lab))
      continue;

    if (it.IsModification())
    {
      YaModif = Standard_True;
      NewShapeIterator it2(it);
      if (!it2.More())
      {
        const TopoShape& S1 = it.Shape();
        if (S1.IsNull())
        {
          Deleted.Append(Lab);
        }
        else
        {
          MS.Add(S1); // Modified
        }
      }
      else
        LastModif(it2, it.Shape(), MS, Updated, Deleted);
    }
  }
  if (!YaModif)
    MS.Add(S);
}

//=================================================================================================

static void LastModif(NewShapeIterator&   it,
                      TopTools_IndexedMapOfShape& MS,
                      const TopoShape&         S,
                      TDF_LabelList&              Deleted)
{
  Standard_Boolean YaModif = Standard_False;
  for (; it.More(); it.Next())
  {
    const DataLabel& Lab = it.Label();
    if (it.IsModification())
    {
      YaModif = Standard_True;
      NewShapeIterator it2(it);
      if (!it2.More())
      {
        const TopoShape& S1 = it.Shape();
        if (S1.IsNull())
        {
          Deleted.Append(Lab);
        }
        else
        {
          MS.Add(S1); // Modified
        }
      }
      else
        LastModif(it2, MS, it.Shape(), Deleted);
    }
  }
  if (!YaModif)
    MS.Add(S);
}

//=================================================================================================

static TopoShape MakeShape(const TopTools_IndexedMapOfShape& MS)
{
  if (!MS.IsEmpty())
  {
    if (MS.Extent() == 1)
    {
      return MS(1);
    }
    else
    {
      TopoCompound C;
      ShapeBuilder    B;
      B.MakeCompound(C);
      for (Standard_Integer anItMS = 1; anItMS <= MS.Extent(); ++anItMS)
      {
        B.Add(C, MS(anItMS));
      }
      return C;
    }
  }
  return TopoShape();
}

//=================================================================================================

TopoShape Tool11::GetShape(const Handle(ShapeAttribute)& NS)
{
  Iterator1           itL(NS);
  TopTools_IndexedMapOfShape MS;
  if (NS->Evolution() == TNaming_SELECTED)
  {
    for (; itL.More(); itL.Next())
    {
      if (!itL.NewShape().IsNull())
      {
        if (itL.NewShape().ShapeType() != TopAbs_VERTEX)
        { // OR-N
          Handle(TNaming_Naming) aNaming;
          NS->Label().FindAttribute(TNaming_Naming::GetID(), aNaming);
          if (!aNaming.IsNull())
          {
            if (aNaming->GetName().Orientation() == TopAbs_FORWARD
                || aNaming->GetName().Orientation() == TopAbs_REVERSED)
            {
              TopoShape aS = itL.NewShape();
              if (aNaming->GetName().Type() == TNaming_ORIENTATION)
              {
                aS.Orientation(aNaming->GetName().Orientation());
              }
              else
              {
                Handle(TNaming_Naming) aNaming2;
                ChildIterator      it(aNaming->Label());
                for (; it.More(); it.Next())
                {
                  const DataLabel& aLabel = it.Value();
                  aLabel.FindAttribute(TNaming_Naming::GetID(), aNaming2);
                  if (!aNaming2.IsNull())
                  {
                    if (aNaming2->GetName().Type() == TNaming_ORIENTATION)
                    {
                      aS.Orientation(aNaming2->GetName().Orientation());
                      break;
                    }
                  }
                }
              }
              MS.Add(aS);
            }
            else
              MS.Add(itL.NewShape());
          }
          else
            MS.Add(itL.NewShape());
        } //
        else
          MS.Add(itL.NewShape());
      }
    }
  }
  else
    for (; itL.More(); itL.Next())
    {
      if (!itL.NewShape().IsNull())
        MS.Add(itL.NewShape());
    }
  return MakeShape(MS);
}

//=================================================================================================

TopoShape Tool11::OriginalShape(const Handle(ShapeAttribute)& NS)
{
  Iterator1           itL(NS);
  TopTools_IndexedMapOfShape MS;
  for (; itL.More(); itL.Next())
  {
    MS.Add(itL.OldShape());
  }
  return MakeShape(MS);
}

//=======================================================================
static void ApplyOrientation(TopTools_IndexedMapOfShape& MS,
                             const TopAbs_Orientation    OrientationToApply)
{
  for (Standard_Integer anItMS = 1; anItMS <= MS.Extent(); ++anItMS)
  {
    MS.Substitute(anItMS, MS(anItMS).Oriented(OrientationToApply));
  }
}

//=================================================================================================

TopoShape Tool11::CurrentShape(const Handle(ShapeAttribute)& Att)
{
  TopTools_IndexedMapOfShape MS;
  TDF_LabelList              Deleted;

  Iterator1 itL(Att);
  for (; itL.More(); itL.Next())
  {
    const TopoShape& S = itL.NewShape();
    if (S.IsNull())
      continue;
    // OR-N
    Standard_Boolean   YaOrientationToApply(Standard_False);
    TopAbs_Orientation OrientationToApply(TopAbs_FORWARD);
    if (Att->Evolution() == TNaming_SELECTED)
    {
      if (itL.More() && itL.NewShape().ShapeType() != TopAbs_VERTEX)
      {
        Handle(TNaming_Naming) aNaming;
        Att->Label().FindAttribute(TNaming_Naming::GetID(), aNaming);
        if (!aNaming.IsNull())
        {
          if (aNaming->GetName().Type() == TNaming_ORIENTATION)
          {
            OrientationToApply = aNaming->GetName().Orientation();
          }
          else
          {
            Handle(TNaming_Naming) aNaming2;
            ChildIterator      it(aNaming->Label());
            for (; it.More(); it.Next())
            {
              const DataLabel& aLabel = it.Value();
              aLabel.FindAttribute(TNaming_Naming::GetID(), aNaming2);
              if (!aNaming2.IsNull())
              {
                if (aNaming2->GetName().Type() == TNaming_ORIENTATION)
                {
                  OrientationToApply = aNaming2->GetName().Orientation();
                  break;
                }
              }
            }
          }
          if (OrientationToApply == TopAbs_FORWARD || OrientationToApply == TopAbs_REVERSED)
            YaOrientationToApply = Standard_True;
        }
      }
    } //
    NewShapeIterator it(itL);
    if (!it.More())
    {
      MS.Add(S);
    }
    else
    {
      //      LastModif(it, MS, S, Deleted);
      TopTools_IndexedMapOfShape MS2; // to be optimized later
      LastModif(it, MS2, S, Deleted);
      if (YaOrientationToApply)
        ApplyOrientation(MS2, OrientationToApply);
      for (Standard_Integer anItMS2 = 1; anItMS2 <= MS2.Extent(); ++anItMS2)
        MS.Add(MS2(anItMS2));
    }
  }
  return MakeShape(MS);
}

//=================================================================================================

TopoShape Tool11::CurrentShape(const Handle(ShapeAttribute)& Att,
                                        const TDF_LabelMap&               Updated)
{
  DataLabel Lab = Att->Label();

  TopTools_IndexedMapOfShape MS;
  TDF_LabelList              Deleted;

  if (!Updated.Contains(Lab))
  {
    return TopoShape();
  }

  Iterator1 itL(Att);
  for (; itL.More(); itL.Next())
  {
    const TopoShape& S = itL.NewShape();
    if (S.IsNull())
      continue;
    // OR-N
    Standard_Boolean   YaOrientationToApply(Standard_False);
    TopAbs_Orientation OrientationToApply(TopAbs_FORWARD);
    if (Att->Evolution() == TNaming_SELECTED)
    {
      if (itL.More() && itL.NewShape().ShapeType() != TopAbs_VERTEX)
      {
        Handle(TNaming_Naming) aNaming;
        Att->Label().FindAttribute(TNaming_Naming::GetID(), aNaming);
        if (!aNaming.IsNull())
        {
          if (aNaming->GetName().Type() == TNaming_ORIENTATION)
          {
            OrientationToApply = aNaming->GetName().Orientation();
          }
          else
          {
            Handle(TNaming_Naming) aNaming2;
            ChildIterator      it(aNaming->Label());
            for (; it.More(); it.Next())
            {
              const DataLabel& aLabel = it.Value();
              aLabel.FindAttribute(TNaming_Naming::GetID(), aNaming2);
              if (!aNaming2.IsNull())
              {
                if (aNaming2->GetName().Type() == TNaming_ORIENTATION)
                {
                  OrientationToApply = aNaming2->GetName().Orientation();
                  break;
                }
              }
            }
          }
          if (OrientationToApply == TopAbs_FORWARD || OrientationToApply == TopAbs_REVERSED)
            YaOrientationToApply = Standard_True;
        }
      }
    } //
    NewShapeIterator it(itL);
    if (!it.More())
    {
      MS.Add(S);
    }
    else
    {
      //      LastModif(it, S, MS, Updated, Deleted);
      TopTools_IndexedMapOfShape MS2; // to be optimized later
      LastModif(it, S, MS2, Updated, Deleted);
      if (YaOrientationToApply)
        ApplyOrientation(MS2, OrientationToApply);
      for (Standard_Integer anItMS2 = 1; anItMS2 <= MS2.Extent(); ++anItMS2)
        MS.Add(MS2(anItMS2));
    }
  }
  return MakeShape(MS);
}

//=================================================================================================

Handle(ShapeAttribute) Tool11::CurrentNamedShape(const Handle(ShapeAttribute)& Att,
                                                           const TDF_LabelMap& Updated)
{
  TopoShape CS = CurrentShape(Att, Updated);
  if (CS.IsNull())
  {
    Handle(ShapeAttribute) NS;
    return NS;
  }
  return NamedShape1(CS, Att->Label());
}

//=================================================================================================

Handle(ShapeAttribute) Tool11::CurrentNamedShape(const Handle(ShapeAttribute)& Att)

{
  TopoShape CS = CurrentShape(Att);
  if (CS.IsNull())
  {
    Handle(ShapeAttribute) NS;
    return NS;
  }
  return NamedShape1(CS, Att->Label());
}

//=================================================================================================

static void FindModifUntil(NewShapeIterator&         it,
                           TopTools_IndexedMapOfShape&       MS,
                           const Handle(ShapeAttribute)& Context)
{
  for (; it.More(); it.Next())
  {
    if (!it.Shape().IsNull())
    {
      if (it.NamedShape1() == Context)
      {
        MS.Add(it.Shape());
      }
      else
      {
        NewShapeIterator it2(it);
        FindModifUntil(it2, MS, Context);
      }
    }
  }
}

//=================================================================================================

TopoShape Tool11::GeneratedShape(const TopoShape&               S,
                                          const Handle(ShapeAttribute)& Generation)

{
  TopTools_IndexedMapOfShape MS;
  Handle(TNaming_UsedShapes) US;
  Generation->Label().Root().FindAttribute(TNaming_UsedShapes::GetID(), US);

  for (NewShapeIterator it(S, US); it.More(); it.Next())
  {
    if (!it.Shape().IsNull() && it.NamedShape1() == Generation)
    {
      MS.Add(it.Shape());
    }
  }
  if (MS.IsEmpty())
  {
    NewShapeIterator it2(S, US);
    FindModifUntil(it2, MS, Generation);
  }
  return MakeShape(MS);
}

//=================================================================================================

void Tool11::FirstOlds(const Handle(TNaming_UsedShapes)& US,
                             const TopoShape&               S,
                             OldShapeIterator&         it,
                             TopTools_IndexedMapOfShape&       MS,
                             TDF_LabelList&                    Labels)
{
  Standard_Integer TransDef;
  Standard_Boolean YaModif = 0;

  for (; it.More(); it.Next())
  {
    if (it.IsModification())
    {
      YaModif = 1;
      OldShapeIterator it2(it);
      if (!it2.More())
      {
        Labels.Append(Tool11::Label(US, it.Shape(), TransDef));
        MS.Add(it.Shape());
      }
      else
      {
        FirstOlds(US, it.Shape(), it2, MS, Labels);
      }
    }
  }
  if (!YaModif)
    MS.Add(S);
}

//=================================================================================================

TopoShape Tool11::InitialShape(const TopoShape& S,
                                        const DataLabel&    Acces,
                                        TDF_LabelList&      Labels)
{
  Handle(TNaming_UsedShapes) US;
  Acces.Root().FindAttribute(TNaming_UsedShapes::GetID(), US);
  TopoShape Res;

  if (!Tool11::HasLabel(US, S))
    return Res;

  Standard_Integer Transdef;
  Label(US, S, Transdef);
  TopTools_IndexedMapOfShape MS;
  OldShapeIterator   it(S, Transdef, US);
  if (!it.More())
  {
    return S;
  }
  else
  {
    FirstOlds(US, S, it, MS, Labels);
  }
  return MakeShape(MS);
}

//=================================================================================================

static void Back(const Handle(ShapeAttribute)& NS, TNaming_MapOfNamedShape& MNS)
{
  for (Iterator1 it(NS); it.More(); it.Next())
  {
    if (it.NewShape().IsNull())
      continue;
    for (OldShapeIterator Oldit(it); Oldit.More(); Oldit.Next())
    {
      const TopoShape& OS = Oldit.Shape();
      if (!OS.IsNull())
      {
        Handle(ShapeAttribute) NOS = Tool11::NamedShape1(OS, NS->Label());
        // Continue de remonter
        if (!NOS.IsNull())
        {
          if (MNS.Add(NOS))
            Back(NOS, MNS);
        }
      }
    }
  }
}

//=================================================================================================

void Tool11::Collect(const Handle(ShapeAttribute)& NS,
                           TNaming_MapOfNamedShape&          MNS,
                           const Standard_Boolean            OnlyModif)
{
  MNS.Add(NS);
  Back(NS, MNS);

  for (Iterator1 it(NS); it.More(); it.Next())
  {
    if (it.NewShape().IsNull())
      continue;
    for (NewShapeIterator NewIt(it); NewIt.More(); NewIt.Next())
    {
      if (!OnlyModif || NewIt.IsModification())
      {
        // Continue la descente
        Collect(NewIt.NamedShape1(), MNS, OnlyModif);
      }
    }
  }
}

// Pour DEBUGGER
#ifdef OCCT_DEBUG

//=================================================================================================

void TNamingTool_DumpLabel(const TopoShape& S, const DataLabel& Acces)
{
  Handle(ShapeAttribute) NS = Tool11::NamedShape1(S, Acces);
  NS->Label().EntryDump(std::cout);
  std::cout << std::endl;
}

  #include <BRepTools.hxx>

//=================================================================================================

void TNamingTool_Write(const TopoShape& S, const Standard_CString File)
{
  BRepTools1::Write(S, File);
}

#endif

//=================================================================================================

void Tool11::FindShape(const TDF_LabelMap& Valid,
                             const TDF_LabelMap& /*Forbiden*/,
                             const Handle(ShapeAttribute)& Arg,
                             TopoShape&                     S)
{
  if (!Valid.IsEmpty() && !Valid.Contains(Arg->Label()))
    return;
  if (Arg.IsNull() || Arg->IsEmpty())
    return;

  // Which type of shape is being expected?
  Handle(TNaming_Naming) aNaming;
  if (!Arg->FindAttribute(TNaming_Naming::GetID(), aNaming))
  {
#ifdef OCCT_DEBUG
//    std::cout<<"Tool11::FindShape(): Naming1 attribute hasn't been found attached at the
//    Argument label"<<std::endl;
#endif
    return;
  }

  // Looking for sub shapes of the result shape
  TopTools_MapOfShape subShapes;
  ShapeExplorer anExpl(Arg->Get(), (TopAbs_ShapeEnum)((int)(aNaming->GetName().ShapeType()) + 1));
  for (; anExpl.More(); anExpl.Next())
    subShapes.Add(anExpl.Current());
#ifdef OCCT_DEBUG
//  std::cout<<"Tool11::FindShape(): Nb of sub shapes = "<<subShapes.Extent()<<std::endl;
#endif

  // Looking for external arguments:
  TNaming_ListOfNamedShape extArgs;
  TDF_AttributeMap         outRefs;
  Tool3::OutReferences(Arg->Label(), outRefs);
  if (outRefs.IsEmpty())
  {
#ifdef OCCT_DEBUG
//    std::cout<<"Tool11::FindShape(): No out references have been found"<<std::endl;
#endif
    return;
  }
  for (TDF_MapIteratorOfAttributeMap itr(outRefs); itr.More(); itr.Next())
  {
    if (itr.Key1()->DynamicType() == STANDARD_TYPE(ShapeAttribute))
    {
#ifdef OCCT_DEBUG
//      Standard_Integer nbExtArgs = extArgs.Extent();
#endif
      Handle(ShapeAttribute)        anExtArg(Handle(ShapeAttribute)::DownCast(itr.Key1()));
      const Handle(ShapeAttribute)& aCurrentExtArg = Tool11::CurrentNamedShape(anExtArg);
      if (!aCurrentExtArg.IsNull() && !aCurrentExtArg->IsEmpty())
        extArgs.Append(aCurrentExtArg);
#ifdef OCCT_DEBUG
//      if (extArgs.Extent() - 1 == nbExtArgs) {
//	std::cout<<"Tool11::FindShape(): An external reference has been found at ";
//	itr.Key1()->Label().EntryDump(std::cout); std::cout<<std::endl;
//      }
#endif
    }
  }

  // The iterator on external arguments:
  TNaming_ListIteratorOfListOfNamedShape extArgsIterator(extArgs);
  for (; extArgsIterator.More(); extArgsIterator.Next())
  {
    Handle(ShapeAttribute) anExtArg = extArgsIterator.Value();

    // Looking for context:
    Handle(ShapeAttribute) aContextNS;
    if (anExtArg->Label().Father().IsNull()
        || !anExtArg->Label().Father().FindAttribute(ShapeAttribute::GetID(), aContextNS))
    {
      aContextNS = anExtArg;
      // #ifdef OCCT_DEBUG
      //       std::cout<<"Tool11::FindShape(): A context shape hasn't been found at the
      //       father label of the external argument"<<std::endl;
      // #endif
      //       continue;
    }

#ifdef OCCT_DEBUG
//    std::cout<<"Tool11::FindShape(): Searching in the external reference ";
//    aContextNS->Label().EntryDump(std::cout); std::cout<<"  ";
#endif

    // Lets find the sub shape of the context which coincides with our sub shapes (subShapes map):
    ShapeExplorer explC(aContextNS->Get(), aNaming->GetName().ShapeType()), explSubC;
    for (; explC.More(); explC.Next())
    {
      Standard_Integer    DoesCoincide   = 0;
      const TopoShape& possibleResult = explC.Current();
      TopTools_MapOfShape subShapesOfResult;
      for (explSubC.Init(possibleResult,
                         (TopAbs_ShapeEnum)((int)(aNaming->GetName().ShapeType()) + 1));
           explSubC.More();
           explSubC.Next())
      {
        subShapesOfResult.Add(explSubC.Current());
      }
      if (subShapesOfResult.Extent() != subShapes.Extent())
        continue;
      for (TopTools_MapIteratorOfMapOfShape itrR(subShapesOfResult); itrR.More(); itrR.Next())
      {
        for (TopTools_MapIteratorOfMapOfShape itr1(subShapes); itr1.More(); itr1.Next())
        {
          if (itrR.Key1().IsSame(itr1.Key1()))
          {
            DoesCoincide++; // std::cout<<".";
            break;
          }
        }
      }
      if (DoesCoincide == subShapes.Extent())
      {
#ifdef OCCT_DEBUG
//	std::cout<<"Tool11::FindShape(): Found! ";
#endif
        S = possibleResult;
        break;
      }
    }

    if (!S.IsNull())
      break;
#ifdef OCCT_DEBUG
//    std::cout<<std::endl;
#endif
  }

#ifdef OCCT_DEBUG
  if (S.IsNull())
  {
    std::cout << "Tool11::FindShape(): There hasn't been found a sub shape of the context "
                 "shape coinciding with the sub shapes of naming"
              << std::endl;
  }
#endif
}
