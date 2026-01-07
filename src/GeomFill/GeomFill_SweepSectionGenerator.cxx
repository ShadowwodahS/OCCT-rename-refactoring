// Created on: 1994-02-28
// Created by: Bruno DUMORTIER
// Copyright (c) 1994-1999 Matra Datavision
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

#include <Adaptor3d_Curve.hxx>
#include <ElCLib.hxx>
#include <GCPnts_QuasiUniformDeflection.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomConvert.hxx>
#include <GeomFill_Profiler.hxx>
#include <GeomFill_SweepSectionGenerator.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <Standard_RangeError.hxx>
#include <TColStd_Array1OfReal.hxx>

#include <stdio.h>
#ifdef DRAW
  #include <DrawTrSurf.hxx>
  #include <Geom_BSplineCurve.hxx>
static Standard_Boolean Affich     = Standard_False;
static Standard_Integer NbSECTIONS = 0;
#endif

//=================================================================================================

GeomFill_SweepSectionGenerator::GeomFill_SweepSectionGenerator()
    : myRadius(0.0),
      myIsDone(Standard_False),
      myNbSections(0),
      myType(-1),
      myPolynomial(Standard_False)
{
}

//=================================================================================================

GeomFill_SweepSectionGenerator::GeomFill_SweepSectionGenerator(const Handle(GeomCurve3d)& Path,
                                                               const Standard_Real       Radius)
{
  Init(Path, Radius);
}

//=================================================================================================

GeomFill_SweepSectionGenerator::GeomFill_SweepSectionGenerator(const Handle(GeomCurve3d)& Path,
                                                               const Handle(GeomCurve3d)& FirstSect)
{
  Init(Path, FirstSect);
}

//=================================================================================================

GeomFill_SweepSectionGenerator::GeomFill_SweepSectionGenerator(const Handle(GeomCurve3d)& Path,
                                                               const Handle(GeomCurve3d)& FirstSect,
                                                               const Handle(GeomCurve3d)& LastSect)
{
  Init(Path, FirstSect, LastSect);
}

//=================================================================================================

GeomFill_SweepSectionGenerator::GeomFill_SweepSectionGenerator(
  const Handle(Adaptor3d_Curve)& Path,
  const Handle(Adaptor3d_Curve)& Curve1,
  const Handle(Adaptor3d_Curve)& Curve2,
  const Standard_Real            Radius)
{
  Init(Path, Curve1, Curve2, Radius);
}

//=================================================================================================

void GeomFill_SweepSectionGenerator::Init(const Handle(GeomCurve3d)& Path,
                                          const Standard_Real       Radius)
{
  myIsDone = Standard_False;
  myRadius = Radius;
  GeomAdaptor_Curve ThePath(Path);

  if (ThePath.GetType() == GeomAbs_Circle)
  {

    myCircPathAxis = ThePath.Circle().Axis();
    myType         = 4;
  }
  else
    myType = 1;
  if (Path->IsKind(STANDARD_TYPE(BSplineCurve3d)))
  {
    myPath = Handle(BSplineCurve3d)::DownCast(Path->Copy());
  }
  else
  {
    myPath = GeomConvert::CurveToBSplineCurve(Path);
  }
}

//=================================================================================================

void GeomFill_SweepSectionGenerator::Init(const Handle(GeomCurve3d)& Path,
                                          const Handle(GeomCurve3d)& FirstSect)
{
  myIsDone = Standard_False;
  myRadius = 0;
  GeomAdaptor_Curve ThePath(Path);

  if (ThePath.GetType() == GeomAbs_Circle)
  {
    myCircPathAxis = ThePath.Circle().Axis();
    myType         = 5;
  }

  else
    myType = 2;

  if (Path->IsKind(STANDARD_TYPE(BSplineCurve3d)))
  {
    myPath = Handle(BSplineCurve3d)::DownCast(Path->Copy());
  }
  else
  {
    myPath = GeomConvert::CurveToBSplineCurve(Path);
  }
  if (FirstSect->IsKind(STANDARD_TYPE(BSplineCurve3d)))
  {
    myFirstSect = Handle(BSplineCurve3d)::DownCast(FirstSect->Copy());
  }
  else
  {
    // JAG
    myFirstSect = GeomConvert::CurveToBSplineCurve(FirstSect, Convert_QuasiAngular);
  }
  if (myFirstSect->IsPeriodic())
    myFirstSect->SetNotPeriodic();
}

//=================================================================================================

