// Copyright (c) 1998-1999 Matra Datavision
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

#include <DsgPrs.hxx>

#include <Aspect_TypeOfMarker.hxx>
#include <ElCLib.hxx>
#include <gce_MakeLin.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <gp_Ax1.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Elips.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Graphic3d_ArrayOfPoints.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Graphic3d_AspectMarker3d.hxx>
#include <Graphic3d_Group.hxx>
#include <Precision.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_DimensionAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Presentation.hxx>
#include <Quantity_Color.hxx>

void DsgPrs1::ComputeSymbol(const Handle(Prs3d_Presentation)&    aPresentation,
                           const Handle(Prs3d_DimensionAspect)& LA,
                           const Point3d&                        pt1,
                           const Point3d&                        pt2,
                           const Dir3d&                        dir1,
                           const Dir3d&                        dir2,
                           const DsgPrs_ArrowSide               ArrowSide,
                           const Standard_Boolean               drawFromCenter)
{
  Handle(Graphic3d_Group) aGroup = aPresentation->NewGroup();

  Color1                   aColor = LA->LineAspect()->Aspect()->Color();
  Handle(Graphic3d_AspectMarker3d) aMarkerAsp =
    new Graphic3d_AspectMarker3d(Aspect_TOM_O, aColor, 1.0);
  aGroup->SetGroupPrimitivesAspect(LA->LineAspect()->Aspect());

  switch (ArrowSide)
  {
    case DsgPrs_AS_NONE: {
      break;
    }
    case DsgPrs_AS_FIRSTAR: {
      Arrow1::Draw1(aGroup, pt1, dir1, LA->ArrowAspect()->Angle(), LA->ArrowAspect()->Length());
      break;
    }
    case DsgPrs_AS_LASTAR: {

      Arrow1::Draw1(aGroup, pt2, dir2, LA->ArrowAspect()->Angle(), LA->ArrowAspect()->Length());
      break;
    }

    case DsgPrs_AS_BOTHAR: {
      Arrow1::Draw1(aGroup, pt1, dir1, LA->ArrowAspect()->Angle(), LA->ArrowAspect()->Length());
      Arrow1::Draw1(aGroup, pt2, dir2, LA->ArrowAspect()->Angle(), LA->ArrowAspect()->Length());

      break;
    }

    case DsgPrs_AS_FIRSTPT: {
      if (drawFromCenter)
      {
        Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints = new Graphic3d_ArrayOfPoints(1);
        anArrayOfPoints->AddVertex(pt1.X(), pt1.Y(), pt1.Z());
        aPresentation->CurrentGroup()->AddPrimitiveArray(anArrayOfPoints);
      }
      break;
    }

    case DsgPrs_AS_LASTPT: {
      // On dessine un rond
      Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints = new Graphic3d_ArrayOfPoints(1);
      anArrayOfPoints->AddVertex(pt2.X(), pt2.Y(), pt2.Z());
      aPresentation->CurrentGroup()->AddPrimitiveArray(anArrayOfPoints);
      break;
    }

    case DsgPrs_AS_BOTHPT: {
      if (drawFromCenter)
      {
        Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints1 = new Graphic3d_ArrayOfPoints(2);
        anArrayOfPoints1->AddVertex(pt1.X(), pt1.Y(), pt1.Z());
        anArrayOfPoints1->AddVertex(pt2.X(), pt2.Y(), pt2.Z());
        aGroup->SetGroupPrimitivesAspect(aMarkerAsp);
        aGroup->AddPrimitiveArray(anArrayOfPoints1);
      }
      break;
    }

    case DsgPrs_AS_FIRSTAR_LASTPT: {
      // an Arrow2
      Arrow1::Draw1(aGroup, pt1, dir1, LA->ArrowAspect()->Angle(), LA->ArrowAspect()->Length());
      // a Round
      Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints = new Graphic3d_ArrayOfPoints(1);
      anArrayOfPoints->AddVertex(pt2.X(), pt2.Y(), pt2.Z());
      aGroup->SetPrimitivesAspect(aMarkerAsp);
      aGroup->AddPrimitiveArray(anArrayOfPoints);
      break;
    }

    case DsgPrs_AS_FIRSTPT_LASTAR: {
      // an Arrow2
      Arrow1::Draw1(aGroup, pt2, dir2, LA->ArrowAspect()->Angle(), LA->ArrowAspect()->Length());

      // a Round
      if (drawFromCenter)
      {
        Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints = new Graphic3d_ArrayOfPoints(1);
        anArrayOfPoints->AddVertex(pt1.X(), pt1.Y(), pt1.Z());
        aGroup->SetPrimitivesAspect(aMarkerAsp);
        aGroup->AddPrimitiveArray(anArrayOfPoints);
      }
      break;
    }
  }
}

