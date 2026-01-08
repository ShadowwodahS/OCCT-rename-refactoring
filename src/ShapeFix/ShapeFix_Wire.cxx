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

//: j6 abv 7 Dec 98: ProSTEP TR10 r0601_id.stp #57676 & #58586:
//    in FixIntersectingEdges, do not cut edges because of influence on adjacent faces
// pdn 17.12.98: shifting whole wire in FixShifted
//: k3 abv 24 Dec 98: BUC50070 #26682 and #30087: removing self-int loops on pcurves
// pdn 30.12.98: PRO10366 #210: shifting pcurve between two singularities
// pdn 05.01.98: renaming method FixLittle to FixSmall
//: l0 abv 10.01.99: remove unused code
//: n2 abv 22.01.99: ma-test5.igs: IGES read (pref3d): remove degen edge with no pcurve
//: o4 abv 17.02.99: r0301_db.stp #53082: using parameter isClosed in CheckOrder
//: p4 abv 23.02.99: PRO9234 #15720: update UV points of edges after shifting pcurves
// #67 rln 01.03.99 S4135: ims010.igs treatment of Geom_SphericalSurface together with V-closed
// surfaces
//: p7 abv 10.03.99 PRO18206: using method IsDegenerated() to detect singularity in FixLacking
//: r0 abv 19.03.99 PRO7226.stp #489490: remove degenerated edges if several
// #78 rln 12.03.99 S4135: checking spatial closure with Precision1
// #79 rln 15.03.99 S4135: bmarkmdl.igs: check for gap before shifting on singularities
//     pdn 17.03.99 S4135: implemented fixing not adjacent intersection
// #86 rln 22.03.99 S4135: repeat of fix self-intersection if it was fixed just before
//%15 pdn 06.04.99 CTS18546-2: fix of self-intersection is repeated while fixed
// #3  smh 01.04.99 S4163: Overflow
// #4  szv          S4163: optimization
//: r7 abv 12.04.99 S4136: FixLk: extend cases of adding degenerated edge
//: r8 abv 12.04.99 PRO10107.stp #2241: try increasing tolerance of edge for fix IE
//: s2 abv 21.04.99 S4136, PRO7978.igs: FixLacking extended to be able to bend pcurves
// szv#9:S4244:19Aug99 Methods FixGaps3d and FixGaps2d were introduced
// #15 smh 03.04.2000 PRO19800. Checking degenerated point on a surface of revolution.
//  sln 25.09.2001  checking order of 3d and 2d representation curves
//  van 19.10.2001  fix of non-adjacent self-intersection corrected to agree with BRepCheck1
//  skl 07.03.2002 fix for bug OCC180
//  skl 15.05.2002 for OCC208 (if few edges have reference to
//                 one pcurve we make replace pcurve)
//  PTV 26.06.2002  Remove regressions after fix OCC450

#include <ShapeFix_Wire.hxx>

#include <BRep_Builder.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepTools.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2dConvert.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_OffsetCurve.hxx>
#include <Geom_Plane.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAPI.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <IntRes2d_IntersectionPoint.hxx>
#include <IntRes2d_SequenceOfIntersectionPoint.hxx>
#include <Message_Msg.hxx>
#include <Precision.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeAnalysis_TransferParametersProj.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <ShapeAnalysis_WireOrder.hxx>
#include <ShapeBuild_Edge.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeBuild_Vertex.hxx>
#include <ShapeConstruct_ProjectCurveOnSurface.hxx>
#include <ShapeExtend.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_IntersectionTool.hxx>
#include <ShapeFix_SplitTool.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <Standard_Type.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HSequenceOfReal.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopTools_HSequenceOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(WireHealer, ShapeFix_Root)

// S4135
// #######################################################################
//   Constructors, initializations, modes, querying
// #######################################################################
//=================================================================================================

WireHealer::WireHealer()
    : myMaxTailAngleSine(0),
      myMaxTailWidth(-1)
{
  myFixEdge  = new ShapeFix_Edge;
  myAnalyzer = new ShapeAnalysis_Wire;
  ClearModes();
  ClearStatuses();
  myStatusRemovedSegment = Standard_False;
}

//=================================================================================================

WireHealer::WireHealer(const TopoWire&  wire,
                             const TopoFace&  face,
                             const Standard_Real prec)
    : myMaxTailAngleSine(0),
      myMaxTailWidth(-1)
{
  myFixEdge  = new ShapeFix_Edge;
  myAnalyzer = new ShapeAnalysis_Wire;
  ClearModes();
  SetMaxTolerance(prec);
  myStatusRemovedSegment = Standard_False;
  Init(wire, face, prec);
}

//=================================================================================================

void WireHealer::SetPrecision(const Standard_Real prec)
{
  ShapeFix_Root::SetPrecision(prec);
  myAnalyzer->SetPrecision(prec);
}

//=================================================================================================

void WireHealer::SetMaxTailAngle(const Standard_Real theMaxTailAngle)
{
  myMaxTailAngleSine = Sin(theMaxTailAngle);
  myMaxTailAngleSine = (myMaxTailAngleSine >= 0) ? myMaxTailAngleSine : 0;
}

//=================================================================================================

void WireHealer::SetMaxTailWidth(const Standard_Real theMaxTailWidth)
{
  myMaxTailWidth = theMaxTailWidth;
}

//=================================================================================================

void WireHealer::ClearModes()
{
  myTopoMode        = Standard_False;
  myGeomMode        = Standard_True;
  myClosedMode      = Standard_True;
  myPreference2d    = Standard_True;
  myFixGapsByRanges = Standard_False;

  myRemoveLoopMode = -1;

  myFixReversed2dMode      = -1;
  myFixRemovePCurveMode    = -1;
  myFixRemoveCurve3dMode   = -1;
  myFixAddPCurveMode       = -1;
  myFixAddCurve3dMode      = -1;
  myFixSeamMode            = -1;
  myFixShiftedMode         = -1;
  myFixSameParameterMode   = -1;
  myFixVertexToleranceMode = -1;

  myFixNotchedEdgesMode                 = -1;
  myFixSelfIntersectingEdgeMode         = -1;
  myFixIntersectingEdgesMode            = -1;
  myFixNonAdjacentIntersectingEdgesMode = -1;
  myFixTailMode                         = 0;

  myFixReorderMode          = -1;
  myFixSmallMode            = -1;
  myFixConnectedMode        = -1;
  myFixEdgeCurvesMode       = -1;
  myFixDegeneratedMode      = -1;
  myFixSelfIntersectionMode = -1;
  myFixLackingMode          = -1;
  myFixGaps3dMode           = -1;
  myFixGaps2dMode           = -1;
}

//=================================================================================================

void WireHealer::ClearStatuses()
{
  Standard_Integer emptyStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);

  myLastFixStatus = emptyStatus;

  myStatusReorder          = emptyStatus;
  myStatusSmall            = emptyStatus;
  myStatusConnected        = emptyStatus;
  myStatusEdgeCurves       = emptyStatus;
  myStatusDegenerated      = emptyStatus;
  myStatusSelfIntersection = emptyStatus;
  myStatusLacking          = emptyStatus;
  myStatusGaps3d           = emptyStatus; // szv#9:S4244:19Aug99 new method introduced
  myStatusGaps2d           = emptyStatus; // szv#9:S4244:19Aug99 new method introduced
  myStatusClosed           = emptyStatus;
  myStatusNotches          = emptyStatus;
  myStatusFixTails         = emptyStatus;
}

//=================================================================================================

void WireHealer::Init(const TopoWire& wire, const TopoFace& face, const Standard_Real prec)
{
  Load(wire);
  SetFace(face);
  SetPrecision(prec);
}

//=================================================================================================

void WireHealer::Init(const Handle(ShapeAnalysis_Wire)& saw)
{
  ClearStatuses();
  myAnalyzer = saw;
  myShape.Nullify();
  //  SetPrecision ( saw.Precision1() );
}

//=================================================================================================

void WireHealer::Load(const TopoWire& wire)
{
  ClearStatuses();

  TopoWire W = wire;
  if (!Context().IsNull())
  {
    TopoShape S = Context()->Apply(wire);
    W              = TopoDS::Wire(S);
  }

  myAnalyzer->Load(W);
  myShape = wire;
}

//=================================================================================================

void WireHealer::Load(const Handle(ShapeExtend_WireData)& sbwd)
{
  ClearStatuses();
  myAnalyzer->Load(sbwd);
  if (!Context().IsNull())
    UpdateWire();
  myShape.Nullify();
}

//=================================================================================================

Standard_Integer WireHealer::NbEdges() const
{
  Handle(ShapeExtend_WireData) sbwd = myAnalyzer->WireData();
  return sbwd.IsNull() ? 0 : sbwd->NbEdges();
}

// #######################################################################
//   Fixing methods of API level
// #######################################################################

//=======================================================================
// function : Perform
// purpose  : This method performs all the available fixes.
//           If some fix is turned on or off explicitly by the flag,
//           it is called or not depending on that flag
//           Else (i.e. if flag is default) fix is called depending on the
//           situation: default is True, but some fixes are not called or
//           are limited if order of edges in the wire is not OK
//=======================================================================

Standard_Boolean WireHealer::Perform()
{
  ClearStatuses();
  if (!IsLoaded())
    return Standard_False;

  if (!Context().IsNull())
    myFixEdge->SetContext(Context());

  Standard_Boolean Fixed = Standard_False;

  // FixReorder is first, because as a rule wire is required to be ordered
  // We shall analyze the order of edges in the wire and set appropriate
  // status even if FixReorder should not be called (if it is forbidden)

  ShapeAnalysis_WireOrder sawo;
  Standard_Boolean        ReorderOK = (myAnalyzer->CheckOrder(sawo, myClosedMode) == 0);
  if (NeedFix(myFixReorderMode, !ReorderOK))
  {
    if (FixReorder())
      Fixed = Standard_True;
    ReorderOK = !StatusReorder(ShapeExtend_FAIL);
  }

  // FixSmall is allowed to change topology only if mode is set and FixReorder
  // did not failed
  if (NeedFix(myFixSmallMode, myTopoMode))
  {
    if (FixSmall(!myTopoMode || !ReorderOK, MinTolerance()))
    {
      Fixed = Standard_True;
      // retry reorder if necessary (after FixSmall it can work better)
      if (NeedFix(myFixReorderMode, !ReorderOK))
      {
        FixReorder();
        ReorderOK = !StatusReorder(ShapeExtend_FAIL);
      }
    }
  }

  if (NeedFix(myFixConnectedMode, ReorderOK))
  {
    if (FixConnected())
      Fixed = Standard_True;
  }

  if (NeedFix(myFixEdgeCurvesMode))
  {
    Standard_Integer savFixShiftedMode = myFixShiftedMode;
    // turn out FixShifted if reorder not done
    if (myFixShiftedMode == -1 && !ReorderOK)
      myFixShiftedMode = 0;
    if (FixEdgeCurves())
      Fixed = Standard_True;
    myFixShiftedMode = savFixShiftedMode;
  }

  if (NeedFix(myFixDegeneratedMode))
  {
    if (FixDegenerated())
      Fixed = Standard_True; // ?? if ! ReorderOK ??
  }

  // pdn - temporary to test
  if (myFixTailMode <= 0 && NeedFix(myFixNotchedEdgesMode, ReorderOK))
  {
    Fixed |= FixNotchedEdges();
    if (Fixed)
      FixShifted(); // skl 07.03.2002 for OCC180
  }

  if (myFixTailMode != 0)
  {
    if (FixTails())
    {
      Fixed = Standard_True;
      FixShifted();
    }
  }

  if (NeedFix(myFixSelfIntersectionMode, myClosedMode))
  {
    Standard_Integer savFixIntersectingEdgesMode = myFixIntersectingEdgesMode;
    // switch off FixIntEdges if reorder not done
    if (myFixIntersectingEdgesMode == -1 && !ReorderOK)
      myFixIntersectingEdgesMode = 0;
    if (FixSelfIntersection())
      Fixed = Standard_True;
    FixReorder();
    myFixIntersectingEdgesMode = savFixIntersectingEdgesMode;
  }

  if (NeedFix(myFixLackingMode, ReorderOK))
  {
    if (FixLacking())
      Fixed = Standard_True;
  }

  // TEMPORARILY without special mode !!!
  Handle(ShapeExtend_WireData) sbwd = WireData();
  for (Standard_Integer iedge = 1; iedge <= sbwd->NbEdges(); iedge++)
    if (myFixEdge->FixVertexTolerance(sbwd->Edge(iedge), Face()))
    {
      Fixed = Standard_True;
    }

  if (!Context().IsNull())
    UpdateWire();

  return Fixed;
}

//=================================================================================================

Standard_Boolean WireHealer::FixReorder(Standard_Boolean theModeBoth)
{
  myStatusReorder = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsLoaded())
  {
    return Standard_False;
  }

  // fix in Both mode for bi-periodic surface
  ShapeAnalysis_WireOrder sawo;
  if (!myAnalyzer->Surface().IsNull() && myAnalyzer->Surface()->Surface()->IsUPeriodic()
      && myAnalyzer->Surface()->Surface()->IsVPeriodic() && theModeBoth)
  {
    myAnalyzer->CheckOrder(sawo, myClosedMode, Standard_True, Standard_True);
  }
  else
  {
    myAnalyzer->CheckOrder(sawo, myClosedMode, Standard_True, Standard_False);
  }

  FixReorder(sawo);

  if (LastFixStatus(ShapeExtend_FAIL))
  {
    myStatusReorder |= ShapeExtend1::EncodeStatus(
      LastFixStatus(ShapeExtend_FAIL1) ? ShapeExtend_FAIL1 : ShapeExtend_FAIL2);
  }
  if (!LastFixStatus(ShapeExtend_DONE))
  {
    return Standard_False;
  }

  myStatusReorder |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
  if (sawo.Status() == 2 || sawo.Status() == -2)
  {
    myStatusReorder |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
  }
  if (sawo.Status() < 0)
  {
    myStatusReorder |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
  }
  if (sawo.Status() == 3)
  {
    // only shifted
    myStatusReorder |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5);
  }
  return Standard_True;
}

//=================================================================================================

Standard_Integer WireHealer::FixSmall(const Standard_Boolean lockvtx,
                                         const Standard_Real    precsmall)
{
  myStatusSmall = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsLoaded())
    return Standard_False;

  for (Standard_Integer i = NbEdges(); i > 0; i--)
  {
    FixSmall(i, lockvtx, precsmall);
    myStatusSmall |= myLastFixStatus;
  }

  return StatusSmall(ShapeExtend_DONE);
}

//=================================================================================================

Standard_Boolean WireHealer::FixConnected(const Standard_Real prec)
{
  myStatusConnected = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsLoaded())
    return Standard_False;

  Standard_Integer stop = (myClosedMode ? 0 : 1);
  for (Standard_Integer i = NbEdges(); i > stop; i--)
  {
    FixConnected(i, prec);
    myStatusConnected |= myLastFixStatus;
  }

  return StatusConnected(ShapeExtend_DONE);
}

//=================================================================================================

