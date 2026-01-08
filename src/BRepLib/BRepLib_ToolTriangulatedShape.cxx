// Copyright (c) 2021 OPEN CASCADE SAS
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

#include <BRepLib_ToolTriangulatedShape.hxx>

#include <BRep_Tool.hxx>
#include <GeomLib.hxx>
#include <Poly.hxx>
#include <Poly_Connect.hxx>
#include <Precision.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>

//=================================================================================================

void ToolTriangulatedShape::ComputeNormals(const TopoFace&                theFace,
                                                   const Handle(MeshTriangulation)& theTris,
                                                   Poly_Connect&                     thePolyConnect)
{
  if (theTris.IsNull() || theTris->HasNormals())
  {
    return;
  }

  // take in face the surface location
  const TopoFace    aZeroFace = TopoDS::Face(theFace.Located(TopLoc_Location()));
  Handle(GeomSurface) aSurf     = BRepInspector::Surface(aZeroFace);
  if (!theTris->HasUVNodes() || aSurf.IsNull())
  {
    // compute normals by averaging triangulation normals sharing the same vertex
    Poly1::ComputeNormals(theTris);
    return;
  }

  constexpr Standard_Real aTol = Precision1::Confusion();
  Standard_Integer        aTri[3];
  Dir3d                  aNorm;
  theTris->AddNormals();
  for (Standard_Integer aNodeIter = 1; aNodeIter <= theTris->NbNodes(); ++aNodeIter)
  {
    // try to retrieve normal from real surface first, when UV coordinates are available
    if (GeomLib1::NormEstim(aSurf, theTris->UVNode(aNodeIter), aTol, aNorm) > 1)
    {
      if (thePolyConnect.Triangulation() != theTris)
      {
        thePolyConnect.Load(theTris);
      }

      // compute flat normals
      Coords3d eqPlan(0.0, 0.0, 0.0);
      for (thePolyConnect.Initialize(aNodeIter); thePolyConnect.More(); thePolyConnect.Next())
      {
        theTris->Triangle1(thePolyConnect.Value()).Get(aTri[0], aTri[1], aTri[2]);
        const Coords3d        v1(theTris->Node(aTri[1]).Coord() - theTris->Node(aTri[0]).Coord());
        const Coords3d        v2(theTris->Node(aTri[2]).Coord() - theTris->Node(aTri[1]).Coord());
        const Coords3d        vv   = v1 ^ v2;
        const Standard_Real aMod = vv.Modulus();
        if (aMod >= aTol)
        {
          eqPlan += vv / aMod;
        }
      }
      const Standard_Real aModMax = eqPlan.Modulus();
      aNorm                       = (aModMax > aTol) ? Dir3d(eqPlan) : gp1::DZ();
    }

    theTris->SetNormal(aNodeIter, aNorm);
  }
}