//=================================================================================================

void DsgPrs1::ComputePlanarFacesLengthPresentation(const Standard_Real FirstArrowLength,
                                                  const Standard_Real SecondArrowLength,
                                                  const Point3d&       AttachmentPoint1,
                                                  const Point3d&       AttachmentPoint2,
                                                  const Dir3d&       DirAttach,
                                                  const Point3d&       OffsetPoint,
                                                  const gp_Pln&       PlaneOfFaces,
                                                  Point3d&             EndOfArrow1,
                                                  Point3d&             EndOfArrow2,
                                                  Dir3d&             DirOfArrow1)
{
  gp_Lin FirstLin(AttachmentPoint1, DirAttach);
  gp_Lin SecondLin(AttachmentPoint2, DirAttach);

  EndOfArrow1 = ElCLib1::Value(ElCLib1::Parameter(FirstLin, OffsetPoint), FirstLin);
  EndOfArrow2 = ElCLib1::Value(ElCLib1::Parameter(SecondLin, OffsetPoint), SecondLin);

  if (EndOfArrow1.SquareDistance(EndOfArrow2) > Precision1::SquareConfusion()) // not null length
  {
    Dir3d LengthDir(Vector3d(EndOfArrow1, EndOfArrow2));
    if ((FirstArrowLength + SecondArrowLength) * (FirstArrowLength + SecondArrowLength)
        < EndOfArrow1.SquareDistance(EndOfArrow2))
      DirOfArrow1 = -LengthDir;
    else
      DirOfArrow1 = LengthDir;
  }
  else // null length
    DirOfArrow1 = PlaneOfFaces.Axis().Direction();
}

//=================================================================================================