Standard_Boolean WireHealer::FixEdgeCurves()
{
  myStatusEdgeCurves = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsLoaded())
    return Standard_False;
  Standard_Boolean isReady = IsReady();

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             i, nb = sbwd->NbEdges();
  TopoFace                  face          = Face();
  Handle(ShapeFix_Edge)        theAdvFixEdge = myFixEdge;
  if (theAdvFixEdge.IsNull())
    myFixReversed2dMode = Standard_False;

  // fix revesred 2d / 3d curves
  if (isReady && NeedFix(myFixReversed2dMode))
  {
    for (i = 1; i <= nb; i++)
    {
      theAdvFixEdge->FixReversed2d(sbwd->Edge(i), face);
      if (theAdvFixEdge->Status(ShapeExtend_DONE))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
      if (theAdvFixEdge->Status(ShapeExtend_FAIL))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
    }
  }

  // add / remove pcurve
  if (isReady && NeedFix(myFixRemovePCurveMode, Standard_False))
  {
    for (i = 1; i <= nb; i++)
    {
      myFixEdge->FixRemovePCurve(sbwd->Edge(i), face);
      if (myFixEdge->Status(ShapeExtend_DONE))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
      if (myFixEdge->Status(ShapeExtend_FAIL))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
    }
  }

  if (isReady && NeedFix(myFixAddPCurveMode))
  {
    Standard_Integer overdegen = 0; //: c0
    for (i = 1; i <= nb; i++)
    {
      myFixEdge->FixAddPCurve(sbwd->Edge(i),
                              face,
                              sbwd->IsSeam(i),
                              myAnalyzer->Surface(),
                              Precision1());
      if (myFixEdge->Status(ShapeExtend_DONE))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
      if (myFixEdge->Status(ShapeExtend_FAIL))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL3);

      // if ( !sbwd->IsSeam(i) && myFixEdge->Status ( ShapeExtend_DONE2 )
      //      && BRepInspector::SameParameter(sbwd->Edge(i)) ) {
      if (!sbwd->IsSeam(i) && myFixEdge->Status(ShapeExtend_DONE2))
      {
        // abv 24 Feb 00: trj3_s1-ac-214.stp #1631 etc.: try to split the edge in singularity
        if (!Context().IsNull())
        {
          Edge2               sbe;
          TopoEdge                   E = sbwd->Edge(i);
          Curve2           SAC;
          Standard_Real                 a, b;
          Handle(GeomCurve3d)            C   = BRepInspector::Curve(E, a, b);
          Handle(ShapeAnalysis_Surface) S   = myAnalyzer->Surface();
          Standard_Integer              nbs = S->NbSingularities(MinTolerance());
          GeomAdaptor_Curve             GAC(C, a, b);
          TColStd_SequenceOfReal        seq;
          for (Standard_Integer j = 1; j <= nbs; j++)
          {
            Standard_Real    Preci;
            gp_Pnt2d         pd1, pd2;
            Point3d           P3d, pr;
            Standard_Real    par1, par2, split;
            Standard_Boolean tmpUIsoDeg;
            S->Singularity(j, Preci, P3d, pd1, pd2, par1, par2, tmpUIsoDeg);
            if (SAC.Project(GAC, P3d, MinTolerance(), pr, split, Standard_True)
                < Max(Preci, MinTolerance()))
            {
              if (split - a > ::Precision1::PConfusion() && b - split > ::Precision1::PConfusion())
              {
                Standard_Integer k;
                for (k = 1; k <= seq.Length(); k++)
                {
                  if (split < seq(k) - ::Precision1::PConfusion())
                  {
                    seq.InsertBefore(k, split);
                    break;
                  }
                  else if (split < seq(k) + ::Precision1::PConfusion())
                    break;
                }
                if (k > seq.Length())
                  seq.Append(split);
              }
            }
          }
          if (seq.Length() > 0)
          { // supposed that edge is SP
#ifdef OCCT_DEBUG
            std::cout << "Edge going over singularity detected; split" << std::endl;
#endif
            Standard_Boolean isFwd = (E.Orientation() == TopAbs_FORWARD);
            E.Orientation(TopAbs_FORWARD);

            // if( BRepInspector::SameParameter(sbwd->Edge(i)) )
            //   sbe.RemovePCurve ( E, face );

            // 10.04.2003 skl for using trimmed lines as pcurves
            Edge1 sae;
            if (BRepInspector::SameParameter(sbwd->Edge(i)))
              sbe.RemovePCurve(E, face);
            else
            {
              if (sae.HasPCurve(E, face))
              {
                Handle(GeomCurve2d) C2d;
                Standard_Real        fp2d, lp2d;
                if (sae.PCurve(E, face, C2d, fp2d, lp2d))
                {
                  if (!C2d->IsKind(STANDARD_TYPE(Geom2d_TrimmedCurve)))
                    sbe.RemovePCurve(E, face);
                }
              }
            }

            //    myFixEdge->FixSameParameter ( E ); // to ensure SameRange & SP
            ShapeBuilder  B;
            TopoVertex V1, V2, V;
            // Edge1 sae;
            V1 = sae.FirstVertex(E);
            V2 = sae.LastVertex(E);

            Handle(ShapeExtend_WireData) sw = new ShapeExtend_WireData;
            for (Standard_Integer k = 0; k <= seq.Length(); k++)
            {
              Standard_Real split = (k < seq.Length() ? seq(k + 1) : b);
              if (k < seq.Length())
              {
                B.MakeVertex(V, C->Value(split), BRepInspector::Tolerance(E));
                // try increase tolerance before splitting
                Standard_Real aDist = BRepInspector::Pnt(V1).Distance(BRepInspector::Pnt(V));
                if (aDist < BRepInspector::Tolerance(V1) * 1.01)
                {
                  B.UpdateVertex(V1, Max(aDist, BRepInspector::Tolerance(V1)));
                  a  = split;
                  V1 = V;
                  continue;
                }
                else
                {
                  aDist = BRepInspector::Pnt(V2).Distance(BRepInspector::Pnt(V));
                  if (aDist < BRepInspector::Tolerance(V2) * 1.01)
                  {
                    B.UpdateVertex(V, Max(aDist, BRepInspector::Tolerance(V2)));
                    b  = split;
                    V2 = V;
                    continue;
                  }
                }
              }
              else
              {
                V = V2;
              }

              TopoEdge edge = sbe.CopyReplaceVertices(E, V1, V);
              if (BRepInspector::SameParameter(sbwd->Edge(i)))
              {
                // TopoEdge edge = sbe.CopyReplaceVertices ( E, V1, V );
                B.Range(edge, a, split);
                sw->Add(edge);
              }
              else
              {
                // TopoEdge edge = sbe.CopyReplaceVertices(sbwd->Edge(i),V1,V);
                Handle(ShapeAnalysis_TransferParameters) sftp =
                  new ShapeAnalysis_TransferParameters(E, face);
                sftp->TransferRange(edge, a, split, Standard_False);
                sw->Add(edge);
              }
              // sw->Add(edge);
              a  = split;
              V1 = V;
            }
            if (!isFwd)
            {
              sw->Reverse();
              E.Orientation(TopAbs_REVERSED);
            }
            Context()->Replace(E, sw->Wire());
            UpdateWire();
            nb = sbwd->NbEdges();
            i--;
            continue;
          }
        }

        overdegen = i;
      }
    }

    //: c0 abv 20 Feb 98: treat case of curve going over degenerated pole and seam
    if (overdegen && myAnalyzer->Surface()->IsUClosed(Precision1()))
    {
      Edge2 sbe;
      Standard_Real   URange, SUF, SUL, SVF, SVL;
      myAnalyzer->Surface()->Bounds(SUF, SUL, SVF, SVL);
      URange = (Abs(SUL - SUF));
      Coords2d              vec(0, 0);
      Edge1 sae;
      Standard_Integer   k;
      for (k = 1; k <= nb; k++)
      {
        Standard_Real        cf, cl;
        Handle(GeomCurve2d) c2d;
        if (!sae.PCurve(sbwd->Edge(k), face, c2d, cf, cl, Standard_True))
          break;
        vec += c2d->Value(cl).XY() - c2d->Value(cf).XY();
      }
      if (k > nb && Abs(Abs(vec.X()) - URange) < 0.1 * URange)
      {
        sbe.RemovePCurve(sbwd->Edge(overdegen), face);
        myFixEdge->Projector()->AdjustOverDegenMode() = Standard_False;
        myFixEdge->FixAddPCurve(sbwd->Edge(overdegen),
                                face,
                                sbwd->IsSeam(overdegen),
                                myAnalyzer->Surface(),
                                Precision1());
      }
#ifdef OCCT_DEBUG
      std::cout << "Edge going over singularity detected; pcurve adjusted" << std::endl;
#endif
    }
  }

  // add / remove pcurve
  if (isReady && NeedFix(myFixRemoveCurve3dMode, Standard_False))
  {
    for (i = 1; i <= nb; i++)
    {
      myFixEdge->FixRemoveCurve3d(sbwd->Edge(i));
      if (myFixEdge->Status(ShapeExtend_DONE))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE4);
      if (myFixEdge->Status(ShapeExtend_FAIL))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL4);
    }
  }
  if (NeedFix(myFixAddCurve3dMode))
  {
    for (i = 1; i <= nb; i++)
    {
      myFixEdge->FixAddCurve3d(sbwd->Edge(i));
      if (myFixEdge->Status(ShapeExtend_DONE))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5);
      if (myFixEdge->Status(ShapeExtend_FAIL))
      {
        //: abv 29.08.01: Spatial_firex_lofting.sat: if 3d curve cannot
        // be built because edge has no pcurves either, remove that edge
        Handle(GeomCurve2d) C;
        Handle(GeomSurface) S;
        TopLoc_Location      L;
        Standard_Real        first = 0., last = 0.;
        BRepInspector::CurveOnSurface(sbwd->Edge(i), C, S, L, first, last);
        if (C.IsNull() || Abs(last - first) < Precision1::PConfusion())
        {
          // clang-format off
          SendWarning ( sbwd->Edge ( i ), Message_Msg ( "FixWire.FixCurve3d.Removed" ) );// Incomplete edge (with no pcurves or 3d curve) removed
          // clang-format on
          sbwd->Remove(i--);
          nb--;
          myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5);
          if (i == nb)
          {
            FixClosed(Precision1());
          }
          else
          {
            FixConnected(i + 1, Precision1());
          }
        }
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL5);
      }
    }
  }

  // fix seam
  if (isReady && NeedFix(myFixSeamMode, Standard_False))
  {
    for (i = 1; i <= nb; i++)
    {
      FixSeam(i);
      if (LastFixStatus(ShapeExtend_DONE))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE6);
      if (LastFixStatus(ShapeExtend_FAIL))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL6);
    }
  }

  // fix shifted
  if (isReady && NeedFix(myFixShiftedMode))
  {
    FixShifted();
    if (LastFixStatus(ShapeExtend_DONE))
      myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE7);
    if (LastFixStatus(ShapeExtend_FAIL))
      myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL7);
  }

  // fix same parameter
  if (isReady && NeedFix(myFixSameParameterMode))
  {
    for (i = 1; i <= nb; i++)
    {
      // skl 28.10.2004 for OCC6366 - check SameRange
      Edge1 sae;
      Standard_Real      First, Last;
      Handle(GeomCurve3d) tmpc3d = BRepInspector::Curve(sbwd->Edge(i), First, Last);
      if (sae.HasPCurve(sbwd->Edge(i), face))
      {
        Handle(GeomCurve2d) C2d;
        Standard_Real        fp2d, lp2d;
        if (sae.PCurve(sbwd->Edge(i), face, C2d, fp2d, lp2d, Standard_False))
        {
          if (fabs(First - fp2d) > Precision1::PConfusion()
              || fabs(Last - lp2d) > Precision1::PConfusion())
          {
            ShapeBuilder B;
            B.SameRange(sbwd->Edge(i), Standard_False);
          }
          else if (!sae.CheckPCurveRange(First, Last, C2d))
          {
            // Replace pcurve
            TopLoc_Location             L;
            const Handle(GeomSurface)& S = BRepInspector::Surface(face, L);
            Edge2().RemovePCurve(sbwd->Edge(i), S, L);
            myFixEdge->FixAddPCurve(sbwd->Edge(i),
                                    face,
                                    sbwd->IsSeam(i),
                                    myAnalyzer->Surface(),
                                    Precision1());
            if (myFixEdge->Status(ShapeExtend_DONE))
              myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
            if (myFixEdge->Status(ShapeExtend_FAIL))
              myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL3);
          }
        }
      }
      myFixEdge->FixSameParameter(sbwd->Edge(i), Face());
      if (myFixEdge->Status(ShapeExtend_DONE))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE8);
      if (myFixEdge->Status(ShapeExtend_FAIL))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL8);
    }
  }

  //: abv 10.06.02: porting C40 -> dev (CC670-12608.stp): moved from Perform()
  // Update with face is needed for plane surfaces (w/o stored pcurves)
  if (NeedFix(myFixVertexToleranceMode))
  {
    for (i = 1; i <= nb; i++)
    {
      myFixEdge->FixVertexTolerance(sbwd->Edge(i), Face());
      if (myFixEdge->Status(ShapeExtend_DONE))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE8);
      if (myFixEdge->Status(ShapeExtend_FAIL))
        myStatusEdgeCurves |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL8);
    }
    if (!Context().IsNull())
      UpdateWire();
  }

  return StatusEdgeCurves(ShapeExtend_DONE);
}

//=================================================================================================

Standard_Boolean WireHealer::FixDegenerated()
{
  myStatusDegenerated = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  //  if ( ! myAnalyzer->Surface()->HasSingularities ( Precision1() ) ) return;

  Standard_Integer lastcoded = -1, prevcoded = 0;
  Standard_Integer stop = (myClosedMode ? 0 : 1);
  for (Standard_Integer i = NbEdges(); i > stop; i--)
  {
    FixDegenerated(i);
    myStatusDegenerated |= myLastFixStatus;
    //: r0 abv 19 Mar 99: PRO7226.stp #489490: remove duplicated degenerated edges
    Standard_Integer coded = (LastFixStatus(ShapeExtend_DONE2) ? 1 : 0);
    if (lastcoded == -1)
      lastcoded = coded;
    if (coded && (prevcoded || (i == 1 && lastcoded)) && NbEdges() > 1)
    {
      Handle(ShapeExtend_WireData) sbwd = WireData();
      ShapeBuilder                 B;
      sbwd->Remove(i);
      if (!prevcoded)
        i = NbEdges();
      B.Degenerated(sbwd->Edge(i++), Standard_False);
      prevcoded = 0;
    }
    else
      prevcoded = coded;
  }

  return StatusDegenerated(ShapeExtend_DONE);
}

//=======================================================================
// function : FixSelfIntersection
// purpose  : Applies FixSelfIntersectingEdge and FixIntersectingEdges
//           and removes wrong edges
//=======================================================================

Standard_Boolean WireHealer::FixSelfIntersection()
{
  myStatusSelfIntersection = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             nb   = sbwd->NbEdges();

  if (NeedFix(myFixSelfIntersectingEdgeMode))
  {
    if (myRemoveLoopMode < 1)
      for (Standard_Integer num = 1; num <= nb; num++)
      {
        FixSelfIntersectingEdge(num);
        myStatusSelfIntersection |= myLastFixStatus;
      }
    else if (myRemoveLoopMode == 1)
    {
      for (Standard_Integer num = 1; num <= nb; num++)
      {
        FixSelfIntersectingEdge(num);
        myStatusSelfIntersection |= myLastFixStatus;
        if (nb < sbwd->NbEdges())
          num--;
        nb = sbwd->NbEdges();
      }
      FixClosed(Precision1());
    }
  }

  if (NeedFix(myFixIntersectingEdgesMode))
  {
    Standard_Integer num = (myClosedMode ? 1 : 2);
    for (; nb > 1 && num <= nb; num++)
    {
      FixIntersectingEdges(num);
      if (LastFixStatus(ShapeExtend_FAIL1))
        myStatusSelfIntersection |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
      if (LastFixStatus(ShapeExtend_FAIL2))
        myStatusSelfIntersection |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
      if (!LastFixStatus(ShapeExtend_DONE))
        continue;

      if (LastFixStatus(ShapeExtend_DONE1))
        myStatusSelfIntersection |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
      if (LastFixStatus(ShapeExtend_DONE2))
        myStatusSelfIntersection |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
      if (LastFixStatus(ShapeExtend_DONE6))
        myStatusSelfIntersection |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE6);

      if (/*! myTopoMode ||*/ nb < 3)
      {
        // #86 rln 22.03.99 sim2.igs, entity 4292: After fixing of self-intersecting
        // BRepCheck1 finds one more self-intersection not found by ShapeAnalysis1
        //%15 pdn 06.04.99 repeat until fixed CTS18546-2 entity 777

        // if the tolerance was modified we should recheck the result, if it was enough
        if (LastFixStatus(ShapeExtend_DONE7)) // num--;
          FixIntersectingEdges(num);
        continue;
      }

      if (LastFixStatus(ShapeExtend_DONE4))
        sbwd->Remove(num);
      if (LastFixStatus(ShapeExtend_DONE3))
        sbwd->Remove(num > 1 ? num - 1 : nb + num - 1);
      if (LastFixStatus(ShapeExtend_DONE4) || LastFixStatus(ShapeExtend_DONE3))
      {
        myStatusSelfIntersection |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
        num = (myClosedMode ? 1 : 2);
        nb  = sbwd->NbEdges();
#ifdef OCCT_DEBUG
        std::cout << "Warning: WireHealer::FixSelfIntersection: Edge removed" << std::endl;
#endif
      }
      else
      {
        // #86 rln 22.03.99
        //%15 pdn 06.04.99 repeat until fixed CTS18546-2 entity 777
        FixIntersectingEdges(num);
        /*if ( LastFixStatus ( ShapeExtend_DONE7 ) )*/
        // Always revisit the fixed edge
        // num--;
      }
    }
    if (!Context().IsNull())
      UpdateWire();
  }

  // pdn 17.03.99 S4135 to avoid regression fixing not adjacent intersection
  if (NeedFix(myFixNonAdjacentIntersectingEdgesMode))
  {

    ShapeFix_IntersectionTool ITool(Context(), Precision1());
    Standard_Integer          NbSplit = 0, NbCut = 0, NbRemoved = 0;
    if (ITool.FixSelfIntersectWire(sbwd, myAnalyzer->Face(), NbSplit, NbCut, NbRemoved))
    {
      myStatusSelfIntersection |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5); // gka 06.09.04
    }
    if (NbSplit > 0 || NbRemoved > 0)
    {
      if (NbRemoved > 0)
        myStatusRemovedSegment = Standard_True;
      // Load(sbwd); commented by skl 29.12.2004 for OCC7624, instead this
      //             string inserted following three strings:
      myAnalyzer->Load(sbwd);
      if (!Context().IsNull())
        UpdateWire();
      myShape.Nullify();
    }
#ifdef OCCT_DEBUG
    if (StatusSelfIntersection(ShapeExtend_DONE5))
      std::cout
        << "Warning: WireHealer::FixIntersection: Non-adjacent intersection fixed (split-"
        << NbSplit << ", cut-" << NbCut << ", removed-" << NbRemoved << ")" << std::endl;
#endif

    /*
        Bnd_Array1OfBox2d boxes(1,nb);
        TopLoc_Location L;
        const Handle(GeomSurface)& S = BRepInspector::Surface(Face(), L);
        Handle(GeomCurve2d) c2d;
        Standard_Real cf,cl;
        Edge1 sae;
        for(Standard_Integer i = 1; i <= nb; i++){
          TopoEdge E = sbwd->Edge (i);
          if(sae.PCurve (E,S,L,c2d,cf,cl,Standard_False)) {
        Bnd_Box2d box;
            Geom2dAdaptor_Curve gac;
            Standard_Real aFirst = c2d->FirstParameter();
            Standard_Real aLast = c2d->LastParameter();
            if(c2d->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve))
               && (cf < aFirst || cl > aLast)) {
              //pdn avoiding problems with segment in Box2
              gac.Load(c2d);
            }
            else
              gac.Load(c2d,cf,cl);
        Add2dCurve::Add(gac,::Precision1::Confusion(),box);
        boxes(i) = box;
          }
        }

        Standard_Boolean isFail = Standard_False, isDone = Standard_False;
        for(Standard_Integer num1 = 1; num1 < nb-1; num1++) {
          Standard_Integer fin = (num1 == 1 ? nb-1 : nb);
          for(Standard_Integer num2 = num1+2; num2 <= fin; num2++)
        if(!boxes(num1).IsOut(boxes(num2))){
          FixIntersectingEdges (num1, num2);
          isFail |= LastFixStatus ( ShapeExtend_FAIL1 );
          isDone |= LastFixStatus ( ShapeExtend_DONE1 );
        }
        }

        if(isFail)
          myStatusSelfIntersection |= ShapeExtend1::EncodeStatus ( ShapeExtend_FAIL3 );
        if(isDone)
          myStatusSelfIntersection |= ShapeExtend1::EncodeStatus ( ShapeExtend_DONE5 );
    #ifdef OCCT_DEBUG
        if (StatusSelfIntersection (ShapeExtend_DONE5))
          std::cout << "Warning: WireHealer::FixSelfIntersection: Non adjacent intersection
    fixed" << std::endl; #endif
    */
  }

  return StatusSelfIntersection(ShapeExtend_DONE);
}

