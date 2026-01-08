// Created on: 1997-01-22
// Created by: Prestataire Michael ALEONARD
// Copyright (c) 1997-1999 Matra Datavision
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

#include <DsgPrs_SymmetricPresentation.hxx>
#include <ElCLib.hxx>
#include <gce_MakeLin.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Graphic3d_ArrayOfPoints.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Graphic3d_AspectMarker3d.hxx>
#include <Graphic3d_Group.hxx>
#include <Precision.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_DimensionAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Presentation.hxx>

//===================================================================
// Function:Add
// Purpose: draws the representation of an axial symmetry between two segments.
//===================================================================
void SymmetricPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
                                       const Handle(StyleDrawer)&       aDrawer,
                                       const Point3d&                     AttachmentPoint1,
                                       const Point3d&                     AttachmentPoint2,
                                       const Dir3d&                     aDirection1,
                                       const gp_Lin&                     aAxis,
                                       const Point3d&                     OffsetPoint)
{
  Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  Point3d        ProjOffsetPoint = ElCLib1::Value(ElCLib1::Parameter(aAxis, OffsetPoint), aAxis);
  Point3d        PjAttachPnt1    = ElCLib1::Value(ElCLib1::Parameter(aAxis, AttachmentPoint1), aAxis);
  Dir3d        aDirectionAxis  = aAxis.Direction();
  Standard_Real h =
    fabs(ProjOffsetPoint.Distance(PjAttachPnt1) / cos(aDirectionAxis.Angle(aDirection1)));

  Vector3d        VL1(aDirection1);
  Vector3d        VLa(PjAttachPnt1, ProjOffsetPoint);
  Standard_Real scal;
  scal = VL1.Dot(VLa);
  if (scal < 0)
    VL1.Reverse();
  VL1.Multiply(h);

  Point3d P1, P2;

  //======================================
  // SYMMETRY OF EDGE PERPEND. TO THE AXIS
  //   ____        :        ____
  // edge2 |       : -=-   | edge 1
  //       |<------:------>|
  //               :
  //======================================

  if (VLa.Dot(VL1) == 0)
  {
    P1 = AttachmentPoint1.Translated(VLa);
    Vector3d VPntat2Axe(PjAttachPnt1, AttachmentPoint2);
    P2 = ProjOffsetPoint.Translated(VPntat2Axe);
  }
  else
  {
    P1 = AttachmentPoint1.Translated(VL1);
    Vector3d VPntat1Axe(P1, ProjOffsetPoint);
    P2 = ProjOffsetPoint.Translated(VPntat1Axe);
  }

  gp_Lin        L3 = gce_MakeLin(P1, P2);
  Standard_Real parmin, parmax, parcur;
  parmin             = ElCLib1::Parameter(L3, P1);
  parmax             = parmin;
  parcur             = ElCLib1::Parameter(L3, P2);
  Standard_Real dist = Abs(parmin - parcur);
  if (parcur < parmin)
    parmin = parcur;
  if (parcur > parmax)
    parmax = parcur;
  parcur      = ElCLib1::Parameter(L3, OffsetPoint);
  Point3d offp = ElCLib1::Value(parcur, L3);

  Standard_Boolean outside = Standard_False;
  if (parcur < parmin)
  {
    parmin  = parcur;
    outside = Standard_True;
  }
  if (parcur > parmax)
  {
    parmax  = parcur;
    outside = Standard_True;
  }

  Point3d PointMin = ElCLib1::Value(parmin, L3);
  Point3d PointMax = ElCLib1::Value(parmax, L3);

  Standard_Real X, Y, Z;
  Standard_Real D1(aAxis.Distance(AttachmentPoint1)), coeff(.5);
  Point3d        pint, Pj_P1, P1Previous = P1;

  /*=======================================================
   TO AVOID CROSSING
          P1  -=- P2                P2  -=- P1
            \<-->/                    |<-->|
             \  /                     |    |
              \/                      |    |
              /\                      |    |
             /  \                     |    |
   Pattach2 /____\ Pattach1 Pattach2 /______\ Pattach1
           /  NO \                  /   YES  \
  =======================================================*/

  Standard_Boolean Cross = Standard_False;
  Vector3d           Attch1_PjAttch1(AttachmentPoint1, PjAttachPnt1);
  Vector3d           v(P1, ProjOffsetPoint);
  if (v.IsOpposite((Attch1_PjAttch1), Precision1::Confusion()))
  {
    Cross = Standard_True;
    Point3d PntTempo;
    PntTempo = P1;
    P1       = P2;
    P2       = PntTempo;
  }
  /*===================================
   FRACTURES OF TRAITS OF CALL
          /             \
         /               \
         |      -=-      |
         |<------------->|
  ===================================*/

  Vector3d        Vfix;
  Standard_Real alpha, b;

  if (aAxis.Distance(P1) > D1 * (1 + coeff) && !Cross)
  {

    //==== PROCESSING OF FACE ===========
    aPresentation->NewGroup();
    aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

    Pj_P1 = ElCLib1::Value(ElCLib1::Parameter(aAxis, P1), aAxis);
    Vector3d Vp(Pj_P1, P1);
    Vfix = Vp.Divided(Vp.Magnitude()).Multiplied(D1 * (1 + coeff));
    P1   = Pj_P1.Translated(Vfix);
    P2   = Pj_P1.Translated(Vfix.Reversed());

    //=================================
    // LISTING AT THE EXTERIOR
    //                        -=-
    //      ->|----------|<------
    //        |          |
    //=================================

    L3     = gce_MakeLin(P1, P2);
    parmin = ElCLib1::Parameter(L3, P1);
    parmax = parmin;
    parcur = ElCLib1::Parameter(L3, P2);
    dist   = Abs(parmin - parcur);
    if (parcur < parmin)
      parmin = parcur;
    if (parcur > parmax)
      parmax = parcur;
    parcur  = ElCLib1::Parameter(L3, OffsetPoint);
    offp    = ElCLib1::Value(parcur, L3);
    outside = Standard_False;
    if (parcur < parmin)
    {
      parmin  = parcur;
      outside = Standard_True;
    }
    if (parcur > parmax)
    {
      parmax  = parcur;
      outside = Standard_True;
    }
    PointMin = ElCLib1::Value(parmin, L3);
    PointMax = ElCLib1::Value(parmax, L3);

    Handle(Graphic3d_ArrayOfPolylines) aPrims = new Graphic3d_ArrayOfPolylines(8, 3);

    aPrims->AddBound(2);
    aPrims->AddVertex(PointMin);
    aPrims->AddVertex(PointMax);

    //==== PROCESSING OF CALL 1 =====
    alpha = aDirectionAxis.Angle(aDirection1);
    b     = (coeff * D1) / sin(alpha);
    Vector3d Vpint(AttachmentPoint1, P1Previous);
    pint = AttachmentPoint1.Translated(Vpint.Divided(Vpint.Magnitude()).Multiplied(b));

    aPrims->AddBound(3);
    aPrims->AddVertex(AttachmentPoint1);
    aPrims->AddVertex(pint);
    aPrims->AddVertex(P1);

    //==== PROCESSING OF CALL 2 =====
    Point3d Pj_pint = ElCLib1::Value(ElCLib1::Parameter(aAxis, pint), aAxis);
    Vector3d V_int(pint, Pj_pint);
    Point3d Sym_pint = Pj_pint.Translated(V_int);

    aPrims->AddBound(3);
    aPrims->AddVertex(AttachmentPoint2);
    aPrims->AddVertex(Sym_pint);
    aPrims->AddVertex(P2);

    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
  }

  /*===================================
   FRACTURES OF PROCESSING OF CALL
                -=-
           |<--------->|
           |           |
          /             \
         /               \
  ===================================*/
  else if (aAxis.Distance(P1) < D1 * (1 - coeff) || Cross)
  {

    //------ PROCESSING OF FACE ------------
    aPresentation->NewGroup();
    aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

    Pj_P1 = ElCLib1::Value(ElCLib1::Parameter(aAxis, P1), aAxis);
    Vector3d VpInf(Pj_P1, P1);
    Vfix = VpInf.Divided(VpInf.Magnitude()).Multiplied(D1 * (1 - coeff));
    Pj_P1.Translated(Vfix).Coord(X, Y, Z);
    P1.SetCoord(X, Y, Z);
    Pj_P1.Translated(Vfix.Reversed()).Coord(X, Y, Z);
    P2.SetCoord(X, Y, Z);

    //=================================
    // LISTING AT THE EXTERIOR
    //                        -=-
    //      ->|----------|<------
    //        |          |
    //=================================
    L3     = gce_MakeLin(P1, P2);
    parmin = ElCLib1::Parameter(L3, P1);
    parmax = parmin;
    parcur = ElCLib1::Parameter(L3, P2);
    dist   = Abs(parmin - parcur);
    if (parcur < parmin)
      parmin = parcur;
    if (parcur > parmax)
      parmax = parcur;
    parcur  = ElCLib1::Parameter(L3, OffsetPoint);
    offp    = ElCLib1::Value(parcur, L3);
    outside = Standard_False;
    if (parcur < parmin)
    {
      parmin  = parcur;
      outside = Standard_True;
    }
    if (parcur > parmax)
    {
      parmax  = parcur;
      outside = Standard_True;
    }
    PointMin = ElCLib1::Value(parmin, L3);
    PointMax = ElCLib1::Value(parmax, L3);

    Handle(Graphic3d_ArrayOfPolylines) aPrims = new Graphic3d_ArrayOfPolylines(8, 3);

    aPrims->AddBound(2);
    aPrims->AddVertex(PointMin);
    aPrims->AddVertex(PointMax);

    //==== PROCESSING OF CALL 1 =====
    alpha = aDirectionAxis.Angle(aDirection1);
    b     = (coeff * D1) / sin(alpha);
    Vector3d Vpint(AttachmentPoint1, P1Previous);
    pint = AttachmentPoint1.Translated(Vpint.Divided(Vpint.Magnitude()).Multiplied(b));

    aPrims->AddBound(3);
    aPrims->AddVertex(AttachmentPoint1);
    aPrims->AddVertex(pint);
    aPrims->AddVertex(P1);

    //==== PROCESSING OF CALL 2 =====
    Point3d Pj_pint = ElCLib1::Value(ElCLib1::Parameter(aAxis, pint), aAxis);
    Vector3d V_int(pint, Pj_pint);
    Point3d Sym_pint = Pj_pint.Translated(V_int);

    aPrims->AddBound(3);
    aPrims->AddVertex(AttachmentPoint2);
    aPrims->AddVertex(Sym_pint);
    aPrims->AddVertex(P2);

    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
  }
  else
  {

    //==== PROCESSING OF FACE ===========
    aPresentation->NewGroup();
    aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

    Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments(6);

    aPrims->AddVertex(PointMin);
    aPrims->AddVertex(PointMax);

    //==== PROCESSING OF CALL 1 =====
    aPrims->AddVertex(AttachmentPoint1);
    aPrims->AddVertex(P1);

    //==== PROCESSING OF CALL 2 =====
    aPrims->AddVertex(AttachmentPoint2);
    aPrims->AddVertex(P2);

    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
  }

  //==== ARROWS ================
  aPresentation->NewGroup();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  if (dist < (LA->ArrowAspect()->Length() + LA->ArrowAspect()->Length()))
    outside = Standard_True;
  Dir3d arrdir = L3.Direction().Reversed();
  if (outside)
    arrdir.Reverse();
  // arrow 1 ----
  Arrow1::Draw1(aPresentation->CurrentGroup(),
                    P1,
                    arrdir,
                    LA->ArrowAspect()->Angle(),
                    LA->ArrowAspect()->Length());

  // arrow 2 ----
  Arrow1::Draw1(aPresentation->CurrentGroup(),
                    P2,
                    arrdir.Reversed(),
                    LA->ArrowAspect()->Angle(),
                    LA->ArrowAspect()->Length());

  //-------------------------------------------------------------------------------------
  //|                                SYMBOL OF SYMMETRY                                 |
  //-------------------------------------------------------------------------------------

  //           -------    : Superior Segment1
  //         -----------  : Axis
  //           -------    : Inferior Segment1

  Vector3d Vvar(P1, P2);
  Vector3d vec;
  Vector3d Vtmp =
    Vvar.Divided(Vvar.Magnitude())
      .Multiplied((aAxis.Distance(AttachmentPoint1) + aAxis.Distance(AttachmentPoint2)));
  vec.SetCoord(Vtmp.X(), Vtmp.Y(), Vtmp.Z());
  Vector3d vecA = vec.Multiplied(.1);

  Dir3d DirAxis = aAxis.Direction();
  Vector3d Vaxe(DirAxis);
  Vector3d vecB = Vaxe.Multiplied(vecA.Magnitude());
  vecB.Multiply(.5);

  Point3d pm, pOff;
  if (VLa.Dot(VL1) == 0)
  {
    Vector3d Vper(P1, ElCLib1::Value(ElCLib1::Parameter(aAxis, P1), aAxis));
    pm = P1.Translated(Vper);
  }
  else
  {
    pm = P1.Translated(Vvar.Multiplied(.5));
  }
  pOff = OffsetPoint.Translated(vecB);

  // Calculate the extremities of the symbol axis
  Vector3d vecAxe = vecA.Multiplied(.7);

  aPresentation->NewGroup();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  Handle(Graphic3d_ArrayOfPolylines) aPrims = new Graphic3d_ArrayOfPolylines(13, 5);

  aPrims->AddBound(2);
  aPrims->AddVertex(pOff.Translated(vecAxe));
  aPrims->AddVertex(pOff.Translated(vecAxe.Reversed()));

  // Calculate the extremities of the superior segment of the symbol
  Vector3d vec1 = vecAxe.Multiplied(.6);
  vecAxe      = Vaxe.Multiplied(vecAxe.Magnitude());
  Vector3d vec2 = vecAxe.Multiplied(.4);

  aPrims->AddBound(2);
  aPrims->AddVertex(pOff.Translated(vec1.Added(vec2)));
  aPrims->AddVertex(pOff.Translated(vec1.Reversed().Added(vec2)));

  // Calculate the extremities of the inferior segment of the symbol
  aPrims->AddBound(2);
  aPrims->AddVertex(pOff.Translated(vec1.Added(vec2.Reversed())));
  aPrims->AddVertex(pOff.Translated(vec1.Reversed().Added(vec2.Reversed())));

  /*--------------------------------------------------------------------------------------
  |                          MARKING OF THE SYMMETRY AXIS                                |
  ----------------------------------------------------------------------------------------
          ____
          \  / :Cursor
           \/
           /\
          /__\
*/
  Standard_Real Dist = (aAxis.Distance(AttachmentPoint1) + aAxis.Distance(AttachmentPoint2)) / 75;
  Vector3d        vs(aDirectionAxis);
  Vector3d        vsym(vs.Divided(vs.Magnitude()).Multiplied(Dist).XYZ());
  Vector3d        vsymper(vsym.Y(), -vsym.X(), vsym.Z());

  aPrims->AddBound(5);
  Point3d pm1 = pm.Translated(vsym.Added(vsymper));
  aPrims->AddVertex(pm1);
  pm1 = pm1.Translated(vsym.Reversed().Multiplied(2).Added(vsymper.Reversed().Multiplied(2)));
  aPrims->AddVertex(pm1);
  pm1 = pm1.Translated(vsymper.Multiplied(2));
  aPrims->AddVertex(pm1);
  pm1 = pm1.Translated(vsym.Multiplied(2).Added(vsymper.Reversed().Multiplied(2)));
  aPrims->AddVertex(pm1);
  pm1 = pm1.Translated(vsymper.Multiplied(2));
  aPrims->AddVertex(pm1);

  vsym.Multiply(4);

  aPrims->AddBound(2);
  aPrims->AddVertex(pm.Translated(vsym));
  aPrims->AddVertex(pm.Translated(vsym.Reversed()));

  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
}

