// Created on: 1993-01-21
// Created by: Remi LEQUETTE
// Copyright (c) 1993-1999 Matra Datavision
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

#include <BRepTools.hxx>

#include <Adaptor3d_CurveOnSurface.hxx>
#include <Bnd_Box2d.hxx>
#include <BndLib_Add2dCurve.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_TEdge.hxx>
#include <BRepTools_ShapeSet.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <Message.hxx>
#include <OSD_FileSystem.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <Standard_Failure.hxx>
#include <Standard_Stream.hxx>
#include <TColStd_MapOfTransient.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <GeomLib_CheckCurveOnSurface.hxx>
#include <errno.h>
#include <BRepTools_Modifier.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

//=================================================================================================

void BRepTools1::UVBounds(const TopoFace& F,
                         Standard_Real&     UMin,
                         Standard_Real&     UMax,
                         Standard_Real&     VMin,
                         Standard_Real&     VMax)
{
  Bnd_Box2d B;
  AddUVBounds(F, B);
  if (!B.IsVoid())
  {
    B.Get(UMin, VMin, UMax, VMax);
  }
  else
  {
    UMin = UMax = VMin = VMax = 0.0;
  }
}

//=================================================================================================

void BRepTools1::UVBounds(const TopoFace& F,
                         const TopoWire& W,
                         Standard_Real&     UMin,
                         Standard_Real&     UMax,
                         Standard_Real&     VMin,
                         Standard_Real&     VMax)
{
  Bnd_Box2d B;
  AddUVBounds(F, W, B);
  if (!B.IsVoid())
  {
    B.Get(UMin, VMin, UMax, VMax);
  }
  else
  {
    UMin = UMax = VMin = VMax = 0.0;
  }
}

//=================================================================================================

void BRepTools1::UVBounds(const TopoFace& F,
                         const TopoEdge& E,
                         Standard_Real&     UMin,
                         Standard_Real&     UMax,
                         Standard_Real&     VMin,
                         Standard_Real&     VMax)
{
  Bnd_Box2d B;
  AddUVBounds(F, E, B);
  if (!B.IsVoid())
  {
    B.Get(UMin, VMin, UMax, VMax);
  }
  else
  {
    UMin = UMax = VMin = VMax = 0.0;
  }
}

//=================================================================================================

void BRepTools1::AddUVBounds(const TopoFace& FF, Bnd_Box2d& B)
{
  TopoFace F = FF;
  F.Orientation(TopAbs_FORWARD);
  ShapeExplorer ex(F, TopAbs_EDGE);

  // fill box for the given face
  Bnd_Box2d aBox;
  for (; ex.More(); ex.Next())
  {
    BRepTools1::AddUVBounds(F, TopoDS::Edge(ex.Current()), aBox);
  }

  // if the box is empty (face without edges or without pcurves),
  // get natural bounds
  if (aBox.IsVoid())
  {
    Standard_Real               UMin, UMax, VMin, VMax;
    TopLoc_Location             L;
    const Handle(GeomSurface)& aSurf = BRepInspector::Surface(F, L);
    if (aSurf.IsNull())
    {
      return;
    }

    aSurf->Bounds(UMin, UMax, VMin, VMax);
    aBox.Update(UMin, VMin, UMax, VMax);
  }

  // add face box to result
  B.Add(aBox);
}

//=================================================================================================

void BRepTools1::AddUVBounds(const TopoFace& F, const TopoWire& W, Bnd_Box2d& B)
{
  ShapeExplorer ex;
  for (ex.Init(W, TopAbs_EDGE); ex.More(); ex.Next())
  {
    BRepTools1::AddUVBounds(F, TopoDS::Edge(ex.Current()), B);
  }
}

//=================================================================================================

