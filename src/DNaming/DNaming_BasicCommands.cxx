// Created on: 1997-01-13
// Created by: VAUTHIER Jean-Claude
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
#include <TCollection_AsciiString.hxx>
#include <TDF_Data.hxx>
#include <TDF_Tool.hxx>
#include <DDF.hxx>
#include <TNaming.hxx>
#include <TNaming_NewShapeIterator.hxx>
#include <TNaming_OldShapeIterator.hxx>
#include <TNaming_Iterator.hxx>
#include <TNaming_Tool.hxx>
#include <TNaming_MapOfNamedShape.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <TopoDS_Shape.hxx>
#include <DBRep.hxx>

#include <TNaming_Builder.hxx>
#include <TDataStd_Name.hxx>

#include <stdio.h>

// POP : first Wrong Declaration : now it is correct
//       second not used
// extern void DNaming_BuildMap(TDF_LabelMap& Updated, const DataLabel& Lab);

//=================================================================================================

static Standard_Integer Ascendants(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
    return 1;

  char name[100];

  Handle(TDF_Data) ND;
  //  Handle(TNaming_UsedShapes) US;

  if (!DDF1::GetDF(a[1], ND))
    return 1;
  //  ND->Root().FindAttribute(TNaming_UsedShapes::GetID(),US);

  TopoShape S = DBRep1::Get(a[2]);
  if (S.IsNull())
    return 1;

  Standard_Integer T;

  if (n > 3)
    T = Draw1::Atoi(a[3]);
  else
    T = ND->Transaction();

  // OldShapeIterator it (S, T, US);
  OldShapeIterator it(S, T, ND->Root());
  Standard_Integer         i = 0;
  AsciiString1  entry;
  for (; it.More(); it.Next())
  {
    S = it.Shape();
    Sprintf(name, "%s_%s_%d", a[2], "old", i++);
    DBRep1::Set(name, it.Shape());
    DataLabel Label = it.Label();
    Tool3::Entry(Label, entry);
    di << entry.ToCString() << "\n";
  }
  return 0;
}

//=================================================================================================

static Standard_Integer Descendants(DrawInterpreter& di, Standard_Integer n, const char** a)

{
  if (n < 3)
    return 1;

  char             name[100];
  Handle(TDF_Data) ND;
  //  Handle(TNaming_UsedShapes) US;
  if (!DDF1::GetDF(a[1], ND))
    return 1;
  //  ND->Root().FindAttribute(TNaming_UsedShapes::GetID(),US);

  TopoShape S = DBRep1::Get(a[2]);
  if (S.IsNull())
    return 1;

  Standard_Integer T;

  if (n > 3)
    T = Draw1::Atoi(a[3]);
  else
    T = ND->Transaction();

  NewShapeIterator it(S, T, ND->Root());
  Standard_Integer         i = 0;
  AsciiString1  entry;
  for (; it.More(); it.Next())
  {
    S = it.Shape();
    Sprintf(name, "%s_%s_%d", a[2], "new", i++);
    DBRep1::Set(name, it.Shape());
    DataLabel Label = it.Label();
    Tool3::Entry(Label, entry);
    di << entry.ToCString() << "\n";
  }

  return 0;
}

//=================================================================================================

static Standard_Integer Getentry(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
    return 1;
  Handle(TDF_Data) ND;
  //  Handle(TNaming_UsedShapes) US;

  if (!DDF1::GetDF(a[1], ND))
    return 1;
  //  ND->Root().FindAttribute(TNaming_UsedShapes::GetID(),US);

  TopoShape S = DBRep1::Get(a[2]);
  if (S.IsNull())
  {
    di << "No shape selected\n";
    // di << 0;
    return 0;
  }
  Standard_Integer        aStatus = 0;
  AsciiString1 Name    = DNaming1::GetEntry(S, ND, aStatus);
  if (aStatus == 0)
  {
    di << "E_NoName";
  }
  else
  {
    di << Name.ToCString();
    if (aStatus == 2)
    {
      di << "Several shapes have the same name\n";
    }
  }
  return 0;
}