void GeomFill_SweepSectionGenerator::Init(const Handle(GeomCurve3d)& Path,
                                          const Handle(GeomCurve3d)& FirstSect,
                                          const Handle(GeomCurve3d)& LastSect)
{
  myIsDone = Standard_False;
  myRadius = 0;
  GeomAdaptor_Curve ThePath(Path);

  if (ThePath.GetType() == GeomAbs_Circle)
  {

    myCircPathAxis = ThePath.Circle().Axis();
    myType         = 6;
  }
  else
    myType = 3;

  if (Path->IsKind(STANDARD_TYPE(BSplineCurve3d)))
  {
    myPath = Handle(BSplineCurve3d)::DownCast(Path->Copy());
  }
  else
  {
    myPath = GeomConvert::CurveToBSplineCurve(Path);
  }

  // JAG
  if (FirstSect->IsKind(STANDARD_TYPE(BSplineCurve3d)))
  {
    myFirstSect = Handle(BSplineCurve3d)::DownCast(FirstSect->Copy());
  }
  else
  {
    myFirstSect = GeomConvert::CurveToBSplineCurve(FirstSect, Convert_QuasiAngular);
  }
  if (LastSect->IsKind(STANDARD_TYPE(BSplineCurve3d)))
  {
    myLastSect = Handle(BSplineCurve3d)::DownCast(LastSect->Copy());
  }
  else
  {
    myLastSect = GeomConvert::CurveToBSplineCurve(LastSect, Convert_QuasiAngular);
  }

  if (myFirstSect->IsPeriodic())
    myFirstSect->SetNotPeriodic();
  if (myLastSect->IsPeriodic())
    myLastSect->SetNotPeriodic();

  // JAG

  GeomFill_Profiler Profil;
  Profil.AddCurve(myFirstSect);
  Profil.AddCurve(myLastSect);
  Profil.Perform(Precision::Confusion());

  myFirstSect = Handle(BSplineCurve3d)::DownCast(Profil.Curve(1));
  myLastSect  = Handle(BSplineCurve3d)::DownCast(Profil.Curve(2));
}

//=================================================================================================

void GeomFill_SweepSectionGenerator::Init(const Handle(Adaptor3d_Curve)& Path,
                                          const Handle(Adaptor3d_Curve)& Curve1,
                                          const Handle(Adaptor3d_Curve)& Curve2,
                                          const Standard_Real            Radius)
{
  myIsDone = Standard_False;
  myRadius = Radius;
  myType   = 0;

  Handle(GeomCurve3d) CC = GeomAdaptor1::MakeCurve(*Path);
  myPath                = GeomConvert::CurveToBSplineCurve(CC);
  myAdpPath             = Path;
  myAdpFirstSect        = Curve1;
  myAdpLastSect         = Curve2;
}

//=================================================================================================

