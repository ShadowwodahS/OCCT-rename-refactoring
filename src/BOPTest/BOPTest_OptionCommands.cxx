// Created by: Peter KURNEV
// Copyright (c) 2014 OPEN CASCADE SAS
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

#include <BOPTest.hxx>
#include <BOPTest_Objects.hxx>
#include <DBRep.hxx>
#include <Draw.hxx>
#include <BOPAlgo_GlueEnum.hxx>

#include <string.h>
static Standard_Integer boptions(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer brunparallel(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bnondestructive(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bfuzzyvalue(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bGlue(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bdrawwarnshapes(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bcheckinverted(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer buseobb(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bsimplify(DrawInterpreter&, Standard_Integer, const char**);

//=================================================================================================

void BOPTest1::OptionCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;
  // Chapter's name
  const char* g = "BOPTest1 commands";
  // Commands
  theCommands.Add("boptions",
                  "Usage: boptions [-default]\n"
                  "\t\tw/o arguments shows current value of BOP options\n"
                  "\t\t-default - allows setting all options to default values",
                  __FILE__,
                  boptions,
                  g);

  theCommands.Add("brunparallel",
                  "Enables/Disables parallel processing mode.\n"
                  "\t\tUsage: brunparallel 0/1",
                  __FILE__,
                  brunparallel,
                  g);

  theCommands.Add("bnondestructive",
                  "Enables/Disables the safe processing mode.\n"
                  "\t\tUsage: bnondestructive 0/1",
                  __FILE__,
                  bnondestructive,
                  g);

  theCommands.Add("bfuzzyvalue",
                  "Sets the additional tolerance for BOP algorithms.\n"
                  "\t\tUsage: bfuzzyvalue value",
                  __FILE__,
                  bfuzzyvalue,
                  g);

  theCommands.Add("bglue",
                  "Sets the gluing mode for the BOP algorithms.\n"
                  "\t\tUsage: bglue [0 (off) / 1 (shift) / 2 (full)]",
                  __FILE__,
                  bGlue,
                  g);

  theCommands.Add("bdrawwarnshapes",
                  "Enables/Disables drawing of warning shapes of BOP algorithms.\n"
                  "\t\tUsage: bdrawwarnshapes 0 (do not draw) / 1 (draw warning shapes)",
                  __FILE__,
                  bdrawwarnshapes,
                  g);

  theCommands.Add(
    "bcheckinverted",
    "Enables/Disables the check of the input solids on inverted status in BOP algorithms\n"
    "\t\tUsage: bcheckinverted 0 (off) / 1 (on)",
    __FILE__,
    bcheckinverted,
    g);

  theCommands.Add("buseobb",
                  "Enables/disables the usage of OBB in BOP algorithms\n"
                  "\t\tUsage: buseobb 0 (off) / 1 (on)",
                  __FILE__,
                  buseobb,
                  g);

  theCommands.Add("bsimplify",
                  "Enables/Disables the result simplification after BOP\n"
                  "\t\tUsage: bsimplify [-e 0/1] [-f 0/1] [-a tol]\n"
                  "\t\t-e 0/1 - enables/disables edges unification\n"
                  "\t\t-f 0/1 - enables/disables faces unification\n"
                  "\t\t-a tol - changes default angular tolerance of unification algo (accepts "
                  "value in degrees).",
                  __FILE__,
                  bsimplify,
                  g);
}

//=================================================================================================

Standard_Integer boptions(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n > 2)
  {
    di.PrintHelp(a[0]);
    return 1;
  }

  if (n == 2)
  {
    if (strcmp(a[1], "-default"))
    {
      di.PrintHelp(a[0]);
      return 1;
    }

    // Set all options to default values
    Objects::SetDefaultOptions();
    return 0;
  }
  //
  char             buf[128];
  BOPAlgo_GlueEnum aGlue = Objects::Glue();
  //
  Sprintf(buf,
          " RunParallel: %s \t\t(%s)\n",
          Objects::RunParallel() ? "Yes" : "No",
          "use \"brunparallel\" command to change");
  di << buf;
  Sprintf(buf,
          " NonDestructive: %s \t\t(%s)\n",
          Objects::NonDestructive() ? "Yes" : "No",
          "use \"bnondestructive\" command to change");
  di << buf;
  Sprintf(buf,
          " FuzzyValue: %g \t\t(%s)\n",
          Objects::FuzzyValue(),
          "use \"bfuzzyvalue\" command to change");
  di << buf;
  Sprintf(buf,
          " GlueOption: %s \t\t(%s)\n",
          ((aGlue == BOPAlgo_GlueOff) ? "Off" : ((aGlue == BOPAlgo_GlueFull) ? "Full" : "Shift")),
          "use \"bglue\" command to change");
  di << buf;
  Sprintf(buf,
          " Draw1 Warning Shapes: %s \t(%s)\n",
          Objects::DrawWarnShapes() ? "Yes" : "No",
          "use \"bdrawwarnshapes\" command to change");
  di << buf;
  Sprintf(buf,
          " Check for invert solids: %s \t(%s)\n",
          Objects::CheckInverted() ? "Yes" : "No",
          "use \"bcheckinverted\" command to change");
  di << buf;
  Sprintf(buf,
          " Use OBB: %s \t\t\t(%s)\n",
          Objects::UseOBB() ? "Yes" : "No",
          "use \"buseobb\" command to change");
  di << buf;
  Sprintf(buf,
          " Unify Edges: %s \t\t(%s)\n",
          Objects::UnifyEdges() ? "Yes" : "No",
          "use \"bsimplify -e\" command to change");
  di << buf;
  Sprintf(buf,
          " Unify Faces: %s \t\t(%s)\n",
          Objects::UnifyFaces() ? "Yes" : "No",
          "use \"bsimplify -f\" command to change");
  di << buf;
  Sprintf(buf,
          " Angular: %g \t\t(%s)\n",
          Objects::Angular(),
          "use \"bsimplify -a\" command to change");
  di << buf;
  //
  return 0;
}

//=================================================================================================

Standard_Integer bfuzzyvalue(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 2)
  {
    di.PrintHelp(a[0]);
    return 1;
  }

  Standard_Real aFuzzyValue = Draw1::Atof(a[1]);
  Objects::SetFuzzyValue(aFuzzyValue);
  return 0;
}

//=================================================================================================

Standard_Integer brunparallel(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 2)
  {
    di.PrintHelp(a[0]);
    return 1;
  }

  Standard_Integer iRunParallel = Draw1::Atoi(a[1]);
  Objects::SetRunParallel(iRunParallel != 0);
  return 0;
}

//=================================================================================================

Standard_Integer bnondestructive(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 2)
  {
    di.PrintHelp(a[0]);
    return 1;
  }

  Standard_Integer iNonDestructive = Draw1::Atoi(a[1]);
  Objects::SetNonDestructive(iNonDestructive != 0);
  return 0;
}

//=================================================================================================

Standard_Integer bGlue(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 2)
  {
    di.PrintHelp(a[0]);
    return 1;
  }

  Standard_Integer iGlue = Draw1::Atoi(a[1]);
  if (iGlue < 0 || iGlue > 2)
  {
    di << "Wrong value.\n";
    di.PrintHelp(a[0]);
    return 1;
  }

  BOPAlgo_GlueEnum aGlue = BOPAlgo_GlueEnum(iGlue);
  Objects::SetGlue(aGlue);
  return 0;
}

//=================================================================================================

Standard_Integer bdrawwarnshapes(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 2)
  {
    di.PrintHelp(a[0]);
    return 1;
  }

  Standard_Integer iDraw = Draw1::Atoi(a[1]);
  Objects::SetDrawWarnShapes(iDraw != 0);
  return 0;
}

//=================================================================================================

Standard_Integer bcheckinverted(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 2)
  {
    di.PrintHelp(a[0]);
    return 1;
  }

  Standard_Integer iCheck = Draw1::Atoi(a[1]);
  Objects::SetCheckInverted(iCheck != 0);
  return 0;
}