//=======================================================================
// function : NamedShape
// purpose  : retrieve label of Primitive or a Generated shape
//=======================================================================
static Standard_Integer NamedShape(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
    return 1;
  Handle(TDF_Data) ND;
  //  Handle(TNaming_UsedShapes) US;

  if (!DDF1::GetDF(a[1], ND))
    return 1;
  TopoShape SS = DBRep1::Get(a[2]);
  if (SS.IsNull())
  {
    di << "No shape selected\n";
    // di << 0;
    return 0;
  }

  Handle(ShapeAttribute) NS = Tool11::NamedShape(SS, ND->Root());

  if (NS.IsNull())
  {
    di << "E_NoName";
    return 0;
  }
  AsciiString1 Name;
  Tool3::Entry(NS->Label(), Name);
  di << Name.ToCString();
  return 0;
}

//=================================================================================================

static Standard_Integer Currentshape(DrawInterpreter&, Standard_Integer n, const char** a)
{
  if (n < 4)
    return 1;

  Handle(TDF_Data) ND;
  if (!DDF1::GetDF(a[1], ND))
    return 1;

  Standard_CString LabelName = a[2];
  TopoShape     S         = DNaming1::CurrentShape(LabelName, ND);
  if (!S.IsNull())
  {
    if (n == 4)
      DBRep1::Set(a[3], S);
    else
      DBRep1::Set(a[2], S);
    return 0;
  }
  return 0;
}

//=================================================================================================

static Standard_Integer Initialshape(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 4)
    return 1;

  Handle(TDF_Data) ND;

  if (!DDF1::GetDF(a[1], ND))
    return 1;

  TopoShape NS = DBRep1::Get(a[2]);
  if (NS.IsNull())
    return 1;

  TDF_LabelList Labels;
  TopoShape  S = Tool11::InitialShape(NS, ND->Root(), Labels);
  if (!S.IsNull())
  {
    DBRep1::Set(a[3], S);
  }
  TDF_ListIteratorOfLabelList itL(Labels);

  AsciiString1 entry;
  if (itL.More())
  {
    Tool3::Entry(itL.Value(), entry);
    di << entry.ToCString();
    itL.Next();
  }
  for (; itL.More(); itL.Next())
  {
    Tool3::Entry(itL.Value(), entry);
    di << " , " << entry.ToCString();
  }
  di << ".\n";
  return 0;
}

//=================================================================================================

static Standard_Integer Exploreshape(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  char name[100];

  if (n < 4)
    return 1;
  Handle(TDF_Data) ND;
  //  Handle(TNaming_UsedShapes) US;

  if (!DDF1::GetDF(a[1], ND))
    return 1;
  //  ND->Root().FindAttribute(TNaming_UsedShapes::GetID(),US);

  Standard_Integer Trans = ND->Transaction();
  if (n == 5)
  {
    Trans = (Standard_Integer)Draw1::Atof(a[4]);
  }

  DataLabel Lab;
  DDF1::FindLabel(ND, a[2], Lab);
  Handle(ShapeAttribute) NS;
  if (!Lab.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    di << "No shape\n";
    return 0;
  }

  // TNaming1::Print(NS->Evolution(),std::cout);
  Standard_SStream aStream;
  TNaming1::Print(NS->Evolution(), aStream);
  di << aStream << "\n";

  Standard_Integer NbShapes1 = 1;

  for (Iterator1 itL(Lab, Trans); itL.More(); itL.Next())
  {
    if (!itL.OldShape().IsNull())
    {
      Sprintf(name, "%s%s_%d", "old", a[3], NbShapes1);
      DBRep1::Set(name, itL.OldShape());
    }
    if (!itL.NewShape().IsNull())
    {
      Sprintf(name, "%s_%d", a[3], NbShapes1);
      DBRep1::Set(name, itL.NewShape());
    }
    NbShapes1++;
  }
  di << "\n";
  if (NbShapes1 == 0)
  {
    di << "No shape\n";
  }

  return 0;
}