void BRepTools1::AddUVBounds(const TopoFace& aF, const TopoEdge& aE, Bnd_Box2d& aB)
{
  Standard_Real   aT1, aT2, aXmin = 0.0, aYmin = 0.0, aXmax = 0.0, aYmax = 0.0;
  Standard_Real   aUmin, aUmax, aVmin, aVmax;
  Bnd_Box2d       aBoxC, aBoxS;
  TopLoc_Location aLoc;
  //
  const Handle(GeomCurve2d) aC2D = BRepInspector::CurveOnSurface(aE, aF, aT1, aT2);
  if (aC2D.IsNull())
  {
    return;
  }
  //
  Add2dCurve::Add(aC2D, aT1, aT2, 0., aBoxC);
  if (!aBoxC.IsVoid())
  {
    aBoxC.Get(aXmin, aYmin, aXmax, aYmax);
  }
  //
  Handle(GeomSurface) aS = BRepInspector::Surface(aF, aLoc);
  aS->Bounds(aUmin, aUmax, aVmin, aVmax);

  if (aS->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    const Handle(Geom_RectangularTrimmedSurface) aSt =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(aS);
    aS = aSt->BasisSurface();
  }

  //
  if (!aS->IsUPeriodic())
  {
    Standard_Boolean isUPeriodic = Standard_False;

    // Additional verification for U-periodicity for B-spline surfaces
    // 1. Verify that the surface is U-closed (if such flag is false). Verification uses 2 points
    // 2. Verify periodicity of surface inside UV-bounds of the edge. Verification uses 3 or 6
    // points.
    if (aS->DynamicType() == STANDARD_TYPE(Geom_BSplineSurface) && (aXmin < aUmin || aXmax > aUmax))
    {
      constexpr Standard_Real aTol2 = 100 * Precision::Confusion() * Precision::Confusion();
      isUPeriodic                   = Standard_True;
      Point3d P1, P2;
      // 1. Verify that the surface is U-closed
      if (!aS->IsUClosed())
      {
        Standard_Real aVStep = aVmax - aVmin;
        for (Standard_Real aV = aVmin; aV <= aVmax; aV += aVStep)
        {
          P1 = aS->Value(aUmin, aV);
          P2 = aS->Value(aUmax, aV);
          if (P1.SquareDistance(P2) > aTol2)
          {
            isUPeriodic = Standard_False;
            break;
          }
        }
      }
      // 2. Verify periodicity of surface inside UV-bounds of the edge
      if (isUPeriodic) // the flag still not changed
      {
        Standard_Real    aV = (aVmin + aVmax) * 0.5;
        Standard_Real    aU[6];   // values of U lying out of surface boundaries
        Standard_Real    aUpp[6]; // corresponding U-values plus/minus period
        Standard_Integer aNbPnt = 0;
        if (aXmin < aUmin)
        {
          aU[0]   = aXmin;
          aU[1]   = (aXmin + aUmin) * 0.5;
          aU[2]   = aUmin;
          aUpp[0] = aU[0] + aUmax - aUmin;
          aUpp[1] = aU[1] + aUmax - aUmin;
          aUpp[2] = aU[2] + aUmax - aUmin;
          aNbPnt += 3;
        }
        if (aXmax > aUmax)
        {
          aU[aNbPnt]       = aUmax;
          aU[aNbPnt + 1]   = (aXmax + aUmax) * 0.5;
          aU[aNbPnt + 2]   = aXmax;
          aUpp[aNbPnt]     = aU[aNbPnt] - aUmax + aUmin;
          aUpp[aNbPnt + 1] = aU[aNbPnt + 1] - aUmax + aUmin;
          aUpp[aNbPnt + 2] = aU[aNbPnt + 2] - aUmax + aUmin;
          aNbPnt += 3;
        }
        for (Standard_Integer anInd = 0; anInd < aNbPnt; anInd++)
        {
          P1 = aS->Value(aU[anInd], aV);
          P2 = aS->Value(aUpp[anInd], aV);
          if (P1.SquareDistance(P2) > aTol2)
          {
            isUPeriodic = Standard_False;
            break;
          }
        }
      }
    }

    if (!isUPeriodic)
    {
      if ((aXmin < aUmin) && (aUmin < aXmax))
      {
        aXmin = aUmin;
      }
      if ((aXmin < aUmax) && (aUmax < aXmax))
      {
        aXmax = aUmax;
      }
    }
  }

  if (!aS->IsVPeriodic())
  {
    Standard_Boolean isVPeriodic = Standard_False;

    // Additional verification for V-periodicity for B-spline surfaces
    // 1. Verify that the surface is V-closed (if such flag is false). Verification uses 2 points
    // 2. Verify periodicity of surface inside UV-bounds of the edge. Verification uses 3 or 6
    // points.
    if (aS->DynamicType() == STANDARD_TYPE(Geom_BSplineSurface) && (aYmin < aVmin || aYmax > aVmax))
    {
      constexpr Standard_Real aTol2 = 100 * Precision::Confusion() * Precision::Confusion();
      isVPeriodic                   = Standard_True;
      Point3d P1, P2;
      // 1. Verify that the surface is V-closed
      if (!aS->IsVClosed())
      {
        Standard_Real aUStep = aUmax - aUmin;
        for (Standard_Real aU = aUmin; aU <= aUmax; aU += aUStep)
        {
          P1 = aS->Value(aU, aVmin);
          P2 = aS->Value(aU, aVmax);
          if (P1.SquareDistance(P2) > aTol2)
          {
            isVPeriodic = Standard_False;
            break;
          }
        }
      }
      // 2. Verify periodicity of surface inside UV-bounds of the edge
      if (isVPeriodic) // the flag still not changed
      {
        Standard_Real    aU = (aUmin + aUmax) * 0.5;
        Standard_Real    aV[6];   // values of V lying out of surface boundaries
        Standard_Real    aVpp[6]; // corresponding V-values plus/minus period
        Standard_Integer aNbPnt = 0;
        if (aYmin < aVmin)
        {
          aV[0]   = aYmin;
          aV[1]   = (aYmin + aVmin) * 0.5;
          aV[2]   = aVmin;
          aVpp[0] = aV[0] + aVmax - aVmin;
          aVpp[1] = aV[1] + aVmax - aVmin;
          aVpp[2] = aV[2] + aVmax - aVmin;
          aNbPnt += 3;
        }
        if (aYmax > aVmax)
        {
          aV[aNbPnt]       = aVmax;
          aV[aNbPnt + 1]   = (aYmax + aVmax) * 0.5;
          aV[aNbPnt + 2]   = aYmax;
          aVpp[aNbPnt]     = aV[aNbPnt] - aVmax + aVmin;
          aVpp[aNbPnt + 1] = aV[aNbPnt + 1] - aVmax + aVmin;
          aVpp[aNbPnt + 2] = aV[aNbPnt + 2] - aVmax + aVmin;
          aNbPnt += 3;
        }
        for (Standard_Integer anInd = 0; anInd < aNbPnt; anInd++)
        {
          P1 = aS->Value(aU, aV[anInd]);
          P2 = aS->Value(aU, aVpp[anInd]);
          if (P1.SquareDistance(P2) > aTol2)
          {
            isVPeriodic = Standard_False;
            break;
          }
        }
      }
    }

    if (!isVPeriodic)
    {
      if ((aYmin < aVmin) && (aVmin < aYmax))
      {
        aYmin = aVmin;
      }
      if ((aYmin < aVmax) && (aVmax < aYmax))
      {
        aYmax = aVmax;
      }
    }
  }

  aBoxS.Update(aXmin, aYmin, aXmax, aYmax);

  aB.Add(aBoxS);
}

//=================================================================================================

void BRepTools1::Update(const TopoVertex&) {}

//=================================================================================================

void BRepTools1::Update(const TopoEdge&) {}

//=================================================================================================

void BRepTools1::Update(const TopoWire&) {}

//=================================================================================================

void BRepTools1::Update(const TopoFace& F)
{
  if (!F.Checked())
  {
    UpdateFaceUVPoints(F);
    F.TShape()->Checked(Standard_True);
  }
}

//=================================================================================================