//===================================================================
// Function:Add
// Purpose: draws the representation of an axial symmetry between two arcs.
//===================================================================
void SymmetricPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
                                       const Handle(StyleDrawer)&       aDrawer,
                                       const Point3d&                     AttachmentPoint1,
                                       const Point3d&                     AttachmentPoint2,
                                       const gp_Circ&                    aCircle1,
                                       const gp_Lin&                     aAxis,
                                       const Point3d&                     OffsetPoint)
{
  Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  Point3d OffsetPnt(OffsetPoint.X(), OffsetPoint.Y(), OffsetPoint.Z());
  Point3d Center1         = aCircle1.Location();
  Point3d ProjOffsetPoint = ElCLib1::Value(ElCLib1::Parameter(aAxis, OffsetPnt), aAxis);
  Point3d ProjCenter1     = ElCLib1::Value(ElCLib1::Parameter(aAxis, Center1), aAxis);
  Vector3d Vp(ProjCenter1, Center1);
  if (Vp.Magnitude() <= Precision1::Confusion())
    Vp = Vector3d(aAxis.Direction()) ^ aCircle1.Position1().Direction();

  Standard_Real Dt, R, h;
  Dt = ProjCenter1.Distance(ProjOffsetPoint);
  R  = aCircle1.Radius();
  if (Dt > .999 * R)
  {
    Dt = .999 * R;
    Vector3d Vout(ProjCenter1, ProjOffsetPoint);
    ProjOffsetPoint = ProjCenter1.Translated(Vout.Divided(Vout.Magnitude()).Multiplied(Dt));
    OffsetPnt       = ProjOffsetPoint;
  }
  h         = Sqrt(R * R - Dt * Dt);
  Point3d P1 = ProjOffsetPoint.Translated(Vp.Added(Vp.Divided(Vp.Magnitude()).Multiplied(h)));
  Vector3d v(P1, ProjOffsetPoint);
  Point3d P2 = ProjOffsetPoint.Translated(v);

  gp_Lin        L3 = gce_MakeLin(P1, P2);
  Standard_Real parmin, parmax, parcur;
  parmin             = ElCLib1::Parameter(L3, P1);
  parmax             = parmin;
  parcur             = ElCLib1::Parameter(L3, P2);
  Standard_Real dist = Abs(parmin - parcur);
  if (parcur < parmin)
    parmin = parcur;
  if (parcur > parmax)
    parmax = parcur;
  parcur = ElCLib1::Parameter(L3, OffsetPnt);

  Standard_Boolean outside = Standard_False;
  if (parcur < parmin)
  {
    parmin  = parcur;
    outside = Standard_True;
  }
  if (parcur > parmax)
  {
    parmax  = parcur;
    outside = Standard_True;
  }
  Point3d PointMin = ElCLib1::Value(parmin, L3);
  Point3d PointMax = ElCLib1::Value(parmax, L3);

  //==== PROCESSING OF FACE ===========
  Handle(Graphic3d_ArrayOfPrimitives) aPrims = new Graphic3d_ArrayOfSegments(2);
  aPrims->AddVertex(PointMin);
  aPrims->AddVertex(PointMax);
  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  //==== PROCESSING OF CALL 1 =====
  Standard_Integer nbp           = 10;
  Standard_Real    ParamP1       = ElCLib1::Parameter(aCircle1, P1);
  Standard_Real    ParamPAttach1 = ElCLib1::Parameter(aCircle1, AttachmentPoint1);
  Standard_Real    alpha, Dalpha, alphaIter;

  alpha = fabs(ParamP1 - ParamPAttach1);
  if (ParamP1 < ParamPAttach1)
  {
    if (alpha > M_PI)
    {
      alpha  = (2. * M_PI) - alpha;
      nbp    = (Standard_Integer)IntegerPart(alpha / (alpha * .02));
      Dalpha = alpha / (nbp - 1);
    }
    else
    {
      nbp    = (Standard_Integer)IntegerPart(alpha / (alpha * .02));
      Dalpha = -alpha / (nbp - 1);
    }
  }
  else
  {
    if (alpha > M_PI)
    {
      alpha  = (2. * M_PI) - alpha;
      nbp    = (Standard_Integer)IntegerPart(alpha / (alpha * .02));
      Dalpha = -alpha / (nbp - 1);
    }
    else
    {
      nbp    = (Standard_Integer)IntegerPart(alpha / (alpha * .02));
      Dalpha = alpha / (nbp - 1);
    }
  }

  aPrims = new Graphic3d_ArrayOfPolylines(nbp);
  aPrims->AddVertex(AttachmentPoint1);
  alphaIter = Dalpha;
  Point3d           PntIter;
  Standard_Integer i;
  for (i = 2; i <= nbp; i++, alphaIter += Dalpha)
    aPrims->AddVertex(ElCLib1::Value(ParamPAttach1 + alphaIter, aCircle1));
  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  //==== PROCESSING OF CALL 2 =====
  Point3d        Center2 = ProjCenter1.Translated(Vp.Reversed());
  Dir3d        DirC2   = aCircle1.Axis().Direction();
  Frame3d        AxeC2(Center2, DirC2);
  gp_Circ       aCircle2(AxeC2, aCircle1.Radius());
  Standard_Real ParamP2       = ElCLib1::Parameter(aCircle2, P2);
  Standard_Real ParamPAttach2 = ElCLib1::Parameter(aCircle2, AttachmentPoint2);

  alpha = fabs(ParamP2 - ParamPAttach2);
  if (alpha <= Precision1::Confusion())
    alpha = 1.e-5;
  if (ParamP2 < ParamPAttach2)
  {
    if (alpha > M_PI)
    {
      alpha  = (2 * M_PI) - alpha;
      nbp    = (Standard_Integer)IntegerPart(alpha / (alpha * .02));
      Dalpha = alpha / (nbp - 1);
    }
    else
    {
      nbp    = (Standard_Integer)IntegerPart(alpha / (alpha * .02));
      Dalpha = -alpha / (nbp - 1);
    }
  }
  else
  {
    if (alpha > M_PI)
    {
      alpha  = (2 * M_PI) - alpha;
      nbp    = (Standard_Integer)IntegerPart(alpha / (alpha * .02));
      Dalpha = -alpha / (nbp - 1);
    }
    else
    {
      nbp    = (Standard_Integer)IntegerPart(alpha / (alpha * .02));
      Dalpha = alpha / (nbp - 1);
    }
  }

  aPrims = new Graphic3d_ArrayOfPolylines(nbp);
  aPrims->AddVertex(AttachmentPoint2);
  alphaIter = Dalpha;
  for (i = 2; i <= nbp; i++, alphaIter += Dalpha)
    aPrims->AddVertex(ElCLib1::Value(ParamPAttach2 + alphaIter, aCircle2));
  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  //==== ARROWS ================
  aPresentation->NewGroup();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  if (dist < (LA->ArrowAspect()->Length() + LA->ArrowAspect()->Length()))
    outside = Standard_True;
  Dir3d arrdir = L3.Direction().Reversed();
  if (outside)
    arrdir.Reverse();
  // arrow 1 ----
  Arrow1::Draw1(aPresentation->CurrentGroup(),
                    P1,
                    arrdir,
                    LA->ArrowAspect()->Angle(),
                    LA->ArrowAspect()->Length());

  // arrow 2 ----
  Arrow1::Draw1(aPresentation->CurrentGroup(),
                    P2,
                    arrdir.Reversed(),
                    LA->ArrowAspect()->Angle(),
                    LA->ArrowAspect()->Length());

  //-------------------------------------------------------------------------------------
  //|                                SYMBOL OF SYMMETRY                                 |
  //-------------------------------------------------------------------------------------

  //           -------    : Superior Segment1
  //         -----------  : Axis
  //           -------    : Inferior Segment1

  Vector3d Vvar(P1, P2);
  Vector3d Vtmp = Vvar.Divided(Vvar.Magnitude()).Multiplied(2 * (aAxis.Distance(Center1)));
  Vector3d vec  = Vtmp;
  Vector3d vecA = vec.Multiplied(.1);

  Dir3d DirAxis = aAxis.Direction();
  Vector3d Vaxe(DirAxis);
  Vector3d vecB = Vaxe.Multiplied(vecA.Magnitude());
  vecB.Multiply(.5);

  Point3d pm   = P1.Translated(Vvar.Multiplied(.5));
  Point3d pOff = OffsetPnt.Translated(vecB);

  // Calculation of extremas of the axis of the symbol
  Vector3d vecAxe = vecA.Multiplied(.7);

  aPresentation->NewGroup();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  aPrims = new Graphic3d_ArrayOfPolylines(13, 5);

  aPrims->AddBound(2);
  aPrims->AddVertex(pOff.Translated(vecAxe));
  aPrims->AddVertex(pOff.Translated(vecAxe.Reversed()));

  // Calculation of extremas of the superior segment of the symbol
  Vector3d vec1 = vecAxe.Multiplied(.6);
  vecAxe      = Vaxe.Multiplied(vecAxe.Magnitude());
  Vector3d vec2 = vecAxe.Multiplied(.4);

  aPrims->AddBound(2);
  aPrims->AddVertex(pOff.Translated(vec1.Added(vec2)));
  aPrims->AddVertex(pOff.Translated(vec1.Reversed().Added(vec2)));

  // Calculation of extremas of the inferior segment of the symbol
  aPrims->AddBound(2);
  aPrims->AddVertex(pOff.Translated(vec1.Added(vec2.Reversed())));
  aPrims->AddVertex(pOff.Translated(vec1.Reversed().Added(vec2.Reversed())));

  /*--------------------------------------------------------------------------------------
    |                          MARKING OF THE AXIS OF SYMMETRY                           |
    --------------------------------------------------------------------------------------
            ____
            \  / :Cursor
             \/
             /\
            /__\
  */
  Standard_Real Dist           = aAxis.Distance(Center1) / 37;
  Dir3d        aDirectionAxis = aAxis.Direction();
  Vector3d        vs(aDirectionAxis);
  Vector3d        vsym(vs.Divided(vs.Magnitude()).Multiplied(Dist).XYZ());
  Vector3d        vsymper(vsym.Y(), -vsym.X(), vsym.Z());

  aPrims->AddBound(5);
  Point3d pm1 = pm.Translated(vsym.Added(vsymper));
  aPrims->AddVertex(pm1);
  pm1 = pm1.Translated(vsym.Reversed().Multiplied(2).Added(vsymper.Reversed().Multiplied(2)));
  aPrims->AddVertex(pm1);
  pm1 = pm1.Translated(vsymper.Multiplied(2));
  aPrims->AddVertex(pm1);
  pm1 = pm1.Translated(vsym.Multiplied(2).Added(vsymper.Reversed().Multiplied(2)));
  aPrims->AddVertex(pm1);
  pm1 = pm1.Translated(vsymper.Multiplied(2));
  aPrims->AddVertex(pm1);

  vsym.Multiply(4);

  aPrims->AddBound(2);
  aPrims->AddVertex(pm.Translated(vsym));
  aPrims->AddVertex(pm.Translated(vsym.Reversed()));

  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
}

