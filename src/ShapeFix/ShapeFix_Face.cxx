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

// pdn 10.12.98: tr9_r0501-ug
// pdn 28.12.98: PRO10366 shifting pcurve between two singularities
//: k7 abv 5.01.99: USA60022.igs ent 243: FixMissingSeam() improved
//: l2 abv 10.01.99: USA60022 7289: corrections for reversed face
// gka 11.01.99 file PRO7755.stp #2018: work-around error in BRepLib_MakeFace
//: p4 abv, pdn 23.02.99: PRO9234 #15720: call BRepTools1::Update() for faces
//    rln 03.03.99 S4135: transmission of parameter precision to SA_Surface::NbSingularities
//: q5 abv 19.03.99 code improvement
//%14 pdn 15.03.99 adding function for fixing null area wires
//%15 pdn 20.03.99 code improvement
//    abv 09.04.99 S4136: improve tolerance management; remove unused flag Closed
// #4  szv          S4163: optimization
//    smh 31.01.01 BUC60810 : Case of small wire on face in solid
// sln 25.09.2001  checking order of 3d and 2d representation curves
// abv 19.10.2001  FixAddNaturalBound improved and extracted as separate fix
// skl,pdn 14.05.2002  OCC55 (correction precision for small faces)

#include <Bnd_Box.hxx>
#include <BndLib_Add2dCurve.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepTools.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2dInt_GInter.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Circle.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_Surface.hxx>
#include <GProp_GProps.hxx>
#include <Message_Msg.hxx>
#include <NCollection_Array1.hxx>
#include <Precision.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeBuild_Edge.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeExtend_CompositeSurface.hxx>
#include <ShapeFix.hxx>
#include <ShapeFix_ComposeShell.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_IntersectionTool.hxx>
#include <ShapeFix_SplitTool.hxx>
#include <ShapeFix_Wire.hxx>
#include <Standard_Type.hxx>
#include <TColGeom_HArray2OfSurface.hxx>
#include <TColgp_SequenceOfPnt2d.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ShapeFix_Face, ShapeFix_Root)

#ifdef OCCT_DEBUG
  #define DEBUG
#endif

static Standard_Boolean IsSurfaceUVInfinite(const Handle(GeomSurface)& theSurf)
{
  Standard_Real UMin, UMax, VMin, VMax;
  theSurf->Bounds(UMin, UMax, VMin, VMax);

  return (Precision1::IsInfinite(UMin) || Precision1::IsInfinite(UMax) || Precision1::IsInfinite(VMin)
          || Precision1::IsInfinite(VMax));
}

static Standard_Boolean IsSurfaceUVPeriodic(const Handle(GeomAdaptor_Surface)& theSurf)
{
  return ((theSurf->IsUPeriodic() && theSurf->IsVPeriodic())
          || theSurf->GetType() == GeomAbs_Sphere);
}

//=================================================================================================

ShapeFix_Face::ShapeFix_Face()
{
  myFwd     = Standard_True;
  myStatus  = 0;
  myFixWire = new WireHealer;
  ClearModes();
}

//=================================================================================================

ShapeFix_Face::ShapeFix_Face(const TopoFace& face)
{
  myFwd     = Standard_True;
  myStatus  = 0;
  myFixWire = new WireHealer;
  ClearModes();
  Init(face);
}

//=================================================================================================

void ShapeFix_Face::ClearModes()
{
  myFixWireMode              = -1;
  myFixOrientationMode       = -1;
  myFixAddNaturalBoundMode   = -1;
  myFixMissingSeamMode       = -1;
  myFixSmallAreaWireMode     = -1;
  myRemoveSmallAreaFaceMode  = -1;
  myFixIntersectingWiresMode = -1;
  myFixLoopWiresMode         = -1;
  myFixSplitFaceMode         = -1;
  myAutoCorrectPrecisionMode = 1;
  myFixPeriodicDegenerated   = -1;
}

//=================================================================================================

void ShapeFix_Face::SetMsgRegistrator(const Handle(ShapeExtend_BasicMsgRegistrator)& msgreg)
{
  ShapeFix_Root::SetMsgRegistrator(msgreg);
  myFixWire->SetMsgRegistrator(msgreg);
}

//=================================================================================================

void ShapeFix_Face::SetPrecision(const Standard_Real preci)
{
  ShapeFix_Root::SetPrecision(preci);
  myFixWire->SetPrecision(preci);
}

//=================================================================================================

void ShapeFix_Face::SetMinTolerance(const Standard_Real mintol)
{
  ShapeFix_Root::SetMinTolerance(mintol);
  myFixWire->SetMinTolerance(mintol);
}

//=================================================================================================

void ShapeFix_Face::SetMaxTolerance(const Standard_Real maxtol)
{
  ShapeFix_Root::SetMaxTolerance(maxtol);
  myFixWire->SetMaxTolerance(maxtol);
}

//=================================================================================================

void ShapeFix_Face::Init(const Handle(GeomSurface)& surf,
                         const Standard_Real         preci,
                         const Standard_Boolean      fwd)
{
  myStatus                          = 0;
  Handle(ShapeAnalysis_Surface) sas = new ShapeAnalysis_Surface(surf);
  Init(sas, preci, fwd);
}

//=================================================================================================

void ShapeFix_Face::Init(const Handle(ShapeAnalysis_Surface)& surf,
                         const Standard_Real                  preci,
                         const Standard_Boolean               fwd)
{
  myStatus = 0;
  mySurf   = surf;
  SetPrecision(preci);
  ShapeBuilder B;
  B.MakeFace(myFace, mySurf->Surface(), ::Precision1::Confusion());
  myShape = myFace;
  myFwd   = fwd;
  if (!fwd)
    myFace.Orientation(TopAbs_REVERSED);
}

//=================================================================================================

void ShapeFix_Face::Init(const TopoFace& face)
{
  myStatus = 0;
  mySurf   = new ShapeAnalysis_Surface(BRepInspector::Surface(face));
  myFwd    = (face.Orientation() != TopAbs_REVERSED);
  myFace   = face;
  myShape  = myFace;
  //  myFace = TopoDS::Face(face.EmptyCopied());
  //  for (TopoDS_Iterator ws (face,Standard_False); ws.More(); ws.Next())
  //    Add (TopoDS::Wire (ws.Value()) );
}

//=================================================================================================

void ShapeFix_Face::Add(const TopoWire& wire)
{
  if (wire.IsNull())
    return;
  ShapeBuilder B;
  // szv#4:S4163:12Mar99 SGI warns
  TopoShape fc = myFace.Oriented(TopAbs_FORWARD); //: l2 abv 10 Jan 99: Oriented()
  B.Add(fc, wire);
}