//=================================================================================================

Standard_Boolean WireHealer::FixLacking(const Standard_Boolean force)
{
  myStatusLacking = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  Standard_Integer start = (myClosedMode ? 1 : 2);
  for (Standard_Integer i = start; i <= NbEdges(); i++)
  {
    FixLacking(i, force);
    myStatusLacking |= myLastFixStatus;
  }

  return StatusLacking(ShapeExtend_DONE);
}

//=================================================================================================

Standard_Boolean WireHealer::FixClosed(const Standard_Real prec)
{
  myStatusClosed = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsLoaded() || NbEdges() < 1)
    return Standard_False;

  FixConnected(1, prec);
  if (LastFixStatus(ShapeExtend_DONE))
    myStatusClosed |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
  if (LastFixStatus(ShapeExtend_FAIL))
    myStatusClosed |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);

  FixDegenerated(1);
  if (LastFixStatus(ShapeExtend_DONE))
    myStatusClosed |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
  if (LastFixStatus(ShapeExtend_FAIL))
    myStatusClosed |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);

  FixLacking(1);
  if (LastFixStatus(ShapeExtend_DONE))
    myStatusClosed |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
  if (LastFixStatus(ShapeExtend_FAIL))
    myStatusClosed |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL3);

  return StatusClosed(ShapeExtend_DONE);
}

// #######################################################################
//   Fixing methods of advanced level
// #######################################################################

//=================================================================================================

Standard_Boolean WireHealer::FixReorder(const ShapeAnalysis_WireOrder& wi)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsLoaded())
    return Standard_False;

  Standard_Integer status = wi.Status();
  if (status == 0)
    return Standard_False;
  if (status <= -10)
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
    return Standard_False;
  }

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             i, nb = sbwd->NbEdges();
  if (nb != wi.NbEdges())
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
    return Standard_False;
  }
  // D abord on protege
  for (i = 1; i <= nb; i++)
  {
    if (wi.Ordered(i) == 0)
    {
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL3);
      return Standard_False;
    }
  }

  Handle(TopTools_HSequenceOfShape) newedges = new TopTools_HSequenceOfShape();
  for (i = 1; i <= nb; i++)
    newedges->Append(sbwd->Edge(wi.Ordered(i)));
  for (i = 1; i <= nb; i++)
    sbwd->Set(TopoDS::Edge(newedges->Value(i)), i);

  myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean WireHealer::FixSmall(const Standard_Integer num,
                                         const Standard_Boolean lockvtx,
                                         const Standard_Real    precsmall)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsLoaded() || NbEdges() <= 1)
    return Standard_False;

  // analysis:
  Handle(ShapeAnalysis_Wire) theAdvAnalyzer = myAnalyzer;
  if (theAdvAnalyzer.IsNull())
    return Standard_False;
  Standard_Integer n = (num > 0 ? num : NbEdges());
  theAdvAnalyzer->CheckSmall(n, precsmall);
  if (theAdvAnalyzer->LastCheckStatus(ShapeExtend_FAIL))
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
    //: n2    return Standard_False;
  }

  if (!theAdvAnalyzer->LastCheckStatus(ShapeExtend_DONE))
    return Standard_False;

  // OUI cette edge est NULLE

  if (theAdvAnalyzer->LastCheckStatus(ShapeExtend_DONE2))
  {
    // edge is small, but vertices are not the same..
    if (lockvtx || !myTopoMode)
    {
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
      return Standard_False;
    }
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
  }
  else
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);

  // action: remove edge
  if (!Context().IsNull())
    Context()->Remove(WireData()->Edge(n));
  // clang-format off
  SendWarning ( WireData()->Edge ( n ), Message_Msg ( "FixAdvWire.FixSmall.MSG0" ) ); //Small edge(s) removed
  // clang-format on
  WireData()->Remove(n);

  // call FixConnected in the case if vertices of the small edge were not the same
  if (LastFixStatus(ShapeExtend_DONE2))
  {
    Standard_Integer savLastFixStatus = myLastFixStatus;
    // #43 rln 20.11.98 S4054 CTS18544 entity 21734 removing last edge
    FixConnected(n <= NbEdges() ? n : 1, precsmall);
    if (LastFixStatus(ShapeExtend_FAIL))
      savLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL3);
    myLastFixStatus = savLastFixStatus;
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean WireHealer::FixConnected(const Standard_Integer num, const Standard_Real prec)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsLoaded() || NbEdges() <= 0)
    return Standard_False;

  // analysis

  myAnalyzer->CheckConnected(num, prec < 0 ? MaxTolerance() : prec);
  if (myAnalyzer->LastCheckStatus(ShapeExtend_FAIL))
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
  }
  if (!myAnalyzer->LastCheckStatus(ShapeExtend_DONE))
    return Standard_False;

  // action: replacing vertex

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             n2   = (num > 0 ? num : sbwd->NbEdges());
  Standard_Integer             n1   = (n2 > 1 ? n2 - 1 : sbwd->NbEdges());
  TopoEdge                  E1   = sbwd->Edge(n1);
  TopoEdge                  E2   = sbwd->Edge(n2);

  Edge1 sae;
  TopoVertex      V1 = sae.LastVertex(E1);
  TopoVertex      V2 = sae.FirstVertex(E2);
  TopoVertex      V;

  if (myAnalyzer->LastCheckStatus(ShapeExtend_DONE1))
  { // absolutely confused
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
    // #40 rln 18.11.98 S4054 BUC60035 entity 2393 (2-nd sub-curve is edge with the same vertex)
    if (V2.IsSame(sae.LastVertex(E2)))
    {
      V = V2;
      if (!Context().IsNull())
        Context()->Replace(V1, V.Oriented(V1.Orientation()));
    }
    else
    {
      V = V1;
      if (!Context().IsNull())
        Context()->Replace(V2, V.Oriented(V2.Orientation()));
    }
  }
  else
  { // on moyenne ...
    if (myAnalyzer->LastCheckStatus(ShapeExtend_DONE2))
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
    else
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
    Vertex2 sbv;
    V = sbv.CombineVertex(V1, V2, 1.0001);
    if (!Context().IsNull())
    {
      Context()->Replace(V1, V.Oriented(V1.Orientation()));
      Context()->Replace(V2, V.Oriented(V2.Orientation()));
    }
  }

  // replace vertices to a new one
  Edge2 sbe;
  if (sbwd->NbEdges() < 2)
  {
    if (E2.Free() && myTopoMode)
    {
      ShapeBuilder B;
      B.Remove(E2, sae.FirstVertex(E2));
      B.Remove(E2, sae.LastVertex(E2));
      B.Add(E2, V.Oriented(TopAbs_FORWARD));
      B.Add(E2, V.Oriented(TopAbs_REVERSED));
    }
    else
    {
      TopoEdge tmpE = sbe.CopyReplaceVertices(E2, V, V);
      sbwd->Set(tmpE, n2);
      if (!Context().IsNull())
        Context()->Replace(E2, tmpE);
    }
  }
  else
  {
    if (E2.Free() && E1.Free() && myTopoMode)
    {
      ShapeBuilder B;
      B.Remove(E2, sae.FirstVertex(E2));
      B.Add(E2, V.Oriented(TopAbs_FORWARD));
      if (!myAnalyzer->LastCheckStatus(ShapeExtend_DONE1)
          || sae.FirstVertex(E2).IsSame(sae.LastVertex(E2)))
      {
        B.Remove(E1, sae.LastVertex(E1));
        B.Add(E1, V.Oriented(TopAbs_REVERSED));
      }
    }
    else
    {
      TopoEdge tmpE2 = sbe.CopyReplaceVertices(E2, V, TopoVertex());
      sbwd->Set(tmpE2, n2);
      if (!Context().IsNull())
        Context()->Replace(E2, tmpE2);
      if (!myAnalyzer->LastCheckStatus(ShapeExtend_DONE1)
          || sae.FirstVertex(E2).IsSame(sae.LastVertex(E2)))
      {
        TopoEdge tmpE1 = sbe.CopyReplaceVertices(E1, TopoVertex(), V);
        sbwd->Set(tmpE1, n1);
        if (!Context().IsNull())
          Context()->Replace(E1, tmpE1);
      }
    }
  }
  if (!Context().IsNull())
    UpdateWire();

  return Standard_True;
}

//=================================================================================================

Standard_Boolean WireHealer::FixSeam(const Standard_Integer num)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  Handle(GeomCurve2d) C1, C2;
  Standard_Real        cf, cl;
  if (!myAnalyzer->CheckSeam(num, C1, C2, cf, cl))
    return Standard_False;

  ShapeBuilder B;
  TopoEdge  E = WireData()->Edge(num > 0 ? num : NbEdges());
  B.UpdateEdge(E, C2, C1, Face(), 0.); //: S4136: BRepInspector::Tolerance(E)
  B.Range(E, Face(), cf, cl);
  myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);

  return Standard_True;
}

//=======================================================================
// function : FixShifted
// purpose  : fix parametric curves which may be shifted
// to whole parametric range of closed surface as result of recomputing
// from 3d representation.
// This can be a curve on a seam or near it.
// This function is to be called before FixDegenerated.
// LIMITATION: this function cannot fix cases when, e.g., closed wire is
// composed of two meridians of the sphere and one of them is seam.
// NOTE: wire is supposed to be closed and sorted !
//=======================================================================

//: p4 abv 23.02.99: PRO9234 #15720: update UV points of edge
static void UpdateEdgeUVPoints(TopoEdge& E, const TopoFace& F)
{
  Standard_Real first, last;
  BRepInspector::Range(E, F, first, last);
  ShapeBuilder().Range(E, F, first, last);
}

//=================================================================================================