void DsgPrs1::ComputeCurvilinearFacesLengthPresentation(const Standard_Real FirstArrowLength,
                                                       const Standard_Real SecondArrowLength,
                                                       const Handle(GeomSurface)& SecondSurf,
                                                       const Point3d&               AttachmentPoint1,
                                                       const Point3d&               AttachmentPoint2,
                                                       const Dir3d&               DirAttach,
                                                       Point3d&                     EndOfArrow2,
                                                       Dir3d&                     DirOfArrow1,
                                                       Handle(GeomCurve3d)&         VCurve,
                                                       Handle(GeomCurve3d)&         UCurve,
                                                       Standard_Real&              FirstU,
                                                       Standard_Real&              deltaU,
                                                       Standard_Real&              FirstV,
                                                       Standard_Real&              deltaV)
{
  PointOnSurfProjector  ProjectorOnSurface;
  GeomAPI_ProjectPointOnCurve ProjectorOnCurve;
  Standard_Real               U1, V1, U2, V2;
  Standard_Real               LastU, LastV;
  constexpr Standard_Real     SquareTolerance = Precision1::SquareConfusion();

  ProjectorOnSurface.Init(AttachmentPoint1, SecondSurf);
  Standard_Integer Index(1);
  Standard_Real    MinDist = RealLast();
  Standard_Real    LocalU, LocalV;
  Vector3d           D1U, D1V;
  Dir3d           LocalDir;
  for (Standard_Integer i = 1; i <= ProjectorOnSurface.NbPoints(); i++)
  {
    ProjectorOnSurface.Parameters(i, LocalU, LocalV);

    SecondSurf->D1(LocalU, LocalV, EndOfArrow2, D1U, D1V);
    if (D1U.SquareMagnitude() <= SquareTolerance || D1V.SquareMagnitude() <= SquareTolerance)
      LocalDir = Dir3d(Vector3d(AttachmentPoint1, ProjectorOnSurface.Point(i)));
    else
      LocalDir = Dir3d(D1U ^ D1V);
    if (DirAttach.IsParallel(LocalDir, Precision1::Angular())
        && ProjectorOnSurface.Distance(i) < MinDist)
    {
      Index   = i;
      MinDist = ProjectorOnSurface.Distance(i);
    }
  }
  EndOfArrow2 = ProjectorOnSurface.Point(Index);
  ProjectorOnSurface.Parameters(Index, U1, V1);

  if ((FirstArrowLength + SecondArrowLength) * (FirstArrowLength + SecondArrowLength)
      < AttachmentPoint1.SquareDistance(EndOfArrow2))
    DirOfArrow1 = -DirAttach;
  else
    DirOfArrow1 = DirAttach;

  if (EndOfArrow2.SquareDistance(AttachmentPoint2) > Precision1::SquareConfusion())
  {
    VCurve = SecondSurf->VIso(V1);
    ProjectorOnCurve.Init(EndOfArrow2, VCurve);
    FirstU = ProjectorOnCurve.LowerDistanceParameter();

    ProjectorOnSurface.Init(AttachmentPoint2, SecondSurf);
    ProjectorOnSurface.LowerDistanceParameters(U2, V2);
    UCurve = SecondSurf->UIso(U2);

    ProjectorOnCurve.Init(AttachmentPoint2, UCurve);
    LastV = ProjectorOnCurve.LowerDistanceParameter();

    Point3d Intersection = SecondSurf->Value(U2, V1);
    ProjectorOnCurve.Init(Intersection, VCurve);
    LastU = ProjectorOnCurve.LowerDistanceParameter();
    ProjectorOnCurve.Init(Intersection, UCurve);
    FirstV = ProjectorOnCurve.LowerDistanceParameter();

    deltaU = LastU - FirstU;
    deltaV = LastV - FirstV;

    if (VCurve->IsPeriodic() && Abs(deltaU) > VCurve->Period() / 2)
    {
      Standard_Real Sign = (deltaU > 0.0) ? -1.0 : 1.0;
      deltaU             = VCurve->Period() - Abs(deltaU);
      deltaU *= Sign;
    }
    if (UCurve->IsPeriodic() && Abs(deltaV) > UCurve->Period() / 2)
    {
      Standard_Real Sign = (deltaV > 0.0) ? -1.0 : 1.0;
      deltaV             = UCurve->Period() - Abs(deltaV);
      deltaV *= Sign;
    }
  }
}

//=================================================================================================

