// Created on: 1995-02-22
// Created by: Jacques GOUSSARD
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

#include <BRepTest.hxx>
#include <DBRep.hxx>
#include <DrawTrSurf.hxx>
#include <Draw_Appli.hxx>
#include <Draw_Interpretor.hxx>

#include <BRepOffsetAPI_DraftAngle.hxx>
#include <BRepOffsetAPI_MakeDraft.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pln.hxx>
#include <gp_Dir.hxx>

static void Print(DrawInterpreter& di, const Draft_ErrorStatus St)
{
  di << "  Error Status : ";
  switch (St)
  {
    case Draft_NoError:
      di << "No error";
      break;

    case Draft_FaceRecomputation:
      di << "Impossible face recomputation";
      break;

    case Draft_EdgeRecomputation:
      di << "Impossible edge recomputation";
      break;

    case Draft_VertexRecomputation:
      di << "Impossible vertex recomputation";
      break;
  }
}

static Standard_Integer DEP(DrawInterpreter& theCommands, Standard_Integer narg, const char** a)
{
  if ((narg < 14) || (narg % 8 != 6))
    return 1;
  TopoShape             V = DBRep1::Get(a[2]);
  BRepOffsetAPI_DraftAngle drft(V);

  Dir3d Dirextract(Draw1::Atof(a[3]), Draw1::Atof(a[4]), Draw1::Atof(a[5]));

  TopoFace   F;
  Standard_Real Angle;
  Point3d        Pax;
  Dir3d        Dax;
  for (Standard_Integer ii = 0; ii < (narg - 6) / 8; ii++)
  {
    TopoShape aLocalShape(DBRep1::Get(a[8 * ii + 6], TopAbs_FACE));
    F = TopoDS::Face(aLocalShape);
    //    F = TopoDS::Face(DBRep1::Get(a[8*ii+6],TopAbs_FACE));
    Angle = Draw1::Atof(a[8 * ii + 7]) * M_PI / 180.;
    Pax.SetCoord(Draw1::Atof(a[8 * ii + 8]), Draw1::Atof(a[8 * ii + 9]), Draw1::Atof(a[8 * ii + 10]));
    Dax.SetCoord(Draw1::Atof(a[8 * ii + 11]),
                 Draw1::Atof(a[8 * ii + 12]),
                 Draw1::Atof(a[8 * ii + 13]));
    drft.Add(F, Dirextract, Angle, gp_Pln(Pax, Dax));
    if (!drft.AddDone())
    {
      break;
    }
  }

  if (!drft.AddDone())
  {
    DBRep1::Set("bugdep", drft.ProblematicShape());
    theCommands << "Bad shape in variable bugdep ";
    Print(theCommands, drft.Status());
    return 1;
  }
  drft.Build();
  if (drft.IsDone())
  {
    DBRep1::Set(a[1], drft);
    return 0;
  }
  DBRep1::Set("bugdep", drft.ProblematicShape());
  theCommands << "Problem encountered during the reconstruction : ";
  theCommands << "bad shape in variable bugdep; ";
  Print(theCommands, drft.Status());
  return 1;
}

static Standard_Integer NDEP(DrawInterpreter& theCommands, Standard_Integer narg, const char** a)
{
  if ((narg < 15) || ((narg) % 9 != 6))
    return 1;
  TopoShape V = DBRep1::Get(a[2]);
  if (V.IsNull())
  {
    // std::cout << a[2] << " is not a Shape" << std::endl;
    theCommands << a[2] << " is not a Shape\n";
    return 1;
  }

  BRepOffsetAPI_DraftAngle drft(V);

  Dir3d Dirextract(Draw1::Atof(a[3]), Draw1::Atof(a[4]), Draw1::Atof(a[5]));

  TopoFace      F;
  Standard_Real    Angle;
  Point3d           Pax;
  Dir3d           Dax;
  Standard_Boolean Flag;
  for (Standard_Integer ii = 0; ii < (narg - 6) / 9; ii++)
  {
    TopoShape aLocalFace(DBRep1::Get(a[9 * ii + 6], TopAbs_FACE));
    F = TopoDS::Face(aLocalFace);
    //    F = TopoDS::Face(DBRep1::Get(a[9*ii+6],TopAbs_FACE));

    if (F.IsNull())
    {
      // std::cout << a[9*ii+6] << " is not a face" << std::endl;
      theCommands << a[9 * ii + 6] << " is not a face\n";
      return 1;
    }

    Flag  = Draw1::Atoi(a[9 * ii + 7]) != 0;
    Angle = Draw1::Atof(a[9 * ii + 8]) * M_PI / 180.;
    Pax.SetCoord(Draw1::Atof(a[9 * ii + 9]), Draw1::Atof(a[9 * ii + 10]), Draw1::Atof(a[9 * ii + 11]));
    Dax.SetCoord(Draw1::Atof(a[9 * ii + 12]),
                 Draw1::Atof(a[9 * ii + 13]),
                 Draw1::Atof(a[9 * ii + 14]));
    drft.Add(F, Dirextract, Angle, gp_Pln(Pax, Dax), Flag);
    if (!drft.AddDone())
    {
      break;
    }
  }

  if (!drft.AddDone())
  {
    DBRep1::Set("bugdep", drft.ProblematicShape());
    theCommands << "Bad shape in variable bugdep ";
    Print(theCommands, drft.Status());
    return 1;
  }
  drft.Build();
  if (drft.IsDone())
  {
    DBRep1::Set(a[1], drft);
    return 0;
  }
  DBRep1::Set("bugdep", drft.ProblematicShape());
  theCommands << "Problem encountered during the reconstruction : ";
  theCommands << "bad shape in variable bugdep; ";
  Print(theCommands, drft.Status());
  return 1;
}