//=================================================================================================

Standard_Integer buseobb(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 2)
  {
    di.PrintHelp(a[0]);
    return 1;
  }

  Standard_Integer iUse = Draw1::Atoi(a[1]);
  Objects::SetUseOBB(iUse != 0);
  return 0;
}

//=================================================================================================

Standard_Integer bsimplify(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n == 1 || n % 2 == 0)
  {
    di.PrintHelp(a[0]);
    return 1;
  }

  for (Standard_Integer i = 1; i < n - 1; ++i)
  {
    if (!strcmp(a[i], "-e"))
    {
      Standard_Integer iUnifyEdges = Draw1::Atoi(a[++i]);
      Objects::SetUnifyEdges(iUnifyEdges != 0);
    }
    else if (!strcmp(a[i], "-f"))
    {
      Standard_Integer iUnifyFaces = Draw1::Atoi(a[++i]);
      Objects::SetUnifyFaces(iUnifyFaces != 0);
    }
    else if (!strcmp(a[i], "-a"))
    {
      Standard_Real anAngTol = Draw1::Atof(a[++i]) * (M_PI / 180.0);
      Objects::SetAngular(anAngTol);
    }
    else
    {
      di << "Wrong key option.\n";
      di.PrintHelp(a[0]);
      return 1;
    }
  }
  return 0;
}