void DsgPrs1::ComputeFacesAnglePresentation(const Standard_Real    ArrowLength,
                                           const Standard_Real    Value,
                                           const Point3d&          CenterPoint,
                                           const Point3d&          AttachmentPoint1,
                                           const Point3d&          AttachmentPoint2,
                                           const Dir3d&          dir1,
                                           const Dir3d&          dir2,
                                           const Dir3d&          axisdir,
                                           const Standard_Boolean isPlane,
                                           const Axis3d&          AxisOfSurf,
                                           const Point3d&          OffsetPoint,
                                           gp_Circ&               AngleCirc,
                                           Standard_Real&         FirstParAngleCirc,
                                           Standard_Real&         LastParAngleCirc,
                                           Point3d&                EndOfArrow1,
                                           Point3d&                EndOfArrow2,
                                           Dir3d&                DirOfArrow1,
                                           Dir3d&                DirOfArrow2,
                                           Point3d&                ProjAttachPoint2,
                                           gp_Circ&               AttachCirc,
                                           Standard_Real&         FirstParAttachCirc,
                                           Standard_Real&         LastParAttachCirc)
{
  if (Value > Precision1::Angular() && Abs(M_PI - Value) > Precision1::Angular())
  {
    // Computing presentation of angle's arc
    Frame3d ax(CenterPoint, axisdir, dir1);
    AngleCirc.SetPosition(ax);
    AngleCirc.SetRadius(CenterPoint.Distance(OffsetPoint));
    Vector3d vec1(dir1);
    vec1 *= AngleCirc.Radius();
    Point3d p1 = CenterPoint.Translated(vec1);
    Vector3d vec2(dir2);
    vec2 *= AngleCirc.Radius();
    Point3d p2 = CenterPoint.Translated(vec2);

    Standard_Real Par1 = 0.;
    Standard_Real Par2 = ElCLib1::Parameter(AngleCirc, p2);
    Standard_Real Par0 = ElCLib1::Parameter(AngleCirc, OffsetPoint);

    Vector3d PosVec(CenterPoint, OffsetPoint);
    Vector3d NormalOfPlane = vec1 ^ vec2;

    Vector3d           Normal1 = NormalOfPlane ^ vec1;
    Vector3d           Normal2 = NormalOfPlane ^ vec2;
    Standard_Integer Sign1   = (PosVec * Normal1 >= 0) ? 1 : -1;
    Standard_Integer Sign2   = (PosVec * Normal2 >= 0) ? 1 : -1;
    if (Sign1 == 1 && Sign2 == -1)
    {
      FirstParAngleCirc = Par1;
      LastParAngleCirc  = Par2;
    }
    else if (Sign1 == 1 && Sign2 == 1)
    {
      FirstParAngleCirc = Par1;
      LastParAngleCirc  = Par0;
    }
    else if (Sign1 == -1 && Sign2 == 1)
    {
      Par1 += M_PI;
      Par2 += M_PI;
      FirstParAngleCirc = Par1;
      LastParAngleCirc  = Par2;
    }
    else // Sign1 == -1 && Sign2 == -1
    {
      AngleCirc.SetPosition(Frame3d(CenterPoint, axisdir, Dir3d(PosVec)));
      Par0              = 0.;
      Par1              = ElCLib1::Parameter(AngleCirc, p1);
      Par2              = ElCLib1::Parameter(AngleCirc, p2);
      FirstParAngleCirc = Par0;
      LastParAngleCirc  = Par2;
    }

    // Computing presentation of arrows
    EndOfArrow1        = ElCLib1::Value(Par1, AngleCirc);
    EndOfArrow2        = ElCLib1::Value(Par2, AngleCirc);
    Standard_Real beta = 0.;
    if (AngleCirc.Radius() > Precision1::Confusion())
      beta = ArrowLength / AngleCirc.Radius();
    Point3d OriginOfArrow1 = ElCLib1::Value(Par1 + beta, AngleCirc);
    Point3d OriginOfArrow2 = ElCLib1::Value(Par2 - beta, AngleCirc);
    DirOfArrow1           = Dir3d(Vector3d(OriginOfArrow1, EndOfArrow1));
    DirOfArrow2           = Dir3d(Vector3d(OriginOfArrow2, EndOfArrow2));
    if (EndOfArrow1.SquareDistance(EndOfArrow2)
        <= (ArrowLength + ArrowLength) * (ArrowLength + ArrowLength))
    {
      DirOfArrow1.Reverse();
      DirOfArrow2.Reverse();
    }
  }
  else // dir1 and dir2 are parallel
  {
    Dir3d ArrowDir = axisdir ^ dir1;
    DirOfArrow1     = ArrowDir;
    DirOfArrow2     = -ArrowDir;
    gp_Lin DirLine(AttachmentPoint1, dir1);
    EndOfArrow1 = ElCLib1::Value(ElCLib1::Parameter(DirLine, OffsetPoint), DirLine);
    EndOfArrow2 = EndOfArrow1;
  }

  // Line or arc from AttachmentPoint2 to its "projection"
  gp_Lin SecondLin(CenterPoint, dir2);
  if (SecondLin.Contains(AttachmentPoint2, Precision1::Confusion()))
    ProjAttachPoint2 = AttachmentPoint2;
  else
  {
    if (isPlane)
      ProjAttachPoint2 = ElCLib1::Value(ElCLib1::Parameter(SecondLin, AttachmentPoint2), SecondLin);
    else
    {
      gp_Lin LineOfAxis(AxisOfSurf);
      Point3d CenterOfArc =
        ElCLib1::Value(ElCLib1::Parameter(LineOfAxis, AttachmentPoint2), LineOfAxis);

      Frame3d Ax2(CenterOfArc,
                 AxisOfSurf.Direction(),
                 Dir3d(Vector3d(CenterOfArc, AttachmentPoint2)));
      AttachCirc.SetPosition(Ax2);
      AttachCirc.SetRadius(CenterOfArc.Distance(AttachmentPoint2));

      GeomAPI_ExtremaCurveCurve Intersection(new GeomCircle(AttachCirc), new GeomLine(SecondLin));
      Intersection.NearestPoints(ProjAttachPoint2, ProjAttachPoint2);

      Standard_Real U2 = ElCLib1::Parameter(AttachCirc, ProjAttachPoint2);
      if (U2 <= M_PI)
      {
        FirstParAttachCirc = 0;
        LastParAttachCirc  = U2;
      }
      else
      {
        FirstParAttachCirc = U2;
        LastParAttachCirc  = 2 * M_PI;
      }
    }
  }
}