void BRepTools1::Update(const TopoShell& S)
{
  ShapeExplorer ex(S, TopAbs_FACE);
  while (ex.More())
  {
    Update(TopoDS::Face(ex.Current()));
    ex.Next();
  }
}

//=================================================================================================

void BRepTools1::Update(const TopoSolid& S)
{
  ShapeExplorer ex(S, TopAbs_FACE);
  while (ex.More())
  {
    Update(TopoDS::Face(ex.Current()));
    ex.Next();
  }
}

//=================================================================================================

void BRepTools1::Update(const TopoDS_CompSolid& CS)
{
  ShapeExplorer ex(CS, TopAbs_FACE);
  while (ex.More())
  {
    Update(TopoDS::Face(ex.Current()));
    ex.Next();
  }
}

//=================================================================================================

void BRepTools1::Update(const TopoCompound& C)
{
  ShapeExplorer ex(C, TopAbs_FACE);
  while (ex.More())
  {
    Update(TopoDS::Face(ex.Current()));
    ex.Next();
  }
}

//=================================================================================================

void BRepTools1::Update(const TopoShape& S)
{
  switch (S.ShapeType())
  {

    case TopAbs_VERTEX:
      Update(TopoDS::Vertex(S));
      break;

    case TopAbs_EDGE:
      Update(TopoDS::Edge(S));
      break;

    case TopAbs_WIRE:
      Update(TopoDS::Wire(S));
      break;

    case TopAbs_FACE:
      Update(TopoDS::Face(S));
      break;

    case TopAbs_SHELL:
      Update(TopoDS::Shell(S));
      break;

    case TopAbs_SOLID:
      Update(TopoDS::Solid(S));
      break;

    case TopAbs_COMPSOLID:
      Update(TopoDS::CompSolid(S));
      break;

    case TopAbs_COMPOUND:
      Update(TopoDS::Compound(S));
      break;

    default:
      break;
  }
}

//=======================================================================
// function : UpdateFaceUVPoints
// purpose  : Reset the UV points of edges on the Face
//=======================================================================
void BRepTools1::UpdateFaceUVPoints(const TopoFace& theF)
{
  // For each edge of the face <F> reset the UV points to the bounding
  // points of the parametric curve of the edge on the face.

  // Get surface of the face
  TopLoc_Location             aLoc;
  const Handle(GeomSurface)& aSurf = BRepInspector::Surface(theF, aLoc);
  // Iterate on edges and reset UV points
  ShapeExplorer anExpE(theF, TopAbs_EDGE);
  for (; anExpE.More(); anExpE.Next())
  {
    const TopoEdge& aE = TopoDS::Edge(anExpE.Current());

    const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&aE.TShape());
    if (TE->Locked())
      return;

    const TopLoc_Location aELoc = aLoc.Predivided(aE.Location());
    // Edge representations
    BRep_ListOfCurveRepresentation&              aLCR = TE->ChangeCurves();
    BRep_ListIteratorOfListOfCurveRepresentation itLCR(aLCR);
    for (; itLCR.More(); itLCR.Next())
    {
      Handle(BRep_GCurve) GC = Handle(BRep_GCurve)::DownCast(itLCR.Value());
      if (!GC.IsNull() && GC->IsCurveOnSurface(aSurf, aELoc))
      {
        // Update UV points
        GC->Update();
        break;
      }
    }
  }
}

//=================================================================================================