//=======================================================================
// function : GeneratedShape
// purpose  : Generatedshape df shape Generationentry [drawname]
//=======================================================================

static Standard_Integer Generatedshape(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  TopoShape               S;
  Handle(ShapeAttribute) A;
  if (nb >= 4)
  {
    Handle(TDF_Data) DF;
    if (!DDF1::GetDF(arg[1], DF))
      return 1;
    TopoShape               Gen = DBRep1::Get(arg[2]);
    Handle(ShapeAttribute) Generation;
    if (!DDF1::Find(DF, arg[3], ShapeAttribute::GetID(), Generation))
      return 1;
    S = Tool11::GeneratedShape(Gen, Generation);
    if (!S.IsNull())
    {
      if (nb == 4)
        DBRep1::Set(arg[4], S);
      else
        DBRep1::Set(arg[3], S);
      return 0;
    }
  }
  di << "GetShape : Error\n";
  return 1;
}

//=================================================================================================

static Standard_Integer Getshape(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  TopoShape               s;
  Handle(ShapeAttribute) A;
  if (nb >= 3)
  {
    Handle(TDF_Data) DF;
    if (!DDF1::GetDF(arg[1], DF))
      return 1;
    if (!DDF1::Find(DF, arg[2], ShapeAttribute::GetID(), A))
      return 1;
    s = Tool11::GetShape(A);
    if (!s.IsNull())
    {
      if (nb == 4)
        DBRep1::Set(arg[3], s);
      else
        DBRep1::Set(arg[2], s);
      return 0;
    }
  }
  di << "DDataStd_GetShape : Error\n";
  return 1;
}

//=================================================================================================

static Standard_Integer Collect(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  TNaming_MapOfNamedShape    MNS;
  Handle(ShapeAttribute) A;
  Standard_Boolean           OnlyModif = 1;

  if (nb >= 3)
  {
    Handle(TDF_Data) DF;
    if (!DDF1::GetDF(arg[1], DF))
      return 1;
    if (!DDF1::Find(DF, arg[2], ShapeAttribute::GetID(), A))
      return 1;
    if (nb >= 4)
    {
      OnlyModif = (Draw1::Atoi(arg[3]) != 0);
    }
    Tool11::Collect(A, MNS, OnlyModif);
    for (TNaming_MapIteratorOfMapOfNamedShape it(MNS); it.More(); it.Next())
    {
      AsciiString1 Name;
      Tool3::Entry(it.Key()->Label(), Name);
      di << Name.ToCString() << " ";
    }
  }
  return 1;
}

//=======================================================================
// function : GetCreationEntry
// purpose  : retrieve label of Primitive or a Generated shape
//=======================================================================
static Standard_Integer Getcreationentry(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
    return 1;
  Handle(TDF_Data) ND;
  //  Handle(TNaming_UsedShapes) US;

  if (!DDF1::GetDF(a[1], ND))
    return 1;
  //  ND->Root().FindAttribute(TNaming_UsedShapes::GetID(),US);

  TopoShape SS = DBRep1::Get(a[2]);

  if (SS.IsNull())
  {
    di << "No shape selected\n";
    // di << 0;
    return 0;
  }

  TDF_LabelList Labels;
  TopoShape  S = Tool11::InitialShape(SS, ND->Root(), Labels);

  if (S.IsNull())
  {
    di << "E_NoName";
    return 0;
  }
  Standard_Integer        aStatus = 0;
  AsciiString1 Name    = DNaming1::GetEntry(S, ND, aStatus);
  if (aStatus == 0)
  {
    di << "E_NoName";
  }
  else
  {
    di << Name.ToCString();
    if (aStatus == 2)
    {
      di << "Several shapes have the same name\n";
    }
  }
  return 0;
}