Standard_Boolean WireHealer::FixShifted()
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  Handle(ShapeAnalysis_Surface) surf = myAnalyzer->Surface();
  // #78 rln 12.03.99 S4135: checking spatial closure with Precision1
  Standard_Boolean uclosed = surf->IsUClosed(Precision1());
  Standard_Boolean vclosed =
    surf->IsVClosed(Precision1()) || surf->Surface()->IsKind(STANDARD_TYPE(Geom_SphericalSurface));
  // #67 rln 01.03.99 S4135: ims010.igs entity D11900 (2D contour is 2*PI higher than V range
  // [-pi/2,p/2])

  // PTV 26.06.2002 begin
  // CTS18546-2.igs entity 2222: base curve is periodic and 2dcurve is shifted
  Standard_Boolean IsVCrvClosed = Standard_False;
  Standard_Real    VRange       = 1.;
  if (surf->Surface()->IsKind(STANDARD_TYPE(Geom_SurfaceOfRevolution)))
  {
    Handle(Geom_SurfaceOfRevolution) aSurOfRev =
      Handle(Geom_SurfaceOfRevolution)::DownCast(surf->Surface());
    Handle(GeomCurve3d) aBaseCrv = aSurOfRev->BasisCurve();
    while ((aBaseCrv->IsKind(STANDARD_TYPE(Geom_OffsetCurve)))
           || (aBaseCrv->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))))
    {
      if (aBaseCrv->IsKind(STANDARD_TYPE(Geom_OffsetCurve)))
        aBaseCrv = Handle(Geom_OffsetCurve)::DownCast(aBaseCrv)->BasisCurve();
      if (aBaseCrv->IsKind(STANDARD_TYPE(Geom_TrimmedCurve)))
        aBaseCrv = Handle(Geom_TrimmedCurve)::DownCast(aBaseCrv)->BasisCurve();
    }
    if (aBaseCrv->IsPeriodic())
    {
      vclosed      = Standard_True;
      VRange       = aBaseCrv->Period();
      IsVCrvClosed = Standard_True;
#ifdef OCCT_DEBUG
      std::cout << "Warning: WireHealer::FixShifted set vclosed True for Surface of Revolution"
                << std::endl;
#endif
    }
  }
  // PTV 26.06.2002 end
  if (!uclosed && !vclosed)
    return Standard_False;

  Standard_Real URange, /*VRange,*/ SUF, SUL, SVF, SVL;
  surf->Surface()->Bounds(SUF, SUL, SVF, SVL);
  Standard_Real SUMid, SVMid;
  SUMid = 0.5 * (SUF + SUL);
  SVMid = 0.5 * (SVF + SVL);
  if (uclosed)
    URange = Abs(SUL - SUF);
  else
    URange = RealLast();
  if (!IsVCrvClosed)
  {
    if (vclosed)
      VRange = Abs(SVL - SVF);
    else
      VRange = RealLast();
  }
  Standard_Real UTol = 0.2 * URange, VTol = 0.2 * VRange;

  Handle(ShapeExtend_WireData) sbwdOring = WireData();
  Edge1           sae;
  Handle(ShapeExtend_WireData) sbwd = new ShapeExtend_WireData;
  for (Standard_Integer i = 1; i <= sbwdOring->NbEdges(); i++)
  {
    TopoEdge E1 = sbwdOring->Edge(i);
    if (BRepInspector::Degenerated(E1) && !sae.HasPCurve(E1, Face()))
      continue;

    sbwd->Add(E1);
  }

  Edge2  sbe;
  Standard_Integer nb  = sbwd->NbEdges();
  Standard_Boolean end = (nb == 0), degstop = Standard_False;
  Standard_Integer stop  = nb;
  Standard_Integer degn2 = 0;
  Point3d           pdeg;
  // pdn 17.12.98 r0901_ec 38237 to shift wire at 0

  // GeomAdaptor_Surface& GAS = myAnalyzer->Surface()->Adaptor3d()->ChangeSurface(); //SK
  Bnd_Box2d box;
  for (Standard_Integer n2 = 1, n1 = nb; !end; n1 = n2++)
  {
    if (n2 > nb)
      n2 = 1;
    if (n2 == stop)
      end = Standard_True;

    TopoEdge E1 = sbwd->Edge(n1);
    TopoEdge E2 = sbwd->Edge(n2);

    if (BRepInspector::Degenerated(E1) || BRepInspector::Degenerated(E2))
    {
      if (!degstop)
      {
        stop    = n2;
        degstop = Standard_True;
      }
      continue;
    }

    TopoVertex V = sae.FirstVertex(E2);
    if (V.IsNull())
      continue;

    Point3d p = BRepInspector::Pnt(V);

    Standard_Real        a1 = 0., b1 = 0., a2 = 0., b2 = 0.;
    Handle(GeomCurve2d) c2d1, c2d2;

    //: abv 29.08.01: torCuts.sat: distinguish degeneration by U and by V;
    // only corresponding move is prohibited
    //    Standard_Boolean isDeg = surf->IsDegenerated ( p, Max ( Precision1(),
    //    BRepInspector::Tolerance(V) ) );
    Standard_Integer isDeg = 0;
    gp_Pnt2d         degP1, degP2;
    Standard_Real    degT1, degT2;
    if (surf->DegeneratedValues(p,
                                Max(Precision1(), BRepInspector::Tolerance(V)),
                                degP1,
                                degP2,
                                degT1,
                                degT2))
      isDeg = (Abs(degP1.X() - degP2.X()) > Abs(degP1.Y() - degP2.Y()) ? 1 : 2);

    // abv 23 Feb 00: UKI60107-6 210: additional check for near-degenerated case
    // smh#15 PRO19800. Check if the surface is surface of revolution.
    if (surf->Surface()->IsKind(STANDARD_TYPE(Geom_SurfaceOfRevolution)))
    {
      if (!isDeg && !vclosed)
      {
        if (c2d1.IsNull() && !sae.PCurve(E1, Face(), c2d1, a1, b1, Standard_True))
          continue;
        gp_Pnt2d p1(SUF, c2d1->Value(b1).Y());
        gp_Pnt2d p2(SUL, c2d1->Value(b1).Y());
        if (surf->IsDegenerated(p1, p2, MaxTolerance(), 10)
            && !surf->IsDegenerated(c2d1->Value(a1),
                                    c2d1->Value(b1),
                                    MaxTolerance(),
                                    10)) // abv 31.07.00: trj4_pm1-ec-214.stp #31274: still allow
                                         // work if edge already exists
          isDeg = 1;
      }
      if (!isDeg && !uclosed)
      {
        if (c2d1.IsNull() && !sae.PCurve(E1, Face(), c2d1, a1, b1, Standard_True))
          continue;
        gp_Pnt2d p1(c2d1->Value(b1).X(), SVF);
        gp_Pnt2d p2(c2d1->Value(b1).X(), SVL);
        if (surf->IsDegenerated(p1, p2, MaxTolerance(), 10)
            && !surf->IsDegenerated(c2d1->Value(a1),
                                    c2d1->Value(b1),
                                    MaxTolerance(),
                                    10)) // abv 31.07.00: trj4_pm1-ec-214.stp #31274: still allow
                                         // work if edge already exists
          isDeg = 2;
      }
    }

    if (isDeg)
    {
      if (!degstop)
      {
        stop    = n2;
        degstop = Standard_True;
      }

      if (!degn2)
      {
        degn2 = n2;
        pdeg  = p;
      }
      else
      {
        if (pdeg.SquareDistance(p) < Precision1() * Precision1())
        {
          degn2 = n2;
          // if ( stop < n2 ) { stop = n2; degstop = Standard_True; }
        }
        else
        {
          Standard_Real        ax1 = 0., bx1 = 0., ax2 = 0., bx2 = 0.;
          Handle(GeomCurve2d) cx1, cx2;
          if ((c2d1.IsNull() && !sae.PCurve(E1, Face(), c2d1, a1, b1, Standard_True))
              || (c2d2.IsNull() && !sae.PCurve(E2, Face(), c2d2, a2, b2, Standard_True))
              || !sae.PCurve(sbwd->Edge(degn2 > 1 ? degn2 - 1 : nb),
                             Face(),
                             cx1,
                             ax1,
                             bx1,
                             Standard_True)
              || !sae.PCurve(sbwd->Edge(degn2), Face(), cx2, ax2, bx2, Standard_True))
          {
            myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
            continue;
          }
          gp_Pnt2d      pd1 = cx1->Value(bx1);
          gp_Pnt2d      pd2 = cx2->Value(ax2);
          gp_Pnt2d      pn1 = c2d1->Value(b1);
          gp_Pnt2d      pn2 = c2d2->Value(a2);
          gp_Vec2d      x(0., 0.); // shift vector
          Standard_Real period;
          if (uclosed)
          {
            x.SetX(1.);
            period = URange;
          }
          else
          {
            x.SetY(1.);
            period = VRange;
          }
          Standard_Real rot1 = (pn1.XY() - pd2.XY()) ^ x.XY();
          Standard_Real rot2 = (pd1.XY() - pn2.XY()) ^ x.XY();
          Standard_Real scld = (pd2.XY() - pd1.XY()) * x.XY();
          Standard_Real scln = (pn2.XY() - pn1.XY()) * x.XY();
          if (rot1 * rot2 < -::Precision1::PConfusion() && scld * scln < -::Precision1::PConfusion()
              && Abs(scln) > 0.1 * period && Abs(scld) > 0.1 * period
              && rot1 * scld > ::Precision1::PConfusion() && rot2 * scln > ::Precision1::PConfusion())
          {
            // abv 02 Mar 00: trying more sophisticated analysis (ie_exhaust-A.stp #37520)
            Standard_Real sign = (rot2 > 0 ? 1. : -1.);
            Standard_Real deep1 =
              Min(sign * (pn2.XY() * x.XY()),
                  Min(sign * (pd1.XY() * x.XY()),
                      Min(sign * (c2d2->Value(b2).XY() * x.XY()),
                          Min(sign * (cx1->Value(ax1).XY() * x.XY()),
                              Min(sign * (c2d2->Value(0.5 * (a2 + b2)).XY() * x.XY()),
                                  sign * (cx1->Value(0.5 * (ax1 + bx1)).XY() * x.XY()))))));
            Standard_Real deep2 =
              Max(sign * (pn1.XY() * x.XY()),
                  Max(sign * (pd2.XY() * x.XY()),
                      Max(sign * (c2d1->Value(a1).XY() * x.XY()),
                          Max(sign * (cx2->Value(bx2).XY() * x.XY()),
                              Max(sign * (c2d1->Value(0.5 * (a1 + b1)).XY() * x.XY()),
                                  sign * (cx2->Value(0.5 * (ax2 + bx2)).XY() * x.XY()))))));
            Standard_Real deep = deep2 - deep1; // estimated current size of wire by x
            // pdn 30 Oct 00: trying correct period [0,period] (trj5_k1-tc-203.stp #4698)
            Standard_Real dx = ShapeAnalysis1::AdjustToPeriod(deep,
                                                             ::Precision1::PConfusion(),
                                                             period + ::Precision1::PConfusion());
            x *= (scld > 0 ? -dx : dx);
            // x *= ( Abs(scld-scln) > 1.5 * period ? 2. : 1. ) *
            //      ( scld >0 ? -period : period );
            Transform2d Shift;
            Shift.SetTranslation(x);
            for (Standard_Integer k = degn2;; k++)
            {
              if (k > nb)
                k = 1;
              if (k == n2)
                break;
              TopoEdge edge = sbwd->Edge(k);
              if (!sae.PCurve(edge, Face(), cx1, ax1, bx1, Standard_True))
                continue;
              // cx1->Transform ( Shift );
              //  skl 15.05.2002 for OCC208 (if few edges have reference to one pcurve)
              Handle(GeomCurve2d) cx1new = Handle(GeomCurve2d)::DownCast(cx1->Transformed(Shift));
              sbe.ReplacePCurve(edge, cx1new, Face());
              UpdateEdgeUVPoints(edge, Face());
            }
            myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
#ifdef OCCT_DEBUG
            std::cout << "Info: WireHealer::FixShifted(): bi - meridian case fixed" << std::endl;
#endif
            continue;
          }
          // degn2 = n2; pdeg = p; // ie_exhaust-A.stp #37520
        }
      }
      /*
            // pdn to fix half sphere
            TopoVertex VE = sae.LastVertex ( E2 );
            Point3d pe = BRepInspector::Pnt ( VE );
            //pdn is second vertex on singular point ?
            if ( surf->IsDegenerated ( pe, Max ( Precision1(), BRepInspector::Tolerance(V) ) ) ) {
          if ( ( c2d1.IsNull() && ! sae.PCurve ( E1, Face(), c2d1, a1, b1, Standard_True ) ) ||
               ( c2d2.IsNull() && ! sae.PCurve ( E2, Face(), c2d2, a2, b2, Standard_True ) ) ) {
            myLastFixStatus |= ShapeExtend1::EncodeStatus ( ShapeExtend_FAIL1 );
            continue;
          }
          gp_Pnt2d p2d1 = c2d1->Value ( b1 ), p2f = c2d2->Value ( a2 ), p2l = c2d2->Value ( b2 );
          Standard_Real pres2 = ::Precision1::PConfusion();
          Standard_Real du = 0.,dv = 0.;
          //#79 rln 15.03.99 S4135: bmarkmdl.igs entity 633 (incorrectly oriented contour) check for
         gap
          if(uclosed&&(Abs(p2f.X()-p2l.X())<pres2)&&Abs(p2d1.X()-p2f.X())>GAS.UResolution(Precision1()))
         { if((Abs(p2f.X()-SUF)<pres2)&&(p2f.Y()<p2l.Y())) du = URange;
            if((Abs(p2f.X()-SUL)<pres2)&&(p2f.Y()>p2l.Y()))
              du = -URange;
          }
          if(vclosed&&(Abs(p2f.Y()-p2l.Y())<pres2)&&Abs(p2d1.Y()-p2f.Y())>GAS.VResolution(Precision1()))
         { if((Abs(p2f.Y()-SVF)<pres2)&&(p2f.X()>p2l.X())) dv = VRange;
            if((Abs(p2f.Y()-SVL)<pres2)&&(p2f.X()<p2l.X()))
              dv = -VRange;
          }
          if ( du ==0. && dv == 0. ) continue;
          myLastFixStatus |= ShapeExtend1::EncodeStatus ( ShapeExtend_DONE1 );
          Transform2d Shift;
          Shift.SetTranslation ( gp_Vec2d ( du, dv ) );
          c2d2->Transform ( Shift );
          UpdateEdgeUVPoints ( E2, Face() );//rln 15.03.99 syntax correction :E1
            }
      */
      //: abv 29.08.01: torCuts.sat:      continue;
    }

    if ((c2d1.IsNull() && !sae.PCurve(E1, Face(), c2d1, a1, b1, Standard_True))
        || (c2d2.IsNull() && !sae.PCurve(E2, Face(), c2d2, a2, b2, Standard_True)))
    {
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
      continue;
    }
    gp_Pnt2d p2d1 = c2d1->Value(b1);
    gp_Pnt2d p2d2 = c2d2->Value(a2);
    box.Add(p2d1);

    Standard_Real du = 0., dv = 0.;
    if (uclosed && isDeg != 1)
    {
      Standard_Real dx = Abs(p2d2.X() - p2d1.X());
      if (dx > URange - UTol)
        du = ShapeAnalysis1::AdjustByPeriod(p2d2.X(), p2d1.X(), URange);
      else if (dx > UTol && stop == nb)
        stop = n2; //: abv 29.08.01: torCuts2.stp
    }
    if (vclosed && isDeg != 2)
    {
      Standard_Real dy = Abs(p2d2.Y() - p2d1.Y());
      if (dy > VRange - VTol)
        dv = ShapeAnalysis1::AdjustByPeriod(p2d2.Y(), p2d1.Y(), VRange);
      else if (dy > VTol && stop == nb)
        stop = n2;
    }
    if (du == 0. && dv == 0.)
      continue;

    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
    Transform2d Shift;
    Shift.SetTranslation(gp_Vec2d(du, dv));
    // c2d2->Transform ( Shift );
    //  skl 15.05.2002 for OCC208 (if few edges have reference to one pcurve)
    Handle(GeomCurve2d) c2d2new = Handle(GeomCurve2d)::DownCast(c2d2->Transformed(Shift));
    sbe.ReplacePCurve(E2, c2d2new, Face());
    UpdateEdgeUVPoints(E2, Face());
  }
  if (box.IsVoid())
    return Standard_False; // #3 smh 01.04.99. S4163: Overflow, when box is void.

  Standard_Real umin, vmin, umax, vmax;
  box.Get(umin, vmin, umax, vmax);
  if (Abs(umin + umax - SUF - SUL) < URange && Abs(vmin + vmax - SVF - SVL) < VRange
      && !LastFixStatus(ShapeExtend_DONE))
    return Standard_False;

  box.SetVoid();
  Standard_Integer n; // svv Jan11 2000 : porting on DEC
  for (n = 1; n <= nb; n++)
  {
    Standard_Real        a, b;
    Handle(GeomCurve2d) c2d;
    if (!sae.PCurve(sbwd->Edge(n), Face(), c2d, a, b, Standard_True))
      continue;
    box.Add(c2d->Value(a));
    box.Add(c2d->Value(0.5 * (a + b)));
  }
  box.Get(umin, vmin, umax, vmax);

  Standard_Real du = 0., dv = 0.;

  if (uclosed)
  {
    Standard_Real umid = 0.5 * (umin + umax);
    //     du = ShapeAnalysis1::AdjustToPeriod(umid, SUF, SUL);
    // PTV 26.06.2002 xloop torus-apple iges face mode
    du = ShapeAnalysis1::AdjustByPeriod(umid, SUMid, URange);
  }
  if (vclosed)
  {
    Standard_Real vmid = 0.5 * (vmin + vmax);
    //     dv = ShapeAnalysis1::AdjustToPeriod(vmid, SVF, SVL);
    // PTV 26.06.2002 xloop torus-apple iges face mode
    dv = ShapeAnalysis1::AdjustByPeriod(vmid, SVMid, VRange);
  }

  if (du == 0. && dv == 0.)
    return Standard_True;

  myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);

  Transform2d Shift;
  Shift.SetTranslation(gp_Vec2d(du, dv));

  for (n = 1; n <= sbwdOring->NbEdges(); n++)
  {
    Standard_Real        a, b;
    Handle(GeomCurve2d) c2d;
    TopoEdge          ed = sbwdOring->Edge(n);
    if (!sae.PCurve(ed, Face(), c2d, a, b, Standard_True))
      continue;
    // skl 15.05.2002 for OCC208 (if few edges have reference to one pcurve)
    Handle(GeomCurve2d) c2d2 = Handle(GeomCurve2d)::DownCast(c2d->Transformed(Shift));
    sbe.ReplacePCurve(ed, c2d2, Face());
    UpdateEdgeUVPoints(ed, Face());
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean WireHealer::FixDegenerated(const Standard_Integer num)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  // analysis
  gp_Pnt2d p2d1, p2d2;
  myAnalyzer->CheckDegenerated(num, p2d1, p2d2);
  if (myAnalyzer->LastCheckStatus(ShapeExtend_FAIL1))
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
  }
  //: abv 29.08.01: torHalf2.sat: if edge was encoded as degenerated but
  //  has no pcurve and no singularity is found at that point, remove it
  if (myAnalyzer->LastCheckStatus(ShapeExtend_FAIL2))
  {
    WireData()->Remove(num);
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
    return Standard_True;
  }
  if (!myAnalyzer->LastCheckStatus(ShapeExtend_DONE))
    return Standard_False;

  // action: create degenerated edge and insert it (or replace)

  gp_Vec2d            vect2d(p2d1, p2d2);
  gp_Dir2d            dir2d(vect2d);
  Handle(Geom2d_Line) line2d = new Geom2d_Line(p2d1, dir2d);

  TopoEdge  degEdge;
  ShapeBuilder B;
  B.MakeEdge(degEdge);
  B.Degenerated(degEdge, Standard_True);
  B.UpdateEdge(degEdge, line2d, Face(), ::Precision1::Confusion());
  B.Range(degEdge, Face(), 0., vect2d.Magnitude());

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             n2   = (num > 0 ? num : sbwd->NbEdges());
  Standard_Integer             n1   = (n2 > 1 ? n2 - 1 : sbwd->NbEdges());

  Standard_Boolean lack = myAnalyzer->LastCheckStatus(ShapeExtend_DONE1);
  Standard_Integer n3   = (lack ? n2 : (n2 < sbwd->NbEdges() ? n2 + 1 : 1));

  Edge1 sae;
  TopoVertex      V1 = sae.LastVertex(sbwd->Edge(n1));
  TopoVertex      V2 = sae.FirstVertex(sbwd->Edge(n3));

  V1.Orientation(TopAbs_FORWARD);
  V2.Orientation(TopAbs_REVERSED);
  B.Add(degEdge, V1);
  B.Add(degEdge, V2);
  degEdge.Orientation(TopAbs_FORWARD);

  if (lack)
  {
    sbwd->Add(degEdge, n2);
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
  }
  else
  {
    sbwd->Set(degEdge, n2);
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
  }

  //  commented to avoid extra messages
  //  SendWarning ( degEdge, Message_Msg ( "FixWire.FixDegenerated.MSG0" ) );// Degenerated edge(s)
  //  detected

  return Standard_True;
}

//=======================================================================
// function : FixSelfIntersectingEdge
// purpose  : Tests edge for self-intersection and updates tolerance of vertex
//           if intersection is found
//           Returns True if tolerance was increased
//=======================================================================

// Create edge on basis of E with new pcurve and call FixSP
// Return resulting tolerance and modified pcurve
static Standard_Boolean TryNewPCurve(const TopoEdge&    E,
                                     const TopoFace&    face,
                                     Handle(GeomCurve2d)& c2d,
                                     Standard_Real&        first,
                                     Standard_Real&        last,
                                     Standard_Real&        tol)
{
  Standard_Real      f, l;
  Handle(GeomCurve3d) crv = BRepInspector::Curve(E, f, l);
  if (crv.IsNull())
    return Standard_False;

  // make temp edge and compute tolerance
  EdgeMaker mkedge(crv, f, l);

  Edge2 SBE;          // skl 17.07.2001
  SBE.SetRange3d(mkedge, f, l); // skl 17.07.2001

  if (!mkedge.IsDone())
    return Standard_False;

  TopoEdge  edge = mkedge;
  ShapeBuilder B;
  B.UpdateEdge(edge, c2d, face, 0.);
  B.Range(edge, face, first, last);
  B.SameRange(edge, Standard_False);
  // no call to BRepLib1:  B.SameParameter ( edge, Standard_False );

  Handle(ShapeFix_Edge) sfe = new ShapeFix_Edge;
  sfe->FixSameParameter(edge, face);
  c2d = BRepInspector::CurveOnSurface(edge, face, first, last);
  tol = BRepInspector::Tolerance(edge);
  return Standard_True;
}

//: k3 abv 24 Dec 98: BUC50070 #26682 and #30087:
// Try to cut out the loop on the pcurve and make new pcurve by concatenating
// the parts.
// If result is not SameParameter with prec, does nothing and returns False
// Warning: resulting pcurve will be C0

//=================================================================================================

static Standard_Integer howMuchPCurves(const TopoEdge& E)
{
  Standard_Integer                             count = 0;
  BRep_ListIteratorOfListOfCurveRepresentation itcr(
    (*((Handle(BRep_TEdge)*)&E.TShape()))->ChangeCurves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurveOnSurface())
      count++;
    itcr.Next();
  }

  return count;
}

//=================================================================================================

