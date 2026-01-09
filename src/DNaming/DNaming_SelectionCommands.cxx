// Created on: 1997-10-20
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

#include <DNaming.hxx>

#include <DDF.hxx>

#include <TNaming_Tool.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TNaming.hxx>
#include <TNaming_Naming.hxx>
#include <TNaming_NamingTool.hxx>
#include <TNaming_MapOfNamedShape.hxx>
#include <TDF_ChildIterator.hxx>
#include <TNaming_Selector.hxx>
#include <TopoDS_Shape.hxx>
#include <TopAbs.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDF_LabelMap.hxx>

#include <Draw_Appli.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <DBRep.hxx>
#include <DBRep_DrawableShape.hxx>
#include <stdio.h>

//=================================================================================================

static void Display(const Standard_CString Name, const TopoShape& S)
{
  // char* name = Name;
  static Standard_Integer nbIsos  = 2;
  static Standard_Real    size    = 100.;
  static Standard_Integer discret = 30;

  Handle(DBRep_DrawableShape) D =
    new DBRep_DrawableShape(S, Draw_jaune, Draw_vert, Draw_bleu, Draw_rouge, size, nbIsos, discret);
  Draw1::Set(Name, D);
}

//=================================================================================================

// static void DumpNaming (const Handle(TNaming_Naming)& naming)
static void DumpNaming(const Handle(TNaming_Naming)& naming, DrawInterpreter& di)
{
  AsciiString1 Entry;
  const TNaming_Name&     AName = naming->GetName();
  // TNaming1::Print(AName.Type(),std::cout);
  Standard_SStream aStream1;
  TNaming1::Print(AName.Type(), aStream1);
  di << aStream1;
  di << " ";
  // TopAbs1::Print(AName.ShapeType(),std::cout);
  Standard_SStream aStream2;
  TopAbs1::Print(AName.ShapeType(), aStream2);
  di << aStream2;
  const TNaming_ListOfNamedShape& NSS = AName.Arguments();
  for (TNaming_ListIteratorOfListOfNamedShape it(NSS); it.More(); it.Next())
  {
    Tool3::Entry(it.Value()->Label(), Entry);
    di << " " << Entry.ToCString();
  }
  if (!AName.StopNamedShape().IsNull())
  {
    Tool3::Entry(AName.StopNamedShape()->Label(), Entry);
    di << " Stop " << Entry.ToCString();
  }
}

//=======================================================================
// function : SelectShape ou SelectGeometry
// purpose  : "Select DF entry shape [context [orient]]",
//=======================================================================

static Standard_Integer DNaming_Select(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n > 3)
  {
    Standard_Boolean geometry = !(strcmp(a[0], "SelectGeometry"));
    Handle(Data2) DF;
    if (!DDF1::GetDF(a[1], DF))
      return 1;
    DataLabel L;
    DDF1::AddLabel(DF, a[2], L);
    NamingSelector SL(L);
    if (n == 4)
    {
      TopoShape S = DBRep1::Get(a[3], TopAbs_SHAPE);
      SL.Select(S, geometry);
    }
    if (n > 4)
    {
      Standard_Boolean Orient(Standard_False);
      if (n == 6)
        Orient = (Draw1::Atoi(a[5]) != 0);
      TopoShape S = DBRep1::Get(a[3], TopAbs_SHAPE);
      TopoShape C = DBRep1::Get(a[4], TopAbs_SHAPE);
      SL.Select(S, C, geometry, Orient);
    }
    return 0;
  }
  di << "DNaming_Select : Error\n";
  return 1;
}

// #define DEB_SELN 1
//=================================================================================================