//=======================================================================
// function : ImportShape
// purpose  : "ImportShape Doc  entry Shape Name"
//=======================================================================

static Standard_Integer DNaming_ImportShape(DrawInterpreter& di,
                                            Standard_Integer  nb,
                                            const char**      a)
{
  if (nb >= 4)
  {
    Handle(TDF_Data) aDF;
    if (!DDF1::GetDF(a[1], aDF))
      return 1;
    DataLabel L;
    DDF1::AddLabel(aDF, a[2], L);
    const TopoShape& aShape = DBRep1::Get(a[3]);
    if (aShape.IsNull())
      return 1;
    if (nb == 5)
    {
      NameAttribute::Set(L, UtfString(a[4], Standard_True));
    }

    DNaming1::LoadImportedShape(L, aShape);

    DDF1::ReturnLabel(di, L);
    return 0;
  }
  di << "DNaming_NewShape : Error";
  return 1;
}

//=======================================================================
// function : CheckNSIter
// purpose  : "CheckNSIter Doc  entry Shape new/old [1|0]"
//=======================================================================

static Standard_Integer CheckIter(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  if (nb > 3)
  {
    Handle(TDF_Data) aDF;
    Standard_Boolean aNew(Standard_True);
    if (!DDF1::GetDF(arg[1], aDF))
      return 1;
    DataLabel aLabel;
    DDF1::AddLabel(aDF, arg[2], aLabel);
    TNaming_Builder     aNB(aLabel);
    const TopoShape& aShape = DBRep1::Get(arg[3]);
    aNB.Generated(aShape);
    Iterator1 aNameIter(aLabel);
    if (nb == 5)
      aNew = (Draw1::Atoi(arg[4]) != 0);
    if (aNew)
    {
      NewShapeIterator aNewShapeIter(aNameIter);
      di << "DNaming_CheckIterator : New It is OK\n";
    }
    else
    {
      OldShapeIterator oldShapeIter(aNameIter);
      di << "DNaming_CheckIterator : Old It is OK\n";
    }
    return 0;
  }
  di << "DNaming_CheckIterator : Error\n";
  return 1;
}

//
//=================================================================================================

void DNaming1::BasicCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  const char* g = "Naming data commands";

  // Exploration
  theCommands.Add("Ascendants", "Ascendants df shape [trans]", __FILE__, Ascendants, g);
  theCommands.Add("Descendants", "Descendants  df shape [trans]", __FILE__, Descendants, g);
  theCommands.Add("ExploreShape", "ExploreShape df entry res [trans]", __FILE__, Exploreshape, g);
  theCommands.Add("GetEntry", "GetEntry df shape", __FILE__, Getentry, g);
  theCommands.Add("GetCreationEntry", "GetCreationEntry df shape", __FILE__, Getcreationentry, g);
  theCommands.Add("NamedShape", "NamedShape df shape", __FILE__, NamedShape, g);
  theCommands.Add("InitialShape", "InitialShape df shape res", __FILE__, Initialshape, g);
  theCommands.Add("CurrentShape", "Currentshape df entry [drawname]", __FILE__, Currentshape, g);
  theCommands.Add("GetShape", "GetShape df entry [drawname]", __FILE__, Getshape, g);
  theCommands.Add("Collect", "Collect  df entry [onlymodif 0/1]", __FILE__, Collect, g);
  theCommands.Add("GeneratedShape",
                  "Generatedshape df shape Generationentry [drawname]",
                  __FILE__,
                  Generatedshape,
                  g);
  theCommands.Add("ImportShape",
                  "ImportShape Doc Entry Shape [Name]",
                  __FILE__,
                  DNaming_ImportShape,
                  g);
  //
  theCommands.Add("CheckNSIter", "CheckNSIter df entry shape new[1|0]", __FILE__, CheckIter, g);
}
