// Created on: 1996-02-13
// Created by: Jean Yves LEBEY
// Copyright (c) 1996-1999 Matra Datavision
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

#include <TopOpeBRepBuild_GIter.hxx>
#include <TopOpeBRepBuild_GTool.hxx>
#include <TopOpeBRepBuild_GTopo.hxx>

//=================================================================================================

GTopologyClassifier GTopologyTool::GFusUnsh(const TopAbs_ShapeEnum t1,
                                                      const TopAbs_ShapeEnum t2)
{
  return GTopologyClassifier(false,
                               false,
                               false,
                               false,
                               false,
                               true,
                               false,
                               true,
                               false,
                               t1,
                               t2,
                               TopOpeBRepDS_UNSHGEOMETRY,
                               TopOpeBRepDS_UNSHGEOMETRY);
}

//=================================================================================================

GTopologyClassifier GTopologyTool::GFusSame(const TopAbs_ShapeEnum t1,
                                                      const TopAbs_ShapeEnum t2)
{
  return GTopologyClassifier(false,
                               false,
                               false,
                               false,
                               true,
                               true,
                               false,
                               true,
                               false,
                               t1,
                               t2,
                               TopOpeBRepDS_SAMEORIENTED,
                               TopOpeBRepDS_SAMEORIENTED);
}

//=================================================================================================

GTopologyClassifier GTopologyTool::GFusDiff(const TopAbs_ShapeEnum t1,
                                                      const TopAbs_ShapeEnum t2)
{
  return GTopologyClassifier(false,
                               false,
                               false,
                               false,
                               false,
                               true,
                               false,
                               true,
                               false,
                               t1,
                               t2,
                               TopOpeBRepDS_DIFFORIENTED,
                               TopOpeBRepDS_SAMEORIENTED);
}

//=================================================================================================

GTopologyClassifier GTopologyTool::GCutUnsh(const TopAbs_ShapeEnum t1,
                                                      const TopAbs_ShapeEnum t2)
{
  return GTopologyClassifier(false,
                               true,
                               false,
                               false,
                               false,
                               true,
                               false,
                               false,
                               false,
                               t1,
                               t2,
                               TopOpeBRepDS_UNSHGEOMETRY,
                               TopOpeBRepDS_UNSHGEOMETRY);
}

//=================================================================================================

GTopologyClassifier GTopologyTool::GCutSame(const TopAbs_ShapeEnum t1,
                                                      const TopAbs_ShapeEnum t2)
{
  return GTopologyClassifier(false,
                               true,
                               false,
                               false,
                               false,
                               true,
                               false,
                               false,
                               false,
                               t1,
                               t2,
                               TopOpeBRepDS_SAMEORIENTED,
                               TopOpeBRepDS_SAMEORIENTED);
}

//=================================================================================================

GTopologyClassifier GTopologyTool::GCutDiff(const TopAbs_ShapeEnum t1,
                                                      const TopAbs_ShapeEnum t2)
{
  return GTopologyClassifier(false,
                               true,
                               false,
                               false,
                               true,
                               true,
                               false,
                               false,
                               false,
                               t1,
                               t2,
                               TopOpeBRepDS_DIFFORIENTED,
                               TopOpeBRepDS_SAMEORIENTED);
}

//=================================================================================================

GTopologyClassifier GTopologyTool::GComUnsh(const TopAbs_ShapeEnum t1,
                                                      const TopAbs_ShapeEnum t2)
{
  return GTopologyClassifier(false,
                               true,
                               false,
                               true,
                               false,
                               false,
                               false,
                               false,
                               false,
                               t1,
                               t2,
                               TopOpeBRepDS_UNSHGEOMETRY,
                               TopOpeBRepDS_UNSHGEOMETRY);
}

//=================================================================================================

GTopologyClassifier GTopologyTool::GComSame(const TopAbs_ShapeEnum t1,
                                                      const TopAbs_ShapeEnum t2)
{
  return GTopologyClassifier(false,
                               true,
                               false,
                               true,
                               true,
                               false,
                               false,
                               false,
                               false,
                               t1,
                               t2,
                               TopOpeBRepDS_SAMEORIENTED,
                               TopOpeBRepDS_SAMEORIENTED);
}

//=================================================================================================

GTopologyClassifier GTopologyTool::GComDiff(const TopAbs_ShapeEnum t1,
                                                      const TopAbs_ShapeEnum t2)
{
  return GTopologyClassifier(false,
                               true,
                               false,
                               true,
                               false,
                               false,
                               false,
                               false,
                               false,
                               t1,
                               t2,
                               TopOpeBRepDS_DIFFORIENTED,
                               TopOpeBRepDS_SAMEORIENTED);
}

//=================================================================================================

void GTopologyTool::Dump(Standard_OStream& OS)
{
  GTopologyIterator gi;
  GTopologyClassifier g;

  g = GTopologyTool::GFusUnsh(TopAbs_FACE, TopAbs_FACE);
  g.Dump(OS);
  for (gi.Init(g); gi.More(); gi.Next())
    gi.Dump(OS);
  OS << std::endl;

  g = GTopologyTool::GFusSame(TopAbs_FACE, TopAbs_FACE);
  g.Dump(OS);
  for (gi.Init(g); gi.More(); gi.Next())
    gi.Dump(OS);
  OS << std::endl;

  g = GTopologyTool::GFusDiff(TopAbs_FACE, TopAbs_FACE);
  g.Dump(OS);
  for (gi.Init(g); gi.More(); gi.Next())
    gi.Dump(OS);
  OS << std::endl;

  g = GTopologyTool::GCutDiff(TopAbs_FACE, TopAbs_EDGE);
  g.Dump(OS);
  for (gi.Init(g); gi.More(); gi.Next())
    gi.Dump(OS);
  OS << std::endl;

  g = g.CopyPermuted();
  g.Dump(OS);
  for (gi.Init(g); gi.More(); gi.Next())
    gi.Dump(OS);
  OS << std::endl;
}