Standard_Boolean FillValidMap(const DataLabel& theLabel, TDF_LabelMap& theValidMap)
{
  Standard_Boolean extRefFound = Standard_False;
  TDF_AttributeMap anExtMap;
#ifdef OCCT_DEBUG_SELN
  AsciiString1 entr1;
  Tool3::Entry(theLabel, entr1);
  std::cout << "\tNaming Attribute at = " << entr1 << std::endl;
#endif
  ChildIterator itr(theLabel, Standard_True);
  for (; itr.More(); itr.Next())
  {
    const DataLabel&       aLabel = itr.Value();
    Handle(TNaming_Naming) aNaming;
    if (!aLabel.IsNull())
      aLabel.FindAttribute(TNaming_Naming::GetID(), aNaming);
    if (aNaming.IsNull())
      continue;
#ifdef OCCT_DEBUG_SELN
    Tool3::Entry(aLabel, entr1);
    std::cout << "\tNaming Attribute at = " << entr1 << std::endl;
#endif
    Tool3::OutReferences(aLabel, anExtMap);
    for (TDF_MapIteratorOfAttributeMap attMItr(anExtMap); attMItr.More(); attMItr.Next())
    {
      const Handle(TDF_Attribute)& att = attMItr.Key1();
#ifdef OCCT_DEBUG_SELN
      Tool3::Entry(att->Label(), entr1);
      std::cout << "## References attribute dynamic type = " << att->DynamicType()
                << " at Label = " << entr1 << std::endl;
#endif
      if (att->Label().IsDifferent(aLabel) && !att->Label().IsDescendant(theLabel))
      {
        theValidMap.Add(att->Label());
        Handle(ShapeAttribute) aNS;
        att->Label().FindAttribute(ShapeAttribute::GetID(), aNS);
        if (!aNS.IsNull())
          NamingTool1::BuildDescendants(aNS, theValidMap);
        extRefFound = Standard_True;
      }
    }
  }
  return extRefFound;
}

//=======================================================================
// function : SolveSelection
// purpose  : "SolveSelection DF entry",
//=======================================================================

static Standard_Integer DNaming_SolveSelection(DrawInterpreter& di,
                                               Standard_Integer  n,
                                               const char**      a)
{
  if (n == 3)
  {
    char             name[100];
    Handle(Data2) DF;
    if (!DDF1::GetDF(a[1], DF))
      return 1;
    DataLabel L;
    DDF1::AddLabel(DF, a[2], L);

    Handle(TNaming_Naming) naming;
    if (!L.FindAttribute(TNaming_Naming::GetID(), naming))
    {
      std::cout << "DNaming__SolveSelection  : not a selection" << std::endl;
      return 1;
    }
    TDF_LabelMap aValidMap;
    if (!FillValidMap(L, aValidMap))
      di << "Valid map is empty\n";
#ifdef OCCT_DEBUG_SELN
    std::cout << "== Valid Label map ==" << std::endl;
    for (TDF_MapIteratorOfLabelMap mapItr(aValidMap); mapItr.More(); mapItr.Next())
    {
      const DataLabel& aLab = mapItr.Key1();

      AsciiString1 entr1;
      Tool3::Entry(aLab, entr1);
      std::cout << "  Label = " << entr1 << std::endl;
    }
#endif

    NamingSelector SL(L);
    Standard_Boolean isSolved = SL.Solve(aValidMap);
    if (!isSolved)
      di << "!!! Solver is failed\n";
    TopoShape Res = Tool11::CurrentShape(SL.NamedShape1());
    Sprintf(name, "%s_%s", "new", a[2]);
    Display(name, Res);
    return 0;
  }
  di << "DNaming_SolveSelection : Error\n";
  return 1;
}

//=======================================================================
// function : DumpSelection
// purpose  : DumpSelection DF entry (R)"
//=======================================================================
static Standard_Integer DNaming_DumpSelection(DrawInterpreter& di,
                                              Standard_Integer  n,
                                              const char**      a)
{
  if (n == 3 || n == 4)
  {
    Handle(Data2) DF;
    if (!DDF1::GetDF(a[1], DF))
      return 1;
    DataLabel L;
    if (!DDF1::FindLabel(DF, a[2], L))
      return 1;
    Handle(TNaming_Naming) naming;
    if (!L.FindAttribute(TNaming_Naming::GetID(), naming))
    {
      di << "DNaming_DumpSelection : not a selection\n";
      return 1;
    }
    DumpNaming(naming, di);
    di << "\n";
    if (n == 4)
    {
      Standard_Integer        depth    = L.Depth();
      Standard_Integer        curdepth = 0;
      AsciiString1 Entry;
      ChildIterator       it(naming->Label(), Standard_True);
      for (; it.More(); it.Next())
      {
        if (it.Value().FindAttribute(TNaming_Naming::GetID(), naming))
        {
          curdepth = (naming->Label().Depth() - depth);
          for (Standard_Integer i = 1; i <= curdepth; i++)
            di << " ";
          Tool3::Entry(naming->Label(), Entry);
          di << Entry.ToCString() << " ";
          DumpNaming(naming, di);
          di << "\n";
        }
      }
    }
    return 0;
  }
  di << "DNaming_DumpSelection : Error\n";
  return 1;
}

