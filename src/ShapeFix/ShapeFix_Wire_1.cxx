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

// szv 19.08.99: new methods for fixing gaps between edges (3d curves and pcurves)
#include <ShapeFix_Wire.hxx>
#include <Standard_Failure.hxx>

#include <Precision.hxx>
#include <Geom2d_Line.hxx>

#include <IntRes2d_IntersectionPoint.hxx>

#include <TopoDS.hxx>

#include <BRep_Builder.hxx>

#include <ShapeExtend.hxx>
#include <ShapeBuild_Edge.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Pln.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <ShapeBuild_ReShape.hxx>

// szv
#include <Geom2d_Circle.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_Parabola.hxx>
#include <Geom2d_Hyperbola.hxx>
#include <Geom2d_OffsetCurve.hxx>
#include <Geom2dInt_GInter.hxx>
#include <IntRes2d_Domain.hxx>
#include <IntRes2d_IntersectionSegment.hxx>
#include <Geom2dAPI_ExtremaCurveCurve.hxx>
#include <Geom2dAPI_ProjectPointOnCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Approx_Curve2d.hxx>
#include <Geom2dConvert.hxx>

#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_OffsetCurve.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Approx_Curve3d.hxx>
#include <GeomConvert.hxx>
#include <TopoDS_Iterator.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <ShapeAnalysis_TransferParametersProj.hxx>
#include <Geom_Conic.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom2d_Conic.hxx>
#include <Geom2d_BezierCurve.hxx>

//=================================================================================================

Standard_Boolean WireHealer::FixGaps3d()
{
  myStatusGaps3d = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  // if ( !IsReady() ) return Standard_False;

  Standard_Integer i, start = (myClosedMode ? 1 : 2);
  if (myFixGapsByRanges)
  {
    for (i = start; i <= NbEdges(); i++)
    {
      FixGap3d(i);
      myStatusGaps3d |= myLastFixStatus;
    }
  }
  for (i = start; i <= NbEdges(); i++)
  {
    FixGap3d(i, Standard_True);
    myStatusGaps3d |= myLastFixStatus;
  }

  return StatusGaps3d(ShapeExtend_DONE);
}

//=================================================================================================

Standard_Boolean WireHealer::FixGaps2d()
{
  myStatusGaps2d = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  //  if ( !IsReady() ) return Standard_False;

  Standard_Integer i, start = (myClosedMode ? 1 : 2);
  if (myFixGapsByRanges)
  {
    for (i = start; i <= NbEdges(); i++)
    {
      FixGap2d(i);
      myStatusGaps2d |= myLastFixStatus;
    }
  }
  for (i = start; i <= NbEdges(); i++)
  {
    FixGap2d(i, Standard_True);
    myStatusGaps2d |= myLastFixStatus;
  }

  return StatusGaps2d(ShapeExtend_DONE);
}

//=================================================================================================

static Standard_Real AdjustOnPeriodic3d(const Handle(GeomCurve3d)& c,
                                        const Standard_Boolean    takefirst,
                                        const Standard_Real       first,
                                        const Standard_Real       last,
                                        const Standard_Real       param)
{
  // 15.11.2002 PTV OCC966
  if (Curve2::IsPeriodic(c))
  {
    Standard_Real T     = c->Period();
    Standard_Real shift = -IntegerPart(first / T) * T;
    if (first < 0.)
      shift += T;
    Standard_Real sfirst = first + shift, slast = last + shift;
    if (takefirst && (param > slast) && (param > sfirst))
      return param - T - shift;
    if (!takefirst && (param < slast) && (param < sfirst))
      return param + T - shift;
  }
  return param;
}