void DsgPrs1::ComputeFilletRadiusPresentation(const Standard_Real /*ArrowLength*/,
                                             const Standard_Real    Value,
                                             const Point3d&          Position1,
                                             const Dir3d&          NormalDir,
                                             const Point3d&          FirstPoint,
                                             const Point3d&          SecondPoint,
                                             const Point3d&          Center,
                                             const Point3d&          BasePnt,
                                             const Standard_Boolean drawRevers,
                                             Standard_Boolean&      SpecCase,
                                             gp_Circ&               FilletCirc,
                                             Standard_Real&         FirstParCirc,
                                             Standard_Real&         LastParCirc,
                                             Point3d&                EndOfArrow,
                                             Dir3d&                DirOfArrow,
                                             Point3d&                DrawPosition)
{
  Dir3d        dir1(Vector3d(Center, FirstPoint));
  Dir3d        dir2(Vector3d(Center, SecondPoint));
  Standard_Real Angle = dir1.Angle(dir2);
  if (Angle <= Precision1::Angular() || (M_PI - Angle) <= Precision1::Angular()
      || Value <= Precision1::Confusion())
    SpecCase = Standard_True;
  else
    SpecCase = Standard_False;
  if (!SpecCase)
  {
    // Computing presentation of fillet's arc
    Frame3d ax(Center, NormalDir, dir1);
    FilletCirc.SetPosition(ax);
    FilletCirc.SetRadius(Center.Distance(FirstPoint)); //***
    Vector3d vec1(dir1);
    vec1 *= FilletCirc.Radius();
    Vector3d vec2(dir2);
    vec2 *= FilletCirc.Radius();
    Vector3d PosVec;
    if (!Center.IsEqual(Position1, Precision1::Confusion()))
      PosVec.SetXYZ(Vector3d(Center, Position1).XYZ());
    else
      PosVec.SetXYZ((vec1.Added(vec2)).XYZ());
    Vector3d           NormalOfPlane = vec1 ^ vec2;
    Vector3d           Normal1       = NormalOfPlane ^ vec1;
    Vector3d           Normal2       = NormalOfPlane ^ vec2;
    Standard_Integer Sign1         = (PosVec * Normal1 >= 0) ? 1 : -1;
    Standard_Integer Sign2         = (PosVec * Normal2 >= 0) ? 1 : -1;
    gp_Lin           L1(Center, dir1);
    gp_Lin           L2(Center, dir2);
    if (Sign1 != Sign2)
    {
      DrawPosition = Position1; //***
      Dir3d        direction(PosVec);
      Standard_Real angle = dir1.Angle(direction);
      if ((dir1 ^ direction) * NormalDir < 0.0e0)
        angle = -angle;
      if (Sign1 == -1)
        angle += M_PI;
      EndOfArrow = ElCLib1::Value(angle, FilletCirc); //***
    }
    else
    {
      if (L1.Distance(Position1) < L2.Distance(Position1))
      {
        EndOfArrow   = FirstPoint; //***
        DrawPosition = ElCLib1::Value(ElCLib1::Parameter(L1, Position1), L1);
      }
      else
      {
        EndOfArrow   = SecondPoint; //***
        DrawPosition = ElCLib1::Value(ElCLib1::Parameter(L2, Position1), L2);
      }
    }
    if ((dir1 ^ dir2).IsOpposite(NormalDir, Precision1::Angular()))
    {
      Dir3d newdir = NormalDir.Reversed();
      Frame3d axnew(Center, newdir, dir1);
      FilletCirc.SetPosition(axnew);
    }
    FirstParCirc = ElCLib1::Parameter(FilletCirc, FirstPoint);
    LastParCirc  = ElCLib1::Parameter(FilletCirc, SecondPoint);
  }
  else // Angle equal 0 or PI or R = 0
  {
    DrawPosition = Position1;
    EndOfArrow   = BasePnt;
  }

  if (drawRevers)
  {
    Vector3d Vd(DrawPosition, EndOfArrow);
    DrawPosition.Translate(Vd * 2);
  }
  DirOfArrow.SetXYZ(Dir3d(Vector3d(DrawPosition, EndOfArrow)).XYZ());
}