//=======================================================================
// function : ArgsSelection
// purpose  : ArgsSelection DF entry"
//=======================================================================
static Standard_Integer DNaming_ArgsSelection(DrawInterpreter& di,
                                              Standard_Integer  n,
                                              const char**      a)
{
  if (n == 3)
  {
    Handle(Data2) DF;
    if (!DDF1::GetDF(a[1], DF))
      return 1;
    DataLabel L;
    if (!DDF1::FindLabel(DF, a[2], L))
      return 1;
    Handle(TNaming_Naming) naming;
    if (!L.FindAttribute(TNaming_Naming::GetID(), naming))
    {
      di << "DNaming_DumpSelection : not a selection\n";
      return 1;
    }
    AsciiString1 Entry;
    NamingSelector        SL(L);
    di << " Selection Arguments : ";
    TDF_AttributeMap args;
    SL.Arguments(args);
    for (TDF_MapIteratorOfAttributeMap it(args); it.More(); it.Next())
    {
      Tool3::Entry(it.Key1()->Label(), Entry);
      di << Entry.ToCString() << " ";
    }
    di << "\n";
    return 0;
  }
  di << "DNaming_ArgsSelection : Error\n";
  return 1;
}

//=================================================================================================

static void CollectAttachment(const DataLabel&              root,
                              const Handle(TNaming_Naming)& naming,
                              TNaming_MapOfNamedShape&      attachment)
{
  TNaming_ListIteratorOfListOfNamedShape itarg;
  const TNaming_ListOfNamedShape&        args = naming->GetName().Arguments();
  for (itarg.Initialize(args); itarg.More(); itarg.Next())
  {
    if (!itarg.Value()->Label().IsDescendant(root))
      attachment.Add(itarg.Value());
  }
  Handle(TNaming_Naming) subnaming;
  for (ChildIterator it(naming->Label(), Standard_True); it.More(); it.Next())
  {
    if (it.Value().FindAttribute(TNaming_Naming::GetID(), subnaming))
    {
      const TNaming_ListOfNamedShape& subargs = subnaming->GetName().Arguments();
      for (itarg.Initialize(subargs); itarg.More(); itarg.Next())
      {
        if (!itarg.Value()->Label().IsDescendant(root))
          attachment.Add(itarg.Value());
      }
    }
  }
}

//=======================================================================
// function : Attachment
// purpose  : Attachment DF entry"
//=======================================================================

static Standard_Integer DNaming_Attachment(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n == 3)
  {
    Handle(Data2) DF;
    if (!DDF1::GetDF(a[1], DF))
      return 1;
    DataLabel L;
    if (!DDF1::FindLabel(DF, a[2], L))
      return 1;
    Handle(TNaming_Naming)  naming;
    TNaming_MapOfNamedShape attachment;
    if (L.FindAttribute(TNaming_Naming::GetID(), naming))
    {
      CollectAttachment(L, naming, attachment);
    }
    else
    {
      for (ChildIterator it(L, Standard_True); it.More(); it.Next())
      {
        if (it.Value().FindAttribute(TNaming_Naming::GetID(), naming))
        {
          CollectAttachment(L, naming, attachment);
          it.NextBrother();
        }
      }
    }
    AsciiString1 Entry;
    Tool3::Entry(L, Entry);
    di << " Attachment of " << Entry.ToCString();
    di << "\n";
    for (TNaming_MapIteratorOfMapOfNamedShape ita(attachment); ita.More(); ita.Next())
    {
      Tool3::Entry(ita.Key1()->Label(), Entry);
      di << Entry.ToCString() << " ";
    }
    di << "\n";
    return 0;
  }
  di << "DNaming_Attachment : Error\n";
  return 1;
}

//=================================================================================================

void DNaming1::SelectionCommands(DrawInterpreter& theCommands)
{

  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  const char* g = "Naming1 data commands";

  theCommands.Add("SelectShape",
                  "SelectShape DF entry shape [context [Orient]]",
                  __FILE__,
                  DNaming_Select,
                  g);

  theCommands.Add("SelectGeometry",
                  "SelectGeometry DF entry shape [context]",
                  __FILE__,
                  DNaming_Select,
                  g);

  theCommands.Add("DumpSelection", "DumpSelected DF entry", __FILE__, DNaming_DumpSelection, g);

  theCommands.Add("ArgsSelection", "ArgsSelection DF entry", __FILE__, DNaming_ArgsSelection, g);

  theCommands.Add("SolveSelection", "DumpSelection DF entry", __FILE__, DNaming_SolveSelection, g);

  theCommands.Add("Attachment", "Attachment DF entry", __FILE__, DNaming_Attachment, g);
}