Standard_Boolean WireHealer::FixGap3d(const Standard_Integer num, const Standard_Boolean convert)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  //  if ( !IsReady() ) return Standard_False;

  //=============
  // First phase: analysis whether the problem (gap) exists
  //=============

  if (Context().IsNull())
    SetContext(new ShapeBuild_ReShape);

  Standard_Real preci = Precision1();

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             n2   = (num > 0 ? num : sbwd->NbEdges());
  Standard_Integer             n1   = (n2 > 1 ? n2 - 1 : sbwd->NbEdges());
  // smh#8
  TopoShape tmp1 = Context()->Apply(sbwd->Edge(n1)), tmp2 = Context()->Apply(sbwd->Edge(n2));
  TopoEdge  E1 = TopoDS::Edge(tmp1), E2 = TopoDS::Edge(tmp2);
  //  TopoFace face = myAnalyzer->Face(); // comment by enk

  // Retrieve curves on edges
  Standard_Real      cfirst1, clast1, cfirst2, clast2;
  Handle(GeomCurve3d) C1, C2;
  Edge1 SAE;
  if (!SAE.Curve3d(E1, C1, cfirst1, clast1) || !SAE.Curve3d(E2, C2, cfirst2, clast2))
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
    return Standard_False;
  }

  // Check gap in 3d space
  Point3d        cpnt1 = C1->Value(clast1), cpnt2 = C2->Value(cfirst2);
  Standard_Real gap = cpnt1.Distance(cpnt2);
  if (!convert && gap <= preci)
    return Standard_False;

  //=============
  // Second phase: collecting data necessary for further analysis
  //=============

  Standard_Boolean reversed1 = (E1.Orientation() == TopAbs_REVERSED),
                   reversed2 = (E2.Orientation() == TopAbs_REVERSED);

  TopoVertex V1 = SAE.LastVertex(E1), V2 = SAE.FirstVertex(E2);
  Point3d        vpnt = (V1.IsSame(V2))
                         ? BRepInspector::Pnt(V1)
                         : Point3d((BRepInspector::Pnt(V1).XYZ() + BRepInspector::Pnt(V2).XYZ()) * 0.5);

  Standard_Real first1, last1, first2, last2;
  if (reversed1)
  {
    first1 = clast1;
    last1  = cfirst1;
  }
  else
  {
    first1 = cfirst1;
    last1  = clast1;
  }
  if (reversed2)
  {
    first2 = clast2;
    last2  = cfirst2;
  }
  else
  {
    first2 = cfirst2;
    last2  = clast2;
  }

  Handle(GeomCurve3d) c1 = C1, c2 = C2;

  // Extract basic curves from trimmed and offset
  Standard_Boolean basic    = Standard_False;
  Standard_Boolean trimmed1 = Standard_False, offset1 = Standard_False;
  Coords3d           offval1(0., 0., 0.);
  while (!basic)
  {
    if (c1->IsKind(STANDARD_TYPE(Geom_TrimmedCurve)))
    {
      c1       = Handle(Geom_TrimmedCurve)::DownCast(c1)->BasisCurve();
      trimmed1 = Standard_True;
    }
    else if (c1->IsKind(STANDARD_TYPE(Geom_OffsetCurve)))
    {
      Handle(Geom_OffsetCurve) oc = Handle(Geom_OffsetCurve)::DownCast(c1);
      c1                          = oc->BasisCurve();
      offval1 += oc->Offset() * oc->Direction().XYZ();
      offset1 = Standard_True;
    }
    else
      basic = Standard_True;
  }
  basic                     = Standard_False;
  Standard_Boolean trimmed2 = Standard_False, offset2 = Standard_False;
  Coords3d           offval2(0., 0., 0.);
  while (!basic)
  {
    if (c2->IsKind(STANDARD_TYPE(Geom_TrimmedCurve)))
    {
      c2       = Handle(Geom_TrimmedCurve)::DownCast(c2)->BasisCurve();
      trimmed2 = Standard_True;
    }
    else if (c2->IsKind(STANDARD_TYPE(Geom_OffsetCurve)))
    {
      Handle(Geom_OffsetCurve) oc = Handle(Geom_OffsetCurve)::DownCast(c2);
      c2                          = oc->BasisCurve();
      offval2 += oc->Offset() * oc->Direction().XYZ();
      offset2 = Standard_True;
    }
    else
      basic = Standard_True;
  }
  // Restore offset curves
  if (offset1)
    c1 = new Geom_OffsetCurve(c1, offval1.Modulus(), Dir3d(offval1));
  if (offset2)
    c2 = new Geom_OffsetCurve(c2, offval2.Modulus(), Dir3d(offval2));

  Standard_Boolean done1 = Standard_False, done2 = Standard_False;

  if (convert)
  {
    // Check that gap satisfies the precision - in this case no conversion produced
    if (cpnt1.Distance(vpnt) < preci && cpnt2.Distance(vpnt) < preci)
      return Standard_False;

    Handle(BSplineCurve3d) bsp1, bsp2;
    Handle(GeomCurve3d)        c;
    Standard_Real             first, last;

    // iterate on curves
    Standard_Integer nbcurv = (n1 == n2 ? 1 : 2);
    for (Standard_Integer j = 1; j <= nbcurv; j++)
    {
      // Standard_Boolean trim = Standard_False;  // skl
      if (j == 1)
      {
        if (cpnt1.Distance(vpnt) < preci)
        {
          if (n1 == n2)
          {
            if (cpnt2.Distance(vpnt) < preci)
              continue;
          }
          else
            continue;
        }
        c     = c1;
        first = first1;
        last  = last1; /*trim = trimmed1;*/ // skl
      }
      else
      {
        if (cpnt2.Distance(vpnt) < preci)
          continue;
        c     = c2;
        first = first2;
        last  = last2; /*trim = trimmed2;*/ // skl
      }

      Handle(BSplineCurve3d) bsp;

      // Convert curve to bspline
      if (c->IsKind(STANDARD_TYPE(BSplineCurve3d)))
      {
        bsp = Handle(BSplineCurve3d)::DownCast(c->Copy());
        // take segment if trim and range differ
        Standard_Real    fbsp = bsp->FirstParameter(), lbsp = bsp->LastParameter();
        Standard_Boolean segment = Standard_False;
        if (first > fbsp)
        {
          fbsp    = first;
          segment = Standard_True;
        }
        if (last < lbsp)
        {
          lbsp    = last;
          segment = Standard_True;
        }
        if (segment)
          bsp = GeomConvert1::SplitBSplineCurve(bsp, fbsp, lbsp, ::Precision1::Confusion());
      }
      else if (c->IsKind(STANDARD_TYPE(Geom_Conic)))
      {
        Approx_Curve3d Conv(new GeomAdaptor_Curve(c, first, last),
                            myAnalyzer->Precision1(),
                            GeomAbs_C1,
                            9,
                            1000);
        if (Conv.IsDone() || Conv.HasResult())
          bsp = Conv.Curve();
      }
      else
      {
        // Restore trim for pcurve
        Handle(GeomCurve3d) tc;
        try
        {
          OCC_CATCH_SIGNALS
          // 15.11.2002 PTV OCC966
          if (!Curve2::IsPeriodic(c))
            tc = new Geom_TrimmedCurve(c,
                                       Max(first, c->FirstParameter()),
                                       Min(last, c->LastParameter()));
          else
            tc = new Geom_TrimmedCurve(c, first, last);
          bsp = GeomConvert1::CurveToBSplineCurve(tc);
        }
        catch (ExceptionBase const& anException)
        {
#ifdef OCCT_DEBUG
          std::cout << "Warning: ShapeFix_Wire_1::FixGap3d: Exception in TrimmedCurve" << first
                    << " " << last << std::endl;
          anException.Print(std::cout);
          std::cout << std::endl;
#endif
          (void)anException;
        }
      }

      if (j == 1)
        bsp1 = bsp;
      else
        bsp2 = bsp;
    }

    // Take curves ends if could not convert
    if (bsp1.IsNull())
      vpnt = cpnt1;
    else if (bsp2.IsNull())
      vpnt = cpnt2;

    if (!bsp1.IsNull())
    {
      if (bsp1->Degree() == 1)
        bsp1->IncreaseDegree(2); // gka
      if (n1 == n2)
      {
        bsp1->SetPole(1, vpnt);
        bsp1->SetPole(bsp1->NbPoles(), vpnt);
      }
      else
      {
        if (reversed1)
          bsp1->SetPole(1, vpnt);
        else
          bsp1->SetPole(bsp1->NbPoles(), vpnt);
      }
      first1 = bsp1->FirstParameter();
      last1  = bsp1->LastParameter();
      c1     = bsp1;
      done1  = Standard_True;
    }
    if (!bsp2.IsNull())
    {
      if (bsp2->Degree() == 1)
        bsp2->IncreaseDegree(2); // gka
      if (reversed2)
        bsp2->SetPole(bsp2->NbPoles(), vpnt);
      else
        bsp2->SetPole(1, vpnt);
      first2 = bsp2->FirstParameter();
      last2  = bsp2->LastParameter();
      c2     = bsp2;
      done2  = Standard_True;
    }
  }
  else
  {

    if (n1 == n2)
    {
      if (c1->IsKind(STANDARD_TYPE(GeomCircle)) || c1->IsKind(STANDARD_TYPE(Geom_Ellipse)))
      {
        Standard_Real diff = M_PI - Abs(clast1 - cfirst2) * 0.5;
        first1 -= diff;
        last1 += diff;
        done1 = Standard_True;
      }
    }
    else
    {

      // Determine domains for extremal points locating
      Standard_Real domfirst1 = first1, domlast1 = last1;
      if (c1->IsKind(STANDARD_TYPE(BSplineCurve3d))
          || c1->IsKind(STANDARD_TYPE(BezierCurve3d)))
      {
        domfirst1 = c1->FirstParameter();
        domlast1  = c1->LastParameter();
      }
      else if (c1->IsKind(STANDARD_TYPE(GeomLine)) || c1->IsKind(STANDARD_TYPE(Geom_Parabola))
               || c1->IsKind(STANDARD_TYPE(Geom_Hyperbola)))
      {
        Standard_Real diff = domlast1 - domfirst1;
        if (reversed1)
          domfirst1 -= 10. * diff;
        else
          domlast1 += 10. * diff;
      }
      else if (c1->IsKind(STANDARD_TYPE(GeomCircle)) || c1->IsKind(STANDARD_TYPE(Geom_Ellipse)))
      {
        domfirst1 = 0.;
        domlast1  = 2 * M_PI;
      }
      Standard_Real domfirst2 = first2, domlast2 = last2;
      if (c2->IsKind(STANDARD_TYPE(BSplineCurve3d))
          || c2->IsKind(STANDARD_TYPE(BezierCurve3d)))
      {
        domfirst2 = c2->FirstParameter();
        domlast2  = c2->LastParameter();
      }
      else if (c2->IsKind(STANDARD_TYPE(GeomLine)) || c2->IsKind(STANDARD_TYPE(Geom_Parabola))
               || c2->IsKind(STANDARD_TYPE(Geom_Hyperbola)))
      {
        Standard_Real diff = domlast2 - domfirst2;
        if (reversed2)
          domlast2 += 10. * diff;
        else
          domfirst2 -= 10. * diff;
      }
      else if (c2->IsKind(STANDARD_TYPE(GeomCircle)) || c2->IsKind(STANDARD_TYPE(Geom_Ellipse)))
      {
        domfirst2 = 0.;
        domlast2  = 2 * M_PI;
      }

      Standard_Real ipar1 = clast1, ipar2 = cfirst2;

      // Try to find projections of vertex point
      GeomAPI_ProjectPointOnCurve Proj;
      Standard_Real               u1 = ipar1, u2 = ipar2;
      Proj.Init(vpnt, c1, domfirst1, domlast1);
      if (Proj.NbPoints())
      {
        Standard_Integer index = 1;
        Standard_Real    dist, mindist = -1.;
        for (Standard_Integer i = 1; i <= Proj.NbPoints(); i++)
        {
          dist = vpnt.Distance(Proj.Point(i));
          if (mindist > dist || mindist < 0.)
          {
            index   = i;
            mindist = dist;
          }
          u1 = Proj.Parameter(index);
        }
      }
      Proj.Init(vpnt, c2, domfirst2, domlast2);
      if (Proj.NbPoints())
      {
        Standard_Integer index = 1;
        Standard_Real    dist, mindist = -1.;
        for (Standard_Integer i = 1; i <= Proj.NbPoints(); i++)
        {
          dist = vpnt.Distance(Proj.Point(i));
          if (mindist > dist || mindist < 0.)
          {
            index   = i;
            mindist = dist;
          }
          u2 = Proj.Parameter(index);
        }
      }
      // Adjust parameters on periodic curves
      u1 = AdjustOnPeriodic3d(c1, reversed1, first1, last1, u1);
      u2 = AdjustOnPeriodic3d(c2, !reversed2, first2, last2, u2);
      // Check points to satisfy distance criterium
      Point3d p1 = c1->Value(u1), p2 = c2->Value(u2);
      if (p1.Distance(p2) <= gap && Abs(cfirst1 - u1) > ::Precision1::PConfusion()
          && Abs(clast2 - u2) > ::Precision1::PConfusion()
          && (((u1 > first1) && (u1 < last1)) || ((u2 > first2) && (u2 < last2))
              || (cpnt1.Distance(p1) <= gap) || (cpnt2.Distance(p2) <= gap)))
      {
        ipar1 = u1;
        ipar2 = u2;
        done1 = done2 = Standard_True;
      }

      // Try to find closest points if nothing yet found
      if (!done1)
      {

        // Recompute domains
        if (reversed1)
        {
          domfirst1 = ipar1;
          domlast1  = last1;
        }
        else
        {
          domfirst1 = first1;
          domlast1  = ipar1;
        }
        if (reversed2)
        {
          domfirst2 = first2;
          domlast2  = ipar2;
        }
        else
        {
          domfirst2 = ipar2;
          domlast2  = last2;
        }

        GeomAPI_ExtremaCurveCurve Extr(c1, c2, domfirst1, domlast1, domfirst2, domlast2);
        if (Extr.NbExtrema())
        {
          try
          {
            OCC_CATCH_SIGNALS
            // First find all intersections
            Point3d           pp1, pp2;
            Standard_Integer index1 = 0, index2 = 0;
            Standard_Real    uu1, uu2, pardist, pardist1 = -1., pardist2 = -1.;
            for (Standard_Integer i = 1; i <= Extr.NbExtrema(); i++)
            {
              Extr.Parameters(i, uu1, uu2);
              // Adjust parameters on periodic curves
              uu1 = AdjustOnPeriodic3d(c1, reversed1, first1, last1, uu1);
              uu2 = AdjustOnPeriodic3d(c2, !reversed2, first2, last2, uu2);
              pp1 = c1->Value(uu1);
              pp2 = c2->Value(uu2);
              if (pp1.Distance(pp2) < ::Precision1::Confusion())
              {
                // assume intersection
                pardist = Abs(cfirst1 - uu1);
                if (pardist1 > pardist || pardist1 < 0.)
                {
                  index1   = i;
                  pardist1 = pardist;
                }
                pardist = Abs(clast2 - uu2);
                if (pardist2 > pardist || pardist2 < 0.)
                {
                  index2   = i;
                  pardist2 = pardist;
                }
              }
            }
            if (index1 != 0 && index2 != 0)
            {
              if (index1 != index2)
              {
                // take intersection closer to vertex point
                Extr.Parameters(index1, uu1, uu2);
                pp1 = Point3d((c1->Value(uu1).XYZ() + c2->Value(uu2).XYZ()) * 0.5);
                Extr.Parameters(index2, uu1, uu2);
                pp2 = Point3d((c1->Value(uu1).XYZ() + c2->Value(uu2).XYZ()) * 0.5);
                if (pp2.Distance(vpnt) < pp1.Distance(vpnt))
                  index1 = index2;
              }
              Extr.Parameters(index1, uu1, uu2);
            }
            else
              Extr.LowerDistanceParameters(uu1, uu2);
            // Adjust parameters on periodic curves
            uu1 = AdjustOnPeriodic3d(c1, reversed1, first1, last1, uu1);
            uu2 = AdjustOnPeriodic3d(c2, !reversed2, first2, last2, uu2);
            // Check points to satisfy distance criterium
            pp1 = c1->Value(uu1), pp2 = c2->Value(uu2);
            if (pp1.Distance(pp2) <= gap && Abs(cfirst1 - uu1) > ::Precision1::PConfusion()
                && Abs(clast2 - uu2) > ::Precision1::PConfusion()
                && (((uu1 > first1) && (uu1 < last1)) || ((uu2 > first2) && (uu2 < last2))
                    || (cpnt1.Distance(pp1) <= gap) || (cpnt2.Distance(pp2) <= gap)))
            {
              ipar1 = uu1;
              ipar2 = uu2;
              done1 = done2 = Standard_True;
            }
          }
          catch (ExceptionBase const&)
          {
          }
        }
      }

      try
      {
        OCC_CATCH_SIGNALS
        if (done1)
        {
          if (ipar1 == clast1)
            done1 = Standard_False;
          else
          {
            // Set up new bounds for curve
            if (reversed1)
              first1 = ipar1;
            else
              last1 = ipar1;
            // Set new trim for old curve
            if (trimmed1)
            {
              // Standard_Real ff1 = c1->FirstParameter();
              // Standard_Real ll1 = c1->LastParameter();
              c1 = new Geom_TrimmedCurve(c1, first1, last1);
            }
          }
        }
        if (done2)
        {
          if (ipar2 == cfirst2)
            done2 = Standard_False;
          else
          {
            // Set up new bounds for curve
            if (reversed2)
              last2 = ipar2;
            else
              first2 = ipar2;
            // Set new trim for old curve
            if (trimmed2)
            {
              // Standard_Real ff2 = c2->FirstParameter();
              // Standard_Real ll2 = c2->LastParameter();
              c2 = new Geom_TrimmedCurve(c2, first2, last2);
            }
          }
        }
      }
      catch (ExceptionBase const& anException)
      {
#ifdef OCCT_DEBUG
        std::cout << "Warning: ShapeFix_Wire_1::FixGap3d: Exception in TrimmedCurve      :"
                  << std::endl;
        anException.Print(std::cout);
        std::cout << std::endl;
#endif
        (void)anException;
      }
    }
  }

  if (done1 || done2)
  {

    ShapeBuilder            B;
    Edge2         SBE;
    ShapeTolerance1 SFST;

    // Update vertices
    TopoVertex nullV, newV1;
    // smh#8
    TopoShape  emptyCopiedV2 = V2.EmptyCopied();
    TopoVertex newV2         = TopoDS::Vertex(emptyCopiedV2);
    SFST.SetTolerance(newV2, ::Precision1::Confusion());
    Context()->Replace(V2, newV2);
    if (V1.IsSame(V2))
    // smh#8
    {
      TopoShape tmpV2 = newV2.Oriented(TopAbs_REVERSED);
      newV1              = TopoDS::Vertex(tmpV2);
    }
    else
    {
      // smh#8
      TopoShape emptyCopied = V1.EmptyCopied();
      newV1                    = TopoDS::Vertex(emptyCopied);
      SFST.SetTolerance(newV1, ::Precision1::Confusion());
      Context()->Replace(V1, newV1);
    }

    if (done1)
    {
      // Update first edge
      TopoEdge newE1 = SBE.CopyReplaceVertices(E1, nullV, newV1);
      // smh#8
      TopoShape tmpE1 = newE1.Oriented(TopAbs_FORWARD);
      B.UpdateEdge(TopoDS::Edge(tmpE1), c1, 0.);
      SBE.SetRange3d(TopoDS::Edge(tmpE1), first1, last1);
      SFST.SetTolerance(newE1, ::Precision1::Confusion(), TopAbs_EDGE);
      B.SameRange(newE1, Standard_False);
      //      B.SameParameter(newE1,Standard_False);

      // To keep NM vertices belonging initial edges
      TopoDS_Iterator aItv(E1, Standard_False);
      for (; aItv.More(); aItv.Next())
      {
        if (aItv.Value().Orientation() == TopAbs_INTERNAL
            || aItv.Value().Orientation() == TopAbs_EXTERNAL)
        {
          TopoVertex aOldV = TopoDS::Vertex(aItv.Value());
          TopoVertex anewV =
            ShapeAnalysis_TransferParametersProj::CopyNMVertex(aOldV, newE1, E1);
          B.Add(newE1, anewV);
          Context()->Replace(aOldV, anewV);
        }
      }

      Context()->Replace(E1, newE1);
      sbwd->Set(newE1, n1);
    }

    if (done2)
    {
      // Update second edge
      TopoEdge newE2 = SBE.CopyReplaceVertices(E2, newV2, nullV);
      // smh#8
      TopoShape tmpE2 = newE2.Oriented(TopAbs_FORWARD);
      B.UpdateEdge(TopoDS::Edge(tmpE2), c2, 0.);
      SBE.SetRange3d(TopoDS::Edge(tmpE2), first2, last2);
      SFST.SetTolerance(newE2, ::Precision1::Confusion(), TopAbs_EDGE);
      B.SameRange(newE2, Standard_False);
      //      B.SameParameter(newE2,Standard_False);

      // To keep NM vertices belonging initial edges
      TopoDS_Iterator aItv(E2, Standard_False);
      for (; aItv.More(); aItv.Next())
      {
        if (aItv.Value().Orientation() == TopAbs_INTERNAL
            || aItv.Value().Orientation() == TopAbs_EXTERNAL)
        {
          TopoVertex aOldV = TopoDS::Vertex(aItv.Value());
          TopoVertex anewV =
            ShapeAnalysis_TransferParametersProj::CopyNMVertex(aOldV, newE2, E2);
          B.Add(newE2, anewV);
          Context()->Replace(aOldV, anewV);
        }
      }
      Context()->Replace(E2, newE2);
      sbwd->Set(newE2, n2);
    }

    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
  }
  else if (convert)
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);

  return (done1 || done2);
}