void GeomFill_SweepSectionGenerator::Perform(const Standard_Boolean Polynomial)
{
  myPolynomial = Polynomial;

  // eval myNbSections.
  Standard_Integer NSpans = myPath->NbKnots() - 1;

  myNbSections = 21 * NSpans;

  Standard_Real U;

  Standard_Real U1 = myPath->FirstParameter();
  Standard_Real U2 = myPath->LastParameter();

  GCPnts_QuasiUniformDeflection Samp;
  // Calcul de la longueur approximative de la courbe
  GeomAdaptor_Curve AdpPath(myPath);
  Point3d            P1     = AdpPath.Value(U1);
  Point3d            P2     = AdpPath.Value((U1 + U2) / 2.);
  Point3d            P3     = AdpPath.Value(U2);
  Standard_Real     Length = P1.Distance(P2) + P2.Distance(P3);
  Standard_Real     Fleche = 1.e-5 * Length;
  Samp.Initialize(AdpPath, Fleche);

  if (Samp.IsDone() && (Samp.NbPoints() > myNbSections))
  {
    myNbSections = Samp.NbPoints();
  }
  // the transformations are calculate on differents points of <myPath>
  // corresponding to the path parameter uniformly reparted.
  Standard_Real        DeltaU = (U2 - U1) / (Standard_Real)(myNbSections - 1);
  TColStd_Array1OfReal Parameters(1, myNbSections);
  //  Parameters(1) = U1;
  //  for (Standard_Integer i = 2; i < myNbSections; i++) {
  //    Parameters(i) = U1 + (i-1) * DeltaU;
  //  }
  //  Parameters(myNbSections) = U2;

  Parameters(1) = 0.;
  for (Standard_Integer i = 2; i < myNbSections; i++)
  {
    Parameters(i) = (i - 1) * DeltaU;
  }
  Parameters(myNbSections) = U2 - U1;

  Vector3d  D1Ref, D1;
  Point3d  PRef, P;
  Transform3d TR, cumulTR, Trans;

  myPath->D1(U1, PRef, D1Ref);

  if ((myType == 1) || (myType == 4))
  {
    // We create a circle with radius <myRadius>. This axis is create with
    // main direction <DRef> (first derivate vector of <myPath> on the first
    // point <PRef> ). This circle is, after transform to BSpline curve,
    // put in <myFirstSect>.

    Frame3d CircleAxis(PRef, D1Ref);
    /*
        Handle(GeomCircle) Circ = new GeomCircle( CircleAxis, myRadius);

        myFirstSect = GeomConvert::CurveToBSplineCurve(Circ);
        // le cercle est segmente car AppBlend_AppSurf ne gere
        // pas les courbes periodiques.
        myFirstSect->Segment(0., 2.*M_PI);
    */
    Handle(Geom_TrimmedCurve) Circ =
      new Geom_TrimmedCurve(new GeomCircle(CircleAxis, myRadius), 0., 2. * M_PI);

    myFirstSect = GeomConvert::CurveToBSplineCurve(Circ, Convert_QuasiAngular);
  }

  if (myType <= 3 && myType >= 1)
  {

    for (Standard_Integer i = 2; i <= myNbSections; i++)
    {

      U = Parameters(i) + U1;
      if (i == myNbSections)
        U = U2;

      myPath->D1(U, P, D1);

      // Eval the translation between the (i-1) section and the i-th.
      Trans.SetTranslation(PRef, P);

      Transform3d Rot;
      if (!D1Ref.IsParallel(D1, Precision::Angular()))
      {
        // Eval the Rotation between (i-1) section and the i-th.
        Rot.SetRotation(Axis3d(P, Dir3d(D1Ref ^ D1)), D1Ref.AngleWithRef(D1, D1Ref ^ D1));
      }
      else if (D1Ref.IsOpposite(D1, Precision::Angular()))
#ifdef OCCT_DEBUG
        std::cout << "Que fais-je ???? " << std::endl;
#endif

      // TR is the transformation between (i-1) section and the i-th.
      TR = Rot * Trans;
      // cumulTR is the transformation between <myFirstSec> and
      // the i-th section.
      cumulTR = TR * cumulTR;

      myTrsfs.Append(cumulTR);

      PRef  = P;
      D1Ref = D1;
    }
  }
  else if (myType != 0)
  {
    for (Standard_Integer i = 2; i <= myNbSections; i++)
    {
      cumulTR.SetRotation(myCircPathAxis, Parameters(i));
      myTrsfs.Append(cumulTR);
    }
  }

  myIsDone = Standard_True;
}

//=================================================================================================

void GeomFill_SweepSectionGenerator::GetShape(Standard_Integer& NbPoles,
                                              Standard_Integer& NbKnots,
                                              Standard_Integer& Degree,
                                              Standard_Integer& NbPoles2d) const
{
  /*
   if ( myType == 1) {
      NbPoles   = 7;
      NbKnots   = 4;
      Degree    = 2;
    }
    else {
  */
  if (myType != 0)
  {
    NbPoles = myFirstSect->NbPoles();
    NbKnots = myFirstSect->NbKnots();
    Degree  = myFirstSect->Degree();
  }
  else
  { // myType == 0
    NbPoles = 7;
    NbKnots = 2;
    Degree  = 6;
  }
  NbPoles2d = 0;
}

//=================================================================================================

void GeomFill_SweepSectionGenerator::Knots(TColStd_Array1OfReal& TKnots) const
{
  /*
    if (myType == 1) {
      Standard_Real U = 2.*M_PI/3.;
      for ( Standard_Integer i = 1; i <= 4; i++)
        TKnots(i) = ( i-1) * U;
    }
    else {
  */
  if (myType != 0)
  {
    myFirstSect->Knots(TKnots);
  }
  else
  {
    TKnots(1) = 0.;
    TKnots(2) = 1.;
  }
  //  }
}

//=================================================================================================