//===================================================================
// Function:Add
// Purpose: draws the representation of an axial symmetry between two vertex.
//===================================================================
void SymmetricPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
                                       const Handle(StyleDrawer)&       aDrawer,
                                       const Point3d&                     AttachmentPoint1,
                                       const Point3d&                     AttachmentPoint2,
                                       const gp_Lin&                     aAxis,
                                       const Point3d&                     OffsetPoint)
{
  Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  if (AttachmentPoint1.IsEqual(AttachmentPoint2, Precision1::Confusion()))
  {
    //==============================================================
    //  SYMMETRY WHEN THE REFERENCE POINT IS ON THE AXIS OF SYM.:
    //==============================================================
    // Marker of localisation of the face
    Color1                   aColor = LA->LineAspect()->Aspect()->Color();
    Handle(Graphic3d_AspectMarker3d) aMarkerAsp =
      new Graphic3d_AspectMarker3d(Aspect_TOM_O, aColor, 1.0);
    aPresentation->CurrentGroup()->SetPrimitivesAspect(aMarkerAsp);
    Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints = new Graphic3d_ArrayOfPoints(1);
    anArrayOfPoints->AddVertex(AttachmentPoint1.X(), AttachmentPoint1.Y(), AttachmentPoint1.Z());
    aPresentation->CurrentGroup()->AddPrimitiveArray(anArrayOfPoints);

    // Trace of the linking segment
    aPresentation->NewGroup();
    aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

    Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments(8);

    aPrims->AddVertex(AttachmentPoint1);
    aPrims->AddVertex(OffsetPoint);

    //--------------------------------------------------------------------------------------
    //|                                SYMBOL OF SYMMETRY                                  |
    //--------------------------------------------------------------------------------------
    //           -------    : Superior Segment1
    //         -----------  : Axis
    //           -------    : Inferior Segment1

    // Calculate extremas of the axis of the symbol
    Vector3d VAO(AttachmentPoint1, OffsetPoint);
    Vector3d uVAO  = VAO.Divided(VAO.Magnitude());
    Point3d pDaxe = OffsetPoint.Translated(uVAO.Multiplied(3.));
    Point3d pFaxe = pDaxe.Translated(uVAO.Multiplied(12.));

    aPrims->AddVertex(pDaxe);
    aPrims->AddVertex(pFaxe);

    // Calculate extremas of the superior segment of the symbol
    Vector3d nVAO(-uVAO.Y(), uVAO.X(), uVAO.Z());
    Point3d sgP11 = pDaxe.Translated(uVAO.Multiplied(2.).Added(nVAO.Multiplied(2.)));
    Point3d sgP12 = sgP11.Translated(uVAO.Multiplied(8.));

    aPrims->AddVertex(sgP11);
    aPrims->AddVertex(sgP12);

    // Calculate extremas of the inferior segment of the symbol
    Vector3d nVAOr = nVAO.Reversed();
    Point3d sgP21 = pDaxe.Translated(uVAO.Multiplied(2.).Added(nVAOr.Multiplied(2.)));
    Point3d sgP22 = sgP21.Translated(uVAO.Multiplied(8.));

    aPrims->AddVertex(sgP21);
    aPrims->AddVertex(sgP22);

    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
  }
  //==============================================================
  //  OTHER CASES                                                 :
  //==============================================================

  else
  {
    Point3d ProjOffsetPoint      = ElCLib1::Value(ElCLib1::Parameter(aAxis, OffsetPoint), aAxis);
    Point3d ProjAttachmentPoint1 = ElCLib1::Value(ElCLib1::Parameter(aAxis, AttachmentPoint1), aAxis);
    Vector3d PjAtt1_Att1(ProjAttachmentPoint1, AttachmentPoint1);
    Point3d P1 = ProjOffsetPoint.Translated(PjAtt1_Att1);
    Point3d P2 = ProjOffsetPoint.Translated(PjAtt1_Att1.Reversed());

    gp_Lin        L3 = gce_MakeLin(P1, P2);
    Standard_Real parmin, parmax, parcur;
    parmin             = ElCLib1::Parameter(L3, P1);
    parmax             = parmin;
    parcur             = ElCLib1::Parameter(L3, P2);
    Standard_Real dist = Abs(parmin - parcur);
    if (parcur < parmin)
      parmin = parcur;
    if (parcur > parmax)
      parmax = parcur;
    parcur = ElCLib1::Parameter(L3, OffsetPoint);

    Standard_Boolean outside = Standard_False;
    if (parcur < parmin)
    {
      parmin  = parcur;
      outside = Standard_True;
    }
    if (parcur > parmax)
    {
      parmax  = parcur;
      outside = Standard_True;
    }
    Point3d PointMin = ElCLib1::Value(parmin, L3);
    Point3d PointMax = ElCLib1::Value(parmax, L3);

    //==== PROCESSING OF FACE ===========
    aPresentation->NewGroup();
    aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

    Handle(Graphic3d_ArrayOfPrimitives) aPrims = new Graphic3d_ArrayOfSegments(6);

    aPrims->AddVertex(PointMin);
    aPrims->AddVertex(PointMax);

    //==== PROCESSING OF CALL 1 =====
    aPrims->AddVertex(AttachmentPoint1);
    aPrims->AddVertex(P1);

    //==== PROCESSING OF CALL 2 =====
    aPrims->AddVertex(AttachmentPoint2);
    aPrims->AddVertex(P2);

    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

    //==== ARROWS ================
    if (dist < (LA->ArrowAspect()->Length() + LA->ArrowAspect()->Length()))
      outside = Standard_True;
    Dir3d arrdir = L3.Direction().Reversed();
    if (outside)
      arrdir.Reverse();
    // arrow 1 ----
    Arrow1::Draw1(aPresentation->CurrentGroup(),
                      P1,
                      arrdir,
                      LA->ArrowAspect()->Angle(),
                      LA->ArrowAspect()->Length());

    // arrow 2 ----
    Arrow1::Draw1(aPresentation->CurrentGroup(),
                      P2,
                      arrdir.Reversed(),
                      LA->ArrowAspect()->Angle(),
                      LA->ArrowAspect()->Length());

    //==== POINTS ================
    // Marker of localization of attachment points:
    aPresentation->NewGroup();
    aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

    Color1                   aColor = LA->LineAspect()->Aspect()->Color();
    Handle(Graphic3d_AspectMarker3d) aMarkerAspAtt =
      new Graphic3d_AspectMarker3d(Aspect_TOM_O, aColor, 1.0);
    aPresentation->CurrentGroup()->SetPrimitivesAspect(aMarkerAspAtt);
    Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints1 = new Graphic3d_ArrayOfPoints(1);
    anArrayOfPoints1->AddVertex(AttachmentPoint1.X(), AttachmentPoint1.Y(), AttachmentPoint1.Z());
    aPresentation->CurrentGroup()->AddPrimitiveArray(anArrayOfPoints1);

    aPresentation->NewGroup();
    aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());
    aPresentation->CurrentGroup()->SetPrimitivesAspect(aMarkerAspAtt);
    Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints2 = new Graphic3d_ArrayOfPoints(1);
    anArrayOfPoints2->AddVertex(AttachmentPoint2.X(), AttachmentPoint2.Y(), AttachmentPoint2.Z());
    aPresentation->CurrentGroup()->AddPrimitiveArray(anArrayOfPoints2);

    //-------------------------------------------------------------------------------------
    //|                                SYMBOL OF SYMMETRY                                 |
    //-------------------------------------------------------------------------------------

    //           -------    : Superior Segment1
    //         -----------  : Axis
    //           -------    : Inferior Segment1

    Vector3d vec(P1, P2);
    Vector3d vecA = vec.Multiplied(.1);

    Dir3d DirAxis = aAxis.Direction();
    Vector3d Vaxe(DirAxis);
    Vector3d vecB = Vaxe.Multiplied(vecA.Magnitude());
    vecB.Multiply(.5);

    Point3d pm   = P1.Translated(vec.Multiplied(.5));
    Point3d pOff = OffsetPoint.Translated(vecB);

    // Calculate the extremas of the axis of the symbol
    Vector3d vecAxe = vecA.Multiplied(.7);

    aPresentation->NewGroup();
    aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

    aPrims = new Graphic3d_ArrayOfPolylines(13, 5);

    aPrims->AddBound(2);
    aPrims->AddVertex(pOff.Translated(vecAxe));
    aPrims->AddVertex(pOff.Translated(vecAxe.Reversed()));

    // Calculate the extremas of the superior segment of the symbol
    Vector3d vec1 = vecAxe.Multiplied(.6);
    vecAxe      = Vaxe.Multiplied(vecAxe.Magnitude());
    Vector3d vec2 = vecAxe.Multiplied(.4);

    aPrims->AddBound(2);
    aPrims->AddVertex(pOff.Translated(vec1.Added(vec2)));
    aPrims->AddVertex(pOff.Translated(vec1.Reversed().Added(vec2)));

    // Calculate the extremas of the inferior segment of the symbol
    aPrims->AddBound(2);
    aPrims->AddVertex(pOff.Translated(vec1.Added(vec2.Reversed())));
    aPrims->AddVertex(pOff.Translated(vec1.Reversed().Added(vec2.Reversed())));

    /*--------------------------------------------------------------------------------------
    |                          MARKING OF THE AXIS OF SYMMETRY                             |
    ----------------------------------------------------------------------------------------
            ____
            \  / :Cursor
             \/
             /\
            /__\
    */
    Standard_Real Dist           = P1.Distance(P2) / 75;
    Dir3d        aDirectionAxis = aAxis.Direction();
    Vector3d        vs(aDirectionAxis);
    Vector3d        vsym(vs.Divided(vs.Magnitude()).Multiplied(Dist).XYZ());
    Vector3d        vsymper(vsym.Y(), -vsym.X(), vsym.Z());

    aPrims->AddBound(5);
    Point3d pm1 = pm.Translated(vsym.Added(vsymper));
    aPrims->AddVertex(pm1);
    pm1 = pm1.Translated(vsym.Reversed().Multiplied(2).Added(vsymper.Reversed().Multiplied(2)));
    aPrims->AddVertex(pm1);
    pm1 = pm1.Translated(vsymper.Multiplied(2));
    aPrims->AddVertex(pm1);
    pm1 = pm1.Translated(vsym.Multiplied(2).Added(vsymper.Reversed().Multiplied(2)));
    aPrims->AddVertex(pm1);
    pm1 = pm1.Translated(vsymper.Multiplied(2));
    aPrims->AddVertex(pm1);

    vsym.Multiply(4);

    aPrims->AddBound(2);
    aPrims->AddVertex(pm.Translated(vsym));
    aPrims->AddVertex(pm.Translated(vsym.Reversed()));

    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
  }
}