static Standard_Integer draft(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  Standard_Integer Inside   = -1;
  Standard_Boolean Internal = Standard_False;
  if (n < 8)
    return 1;

  Standard_Real x, y, z, teta;
  TopoShape  SInit = DBRep1::Get(a[2]); // shape d'arret

  x    = Draw1::Atof(a[3]);
  y    = Draw1::Atof(a[4]); // direction de depouille
  z    = Draw1::Atof(a[5]);
  teta = Draw1::Atof(a[6]); // angle de depouille (teta)

  Dir3d D(x, y, z);

  BRepOffsetAPI_MakeDraft MkDraft(SInit, D, teta);

  if (n > 8)
  {
    Standard_Integer cur = 8;
    if (!strcmp(a[cur], "-IN"))
    {
      Inside = 1;
      cur++;
    }
    else if (!strcmp(a[cur], "-OUT"))
    {
      Inside = 0;
      cur++;
    }

    if (cur < n)
    {
      if (!strcmp(a[cur], "-Ri"))
      {
        MkDraft.SetOptions(BRepBuilderAPI_RightCorner);
        cur++;
      }
      else if (!strcmp(a[cur], "-Ro"))
      {
        MkDraft.SetOptions(BRepBuilderAPI_RoundCorner);
        cur++;
      }
    }
    if (cur < n)
    {
      if (!strcmp(a[cur], "-Internal"))
      {
        Internal = Standard_True;
        cur++;
      }
    }
  }

  if (Internal)
  {
    MkDraft.SetDraft(Internal);
    di << "Internal Draft1 : \n";
    // std::cout << "Internal Draft1 : " << std::endl;
  }
  else
    di << "External Draft1 : \n";
  // std::cout << "External Draft1 : " << std::endl;

  TopoShape Stop = DBRep1::Get(a[7]); // shape d'arret
  if (!Stop.IsNull())
  {
    Standard_Boolean KeepOutside = Standard_True;
    if (Inside == 0)
      KeepOutside = Standard_False;
    MkDraft.Perform(Stop, KeepOutside);
  }
  else
  {
    Handle(GeomSurface) Surf = DrawTrSurf1::GetSurface(a[7]);
    if (!Surf.IsNull())
    { // surface d'arret
      Standard_Boolean KeepInside = Standard_True;
      if (Inside == 1)
        KeepInside = Standard_False;
      MkDraft.Perform(Surf, KeepInside);
    }
    else
    { // by Length
      Standard_Real L = Draw1::Atof(a[7]);
      if (L > 1.e-7)
      {
        MkDraft.Perform(L);
      }
      else
        return 1;
    }
  }

  DBRep1::Set(a[1], MkDraft.Shape());
  DBRep1::Set("DraftShell", MkDraft.Shell());

  return 0;
}

//=================================================================================================

void BRepTest1::DraftAngleCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  DBRep1::BasicCommands(theCommands);

  const char* g = "Draft1 angle modification commands";

  theCommands.Add("depouille",
                  " Inclines faces of a shape, dep result shape dirx diry dirz face angle x y x dx "
                  "dy dz [face angle...]",
                  __FILE__,
                  DEP,
                  g);

  theCommands.Add("ndepouille",
                  " Inclines faces of a shape, dep result shape dirx diry dirz face 0/1 angle x y "
                  "x dx dy dz [face 0/1 angle...]",
                  __FILE__,
                  NDEP,
                  g);

  theCommands.Add("draft",
                  " Compute a draft surface along a shape, \n draft result shape dirx diry dirz  "
                  "angle shape/surf/length [-IN/-OUT] [Ri/Ro] [-Internal]",
                  __FILE__,
                  draft,
                  g);
}