//=================================================================================================

static Standard_Real AdjustOnPeriodic2d(const Handle(GeomCurve2d)& pc,
                                        const Standard_Boolean      takefirst,
                                        const Standard_Real         first,
                                        const Standard_Real         last,
                                        const Standard_Real         param)
{
  // 15.11.2002 PTV OCC966
  if (Curve2::IsPeriodic(pc))
  {
    Standard_Real T     = pc->Period();
    Standard_Real shift = -IntegerPart(first / T) * T;
    if (first < 0.)
      shift += T;
    Standard_Real sfirst = first + shift, slast = last + shift;
    if (takefirst && (param > slast) && (param > sfirst))
      return param - T - shift;
    if (!takefirst && (param < slast) && (param < sfirst))
      return param + T - shift;
  }
  return param;
}

Standard_Boolean WireHealer::FixGap2d(const Standard_Integer num, const Standard_Boolean convert)
{
  myLastFixStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  if (!IsReady())
    return Standard_False;

  //=============
  // First phase: analysis whether the problem (gap) exists
  //=============

  if (Context().IsNull())
    SetContext(new ShapeBuild_ReShape);

  constexpr Standard_Real preci = ::Precision1::PConfusion();
  // Standard_Real preci = Precision1();
  // GeomAdaptor_Surface& SA = Analyzer().Surface()->Adaptor()->ChangeSurface();
  // preci = Max(SA.UResolution(preci), SA.VResolution(preci));

  Handle(ShapeExtend_WireData) sbwd = WireData();
  Standard_Integer             n2   = (num > 0 ? num : sbwd->NbEdges());
  Standard_Integer             n1   = (n2 > 1 ? n2 - 1 : sbwd->NbEdges());
  // smh#8
  TopoShape tmp1 = Context()->Apply(sbwd->Edge(n1)), tmp2 = Context()->Apply(sbwd->Edge(n2));
  TopoEdge  E1 = TopoDS::Edge(tmp1), E2 = TopoDS::Edge(tmp2);
  TopoFace  face = myAnalyzer->Face();

  // Retrieve pcurves on edges
  Standard_Real        cfirst1, clast1, cfirst2, clast2;
  Handle(GeomCurve2d) PC1, PC2;
  Edge1   SAE;
  if (!SAE.PCurve(E1, face, PC1, cfirst1, clast1) || !SAE.PCurve(E2, face, PC2, cfirst2, clast2)
      || sbwd->IsSeam(n1) || sbwd->IsSeam(n2))
  {
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
    return Standard_False;
  }

  // Check gap in 2d space
  gp_Pnt2d      cpnt1 = PC1->Value(clast1), cpnt2 = PC2->Value(cfirst2);
  Standard_Real gap = cpnt1.Distance(cpnt2);
  if (gap <= preci)
    return Standard_False;

  //=============
  // Second phase: collecting data necessary for further analysis
  //=============

  Standard_Boolean reversed1 = (E1.Orientation() == TopAbs_REVERSED),
                   reversed2 = (E2.Orientation() == TopAbs_REVERSED);

  Standard_Real first1, last1, first2, last2;
  if (reversed1)
  {
    first1 = clast1;
    last1  = cfirst1;
  }
  else
  {
    first1 = cfirst1;
    last1  = clast1;
  }
  if (reversed2)
  {
    first2 = clast2;
    last2  = cfirst2;
  }
  else
  {
    first2 = cfirst2;
    last2  = clast2;
  }

  Handle(GeomCurve2d) pc1 = PC1, pc2 = PC2;

  // Extract basic curves from trimmed and offset
  Standard_Boolean basic    = Standard_False;
  Standard_Boolean trimmed1 = Standard_False, offset1 = Standard_False;
  Standard_Real    offval1 = 0.;
  while (!basic)
  {
    if (pc1->IsKind(STANDARD_TYPE(Geom2d_TrimmedCurve)))
    {
      pc1      = Handle(Geom2d_TrimmedCurve)::DownCast(pc1)->BasisCurve();
      trimmed1 = Standard_True;
    }
    else if (pc1->IsKind(STANDARD_TYPE(Geom2d_OffsetCurve)))
    {
      Handle(Geom2d_OffsetCurve) oc = Handle(Geom2d_OffsetCurve)::DownCast(pc1);
      pc1                           = oc->BasisCurve();
      offval1 += oc->Offset();
      offset1 = Standard_True;
    }
    else
      basic = Standard_True;
  }
  basic                     = Standard_False;
  Standard_Boolean trimmed2 = Standard_False, offset2 = Standard_False;
  Standard_Real    offval2 = 0.;
  while (!basic)
  {
    if (pc2->IsKind(STANDARD_TYPE(Geom2d_TrimmedCurve)))
    {
      pc2      = Handle(Geom2d_TrimmedCurve)::DownCast(pc2)->BasisCurve();
      trimmed2 = Standard_True;
    }
    else if (pc2->IsKind(STANDARD_TYPE(Geom2d_OffsetCurve)))
    {
      Handle(Geom2d_OffsetCurve) oc = Handle(Geom2d_OffsetCurve)::DownCast(pc2);
      pc2                           = oc->BasisCurve();
      offval2 += oc->Offset();
      offset2 = Standard_True;
    }
    else
      basic = Standard_True;
  }
  // Restore offset curves
  if (offset1)
    pc1 = new Geom2d_OffsetCurve(pc1, offval1);
  if (offset2)
    pc2 = new Geom2d_OffsetCurve(pc2, offval2);

  Standard_Boolean done1 = Standard_False, done2 = Standard_False;

  // Determine same edge case
  if (convert)
  {

    Handle(Geom2d_BSplineCurve) bsp1, bsp2;
    Handle(GeomCurve2d)        pc;
    Standard_Real               first, last;

    // iterate on pcurves
    Standard_Integer nbcurv = (n1 == n2 ? 1 : 2);
    for (Standard_Integer j = 1; j <= nbcurv; j++)
    {
      // Standard_Integer trim = Standard_False; // skl
      Handle(Geom2d_BSplineCurve) bsp;

      if (j == 1)
      {
        pc    = pc1;
        first = first1;
        last  = last1; /*trim = trimmed1;*/
      } // skl
      else
      {
        pc    = pc2;
        first = first2;
        last  = last2; /*trim = trimmed2;*/
      } // skl

      // Convert pcurve to bspline
      if (pc->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve)))
      {
        bsp = Handle(Geom2d_BSplineCurve)::DownCast(pc->Copy());
        // take segment if trim and range differ
        Standard_Real    fbsp = bsp->FirstParameter(), lbsp = bsp->LastParameter();
        Standard_Boolean segment = Standard_False;
        if (first > fbsp)
        {
          fbsp    = first;
          segment = Standard_True;
        }
        if (last < lbsp)
        {
          lbsp    = last;
          segment = Standard_True;
        }
        if (segment)
          bsp = Geom2dConvert1::SplitBSplineCurve(bsp, fbsp, lbsp, ::Precision1::PConfusion());
      }
      else if (pc->IsKind(STANDARD_TYPE(Geom2d_Conic)))
      {
        GeomAdaptor_Surface& AS   = *myAnalyzer->Surface()->Adaptor3d();
        Standard_Real        tolu = AS.UResolution(myAnalyzer->Precision1()),
                      tolv        = AS.VResolution(myAnalyzer->Precision1());
        Approx_Curve2d Conv(new Geom2dAdaptor_Curve(pc, first, last),
                            first,
                            last,
                            tolu,
                            tolv,
                            GeomAbs_C1,
                            9,
                            1000);
        if (Conv.IsDone() || Conv.HasResult())
          bsp = Conv.Curve();
      }
      else
      {
        // Restore trim for pcurve
        try
        {
          OCC_CATCH_SIGNALS
          Handle(GeomCurve2d) c;
          // 15.11.2002 PTV OCC966
          if (!Curve2::IsPeriodic(pc))
            c = new Geom2d_TrimmedCurve(pc,
                                        Max(first, pc->FirstParameter()),
                                        Min(last, pc->LastParameter()));
          else
            c = new Geom2d_TrimmedCurve(pc, first, last);
          bsp = Geom2dConvert1::CurveToBSplineCurve(c);
        }
        catch (ExceptionBase const& anException)
        {
#ifdef OCCT_DEBUG
          std::cout << "Warning: ShapeFix_Wire_1::FixGap2d: Exception in TrimmedCurve2d" << first
                    << " " << last << std::endl;
          anException.Print(std::cout);
          std::cout << std::endl;
#endif
          (void)anException;
        }
      }

      if (j == 1)
        bsp1 = bsp;
      else
        bsp2 = bsp;
    }

    // Take curves ends if could not convert
    gp_Pnt2d mpnt((cpnt1.XY() + cpnt2.XY()) * 0.5);
    if (bsp1.IsNull())
      mpnt = cpnt1;
    else if (bsp2.IsNull())
      mpnt = cpnt2;

    if (!bsp1.IsNull())
    {
      if (bsp1->Degree() == 1)
        bsp1->IncreaseDegree(2);
      if (n1 == n2)
      {
        bsp1->SetPole(1, mpnt);
        bsp1->SetPole(bsp1->NbPoles(), mpnt);
      }
      else
      {
        if (reversed1)
          bsp1->SetPole(1, mpnt);
        else
          bsp1->SetPole(bsp1->NbPoles(), mpnt);
      }
      first1 = bsp1->FirstParameter();
      last1  = bsp1->LastParameter();
      pc1    = bsp1;
      done1  = Standard_True;
    }
    if (!bsp2.IsNull())
    {
      if (bsp2->Degree() == 1)
        bsp2->IncreaseDegree(2);
      if (reversed2)
        bsp2->SetPole(bsp2->NbPoles(), mpnt);
      else
        bsp2->SetPole(1, mpnt);
      first2 = bsp2->FirstParameter();
      last2  = bsp2->LastParameter();
      pc2    = bsp2;
      done2  = Standard_True;
    }
  }
  else
  {

    if (n1 == n2)
    {
      if (pc1->IsKind(STANDARD_TYPE(Geom2d_Circle)) || pc1->IsKind(STANDARD_TYPE(Geom2d_Ellipse)))
      {
        Standard_Real diff = M_PI - Abs(clast1 - cfirst2) * 0.5;
        first1 -= diff;
        last1 += diff;
        done1 = Standard_True;
      }
    }
    else
    {

      // Determine domains for extremal points locating
      Standard_Real domfirst1 = first1, domlast1 = last1;
      if (pc1->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve))
          || pc1->IsKind(STANDARD_TYPE(Geom2d_BezierCurve)))
      {
        domfirst1 = pc1->FirstParameter();
        domlast1  = pc1->LastParameter();
      }
      else if (pc1->IsKind(STANDARD_TYPE(Geom2d_Line))
               || pc1->IsKind(STANDARD_TYPE(Geom2d_Parabola))
               || pc1->IsKind(STANDARD_TYPE(Geom2d_Hyperbola)))
      {
        Standard_Real diff = domlast1 - domfirst1;
        if (reversed1)
          domfirst1 -= 10. * diff;
        else
          domlast1 += 10. * diff;
      }
      else if (pc1->IsKind(STANDARD_TYPE(Geom2d_Circle))
               || pc1->IsKind(STANDARD_TYPE(Geom2d_Ellipse)))
      {
        domfirst1 = 0.;
        domlast1  = 2 * M_PI;
      }
      Standard_Real domfirst2 = first2, domlast2 = last2;
      if (pc2->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve))
          || pc2->IsKind(STANDARD_TYPE(Geom2d_BezierCurve)))
      {
        domfirst2 = pc2->FirstParameter();
        domlast2  = pc2->LastParameter();
      }
      else if (pc2->IsKind(STANDARD_TYPE(Geom2d_Line))
               || pc2->IsKind(STANDARD_TYPE(Geom2d_Parabola))
               || pc2->IsKind(STANDARD_TYPE(Geom2d_Hyperbola)))
      {
        Standard_Real diff = domlast2 - domfirst2;
        if (reversed2)
          domlast2 += 10. * diff;
        else
          domfirst2 -= 10. * diff;
      }
      else if (pc2->IsKind(STANDARD_TYPE(Geom2d_Circle))
               || pc2->IsKind(STANDARD_TYPE(Geom2d_Ellipse)))
      {
        domfirst2 = 0.;
        domlast2  = 2 * M_PI;
      }

      Standard_Real ipar1 = clast1, ipar2 = cfirst2;

      Geom2dInt_GInter        Inter;
      constexpr Standard_Real tolint = ::Precision1::PConfusion();

      Geom2dAdaptor_Curve AC1(pc1), AC2(pc2);

      // Try to find intersection points
      Domain2 dom1(pc1->Value(domfirst1),
                           domfirst1,
                           tolint,
                           pc1->Value(domlast1),
                           domlast1,
                           tolint);
      Domain2 dom2(pc2->Value(domfirst2),
                           domfirst2,
                           tolint,
                           pc2->Value(domlast2),
                           domlast2,
                           tolint);
      Inter.Perform(AC1, dom1, AC2, dom2, tolint, tolint);
      if (Inter.IsDone())
      {
        if (Inter.NbPoints() || Inter.NbSegments())
        {
          Standard_Integer i, index1 = 0, index2 = 0;
          Standard_Real    pardist, pardist1 = -1., pardist2 = -1.;
          // iterate on intersection points
          IntRes2d_IntersectionPoint IP;
          for (i = 1; i <= Inter.NbPoints(); i++)
          {
            IP = Inter.Point(i);
            // Adjust parameters on periodic curves
            Standard_Real u1 = AdjustOnPeriodic2d(pc1, reversed1, first1, last1, IP.ParamOnFirst());
            Standard_Real u2 =
              AdjustOnPeriodic2d(pc2, !reversed2, first2, last2, IP.ParamOnSecond());
            pardist = Abs(cfirst1 - u1);
            if (pardist1 > pardist || pardist1 < 0.)
            {
              index1   = i;
              pardist1 = pardist;
            }
            pardist = Abs(clast2 - u2);
            if (pardist2 > pardist || pardist2 < 0.)
            {
              index2   = i;
              pardist2 = pardist;
            }
          }
          Standard_Integer flag1 = 0, flag2 = 0;
          // iterate on intersection segments
          IntRes2d_IntersectionSegment IS;
          for (i = 1; i <= Inter.NbSegments(); i++)
          {
            IS = Inter.Segment1(i);
            for (Standard_Integer j = 1; j <= 2; j++)
            {
              if ((j == 1 && IS.HasFirstPoint()) || (j == 2 && IS.HasLastPoint()))
              {
                if (j == 1)
                  IP = IS.FirstPoint();
                else
                  IP = IS.LastPoint();
                // Adjust parameters on periodic curves
                Standard_Real u1 =
                  AdjustOnPeriodic2d(pc1, reversed1, first1, last1, IP.ParamOnFirst());
                Standard_Real u2 =
                  AdjustOnPeriodic2d(pc2, !reversed2, first2, last2, IP.ParamOnSecond());
                pardist = Abs(cfirst1 - u1);
                if (pardist1 > pardist || pardist1 < 0.)
                {
                  flag1    = j;
                  index1   = i;
                  pardist1 = pardist;
                }
                pardist = Abs(clast2 - u2);
                if (pardist2 > pardist || pardist2 < 0.)
                {
                  flag2    = j;
                  index2   = i;
                  pardist2 = pardist;
                }
              }
            }
          }
          if (index1 != index2 || flag1 != flag2)
          {
            // take intersection closer to mean point
            gp_Pnt2d pt1, pt2;
            if (flag1 == 0)
              pt1 = Inter.Point(index1).Value();
            else
            {
              IS = Inter.Segment1(index1);
              if (flag1 == 1)
                pt1 = IS.FirstPoint().Value();
              else
                pt1 = IS.LastPoint().Value();
            }
            if (flag2 == 0)
              pt2 = Inter.Point(index2).Value();
            else
            {
              IS = Inter.Segment1(index2);
              if (flag2 == 1)
                pt2 = IS.FirstPoint().Value();
              else
                pt2 = IS.LastPoint().Value();
            }
            gp_Pnt2d mpnt((cpnt1.XY() + cpnt2.XY()) * 0.5);
            if (pt2.Distance(mpnt) < pt1.Distance(mpnt))
            {
              index1 = index2;
              flag1  = flag2;
            }
          }
          if (flag1 == 0)
            IP = Inter.Point(index1);
          else
          {
            IS = Inter.Segment1(index1);
            if (flag1 == 1)
              IP = IS.FirstPoint();
            else
              IP = IS.LastPoint();
          }
          // Adjust parameters on periodic curves
          Standard_Real u1 = AdjustOnPeriodic2d(pc1, reversed1, first1, last1, IP.ParamOnFirst());
          Standard_Real u2 = AdjustOnPeriodic2d(pc2, !reversed2, first2, last2, IP.ParamOnSecond());
          // Check points to satisfy distance criterium
          gp_Pnt2d p1 = pc1->Value(u1), p2 = pc2->Value(u2);
          if (p1.Distance(p2) <= gap && Abs(cfirst1 - u1) > ::Precision1::PConfusion()
              && Abs(clast2 - u2) > ::Precision1::PConfusion()
              && (((u1 > first1) && (u1 < last1)) || ((u2 > first2) && (u2 < last2))
                  || (cpnt1.Distance(p1) <= gap) || (cpnt2.Distance(p2) <= gap)))
          {
            ipar1 = u1;
            ipar2 = u2;
            done1 = done2 = Standard_True;
          }
        }
      }

      // Try to find closest points if nothing yet found
      if (!done1)
      {
        Geom2dAPI_ExtremaCurveCurve Extr(pc1, pc2, domfirst1, domlast1, domfirst2, domlast2);
        if (Extr.NbExtrema())
        {
          Standard_Real u1, u2;
          Extr.LowerDistanceParameters(u1, u2);
          // Adjust parameters on periodic curves
          u1 = AdjustOnPeriodic2d(pc1, reversed1, first1, last1, u1);
          u2 = AdjustOnPeriodic2d(pc2, !reversed2, first2, last2, u2);
          // Check points to satisfy distance criterium
          gp_Pnt2d p1 = pc1->Value(u1), p2 = pc2->Value(u2);
          if (p1.Distance(p2) <= gap && Abs(cfirst1 - u1) > ::Precision1::PConfusion()
              && Abs(clast2 - u2) > ::Precision1::PConfusion()
              && (((u1 > first1) && (u1 < last1)) || ((u2 > first2) && (u2 < last2))
                  || (cpnt1.Distance(p1) <= gap) || (cpnt2.Distance(p2) <= gap)))
          {
            ipar1 = u1;
            ipar2 = u2;
            done1 = done2 = Standard_True;
          }
        }
      }

      // Try to find projections if nothing yet found
      if (!done1)
      {
        Geom2dAPI_ProjectPointOnCurve Proj;
        gp_Pnt2d                      ipnt1 = cpnt1, ipnt2 = cpnt2;
        Standard_Real                 u1 = ipar1, u2 = ipar2;
        Proj.Init(cpnt2, pc1, domfirst1, domlast1);
        if (Proj.NbPoints())
        {
          Standard_Integer index = 1;
          Standard_Real    dist, mindist = -1.;
          for (Standard_Integer i = 1; i <= Proj.NbPoints(); i++)
          {
            dist = cpnt2.Distance(Proj.Point(i));
            if (mindist > dist || mindist < 0.)
            {
              index   = i;
              mindist = dist;
            }
            ipnt1 = Proj.Point(index);
            u1    = Proj.Parameter(index);
          }
        }
        Proj.Init(cpnt1, pc2, domfirst2, domlast2);
        if (Proj.NbPoints())
        {
          Standard_Integer index = 1;
          Standard_Real    dist, mindist = -1.;
          for (Standard_Integer i = 1; i <= Proj.NbPoints(); i++)
          {
            dist = cpnt1.Distance(Proj.Point(i));
            if (mindist > dist || mindist < 0.)
            {
              index   = i;
              mindist = dist;
            }
            ipnt2 = Proj.Point(index);
            u2    = Proj.Parameter(index);
          }
        }
        // Adjust parameters on periodic curves
        u1 = AdjustOnPeriodic2d(pc1, reversed1, first1, last1, u1);
        u2 = AdjustOnPeriodic2d(pc2, !reversed2, first2, last2, u2);
        // Process special case of projection
        if ((((reversed1 && u1 > clast1) || (!reversed1 && u1 < clast1))
             && ((reversed2 && u2 < cfirst2) || (!reversed2 && u2 > cfirst2)))
            || (((reversed1 && u1 < clast1) || (!reversed1 && u1 > clast1))
                && ((reversed2 && u2 > cfirst2) || (!reversed2 && u2 < cfirst2))))
        {
          // both projections lie inside/outside initial domains
          // project mean point
          gp_Pnt2d mpnt((cpnt1.XY() + cpnt2.XY()) * 0.5);
          u1 = ipar1;
          u2 = ipar2;
          Proj.Init(mpnt, pc1, domfirst1, domlast1);
          if (Proj.NbPoints())
          {
            Standard_Integer index = 1;
            Standard_Real    dist, mindist = -1.;
            for (Standard_Integer i = 1; i <= Proj.NbPoints(); i++)
            {
              dist = mpnt.Distance(Proj.Point(i));
              if (mindist > dist || mindist < 0.)
              {
                index   = i;
                mindist = dist;
              }
              ipnt1 = Proj.Point(index);
              u1    = Proj.Parameter(index);
            }
          }
          Proj.Init(mpnt, pc2, domfirst2, domlast2);
          if (Proj.NbPoints())
          {
            Standard_Integer index = 1;
            Standard_Real    dist, mindist = -1.;
            for (Standard_Integer i = 1; i <= Proj.NbPoints(); i++)
            {
              dist = mpnt.Distance(Proj.Point(i));
              if (mindist > dist || mindist < 0.)
              {
                index   = i;
                mindist = dist;
              }
              ipnt2 = Proj.Point(index);
              u2    = Proj.Parameter(index);
            }
          }
        }
        else
        {
          if (cpnt1.Distance(ipnt2) < cpnt2.Distance(ipnt1))
            u1 = ipar1;
          else
            u2 = ipar2;
        }
        // Adjust parameters on periodic curves
        u1 = AdjustOnPeriodic2d(pc1, reversed1, first1, last1, u1);
        u2 = AdjustOnPeriodic2d(pc2, !reversed2, first2, last2, u2);
        // Check points to satisfy distance criterium
        gp_Pnt2d p1 = pc1->Value(u1), p2 = pc2->Value(u2);
        if (p1.Distance(p2) <= gap && Abs(cfirst1 - u1) > ::Precision1::PConfusion()
            && Abs(clast2 - u2) > ::Precision1::PConfusion()
            && (((u1 > first1) && (u1 < last1)) || ((u2 > first2) && (u2 < last2))
                || (cpnt1.Distance(p1) <= gap) || (cpnt2.Distance(p2) <= gap)))
        {
          ipar1 = u1;
          ipar2 = u2;
          done1 = done2 = Standard_True;
        }
      }

      if (done1)
      {

        if (ipar1 < first1 || ipar1 > last1 || ipar2 < first2 || ipar2 > last2)
        {

          // Check whether new points lie inside the surface bounds
          Standard_Real umin, umax, vmin, vmax;
          myAnalyzer->Surface()->Surface()->Bounds(umin, umax, vmin, vmax);
          if (::Precision1::IsInfinite(umin) || ::Precision1::IsInfinite(umax)
              || ::Precision1::IsInfinite(vmin) || ::Precision1::IsInfinite(vmax))
          {
            Standard_Real fumin, fumax, fvmin, fvmax;
            BRepTools1::UVBounds(face, fumin, fumax, fvmin, fvmax);
            if (::Precision1::IsInfinite(umin))
              umin = fumin - preci;
            if (::Precision1::IsInfinite(umax))
              umax = fumax + preci;
            if (::Precision1::IsInfinite(vmin))
              vmin = fvmin - preci;
            if (::Precision1::IsInfinite(vmax))
              vmax = fvmax + preci;
          }

          gp_Pnt2d         ipnt, P1, P2;
          Standard_Real    u, v;
          Standard_Boolean out;
          // iterate on curves
          for (Standard_Integer j = 1; j <= 2; j++)
          {

            if (j == 1)
            {
              if (ipar1 >= first1 && ipar1 <= last1)
                continue;
              ipnt = pc1->Value(ipar1);
            }
            else
            {
              if (ipar2 >= first2 && ipar2 <= last2)
                continue;
              ipnt = pc2->Value(ipar2);
            }

            // iterate on bounding lines
            for (Standard_Integer k = 1; k <= 2; k++)
            {

              u = ipnt.X();
              v = ipnt.Y();

              out = Standard_True;
              if (k == 1)
              {
                if (u < umin)
                {
                  P1 = gp_Pnt2d(umin, vmin);
                  P2 = gp_Pnt2d(umin, vmax);
                }
                else if (u > umax)
                {
                  P1 = gp_Pnt2d(umax, vmin);
                  P2 = gp_Pnt2d(umax, vmax);
                }
                else
                  out = Standard_False;
              }
              else
              {
                if (v < vmin)
                {
                  P1 = gp_Pnt2d(umin, vmin);
                  P2 = gp_Pnt2d(umax, vmin);
                }
                else if (v > vmax)
                {
                  P1 = gp_Pnt2d(umin, vmax);
                  P2 = gp_Pnt2d(umax, vmax);
                }
                else
                  out = Standard_False;
              }

              if (out)
              {
                // Intersect pcurve with bounding line
                Handle(Geom2d_Line) lin = new Geom2d_Line(P1, gp_Dir2d(gp_Vec2d(P1, P2)));
                Geom2dAdaptor_Curve ACL(lin);
                Domain2     dlin(P1, 0., tolint, P2, P1.Distance(P2), tolint);

                Handle(GeomCurve2d) pc;
                Standard_Real        fpar, lpar;
                if (j == 1)
                {
                  if (cfirst1 < ipar1)
                  {
                    fpar = cfirst1, lpar = ipar1;
                  }
                  else
                  {
                    fpar = ipar1, lpar = cfirst1;
                  }
                  pc = pc1;
                }
                else
                {
                  if (clast2 < ipar2)
                  {
                    fpar = clast2, lpar = ipar2;
                  }
                  else
                  {
                    fpar = ipar2, lpar = clast2;
                  }
                  pc = pc2;
                }
                Geom2dAdaptor_Curve ACC(pc);
                Domain2 domc(pc->Value(fpar), fpar, tolint, pc->Value(lpar), lpar, tolint);

                // Intersect line with the pcurve
                Inter.Perform(ACL, dlin, ACC, domc, tolint, tolint);
                if (Inter.IsDone())
                {
                  if (Inter.NbPoints() || Inter.NbSegments())
                  {
                    Standard_Integer i, index = 1;
                    Standard_Real    uu, dist, mindist = -1.;
                    // iterate on intersection points
                    for (i = 1; i <= Inter.NbPoints(); i++)
                    {
                      // Adjust parameters on periodic curve
                      uu   = AdjustOnPeriodic2d(pc,
                                              (j == 1 ? reversed1 : !reversed2),
                                              fpar,
                                              lpar,
                                              Inter.Point(i).ParamOnSecond());
                      dist = Abs((j == 1 ? cfirst1 : clast2) - uu);
                      if (mindist > dist || mindist < 0.)
                      {
                        index   = i;
                        mindist = dist;
                      }
                    }
                    // iterate on intersection segments
                    Standard_Integer             flag = 0;
                    IntRes2d_IntersectionPoint   IP;
                    IntRes2d_IntersectionSegment IS;
                    for (i = 1; i <= Inter.NbSegments(); i++)
                    {
                      IS = Inter.Segment1(i);
                      for (Standard_Integer jj = 1; jj <= 2; jj++)
                      {
                        if ((jj == 1 && IS.HasFirstPoint()) || (jj == 2 && IS.HasLastPoint()))
                        {
                          if (jj == 1)
                            IP = IS.FirstPoint();
                          else
                            IP = IS.LastPoint();
                          // Adjust parameters on periodic curve
                          uu   = AdjustOnPeriodic2d(pc,
                                                  (jj == 1 ? reversed1 : !reversed2),
                                                  fpar,
                                                  lpar,
                                                  IP.ParamOnSecond());
                          dist = Abs((jj == 1 ? cfirst1 : clast2) - uu);
                          if (mindist > dist || mindist < 0.)
                          {
                            flag    = jj;
                            index   = i;
                            mindist = dist;
                          }
                        }
                      }
                    }
                    if (flag == 0)
                      IP = Inter.Point(index);
                    else
                    {
                      IS = Inter.Segment1(index);
                      if (flag == 1)
                        IP = IS.FirstPoint();
                      else
                        IP = IS.LastPoint();
                    }
                    // Adjust parameters on periodic curve
                    uu = AdjustOnPeriodic2d(pc,
                                            (j == 1 ? reversed1 : !reversed2),
                                            fpar,
                                            lpar,
                                            IP.ParamOnSecond());
                    if (j == 1 && Abs(cfirst1 - uu) > ::Precision1::PConfusion())
                    {
                      ipar1 = uu;
                      ipnt  = IP.Value();
                    }
                    if (j == 2 && Abs(clast2 - uu) > ::Precision1::PConfusion())
                    {
                      ipar2 = uu;
                      ipnt  = IP.Value();
                    }
                  }
                }
              }
            }

            // Adjust if intersection lies inside old bounds
            if (j == 1)
            {
              if (reversed1)
              {
                if (ipar1 > first1)
                  ipar1 = first1;
              }
              else
              {
                if (ipar1 < last1)
                  ipar1 = last1;
              }
            }
            else
            {
              if (reversed2)
              {
                if (ipar2 < last2)
                  ipar2 = last2;
              }
              else
              {
                if (ipar2 > first2)
                  ipar2 = first2;
              }
            }
          }
        }
      }
      try
      {
        OCC_CATCH_SIGNALS
        if (done1)
        {
          if (ipar1 == clast1)
            done1 = Standard_False;
          else
          {
            // Set up new bounds for pcurve
            if (reversed1)
              first1 = ipar1;
            else
              last1 = ipar1;
            // Set new trim for old pcurve
            if (trimmed1)
              pc1 = new Geom2d_TrimmedCurve(pc1, first1, last1);
          }
        }
        if (done2)
        {
          if (ipar2 == cfirst2)
            done2 = Standard_False;
          else
          {
            // Set up new bounds for pcurve
            if (reversed2)
              last2 = ipar2;
            else
              first2 = ipar2;
            // Set new trim for old pcurve
            if (trimmed2)
              pc2 = new Geom2d_TrimmedCurve(pc2, first2, last2);
          }
        }
      }
      catch (ExceptionBase const& anException)
      {
#ifdef OCCT_DEBUG
        std::cout << "Warning: ShapeFix_Wire_1::FixGap2d: Exception in TrimmedCurve2d  :"
                  << std::endl;
        anException.Print(std::cout);
        std::cout << std::endl;
#endif
        (void)anException;
      }
    }
  }

  if (done1 || done2)
  {

    ShapeBuilder            B;
    Edge2         SBE;
    ShapeTolerance1 SFST;

    // Update vertices
    TopoVertex V1 = SAE.LastVertex(E1), V2 = SAE.FirstVertex(E2);
    TopoVertex nullV, newV1;
    // smh#8
    TopoShape  emptyCopiedV2 = V2.EmptyCopied();
    TopoVertex newV2         = TopoDS::Vertex(emptyCopiedV2);
    SFST.SetTolerance(newV2, ::Precision1::Confusion());
    Context()->Replace(V2, newV2);
    if (V1.IsSame(V2))
    // smh#8
    {
      TopoShape tmpVertexRev = newV2.Oriented(TopAbs_REVERSED);
      newV1                     = TopoDS::Vertex(tmpVertexRev);
    }
    else
    {
      // smh#8
      TopoShape emptyCopiedV1 = V1.EmptyCopied();
      newV1                      = TopoDS::Vertex(emptyCopiedV1);
      SFST.SetTolerance(newV1, ::Precision1::Confusion());
      Context()->Replace(V1, newV1);
    }

    if (done1)
    {
      // Update first edge
      TopoEdge newE1 = SBE.CopyReplaceVertices(E1, nullV, newV1);
      // smh#8
      TopoShape tmpE1 = newE1.Oriented(TopAbs_FORWARD);
      B.UpdateEdge(TopoDS::Edge(tmpE1), pc1, face, 0.);
      B.Range(TopoDS::Edge(tmpE1), face, first1, last1);
      SFST.SetTolerance(newE1, ::Precision1::Confusion(), TopAbs_EDGE);
      B.SameRange(newE1, Standard_False);
      //      B.SameParameter(newE1,Standard_False);

      // To keep NM vertices belonging initial edges
      TopoDS_Iterator aItv(E1, Standard_False);
      for (; aItv.More(); aItv.Next())
      {
        if (aItv.Value().Orientation() == TopAbs_INTERNAL
            || aItv.Value().Orientation() == TopAbs_EXTERNAL)
        {
          TopoVertex aOldV = TopoDS::Vertex(aItv.Value());
          TopoVertex anewV =
            ShapeAnalysis_TransferParametersProj::CopyNMVertex(aOldV, newE1, E1);
          B.Add(newE1, anewV);
          Context()->Replace(aOldV, anewV);
        }
      }

      Context()->Replace(E1, newE1);
      sbwd->Set(newE1, n1);
    }

    if (done2)
    {
      // Update second edge
      TopoEdge newE2 = SBE.CopyReplaceVertices(E2, newV2, nullV);
      // smh#8
      TopoShape tmpE2 = newE2.Oriented(TopAbs_FORWARD);
      B.UpdateEdge(TopoDS::Edge(tmpE2), pc2, face, 0.);
      B.Range(TopoDS::Edge(tmpE2), face, first2, last2);
      SFST.SetTolerance(newE2, ::Precision1::Confusion(), TopAbs_EDGE);
      B.SameRange(newE2, Standard_False);
      //      B.SameParameter(newE2,Standard_False);
      // To keep NM vertices belonging initial edges
      TopoDS_Iterator aItv(E2, Standard_False);
      for (; aItv.More(); aItv.Next())
      {
        if (aItv.Value().Orientation() == TopAbs_INTERNAL
            || aItv.Value().Orientation() == TopAbs_EXTERNAL)
        {
          TopoVertex aOldV = TopoDS::Vertex(aItv.Value());
          TopoVertex anewV =
            ShapeAnalysis_TransferParametersProj::CopyNMVertex(aOldV, newE2, E2);
          B.Add(newE2, anewV);
          Context()->Replace(aOldV, anewV);
        }
      }
      Context()->Replace(E2, newE2);
      sbwd->Set(newE2, n2);
    }

    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
  }
  else if (convert)
    myLastFixStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);

  return (done1 || done2);
}