//=======================================================================
// function : SplitWire
// purpose  : auxiliary - try to split wire (it is needed if some segments
//           were removed in WireHealer::FixSelfIntersection() )
//=======================================================================
static Standard_Boolean SplitWire(const TopoFace&        face,
                                  const TopoWire&        wire,
                                  TopTools_SequenceOfShape& aResWires)
{
  TColStd_MapOfInteger         UsedEdges;
  Handle(ShapeExtend_WireData) sewd = new ShapeExtend_WireData(wire);
  Standard_Integer             i, j, k;
  Edge1           sae;
  for (i = 1; i <= sewd->NbEdges(); i++)
  {
    if (UsedEdges.Contains(i))
      continue;
    TopoEdge E1 = sewd->Edge(i);
    UsedEdges.Add(i);
    TopoVertex V0, V1, V2;
    V0                                 = sae.FirstVertex(E1);
    V1                                 = sae.LastVertex(E1);
    Handle(ShapeExtend_WireData) sewd1 = new ShapeExtend_WireData;
    sewd1->Add(E1);
    Standard_Boolean IsConnectedEdge = Standard_True;
    for (j = 2; j <= sewd->NbEdges() && IsConnectedEdge; j++)
    {
      TopoEdge E2;
      for (k = 2; k <= sewd->NbEdges(); k++)
      {
        if (UsedEdges.Contains(k))
          continue;
        E2                = sewd->Edge(k);
        TopoVertex V21 = sae.FirstVertex(E2);
        TopoVertex V22 = sae.LastVertex(E2);
        if (sae.FirstVertex(E2).IsSame(V1))
        {
          sewd1->Add(E2);
          UsedEdges.Add(k);
          V1 = sae.LastVertex(E2);
          break;
        }
      }
      if (k > sewd->NbEdges())
      {
        IsConnectedEdge = Standard_False;
        break;
      }
      if (V1.IsSame(V0))
      {
        // check that V0 and V1 are same in 2d too
        Standard_Real        a1, b1, a2, b2;
        Handle(GeomCurve2d) curve1 = BRepInspector::CurveOnSurface(E1, face, a1, b1);
        Handle(GeomCurve2d) curve2 = BRepInspector::CurveOnSurface(E2, face, a2, b2);
        gp_Pnt2d             v0, v1;
        if (E1.Orientation() == TopAbs_REVERSED)
          a1 = b1;
        if (E2.Orientation() == TopAbs_REVERSED)
          b2 = a2;
        curve1->D0(a1, v0);
        curve2->D0(b2, v1);
        GeomAdaptor_Surface anAdaptor(BRepInspector::Surface(face));
        Standard_Real       tol = Max(BRepInspector::Tolerance(V0), BRepInspector::Tolerance(V1));
        Standard_Real       maxResolution =
          2 * Max(anAdaptor.UResolution(tol), anAdaptor.VResolution(tol));
        if (v0.SquareDistance(v1) < maxResolution)
        {
          // new wire is closed, put it into sequence
          aResWires.Append(sewd1->Wire());
          break;
        }
      }
    }
    if (!IsConnectedEdge)
    {
      // create new notclosed wire
      aResWires.Append(sewd1->Wire());
    }
    if (UsedEdges.Extent() == sewd->NbEdges())
      break;
  }

  if (aResWires.Length() > 1)
  {
#ifdef OCCT_DEBUG
    std::cout << "Wire was split on " << aResWires.Length() << " wires" << std::endl;
#endif
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::Perform()
{
  myStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  myFixWire->SetContext(Context());
  Handle(WireHealer) theAdvFixWire = myFixWire;
  if (theAdvFixWire.IsNull())
    return Standard_False;

  ShapeBuilder B;
  TopoShape aInitFace = myFace;
  // perform first part of fixes on wires
  Standard_Boolean isfixReorder = Standard_False;
  Standard_Boolean isReplaced   = Standard_False;

  // gka fix in order to avoid lost messages (following OCC21771)
  TopTools_DataMapOfShapeShape aMapReorderedWires;

  Standard_Real aSavPreci = Precision1();
  if (NeedFix(myFixWireMode))
  {
    theAdvFixWire->SetFace(myFace);

    Standard_Integer usFixLackingMode = theAdvFixWire->FixLackingMode();
    // Standard_Integer usFixNotchedEdgesMode = theAdvFixWire->FixNotchedEdgesMode(); // CR0024983
    Standard_Integer usFixSelfIntersectionMode = theAdvFixWire->FixSelfIntersectionMode();
    theAdvFixWire->FixLackingMode()            = Standard_False;
    // theAdvFixWire->FixNotchedEdgesMode() = Standard_False; // CR0024983
    theAdvFixWire->FixSelfIntersectionMode() = Standard_False;

    Standard_Boolean fixed = Standard_False;
    TopoShape     S     = myFace;
    if (!Context().IsNull())
      S = Context()->Apply(myFace);
    TopoShape emptyCopied = S.EmptyCopied();
    TopoFace  tmpFace     = TopoDS::Face(emptyCopied);
    tmpFace.Orientation(TopAbs_FORWARD);

    /*
    // skl 14.05.2002 OCC55 + corrected 03.03.2004
    Standard_Real dPreci = aSavPreci*aSavPreci;
    dPreci*=4;
    Standard_Real newpreci=dPreci;
    for(ShapeExplorer exp(S,TopAbs_EDGE); exp.More(); exp.Next()) {
      TopoEdge edge = TopoDS::Edge ( exp.Current() );
      Standard_Real first,last;
      Handle(GeomCurve3d) c3d = BRepInspector::Curve(edge, first, last);
      if(!c3d.IsNull()) {
        Box2 bb;
        bb.Add(c3d->Value(first));
        bb.Add(c3d->Value(last));
        bb.Add(c3d->Value((last+first)/2.));
        Standard_Real x1,x2,y1,y2,z1,z2,size;
        bb.Get(x1,y1,z1,x2,y2,z2);
        size = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1);
        if(size<newpreci) newpreci=size;
      }
    }
    newpreci=sqrt(newpreci)/2.*1.00001;
    if( aSavPreci > newpreci && newpreci > Precision1::Confusion()) {
      SetPrecision(newpreci);
      theAdvFixWire->SetPrecision(newpreci);
    }
    // end skl 14.05.2002
    */

    // skl 29.03.2010 (OCC21623)
    if (myAutoCorrectPrecisionMode)
    {
      Standard_Real size     = ShapeFix1::LeastEdgeSize(S);
      Standard_Real newpreci = Min(aSavPreci, size / 2.);
      newpreci               = newpreci * 1.00001;
      if (aSavPreci > newpreci && newpreci > Precision1::Confusion())
      {
        SetPrecision(newpreci);
        theAdvFixWire->SetPrecision(newpreci);
      }
    }

    isfixReorder = Standard_False;
    for (TopoDS_Iterator iter(S, Standard_False); iter.More(); iter.Next())
    {
      if (iter.Value().ShapeType() != TopAbs_WIRE)
      {
        B.Add(tmpFace, iter.Value());
        continue;
      }
      TopoWire wire = TopoDS::Wire(iter.Value());
      theAdvFixWire->Load(wire);
      if (theAdvFixWire->NbEdges() == 0)
      {
        if (theAdvFixWire->WireData()->NbNonManifoldEdges())
          B.Add(tmpFace, wire);
        else
        {
          fixed = Standard_True;
          myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5);
        }
        continue;
      }
      if (theAdvFixWire->Perform())
      {
        // fixed = Standard_True;
        isfixReorder  = (theAdvFixWire->StatusReorder(ShapeExtend_DONE) || isfixReorder);
        fixed         = (theAdvFixWire->StatusSmall(ShapeExtend_DONE)
                 || theAdvFixWire->StatusConnected(ShapeExtend_DONE)
                 || theAdvFixWire->StatusEdgeCurves(ShapeExtend_DONE)
                 || theAdvFixWire->StatusNotches(ShapeExtend_DONE) || // CR0024983
                 theAdvFixWire->StatusFixTails(ShapeExtend_DONE)
                 || theAdvFixWire->StatusDegenerated(ShapeExtend_DONE)
                 || theAdvFixWire->StatusClosed(ShapeExtend_DONE));
        TopoWire w = theAdvFixWire->Wire();
        if (fixed)
        {
          if (!Context().IsNull())
            Context()->Replace(wire, w);
          if (theAdvFixWire->NbEdges() == 0)
          {
            myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5);
            continue;
          }
        }
        else if (!wire.IsSame(w))
          aMapReorderedWires.Bind(wire, w);

        wire = w;
      }
      B.Add(tmpFace, wire);
      //      if ( theAdvFixWire->Status ( ShapeExtend_FAIL ) )
      //	myStatus |= ShapeExtend1::EncodeStatus ( ShapeExtend_FAIL1 );
    }

    theAdvFixWire->FixLackingMode() = usFixLackingMode;
    // theAdvFixWire->FixNotchedEdgesMode() = usFixNotchedEdgesMode;  // CR0024983
    theAdvFixWire->FixSelfIntersectionMode() = usFixSelfIntersectionMode;
    if (!myFwd)
      tmpFace.Orientation(TopAbs_REVERSED);

    if (fixed)
    {
      // if ( ! myFwd ) tmpFace.Orientation ( TopAbs_REVERSED );
      if (!Context().IsNull())
        Context()->Replace(S, tmpFace);
      // myFace = tmpFace;
      isReplaced = Standard_True;
    }
    if (fixed || isfixReorder)
    {
      myFace = tmpFace;
      if (!theAdvFixWire->StatusReorder(ShapeExtend_DONE5))
      {
        myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
      }
    }
  }

  myResult              = myFace;
  TopoShape savShape = myFace; // gka BUG 6555

  // Specific case for conic surfaces
  if (NeedFix(myFixPeriodicDegenerated))
    this->FixPeriodicDegenerated();

  // fix missing seam
  if (NeedFix(myFixMissingSeamMode))
  {
    if (FixMissingSeam())
    {
      myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
    }
  }

  // cycle by all possible faces coming from FixMissingSeam
  // each face is processed as if it was single
  ShapeExplorer exp(myResult, TopAbs_FACE);
  for (; exp.More(); exp.Next())
  {
    myFace                              = TopoDS::Face(exp.Current());
    Standard_Boolean NeedCheckSplitWire = Standard_False;

    // perform second part of fixes on wires
    if (NeedFix(myFixWireMode))
    {
      theAdvFixWire->SetFace(myFace);

      Standard_Integer usFixSmallMode       = theAdvFixWire->FixSmallMode();
      Standard_Integer usFixConnectedMode   = theAdvFixWire->FixConnectedMode();
      Standard_Integer usFixEdgeCurvesMode  = theAdvFixWire->FixEdgeCurvesMode();
      Standard_Integer usFixDegeneratedMode = theAdvFixWire->FixDegeneratedMode();
      theAdvFixWire->FixSmallMode()         = Standard_False;
      theAdvFixWire->FixConnectedMode()     = Standard_False;
      theAdvFixWire->FixEdgeCurvesMode()    = Standard_False;
      theAdvFixWire->FixDegeneratedMode()   = Standard_False;

      Standard_Boolean fixed = Standard_False;
      TopoShape     S     = myFace;
      if (!Context().IsNull())
        S = Context()->Apply(myFace);
      TopoShape emptyCopied = S.EmptyCopied();
      TopoFace  tmpFace     = TopoDS::Face(emptyCopied);
      tmpFace.Orientation(TopAbs_FORWARD);
      for (TopoDS_Iterator iter(S, Standard_False); iter.More(); iter.Next())
      {
        if (iter.Value().ShapeType() != TopAbs_WIRE)
        {
          B.Add(tmpFace, iter.Value());
          continue;
        }

        TopoWire wire = TopoDS::Wire(iter.Value());
        theAdvFixWire->Load(wire);
        if (theAdvFixWire->NbEdges() == 0)
        {
          if (theAdvFixWire->WireData()->NbNonManifoldEdges())
            B.Add(tmpFace, wire);
          else
          {
            fixed = Standard_True;
            myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5);
          }
          continue;
        }
        if (theAdvFixWire->Perform())
        {
          isfixReorder  = theAdvFixWire->StatusReorder(ShapeExtend_DONE);
          fixed         = (theAdvFixWire->StatusLacking(ShapeExtend_DONE)
                   || theAdvFixWire->StatusSelfIntersection(ShapeExtend_DONE)
                   || theAdvFixWire->StatusNotches(ShapeExtend_DONE)
                   || theAdvFixWire->StatusFixTails(ShapeExtend_DONE));
          TopoWire w = theAdvFixWire->Wire();
          if (fixed)
          {
            if (!Context().IsNull())
              Context()->Replace(wire, w);
          }
          else if (!wire.IsSame(w))
            aMapReorderedWires.Bind(wire, w);

          wire = w;
        }
        if (theAdvFixWire->StatusRemovedSegment())
          NeedCheckSplitWire = Standard_True;

        // fix for loop of wire
        TopTools_SequenceOfShape aLoopWires;
        if (NeedFix(myFixLoopWiresMode) && FixLoopWire(aLoopWires))
        {
          if (aLoopWires.Length() > 1)
            // clang-format off
            SendWarning ( wire, Message_Msg ( "FixAdvFace.FixLoopWire.MSG0" ) );// Wire was split on several wires
          // clang-format on
          myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE7);
          fixed              = Standard_True;
          Standard_Integer k = 1;
          for (; k <= aLoopWires.Length(); k++)
            B.Add(tmpFace, aLoopWires.Value(k));
        }
        else
          B.Add(tmpFace, wire);
      }

      theAdvFixWire->FixSmallMode()       = usFixSmallMode;
      theAdvFixWire->FixConnectedMode()   = usFixConnectedMode;
      theAdvFixWire->FixEdgeCurvesMode()  = usFixEdgeCurvesMode;
      theAdvFixWire->FixDegeneratedMode() = usFixDegeneratedMode;

      if (fixed)
      {
        if (!myFwd)
          tmpFace.Orientation(TopAbs_REVERSED);
        if (!isReplaced && !aInitFace.IsSame(myResult)
            && !Context().IsNull()) // gka 06.09.04 BUG 6555
          Context()->Replace(aInitFace, savShape);
        if (!Context().IsNull())
          Context()->Replace(S, tmpFace);
        myFace = tmpFace;
        myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
      }
    }

    if (NeedCheckSplitWire)
    {
      // try to split wire - it is needed if some segments were removed
      // in WireHealer::FixSelfIntersection()
      TopoShape S = myFace;
      if (!Context().IsNull())
        S = Context()->Apply(myFace);
      TopoShape emptyCopied = S.EmptyCopied();
      TopoFace  tmpFace     = TopoDS::Face(emptyCopied);
      tmpFace.Orientation(TopAbs_FORWARD);
      TopTools_SequenceOfShape aWires;
      Standard_Integer         nbw = 0;
      for (TopoDS_Iterator iter(S, Standard_False); iter.More(); iter.Next())
      {
        if (iter.Value().ShapeType() != TopAbs_WIRE)
        {
          B.Add(tmpFace, iter.Value());
          continue;
        }
        if (iter.Value().Orientation() != TopAbs_FORWARD
            && iter.Value().Orientation() != TopAbs_REVERSED)
        {
          B.Add(tmpFace, TopoDS::Wire(iter.Value()));
          continue;
        }
        nbw++;
        TopoWire wire = TopoDS::Wire(iter.Value());
        SplitWire(tmpFace, wire, aWires);
      }
      if (nbw < aWires.Length())
      {
        for (Standard_Integer iw = 1; iw <= aWires.Length(); iw++)
          B.Add(tmpFace, aWires.Value(iw));
        if (!Context().IsNull())
          Context()->Replace(S, tmpFace);
        myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE8);
        myFace = tmpFace;
      }
    }

    // fix intersecting wires
    if (FixWiresTwoCoincEdges())
      myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE7);
    if (NeedFix(myFixIntersectingWiresMode))
    {
      if (FixIntersectingWires())
      {
        myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE6);
      }
    }

    // fix orientation
    TopTools_DataMapOfShapeListOfShape MapWires;
    MapWires.Clear();
    if (NeedFix(myFixOrientationMode))
    {
      if (FixOrientation(MapWires))
        myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
    }

    BRepTools1::Update(myFace);

    // fix natural bounds
    Standard_Boolean NeedSplit = Standard_True;
    if (FixAddNaturalBound())
    {
      NeedSplit = Standard_False;
      myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5);
    }

    // split face
    if (NeedFix(myFixSplitFaceMode) && NeedSplit && MapWires.Extent() > 1)
    {
      if (FixSplitFace(MapWires))
        myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE8);
    }
  }

  // return the original preci
  SetPrecision(aSavPreci);
  theAdvFixWire->SetPrecision(aSavPreci);

  // cycle by all possible faces coming from FixAddNaturalBound
  // each face is processed as if it was single
  for (exp.Init(myResult, TopAbs_FACE); exp.More(); exp.Next())
  {
    myFace = TopoDS::Face(exp.Current());

    // fix small-area wires
    if (NeedFix(myFixSmallAreaWireMode, Standard_False))
    {
      const Standard_Boolean isRemoveFace = NeedFix(myRemoveSmallAreaFaceMode, Standard_False);
      if (FixSmallAreaWire(isRemoveFace))
        myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE4);
    }
  }

  if (!Context().IsNull())
  {
    if (Status(ShapeExtend_DONE) && !isReplaced && !aInitFace.IsSame(savShape))
    {
      // gka fix in order to avoid lost messages (following OCC21771)
      if (aMapReorderedWires.Extent())
      {
        TopoDS_Iterator aItW(aInitFace, Standard_False);
        for (; aItW.More(); aItW.Next())
        {
          TopoShape aCurW = aItW.Value();
          while (aMapReorderedWires.IsBound(aCurW))
          {
            const TopoShape& aFixW = aMapReorderedWires.Find(aCurW);
            Context()->Replace(aCurW, aFixW);
            aCurW = aFixW;
          }
        }
      }
      Context()->Replace(aInitFace, savShape);
    }
    myResult = Context()->Apply(aInitFace); // gka 06.09.04
  }
  else if (!Status(ShapeExtend_DONE))
    myResult = aInitFace;

  return Status(ShapeExtend_DONE);
}

//=================================================================================================

// Shift all pcurves of edges in the given wire on the given face
// to vector <vec>
static void Shift2dWire(const TopoWire&                   w,
                        const TopoFace&                   f,
                        const gp_Vec2d                       vec,
                        const Handle(ShapeAnalysis_Surface)& mySurf,
                        Standard_Boolean                     recompute3d = Standard_False)
{
  Transform2d tr2d;
  tr2d.SetTranslation(vec.XY());
  Edge1 sae;
  Edge2    sbe;
  ShapeBuilder       B;
  for (TopoDS_Iterator ei(w, Standard_False); ei.More(); ei.Next())
  {
    TopoEdge          edge = TopoDS::Edge(ei.Value());
    Handle(GeomCurve2d) C2d;
    Standard_Real        cf, cl;
    if (!sae.PCurve(edge, f, C2d, cf, cl, Standard_True))
      continue;
    C2d->Transform(tr2d);
    if (recompute3d)
    {
      // recompute 3d curve and vertex
      sbe.RemoveCurve3d(edge);
      sbe.BuildCurve3d(edge);
      B.UpdateVertex(sae.FirstVertex(edge), mySurf->Value(C2d->Value(cf)), 0.);
    }
  }
}

// Cut interval from the sequence of intervals
static Standard_Boolean CutInterval(TColgp_SequenceOfPnt2d& intervals,
                                    const gp_Pnt2d&         toAddI,
                                    const Standard_Real     period)
{
  if (intervals.Length() <= 0)
    return Standard_False;
  for (Standard_Integer j = 0; j < 2; j++)
  { // try twice, align to bottom and to top
    for (Standard_Integer i = 1; i <= intervals.Length(); i++)
    {
      gp_Pnt2d interval = intervals(i);
      // ACIS907, OCC921 a054a.sat (face 124)
      Standard_Real shift = ShapeAnalysis1::AdjustByPeriod((j ? toAddI.X() : toAddI.Y()),
                                                          0.5 * (interval.X() + interval.Y()),
                                                          period);
      gp_Pnt2d      toAdd(toAddI.X() + shift, toAddI.Y() + shift);
      if (toAdd.Y() <= interval.X() || toAdd.X() >= interval.Y())
        continue;
      if (toAdd.X() > interval.X())
      {
        if (toAdd.Y() < interval.Y())
        {
          intervals.InsertBefore(i, interval);
          intervals.ChangeValue(i + 1).SetX(toAdd.Y()); // i++...
        }
        intervals.ChangeValue(i).SetY(toAdd.X());
      }
      else if (toAdd.Y() < interval.Y())
      {
        intervals.ChangeValue(i).SetX(toAdd.Y());
      }
      else
        intervals.Remove(i--);
    }
  }
  return Standard_True;
}