//=================================================================================================

void DsgPrs1::ComputeRadiusLine(const Point3d&          aCenter,
                               const Point3d&          anEndOfArrow,
                               const Point3d&          aPosition,
                               const Standard_Boolean drawFromCenter,
                               Point3d&                aRadLineOrign,
                               Point3d&                aRadLineEnd)
{
  if (drawFromCenter)
  {
    gp_Lin        RadiusLine    = gce_MakeLin(aCenter, anEndOfArrow);
    Standard_Real PosParOnLine  = ElCLib1::Parameter(RadiusLine, aPosition);
    Standard_Real EndOfArrowPar = ElCLib1::Parameter(RadiusLine, anEndOfArrow);
    if (PosParOnLine < 0.0)
    {
      aRadLineOrign = aPosition;
      aRadLineEnd   = anEndOfArrow;
    }
    else if (PosParOnLine > EndOfArrowPar)
    {
      aRadLineOrign = aPosition;
      aRadLineEnd   = aCenter;
    }
    else
    {
      aRadLineOrign = aCenter;
      aRadLineEnd   = anEndOfArrow;
    }
  }
  else
  {
    aRadLineOrign = aPosition;
    aRadLineEnd   = anEndOfArrow;
  }
}

//=================================================================================================

Standard_Real DsgPrs1::DistanceFromApex(const gp_Elips&     elips,
                                       const Point3d&       Apex,
                                       const Standard_Real par)
{
  Standard_Real dist;
  Standard_Real parApex = ElCLib1::Parameter(elips, Apex);
  if (parApex == 0.0 || parApex == M_PI)
  {                     // Major case
    if (parApex == 0.0) // pos Apex
      dist = (par < M_PI) ? par : (2 * M_PI - par);
    else // neg Apex
      dist = (par < M_PI) ? (M_PI - par) : (par - M_PI);
  }
  else
  {                          // Minor case
    if (parApex == M_PI / 2) // pos Apex
    {
      if (par <= parApex + M_PI && par > parApex)
        dist = par - parApex;
      else
      {
        if (par > parApex + M_PI)
          dist = 2 * M_PI - par + parApex;
        else
          dist = parApex - par; // 0 < par < M_PI/2
      }
    }
    else // neg Apex == 3/2 PI
    {
      if (par <= parApex && par >= M_PI / 2)
        dist = parApex - par;
      else
      {
        if (par > parApex)
          dist = par - parApex;
        else
          dist = par + M_PI / 2; // 0 < par < PI/2
      }
    }
  }
  return dist;
}