static Standard_Boolean RemoveLoop(TopoEdge&                      E,
                                   const TopoFace&                face,
                                   const IntRes2d_IntersectionPoint& IP,
                                   const Standard_Real               tolfact,
                                   const Standard_Real               prec,
                                   const Standard_Boolean            RemoveLoop3d)
{
  Standard_Boolean loopRemoved3d;
  if (BRepInspector::IsClosed(E, face))
    return Standard_False;

  Standard_Real      f, l;
  Handle(GeomCurve3d) crv = BRepInspector::Curve(E, f, l);

  Standard_Real t1 = IP.ParamOnFirst();
  Standard_Real t2 = IP.ParamOnSecond();
  if (t1 > t2)
  {
    Standard_Real t = t1;
    t1              = t2;
    t2              = t;
  }

  Edge1   sae;
  Standard_Real        a, b;
  Handle(GeomCurve2d) c2d;
  if (!sae.PCurve(E, face, c2d, a, b, Standard_False))
    return Standard_False;

#ifdef OCCT_DEBUG
  std::cout << "Cut Loop: params (" << t1 << ", " << t2;
#endif
  GeomAdaptor_Curve GAC(crv, f, l);
  Standard_Real     dt = tolfact * GAC.Resolution(prec);
  t1 -= dt; // 1e-3;//::Precision1::PConfusion();
  t2 += dt; // 1e-3;//::Precision1::PConfusion();
#ifdef OCCT_DEBUG
  std::cout << ") -> (" << t1 << ", " << t2 << ")" << std::endl;
#endif

  if (t1 <= a || t2 >= b)
  { // should not be so, but to be sure ..
    return Standard_False;
  }

  // direct construction causes error on osf system.
  Point3d p(0, 0, 0);
  Dir3d d(0, 0, 1);
  Ax3 ax(p, d);
  gp_Pln Pln(ax);

  // PTV OCC884
  Handle(GeomPlane) aPlaneSurf = Handle(GeomPlane)::DownCast(BRepInspector::Surface(face));
  Handle(GeomCurve3d) pcurve3d   = crv;
  if (!aPlaneSurf.IsNull())
  {
    Pln = aPlaneSurf->Pln();
  }
  else
    pcurve3d = GeomAPI1::To3d(c2d, Pln);

  // first segment
  // Handle(GeomCurve3d) pcurve3d = GeomAPI1::To3d ( c2d, Pln );
  Handle(Geom_TrimmedCurve)           trim = new Geom_TrimmedCurve(pcurve3d, a, t1);
  GeomConvert_CompCurveToBSplineCurve connect(trim);

  // null-length segment patch instead of loop
  TColgp_Array1OfPnt      Poles(1, 2);
  TColStd_Array1OfReal    Knots(1, 2);
  TColStd_Array1OfInteger Mults(1, 2);

  Poles.SetValue(1, trim->Value(t1));
  Knots.SetValue(1, t1);
  Mults.SetValue(1, 2);

  trim = new Geom_TrimmedCurve(pcurve3d, t2, b);
  Poles.SetValue(2, trim->Value(t2));
  Knots.SetValue(2, t2);
  Mults.SetValue(2, 2);

  Handle(BSplineCurve3d) patch = new BSplineCurve3d(Poles, Knots, Mults, 1);
  if (!connect.Add(patch, ::Precision1::PConfusion(), Standard_True, Standard_False))
    return Standard_False;

  // last segment
  if (!connect.Add(trim, ::Precision1::PConfusion(), Standard_True, Standard_False))
    return Standard_False;
  // PTV OCC884
  // keep created 3d curve
  Handle(GeomCurve3d) aNew3dCrv = connect.BSplineCurve();

  Handle(GeomCurve2d) bs = GeomAPI1::To2d(aNew3dCrv, Pln);
  if (bs.IsNull())
    return Standard_False;

  // make temp edge and compute tolerance
  ShapeBuilder B;

  if (!RemoveLoop3d)
  { // old variant (not remove loop 3d)
    Standard_Real newtol = 0;
    // OCC901
    Standard_Integer nbC2d = howMuchPCurves(E);
    if (nbC2d <= 1 && !aPlaneSurf.IsNull())
      B.UpdateEdge(E, bs, face, 0);
    else if (!TryNewPCurve(E, face, bs, a, b, newtol))
      return Standard_False;

    Standard_Real tol = BRepInspector::Tolerance(E);
#ifdef OCCT_DEBUG
    std::cout << "Cut Loop: tol orig " << tol << ", prec " << prec << ", new tol " << newtol
              << std::endl;
#endif
    if (newtol > Max(prec, tol))
      return Standard_False;
    //: s2  bs = BRepInspector::CurveOnSurface ( edge, face, a, b );
    if (Abs(a - f) > ::Precision1::PConfusion() || // smth strange, cancel
        Abs(b - l) > ::Precision1::PConfusion())
      return Standard_False;
    // PTV OCC884
    if (!aPlaneSurf.IsNull())
    {
      B.UpdateEdge(E, aNew3dCrv, Max(newtol, tol));
      // OCC901
      if (!TryNewPCurve(E, face, bs, a, b, newtol))
        return Standard_False;
    }
    B.UpdateEdge(E, bs, face, newtol);
    B.UpdateVertex(sae.FirstVertex(E), newtol);
    B.UpdateVertex(sae.LastVertex(E), newtol);
    return Standard_True;
  }

  //: q1
  TopLoc_Location             L;
  Handle(GeomSurface)        S  = BRepInspector::Surface(face, L);
  Handle(Geom2dAdaptor_Curve) AC = new Geom2dAdaptor_Curve(c2d);
  Handle(GeomAdaptor_Surface) AS = new GeomAdaptor_Surface(S);

  Adaptor3d_CurveOnSurface ACS(AC, AS);
  Point3d                   P1(ACS.Value(t1));
  Point3d                   P2(ACS.Value(t2));
  Point3d pcurPnt((P1.X() + P2.X()) / 2, (P1.Y() + P2.Y()) / 2, (P1.Z() + P2.Z()) / 2);

  ShapeAnalysis_TransferParametersProj SFTP(E, face);
  Handle(TColStd_HSequenceOfReal)      Seq2d = new TColStd_HSequenceOfReal;
  Seq2d->Append(t1);
  Seq2d->Append(t2);
  Seq2d->Append((t1 + t2) / 2);
  Handle(TColStd_HSequenceOfReal) Seq3d = SFTP.Perform(Seq2d, Standard_False);

  // clang-format off
  Standard_Real dist1 = pcurPnt.Distance(crv->Value(Seq3d->Value(1)));// correcting Seq3d already project
  // clang-format on
  Standard_Real dist2 = pcurPnt.Distance(crv->Value(Seq3d->Value(2)));
  Standard_Real dist3 = pcurPnt.Distance(crv->Value(Seq3d->Value(3)));
  Standard_Real ftrim, ltrim;
  if (dist3 > Max(dist1, dist2))
  {
    loopRemoved3d = Standard_False;
  }
  else
  {
    loopRemoved3d = Standard_True;
  }

  Handle(GeomCurve3d) bs1;

  if (!loopRemoved3d)
  {
    // create new 3d curve
    ftrim = Seq3d->Value(1);
    ltrim = Seq3d->Value(2);
    ftrim -= dt;
    ltrim += dt;
    Handle(Geom_TrimmedCurve)           trim1 = new Geom_TrimmedCurve(crv, f, ftrim);
    GeomConvert_CompCurveToBSplineCurve connect1(trim1);
    TColgp_Array1OfPnt                  Poles1(1, 2);
    TColStd_Array1OfReal                Knots1(1, 2);
    TColStd_Array1OfInteger             Mults1(1, 2);

    Poles1.SetValue(1, trim1->Value(ftrim));
    Knots1.SetValue(1, ftrim);
    Mults1.SetValue(1, 2);

    trim1 = new Geom_TrimmedCurve(crv, ltrim, l);
    Poles1.SetValue(2, trim1->Value(ltrim));
    Knots1.SetValue(2, ltrim);
    Mults1.SetValue(2, 2);

    // create b-spline curve
    Handle(BSplineCurve3d) patch1 = new BSplineCurve3d(Poles1, Knots1, Mults1, 1);

    if (!connect1.Add(patch1, ::Precision1::PConfusion(), Standard_True, Standard_False))
      return Standard_False;

    // last segment
    if (!connect1.Add(trim1, ::Precision1::PConfusion(), Standard_True, Standard_False))
      return Standard_False;

    bs1 = connect1.BSplineCurve();

    if (bs1.IsNull())
      return Standard_False;
  }
  //  Standard_Real oldtol = BRepInspector::Tolerance ( E );

  if (!loopRemoved3d)
    B.UpdateEdge(E, bs1, L, 0.);
  B.UpdateEdge(E, bs, face, 0.);
  B.Range(E, face, f, l);
  B.SameRange(E, Standard_False);

  Handle(ShapeFix_Edge) sfe = new ShapeFix_Edge;
  sfe->FixSameParameter(E);

  return Standard_True;
}

//=================================================================================================

static Standard_Boolean RemoveLoop(TopoEdge&                      E,
                                   const TopoFace&                face,
                                   const IntRes2d_IntersectionPoint& IP2d,
                                   TopoEdge&                      E1,
                                   TopoEdge&                      E2)
{
#ifdef OCCT_DEBUG
  std::cout << "Info: WireHealer::FixSelfIntersection : Try insert vertex" << std::endl;
#endif

  if (BRepInspector::IsClosed(E, face))
    return Standard_False;

  Standard_Real      f, l;
  Handle(GeomCurve3d) crv = BRepInspector::Curve(E, f, l);

  Standard_Real t1 = IP2d.ParamOnFirst();
  Standard_Real t2 = IP2d.ParamOnSecond();

  if (t1 > t2)
  {
    Standard_Real t = t1;
    t1              = t2;
    t2              = t;
  }

  Edge1 sae;

  // define vertexes Vfirst , Vlast, Vmid
  TopoVertex Vfirst, Vlast, Vmid;
  // initialize Vfirst and Vlast
  Vfirst = sae.FirstVertex(E);
  Vlast  = sae.LastVertex(E);

  // find a 2d curve and parameters from edge E
  Standard_Real        a, b;
  Handle(GeomCurve2d) c2d;
  if (!sae.PCurve(E, face, c2d, a, b, Standard_False))
    return Standard_False;

  // first segment for 2d curve
  Handle(Geom2d_TrimmedCurve) trim1;
  if ((t1 - a) > Precision1::PConfusion())
    trim1 = new Geom2d_TrimmedCurve(c2d, a, t1);
  // second segment for 2d curve
  Handle(Geom2d_TrimmedCurve) trim2 = new Geom2d_TrimmedCurve(c2d, t2, b);

  //  if ( trim1.IsNull() || trim2.IsNull()  )
  if (trim2.IsNull())
    return Standard_False;

  TopLoc_Location             L;
  Handle(GeomSurface)        S  = BRepInspector::Surface(face, L);
  Handle(Geom2dAdaptor_Curve) AC = new Geom2dAdaptor_Curve(c2d);
  Handle(GeomAdaptor_Surface) AS = new GeomAdaptor_Surface(S);

  Adaptor3d_CurveOnSurface ACS(AC, AS);
  Point3d                   P1(ACS.Value(t1));
  Point3d                   P2(ACS.Value(t2));
  Point3d pcurPnt((P1.X() + P2.X()) / 2, (P1.Y() + P2.Y()) / 2, (P1.Z() + P2.Z()) / 2);

  // transfer parameters from pcurve to 3d curve
  ShapeAnalysis_TransferParametersProj SFTP(E, face);
  Handle(TColStd_HSequenceOfReal)      Seq2d = new TColStd_HSequenceOfReal;
  Seq2d->Append(t1);
  Seq2d->Append(t2);
  Seq2d->Append((t1 + t2) / 2);
  Handle(TColStd_HSequenceOfReal) Seq3d = SFTP.Perform(Seq2d, Standard_False);

  // clang-format off
  Standard_Real dist1 = pcurPnt.Distance(crv->Value(Seq3d->Value(1)));// correcting Seq3d already project
  // clang-format on
  Standard_Real dist2 = pcurPnt.Distance(crv->Value(Seq3d->Value(2)));
  Standard_Real dist3 = pcurPnt.Distance(crv->Value(Seq3d->Value(3)));
  Standard_Real ftrim, ltrim;
  if (dist3 > Max(dist1, dist2))
  { // is loop in 3d
    ftrim = Seq3d->Value(1);
    ltrim = Seq3d->Value(2);
  }
  else
  { // not loop in 3d
    ftrim = Seq3d->Value(3);
    ltrim = Seq3d->Value(3);
  }

  //  ftrim = Seq3d->Value(1);
  //  ltrim = Seq3d->Value(2);

  // trim for 3d curve 'crv' with parameters from 'f' to 'l'
  Handle(Geom_TrimmedCurve) trim3;
  if (!trim1.IsNull())
    trim3 = new Geom_TrimmedCurve(crv, f, ftrim);
  // second segment for 3d curve
  Handle(Geom_TrimmedCurve) trim4 = new Geom_TrimmedCurve(crv, ltrim, l);

  //  if ( trim3.IsNull() || trim4.IsNull()  )
  if (trim4.IsNull())
    return Standard_False;

  // create a point for middle vertex
  Point3d pnt1 = crv->Value(ftrim);
  Point3d pnt2 = crv->Value(ltrim);
  Point3d Pmid((pnt1.X() + pnt2.X()) / 2, (pnt1.Y() + pnt2.Y()) / 2, (pnt1.Z() + pnt2.Z()) / 2);

  ShapeBuilder B;

  // create new copies for E1 and E2
  if (!trim1.IsNull())
    E1 = TopoDS::Edge(E.EmptyCopied());
  E2 = TopoDS::Edge(E.EmptyCopied());

  // initialize middle vertex Vmid
  if (trim1.IsNull())
    B.MakeVertex(Vmid, pnt2, 0.);
  else
    B.MakeVertex(Vmid, Pmid, 0.);

  Edge2 sbe;

  // replace verteces for new edges E1 and E2
  if (E.Orientation() == TopAbs_FORWARD)
  {
    if (!E1.IsNull())
      E1 = sbe.CopyReplaceVertices(E1, Vfirst, Vmid);
    E2 = sbe.CopyReplaceVertices(E2, Vmid, Vlast);
  }
  else
  {
    if (!E1.IsNull())
      E1 = sbe.CopyReplaceVertices(E1, Vmid, Vlast);
    E2 = sbe.CopyReplaceVertices(E2, Vfirst, Vmid);
  }

  // Update edges by 2d and 3d curves
  Handle(ShapeFix_Edge) mySfe = new ShapeFix_Edge;
  if (!E1.IsNull())
  {
    B.UpdateEdge(E1, trim1, face, 0.);
    B.UpdateEdge(E1, trim3, 0.);
    B.Range(E1, f, ftrim);
    B.SameRange(E1, Standard_False);
    //    B.SameParameter(E1,Standard_False);
    mySfe->FixSameParameter(E1);
    mySfe->FixVertexTolerance(E1);
  }
  B.UpdateEdge(E2, trim2, face, 0.);
  B.UpdateEdge(E2, trim4, 0.);
  B.Range(E2, ltrim, l);
  B.SameRange(E2, Standard_False);
  //  B.SameParameter(E2, Standard_False);
  mySfe->FixSameParameter(E2);
  mySfe->FixVertexTolerance(E2);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean WireHealer::FixSelfIntersectingEdge(const Standard_Integer num)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  // analysis
  IntRes2d_SequenceOfIntersectionPoint points2d;
  TColgp_SequenceOfPnt                 points3d;
  Handle(ShapeAnalysis_Wire)           theAdvAnalyzer = myAnalyzer;
  if (theAdvAnalyzer.IsNull())
    return Standard_False;
  theAdvAnalyzer->CheckSelfIntersectingEdge(num, points2d, points3d);
  if (theAdvAnalyzer->LastCheckStatus(ShapeExtend_FAIL))
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
  }
  if (!theAdvAnalyzer->LastCheckStatus(ShapeExtend_DONE))
    return Standard_False;

  // action: increase tolerance of vertex

  TopoEdge E = WireData()->Edge(num > 0 ? num : NbEdges());

  Edge1 sae;
  TopoVertex      V1   = sae.FirstVertex(E);
  TopoVertex      V2   = sae.LastVertex(E);
  Standard_Real      tol1 = BRepInspector::Tolerance(V1);
  Standard_Real      tol2 = BRepInspector::Tolerance(V2);
  Point3d             pnt1 = BRepInspector::Pnt(V1);
  Point3d             pnt2 = BRepInspector::Pnt(V2);

  // cycle is to verify fix in case of RemoveLoop
  Standard_Real        tolfact = 0.1; // factor for shifting by parameter in RemoveLoop
  Standard_Real        f2d = 0., l2d = 0.;
  Handle(GeomCurve2d) c2d;
  Standard_Real        newtol = 0.; // = Precision1();

  if (myRemoveLoopMode < 1)
  {
    for (Standard_Integer iter = 0; iter < 30; iter++)
    {
      Standard_Boolean loopRemoved = Standard_False;
      Standard_Real    prevFirst = 0, prevLast = 0;
      for (Standard_Integer i = 1; i <= points2d.Length(); i++)
      {
        Point3d        pint   = points3d.Value(i);
        Standard_Real dist21 = pnt1.SquareDistance(pint);
        Standard_Real dist22 = pnt2.SquareDistance(pint);
        if (dist21 < tol1 * tol1 || dist22 < tol2 * tol2)
          continue;
        newtol = 1.001 * Sqrt(Min(dist21, dist22)); //: f8

        //: k3 abv 24 Dec 98: BUC50070 #26682 and #30087: try to remove loop
        if (myGeomMode)
        {
          if (c2d.IsNull())
            sae.PCurve(E, Face(), c2d, f2d, l2d, Standard_False);
          Standard_Real firstpar = points2d.Value(i).ParamOnFirst();
          Standard_Real lastpar  = points2d.Value(i).ParamOnSecond();
          if (firstpar > prevFirst && lastpar < prevLast)
            continue;
          if (RemoveLoop(E,
                         Face(),
                         points2d.Value(i),
                         tolfact,
                         Min(MaxTolerance(), Max(newtol, Precision1())),
                         myRemoveLoopMode == 0))
          {
            myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE4);
            loopRemoved = Standard_True;
            prevFirst   = firstpar;
            prevLast    = lastpar;
            continue; // repeat of fix on that edge required (to be done by caller)
          }
        }
        if (newtol < MaxTolerance())
        {
          myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
          ShapeBuilder B;
          if (dist21 < dist22)
            B.UpdateVertex(V1, tol1 = newtol);
          else
            B.UpdateVertex(V2, tol2 = newtol);
        }
        else
          myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
      }

      // after RemoveLoop, check that self-intersection disappeared
      if (loopRemoved)
      {
        IntRes2d_SequenceOfIntersectionPoint pnts2d;
        TColgp_SequenceOfPnt                 pnts3d;
        theAdvAnalyzer->CheckSelfIntersectingEdge(num, pnts2d, pnts3d);
        if (!theAdvAnalyzer->LastCheckStatus(ShapeExtend_DONE))
          break;
        // points3d.Append(pnts3d);
        // points2d.Append(pnts2d);
        points3d = pnts3d;
        points2d = pnts2d;
        ShapeBuilder B;
        B.UpdateEdge(E, c2d, Face(), 0.);
        B.Range(E, Face(), f2d, l2d);
        // newtol+=Precision1();
      }
      else
      {
        break;
      }
    }
  }

  //===============================================
  // RemoveLoopMode = 1 , insert vertex
  //===============================================
  if (myRemoveLoopMode == 1)
  {
    // after fixing will be nb+1 edges
    Standard_Boolean loopRemoved;
    // create a sequence of resulting edges
    Handle(TopTools_HSequenceOfShape) TTSS = new TopTools_HSequenceOfShape;
    TopoEdge                       E2;

    loopRemoved = Standard_False;
    //: k3 abv 24 Dec 98: BUC50070 #26682 and #30087: try to remove loop
    if (myGeomMode)
    {
      if (c2d.IsNull())
        sae.PCurve(E, Face(), c2d, f2d, l2d, Standard_False);
      TopoEdge E1;
      if (RemoveLoop(E, Face(), points2d.Value(1), E1, E2))
      {
        myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE4);
        loopRemoved = Standard_True;
        if (!E1.IsNull())
        {
          TTSS->Append(E1);
          newtol = Max(BRepInspector::Tolerance(E1), BRepInspector::Tolerance(E2));
        }
        else
          newtol = BRepInspector::Tolerance(E2);
      }
    }

    TTSS->Append(E2);

    if (newtol > MaxTolerance())
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);

    ShapeExtend_WireData sewd;
    for (Standard_Integer i = 1; i <= TTSS->Length(); i++)
    {
      sewd.Add(TopoDS::Edge(TTSS->Value(i)));
    }
    if (!Context().IsNull())
    {
      Context()->Replace(E, sewd.Wire());
      UpdateWire();
    }
    else
    {
      WireData()->Remove(num > 0 ? num : NbEdges());
      WireData()->Add(sewd.Wire(), num > 0 ? num : NbEdges());
    }
    if (loopRemoved)
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE8);
  }

  if (LastFixStatus(ShapeExtend_DONE) && !myShape.IsNull())
  {
    // clang-format off
    SendWarning ( E, Message_Msg ( "FixAdvWire.FixIntersection.MSG5" ) );// Edge was self-intersecting, corrected
    // clang-format on
  }

  return LastFixStatus(ShapeExtend_DONE);
}