// Find middle of the biggest interval
static Standard_Real FindBestInterval(TColgp_SequenceOfPnt2d& intervals)
{
  Standard_Real shift = 0., max = -1.;
  for (Standard_Integer i = 1; i <= intervals.Length(); i++)
  {
    gp_Pnt2d interval = intervals(i);
    if (interval.Y() - interval.X() <= max)
      continue;
    max   = interval.Y() - interval.X();
    shift = interval.X() + 0.5 * max;
  }
  return shift;
}

//=================================================================================================

// Detect missing natural boundary on spherical surfaces and add it if
// necessary
// pdn 981202: add natural bounds if missing (on sphere only)
//: abv 28.08.01: rewritten and extended for toruses

Standard_Boolean ShapeFix_Face::FixAddNaturalBound()
{
  if (!Context().IsNull())
  {
    TopoShape S = Context()->Apply(myFace);
    myFace         = TopoDS::Face(S);
  }

  // collect wires in sequence
  TopTools_SequenceOfShape ws;
  TopTools_SequenceOfShape vs;
  TopoDS_Iterator          wi(myFace, Standard_False);
  for (; wi.More(); wi.Next())
  {
    if (wi.Value().ShapeType() == TopAbs_WIRE
        && (wi.Value().Orientation() == TopAbs_FORWARD
            || wi.Value().Orientation() == TopAbs_REVERSED))
      ws.Append(wi.Value());
    else
      vs.Append(wi.Value());
  }

  // deal with the case of an empty face: just create a new face by a standard tool
  if (ws.IsEmpty() && !IsSurfaceUVInfinite(mySurf->Surface()))
  {
    FaceMaker aFaceBuilder(mySurf->Surface(), Precision1::Confusion());

    TopoFace aNewFace = aFaceBuilder.Face();
    aNewFace.Orientation(myFace.Orientation());

    if (!Context().IsNull())
      Context()->Replace(myFace, aNewFace);

    // taking into account orientation
    myFace = aNewFace;

    // gka 11.01.99 file PRO7755.stp entity #2018 surface #1895: error BRepLib_MakeFace func
    // IsDegenerated
    Handle(ShapeFix_Edge) sfe = myFixWire->FixEdgeTool();
    for (ShapeExplorer Eed(myFace, TopAbs_EDGE); Eed.More(); Eed.Next())
    {
      TopoEdge edg = TopoDS::Edge(Eed.Current());
      sfe->FixVertexTolerance(edg, myFace);
    }

    //    B.UpdateFace (myFace,myPrecision);
    // clang-format off
    SendWarning ( myFace, Message_Msg ( "FixAdvFace.FixOrientation.MSG0" ) );// Face created with natural bounds
    // clang-format on
    BRepTools1::Update(myFace);
    myResult = myFace;
    return Standard_True;
  }

  // check if surface doesn't need natural bounds
  if (!isNeedAddNaturalBound(ws))
  {
    return Standard_False;
  }

  // Collect information on free intervals in U and V
  TColgp_SequenceOfPnt2d intU, intV, centers;
  Standard_Real          SUF, SUL, SVF, SVL;
  mySurf->Bounds(SUF, SUL, SVF, SVL);
  intU.Append(gp_Pnt2d(SUF, SUL));
  intV.Append(gp_Pnt2d(SVF, SVL));
  Standard_Integer nb = ws.Length();
  Standard_Integer i;

  for (i = 1; i <= nb; i++)
  {
    Standard_Real Umin, Vmin, Umax, Vmax;
    //     Bnd_Box2d B;
    TopoWire aw = TopoDS::Wire(ws.Value(i));
    // PTV 01.11.2002 ACIS907, OCC921 begin
    //     BRepTools1::AddUVBounds(myFace,aw,B);
    //     B.Get(Umin, Vmin, Umax, Vmax);
    TopoFace  aWireFace = TopoDS::Face(myFace.EmptyCopied());
    ShapeBuilder aB;
    aB.Add(aWireFace, aw);
    ShapeAnalysis1::GetFaceUVBounds(aWireFace, Umin, Umax, Vmin, Vmax);

    // PTV 01.11.2002 ACIS907, OCC921 end
    if (mySurf->IsUClosed())
      CutInterval(intU, gp_Pnt2d(Umin, Umax), SUL - SUF);
    if (mySurf->IsVClosed())
      CutInterval(intV, gp_Pnt2d(Vmin, Vmax), SVL - SVF);
    centers.Append(gp_Pnt2d(0.5 * (Umin + Umax), 0.5 * (Vmin + Vmax)));
  }

  // find best interval and thus compute shift
  gp_Pnt2d shift(0., 0.);
  if (mySurf->IsUClosed())
    shift.SetX(FindBestInterval(intU));
  if (mySurf->IsVClosed())
    shift.SetY(FindBestInterval(intV));

  // Adjust all other wires to be inside outer one
  gp_Pnt2d center(shift.X() + 0.5 * (SUL - SUF), shift.Y() + 0.5 * (SVL - SVF));
  for (i = 1; i <= nb; i++)
  {
    TopoWire wire = TopoDS::Wire(ws.Value(i));
    gp_Pnt2d    sh(0., 0.);
    if (mySurf->IsUClosed())
      sh.SetX(ShapeAnalysis1::AdjustByPeriod(centers(i).X(), center.X(), SUL - SUF));
    if (mySurf->IsVClosed())
      sh.SetY(ShapeAnalysis1::AdjustByPeriod(centers(i).Y(), center.Y(), SVL - SVF));
    Shift2dWire(wire, myFace, sh.XY(), mySurf);
  }

  // Create naturally bounded surface and add that wire to sequence
  /* variant 1
    // Create fictive grid and call ComposeShell
    Handle(Geom_RectangularTrimmedSurface) RTS =
      new Geom_RectangularTrimmedSurface ( mySurf->Surface(), SUF+shift.X(), SUL+shift.X(),
                                           SVF+shift.Y(), SVL+shift.Y() );
    Handle(TColGeom_HArray2OfSurface) grid = new TColGeom_HArray2OfSurface ( 1, 1, 1, 1 );
    grid->SetValue ( 1, 1, RTS );
    Handle(ShapeExtend_CompositeSurface) G = new ShapeExtend_CompositeSurface ( grid );
    TopLoc_Location L;

    ShapeFix_ComposeShell CompShell;
    CompShell.Init ( G, L, myFace, ::Precision1::Confusion() );
    CompShell.ClosedMode() = Standard_True;
    CompShell.NaturalBoundMode() = Standard_True;
    CompShell.SetContext( Context() );
    CompShell.SetMaxTolerance(MaxTolerance());
    CompShell.Perform();
    TopoShape res = CompShell.Result();

    Context()->Replace ( myFace, res );
    for (ShapeExplorer exp ( res, TopAbs_FACE ); exp.More(); exp.Next() ) {
      myFace = TopoDS::Face ( exp.Current() );
      BRepTools1::Update(myFace); //:p4
    }
    myResult = Context()->Apply ( myResult );
  */
  /* variant 2 */
  TopLoc_Location         L;
  Handle(GeomSurface)    surf = BRepInspector::Surface(myFace, L);
  FaceMaker mf(surf, Precision1::Confusion());
  TopoFace             ftmp = mf.Face();
  ftmp.Location(L);
  for (wi.Initialize(ftmp, Standard_False); wi.More(); wi.Next())
  {
    if (wi.Value().ShapeType() != TopAbs_WIRE)
      continue;
    TopoWire wire = TopoDS::Wire(wi.Value());
    ws.Append(wire);
    if (shift.XY().Modulus() < ::Precision1::PConfusion())
      continue;
    Shift2dWire(wire, myFace, shift.XY(), mySurf, Standard_True);
  }

  // Fix possible case on sphere when gap contains degenerated edge
  // and thus has a common part with natural boundary
  // Such hole should be merged with boundary
  if (mySurf->Adaptor3d()->GetType() == GeomAbs_Sphere && ws.Length() == nb + 1)
  {
    Handle(ShapeExtend_WireData) bnd = new ShapeExtend_WireData(TopoDS::Wire(ws.Last()));
    // code to become separate method FixTouchingWires()
    for (i = 1; i <= nb; i++)
    {
      Handle(ShapeExtend_WireData) sbwd = new ShapeExtend_WireData(TopoDS::Wire(ws.Value(i)));
      for (Standard_Integer j = 1; j <= sbwd->NbEdges(); j++)
      {
        if (!BRepInspector::Degenerated(sbwd->Edge(j)))
          continue;
        // find corresponding place in boundary
        Edge1 sae;
        TopoVertex      V = sae.FirstVertex(sbwd->Edge(j));
        Standard_Integer   k;
        for (k = 1; k <= bnd->NbEdges(); k++)
        {
          if (!BRepInspector::Degenerated(bnd->Edge(k)))
            continue;
          if (BRepTools1::Compare(V, sae.FirstVertex(bnd->Edge(k))))
            break;
        }
        if (k > bnd->NbEdges())
          continue;
        // and insert hole to that place
        ShapeBuilder B;
        B.Degenerated(sbwd->Edge(j), Standard_False);
        B.Degenerated(bnd->Edge(k), Standard_False);
        sbwd->SetLast(j);
        bnd->Add(sbwd, k + 1);
        ws.Remove(i--);
        nb--;
        myFixWire->SetFace(myFace);
        myFixWire->Load(bnd);
        myFixWire->FixConnected();
        myFixWire->FixDegenerated();
        ws.SetValue(ws.Length(), bnd->Wire());
        break;
      }
    }
  }

  // Create resulting face
  ShapeBuilder B;
  TopoShape S = myFace.EmptyCopied();
  S.Orientation(TopAbs_FORWARD);
  for (i = 1; i <= ws.Length(); i++)
    B.Add(S, ws.Value(i));
  for (i = 1; i <= vs.Length(); i++)
    B.Add(S, vs.Value(i));
  if (!myFwd)
    S.Orientation(TopAbs_REVERSED);
  if (!Context().IsNull())
    Context()->Replace(myFace, S);
  myFace = TopoDS::Face(S);
  BRepTools1::Update(myFace);

/**/
#ifdef OCCT_DEBUG
  std::cout << "Natural bound on sphere or torus with holes added" << std::endl; // mise au point !
#endif
  // clang-format off
  SendWarning ( myFace, Message_Msg ( "FixAdvFace.FixOrientation.MSG0" ) );// Face created with natural bounds
  // clang-format on
  return Standard_True;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::FixOrientation()
{
  TopTools_DataMapOfShapeListOfShape MapWires;
  MapWires.Clear();
  return FixOrientation(MapWires);
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::isNeedAddNaturalBound(
  const TopTools_SequenceOfShape& theOrientedWires)
{
  // if fix is not needed
  if (!NeedFix(myFixAddNaturalBoundMode))
  {
    return Standard_False;
  }
  // if surface is not double-closed
  if (!IsSurfaceUVPeriodic(mySurf->Adaptor3d()))
  {
    return Standard_False;
  }
  // if face has an OUTER bound
  if (ShapeAnalysis1::IsOuterBound(myFace))
  {
    return Standard_False;
  }
  // check that not any wire has a seam edge and not any edge is degenerated.
  // because the presence of a seam or degenerated edge indicates that this wire should be an
  // external one, and in case of its incorrect orientation, this will be corrected.
  Standard_Integer aNbOriented = theOrientedWires.Length();
  for (Standard_Integer i = 1; i <= aNbOriented; i++)
  {
    TopoWire aWire = TopoDS::Wire(theOrientedWires.Value(i));
    for (TopoDS_Iterator anEdgeIt(aWire); anEdgeIt.More(); anEdgeIt.Next())
    {
      TopoEdge anEdge = TopoDS::Edge(anEdgeIt.Value());
      if (BRepInspector::Degenerated(anEdge))
      {
        return Standard_False;
      }
      if (BRepInspector::IsClosed(anEdge, myFace))
      {
        return Standard_False;
      }
    }
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::FixOrientation(TopTools_DataMapOfShapeListOfShape& MapWires)
{
  Standard_Boolean done = Standard_False;

  if (!Context().IsNull())
  {
    TopoShape S = Context()->Apply(myFace);
    myFace         = TopoDS::Face(S);
  }
  TopTools_SequenceOfShape ws;
  TopTools_SequenceOfShape allSubShapes;
  // smh: BUC60810 : protection against very small wires (one-edge, null-length)
  TopTools_SequenceOfShape VerySmallWires;
  for (TopoDS_Iterator wi(myFace, Standard_False); wi.More(); wi.Next())
  {
    if (wi.Value().ShapeType() == TopAbs_VERTEX
        || (wi.Value().Orientation() != TopAbs_FORWARD
            && wi.Value().Orientation() != TopAbs_REVERSED))
    {
      allSubShapes.Append(wi.Value());
      // ws.Append (wi.Value());
      continue;
    }

    TopoDS_Iterator ei(wi.Value(), Standard_False);
    TopoEdge     anEdge;
    Standard_Real   length = RealLast();
    if (ei.More())
    {
      anEdge = TopoDS::Edge(ei.Value());
      ei.Next();
      if (!ei.More())
      {
        length = 0;
        Standard_Real      First, Last;
        Handle(GeomCurve3d) c3d;
        Edge1 sae;
        if (sae.Curve3d(anEdge, c3d, First, Last))
        {
          Point3d pntIni = c3d->Value(First);
          Coords3d prev;
          prev                       = pntIni.XYZ();
          Standard_Integer NbControl = 10;
          for (Standard_Integer j = 1; j < NbControl; j++)
          {
            Standard_Real prm     = ((NbControl - 1 - j) * First + j * Last) / (NbControl - 1);
            Point3d        pntCurr = c3d->Value(prm);
            Coords3d        curr    = pntCurr.XYZ();
            Coords3d        delta   = curr - prev;
            length += delta.Modulus();
            prev = curr;
          }
        }
      }
    }
    else
      length = 0;
    if (length > ::Precision1::Confusion())
    {
      ws.Append(wi.Value());
      allSubShapes.Append(wi.Value());
    }
    else
      VerySmallWires.Append(wi.Value());
  }
  if (VerySmallWires.Length() > 0)
    done = Standard_True;

  Standard_Integer nb    = ws.Length();
  Standard_Integer nbAll = allSubShapes.Length();
  ShapeBuilder     B;

  // if no wires, just do nothing
  if (nb <= 0)
    return Standard_False;

  Standard_Boolean          isAddNaturalBounds = isNeedAddNaturalBound(ws);
  TColStd_SequenceOfInteger aSeqReversed;
  // if wire is only one, check its orientation
  if (nb == 1)
  {
    // skl 12.04.2002 for cases with nbwires>1 (VerySmallWires>1)
    // make face with only one wire (ws.Value(1))
    TopoShape dummy = myFace.EmptyCopied();
    TopoFace  af    = TopoDS::Face(dummy);
    af.Orientation(TopAbs_FORWARD);
    B.Add(af, ws.Value(1));

    if (!isAddNaturalBounds && !ShapeAnalysis1::IsOuterBound(af))
    {
      Handle(ShapeExtend_WireData) sbdw = new ShapeExtend_WireData(TopoDS::Wire(ws.Value(1)));
      sbdw->Reverse(myFace);
      ws.SetValue(1, sbdw->Wire());
      // clang-format off
      SendWarning(sbdw->Wire(), Message_Msg("FixAdvFace.FixOrientation.MSG5"));// Wire on face was reversed
      // clang-format on
      done = Standard_True;
    }
  }
  // in case of several wires, perform complex1 analysis
  //  ATTENTION ESSAI
  //  Plusieurs wires : orientations relatives
  //  Chaque wire doit "contenir" tous les autres
  //  Evidemment, en cas de peau de leopard, il peut y avoir probleme
  else
  {
    //    On prend chaque wire (NB: pcurves presentes !)
    //    En principe on devrait rejeter les wires non fermes (cf couture manque ?)
    //    On le classe par rapport aux autres, qui doivent tous etre, soit IN soit
    //    OUT. Sinon il y a imbrication -> SDB. Si IN, OK, si OUT on inverse
    //      (nb : ici pas myClos donc pas de pb de couture)
    //    Si au moins une inversion, il faut refaire la face (cf myRebil)

    //: 94 abv 30 Jan 98: calculate parametric precision

    //    GeomAdaptor_Surface& Ads = mySurf->Adaptor3d()->ChangeSurface();
    //    Standard_Real toluv = Min ( Ads.UResolution(Precision1()), Ads.VResolution(Precision1()) );
    Standard_Boolean uclosed = mySurf->IsUClosed();
    Standard_Boolean vclosed = mySurf->IsVClosed();
    Standard_Real    SUF, SUL, SVF, SVL;
    mySurf->Bounds(SUF, SUL, SVF, SVL);
    Standard_Real uRange = SUL - SUF;
    Standard_Real vRange = SVL - SVF;

    TopTools_DataMapOfShapeListOfShape MW;
    TopTools_DataMapOfShapeInteger     SI;
    TopTools_MapOfShape                MapIntWires;
    MW.Clear();
    SI.Clear();
    MapIntWires.Clear();
    Standard_Integer i;

    NCollection_Array1<Bnd_Box2d> aWireBoxes(1, nb);
    Standard_Real                 uMiddle = 0, vMiddle = 0;
    Standard_Boolean              isFirst = Standard_True;
    // create Bounding boxes for each wire
    for (i = 1; i <= nb; i++)
    {
      TopoShape    aShape = ws.Value(i);
      TopoWire     aWire  = TopoDS::Wire(aShape);
      Bnd_Box2d       aBox;
      Standard_Real   cf, cl;
      TopoDS_Iterator ew(aWire);
      for (; ew.More(); ew.Next())
      {
        TopoEdge          ed = TopoDS::Edge(ew.Value());
        Handle(GeomCurve2d) cw = BRepInspector::CurveOnSurface(ed, myFace, cf, cl);
        if (cw.IsNull())
        {
          continue;
        }
        Geom2dAdaptor_Curve gac;
        Standard_Real       aFirst = cw->FirstParameter();
        Standard_Real       aLast  = cw->LastParameter();
        if (cw->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve)) && (cf < aFirst || cl > aLast))
        {
          // avoiding problems with segment in Box2
          gac.Load(cw);
        }
        else
          gac.Load(cw, cf, cl);
        Add2dCurve::Add(gac, ::Precision1::Confusion(), aBox);
      }

      Standard_Real aXMin, aXMax, aYMin, aYMax;
      aBox.Get(aXMin, aYMin, aXMax, aYMax);
      if (isFirst)
      {
        isFirst = Standard_False;
        uMiddle = (aXMin + aXMax) * 0.5;
        vMiddle = (aYMin + aYMax) * 0.5;
      }
      else
      {
        Standard_Real xShift = 0, yShift = 0;
        if (mySurf->IsUClosed())
          xShift = ShapeAnalysis1::AdjustByPeriod(0.5 * (aXMin + aXMax), uMiddle, uRange);
        if (mySurf->IsVClosed())
          yShift = ShapeAnalysis1::AdjustByPeriod(0.5 * (aYMin + aYMax), vMiddle, vRange);
        aBox.Update(aXMin + xShift, aYMin + yShift, aXMax + xShift, aYMax + yShift);
      }
      aWireBoxes.ChangeValue(i) = aBox;
    }

    for (i = 1; i <= nb; i++)
    {
      TopoShape asw   = ws.Value(i);
      TopoWire  aw    = TopoDS::Wire(asw);
      Bnd_Box2d    aBox1 = aWireBoxes.Value(i);
      TopoShape dummy = myFace.EmptyCopied();
      TopoFace  af    = TopoDS::Face(dummy);
      //      B.MakeFace (af,mySurf->Surface(),::Precision1::Confusion());
      af.Orientation(TopAbs_FORWARD);
      B.Add(af, aw);
      // PTV OCC945 06.11.2002 files ie_exhaust-A.stp (entities 3782,  3787)
      // tolerance is too big. It is seems that to identify placement of 2d point
      // it is enough Precision1::PConfusion(), cause wea re know that 2d point in TopAbs_ON
      // BRepTopAdaptor_FClass2d clas (af,toluv);
      Standard_Boolean        CheckShift = Standard_True;
      BRepTopAdaptor_FClass2d clas(af, ::Precision1::PConfusion());
      TopAbs_State            sta    = TopAbs_OUT;
      TopAbs_State            staout = clas.PerformInfinitePoint();
      ShapeList    IntWires;
      Standard_Integer        aWireIt = 0;
      for (Standard_Integer j = 1; j <= nbAll; j++)
      {
        aWireIt++;
        // if(i==j) continue;
        TopoShape aSh2 = allSubShapes.Value(j);
        if (aw == aSh2)
          continue;
        TopAbs_State stb = TopAbs_UNKNOWN;
        if (aSh2.ShapeType() == TopAbs_VERTEX)
        {
          aWireIt--;
          Point3d   aP  = BRepInspector::Pnt(TopoDS::Vertex(aSh2));
          gp_Pnt2d p2d = mySurf->ValueOfUV(aP, Precision1::Confusion());
          stb          = clas.Perform(p2d, Standard_False);
          if (stb == staout && (uclosed || vclosed))
          {
            gp_Pnt2d p2d1;
            if (uclosed)
            {
              p2d1.SetCoord(p2d.X() + uRange, p2d.Y());
              stb = clas.Perform(p2d1, Standard_False);
            }
            if (stb == staout && vclosed)
            {
              p2d1.SetCoord(p2d.X(), p2d.Y() + vRange);
              stb = clas.Perform(p2d1, Standard_False);
            }
          }
        }
        else if (aSh2.ShapeType() == TopAbs_WIRE)
        {
          CheckShift     = Standard_True;
          TopoWire bw = TopoDS::Wire(aSh2);
          // Standard_Integer numin =0;
          Bnd_Box2d aBox2 = aWireBoxes.Value(aWireIt);
          if (aBox2.IsOut(aBox1))
            continue;

          TopoDS_Iterator ew(bw);
          for (; ew.More(); ew.Next())
          {
            TopoEdge          ed = TopoDS::Edge(ew.Value());
            Standard_Real        cf, cl;
            Handle(GeomCurve2d) cw = BRepInspector::CurveOnSurface(ed, myFace, cf, cl);
            if (cw.IsNull())
              continue;
            gp_Pnt2d     unp = cw->Value((cf + cl) / 2.);
            TopAbs_State ste = clas.Perform(unp, Standard_False);
            if (ste == TopAbs_OUT || ste == TopAbs_IN)
            {
              if (stb == TopAbs_UNKNOWN)
              {
                stb = ste;
              }
              else
              {
                if (!(stb == ste))
                {
                  sta = TopAbs_UNKNOWN;
                  SI.Bind(aw, 0);
                  j = nbAll;
                  break;
                }
              }
            }

            Standard_Boolean found = Standard_False;
            gp_Pnt2d         unp1;
            if (stb == staout && CheckShift)
            {
              CheckShift = Standard_False;
              if (uclosed)
              {
                unp1.SetCoord(unp.X() + uRange, unp.Y());
                found = (staout != clas.Perform(unp1, Standard_False));
                if (!found)
                {
                  unp1.SetX(unp.X() - uRange);
                  found = (staout != clas.Perform(unp1, Standard_False));
                }
              }
              if (vclosed && !found)
              {
                unp1.SetCoord(unp.X(), unp.Y() + vRange);
                found = (staout != clas.Perform(unp1, Standard_False));
                if (!found)
                {
                  unp1.SetY(unp.Y() - vRange);
                  found = (staout != clas.Perform(unp1, Standard_False));
                }
              }
              // Additional check of diagonal steps for toroidal surfaces
              if (!found && uclosed && vclosed)
              {
                for (Standard_Real dX = -1.0; dX <= 1.0 && !found; dX += 2.0)
                  for (Standard_Real dY = -1.0; dY <= 1.0 && !found; dY += 2.0)
                  {
                    unp1.SetCoord(unp.X() + uRange * dX, unp.Y() + vRange * dY);
                    found = (staout != clas.Perform(unp1, Standard_False));
                  }
              }
            }
            if (found)
            {
              if (stb == TopAbs_IN)
                stb = TopAbs_OUT;
              else
                stb = TopAbs_IN;
              Shift2dWire(bw, myFace, unp1.XY() - unp.XY(), mySurf);
            }
          }
        }
        if (stb == staout)
        {
          sta = TopAbs_IN;
        }
        else
        {
          IntWires.Append(aSh2);
          MapIntWires.Add(aSh2);
        }
      }

      if (sta == TopAbs_UNKNOWN)
      { // ERREUR
        // clang-format off
        SendWarning ( aw, Message_Msg ( "FixAdvFace.FixOrientation.MSG11" ) );// Cannot orient wire
        // clang-format on
      }
      else
      {
        MW.Bind(aw, IntWires);
        if (sta == TopAbs_OUT)
        {
          if (staout == TopAbs_IN)
          {
            // wire is OUT but InfinitePoint is IN => need to reverse
            ShapeExtend_WireData sewd(aw);
            sewd.Reverse(myFace);
            ws.SetValue(i, sewd.Wire());
            // clang-format off
            SendWarning ( sewd.Wire(), Message_Msg ( "FixAdvFace.FixOrientation.MSG5" ) );// Wire on face was reversed
            // clang-format on
            aSeqReversed.Append(i);
            done = Standard_True;
            SI.Bind(ws.Value(i), 1);
            MapWires.Bind(ws.Value(i), IntWires);
          }
          else
          {
            SI.Bind(aw, 1);
            MapWires.Bind(aw, IntWires);
          }
        }
        else
        {
          if (staout == TopAbs_OUT)
            SI.Bind(aw, 2);
          else
            SI.Bind(aw, 3);
        }
      }
    }

    for (i = 1; i <= nb; i++)
    {
      TopoWire      aw   = TopoDS::Wire(ws.Value(i));
      Standard_Integer tmpi = SI.Find(aw);
      if (tmpi > 1)
      {
        if (!MapIntWires.Contains(aw))
        {
          const ShapeList& IW = MW.Find(aw);
          if (tmpi == 3)
          {
            // wire is OUT but InfinitePoint is IN => need to reverse
            ShapeExtend_WireData sewd(aw);
            sewd.Reverse(myFace);
            ws.SetValue(i, sewd.Wire());
            // clang-format off
            SendWarning ( sewd.Wire(), Message_Msg ( "FixAdvFace.FixOrientation.MSG5" ) );// Wire on face was reversed
            // clang-format on
            aSeqReversed.Append(i);
            done = Standard_True;
            MapWires.Bind(ws.Value(i), IW);
          }
          else
            MapWires.Bind(aw, IW);
        }
        else
        {
          if (tmpi == 2)
          {
            // wire is IN but InfinitePoint is OUT => need to reverse
            ShapeExtend_WireData sewd(aw);
            sewd.Reverse(myFace);
            ws.SetValue(i, sewd.Wire());
            // clang-format off
            SendWarning ( sewd.Wire(), Message_Msg ( "FixAdvFace.FixOrientation.MSG5" ) );// Wire on face was reversed
            // clang-format on
            aSeqReversed.Append(i);
            done = Standard_True;
          }
        }
      }
    }
  }

  if (isAddNaturalBounds && nb == aSeqReversed.Length())
    done = Standard_False;

  //    Faut-il reconstruire ? si myRebil est mis
  if (done)
  {
    TopoShape S = myFace.EmptyCopied();
    S.Orientation(TopAbs_FORWARD);
    Standard_Integer i = 1;
    for (; i <= nb; i++)
      B.Add(S, ws.Value(i));

    if (nb < nbAll)
    {
      for (i = 1; i <= nbAll; i++)
      {
        const TopoShape& aS2 = allSubShapes.Value(i);
        if (aS2.ShapeType() != TopAbs_WIRE
            || (aS2.Orientation() != TopAbs_FORWARD && aS2.Orientation() != TopAbs_REVERSED))
          B.Add(S, aS2);
      }
    }

    if (!myFwd)
      S.Orientation(TopAbs_REVERSED);
    if (!Context().IsNull())
      Context()->Replace(myFace, S);
    myFace = TopoDS::Face(S);
    BRepTools1::Update(myFace);
    Standard_Integer k = 1;
    for (; k <= aSeqReversed.Length(); k++)
    {
#ifdef OCCT_DEBUG
      // clang-format off
      std::cout<<"Wire no "<<aSeqReversed.Value(k)<<" of "<<nb<<" reversed"<<std::endl; // mise au point !
// clang-format on
#endif
    }
  }
  return done;
}

//=======================================================================
// function : CheckWire
// purpose  : auxiliary for FixMissingSeam
//=======================================================================
//: i7 abv 18 Sep 98: ProSTEP TR9 r0501-ug.stp: algorithm of fixing missing seam changed
// test whether the wire is opened on period of periodical surface
static Standard_Boolean CheckWire(const TopoWire&  wire,
                                  const TopoFace&  face,
                                  const Standard_Real dU,
                                  const Standard_Real dV,
                                  Standard_Integer&   isuopen,
                                  Standard_Integer&   isvopen,
                                  Standard_Boolean&   isDeg)
{
  Coords2d vec;
  vec.SetX(0);
  vec.SetY(0);
  Edge1 sae;

  isuopen = isvopen = 0;
  isDeg             = Standard_True;
  for (TopoDS_Iterator ed(wire); ed.More(); ed.Next())
  {
    TopoEdge edge = TopoDS::Edge(ed.Value());
    if (!BRepInspector::Degenerated(edge))
      isDeg = Standard_False;
    Handle(GeomCurve2d) c2d;
    Standard_Real        f, l;
    if (!sae.PCurve(edge, face, c2d, f, l, Standard_True))
      return Standard_False;
    vec += c2d->Value(l).XY() - c2d->Value(f).XY();
  }

  Standard_Real aDelta = Abs(vec.X()) - dU;
  if (Abs(aDelta) < 0.1 * dU)
  {
    if (vec.X() > 0.0)
    {
      isuopen = 1;
    }
    else
    {
      isuopen = -1;
    }
  }
  else
  {
    isuopen = 0;
  }

  aDelta = Abs(vec.Y()) - dV;
  if (Abs(aDelta) < 0.1 * dV)
  {
    if (vec.Y() > 0.0)
    {
      isvopen = 1;
    }
    else
    {
      isvopen = -1;
    }
  }
  else
  {
    isvopen = 0;
  }

  return isuopen || isvopen;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::FixMissingSeam()
{
  Standard_Boolean uclosed = mySurf->IsUClosed();
  Standard_Boolean vclosed = mySurf->IsVClosed();

  if (!uclosed && !vclosed)
    return Standard_False;

  if (!Context().IsNull())
  {
    TopoShape S = Context()->Apply(myFace);
    myFace         = TopoDS::Face(S);
  }

  //%pdn: surface should be made periodic before (see ShapeCustom_Surface)!
  if (mySurf->Surface()->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
  {
    Handle(Geom_BSplineSurface) BSpl = Handle(Geom_BSplineSurface)::DownCast(mySurf->Surface());
    if (!BSpl->IsUPeriodic() && !BSpl->IsVPeriodic())
      return Standard_False;
  }

  Standard_Real URange, VRange, SUF, SUL, SVF, SVL;
  mySurf->Bounds(SUF, SUL, SVF, SVL);
  Standard_Real fU1, fU2, fV1, fV2;
  BRepTools1::UVBounds(myFace, fU1, fU2, fV1, fV2);

  // pdn OCC55 fix to faces without the wires to avoid identical first and last parameters
  if (::Precision1::IsInfinite(SUF) || ::Precision1::IsInfinite(SUL))
  {
    if (::Precision1::IsInfinite(SUF))
      SUF = fU1;
    if (::Precision1::IsInfinite(SUL))
      SUL = fU2;
    if (Abs(SUL - SUF) < ::Precision1::PConfusion())
    {
      if (::Precision1::IsInfinite(SUF))
        SUF -= 1000.;
      else
        SUL += 1000.;
    }
  }
  if (::Precision1::IsInfinite(SVF) || ::Precision1::IsInfinite(SVL))
  {
    if (::Precision1::IsInfinite(SVF))
      SVF = fV1;
    if (::Precision1::IsInfinite(SVL))
      SVL = fV2;
    if (Abs(SVL - SVF) < ::Precision1::PConfusion())
    {
      if (::Precision1::IsInfinite(SVF))
        SVF -= 1000.;
      else
        SVL += 1000.;
    }
  }

  URange = Min(Abs(SUL - SUF), Precision1::Infinite());
  VRange = Min(Abs(SVL - SVF), Precision1::Infinite());
  //  Standard_Real UTol = 0.2 * URange, VTol = 0.2 * VRange;
  Standard_Integer ismodeu = 0, ismodev = 0; // szv#4:S4163:12Mar99 was Boolean
  Standard_Integer isdeg1 = 0, isdeg2 = 0;

  TopTools_SequenceOfShape ws;
  TopTools_SequenceOfShape aSeqNonManif;
  for (TopoDS_Iterator wi(myFace, Standard_False); wi.More(); wi.Next())
  {
    if (wi.Value().ShapeType() != TopAbs_WIRE
        || (wi.Value().Orientation() != TopAbs_FORWARD
            && wi.Value().Orientation() != TopAbs_REVERSED))
    {
      aSeqNonManif.Append(wi.Value());
      continue;
    }
    ws.Append(wi.Value());
  }

  TopoWire      w1, w2;
  Standard_Integer i;
  for (i = 1; i <= ws.Length(); i++)
  {
    TopoWire      wire = TopoDS::Wire(ws.Value(i));
    Standard_Integer isuopen, isvopen;
    Standard_Boolean isdeg;
    if (!CheckWire(wire, myFace, URange, VRange, isuopen, isvopen, isdeg))
      continue;
    if (w1.IsNull())
    {
      w1      = wire;
      ismodeu = isuopen;
      ismodev = isvopen;
      isdeg1  = isdeg ? i : 0;
    }
    else if (w2.IsNull())
    {
      if (ismodeu == -isuopen && ismodev == -isvopen)
      {
        w2     = wire;
        isdeg2 = isdeg ? i : 0;
      }
      else if (ismodeu == isuopen && ismodev == isvopen)
      {
        w2     = wire;
        isdeg2 = isdeg;
        //: abv 29.08.01: If wires are contraversal, reverse one of them
        // If first one is single degenerated edge, reverse it; else second
        if (isdeg1)
        {
          w1.Reverse();
          ismodeu = -ismodeu;
          ismodev = -ismodev;
        }
        else
        {
          w2.Reverse();
#ifdef OCCT_DEBUG
          if (!isdeg2)
            std::cout << "Warning: ShapeFix_Face::FixMissingSeam(): wire reversed" << std::endl;
#endif
        }
      }
#ifdef OCCT_DEBUG
      else
        std::cout << "Warning: ShapeFix_Face::FixMissingSeam(): incompatible open wires"
                  << std::endl;
#endif
    }
    //    else return Standard_False; //  abort
    else
    {
#ifdef OCCT_DEBUG
      std::cout << "Warning: ShapeFix_Face::FixMissingSeam(): more than two open wires detected!"
                << std::endl;
#endif
      //: abv 30.08.09: if more than one open wires and more than two of them are
      // completely degenerated, remove any of them
      if (isdeg || isdeg1 || isdeg2)
      {
        ws.Remove(isdeg ? i : isdeg2 ? isdeg2 : isdeg1);
        w1.Nullify();
        w2.Nullify();
        i = 0;
#ifdef OCCT_DEBUG
        std::cout << "Warning: ShapeFix_Face::FixMissingSeam(): open degenerated wire removed"
                  << std::endl;
#endif
        continue;
      }
    }
  }

  ShapeBuilder                 B;
  Handle(Geom_ToroidalSurface) aTorSurf = Handle(Geom_ToroidalSurface)::DownCast(mySurf->Surface());
  Standard_Boolean             anIsDegeneratedTor =
    (aTorSurf.IsNull() ? Standard_False : aTorSurf->MajorRadius() < aTorSurf->MinorRadius());

  if (w1.IsNull())
    return Standard_False;
  else if (w2.IsNull())
  {
    // For spheres and BSpline cone-like surfaces(bug 24055):
    // If only one of wires limiting face on surface is open in 2d,
    // this may means that degenerated edge should be added, and
    // then usual procedure applied
    gp_Pnt2d      p;
    gp_Dir2d      d;
    Standard_Real aRange;

    if (ismodeu && anIsDegeneratedTor)
    {
      Standard_Real aRa  = aTorSurf->MajorRadius();
      Standard_Real aRi  = aTorSurf->MinorRadius();
      Standard_Real aPhi = ACos(-aRa / aRi);
      p.SetCoord(0.0, (ismodeu > 0 ? M_PI + aPhi : aPhi));

      Standard_Real aXCoord = -ismodeu;
      d.SetCoord(aXCoord, 0.);
      aRange = 2. * M_PI;
    }
    else if (ismodeu && mySurf->Surface()->IsKind(STANDARD_TYPE(Geom_SphericalSurface)))
    {
      p.SetCoord((ismodeu < 0 ? 0. : 2. * M_PI), ismodeu * 0.5 * M_PI);
      Standard_Real aXCoord = -ismodeu;
      d.SetCoord(aXCoord, 0.);
      aRange = 2. * M_PI;
    }
    else if (ismodev && mySurf->Surface()->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
    {
      Standard_Real uCoord;
      if (mySurf->Value(SUF, SVF).Distance(mySurf->Value(SUF, (SVF + SVL) / 2))
          < ::Precision1::Confusion())
        uCoord = SUF;
      else if (mySurf->Value(SUL, SVF).Distance(mySurf->Value(SUL, (SVF + SVL) / 2))
               < ::Precision1::Confusion())
        uCoord = SUL;
      else
        return Standard_False;

      p.SetCoord(uCoord, (ismodev < 0 ? 0. : VRange));
      d.SetCoord(0., -ismodev);
      aRange = VRange;
    }
    else if (ismodeu && mySurf->Surface()->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
    {
      Standard_Real vCoord;
      if (mySurf->Value(SUF, SVF).Distance(mySurf->Value((SUF + SUL) / 2, SVF))
          < ::Precision1::Confusion())
        vCoord = SVF;
      else if (mySurf->Value(SUL, SVL).Distance(mySurf->Value((SUF + SUL) / 2, SVL))
               < ::Precision1::Confusion())
        vCoord = SVL;
      else
        return Standard_False;

      p.SetCoord((ismodeu < 0 ? 0. : URange), vCoord);
      Standard_Real aXCoord = -ismodeu;
      d.SetCoord(aXCoord, 0.);
      aRange = URange;
    }
    else
      return Standard_False;

    Handle(Geom2d_Line) line = new Geom2d_Line(p, d);
    TopoEdge         edge;
    B.MakeEdge(edge);
    B.Degenerated(edge, Standard_True);
    B.UpdateEdge(edge, line, myFace, ::Precision1::Confusion());
    B.Range(edge, myFace, 0., aRange);
    TopoVertex V;
    B.MakeVertex(V, mySurf->Value(p.X(), p.Y()), ::Precision1::Confusion());
    V.Orientation(TopAbs_FORWARD);
    B.Add(edge, V);
    V.Orientation(TopAbs_REVERSED);
    B.Add(edge, V);
    B.MakeWire(w2);
    B.Add(w2, edge);
    ws.Append(w2);
  }

  // Check consistency of orientations of the two wires that need to be connected by a seam
  Standard_Real    uf = SUF, vf = SVF;
  Standard_Integer coord  = (ismodeu ? 1 : 0);
  Standard_Integer isneg  = (ismodeu ? ismodeu : -ismodev);
  Standard_Real    period = (ismodeu ? URange : VRange);
  TopoShape     S;
  Standard_Real    m1[2][2], m2[2][2];
  S = myFace.EmptyCopied();
  B.Add(S, w1);
  ShapeAnalysis1::GetFaceUVBounds(TopoDS::Face(S), m1[0][0], m1[0][1], m1[1][0], m1[1][1]);
  S = myFace.EmptyCopied();
  B.Add(S, w2);
  ShapeAnalysis1::GetFaceUVBounds(TopoDS::Face(S), m2[0][0], m2[0][1], m2[1][0], m2[1][1]);

  // For the case when surface is closed only in one direction it is necessary to check
  // validity of orientation of the open wires in parametric space.
  // In case of U closed surface wire with minimal V coordinate should be directed in positive
  // direction by U In case of V closed surface wire with minimal U coordinate should be directed in
  // negative direction by V
  if (!vclosed || !uclosed || anIsDegeneratedTor)
  {
    Standard_Real deltaOther =
      0.5 * (m2[coord][0] + m2[coord][1]) - 0.5 * (m1[coord][0] + m1[coord][1]);
    if (deltaOther * isneg < 0)
    {
      w1.Reverse();
      w2.Reverse();
    }
  }

  // sort original wires
  Handle(WireHealer) sfw = new WireHealer;
  sfw->SetFace(myFace);
  sfw->SetPrecision(Precision1());
  Handle(ShapeExtend_WireData) wd1 = new ShapeExtend_WireData(w1);
  Handle(ShapeExtend_WireData) wd2 = new ShapeExtend_WireData(w2);
  sfw->Load(wd1);
  sfw->FixReorder();
  sfw->Load(wd2);
  sfw->FixReorder();
  TopoWire w11 = wd1->Wire();
  TopoWire w21 = wd2->Wire();

  //: abv 29.08.01: reconstruct face taking into account reversing
  TopoShape dummy = myFace.EmptyCopied();
  TopoFace  tmpF  = TopoDS::Face(dummy);
  tmpF.Orientation(TopAbs_FORWARD);
  for (i = 1; i <= ws.Length(); i++)
  {
    TopoWire wire = TopoDS::Wire(ws.Value(i));
    if (wire.IsSame(w1))
      wire = w11;
    else if (wire.IsSame(w2))
      wire = w21;
    else
    {
      // other wires (not boundary) are considered as holes; make sure to have them oriented
      // accordingly
      TopoShape curface = tmpF.EmptyCopied();
      B.Add(curface, wire);
      curface.Orientation(myFace.Orientation());
      if (ShapeAnalysis1::IsOuterBound(TopoDS::Face(curface)))
        wire.Reverse();
    }
    B.Add(tmpF, wire);
  }

  tmpF.Orientation(myFace.Orientation());

  // A special kind of FixShifted is necessary for torus-like
  // surfaces to adjust wires by period ALONG the missing SEAM direction
  // tr9_r0501-ug.stp #187640
  if (uclosed && vclosed && !anIsDegeneratedTor)
  {
    Standard_Real shiftw2 = ShapeAnalysis1::AdjustByPeriod(
      0.5 * (m2[coord][0] + m2[coord][1]),
      0.5 * (m1[coord][0] + m1[coord][1] + isneg * (period + ::Precision1::PConfusion())),
      period);
    m1[coord][0] = Min(m1[coord][0], m2[coord][0] + shiftw2);
    m1[coord][1] = Max(m1[coord][1], m2[coord][1] + shiftw2);
    for (TopoDS_Iterator it(tmpF, Standard_False); it.More(); it.Next())
    {
      if (it.Value().ShapeType() != TopAbs_WIRE)
        continue;
      TopoWire w = TopoDS::Wire(it.Value());
      if (w == w11)
        continue;
      Standard_Real shift;
      if (w == w21)
        shift = shiftw2;

      else
      {
        S = tmpF.EmptyCopied();
        B.Add(S, w);
        ShapeAnalysis1::GetFaceUVBounds(TopoDS::Face(S), m2[0][0], m2[0][1], m2[1][0], m2[1][1]);
        shift = ShapeAnalysis1::AdjustByPeriod(0.5 * (m2[coord][0] + m2[coord][1]),
                                              0.5 * (m1[coord][0] + m1[coord][1]),
                                              period);
      }
      if (shift != 0.)
      {
        gp_Vec2d V(0., 0.);
        V.SetCoord(coord + 1, shift);
        Edge1 sae;
        for (TopoDS_Iterator iw(w); iw.More(); iw.Next())
        {
          TopoEdge          E = TopoDS::Edge(iw.Value());
          Handle(GeomCurve2d) C;
          Standard_Real        a, b;
          if (!sae.PCurve(E, tmpF, C, a, b))
            continue;
          C->Translate(V);
        }
      }
    }
    // abv 05 Feb 02: OCC34
    // by the way, select proper split place by V to avoid extra intersections
    if (m1[coord][1] - m1[coord][0] <= period)
    {
      Standard_Real other = 0.5 * (m1[coord][0] + m1[coord][1] - period);
      if (ismodeu)
        vf = other;
      else
        uf = other;
    }
  }

  // find the best place by u and v to insert a seam
  // (so as to minimize splitting edges as possible)
  Edge1 sae;
  Standard_Integer   foundU = 0, foundV = 0;
  Standard_Integer   nb1 = wd1->NbEdges();
  Standard_Integer   nb2 = wd2->NbEdges();
  for (Standard_Integer i1 = 1; i1 <= nb1 + nb2; i1++)
  {
    TopoEdge          edge1 = (i1 <= nb1 ? wd1->Edge(i1) : wd2->Edge(i1 - nb1));
    Handle(GeomCurve2d) c2d;
    Standard_Real        f, l;
    if (!sae.PCurve(edge1, tmpF, c2d, f, l, Standard_True))
      return Standard_False;
    gp_Pnt2d pos1 = c2d->Value(l).XY();
    // the best place is end of edge which is nearest to 0
    Standard_Boolean skipU = !uclosed;
    if (uclosed && ismodeu)
    {
      pos1.SetX(pos1.X() + ShapeAnalysis1::AdjustByPeriod(pos1.X(), SUF, URange));
      if (foundU == 2 && Abs(pos1.X()) > Abs(uf))
        skipU = Standard_True;
      else if (!foundU || (foundU == 1 && Abs(pos1.X()) < Abs(uf)))
      {
        foundU = 1;
        uf     = pos1.X();
      }
    }
    Standard_Boolean skipV = !vclosed;
    if (vclosed && !ismodeu)
    {
      pos1.SetY(pos1.Y() + ShapeAnalysis1::AdjustByPeriod(pos1.Y(), SVF, VRange));
      if (foundV == 2 && Abs(pos1.Y()) > Abs(vf))
        skipV = Standard_True;
      else if (!foundV || (foundV == 1 && Abs(pos1.Y()) < Abs(vf)))
      {
        foundV = 1;
        vf     = pos1.Y();
      }
    }
    if (skipU && skipV)
    {
      if (i1 <= nb1)
        continue;
      else
        break;
    }
    // or yet better - if it is end of some edges on both wires
    for (Standard_Integer i2 = 1; i1 <= nb1 && i2 <= nb2; i2++)
    {
      TopoEdge edge2 = wd2->Edge(i2);
      if (!sae.PCurve(edge2, tmpF, c2d, f, l, Standard_True))
        return Standard_False;
      gp_Pnt2d pos2 = c2d->Value(f).XY();
      if (uclosed && ismodeu)
      {
        pos2.SetX(pos2.X() + ShapeAnalysis1::AdjustByPeriod(pos2.X(), pos1.X(), URange));
        if (Abs(pos2.X() - pos1.X()) < ::Precision1::PConfusion()
            && (foundU != 2 || Abs(pos1.X()) < Abs(uf)))
        {
          foundU = 2;
          uf     = pos1.X();
        }
      }
      if (vclosed && !ismodeu)
      {
        pos2.SetY(pos2.Y() + ShapeAnalysis1::AdjustByPeriod(pos2.Y(), pos1.Y(), VRange));
        if (Abs(pos2.Y() - pos1.Y()) < ::Precision1::PConfusion()
            && (foundV != 2 || Abs(pos1.Y()) < Abs(vf)))
        {
          foundV = 2;
          vf     = pos1.Y();
        }
      }
    }
  }

  // pdn fixing RTS on offsets
  if (uf < SUF || uf > SUL)
    uf += ShapeAnalysis1::AdjustToPeriod(uf, SUF, SUF + URange);
  if (vf < SVF || vf > SVL)
    vf += ShapeAnalysis1::AdjustToPeriod(vf, SVF, SVF + VRange);

  // Create fictive grid and call ComposeShell to insert a seam
  Handle(Geom_RectangularTrimmedSurface) RTS =
    new Geom_RectangularTrimmedSurface(mySurf->Surface(), uf, uf + URange, vf, vf + VRange);
  Handle(TColGeom_HArray2OfSurface) grid = new TColGeom_HArray2OfSurface(1, 1, 1, 1);
  grid->SetValue(1, 1, RTS); // mySurf->Surface() );
  Handle(ShapeExtend_CompositeSurface) G = new ShapeExtend_CompositeSurface(grid);
  TopLoc_Location                      L;

  // addition non-manifold topology
  Standard_Integer j = 1;
  for (; j <= aSeqNonManif.Length(); j++)
    B.Add(tmpF, aSeqNonManif.Value(j));

  ShapeFix_ComposeShell CompShell;
  //  TopoFace tmpF = myFace;
  //  tmpF.Orientation(TopAbs_FORWARD);
  CompShell.Init(G, L, tmpF, ::Precision1::Confusion()); // myPrecision
  if (Context().IsNull())
    SetContext(new ShapeBuild_ReShape);
  CompShell.ClosedMode() = Standard_True;
  CompShell.SetContext(Context());
  CompShell.SetMaxTolerance(MaxTolerance());
  CompShell.Perform();

  // abv 03.07.00: CAX-IF TRJ4: trj4_k1_goe-tu-214.stp: #785: reset mySurf
  mySurf = new ShapeAnalysis_Surface(RTS);

  myResult = CompShell.Result();

  Context()->Replace(myFace, myResult);

  // Remove small wires and / or faces that can be generated by ComposeShell
  // (see tests bugs step bug30052_4, de step_3 E6)
  Standard_Integer nbFaces = 0;
  ShapeExplorer  expF(myResult, TopAbs_FACE);
  for (; expF.More(); expF.Next())
  {
    TopoFace      aFace = TopoDS::Face(expF.Value());
    ShapeExplorer  aExpW(aFace, TopAbs_WIRE);
    Standard_Integer nbWires = 0;
    for (; aExpW.More(); aExpW.Next())
    {
      WireHealer aSfw(TopoDS::Wire(aExpW.Value()), aFace, Precision1());
      aSfw.SetContext(Context());
      if (aSfw.NbEdges())
        aSfw.FixSmall(Standard_True, Precision1());
      if (!aSfw.NbEdges())
      {
        Context()->Remove(aExpW.Value());
        continue;
      }
      nbWires++;
    }
    if (!nbWires)
    {
      Context()->Remove(aFace);
      continue;
    }
    nbFaces++;
  }

  myResult = Context()->Apply(myResult);
  for (ShapeExplorer exp(myResult, TopAbs_FACE); exp.More(); exp.Next())
  {
    myFace = TopoDS::Face(Context()->Apply(exp.Current()));
    if (myFace.IsNull())
      continue;
    if (nbFaces > 1)
    {
      FixSmallAreaWire(Standard_True);
      TopoShape aShape = Context()->Apply(myFace);
      if (aShape.IsNull())
        continue;
      myFace = TopoDS::Face(aShape);
    }
    BRepTools1::Update(myFace); //: p4
  }
  myResult = Context()->Apply(myResult);

  SendWarning(Message_Msg("FixAdvFace.FixMissingSeam.MSG0")); // Missing seam-edge added
  return Standard_True;
}

//=================================================================================================

//%14 pdn 24.02.99 PRO10109, USA60293 fix wire on face with small area.
Standard_Boolean ShapeFix_Face::FixSmallAreaWire(const Standard_Boolean theIsRemoveSmallFace)
{
  if (!Context().IsNull())
  {
    TopoShape aShape = Context()->Apply(myFace);
    myFace              = TopoDS::Face(aShape);
  }

  ShapeBuilder     aBuilder;
  Standard_Integer nbRemoved = 0, nbWires = 0;

  TopoShape anEmptyCopy = myFace.EmptyCopied();
  TopoFace  aFace       = TopoDS::Face(anEmptyCopy);
  aFace.Orientation(TopAbs_FORWARD);

  const Standard_Real aTolerance3d = ShapeFix_Root::Precision1();
  for (TopoDS_Iterator aWIt(myFace, Standard_False); aWIt.More(); aWIt.Next())
  {
    const TopoShape& aShape = aWIt.Value();
    if (aShape.ShapeType() != TopAbs_WIRE && aShape.Orientation() != TopAbs_FORWARD
        && aShape.Orientation() != TopAbs_REVERSED)
    {
      continue;
    }

    const TopoWire&         aWire      = TopoDS::Wire(aShape);
    Handle(ShapeAnalysis_Wire) anAnalyzer = new ShapeAnalysis_Wire(aWire, myFace, aTolerance3d);
    if (anAnalyzer->CheckSmallArea(aWire))
    {
      // Null area wire detected, wire skipped
      SendWarning(aWire, Message_Msg("FixAdvFace.FixSmallAreaWire.MSG0"));
      ++nbRemoved;
    }
    else
    {
      aBuilder.Add(aFace, aWire);
      ++nbWires;
    }
  }

  if (nbRemoved <= 0)
    return Standard_False;

  if (nbWires <= 0)
  {
#ifdef OCCT_DEBUG
    std::cout << "Warning: ShapeFix_Face: All wires on a face have small area; left untouched"
              << std::endl;
#endif
    if (theIsRemoveSmallFace && !Context().IsNull())
      Context()->Remove(myFace);

    return Standard_False;
  }
#ifdef OCCT_DEBUG
  std::cout << "Warning: ShapeFix_Face: " << nbRemoved << " small area wire(s) removed"
            << std::endl;
#endif
  aFace.Orientation(myFace.Orientation());
  if (!Context().IsNull())
    Context()->Replace(myFace, aFace);

  myFace = aFace;
  return Standard_True;
}

//=================================================================================================

static void FindNext(const TopoShape&                 aVert,
                     const TopoShape&                 ainitEdge,
                     TopTools_IndexedMapOfShape&         aMapVertices,
                     TopTools_DataMapOfShapeListOfShape& aMapVertexEdges,
                     const TopTools_MapOfShape&          aMapSmallEdges,
                     const TopTools_MapOfShape&          aMapSeemEdges,
                     TopTools_MapOfShape&                aMapEdges,
                     Handle(ShapeExtend_WireData)&       aWireData)
{
  TopoDS_Iterator  aItV(ainitEdge);
  TopoShape     anextVert = aVert;
  Standard_Boolean isFind    = Standard_False;
  for (; aItV.More() && !isFind; aItV.Next())
  {
    if (!aItV.Value().IsSame(aVert))
    {
      isFind    = Standard_True;
      anextVert = aItV.Value();
    }
  }

  if (!isFind && !aMapSmallEdges.Contains(ainitEdge))
    return;
  if (isFind && aMapVertices.Contains(anextVert))
    return;

  const ShapeList&        aledges = aMapVertexEdges.Find(anextVert);
  TopTools_ListIteratorOfListOfShape liter(aledges);
  isFind = Standard_False;
  TopoShape anextEdge;
  for (; liter.More() && !isFind; liter.Next())
  {
    if (!aMapEdges.Contains(liter.Value()) && !liter.Value().IsSame(ainitEdge))
    {
      anextEdge = liter.Value();
      aWireData->Add(anextEdge);
      if (aMapSeemEdges.Contains(anextEdge))
        aWireData->Add(anextEdge.Reversed());
      isFind = Standard_True;
      aMapEdges.Add(anextEdge);
      FindNext(anextVert,
               anextEdge,
               aMapVertices,
               aMapVertexEdges,
               aMapSmallEdges,
               aMapSeemEdges,
               aMapEdges,
               aWireData);
    }
  }
  return;
}

static Standard_Boolean isClosed2D(const TopoFace& aFace, const TopoWire& aWire)
{
  Standard_Boolean           isClosed = Standard_True;
  Handle(ShapeAnalysis_Wire) asaw = new ShapeAnalysis_Wire(aWire, aFace, Precision1::Confusion());
  for (Standard_Integer i = 1; i <= asaw->NbEdges() && isClosed; i++)
  {
    TopoEdge edge1 = asaw->WireData()->Edge(i);
    // checking that wire is closed in 2D space with tolerance of vertex.
    Edge1 sae;
    TopoVertex      v1 = sae.FirstVertex(edge1);
    asaw->SetPrecision(BRepInspector::Tolerance(v1));
    asaw->CheckGap2d(i);
    isClosed = (asaw->LastCheckStatus(ShapeExtend_OK));
  }
  return isClosed;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::FixLoopWire(TopTools_SequenceOfShape& aResWires)
{
  TopTools_IndexedMapOfShape         aMapVertices;
  TopTools_DataMapOfShapeListOfShape aMapVertexEdges;
  TopTools_MapOfShape                aMapSmallEdges;
  TopTools_MapOfShape                aMapSeemEdges;
  if (!FixWireTool()->Analyzer()->CheckLoop(aMapVertices,
                                            aMapVertexEdges,
                                            aMapSmallEdges,
                                            aMapSeemEdges))
    return Standard_False;

  TopTools_MapOfShape      aMapEdges;
  TopTools_SequenceOfShape aSeqWires;

  // collecting wires from common vertex belonging more than 2 edges
  Standard_Integer i = 1;
  for (; i <= aMapVertices.Extent(); i++)
  {
    TopoShape                       aVert   = aMapVertices.FindKey(i);
    const ShapeList&        aledges = aMapVertexEdges.Find(aVert);
    TopTools_ListIteratorOfListOfShape liter(aledges);
    for (; liter.More(); liter.Next())
    {
      TopoEdge Edge = TopoDS::Edge(liter.Value());
      if (aMapEdges.Contains(Edge))
        continue;

      Handle(ShapeExtend_WireData) aWireData = new ShapeExtend_WireData;
      aWireData->Add(Edge);
      if (aMapSeemEdges.Contains(Edge))
        aWireData->Add(Edge.Reversed());
      aMapEdges.Add(Edge);
      FindNext(aVert,
               Edge,
               aMapVertices,
               aMapVertexEdges,
               aMapSmallEdges,
               aMapSeemEdges,
               aMapEdges,
               aWireData);
      if (aWireData->NbEdges() == 1 && aMapSmallEdges.Contains(aWireData->Edge(1)))
        continue;
      TopoVertex aV1, aV2;
      TopoWire   aWire = aWireData->Wire();
      TopExp1::Vertices(aWire, aV1, aV2);

      if (aV1.IsSame(aV2))
      {
        Handle(ShapeExtend_WireData) asewd = new ShapeExtend_WireData(aWire);
        Handle(WireHealer)        asfw  = new WireHealer;
        asfw->Load(asewd);
        asfw->FixReorder();
        TopoWire awire2 = asfw->Wire();
        aResWires.Append(awire2);
      }
      else
        aSeqWires.Append(aWireData->Wire());
    }
  }

  if (aSeqWires.Length() == 1)
  {
    aResWires.Append(aSeqWires.Value(1));
  }
  else
  {
    // collecting whole wire from two not closed wires having two common vertices.
    for (i = 1; i <= aSeqWires.Length(); i++)
    {
      TopoVertex aV1, aV2;
      TopoWire   aWire = TopoDS::Wire(aSeqWires.Value(i));
      TopExp1::Vertices(aWire, aV1, aV2);
      Standard_Integer j = i + 1;
      for (; j <= aSeqWires.Length(); j++)
      {
        TopoVertex aV21, aV22;
        TopoWire   aWire2 = TopoDS::Wire(aSeqWires.Value(j));
        TopExp1::Vertices(aWire2, aV21, aV22);
        if ((aV1.IsSame(aV21) || aV1.IsSame(aV22)) && (aV2.IsSame(aV21) || aV2.IsSame(aV22)))
        {
          Handle(ShapeExtend_WireData) asewd = new ShapeExtend_WireData(aWire);
          asewd->Add(aWire2);
          Handle(WireHealer) asfw = new WireHealer;
          asfw->Load(asewd);
          asfw->FixReorder();
          aResWires.Append(asfw->Wire());
          aSeqWires.Remove(j--);
          myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE7);
          break;
        }
      }
      if (j <= aSeqWires.Length())
        aSeqWires.Remove(i--);
    }
    if (aSeqWires.Length() < 3)
    {
      for (i = 1; i <= aSeqWires.Length(); i++)
        aResWires.Append(aSeqWires.Value(i));
    }
    else
    {
      // collecting wires having one common vertex
      for (i = 1; i <= aSeqWires.Length(); i++)
      {
        TopoVertex aV1, aV2;
        TopoWire   aWire = TopoDS::Wire(aSeqWires.Value(i));
        TopExp1::Vertices(aWire, aV1, aV2);
        Standard_Integer j = i + 1;
        for (; j <= aSeqWires.Length(); j++)
        {
          TopoVertex aV21, aV22;
          TopoWire   aWire2 = TopoDS::Wire(aSeqWires.Value(j));
          TopExp1::Vertices(aWire2, aV21, aV22);
          if ((aV1.IsSame(aV21) || aV1.IsSame(aV22)) || (aV2.IsSame(aV21) || aV2.IsSame(aV22)))
          {
            Handle(ShapeExtend_WireData) asewd = new ShapeExtend_WireData(aWire);
            asewd->Add(aWire2);
            Handle(WireHealer) asfw = new WireHealer;
            asfw->Load(asewd);
            asfw->FixReorder();
            aWire = asfw->Wire();
            TopExp1::Vertices(aWire, aV1, aV2);
            aSeqWires.Remove(j--);
            myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE7);
          }
        }
        aResWires.Append(aWire);
      }
    }
  }
  Standard_Boolean isClosed = Standard_True;

  // checking that obtained wires is closed in 2D space
  if (mySurf->Adaptor3d()->GetType() != GeomAbs_Plane)
  {

    TopoShape emptyCopied = myFace.EmptyCopied();
    TopoFace  tmpFace     = TopoDS::Face(emptyCopied);
    tmpFace.Orientation(TopAbs_FORWARD);

    for (i = 1; i <= aResWires.Length() && isClosed; i++)
    {
      TopoWire awire = TopoDS::Wire(aResWires.Value(i));
      isClosed          = isClosed2D(tmpFace, awire);
    }
  }

  Standard_Boolean isDone = (aResWires.Length() && isClosed);
  if (isDone && aResWires.Length() > 1)
  {
#ifdef OCCT_DEBUG
    std::cout << "Wire was split on " << aResWires.Length() << " wires" << std::endl;
#endif
  }

  return isDone;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::SplitEdge(const Handle(ShapeExtend_WireData)& sewd,
                                          const Standard_Integer              num,
                                          const Standard_Real                 param,
                                          const TopoVertex&                vert,
                                          const Standard_Real                 preci,
                                          ShapeFix_DataMapOfShapeBox2d&       boxes)
{
  TopoEdge        edge = sewd->Edge(num);
  TopoEdge        newE1, newE2;
  SplitTool aTool;
  if (aTool.SplitEdge(edge, param, vert, myFace, newE1, newE2, preci, 0.01 * preci))
  {
    // change context
    Handle(ShapeExtend_WireData) wd = new ShapeExtend_WireData;
    wd->Add(newE1);
    wd->Add(newE2);
    if (!Context().IsNull())
      Context()->Replace(edge, wd->Wire());
    for (ShapeExplorer exp(wd->Wire(), TopAbs_EDGE); exp.More(); exp.Next())
    {
      TopoEdge E = TopoDS::Edge(exp.Current());
      BRepTools1::Update(E);
    }

    //    for ( Standard_Integer i=1; i <= sewd->NbEdges(); i++ ) {
    //      TopoEdge E = sewd->Edge(i);
    //      TopoShape S = Context()->Apply ( E );
    //      if ( S == E ) continue;
    //      for ( ShapeExplorer exp(S,TopAbs_EDGE); exp.More(); exp.Next() )
    //        sewd->Add ( exp.Current(), i++ );
    //      sewd->Remove ( i-- );
    //    }

    // change sewd and boxes
    sewd->Set(newE1, num);
    if (num == sewd->NbEdges())
      sewd->Add(newE2);
    else
      sewd->Add(newE2, num + 1);

    boxes.UnBind(edge);
    TopLoc_Location             L;
    const Handle(GeomSurface)& S = BRepInspector::Surface(myFace, L);
    Handle(GeomCurve2d)        c2d;
    Standard_Real               cf, cl;
    Edge1          sae;
    if (sae.PCurve(newE1, S, L, c2d, cf, cl, Standard_False))
    {
      Bnd_Box2d           box;
      Geom2dAdaptor_Curve gac;
      Standard_Real       aFirst = c2d->FirstParameter();
      Standard_Real       aLast  = c2d->LastParameter();
      if (c2d->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve)) && (cf < aFirst || cl > aLast))
      {
        // pdn avoiding problems with segment in Box2
        gac.Load(c2d);
      }
      else
        gac.Load(c2d, cf, cl);
      Add2dCurve::Add(gac, ::Precision1::Confusion(), box);
      boxes.Bind(newE1, box);
    }
    if (sae.PCurve(newE2, S, L, c2d, cf, cl, Standard_False))
    {
      Bnd_Box2d           box;
      Geom2dAdaptor_Curve gac;
      Standard_Real       aFirst = c2d->FirstParameter();
      Standard_Real       aLast  = c2d->LastParameter();
      if (c2d->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve)) && (cf < aFirst || cl > aLast))
      {
        // pdn avoiding problems with segment in Box2
        gac.Load(c2d);
      }
      else
        gac.Load(c2d, cf, cl);
      Add2dCurve::Add(gac, ::Precision1::Confusion(), box);
      boxes.Bind(newE2, box);
    }
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::SplitEdge(const Handle(ShapeExtend_WireData)& sewd,
                                          const Standard_Integer              num,
                                          const Standard_Real                 param1,
                                          const Standard_Real                 param2,
                                          const TopoVertex&                vert,
                                          const Standard_Real                 preci,
                                          ShapeFix_DataMapOfShapeBox2d&       boxes)
{
  TopoEdge        edge = sewd->Edge(num);
  TopoEdge        newE1, newE2;
  SplitTool aTool;
  if (aTool.SplitEdge(edge, param1, param2, vert, myFace, newE1, newE2, preci, 0.01 * preci))
  {
    // change context
    Handle(ShapeExtend_WireData) wd = new ShapeExtend_WireData;
    wd->Add(newE1);
    wd->Add(newE2);
    if (!Context().IsNull())
      Context()->Replace(edge, wd->Wire());
    for (ShapeExplorer exp(wd->Wire(), TopAbs_EDGE); exp.More(); exp.Next())
    {
      TopoEdge E = TopoDS::Edge(exp.Current());
      BRepTools1::Update(E);
    }

    // change sewd and boxes
    sewd->Set(newE1, num);
    if (num == sewd->NbEdges())
      sewd->Add(newE2);
    else
      sewd->Add(newE2, num + 1);

    boxes.UnBind(edge);
    TopLoc_Location             L;
    const Handle(GeomSurface)& S = BRepInspector::Surface(myFace, L);
    Handle(GeomCurve2d)        c2d;
    Standard_Real               cf, cl;
    Edge1          sae;
    if (sae.PCurve(newE1, S, L, c2d, cf, cl, Standard_False))
    {
      Bnd_Box2d           box;
      Geom2dAdaptor_Curve gac;
      Standard_Real       aFirst = c2d->FirstParameter();
      Standard_Real       aLast  = c2d->LastParameter();
      if (c2d->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve)) && (cf < aFirst || cl > aLast))
      {
        // pdn avoiding problems with segment in Box2
        gac.Load(c2d);
      }
      else
        gac.Load(c2d, cf, cl);
      Add2dCurve::Add(gac, ::Precision1::Confusion(), box);
      boxes.Bind(newE1, box);
    }
    if (sae.PCurve(newE2, S, L, c2d, cf, cl, Standard_False))
    {
      Bnd_Box2d           box;
      Geom2dAdaptor_Curve gac;
      Standard_Real       aFirst = c2d->FirstParameter();
      Standard_Real       aLast  = c2d->LastParameter();
      if (c2d->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve)) && (cf < aFirst || cl > aLast))
      {
        // pdn avoiding problems with segment in Box2
        gac.Load(c2d);
      }
      else
        gac.Load(c2d, cf, cl);
      Add2dCurve::Add(gac, ::Precision1::Confusion(), box);
      boxes.Bind(newE2, box);
    }
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::FixIntersectingWires()
{
  ShapeFix_IntersectionTool ITool(Context(), Precision1(), MaxTolerance());
  return ITool.FixIntersectingWires(myFace);
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::FixWiresTwoCoincEdges()
{
  if (!Context().IsNull())
  {
    TopoShape S = Context()->Apply(myFace);
    myFace         = TopoDS::Face(S);
  }

  TopAbs_Orientation ori         = myFace.Orientation();
  TopoShape       emptyCopied = myFace.EmptyCopied();
  TopoFace        face        = TopoDS::Face(emptyCopied);
  face.Orientation(TopAbs_FORWARD);
  Standard_Integer nbWires = 0;
  ShapeBuilder     B;

  for (TopoDS_Iterator it(myFace, Standard_False); it.More(); it.Next())
  {
    if (it.Value().ShapeType() != TopAbs_WIRE
        || (it.Value().Orientation() != TopAbs_FORWARD
            && it.Value().Orientation() != TopAbs_REVERSED))
    {
      continue;
    }
    nbWires++;
  }
  if (nbWires < 2)
    return Standard_False;
  Standard_Boolean isFixed = Standard_False;
  for (TopoDS_Iterator wi(myFace, Standard_False); wi.More(); wi.Next())
  {
    if (wi.Value().ShapeType() != TopAbs_WIRE
        || (wi.Value().Orientation() != TopAbs_FORWARD
            && wi.Value().Orientation() != TopAbs_REVERSED))
    {
      B.Add(face, wi.Value());
      continue;
    }
    TopoWire                  wire = TopoDS::Wire(wi.Value());
    Handle(ShapeExtend_WireData) sewd = new ShapeExtend_WireData(wire);
    if (sewd->NbEdges() == 2)
    {
      TopoEdge E1 = sewd->Edge(1);
      TopoEdge E2 = sewd->Edge(2);
      E1.Orientation(TopAbs_FORWARD);
      E2.Orientation(TopAbs_FORWARD);
      if (!(E1 == E2))
      {
        B.Add(face, wire);
      }
      else
        isFixed = Standard_True;
    }
    else
    {
      B.Add(face, wire);
    }
  }
  if (isFixed)
  {
    face.Orientation(ori);
    if (!Context().IsNull())
      Context()->Replace(myFace, face);
    myFace = face;
  }

  return isFixed;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::FixSplitFace(const TopTools_DataMapOfShapeListOfShape& MapWires)
{
  ShapeBuilder             B;
  TopTools_SequenceOfShape faces;
  TopoShape             S = myFace;
  if (!Context().IsNull())
    S = Context()->Apply(myFace);
  Standard_Integer NbWires = 0, NbWiresNew = 0, NbEdges;
  for (TopoDS_Iterator iter(S, Standard_False); iter.More(); iter.Next())
  {
    const TopoShape& aShape = iter.Value();
    if (aShape.ShapeType() != TopAbs_WIRE
        || (aShape.Orientation() != TopAbs_FORWARD && aShape.Orientation() != TopAbs_REVERSED))
      continue;
    TopoWire wire = TopoDS::Wire(aShape);
    NbWires++;
    if (MapWires.IsBound(wire))
    {
      // if wire not closed --> stop split and return false
      Handle(ShapeExtend_WireData) sewd = new ShapeExtend_WireData(wire);
      NbEdges                           = sewd->NbEdges();
      if (NbEdges == 0)
      {
        continue;
      }
      //
      TopoEdge        E1 = sewd->Edge(1);
      TopoEdge        E2 = sewd->Edge(NbEdges);
      TopoVertex      V1, V2;
      Edge1 sae;
      V1 = sae.FirstVertex(E1);
      V2 = sae.LastVertex(E2);
      if (!V1.IsSame(V2))
      {
#ifdef OCCT_DEBUG
        std::cout << "wire not closed --> stop split" << std::endl;
#endif
        return Standard_False;
      }
      // create face
      TopoShape emptyCopied = S.EmptyCopied();
      TopoFace  tmpFace     = TopoDS::Face(emptyCopied);
      tmpFace.Orientation(TopAbs_FORWARD);
      B.Add(tmpFace, wire);
      NbWiresNew++;
      const ShapeList&        IntWires = MapWires.Find(wire);
      TopTools_ListIteratorOfListOfShape liter(IntWires);
      for (; liter.More(); liter.Next())
      {
        TopoShape aShapeEmptyCopied = tmpFace.EmptyCopied();
        TopoFace  aFace             = TopoDS::Face(aShapeEmptyCopied);
        aFace.Orientation(TopAbs_FORWARD);
        B.Add(aFace, liter.Value());
        BRepTopAdaptor_FClass2d clas(aFace, ::Precision1::PConfusion());
        TopAbs_State            staout = clas.PerformInfinitePoint();
        if (staout == TopAbs_IN)
          B.Add(tmpFace, liter.Value());
        else
          B.Add(tmpFace, liter.Value().Reversed());
        NbWiresNew++;
      }
      if (!myFwd)
        tmpFace.Orientation(TopAbs_REVERSED);
      faces.Append(tmpFace);
    }
  }

  if (NbWires != NbWiresNew)
    return Standard_False;

  if (faces.Length() > 1)
  {
    TopoCompound Comp;
    B.MakeCompound(Comp);
    for (Standard_Integer i = 1; i <= faces.Length(); i++)
      B.Add(Comp, faces(i));
    myResult = Comp;

    if (!Context().IsNull())
    {
      Context()->Replace(myFace, myResult);
    }

    for (ShapeExplorer exp(myResult, TopAbs_FACE); exp.More(); exp.Next())
    {
      myFace = TopoDS::Face(exp.Current());
      BRepTools1::Update(myFace);
    }
    return Standard_True;
  }

  return Standard_False;
}

//=======================================================================
// function : IsPeriodicConicalLoop
// purpose  : Checks whether the passed wire makes up a periodic loop on
//           passed conical surface
//=======================================================================

static Standard_Boolean IsPeriodicConicalLoop(const Handle(Geom_ConicalSurface)& theSurf,
                                              const TopoWire&                 theWire,
                                              const Standard_Real                theTolerance,
                                              Standard_Real&                     theMinU,
                                              Standard_Real&                     theMaxU,
                                              Standard_Real&                     theMinV,
                                              Standard_Real&                     theMaxV,
                                              Standard_Boolean&                  isUDecrease)
{
  if (theSurf.IsNull())
    return Standard_False;

  Edge1 aSAE;
  TopLoc_Location    aLoc;

  Standard_Real aCumulDeltaU = 0.0, aCumulDeltaUAbs = 0.0;
  Standard_Real aMinU = RealLast();
  Standard_Real aMinV = aMinU;
  Standard_Real aMaxU = -aMinU;
  Standard_Real aMaxV = aMaxU;

  // Iterate over the edges to check whether the wire is periodic on conical surface
  TopoDS_Iterator aWireIter(theWire, Standard_False);
  for (; aWireIter.More(); aWireIter.Next())
  {
    const TopoEdge&   aCurrentEdge = TopoDS::Edge(aWireIter.Value());
    Handle(GeomCurve2d) aC2d;
    Standard_Real        aPFirst, aPLast;

    aSAE.PCurve(aCurrentEdge, theSurf, aLoc, aC2d, aPFirst, aPLast, Standard_True);

    if (aC2d.IsNull())
      return Standard_False;

    gp_Pnt2d aUVFirst = aC2d->Value(aPFirst), aUVLast = aC2d->Value(aPLast);

    Standard_Real aUFirst = aUVFirst.X(), aULast = aUVLast.X();
    Standard_Real aVFirst = aUVFirst.Y(), aVLast = aUVLast.Y();

    Standard_Real aCurMaxU = Max(aUFirst, aULast), aCurMinU = Min(aUFirst, aULast);
    Standard_Real aCurMaxV = Max(aVFirst, aVLast), aCurMinV = Min(aVFirst, aVLast);

    if (aCurMinU < aMinU)
      aMinU = aCurMinU;
    if (aCurMaxU > aMaxU)
      aMaxU = aCurMaxU;
    if (aCurMinV < aMinV)
      aMinV = aCurMinV;
    if (aCurMaxV > aMaxV)
      aMaxV = aCurMaxV;

    Standard_Real aDeltaU = aULast - aUFirst;

    aCumulDeltaU += aDeltaU;
    aCumulDeltaUAbs += Abs(aDeltaU);
  }

  theMinU     = aMinU;
  theMaxU     = aMaxU;
  theMinV     = aMinV;
  theMaxV     = aMaxV;
  isUDecrease = (aCumulDeltaU < 0 ? Standard_True : Standard_False);

  Standard_Boolean is2PIDelta   = Abs(aCumulDeltaUAbs - 2 * M_PI) <= theTolerance;
  Standard_Boolean isAroundApex = Abs(theMaxU - theMinU) > 2 * M_PI - theTolerance;

  return is2PIDelta && isAroundApex;
}

//=================================================================================================

Standard_Boolean ShapeFix_Face::FixPeriodicDegenerated()
{
  /* =====================
   *  Prepare fix routine
   * ===================== */

  if (!Context().IsNull())
  {
    TopoShape aSh = Context()->Apply(myFace);
    myFace           = TopoDS::Face(aSh);
  }

  /* ================================================
   *  Check if fix can be applied on the passed face
   * ================================================ */

  // Collect all wires owned by the face
  TopTools_SequenceOfShape aWireSeq;
  for (TopoDS_Iterator aWireIt(myFace, Standard_False); aWireIt.More(); aWireIt.Next())
  {
    const TopoShape& aSubSh = aWireIt.Value();
    if (aSubSh.ShapeType() != TopAbs_WIRE
        || (aSubSh.Orientation() != TopAbs_FORWARD && aSubSh.Orientation() != TopAbs_REVERSED))
      continue;

    aWireSeq.Append(aWireIt.Value());
  }

  // Get number of wires and surface
  Standard_Integer     aNbWires = aWireSeq.Length();
  Handle(GeomSurface) aSurface = BRepInspector::Surface(myFace);

  // Only single wires on conical surfaces are checked
  if (aNbWires != 1 || aSurface.IsNull()
      || aSurface->DynamicType() != STANDARD_TYPE(Geom_ConicalSurface))
    return Standard_False;

  // Get the single wire
  TopoWire aSoleWire = TopoDS::Wire(aWireSeq.Value(1));

  // Check whether this wire is belting the conical surface by period
  Handle(Geom_ConicalSurface) aConeSurf = Handle(Geom_ConicalSurface)::DownCast(aSurface);
  Standard_Real               aMinLoopU = 0.0, aMaxLoopU = 0.0, aMinLoopV = 0.0, aMaxLoopV = 0.0;
  Standard_Boolean            isUDecrease = Standard_False;

  Standard_Boolean isConicLoop = IsPeriodicConicalLoop(aConeSurf,
                                                       aSoleWire,
                                                       Precision1(),
                                                       aMinLoopU,
                                                       aMaxLoopU,
                                                       aMinLoopV,
                                                       aMaxLoopV,
                                                       isUDecrease);

  if (!isConicLoop)
    return Standard_False;

  /* ===============
   *  Retrieve apex
   * =============== */

  // Get base circle of the conical surface (the circle it was built from)
  Handle(GeomCurve3d)  aConeBaseCrv  = aConeSurf->VIso(0.0);
  Handle(GeomCircle) aConeBaseCirc = Handle(GeomCircle)::DownCast(aConeBaseCrv);

  // Retrieve conical props
  Standard_Real aConeBaseR = aConeBaseCirc->Radius();
  Standard_Real aSemiAngle = aConeSurf->SemiAngle();

  if (fabs(aSemiAngle) <= Precision1::Confusion())
    return Standard_False; // Bad surface

  // Find the V parameter of the apex
  Standard_Real aConeBaseH = aConeBaseR / Sin(aSemiAngle);
  Standard_Real anApexV    = -aConeBaseH;

  // Get apex vertex
  TopoVertex anApex = BRepBuilderAPI_MakeVertex(aConeSurf->Apex());

  // ====================================
  //  Build degenerated edge in the apex
  // ====================================

  TopoEdge  anApexEdge;
  ShapeBuilder aBuilder;
  aBuilder.MakeEdge(anApexEdge);

  // Check if positional relationship between the initial wire and apex
  // line in 2D is going to be consistent
  if (fabs(anApexV - aMinLoopV) <= Precision1() || fabs(anApexV - aMaxLoopV) <= Precision1()
      || (anApexV < aMaxLoopV && anApexV > aMinLoopV))
    return Standard_False;

  Handle(Geom2d_Line) anApexCurve2d;

  // Apex curve below the wire
  if (anApexV < aMinLoopV)
  {
    anApexCurve2d = new Geom2d_Line(gp_Pnt2d(aMinLoopU, anApexV), gp_Dir2d(1, 0));
    if (!isUDecrease)
      aSoleWire.Reverse();
  }

  // Apex curve above the wire
  if (anApexV > aMaxLoopV)
  {
    anApexCurve2d = new Geom2d_Line(gp_Pnt2d(aMaxLoopU, anApexV), gp_Dir2d(-1, 0));
    if (isUDecrease)
      aSoleWire.Reverse();
  }

  // Create degenerated edge & wire for apex
  aBuilder.UpdateEdge(anApexEdge, anApexCurve2d, myFace, Precision1());
  aBuilder.Add(anApexEdge, anApex);
  aBuilder.Add(anApexEdge, anApex.Reversed());
  aBuilder.Degenerated(anApexEdge, Standard_True);
  aBuilder.Range(anApexEdge, 0, fabs(aMaxLoopU - aMinLoopU));
  TopoWire anApexWire = BRepBuilderAPI_MakeWire(anApexEdge);

  // ===============================================================
  //  Finalize the fix building new face and setting up the results
  // ===============================================================

  // Collect the resulting set of wires
  TopTools_SequenceOfShape aNewWireSeq;
  aNewWireSeq.Append(aSoleWire);
  aNewWireSeq.Append(anApexWire);

  // Assemble new face
  TopoFace aNewFace = TopoDS::Face(myFace.EmptyCopied());
  aNewFace.Orientation(TopAbs_FORWARD);
  ShapeBuilder aFaceBuilder;
  for (Standard_Integer i = 1; i <= aNewWireSeq.Length(); i++)
  {
    TopoWire aNewWire = TopoDS::Wire(aNewWireSeq.Value(i));
    aFaceBuilder.Add(aNewFace, aNewWire);
  }
  aNewFace.Orientation(myFace.Orientation());

  // Adjust the resulting state of the healing tool
  myResult = aNewFace;
  Context()->Replace(myFace, myResult);

  return Standard_True;
}