void GeomFill_SweepSectionGenerator::Mults(TColStd_Array1OfInteger& TMults) const
{
  /*
    if ( myType == 1) {
      TMults( 1) = TMults( 4) = 3;
      TMults( 2) = TMults( 3) = 2;
    }
    else {
  */
  if (myType != 0)
  {
    myFirstSect->Multiplicities(TMults);
  }
  else
  {
    TMults(1) = TMults(2) = 7;
  }
  //  }
}

//=================================================================================================

Standard_Boolean GeomFill_SweepSectionGenerator::Section(const Standard_Integer P,
                                                         TColgp_Array1OfPnt&    Poles,
                                                         TColgp_Array1OfVec&    DPoles,
                                                         TColgp_Array1OfPnt2d&  Poles2d,
                                                         TColgp_Array1OfVec2d&, // DPoles2d,
                                                         TColStd_Array1OfReal& Weigths,
                                                         TColStd_Array1OfReal& DWeigths) const
{
  Section(P, Poles, Poles2d, Weigths);

  // pour les tuyaux sur aretes pour l'instant on ne calcule pas les derivees
  if (myType == 0)
    return Standard_False; // a voir pour mieux.

  // calcul des derivees sur la surface
  // on calcule les derivees en approximant le path au voisinage du point
  // P(u) par le cercle osculateur au path .

  // calcul du cercle osculateur.

  Standard_Real U;
  if (P == 1)
  {
    U = myPath->FirstParameter();
  }
  else if (P == myNbSections)
  {
    U = myPath->LastParameter();
  }
  else
    return Standard_False;

  Vector3d D1, D2;
  Point3d Pt;

  myPath->D2(U, Pt, D1, D2);
  Standard_Real l = D1.Magnitude();

  if (l < Epsilon(1.))
    return Standard_False;

  Dir3d        T = D1;
  Standard_Real m = D2.Dot(T);
  Vector3d        D = D2 - m * T;
  Standard_Real c = D.Magnitude() / (l * l);

  if (c < Epsilon(1.))
  {
    // null curvature : equivalent to a translation of the section
    for (Standard_Integer i = 1; i <= myFirstSect->NbPoles(); i++)
    {
      DPoles(i) = D1;
    }
  }
  else
  {
    Dir3d        N = D;
    Point3d        Q = Pt.Translated((1. / c) * Vector3d(N));
    Standard_Real x, y;
    Vector3d        V;
    for (Standard_Integer i = 1; i <= myFirstSect->NbPoles(); i++)
    {
      V         = Vector3d(Q, Poles(i));
      x         = V * Vector3d(T);
      y         = V * Vector3d(N);
      DPoles(i) = x * Vector3d(N) - y * Vector3d(T);
      if (DPoles(i).Magnitude() > Epsilon(1.))
      {
        DPoles(i).Normalize();
        DPoles(i) *= Sqrt(x * x + y * y);
      }
    }
  }

  for (Standard_Integer i = 1; i <= myFirstSect->NbPoles(); i++)
  {
    DWeigths(i) = 0.;
  }

  return Standard_True;
}

//=================================================================================================