//=================================================================================================

static Standard_Real ComputeLocalDeviation(const TopoEdge& edge,
                                           const Point3d&      pint,
                                           const Point3d&      pnt,
                                           Standard_Real      f,
                                           Standard_Real      l,
                                           const TopoFace& face)
{
  Edge1 sae;
  Handle(GeomCurve3d) c3d;
  Standard_Real      a, b;
  if (!sae.Curve3d(edge, c3d, a, b, Standard_False))
    return RealLast();

  gp_Lin line(pint, Vector3d(pint, pnt));

  Handle(GeomCurve2d) Crv;
  Standard_Real        fp, lp;
  if (sae.PCurve(edge, face, Crv, fp, lp, Standard_False))
  {
    if (Crv->IsKind(STANDARD_TYPE(Geom2d_TrimmedCurve)))
    {
      Handle(Geom2d_TrimmedCurve) tc = Handle(Geom2d_TrimmedCurve)::DownCast(Crv);
      if (tc->BasisCurve()->IsKind(STANDARD_TYPE(Geom2d_Line)))
      {
        f = a + (f - fp) * (b - a) / (lp - fp);
        l = a + (l - fp) * (b - a) / (lp - fp);
      }
    }
  }

  const Standard_Integer NSEG = 10;
  Standard_Real          step = (l - f) / NSEG;
  Standard_Real          dev  = 0.;
  for (Standard_Integer i = 1; i < NSEG; i++)
  {
    Point3d        p = c3d->Value(f + i * step);
    Standard_Real d = line.Distance(p);
    if (dev < d)
      dev = d;
  }
  return dev;
}

//=================================================================================================

Standard_Boolean WireHealer::FixIntersectingEdges(const Standard_Integer num)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady() || NbEdges() < 2)
    return Standard_False;

  // analysis
  IntRes2d_SequenceOfIntersectionPoint points2d;
  TColgp_SequenceOfPnt                 points3d;
  TColStd_SequenceOfReal               errors;
  Handle(ShapeAnalysis_Wire)           theAdvAnalyzer = myAnalyzer;
  if (theAdvAnalyzer.IsNull())
    return Standard_False;
  theAdvAnalyzer->CheckIntersectingEdges(num, points2d, points3d, errors);
  if (theAdvAnalyzer->LastCheckStatus(ShapeExtend_FAIL))
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
  }
  if (!theAdvAnalyzer->LastCheckStatus(ShapeExtend_DONE))
    return Standard_False;

  // rln 03/02/98: CSR#BUC50004 entity 56 (to avoid later inserting lacking edge)
  //: l0  Standard_Boolean isLacking = myAnalyzer->CheckLacking ( num );

  // action: increase tolerance of vertex

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             n2   = (num > 0 ? num : sbwd->NbEdges());
  Standard_Integer             n1   = (n2 > 1 ? n2 - 1 : sbwd->NbEdges());
  TopoEdge                  E1   = sbwd->Edge(n1);
  TopoEdge                  E2   = sbwd->Edge(n2);
  if (!Context().IsNull())
  {
    E1 = TopoDS::Edge(Context()->Apply(sbwd->Edge(n1)));
    E2 = TopoDS::Edge(Context()->Apply(sbwd->Edge(n2)));
  }

  Standard_Boolean isForward1 = (E1.Orientation() == TopAbs_FORWARD);
  Standard_Boolean isForward2 = (E2.Orientation() == TopAbs_FORWARD);
  Standard_Real    a1, b1, a2, b2;
  BRepInspector::Range(E1, Face(), a1, b1);
  BRepInspector::Range(E2, Face(), a2, b2);

  Edge1 sae;
  TopoVertex      Vp = sae.FirstVertex(E1);
  TopoVertex      V1 = sae.LastVertex(E1);
  TopoVertex      V2 = sae.FirstVertex(E2);
  TopoVertex      Vn = sae.LastVertex(E2);

  Standard_Real tol = BRepInspector::Tolerance(V1);
  Point3d        pnt = BRepInspector::Pnt(V1);

  Standard_Real    prevRange1 = RealLast(), prevRange2 = RealLast();
  Standard_Boolean cutEdge1 = Standard_False, cutEdge2 = Standard_False;
  Standard_Boolean IsCutLine     = Standard_False;
  Standard_Boolean isChangedEdge = Standard_False;

  ShapeBuilder B;

  Standard_Integer nb = points3d.Length();
  for (Standard_Integer i = 1; i <= nb; i++)
  {
    const IntRes2d_IntersectionPoint& IP     = points2d.Value(i);
    Standard_Real                     param1 = (num == 1 ? IP.ParamOnSecond() : IP.ParamOnFirst());
    Standard_Real                     param2 = (num == 1 ? IP.ParamOnFirst() : IP.ParamOnSecond());

    Standard_Real newRange1 = Abs((isForward1 ? a1 : b1) - param1);
    Standard_Real newRange2 = Abs((isForward2 ? b2 : a2) - param2);
    if (newRange1 > prevRange1 && newRange2 > prevRange2)
      continue;

    Point3d        pint   = points3d.Value(i);
    Standard_Real rad    = errors.Value(i);
    Standard_Real newtol = 1.0001 * (pnt.Distance(pint) + rad);

    // GeomAdaptor_Surface& Ads = myAnalyzer->Surface()->Adaptor3d()->ChangeSurface();

    //: r8 abv 12 Apr 99: try increasing tolerance of edge

    Standard_Boolean locMayEdit = myTopoMode;
    // Always try to modify the tolerance firstly as a better solution
    if (/*! myTopoMode &&*/ newtol > tol)
    {
      Standard_Real te1 =
        rad + ComputeLocalDeviation(E1, pint, pnt, param1, (isForward1 ? b1 : a1), Face());
      Standard_Real te2 =
        rad + ComputeLocalDeviation(E2, pint, pnt, (isForward2 ? a2 : b2), param2, Face());
      Standard_Real maxte = Max(te1, te2);
      if (maxte < MaxTolerance() && maxte < newtol)
      {
        if (BRepInspector::Tolerance(E1) < te1 || BRepInspector::Tolerance(E2) < te2)
        {
#ifdef OCCT_DEBUG
          std::cout << "Warning: WireHealer::FixIE: edges tolerance increased: (" << te1 << ", "
                    << te2 << ") / " << newtol << std::endl;
#endif

          // Make copy of edges.
          if (!Context().IsNull())
          {
            isChangedEdge = Standard_True; // To avoid double copying of vertexes.

            // Intersection point of two base edges.
            Edge2 aSBE;
            TopoVertex   VV1 = Context()->CopyVertex(V1);

            TopoVertex VVp = Vp;
            TopoVertex VVn = Vn;
            if (Vp.IsSame(Vn))
            {
              // Should modify only one vertex.
              VVp = Context()->CopyVertex(Vp);
              VVn = VVp;
            }
            else
            {
              VVp = Context()->CopyVertex(Vp);
              VVn = Context()->CopyVertex(Vn);
            }

            TopoEdge EE1 = aSBE.CopyReplaceVertices(E1, VVp, VV1);
            TopoEdge EE2 = aSBE.CopyReplaceVertices(E2, VV1, VVn);

            Context()->Replace(E1, EE1);
            Context()->Replace(E2, EE2);

            UpdateWire();
            E1 = sbwd->Edge(n1);
            E2 = sbwd->Edge(n2);
            Vp = sae.FirstVertex(E1);
            V1 = sae.LastVertex(E1);
            V2 = sae.FirstVertex(E2);
            Vn = sae.LastVertex(E2);
          }

          B.UpdateEdge(E1, 1.000001 * te1);
          B.UpdateVertex(sae.FirstVertex(E1), 1.000001 * te1);
          B.UpdateVertex(sae.LastVertex(E1), 1.000001 * te1);
          B.UpdateEdge(E2, 1.000001 * te2);
          B.UpdateVertex(sae.FirstVertex(E2), 1.000001 * te2);
          B.UpdateVertex(sae.LastVertex(E2), 1.000001 * te2);

          myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE6);
          locMayEdit = Standard_False;
        }
        newtol = 1.000001 * maxte;
      }
    }

    if (locMayEdit || newtol <= MaxTolerance())
    {
      prevRange1 = newRange1;
      prevRange2 = newRange2;
      if (locMayEdit)
      {
        newtol = 1.0001 * (pnt.Distance(pint) + rad);
        //: j6 abv 7 Dec 98: ProSTEP TR10 r0601_id.stp #57676 & #58586: do not cut edges because of
        //: influence on adjacent faces
        SplitTool aTool;

        if (!aTool.CutEdge(E1, (isForward1 ? a1 : b1), param1, Face(), IsCutLine))
        {
          if (V1.IsSame(Vp))
            myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
          else
            locMayEdit = Standard_False;
        }
        else
          cutEdge1 = Standard_True; //: h4

        if (!aTool.CutEdge(E2, (isForward2 ? b2 : a2), param2, Face(), IsCutLine))
        {
          if (V2.IsSame(Vn))
            myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE4);
          else
            locMayEdit = Standard_False;
        }
        else
          cutEdge2 = Standard_True; //: h4
      }

      if (locMayEdit && newRange1 <= prevRange1 && newRange2 <= prevRange2 && // rln 09/01/98
          BRepInspector::SameParameter(E1) && BRepInspector::SameParameter(E2))
      {
        myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
        pnt = pint;
        if (tol <= rad)
        {
          myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
          tol = 1.001 * rad;
        }
      }
      else
      {
        if (IsCutLine)
        {
          myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
          pnt = pint;
          if (tol <= rad)
          {
            myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
            tol = 1.001 * rad;
          }
        }
        else
        { // else increase tolerance
          if (tol < newtol)
          { // rln 07.04.99 CCI60005-brep.igs
            myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
            tol = newtol;
          }
        }
      }
    }
    else
    {
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
    }
  }

  if (!LastFixStatus(ShapeExtend_DONE))
    return Standard_False;

  if (isChangedEdge)
  {
    B.UpdateVertex(V1, pnt, tol);
    B.UpdateVertex(V2, pnt, tol);
  }
  else
  {
    if (!Context().IsNull())
    {
      if (V1.IsSame(V2))
      {
        Context()->CopyVertex(V1, pnt, tol);
      }
      else
      {
        Context()->CopyVertex(V1, pnt, tol);
        Context()->CopyVertex(V2, pnt, tol);
      }
    }
    else
    {
      B.UpdateVertex(V1, pnt, tol);
      B.UpdateVertex(V2, pnt, tol);
    }
  }

  //: h4: make edges SP (after all cuts: t4mug.stp #3730+#6460)
  if (cutEdge1)
  {
    if (!Context().IsNull())
      E1 = TopoDS::Edge(Context()->Apply(E1));
    myFixEdge->FixSameParameter(E1);
  }
  if (cutEdge2 && !IsCutLine)
  {
    if (!Context().IsNull())
      E2 = TopoDS::Edge(Context()->Apply(E2));
    myFixEdge->FixSameParameter(E2);
  }
  if (cutEdge1 || cutEdge2)
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE7);
  }
  if (!myShape.IsNull())
  {
    // clang-format off
    SendWarning ( Message_Msg ( "FixAdvWire.FixIntersection.MSG10" ) );// Edges were intersecting, corrected
    // clang-format on
  }
  return Standard_True;
}

//=================================================================================================

// pdn 17.03.99 fixing non adjacent intersection by increasing tolerance of vertex

Standard_Boolean WireHealer::FixIntersectingEdges(const Standard_Integer num1,
                                                     const Standard_Integer num2)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;
  IntRes2d_SequenceOfIntersectionPoint points2d;
  TColgp_SequenceOfPnt                 points3d;
  TColStd_SequenceOfReal               errors;
  Handle(ShapeAnalysis_Wire)           theAdvAnalyzer = myAnalyzer;
  if (theAdvAnalyzer.IsNull())
    return Standard_False;
  theAdvAnalyzer->CheckIntersectingEdges(num1, num2, points2d, points3d, errors);
  if (theAdvAnalyzer->LastCheckStatus(ShapeExtend_FAIL))
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
  }
  if (!theAdvAnalyzer->LastCheckStatus(ShapeExtend_DONE))
    return Standard_False;
  TColgp_Array1OfPnt     vertexPoints(1, 4);
  TColStd_Array1OfReal   vertexTolers(1, 4);
  TColStd_Array1OfReal   newTolers(1, 4);
  TopTools_Array1OfShape vertices(1, 4);
  newTolers.Init(0);

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             n2   = (num1 > 0 ? num1 : sbwd->NbEdges());
  Standard_Integer             n1   = (num2 > 1 ? num2 : sbwd->NbEdges());
  if (n1 == n2)
    return Standard_False;

  TopoEdge edge1 = sbwd->Edge(n1);
  TopoEdge edge2 = sbwd->Edge(n2);

  Edge1 sae;
  vertices(1) = sae.FirstVertex(edge1);
  vertices(2) = sae.LastVertex(edge1);
  vertices(3) = sae.FirstVertex(edge2);
  vertices(4) = sae.LastVertex(edge2);

  Standard_Integer i; // svv Jan11 2000 : porting on DEC
  for (i = 1; i <= 4; i++)
  {
    vertexPoints(i) = BRepInspector::Pnt(TopoDS::Vertex(vertices(i)));
    vertexTolers(i) = BRepInspector::Tolerance(TopoDS::Vertex(vertices(i)));
  }

  Standard_Real    aNewTolEdge1 = 0.0, aNewTolEdge2 = 0.0;
  Standard_Integer nb = points3d.Length();
  for (i = 1; i <= nb; i++)
  {
    Point3d pint = points3d.Value(i);

    // searching for the nearest vertexies to the intersection point
    Standard_Real    aVtx1Param = 0., aVtx2Param = 0.;
    Standard_Integer aVC1, aVC2;
    Standard_Real    aMinDist = RealLast();
    Point3d           aNearestVertex;
    Standard_Real    aNecessaryVtxTole = 0.0;
    for (aVC1 = 1; aVC1 <= 2; aVC1++)
    {
      for (aVC2 = 3; aVC2 <= 4; aVC2++)
      {

        Standard_Real aVtxIPDist  = pint.Distance(vertexPoints(aVC1));
        Standard_Real aVtxVtxDist = vertexPoints(aVC1).Distance(vertexPoints(aVC2));
        if (aMinDist > aVtxIPDist && aVtxIPDist > aVtxVtxDist)
        {
          aNecessaryVtxTole = aVtxVtxDist;
          aNearestVertex    = vertexPoints(aVC1);
          aMinDist          = aVtxIPDist;
          aVtx1Param        = BRepInspector::Parameter(TopoDS::Vertex(vertices(aVC1)), edge1);
          aVtx2Param        = BRepInspector::Parameter(TopoDS::Vertex(vertices(aVC2)), edge2);
        }
      }
    }

    // calculation of necessary tolerances of edges
    const IntRes2d_IntersectionPoint& IP     = points2d.Value(i);
    Standard_Real                     param1 = IP.ParamOnFirst();
    Standard_Real                     param2 = IP.ParamOnSecond();
    Handle(GeomCurve3d)                aCurve1, aCurve2;
    Standard_Real                     f, l;
    TopLoc_Location                   L1, L2;
    aCurve1 = BRepInspector::Curve(edge1, L1, f, l);
    aCurve2 = BRepInspector::Curve(edge2, L2, f, l);

    // if aMinDist lower than resolution than the intersection point lyes inside the vertex
    if (aMinDist < gp1::Resolution())
      continue;

    Standard_Real aMaxEdgeTol1 = 0.0, aMaxEdgeTol2 = 0.0;
    if (aMinDist < RealLast() && !aCurve1.IsNull() && !aCurve2.IsNull())
    {
      gp_Lin           aLig(aNearestVertex, Vector3d(aNearestVertex, pint));
      Standard_Integer aPointsC;
      Standard_Real    du1   = 0.05 * (param1 - aVtx1Param);
      Standard_Real    du2   = 0.05 * (param2 - aVtx2Param);
      Standard_Real    tole1 = BRepInspector::Tolerance(edge1);
      Standard_Real    tole2 = BRepInspector::Tolerance(edge2);
      for (aPointsC = 2; aPointsC < 19; aPointsC++)
      {
        Standard_Real u  = aVtx1Param + aPointsC * du1;
        Point3d        P1 = aCurve1->Value(u);
        P1.Transform(L1.Transformation());
        Standard_Real d1 = aLig.Distance(P1) * 2.0000001;
        if (d1 > tole1 && d1 > aMaxEdgeTol1)
          aMaxEdgeTol1 = d1;

        u         = aVtx2Param + aPointsC * du2;
        Point3d P2 = aCurve2->Value(u);
        P2.Transform(L2.Transformation());
        Standard_Real d2 = aLig.Distance(P2) * 2.0000001;
        if (d2 > tole2 && d2 > aMaxEdgeTol2)
          aMaxEdgeTol2 = d2;
      }
      if (aMaxEdgeTol1 == 0.0 && aMaxEdgeTol2 == 0.0)
        continue;
      // if the vertexies are far than tolerances so
      // we do not need to increase edge tolerance
      if (aNecessaryVtxTole > Max(aMaxEdgeTol1, tole1)
          || aNecessaryVtxTole > Max(aMaxEdgeTol2, tole2))
      {
        aMaxEdgeTol1 = 0.0;
        aMaxEdgeTol2 = 0.0;
      }
    }

    Standard_Real    rad    = errors.Value(i);
    Standard_Real    finTol = RealLast();
    Standard_Integer rank   = 1;
    for (Standard_Integer j = 1; j <= 4; j++)
    {
      Standard_Real newtol = 1.0001 * (pint.Distance(vertexPoints(j)) + rad);
      if (newtol < finTol)
      {
        rank   = j;
        finTol = newtol;
      }
    }
    if (finTol <= MaxTolerance())
    {
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
      if (newTolers(rank) < finTol)
      {
        if (Max(aMaxEdgeTol1, aMaxEdgeTol2) < finTol && (aMaxEdgeTol1 > 0 || aMaxEdgeTol2 > 0))
        {
          aNewTolEdge1 = Max(aNewTolEdge1, aMaxEdgeTol1);
          aNewTolEdge2 = Max(aNewTolEdge2, aMaxEdgeTol2);
        }
        else
        {
          newTolers(rank) = finTol;
        }
      }
    }
    else
    {
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
    }
  }

  ShapeBuilder B;
  // update of tolerances of edges
  if (aNewTolEdge1 > 0)
  {
    for (i = 1; i <= 2; i++)
      if (aNewTolEdge1 > Max(vertexTolers(i), newTolers(i)))
        newTolers(i) = aNewTolEdge1;
    B.UpdateEdge(edge1, aNewTolEdge1);
  }
  if (aNewTolEdge2 > 0)
  {
    for (i = 3; i <= 4; i++)
      if (aNewTolEdge2 > Max(vertexTolers(i), newTolers(i)))
        newTolers(i) = aNewTolEdge2;
    B.UpdateEdge(edge2, aNewTolEdge2);
  }

  // update of tolerances of vertexies
  for (i = 1; i <= 4; i++)
    if (newTolers(i) > 0)
      B.UpdateVertex(TopoDS::Vertex(vertices(i)), newTolers(i));

  if (!myShape.IsNull())
  {
    // clang-format off
    SendWarning ( Message_Msg ( "FixAdvWire.FixIntersection.MSG10" ) );// Edges were intersecting, corrected
    // clang-format on
  }
  return Standard_True;
}

