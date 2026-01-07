// Created on: 1999-06-24
// Created by: Sergey ZARITCHNY
// Copyright (c) 1999-1999 Matra Datavision
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

#include <DBRep.hxx>
#include <DNaming.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <TCollection_AsciiString.hxx>
#include <TNaming_CopyShape.hxx>
#include <TNaming_Translator.hxx>
#include <DNaming_DataMapIteratorOfDataMapOfShapeOfName.hxx>
#include <TopTools_MapIteratorOfMapOfShape.hxx>

//=======================================================================
// function : DNaming_CheckHasSame
// purpose  : CheckIsSame  Shape1 Shape2
//           - for test ShapeCopy mechanism
//=======================================================================

static Standard_Integer DNaming_CheckHasSame(DrawInterpreter& di,
                                             Standard_Integer  nb,
                                             const char**      arg)
{
  if (nb < 4)
    return 1;
  TopoShape S1 = DBRep1::Get(arg[1]);
  if (S1.IsNull())
  {
    ShapeBuilder aBuilder;
    BRepTools1::Read(S1, arg[1], aBuilder);
  }

  TopoShape S2 = DBRep1::Get(arg[2]);
  if (S2.IsNull())
  {
    ShapeBuilder aBuilder;
    BRepTools1::Read(S2, arg[2], aBuilder);
  }
  char M[8];
  strcpy(M, arg[3]);
  strtok(M, " \t");
  TopAbs_ShapeEnum mod = TopAbs_FACE;
  if (M[0] == 'F' || M[0] == 'f')
    mod = TopAbs_FACE;
  else if (M[0] == 'E' || M[0] == 'e')
    mod = TopAbs_EDGE;
  else if (M[0] == 'V' || M[0] == 'v')
    mod = TopAbs_VERTEX;
  else
    return 1;

  ShapeExplorer Exp1, Exp2;

  TopTools_MapOfShape M1, M2;
  for (Exp1.Init(S1, mod); Exp1.More(); Exp1.Next())
  {
    M1.Add(Exp1.Current());
  }
  for (Exp2.Init(S2, mod); Exp2.More(); Exp2.Next())
  {
    M2.Add(Exp2.Current());
  }

  TopTools_MapIteratorOfMapOfShape itr1(M1);
  TopTools_MapIteratorOfMapOfShape itr2;
  for (; itr1.More(); itr1.Next())
  {
    const TopoShape& s1 = itr1.Key();

    for (itr2.Initialize(M2); itr2.More(); itr2.Next())
    {
      const TopoShape& s2 = itr2.Key();
      if (s1.IsSame(s2))
        di << "Shapes " << arg[1] << " and " << arg[2] << " have SAME subshapes\n";
    }
  }

  return 0;
}

//=======================================================================
// function : DNaming_TCopyShape
// purpose  : CopyShape  Shape1 [Shape2 ...]
//           - for test ShapeCopy mechanism
//=======================================================================

static Standard_Integer DNaming_TCopyShape(DrawInterpreter& di,
                                           Standard_Integer  nb,
                                           const char**      arg)
{
  NamingTranslator TR;
  if (nb < 2)
    return (1);

  DNaming_DataMapOfShapeOfName aDMapOfShapeOfName;
  for (Standard_Integer i = 1; i < nb; i++)
  {
    TopoShape            S = DBRep1::Get(arg[i]);
    AsciiString1 name(arg[i]);
    name.AssignCat("_c");
    if (S.IsNull())
    {
      ShapeBuilder aBuilder;
      BRepTools1::Read(S, arg[i], aBuilder);
    }

    // Add to Map
    if (S.IsNull())
      return (1);
    else
    {
      aDMapOfShapeOfName.Bind(S, name);
      TR.Add(S);
    }
  } // for ...

  // PERFORM
  TR.Perform();

  if (TR.IsDone())
  {
    di << "DNaming_CopyShape:: Copy is Done \n";

    DNaming_DataMapIteratorOfDataMapOfShapeOfName itrn(aDMapOfShapeOfName);
    for (; itrn.More(); itrn.Next())
    {
      const AsciiString1& name   = itrn.Value();
      const TopoShape             Result = TR.Copied(itrn.Key());
      DBRep1::Set(name.ToCString(), Result);
      di.AppendElement(name.ToCString());
    }
    return 0;
  }
  di << "DNaming_CopyShape : Error\n";
  return 1;
}

//=======================================================================
// function : DNaming_TCopyTool
// purpose  : CopyTool  Shape1 [Shape2 ...]
//           - for test ShapeCopier::CopyTool mechanism
//=======================================================================

static Standard_Integer DNaming_TCopyTool(DrawInterpreter& di,
                                          Standard_Integer  nb,
                                          const char**      arg)
{
  if (nb < 2)
  {
    di << "Usage: CopyTool Shape1 [Shape2] ...\n";
    return 1;
  }

  Standard_Integer                           i;
  AsciiString1                    aCopyNames;
  ShapeBuilder                               aBuilder;
  TColStd_IndexedDataMapOfTransientTransient aMap;
  TopoShape                               aResult;

  for (i = 1; i < nb; i++)
  {
    TopoShape aShape = DBRep1::Get(arg[i]);

    if (aShape.IsNull())
    {
      BRepTools1::Read(aShape, arg[i], aBuilder);
    }

    if (aShape.IsNull())
    {
      di << arg[i] << " is neither a shape nor a BREP file. Skip it.\n";
      continue;
    }

    // Perform copying.
    ShapeCopier::CopyTool(aShape, aMap, aResult);

    // Draw1 result.
    AsciiString1 aName(arg[i]);

    aName.AssignCat("_c");
    DBRep1::Set(aName.ToCString(), aResult);

    // Compose all names of copies.
    if (!aCopyNames.IsEmpty())
    {
      aCopyNames.AssignCat(" ");
    }

    aCopyNames.AssignCat(aName);
  }

  di << aCopyNames.ToCString() << "\n";

  return 0;
}

//=================================================================================================

void DNaming1::ToolsCommands(DrawInterpreter& theCommands)
{

  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done          = Standard_True;
  const char* g = "Naming data commands ";

  theCommands.Add("CopyShape", "CopyShape (Shape1 [Shape2] ...)", __FILE__, DNaming_TCopyShape, g);

  theCommands.Add("CopyTool", "CopyTool Shape1 [Shape2] ...", __FILE__, DNaming_TCopyTool, g);

  theCommands.Add("CheckSame",
                  "CheckSame (Shape1 Shape2 ExploMode[F|E|V])",
                  __FILE__,
                  DNaming_CheckHasSame,
                  g);
}
