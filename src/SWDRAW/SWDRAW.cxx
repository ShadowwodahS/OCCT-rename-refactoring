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

#include <SWDRAW.hxx>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <DBRep.hxx>
#include <gp_Trsf.hxx>
#include <ShapeProcess_OperLibrary.hxx>
#include <SWDRAW_ShapeAnalysis.hxx>
#include <SWDRAW_ShapeCustom.hxx>
#include <SWDRAW_ShapeExtend.hxx>
#include <SWDRAW_ShapeFix.hxx>
#include <SWDRAW_ShapeProcess.hxx>
#include <SWDRAW_ShapeProcessAPI.hxx>
#include <SWDRAW_ShapeTool.hxx>
#include <SWDRAW_ShapeUpgrade.hxx>

// #72 rln 09.03.99 Packaging of SWDRAW1
//   for NSPApply -- CKY 12 JUL 2001
static int dejadraw = 0;

// #72 rln 09.03.99 Packaging of SWDRAW1

//=================================================================================================

static Standard_Integer LocSet(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 2)
  {
    di << argv[0] << "LocSet a [b [c]]: set location for shape \"a\":\n";
    di << "- to Null if one argument is given\n";
    di << "- to location of shape b if two arguments are given\n";
    di << "- to difference of locations of shapes b and c if three arguments are given\n";
    return 1;
  }

  TopoShape a = DBRep1::Get(argv[1]);
  if (a.IsNull())
  {
    di << "No shape named \"" << argv[1] << "\" found\n";
    return 1;
  }
  TopLoc_Location L;
  if (argc > 2)
  {
    TopoShape b = DBRep1::Get(argv[2]);
    if (b.IsNull())
    {
      di << "No shape named \"" << argv[2] << "\" found\n";
      return 1;
    }
    if (argc > 3)
    {
      TopoShape c = DBRep1::Get(argv[3]);
      if (c.IsNull())
      {
        di << "No shape named \"" << argv[3] << "\" found\n";
        return 1;
      }
      L = b.Location().Multiplied(c.Location().Inverted());
    }
    else
      L = b.Location();
  }
  a.Location(L);
  DBRep1::Set(argv[1], a);

  return 0;
}

//=================================================================================================

static Standard_Integer LocDump(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 2)
  {
    di << argv[0] << "LocDump a: dump location of shape \"a\"\n";
    return 1;
  }

  TopoShape a = DBRep1::Get(argv[1]);
  if (a.IsNull())
  {
    di << "No shape named \"" << argv[1] << "\" found\n";
    return 1;
  }

  const TopLoc_Location& L = a.Location();
  di << "Location of shape " << argv[1] << ":\n";
  di << "Results in:\n";
  Transform3d          T = L.Transformation();
  TopLoc_Location  l(T);
  Standard_SStream aSStream;
  l.ShallowDump(aSStream);
  di << aSStream;

  return 0;
}

//=================================================================================================

void SWDRAW1::Init(DrawInterpreter& theCommands)
{
  if (!dejadraw)
  {
    dejadraw = 1;
  }

  ShapeTool1::InitCommands(theCommands);
  ShapeAnalysis2::InitCommands(theCommands);
  ShapeCustom2::InitCommands(theCommands);
  ShapeExtend2::InitCommands(theCommands);
  ShapeFix2::InitCommands(theCommands);
  ShapeUpgrade2::InitCommands(theCommands);
  ShapeProcess2::InitCommands(theCommands);
  ShapeProcessAPI::InitCommands(theCommands);

  // locations
  theCommands.Add("LocSet",
                  "a [b [c]]: set loc b->a; use no args to get help",
                  __FILE__,
                  LocSet,
                  "essai");
  theCommands.Add("LocDump", "a: dump location of a", __FILE__, LocDump, "essai");

  // register operators for ShapeProcessing
  OperLibrary::Init();
}

//=================================================================================================

Standard_CString SWDRAW1::GroupName()
{
  return "Shape Healing";
}