//=======================================================================
// function : FixLacking
// purpose  : Test if two adjucent edges are disconnected in 2d (while connected
//           in 3d), and in that case either increase tolerance of the vertex or
//           add a new edge (straight in 2d space), in order to close wire in 2d.
//           Returns True if edge was added or tolerance was increased.
// NOTE     : Is to be run after FixDegenerated
// Algorithm: 1. Compute the 2d gap between edges and calculate a tolerance
//              which should have vertex in order to comprise the gap
//              (using GeomAdaptor_Surface); computed value is inctol
//           2. If inctol < tol of vertex, return False (everything is OK)
//           3. If inctol < Precision1, just increase tolerance of vertex to inctol
//           4. Else (if both edges are not degenerated) try to add new edge
//              with straight pcurve (in order to close the gap):
//              a) if flag MayEdit is False
//                 1. if inctol < MaxTolerance, increase tolerance of vertex to inctol
//                 2. else try to add degenerated edge (check that middle point of
//                    that pcurveis inside the vertex)
//              b) if MayEdit is True
//                 1. try to replace big vertex with two new small vertices
//                    connected by new edge. This is made if there is a 3d space
//                    between ends of adjacent edges.
//                 2. if inctol < MaxTolerance, increase tolerance of vertex to inctol
//                 3. else add either degenerated or closed edge (if middle point
//                    of a pcurve of a new edge is inside the vertex, then
//		      degenerated edge is added, else new edge is closed).
//           5. If new edge cannot be added, but inctol < MaxTolerance,
//              when increase tolerance of vertex to a value of inctol
// Short list of some internal variables:
// tol    - tolerance of vertex
// tol2d  - tolerance in parametric space of the surface corresponding to 2*tol
// dist2d - distance between ends of pcurves of edges (2d)
// inctol - tolerance required for vertex to close 2d gap (=tol*dist2d/tol2d)
// tol1, tol2 - tolerances of edges, tol0 = tol1 + tol2
// p3d1, p3d2 - ends of 3d curves of edges
//=======================================================================
//: h2 abv 28 May 98: merged modifications by abv 22 Apr 98, gka 27 May 98
// and pdn 25 May 98 concerning lacking closed or degenerated edges
// Example files: r0501_pe #107813, UKI60107-6 250, UKI60107-3 1577.

//: s2 abv 21 Apr 99: add functionality for bending pcurve
static Standard_Boolean TryBendingPCurve(const TopoEdge&     E,
                                         const TopoFace&     face,
                                         const gp_Pnt2d         p2d,
                                         const Standard_Boolean end,
                                         Handle(GeomCurve2d)&  c2d,
                                         Standard_Real&         first,
                                         Standard_Real&         last,
                                         Standard_Real&         tol)
{
  Edge1 sae;
  if (!sae.PCurve(E, face, c2d, first, last, Standard_False))
    return Standard_False;

  {
    try
    {
      OCC_CATCH_SIGNALS
      Handle(Geom2d_BSplineCurve) bs;
      if (c2d->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve)))
        bs = Handle(Geom2d_BSplineCurve)::DownCast(c2d->Copy());
      else // if ( c2d->IsKind(STANDARD_TYPE(Geom2d_Line)) )
      {
        Handle(Geom2d_TrimmedCurve) trim = new Geom2d_TrimmedCurve(c2d, first, last);
        bs                               = Geom2dConvert1::CurveToBSplineCurve(trim);
      }
      if (bs.IsNull())
        return Standard_False;

      Standard_Real par = (end ? last : first);
      if (fabs(bs->FirstParameter() - par) < ::Precision1::PConfusion()
          && bs->Multiplicity(1) > bs->Degree())
        bs->SetPole(1, p2d);
      else if (fabs(bs->LastParameter() - par) < ::Precision1::PConfusion()
               && bs->Multiplicity(bs->NbKnots()) > bs->Degree())
        bs->SetPole(bs->NbPoles(), p2d);
      else
      {
        bs->Segment1(first, last);
        if (fabs(bs->FirstParameter() - par) < ::Precision1::PConfusion()
            && bs->Multiplicity(1) > bs->Degree())
          bs->SetPole(1, p2d);
        else if (fabs(bs->LastParameter() - par) < ::Precision1::PConfusion()
                 && bs->Multiplicity(bs->NbKnots()) > bs->Degree())
          bs->SetPole(bs->NbPoles(), p2d);
        else
          return Standard_False;
      }
      c2d = bs;

      if (!TryNewPCurve(E, face, c2d, first, last, tol))
        return Standard_False;
    }
    catch (ExceptionBase const&)
    {
#ifdef OCCT_DEBUG
      std::cout << "Warning: WireHealer::FixLacking: Exception in Geom2d_BSplineCurve::Segment1()"
                << std::endl;
#endif
      return Standard_False;
    }
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean WireHealer::FixLacking(const Standard_Integer num, const Standard_Boolean force)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  //=============
  // First phase: analysis whether the problem (gap) exists
  gp_Pnt2d p2d1, p2d2;
  myAnalyzer->CheckLacking(num, (force ? Precision1() : 0.), p2d1, p2d2);
  if (myAnalyzer->LastCheckStatus(ShapeExtend_FAIL))
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
  }
  if (!myAnalyzer->LastCheckStatus(ShapeExtend_DONE))
    return Standard_False;

  //=============
  // Second phase: collection of data necessary for further analysis

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             n2   = (num > 0 ? num : sbwd->NbEdges());
  Standard_Integer             n1   = (n2 > 1 ? n2 - 1 : sbwd->NbEdges());
  TopoEdge                  E1   = sbwd->Edge(n1);
  TopoEdge                  E2   = sbwd->Edge(n2);

  Edge1 sae;
  TopoVertex      V1  = sae.LastVertex(E1);
  TopoVertex      V2  = sae.FirstVertex(E2);
  Standard_Real      tol = Max(BRepInspector::Tolerance(V1), BRepInspector::Tolerance(V2));

  Standard_Real Prec   = Precision1();
  Standard_Real dist2d = myAnalyzer->MaxDistance2d();
  Standard_Real inctol = myAnalyzer->MaxDistance3d();

  TopoFace                   face = myAnalyzer->Face();
  Handle(ShapeAnalysis_Surface) surf = myAnalyzer->Surface();

  Point3d        p3d1, p3d2;
  Standard_Real tol1 = ::Precision1::Confusion(), tol2 = ::Precision1::Confusion(); // SK

  //=============
  //: s2 abv 21 Apr 99: Speculation: try bending pcurves
  Standard_Real        bendtol1 = 0., bendtol2 = 0.;
  Handle(GeomCurve2d) bendc1, bendc2;
  Standard_Real        bendf1 = 0., bendl1 = 0., bendf2 = 0., bendl2 = 0.;
  if (myGeomMode && !BRepInspector::IsClosed(E1, face) && !BRepInspector::IsClosed(E2, face))
  {
    gp_Pnt2d         p2d = 0.5 * (p2d1.XY() + p2d2.XY());
    Standard_Boolean ok1 = TryBendingPCurve(E1,
                                            face,
                                            p2d,
                                            E1.Orientation() == TopAbs_FORWARD,
                                            bendc1,
                                            bendf1,
                                            bendl1,
                                            bendtol1);
    Standard_Boolean ok2 = TryBendingPCurve(E2,
                                            face,
                                            p2d,
                                            E2.Orientation() == TopAbs_REVERSED,
                                            bendc2,
                                            bendf2,
                                            bendl2,
                                            bendtol2);
    if (ok1 && !ok2)
    {
      bendtol2 = BRepInspector::Tolerance(E2);
      ok1      = TryBendingPCurve(E1,
                             face,
                             p2d2,
                             E1.Orientation() == TopAbs_FORWARD,
                             bendc1,
                             bendf1,
                             bendl1,
                             bendtol1);
    }
    else if (!ok1 && ok2)
    {
      bendtol1 = BRepInspector::Tolerance(E1);
      ok2      = TryBendingPCurve(E2,
                             face,
                             p2d1,
                             E2.Orientation() == TopAbs_FORWARD,
                             bendc2,
                             bendf2,
                             bendl2,
                             bendtol2);
    }
    if (!ok1 && !ok2)
      bendc1.Nullify();
  }

  //=============
  // Third phase: analyse how to fix the problem

  // selector of solutions
  Standard_Boolean doIncrease  = Standard_False; // increase tolerance
  Standard_Boolean doAddLong   = Standard_False; // add long 3d edge in replacement of a vertex
  Standard_Boolean doAddClosed = Standard_False; // add closed 3d edge
  Standard_Boolean doAddDegen  = Standard_False; // add degenerated edge
  Standard_Boolean doBend      = Standard_False; //: s2 bend pcurves

  // if bending is OK with existing tolerances of edges, take it
  if (!bendc1.IsNull() && !bendc2.IsNull()
      && ((bendtol1 < BRepInspector::Tolerance(E1) && bendtol2 < BRepInspector::Tolerance(E2))
          || (inctol < Prec && bendtol1 < inctol && bendtol2 < inctol)))
    doBend = Standard_True;

  // is it OK just to increase tolerance (to a value less than preci)?
  else if (inctol < Prec)
    doIncrease = Standard_True;

  // If increase is not OK or force, try to find other solutions (adding edge)
  else if (!BRepInspector::Degenerated(E2) && !BRepInspector::Degenerated(E1))
  {

    // analyze the 3d space btw edges: is it enough to add long 3d edge?
    if (myTopoMode)
    {
      Handle(GeomCurve3d) c3d;
      Standard_Real      a, b;
      if (!sae.Curve3d(E1, c3d, a, b, Standard_True))
      { // cannot work
        myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
        return Standard_False;
      }
      p3d1                    = c3d->Value(b);
      Standard_Real dist2d3d1 = p3d1.Distance(surf->Value(p2d1));
      if (!sae.Curve3d(E2, c3d, a, b, Standard_True))
      { // cannot work
        myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
        return Standard_False;
      }
      p3d2                    = c3d->Value(a);
      Standard_Real dist2d3d2 = p3d2.Distance(surf->Value(p2d2));

      tol1 = Max(BRepInspector::Tolerance(E1), dist2d3d1);
      tol2 = Max(BRepInspector::Tolerance(E2), dist2d3d2);
      //: c5  Standard_Real tol0 = Max ( tol1 + tol2, thepreci );
      Standard_Real tol0    = tol1 + tol2; //: c5 abv 26 Feb 98: CTS17806 #44418
      Standard_Real dist3d2 = p3d1.SquareDistance(p3d2);

      // is it OK to add a long 3d edge?
      if (!myAnalyzer->LastCheckStatus(ShapeExtend_DONE2)
          && //: 81 abv 20 Jan 98: don`t add back-going edges (zigzags)
          dist3d2 > 1.25 * tol0 * tol0
          && (force || dist3d2 > Prec * Prec || inctol > MaxTolerance()))
      {
        doAddLong = Standard_True;
      }
    }

    //: h6 abv 25 Jun 98: BUC40132 6361: try to increase tol up to MaxTol if not add
    if (!doAddLong && inctol < MaxTolerance()
        && !myAnalyzer->Surface()->IsDegenerated(p2d1, p2d2, 2. * tol, 10.))
    { //: p7
      if (!bendc1.IsNull() && !bendc2.IsNull() && bendtol1 < inctol && bendtol2 < inctol)
        doBend = Standard_True;
      else
        doIncrease = Standard_True;
    }
    else

      // else try to add either degenerated or closed edge
      if (!doAddLong)
      {
        Point3d pV = 0.5 * (BRepInspector::Pnt(V1).XYZ() + BRepInspector::Pnt(V2).XYZ());
        Point3d pm = myAnalyzer->Surface()->Value(0.5 * (p2d1.XY() + p2d2.XY()));

        Standard_Real dist = pV.Distance(pm);
        if (dist <= tol)
          doAddDegen = Standard_True;
        else if (myTopoMode)
          doAddClosed = Standard_True;
        else if (dist <= MaxTolerance())
        { //: r7 abv 12 Apr 99: t3d_opt.stp #14245 after S4136
          doAddDegen = Standard_True;
          doIncrease = Standard_True;
          inctol     = dist;
        }
      }
  }

  else if (!BRepInspector::Degenerated(E2) && BRepInspector::Degenerated(E1))
  {
    // create new degenerated edge and replace E1 to new edge
  }
  else if (BRepInspector::Degenerated(E2) && !BRepInspector::Degenerated(E1))
  {
    // create new degenerated edge and replace E2 to new edge
  }

  //=============
  // Third phase - do the fixes
  ShapeBuilder B;

  // add edge
  if (doAddLong || doAddDegen || doAddClosed)
  {

    // construct new vertices
    TopoVertex newV1, newV2;
    if (doAddLong)
    {
      newV1 = BRepBuilderAPI_MakeVertex(p3d1);
      newV1.Reverse();
      newV2 = BRepBuilderAPI_MakeVertex(p3d2);
      B.UpdateVertex(newV1, 1.001 * tol1);
      B.UpdateVertex(newV2, 1.001 * tol2);
    }
    else
    {
      newV1 = V1;
      newV2 = V2;
    }

    // prepare new edge
    TopoEdge edge;
    B.MakeEdge(edge);
    if (doAddDegen)
      B.Degenerated(edge, Standard_True); // sln: do it before adding curve
    gp_Vec2d            v12(p2d1, p2d2);
    Handle(Geom2d_Line) theLine2d = new Geom2d_Line(p2d1, gp_Dir2d(v12));
    B.UpdateEdge(edge, theLine2d, face, ::Precision1::Confusion());
    B.Range(edge, face, 0, dist2d);
    B.Add(edge, newV1.Oriented(TopAbs_FORWARD));
    B.Add(edge, newV2.Oriented(TopAbs_REVERSED));
    Edge2 sbe;
    if (!doAddDegen && !sbe.BuildCurve3d(edge))
    {
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL3);
      return Standard_False;
    }

    // if long edge is added, replace vertices of adjacent edges
    if (doAddLong)
    {

      // replace 1st edge (n1==n2 - special case: wire consists of one edge)
      TopoEdge edge1 = sbe.CopyReplaceVertices(E1, (n1 == n2 ? newV2 : TopoVertex()), newV1);
      sbwd->Set(edge1, n1);
      if (!Context().IsNull())
      {
        Context()->Replace(E1, edge1);
        // actually, this will occur only in context of single face
        // hence, recording to ReShape is rather for tracking modifications
        // than for keeping sharing
        Context()->Replace(V1, newV1.Oriented(V1.Orientation()));
        if (!V1.IsSame(V2))
        {
          Context()->Replace(V2, newV2.Oriented(V2.Orientation()));
        }
      }
      // replace 2nd edge
      if (n1 != n2)
      {
        TopoEdge edge2 = sbe.CopyReplaceVertices(E2, newV2, TopoVertex());
        sbwd->Set(edge2, n2);
        if (!Context().IsNull())
          Context()->Replace(E2, edge2);
      }
      if (!Context().IsNull())
        UpdateWire();
    }

    // insert new edge
    if (doAddDegen)
    {
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE3);
#ifdef OCCT_DEBUG
      std::cout << "Warning: WireHealer::FixLacking: degenerated edge added" << std::endl;
#endif
    }
    else if (!doAddLong)
    {
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE4);
    }
    sbwd->Add(edge, n2);
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
  }

  // else try to increase tol up to MaxTol
  else if (inctol > tol && inctol < MaxTolerance())
  {
    if (!bendc1.IsNull() && !bendc2.IsNull() && bendtol1 < inctol && bendtol2 < inctol)
      doBend = Standard_True;
    else
      doIncrease = Standard_True;
  }

  // bend pcurves
  if (doBend)
  { //: s2 abv 21 Apr 99
    B.UpdateEdge(E1, bendc1, face, bendtol1);
    B.Range(E1, face, bendf1, bendl1);
    B.UpdateEdge(E2, bendc2, face, bendtol2);
    B.Range(E2, face, bendf2, bendl2);
    B.UpdateVertex(sae.FirstVertex(E1), bendtol1);
    B.UpdateVertex(sae.LastVertex(E1), bendtol1);
    B.UpdateVertex(sae.FirstVertex(E2), bendtol2);
    B.UpdateVertex(sae.LastVertex(E2), bendtol2);
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5);
    //: s3 abv 22 Apr 99: PRO7187 #11534: self-intersection not detected unitil curve is bent (!)
    FixSelfIntersectingEdge(n1);
    FixSelfIntersectingEdge(n2);
    FixIntersectingEdges(n2); // skl 24.04.2003 for OCC58