void GeomFill_SweepSectionGenerator::Section(const Standard_Integer P,
                                             TColgp_Array1OfPnt&    Poles,
                                             TColgp_Array1OfPnt2d&, // Poles2d,
                                             TColStd_Array1OfReal& Weigths) const
{
  if (myType != 0)
  {
    myFirstSect->Poles(Poles);
    myFirstSect->Weights(Weigths);
    Transform3d cumulTR;
    if (P > 1)
    {
      cumulTR = myTrsfs(P - 1);
      // <cumulTR> transform <myFirstSect> to the P ieme Section. In fact
      // each points of the array <poles> will be transformed.

      if ((myType == 3) || (myType == 6))
      {
        for (Standard_Integer i = 1; i <= myFirstSect->NbPoles(); i++)
        {
          Poles(i).SetXYZ((myNbSections - P) * myFirstSect->Pole(i).XYZ()
                          + (P - 1) * myLastSect->Pole(i).XYZ());
          Poles(i).SetXYZ(Poles(i).XYZ() / (myNbSections - 1));

          Weigths(i) =
            (myNbSections - P) * myFirstSect->Weight(i) + (P - 1) * myLastSect->Weight(i);
          Weigths(i) /= myNbSections - 1;
        }
      }

      for (Standard_Integer i = 1; i <= Poles.Length(); i++)
        Poles(i).Transform(cumulTR);
    }
#ifdef DRAW
    if (Affich)
    {
      char name[256];
      sprintf(name, "SECTION_%d", ++NbSECTIONS);
      DrawTrSurf1::Set(name, myFirstSect->Transformed(cumulTR));
    }
#endif
  }
  else
  {

    Standard_Real Coef = (P - 1.) / (myNbSections - 1.);
    Standard_Real U = (1 - Coef) * myAdpPath->FirstParameter() + Coef * myAdpPath->LastParameter();

    Point3d PPath = myAdpPath->Value(U);

    Standard_Real Alpha = U - myAdpPath->FirstParameter();
    Alpha /= myAdpPath->LastParameter() - myAdpPath->FirstParameter();

    Standard_Real U1 =
      (1 - Alpha) * myAdpFirstSect->FirstParameter() + Alpha * myAdpFirstSect->LastParameter();

    if (myAdpFirstSect->GetType() == GeomAbs_Line)
    {
      if (Precision::IsInfinite(myAdpFirstSect->FirstParameter())
          || Precision::IsInfinite(myAdpFirstSect->LastParameter()))
      {
        gp_Lin aLine = myAdpFirstSect->Line();
        U1           = ElCLib::Parameter(aLine, PPath);
      }
    }
    Point3d P1 = myAdpFirstSect->Value(U1);

    Standard_Real U2 =
      (1 - Alpha) * myAdpLastSect->FirstParameter() + Alpha * myAdpLastSect->LastParameter();

    if (myAdpLastSect->GetType() == GeomAbs_Line)
    {
      if (Precision::IsInfinite(myAdpLastSect->FirstParameter())
          || Precision::IsInfinite(myAdpLastSect->LastParameter()))
      {
        gp_Lin aLine = myAdpLastSect->Line();
        U2           = ElCLib::Parameter(aLine, PPath);
      }
    }
    Point3d P2 = myAdpLastSect->Value(U2);

    Frame3d        Axis;
    Standard_Real Angle;
    if (P1.Distance(P2) < Precision::Confusion())
    {
      Angle = 0.;
    }
    else
    {
      Axis  = Frame3d(PPath, Vector3d(PPath, P1) ^ Vector3d(PPath, P2), Vector3d(PPath, P1));
      Angle = ElCLib::CircleParameter(Axis, P2);
    }
#ifdef OCCT_DEBUG
/*
    if (Standard_False) {
      Vector3d dummyD1 = myAdpPath->DN(U,1);
      Vector3d dummyTg = Axis.Direction();
      Standard_Real Cos = dummyD1.Dot(dummyTg);
      if ( Cos > 0.) std::cout << "+" ;
      else           std::cout << "-" ;
    }
*/
#endif
    if (Angle < Precision::Angular())
    {
      for (Standard_Integer i = 1; i <= Poles.Upper(); i++)
      {
        Poles(i)   = P1;
        Weigths(i) = 1;
      }
    }
    else
    {
      Handle(GeomCircle)       Circ = new GeomCircle(Axis, myRadius);
      Handle(Geom_TrimmedCurve) CT   = new Geom_TrimmedCurve(Circ, 0., Angle);
      Handle(BSplineCurve3d) BS;
      if (myPolynomial)
        BS = GeomConvert::CurveToBSplineCurve(CT, Convert_Polynomial);
      else
        BS = GeomConvert::CurveToBSplineCurve(CT, Convert_QuasiAngular);

#ifdef DRAW
      if (Affich)
      {
        char name[256];
        sprintf(name, "SECTION_%d", ++NbSECTIONS);
        DrawTrSurf1::Set(name, BS);
      }
#endif

      BS->Poles(Poles);
      BS->Weights(Weigths);
    }
  }
}

//=================================================================================================

const Transform3d& GeomFill_SweepSectionGenerator::Transformation(const Standard_Integer Index) const
{
  if (Index > myTrsfs.Length())
    throw Standard_RangeError("GeomFill_SweepSectionGenerator::Transformation");

  return myTrsfs(Index);
}

//=================================================================================================

Standard_Real GeomFill_SweepSectionGenerator::Parameter(const Standard_Integer P) const
{
  if (P == 1)
  {
    return myPath->FirstParameter();
  }
  else if (P == myNbSections)
  {
    return myPath->LastParameter();
  }
  else
  {
    Standard_Real U1 = myPath->FirstParameter();
    Standard_Real U2 = myPath->LastParameter();
    Standard_Real prm =
      ((myNbSections - P) * U1 + (P - 1) * U2) / (Standard_Real)(myNbSections - 1);
    return prm;
  }
}