Standard_Boolean BRepTools1::Compare(const TopoVertex& V1, const TopoVertex& V2)
{
  if (V1.IsSame(V2))
    return Standard_True;
  Point3d        p1 = BRepInspector::Pnt(V1);
  Point3d        p2 = BRepInspector::Pnt(V2);
  Standard_Real l  = p1.Distance(p2);
  if (l <= BRepInspector::Tolerance(V1))
    return Standard_True;
  if (l <= BRepInspector::Tolerance(V2))
    return Standard_True;
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRepTools1::Compare(const TopoEdge& E1, const TopoEdge& E2)
{
  if (E1.IsSame(E2))
    return Standard_True;
  return Standard_False;
}

//=================================================================================================

TopoWire BRepTools1::OuterWire(const TopoFace& F)
{
  TopoWire     Wres;
  ShapeExplorer expw(F, TopAbs_WIRE);

  if (expw.More())
  {
    Wres = TopoDS::Wire(expw.Current());
    expw.Next();
    if (expw.More())
    {
      Standard_Real UMin, UMax, VMin, VMax;
      Standard_Real umin, umax, vmin, vmax;
      BRepTools1::UVBounds(F, Wres, UMin, UMax, VMin, VMax);
      while (expw.More())
      {
        const TopoWire& W = TopoDS::Wire(expw.Current());
        BRepTools1::UVBounds(F, W, umin, umax, vmin, vmax);
        if (((umin - UMin) <= Precision::PConfusion())
            && ((umax - UMax) >= -Precision::PConfusion())
            && ((vmin - VMin) <= Precision::PConfusion())
            && ((vmax - VMax) >= -Precision::PConfusion()))
        {
          Wres = W;
          UMin = umin;
          UMax = umax;
          VMin = vmin;
          VMax = vmax;
        }
        expw.Next();
      }
    }
  }
  return Wres;
}

//=================================================================================================

void BRepTools1::Map3DEdges(const TopoShape& S, TopTools_IndexedMapOfShape& M)
{
  ShapeExplorer Ex;
  for (Ex.Init(S, TopAbs_EDGE); Ex.More(); Ex.Next())
  {
    if (!BRepInspector::Degenerated(TopoDS::Edge(Ex.Current())))
      M.Add(Ex.Current());
  }
}

//=================================================================================================

void BRepTools1::Dump(const TopoShape& Sh, Standard_OStream& S)
{
  BRepTools_ShapeSet SS;
  SS.Add(Sh);
  SS.Dump(Sh, S);
  SS.Dump(S);
}

//=================================================================================================

void BRepTools1::Write(const TopoShape&          theShape,
                      Standard_OStream&            theStream,
                      const Standard_Boolean       theWithTriangles,
                      const Standard_Boolean       theWithNormals,
                      const TopTools_FormatVersion theVersion,
                      const Message_ProgressRange& theProgress)
{
  BRepTools_ShapeSet aShapeSet(theWithTriangles, theWithNormals);
  aShapeSet.SetFormatNb(theVersion);
  aShapeSet.Add(theShape);
  aShapeSet.Write(theStream, theProgress);
  aShapeSet.Write(theShape, theStream);
}

//=================================================================================================

void BRepTools1::Read(TopoShape&                Sh,
                     std::istream&                S,
                     const ShapeBuilder&          B,
                     const Message_ProgressRange& theProgress)
{
  BRepTools_ShapeSet SS(B);
  SS.Read(S, theProgress);
  SS.Read(Sh, S);
}

//=================================================================================================

Standard_Boolean BRepTools1::Write(const TopoShape&          theShape,
                                  const Standard_CString       theFile,
                                  const Standard_Boolean       theWithTriangles,
                                  const Standard_Boolean       theWithNormals,
                                  const TopTools_FormatVersion theVersion,
                                  const Message_ProgressRange& theProgress)
{
  const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
  std::shared_ptr<std::ostream> aStream =
    aFileSystem->OpenOStream(theFile, std::ios::out | std::ios::binary);
  if (aStream.get() == NULL || !aStream->good())
  {
    return Standard_False;
  }

  Standard_Boolean isGood = (aStream->good() && !aStream->eof());
  if (!isGood)
    return isGood;

  BRepTools_ShapeSet SS(theWithTriangles, theWithNormals);
  SS.SetFormatNb(theVersion);
  SS.Add(theShape);

  *aStream << "DBRep_DrawableShape\n"; // for easy Draw1 read
  SS.Write(*aStream, theProgress);
  isGood = aStream->good();
  if (isGood)
  {
    SS.Write(theShape, *aStream);
  }
  aStream->flush();
  isGood = aStream->good();

  errno  = 0;
  isGood = aStream->good() && isGood && !errno;
  aStream.reset();

  return isGood;
}

//=================================================================================================

Standard_Boolean BRepTools1::Read(TopoShape&                Sh,
                                 const Standard_CString       File,
                                 const ShapeBuilder&          B,
                                 const Message_ProgressRange& theProgress)
{
  const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
  std::shared_ptr<std::istream> aStream     = aFileSystem->OpenIStream(File, std::ios::in);
  if (aStream.get() == NULL)
  {
    return Standard_False;
  }
  BRepTools_ShapeSet SS(B);
  SS.Read(*aStream, theProgress);
  if (!SS.NbShapes1())
    return Standard_False;
  SS.Read(Sh, *aStream);
  return Standard_True;
}

//=================================================================================================

void BRepTools1::Clean(const TopoShape& theShape, const Standard_Boolean theForce)
{
  if (theShape.IsNull())
    return;

  ShapeBuilder                        aBuilder;
  Handle(MeshTriangulation)          aNullTriangulation;
  Handle(Poly_PolygonOnTriangulation) aNullPoly;

  TopTools_MapOfShape   aShapeMap;
  const TopLoc_Location anEmptyLoc;

  ShapeExplorer aFaceIt(theShape, TopAbs_FACE);
  for (; aFaceIt.More(); aFaceIt.Next())
  {
    TopoShape aFaceNoLoc = aFaceIt.Value();
    aFaceNoLoc.Location(anEmptyLoc);
    if (!aShapeMap.Add(aFaceNoLoc))
    {
      // the face has already been processed
      continue;
    }

    const TopoFace& aFace = TopoDS::Face(aFaceIt.Current());
    if (!BRepInspector::IsGeometric(aFace))
    {
      // Do not remove triangulation as there is no surface to recompute it.
      continue;
    }

    TopLoc_Location                   aLoc;
    const Handle(MeshTriangulation)& aTriangulation = BRepInspector::Triangulation(aFace, aLoc);

    if (aTriangulation.IsNull())
      continue;

    // Nullify edges
    // Theoretically, the edges on the face (with surface) may have no geometry
    // (no curve 3d or 2d or both). Such faces should be considered as invalid and
    // are not supported by current implementation. So, both triangulation of the face
    // and polygon on triangulation of the edges are removed unconditionally.
    ShapeExplorer aEdgeIt(aFace, TopAbs_EDGE);
    for (; aEdgeIt.More(); aEdgeIt.Next())
    {
      const TopoEdge& anEdge = TopoDS::Edge(aEdgeIt.Current());
      aBuilder.UpdateEdge(anEdge, aNullPoly, aTriangulation, aLoc);
    }

    aBuilder.UpdateFace(aFace, aNullTriangulation);
  }

  // Iterate over all edges seeking for 3d polygons
  Handle(Poly_Polygon3D) aNullPoly3d;
  ShapeExplorer        aEdgeIt(theShape, TopAbs_EDGE);
  for (; aEdgeIt.More(); aEdgeIt.Next())
  {
    TopoEdge anEdge = TopoDS::Edge(aEdgeIt.Value());
    anEdge.Location(anEmptyLoc);

    if (!aShapeMap.Add(anEdge))
    {
      // the edge has already been processed
      continue;
    }

    if (!BRepInspector::IsGeometric(TopoDS::Edge(anEdge)))
    {
      // Do not remove polygon 3d as there is no curve to recompute it.
      continue;
    }

    TopLoc_Location        aLoc;
    Handle(Poly_Polygon3D) aPoly3d = BRepInspector::Polygon3D(anEdge, aLoc);

    if (!aPoly3d.IsNull())
    {
      aBuilder.UpdateEdge(anEdge, aNullPoly3d);
    }
    if (theForce)
    {
      Handle(BRep_CurveRepresentation) aCR;
      BRep_TEdge*                      aTE  = static_cast<BRep_TEdge*>(anEdge.TShape().get());
      BRep_ListOfCurveRepresentation&  aLCR = aTE->ChangeCurves();
      BRep_ListIteratorOfListOfCurveRepresentation anIterCR(aLCR);

      // find and remove all representations
      while (anIterCR.More())
      {
        aCR = anIterCR.Value();
        if (aCR->IsPolygonOnTriangulation())
        {
          aLCR.Remove(anIterCR);
        }
        else
        {
          anIterCR.Next();
        }
      }
      aTE->Modified(Standard_True);
    }
  }
}

//=================================================================================================

void BRepTools1::CleanGeometry(const TopoShape& theShape)
{
  if (theShape.IsNull())
    return;

  ShapeBuilder aBuilder;

  for (ShapeExplorer aFaceIt(theShape, TopAbs_FACE); aFaceIt.More(); aFaceIt.Next())
  {
    TopLoc_Location             aLocation;
    const TopoFace&          aFace    = TopoDS::Face(aFaceIt.Current());
    const Handle(GeomSurface)& aSurface = BRepInspector::Surface(aFace, aLocation);

    for (ShapeExplorer aEdgeIt(aFace, TopAbs_EDGE); aEdgeIt.More(); aEdgeIt.Next())
    {
      const TopoEdge& anEdge = TopoDS::Edge(aEdgeIt.Current());
      aBuilder.UpdateEdge(anEdge,
                          Handle(GeomCurve2d)(),
                          aSurface,
                          aLocation,
                          BRepInspector::Tolerance(anEdge));
    }

    aBuilder.UpdateFace(aFace,
                        Handle(GeomSurface)(),
                        aFace.Location(),
                        BRepInspector::Tolerance(aFace));
  }

  for (ShapeExplorer aEdgeIt2(theShape, TopAbs_EDGE); aEdgeIt2.More(); aEdgeIt2.Next())
  {
    const TopoEdge& anEdge = TopoDS::Edge(aEdgeIt2.Current());
    aBuilder.UpdateEdge(anEdge,
                        Handle(GeomCurve3d)(),
                        TopLoc_Location(),
                        BRepInspector::Tolerance(anEdge));
  }

  RemoveUnusedPCurves(theShape);
}

//=================================================================================================

void BRepTools1::RemoveUnusedPCurves(const TopoShape& S)
{
  TColStd_MapOfTransient UsedSurfaces;

  ShapeExplorer Explo(S, TopAbs_FACE);
  for (; Explo.More(); Explo.Next())
  {
    TopoFace          aFace = TopoDS::Face(Explo.Current());
    TopLoc_Location      aLoc;
    Handle(GeomSurface) aSurf = BRepInspector::Surface(aFace, aLoc);
    UsedSurfaces.Add(aSurf);
  }

  TopTools_IndexedMapOfShape Emap;
  TopExp1::MapShapes(S, TopAbs_EDGE, Emap);

  Standard_Integer i;
  for (i = 1; i <= Emap.Extent(); i++)
  {
    const Handle(BRep_TEdge)&                    TE  = *((Handle(BRep_TEdge)*)&Emap(i).TShape());
    BRep_ListOfCurveRepresentation&              lcr = TE->ChangeCurves();
    BRep_ListIteratorOfListOfCurveRepresentation itrep(lcr);
    while (itrep.More())
    {
      Standard_Boolean ToRemove = Standard_False;

      Handle(BRep_CurveRepresentation) CurveRep = itrep.Value();
      if (CurveRep->IsCurveOnSurface())
      {
        Handle(GeomSurface) aSurface = CurveRep->Surface();
        if (!UsedSurfaces.Contains(aSurface))
          ToRemove = Standard_True;
      }
      else if (CurveRep->IsRegularity())
      {
        Handle(GeomSurface) Surf1 = CurveRep->Surface();
        Handle(GeomSurface) Surf2 = CurveRep->Surface2();
        ToRemove = (!UsedSurfaces.Contains(Surf1) || !UsedSurfaces.Contains(Surf2));
      }

      if (ToRemove)
        lcr.Remove(itrep);
      else
        itrep.Next();
    }
  }
}

//=================================================================================================

Standard_Boolean BRepTools1::Triangulation(const TopoShape&    theShape,
                                          const Standard_Real    theLinDefl,
                                          const Standard_Boolean theToCheckFreeEdges)
{
  ShapeExplorer anEdgeIter;
  TopLoc_Location aDummyLoc;
  for (ShapeExplorer aFaceIter(theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoFace&                aFace = TopoDS::Face(aFaceIter.Current());
    const Handle(MeshTriangulation)& aTri  = BRepInspector::Triangulation(aFace, aDummyLoc);
    if (aTri.IsNull() || aTri->Deflection() > theLinDefl)
    {
      return Standard_False;
    }

    for (anEdgeIter.Init(aFace, TopAbs_EDGE); anEdgeIter.More(); anEdgeIter.Next())
    {
      const TopoEdge&                         anEdge = TopoDS::Edge(anEdgeIter.Current());
      const Handle(Poly_PolygonOnTriangulation)& aPoly =
        BRepInspector::PolygonOnTriangulation(anEdge, aTri, aDummyLoc);
      if (aPoly.IsNull())
      {
        return Standard_False;
      }
    }
  }
  if (!theToCheckFreeEdges)
  {
    return Standard_True;
  }

  Handle(MeshTriangulation) anEdgeTri;
  for (anEdgeIter.Init(theShape, TopAbs_EDGE, TopAbs_FACE); anEdgeIter.More(); anEdgeIter.Next())
  {
    const TopoEdge&            anEdge   = TopoDS::Edge(anEdgeIter.Current());
    const Handle(Poly_Polygon3D)& aPolygon = BRepInspector::Polygon3D(anEdge, aDummyLoc);
    if (!aPolygon.IsNull())
    {
      if (aPolygon->Deflection() > theLinDefl)
      {
        return Standard_False;
      }
    }
    else
    {
      const Handle(Poly_PolygonOnTriangulation)& aPoly =
        BRepInspector::PolygonOnTriangulation(anEdge, anEdgeTri, aDummyLoc);
      if (aPoly.IsNull() || anEdgeTri.IsNull() || anEdgeTri->Deflection() > theLinDefl)
      {
        return Standard_False;
      }
    }
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools1::LoadTriangulation(const TopoShape&           theShape,
                                              const Standard_Integer        theTriangulationIdx,
                                              const Standard_Boolean        theToSetAsActive,
                                              const Handle(OSD_FileSystem)& theFileSystem)
{
  Standard_ASSERT_RAISE(theTriangulationIdx >= -1, "Invalid negative triangulation index!");

  Standard_Boolean              wasLoaded = false;
  ShapeBuilder                  aBuilder;
  TopLoc_Location               aDummyLoc;
  const Handle(OSD_FileSystem)& aFileSystem =
    !theFileSystem.IsNull() ? theFileSystem : OSD_FileSystem::DefaultFileSystem();
  for (ShapeExplorer aFaceIter(theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoFace&         aFace = TopoDS::Face(aFaceIter.Current());
    Handle(MeshTriangulation) aTriangulation;
    if (theTriangulationIdx == -1)
    {
      // load an active triangulation
      aTriangulation = BRepInspector::Triangulation(aFace, aDummyLoc);
    }
    else
    {
      const Poly_ListOfTriangulation& aTriangulations = BRepInspector::Triangulations(aFace, aDummyLoc);
      if (theTriangulationIdx >= aTriangulations.Size())
      {
        // triangulation index is out of range
        continue;
      }
      Standard_Integer aTriangulationIdx = 0;
      for (Poly_ListOfTriangulation::Iterator anIter(aTriangulations); anIter.More();
           anIter.Next(), aTriangulationIdx++)
      {
        if (aTriangulationIdx != theTriangulationIdx)
        {
          continue;
        }
        aTriangulation = anIter.Value();
        break;
      }
    }
    if (aTriangulation.IsNull() || !aTriangulation->HasDeferredData())
    {
      // NULL triangulation, already loaded triangulation or triangulation without deferred storage
      // cannot be loaded
      continue;
    }
    if (aTriangulation->LoadDeferredData(aFileSystem))
    {
      wasLoaded = true;
      if (theToSetAsActive && (theTriangulationIdx != -1)) // triangulation is already active
      {
        aBuilder.UpdateFace(aFace, aTriangulation, false);
      }
    }
  }
  return wasLoaded;
}

//=================================================================================================

Standard_Boolean BRepTools1::LoadAllTriangulations(const TopoShape&           theShape,
                                                  const Handle(OSD_FileSystem)& theFileSystem)
{
  Standard_Boolean              wasLoaded = false;
  TopLoc_Location               aDummyLoc;
  const Handle(OSD_FileSystem)& aFileSystem =
    !theFileSystem.IsNull() ? theFileSystem : OSD_FileSystem::DefaultFileSystem();
  for (ShapeExplorer aFaceIter(theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoFace& aFace = TopoDS::Face(aFaceIter.Current());
    for (Poly_ListOfTriangulation::Iterator anIter(BRepInspector::Triangulations(aFace, aDummyLoc));
         anIter.More();
         anIter.Next())
    {
      const Handle(MeshTriangulation)& aTriangulation = anIter.Value();
      if (aTriangulation.IsNull() || !aTriangulation->HasDeferredData())
      {
        // NULL triangulation, already loaded triangulation or triangulation without deferred
        // storage cannot be loaded
        continue;
      }
      wasLoaded = aTriangulation->LoadDeferredData(aFileSystem);
    }
  }
  return wasLoaded;
}

//=================================================================================================

Standard_Boolean BRepTools1::UnloadTriangulation(const TopoShape&    theShape,
                                                const Standard_Integer theTriangulationIdx)
{
  Standard_ASSERT_RAISE(theTriangulationIdx >= -1, "Invalid negative triangulation index!");

  Standard_Boolean wasUnloaded = false;
  TopLoc_Location  aDummyLoc;
  for (ShapeExplorer aFaceIter(theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoFace&         aFace = TopoDS::Face(aFaceIter.Current());
    Handle(MeshTriangulation) aTriangulation;
    if (theTriangulationIdx == -1)
    {
      // unload an active triangulation
      aTriangulation = BRepInspector::Triangulation(aFace, aDummyLoc);
    }
    else
    {
      Standard_Integer                aTriangulationIdx = 0;
      const Poly_ListOfTriangulation& aTriangulations = BRepInspector::Triangulations(aFace, aDummyLoc);
      if (theTriangulationIdx >= aTriangulations.Size())
      {
        // triangulation index is out of range
        continue;
      }
      for (Poly_ListOfTriangulation::Iterator anIter(aTriangulations); anIter.More();
           anIter.Next(), aTriangulationIdx++)
      {
        if (aTriangulationIdx != theTriangulationIdx)
        {
          continue;
        }
        aTriangulation = anIter.Value();
        break;
      }
    }
    if (aTriangulation.IsNull() || !aTriangulation->HasDeferredData())
    {
      // NULL triangulation or triangulation without deferred storage cannot be unloaded
      continue;
    }
    wasUnloaded = aTriangulation->UnloadDeferredData();
  }
  return wasUnloaded;
}

//=================================================================================================

Standard_Boolean BRepTools1::UnloadAllTriangulations(const TopoShape& theShape)
{
  Standard_Boolean wasUnloaded = false;
  TopLoc_Location  aDummyLoc;
  for (ShapeExplorer aFaceIter(theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoFace&         aFace = TopoDS::Face(aFaceIter.Current());
    Handle(MeshTriangulation) aTriangulation;
    for (Poly_ListOfTriangulation::Iterator anIter(BRepInspector::Triangulations(aFace, aDummyLoc));
         anIter.More();
         anIter.Next())
    {
      aTriangulation = anIter.Value();
      if (aTriangulation.IsNull() || !aTriangulation->HasDeferredData())
      {
        // NULL triangulation or triangulation without deferred storage cannot be unloaded
        continue;
      }
      wasUnloaded = aTriangulation->UnloadDeferredData();
    }
  }
  return wasUnloaded;
}

//=================================================================================================

Standard_Boolean BRepTools1::ActivateTriangulation(const TopoShape&    theShape,
                                                  const Standard_Integer theTriangulationIdx,
                                                  const Standard_Boolean theToActivateStrictly)
{
  Standard_ASSERT_RAISE(theTriangulationIdx > -1, "Invalid negative triangulation index!");

  Standard_Boolean wasActivated = false;
  ShapeBuilder     aBuilder;
  TopLoc_Location  aDummyLoc;
  for (ShapeExplorer aFaceIter(theShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoFace&              aFace             = TopoDS::Face(aFaceIter.Current());
    Standard_Integer                aTriangulationIdx = theTriangulationIdx;
    const Poly_ListOfTriangulation& aTriangulations   = BRepInspector::Triangulations(aFace, aDummyLoc);
    const Standard_Integer          aTriangulationsNb = aTriangulations.Size();
    if (theTriangulationIdx >= aTriangulationsNb)
    {
      // triangulation index is out of range
      if (theToActivateStrictly)
      {
        // skip activation
        continue;
      }
      // use last available
      aTriangulationIdx = aTriangulationsNb - 1;
    }
    Handle(MeshTriangulation) anActiveTriangulation;
    Standard_Integer           aTriangulationIter = 0;
    for (Poly_ListOfTriangulation::Iterator anIter(aTriangulations); anIter.More();
         anIter.Next(), aTriangulationIter++)
    {
      if (aTriangulationIter != aTriangulationIdx)
      {
        continue;
      }
      anActiveTriangulation = anIter.Value();
      break;
    }
    if (anActiveTriangulation.IsNull())
    {
      continue;
    }
    aBuilder.UpdateFace(aFace, anActiveTriangulation, false);
    wasActivated = true;
  }
  return wasActivated;
}

//=================================================================================================

Standard_Boolean BRepTools1::IsReallyClosed(const TopoEdge& E, const TopoFace& F)
{
  if (!BRepInspector::IsClosed(E, F))
  {
    return Standard_False;
  }
  Standard_Integer nbocc = 0;
  ShapeExplorer  exp;
  for (exp.Init(F, TopAbs_EDGE); exp.More(); exp.Next())
  {
    if (exp.Current().IsSame(E))
    {
      nbocc++;
    }
  }
  return nbocc == 2;
}

//=================================================================================================

void BRepTools1::DetectClosedness(const TopoFace& theFace,
                                 Standard_Boolean&  theUclosed,
                                 Standard_Boolean&  theVclosed)
{
  theUclosed = theVclosed = Standard_False;

  ShapeExplorer Explo(theFace, TopAbs_EDGE);
  for (; Explo.More(); Explo.Next())
  {
    const TopoEdge& anEdge = TopoDS::Edge(Explo.Current());
    if (BRepInspector::IsClosed(anEdge, theFace) && BRepTools1::IsReallyClosed(anEdge, theFace))
    {
      Standard_Real        fpar, lpar;
      Handle(GeomCurve2d) PCurve1 = BRepInspector::CurveOnSurface(anEdge, theFace, fpar, lpar);
      Handle(GeomCurve2d) PCurve2 =
        BRepInspector::CurveOnSurface(TopoDS::Edge(anEdge.Reversed()), theFace, fpar, lpar);
      gp_Pnt2d         Point1 = PCurve1->Value(fpar);
      gp_Pnt2d         Point2 = PCurve2->Value(fpar);
      Standard_Boolean IsUiso = (Abs(Point1.X() - Point2.X()) > Abs(Point1.Y() - Point2.Y()));
      if (IsUiso)
        theUclosed = Standard_True;
      else
        theVclosed = Standard_True;
    }
  }
}

//=================================================================================================

Standard_Real BRepTools1::EvalAndUpdateTol(const TopoEdge&          theE,
                                          const Handle(GeomCurve3d)&   C3d,
                                          const Handle(GeomCurve2d)& C2d,
                                          const Handle(GeomSurface)& S,
                                          const Standard_Real         f,
                                          const Standard_Real         l)
{
  Standard_Real newtol = 0.;
  Standard_Real first = f, last = l;
  // Set first, last to avoid ErrosStatus = 2 because of
  // too strong checking of limits in class CheckCurveOnSurface
  //
  if (!C3d->IsPeriodic())
  {
    first = Max(first, C3d->FirstParameter());
    last  = Min(last, C3d->LastParameter());
  }
  if (!C2d->IsPeriodic())
  {
    first = Max(first, C2d->FirstParameter());
    last  = Min(last, C2d->LastParameter());
  }
  const Handle(Adaptor3d_Curve) aGeomAdaptorCurve = new GeomAdaptor_Curve(C3d, first, last);

  Handle(Adaptor2d_Curve2d)   aGeom2dAdaptorCurve = new Geom2dAdaptor_Curve(C2d, first, last);
  Handle(GeomAdaptor_Surface) aGeomAdaptorSurface = new GeomAdaptor_Surface(S);

  Handle(Adaptor3d_CurveOnSurface) anAdaptor3dCurveOnSurface =
    new Adaptor3d_CurveOnSurface(aGeom2dAdaptorCurve, aGeomAdaptorSurface);

  GeomLib_CheckCurveOnSurface CT(aGeomAdaptorCurve);
  CT.Perform(anAdaptor3dCurveOnSurface);
  if (CT.IsDone())
  {
    newtol = CT.MaxDistance();
  }
  else
  {
    if (CT.ErrorStatus() == 3
        || (CT.ErrorStatus() == 2 && (C3d->IsPeriodic() || C2d->IsPeriodic())))
    {
      // Try to estimate by sample points
      Standard_Integer nbint = 22;
      Standard_Real    dt    = (last - first) / nbint;
      dt                     = Max(dt, Precision::Confusion());
      Standard_Real    d, dmax = 0.;
      gp_Pnt2d         aP2d;
      Point3d           aPC, aPS;
      Standard_Integer cnt = 0;
      Standard_Real    t   = first;
      for (; t <= last; t += dt)
      {
        cnt++;
        C2d->D0(t, aP2d);
        C3d->D0(t, aPC);
        S->D0(aP2d.X(), aP2d.Y(), aPS);
        d = aPS.SquareDistance(aPC);
        if (d > dmax)
        {
          dmax = d;
        }
      }
      if (cnt < nbint + 1)
      {
        t = last;
        C2d->D0(t, aP2d);
        C3d->D0(t, aPC);
        S->D0(aP2d.X(), aP2d.Y(), aPS);
        d = aPS.SquareDistance(aPC);
        if (d > dmax)
        {
          dmax = d;
        }
      }

      newtol = 1.2 * Sqrt(dmax);
    }
  }
  Standard_Real Tol = BRepInspector::Tolerance(theE);
  if (newtol > Tol)
  {
    Tol = newtol;
    ShapeBuilder B;
    B.UpdateEdge(theE, Tol);
  }

  return Tol;
}

//=================================================================================================

TopAbs_Orientation BRepTools1::OriEdgeInFace(const TopoEdge& E, const TopoFace& F)

{
  ShapeExplorer Exp(F.Oriented(TopAbs_FORWARD), TopAbs_EDGE);

  for (; Exp.More(); Exp.Next())
  {
    if (Exp.Current().IsSame(E))
    {
      return Exp.Current().Orientation();
    }
  }
  throw Standard_ConstructionError("BRepTools1::OriEdgeInFace");
}

namespace
{
//=======================================================================
// function : findInternalsToKeep
// purpose  : Looks for internal sub-shapes which has to be kept to preserve
//           topological connectivity.
//=======================================================================
static void findInternalsToKeep(const TopoShape&  theS,
                                TopTools_MapOfShape& theAllNonInternals,
                                TopTools_MapOfShape& theAllInternals,
                                TopTools_MapOfShape& theShapesToKeep)
{
  for (TopoDS_Iterator it(theS, Standard_True); it.More(); it.Next())
  {
    const TopoShape& aSS = it.Value();
    findInternalsToKeep(aSS, theAllNonInternals, theAllInternals, theShapesToKeep);

    if (aSS.Orientation() == TopAbs_INTERNAL)
      theAllInternals.Add(aSS);
    else
      theAllNonInternals.Add(aSS);

    if (theAllNonInternals.Contains(aSS) && theAllInternals.Contains(aSS))
      theShapesToKeep.Add(aSS);
  }
}

//=======================================================================
// function : removeShapes
// purpose  : Removes sub-shapes from the shape
//=======================================================================
static void removeShapes(TopoShape& theS, const ShapeList& theLS)
{
  ShapeBuilder     aBB;
  Standard_Boolean isFree = theS.Free();
  theS.Free(Standard_True);

  for (ShapeList::Iterator it(theLS); it.More(); it.Next())
  {
    aBB.Remove(theS, it.Value());
  }
  theS.Free(isFree);
}

//=======================================================================
// function : removeInternals
// purpose  : Removes recursively all internal sub-shapes from the given shape.
//           Returns true if all sub-shapes have been removed from the shape.
//=======================================================================
static Standard_Boolean removeInternals(TopoShape&              theS,
                                        const TopTools_MapOfShape* theShapesToKeep)
{
  ShapeList aLRemove;
  for (TopoDS_Iterator it(theS, Standard_True); it.More(); it.Next())
  {
    const TopoShape& aSS = it.Value();
    if (aSS.Orientation() == TopAbs_INTERNAL)
    {
      if (!theShapesToKeep || !theShapesToKeep->Contains(aSS))
        aLRemove.Append(aSS);
    }
    else
    {
      if (removeInternals(*(TopoShape*)&aSS, theShapesToKeep))
        aLRemove.Append(aSS);
    }
  }

  Standard_Integer aNbSToRemove = aLRemove.Extent();
  if (aNbSToRemove)
  {
    removeShapes(theS, aLRemove);
    return (theS.NbChildren() == 0);
  }
  return Standard_False;
}

} // namespace

//=================================================================================================

void BRepTools1::RemoveInternals(TopoShape& theS, const Standard_Boolean theForce)
{
  TopTools_MapOfShape *pMKeep = NULL, aMKeep;
  if (!theForce)
  {
    // Find all internal sub-shapes which has to be kept to preserve topological connectivity.
    // Note that if the multi-connected shape is not directly contained in some shape,
    // but as a part of bigger sub-shape which will be removed, the multi-connected
    // shape is going to be removed also, breaking topological connectivity.
    // For instance, <theS> is a compound of the face and edge, which does not
    // belong to the face. The face contains internal wire and the edge shares
    // the vertex with one of the vertices of that wire. The vertex is not directly
    // contained in the face, thus will be removed as part of internal wire, and topological
    // connectivity between edge and face will be lost.
    TopTools_MapOfShape anAllNonInternals, anAllInternals;
    findInternalsToKeep(theS, anAllNonInternals, anAllInternals, aMKeep);
    if (aMKeep.Extent())
      pMKeep = &aMKeep;
  }

  removeInternals(theS, pMKeep);
}

//=================================================================================================

void BRepTools1::CheckLocations(const TopoShape& theS, ShapeList& theProblemShapes)
{
  if (theS.IsNull())
    return;

  TopTools_IndexedMapOfShape aMapS;
  TopExp1::MapShapes(theS, aMapS, Standard_False, Standard_False);

  Standard_Integer i;
  for (i = 1; i <= aMapS.Extent(); ++i)
  {
    const TopoShape&    anS   = aMapS(i);
    const TopLoc_Location& aLoc  = anS.Location();
    const Transform3d&         aTrsf = aLoc.Transformation();
    Standard_Boolean       isBadTrsf =
      aTrsf.IsNegative() || (Abs(Abs(aTrsf.ScaleFactor()) - 1.) > TopLoc_Location::ScalePrec());

    if (isBadTrsf)
    {
      theProblemShapes.Append(anS);
    }
  }
}