#ifdef OCCT_DEBUG
    std::cout << "Info: WireHealer::FixLacking: Bending pcurves" << std::endl;
#endif
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE5);
  }

  // increase vertex tolerance
  if (doIncrease)
  {
    B.UpdateVertex(V1, 1.001 * inctol);
    B.UpdateVertex(V2, 1.001 * inctol);
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
  }

  if (LastFixStatus(ShapeExtend_DONE))
    return Standard_True;

  myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
  return Standard_False;
}

//=================================================================================================

Standard_Boolean WireHealer::FixNotchedEdges()
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  Handle(ShapeAnalysis_Wire) theAdvAnalyzer = myAnalyzer;
  TopoFace                face           = Face();
  if (!Context().IsNull())
    UpdateWire();
  Handle(ShapeExtend_WireData) sewd = WireData();

  for (Standard_Integer i = 1; i <= NbEdges() && NbEdges() > 2; i++)
  {
    Standard_Real    param;
    Standard_Integer toRemove;
    if (theAdvAnalyzer->CheckNotchedEdges(i, toRemove, param, MinTolerance()))
    {
      Standard_Integer     n2            = (i > 0) ? i : NbEdges();
      Standard_Integer     n1            = (n2 > 1) ? n2 - 1 : NbEdges();
      Standard_Boolean     isRemoveFirst = (n1 == toRemove);
      Standard_Integer     toSplit       = (n2 == toRemove ? n1 : n2);
      TopoEdge          splitE        = sewd->Edge(toSplit);
      Edge1   sae;
      Handle(GeomCurve2d) c2d;
      Standard_Real        a, b;
      sae.PCurve(splitE, face, c2d, a, b, Standard_True);
      Edge2    sbe;
      TopAbs_Orientation orient = splitE.Orientation();

      // check whether the whole edges should be removed - this is the case
      // when split point coincides with the end of the edge;
      // for closed edges split point may fall at the other end (see issue #0029780)
      if (Abs(param - (isRemoveFirst ? b : a)) <= ::Precision1::PConfusion()
          || (sae.IsClosed3d(splitE)
              && Abs(param - (isRemoveFirst ? a : b)) <= ::Precision1::PConfusion()))
      {
        FixDummySeam(n1);
        // The seam edge is removed from the list. So, need to step back to avoid missing of edge
        // processing
        i--;
      }
      else // perform splitting of the edge and adding to wire
      {
        // pdn check if it is necessary
        if (Abs((isRemoveFirst ? a : b) - param) < ::Precision1::PConfusion())
        {
          continue;
        }

        Handle(ShapeAnalysis_TransferParametersProj) transferParameters =
          new ShapeAnalysis_TransferParametersProj;
        transferParameters->SetMaxTolerance(MaxTolerance());
        transferParameters->Init(splitE, face);
        Standard_Real first, last;
        if (a < b)
        {
          first = a;
          last  = b;
        }
        else
        {
          first = b;
          last  = a;
        }
        TopoVertex Vnew;
        ShapeBuilder  B;
        B.MakeVertex(Vnew,
                     Analyzer()->Surface()->Value(c2d->Value(param)),
                     ::Precision1::Confusion());
        TopoEdge wE = splitE;
        wE.Orientation(TopAbs_FORWARD);
        TopoShape aTmpShape = Vnew.Oriented(TopAbs_REVERSED); // for porting
        TopoEdge  newE1 =
          sbe.CopyReplaceVertices(wE, sae.FirstVertex(wE), TopoDS::Vertex(aTmpShape));
        sbe.CopyPCurves(newE1, wE);
        transferParameters->TransferRange(newE1, first, param, Standard_True);
        B.SameRange(newE1, Standard_False);
        B.SameParameter(newE1, Standard_False);
        aTmpShape = Vnew.Oriented(TopAbs_FORWARD);
        TopoEdge newE2 =
          sbe.CopyReplaceVertices(wE, TopoDS::Vertex(aTmpShape), sae.LastVertex(wE));
        sbe.CopyPCurves(newE2, wE);
        transferParameters->TransferRange(newE2, param, last, Standard_True);
        B.SameRange(newE2, Standard_False);
        B.SameParameter(newE2, Standard_False);

        if (!Context().IsNull())
        {
          TopoWire wire;
          B.MakeWire(wire);
          B.Add(wire, newE1);
          B.Add(wire, newE2);
          Context()->Replace(wE, wire);
        }

        newE1.Orientation(orient);
        newE2.Orientation(orient);
        if (orient == TopAbs_REVERSED)
        {
          TopoEdge tmp = newE2;
          newE2           = newE1;
          newE1           = tmp;
        }

        Standard_Boolean isRemoveLast = ((n1 == NbEdges()) && (n2 == 1));
        sewd->Set(newE1, toSplit);
        sewd->Add(newE2, (toSplit == NbEdges() ? 0 : toSplit + 1));

        FixDummySeam(isRemoveLast ? NbEdges() : toRemove);
        myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
      }

      i--;
      if (!Context().IsNull()) // skl 07.03.2002 for OCC180
        UpdateWire();
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
    }
  }
  myStatusNotches = myLastFixStatus;
  return LastFixStatus(ShapeExtend_DONE);
}

//=================================================================================================

static void CopyReversePcurves(const TopoEdge&     toedge,
                               const TopoEdge&     fromedge,
                               const Standard_Boolean reverse)
{
  const TopLoc_Location& fromLoc = fromedge.Location();
  const TopLoc_Location& toLoc   = toedge.Location();
  for (BRep_ListIteratorOfListOfCurveRepresentation fromitcr(
         (*((Handle(BRep_TEdge)*)&fromedge.TShape()))->ChangeCurves());
       fromitcr.More();
       fromitcr.Next())
  {
    Handle(BRep_GCurve) fromGC = Handle(BRep_GCurve)::DownCast(fromitcr.Value());
    if (fromGC.IsNull())
      continue;
    if (fromGC->IsCurveOnSurface())
    {
      Handle(GeomSurface)            surface = fromGC->Surface();
      TopLoc_Location                 L       = fromGC->Location();
      Standard_Boolean                found   = Standard_False;
      BRep_ListOfCurveRepresentation& tolist =
        (*((Handle(BRep_TEdge)*)&toedge.TShape()))->ChangeCurves();
      Handle(BRep_GCurve) toGC;
      for (BRep_ListIteratorOfListOfCurveRepresentation toitcr(tolist); toitcr.More() && !found;
           toitcr.Next())
      {
        toGC = Handle(BRep_GCurve)::DownCast(toitcr.Value());
        if (toGC.IsNull() || !toGC->IsCurveOnSurface() || surface != toGC->Surface()
            || L != toGC->Location())
          continue;
        found = Standard_True;
        break;
      }
      if (!found)
      {
        Standard_Real fp = fromGC->First();
        Standard_Real lp = fromGC->Last();
        toGC             = Handle(BRep_GCurve)::DownCast(fromGC->Copy());
        tolist.Append(toGC);
        Handle(GeomCurve2d) pcurve = Handle(GeomCurve2d)::DownCast(fromGC->PCurve()->Copy());
        if (reverse)
        {
          fp = pcurve->ReversedParameter(fp);
          lp = pcurve->ReversedParameter(lp);
          pcurve->Reverse();
          Standard_Real tmp = fp;
          fp                = lp;
          lp                = tmp;
        }
        // bug OCC209 invalid location of pcurve in the edge after copying
        TopLoc_Location newLoc = (fromLoc * L).Predivided(toLoc);
        toGC->SetRange(fp, lp);
        toGC->PCurve(pcurve);
        toGC->Location(newLoc);
        if (fromGC->IsCurveOnClosedSurface())
        {
          pcurve = fromGC->PCurve2();
          toGC->PCurve2(Handle(GeomCurve2d)::DownCast(pcurve->Copy()));
        }
      }
    }
  }
}

//=================================================================================================

//  Note:    This function temporarily not used, because adress to it in
//           function FixDummySeam() (see below line 2472) not used too
//
// static Standard_Boolean HasNewPCurves(const TopoEdge& toedge,
//				      const TopoEdge& fromedge)
//
//{
//  for (BRep_ListIteratorOfListOfCurveRepresentation fromitcr
//       ((*((Handle(BRep_TEdge)*)&fromedge.TShape()))->ChangeCurves()); fromitcr.More();
//       fromitcr.Next()) {
//    Handle(BRep_GCurve) fromGC = Handle(BRep_GCurve)::DownCast(fromitcr.Value());
//    if ( fromGC.IsNull() ) continue;
//    if ( fromGC->IsCurveOnSurface() ) {
//      Handle(GeomSurface) surface = fromGC->Surface();
//      TopLoc_Location L = fromGC->Location();
//      Standard_Boolean found = Standard_False;
//      BRep_ListOfCurveRepresentation& tolist =
//      (*((Handle(BRep_TEdge)*)&toedge.TShape()))->ChangeCurves(); Handle(BRep_GCurve) toGC; for
//      (BRep_ListIteratorOfListOfCurveRepresentation toitcr (tolist); toitcr.More() && !found;
//      toitcr.Next()) {
//	toGC = Handle(BRep_GCurve)::DownCast(toitcr.Value());
//	if ( toGC.IsNull() || !toGC->IsCurveOnSurface() ||
//	    surface != toGC->Surface() || L != toGC->Location() )  continue;
//	found = Standard_True;
//	break;
//      }
//      if (!found)
//	return Standard_True;
//    }
//  }
//  return Standard_False;
//}

//=================================================================================================

void WireHealer::FixDummySeam(const Standard_Integer num)
{
  Edge1           sae;
  Edge2              sbe;
  Vertex2            sbv;
  Standard_Integer             num1 = (num == NbEdges()) ? 1 : num + 1;
  Handle(ShapeExtend_WireData) sewd = WireData();
  TopoEdge                  E1 = sewd->Edge(num), E2 = sewd->Edge(num1);
  TopoVertex                V1 = sae.FirstVertex(E1), V2 = sae.LastVertex(E2);
  TopoVertex                Vm = sbv.CombineVertex(V1, V2, 1.0001);

  // pnd defining if new pcurves exists
  // pdn Temporary not removed
  //  Standard_Boolean toRemove = !(HasNewPCurves(E1,E2)||HasNewPCurves(E2,E1));
  Standard_Boolean toRemove = Standard_False;

  // creating new edge with pcurves and new vertex
  TopoVertex Vs = sae.FirstVertex(E2);
  if (Vs.IsSame(V1) || Vs.IsSame(V2))
    Vs = Vm;
  TopoEdge newEdge = sbe.CopyReplaceVertices(E2, Vs, Vm);
  CopyReversePcurves(newEdge, E1, E1.Orientation() == E2.Orientation());
  ShapeBuilder B;
  B.SameRange(newEdge, Standard_False);
  B.SameParameter(newEdge, Standard_False);

  if (!Context().IsNull())
  {
    if (toRemove)
    {
      Context()->Remove(E2);
      Context()->Remove(E1);
    }
    else
    {
      Context()->Replace(E2, newEdge);
      Context()->Replace(E1, newEdge.Reversed());
    }
    Context()->Replace(V1, Vm.Oriented(V1.Orientation()));
    Context()->Replace(V2, Vm.Oriented(V2.Orientation()));
  }

  Standard_Integer next  = (num1 == NbEdges()) ? 1 : num1 + 1;
  Standard_Integer prev  = (num > 1) ? num - 1 : NbEdges();
  TopoEdge      prevE = sewd->Edge(prev), nextE = sewd->Edge(next);

  TopoEdge tmpE1 = sbe.CopyReplaceVertices(prevE, TopoVertex(), Vm);
  sewd->Set(tmpE1, prev);
  if (!Context().IsNull())
    Context()->Replace(prevE, tmpE1);

  tmpE1 = sbe.CopyReplaceVertices(nextE, Vm, TopoVertex());
  sewd->Set(tmpE1, next);
  if (!Context().IsNull())
    Context()->Replace(nextE, tmpE1);

  // removing edges from wire
  Standard_Integer n1, n2;
  if (num < num1)
  {
    n1 = num;
    n2 = num1;
  }
  else
  {
    n1 = num1;
    n2 = num;
  }
  sewd->Remove(n2);
  sewd->Remove(n1);
}

//=================================================================================================

void WireHealer::UpdateWire()
{
  Handle(ShapeExtend_WireData) sbwd = WireData();
  for (Standard_Integer i = 1; i <= sbwd->NbEdges(); i++)
  {
    TopoEdge  E = sbwd->Edge(i);
    TopoShape S = Context()->Apply(E);
    if (S == E)
      continue;
    for (ShapeExplorer exp(S, TopAbs_EDGE); exp.More(); exp.Next())
      sbwd->Add(exp.Current(), i++);
    sbwd->Remove(i--);
  }
}

//=================================================================================================

Standard_Boolean WireHealer::FixTails()
{
  if (myMaxTailWidth < 0 || !IsReady())
  {
    return Standard_False;
  }

  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!Context().IsNull())
  {
    UpdateWire();
  }
  Handle(ShapeExtend_WireData) aSEWD   = WireData();
  Standard_Integer             aECount = NbEdges(), aENs[] = {aECount, 1};
  Standard_Boolean             aCheckAngle = Standard_True;
  while (aECount >= 2 && aENs[1] <= aECount)
  {
    const TopoEdge aEs[] = {aSEWD->Edge(aENs[0]), aSEWD->Edge(aENs[1])};
    TopoEdge       aEParts[2][2];
    if (!myAnalyzer->CheckTail(aEs[0],
                               aEs[1],
                               aCheckAngle ? myMaxTailAngleSine : -1,
                               myMaxTailWidth,
                               MaxTolerance(),
                               aEParts[0][0],
                               aEParts[0][1],
                               aEParts[1][0],
                               aEParts[1][1]))
    {
      aENs[0]     = aENs[1]++;
      aCheckAngle = Standard_True;
      continue;
    }

    // Provide not less than 1 edge in the result wire.
    Standard_Integer aSplitCounts[] = {aEParts[0][1].IsNull() ? 0 : 1,
                                       aEParts[1][1].IsNull() ? 0 : 1};
    const Standard_Integer aRemoveCount =
      (aEParts[0][0].IsNull() ? 0 : 1) + (aEParts[1][0].IsNull() ? 0 : 1);
    if (aECount + aSplitCounts[0] + aSplitCounts[1] < 1 + aRemoveCount)
    {
      aENs[0]     = aENs[1]++;
      aCheckAngle = Standard_True;
      continue;
    }

    // Split the edges.
    for (Standard_Integer aEI = 0; aEI < 2; ++aEI)
    {
      if (aSplitCounts[aEI] == 0)
      {
        continue;
      }

      // Replace the edge by the wire of its parts in the shape.
      const TopoEdge aE = aEs[aEI];
      if (!Context().IsNull())
      {
        TopoWire aEWire;
        ShapeBuilder().MakeWire(aEWire);
        ShapeBuilder().Add(aEWire, aEParts[aEI][0]);
        ShapeBuilder().Add(aEWire, aEParts[aEI][1]);
        TopoEdge aFE = TopoDS::Edge(aE.Oriented(TopAbs_FORWARD));
        Context()->Replace(aFE, aEWire);
      }

      // Replace the edge by its parts in the edge wire.
      const TopAbs_Orientation aOrient = aE.Orientation();
      aEParts[aEI][0].Orientation(aOrient);
      aEParts[aEI][1].Orientation(aOrient);
      const Standard_Integer aFirstPI = (aOrient != TopAbs_REVERSED) ? 0 : 1;
      const Standard_Integer aAdd     = (aEI == 0 || aENs[1] < aENs[0]) ? 0 : aSplitCounts[0];
      aSEWD->Set(aEParts[aEI][aFirstPI], aENs[aEI] + aAdd);
      aSEWD->Add(aEParts[aEI][1 - aFirstPI], aENs[aEI] + 1 + aAdd);
    }

    // Remove the tail.
    if (aRemoveCount == 2)
    {
      aCheckAngle = Standard_True;
      FixDummySeam(aENs[0] + aSplitCounts[0] + ((aENs[0] < aENs[1]) ? 0 : aSplitCounts[1]));
      if (!Context().IsNull())
      {
        UpdateWire();
      }
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE);

      if (aSplitCounts[0] + aSplitCounts[1] == 2)
      {
        aENs[0] = aENs[1]++;
        continue;
      }

      if (aSplitCounts[0] == aSplitCounts[1])
      {
        aECount -= 2;
        if (aENs[1] >= 3)
        {
          --aENs[0];
          --aENs[1];
        }
        else
        {
          aENs[0] = aECount;
          aENs[1] = 1;
        }
        aCheckAngle = Standard_False;
      }
      else
      {
        --aECount;
        if (aSplitCounts[0] != 0)
        {
          aENs[0] = (aENs[0] <= aECount) ? aENs[0] : aECount;
        }
        else
        {
          if (aENs[1] >= 3)
          {
            --aENs[0];
            --aENs[1];
          }
          else
          {
            aENs[0] = aECount;
            aENs[1] = 1;
          }
        }
      }
    }
    else
    {
      aCheckAngle = Standard_False;
      --aECount;
      const Standard_Integer aRI = aEParts[0][0].IsNull() ? 1 : 0;
      if (aSplitCounts[aRI] != 0)
      {
        if (aRI == 0)
        {
          if (aENs[1] >= 3)
          {
            --aENs[0];
            --aENs[1];
          }
          else
          {
            aENs[0] = aECount;
            aENs[1] = 1;
          }
        }
        else
        {
          aENs[0] = (aENs[1] > 1) ? aENs[0] : aECount;
        }
      }
      aSEWD->Remove(aENs[aRI] + ((aRI != 0 || aSplitCounts[0] == 0) ? 0 : 1));
      if (!Context().IsNull())
      {
        Context()->Remove(aEs[aRI].Oriented(TopAbs_FORWARD));
        UpdateWire();
      }
      myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE);
    }
  }
  myStatusNotches = myLastFixStatus;
  return ShapeExtend1::DecodeStatus(myLastFixStatus, ShapeExtend_DONE);
}
