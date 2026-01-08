// Created on: 2004-05-20
// Created by: Sergey ZARITCHNY
// Copyright (c) 2004-2014 OPEN CASCADE SAS
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

#include <BinTools.hxx>
#include <BinTools_CurveSet.hxx>
#include <BinTools_SurfaceSet.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Pln.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <Message_ProgressScope.hxx>

#define PLANE 1
#define CYLINDER 2
#define CONE 3
#define SPHERE 4
#define TORUS 5
#define LINEAREXTRUSION 6
#define REVOLUTION 7
#define BEZIER 8
#define BSPLINE 9
#define RECTANGULAR 10
#define OFFSET 11

//=================================================================================================

SurfaceBinarySet::SurfaceBinarySet() {}

//=================================================================================================

void SurfaceBinarySet::Clear()
{
  myMap.Clear();
}

//=================================================================================================

Standard_Integer SurfaceBinarySet::Add(const Handle(GeomSurface)& S)
{
  return myMap.Add(S);
}

//=================================================================================================

Handle(GeomSurface) SurfaceBinarySet::Surface(const Standard_Integer I) const
{
  return Handle(GeomSurface)::DownCast(myMap(I));
}

//=================================================================================================

Standard_Integer SurfaceBinarySet::Index(const Handle(GeomSurface)& S) const
{
  return myMap.FindIndex(S);
}

//=======================================================================
// function : operator <<(GeomPlane)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(GeomPlane)& S)
{
  OS << (Standard_Byte)PLANE;
  gp_Pln P = S->Pln();
  OS << P.Location(); // Pnt
  OS << P.Axis().Direction();
  OS << P.XAxis().Direction();
  OS << P.YAxis().Direction();
  return OS;
}

//=======================================================================
// function : operator <<(Geom_CylindricalSurface)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_CylindricalSurface)& S)
{
  OS << (Standard_Byte)CYLINDER;
  Cylinder1 P = S->Cylinder();
  OS << P.Location(); // Pnt
  OS << P.Axis().Direction();
  OS << P.XAxis().Direction();
  OS << P.YAxis().Direction();
  OS << P.Radius();
  return OS;
}

//=======================================================================
// function : operator <<(Geom_ConicalSurface)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_ConicalSurface)& S)
{
  OS << (Standard_Byte)CONE;
  Cone1 P = S->Cone();
  OS << P.Location(); // Pnt
  OS << P.Axis().Direction();
  OS << P.XAxis().Direction();
  OS << P.YAxis().Direction();
  OS << P.RefRadius();
  OS << P.SemiAngle();
  return OS;
}

//=======================================================================
// function : operator <<(Geom_SphericalSurface)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_SphericalSurface)& S)
{
  OS << (Standard_Byte)SPHERE;
  Sphere3 P = S->Sphere();
  OS << P.Location(); // Pnt
  OS << P.Position().Axis().Direction();
  OS << P.XAxis().Direction();
  OS << P.YAxis().Direction();
  OS << P.Radius();
  return OS;
}

//=======================================================================
// function : operator <<(Geom_ToroidalSurface)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_ToroidalSurface)& S)
{
  OS << (Standard_Byte)TORUS;
  gp_Torus P = S->Torus();
  OS << P.Location(); // Pnt
  OS << P.Axis().Direction();
  OS << P.XAxis().Direction();
  OS << P.YAxis().Direction();
  OS << P.MajorRadius();
  OS << P.MinorRadius();
  return OS;
}

//=======================================================================
// function : operator <<(Geom_SurfaceOfLinearExtrusion)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream&                            OS,
                                    const Handle(Geom_SurfaceOfLinearExtrusion)& S)
{
  OS << (Standard_Byte)LINEAREXTRUSION;
  OS << S->Direction();
  CurveBinarySet::WriteCurve(S->BasisCurve(), OS);
  return OS;
}

//=======================================================================
// function : operator <<(Geom_SurfaceOfRevolution)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_SurfaceOfRevolution)& S)
{
  OS << (Standard_Byte)REVOLUTION;
  OS << S->Location();
  OS << S->Direction();
  CurveBinarySet::WriteCurve(S->BasisCurve(), OS);
  return OS;
}

//=======================================================================
// function : operator <<(Geom_BezierSurface)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_BezierSurface)& S)
{
  OS << (Standard_Byte)BEZIER;
  Standard_Boolean urational = S->IsURational() ? 1 : 0;
  Standard_Boolean vrational = S->IsVRational() ? 1 : 0;
  OS << urational; // rational
  OS << vrational;

  // poles and weights
  Standard_Integer i, j, udegree, vdegree;
  udegree = S->UDegree();
  vdegree = S->VDegree();
  OS << (Standard_ExtCharacter)udegree;
  OS << (Standard_ExtCharacter)vdegree;
  for (i = 1; i <= udegree + 1; i++)
  {
    for (j = 1; j <= vdegree + 1; j++)
    {
      OS << S->Pole(i, j); // Pnt
      if (urational || vrational)
      {
        OS << S->Weight(i, j); // Real
      }
    }
  }
  return OS;
}

//=======================================================================
// function : operator <<(Geom_BSplineSurface)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_BSplineSurface)& S)
{
  OS << (Standard_Byte)BSPLINE;
  Standard_Boolean urational = S->IsURational() ? 1 : 0;
  Standard_Boolean vrational = S->IsVRational() ? 1 : 0;
  Standard_Boolean uperiodic = S->IsUPeriodic() ? 1 : 0;
  Standard_Boolean vperiodic = S->IsVPeriodic() ? 1 : 0;
  OS << urational;
  OS << vrational;
  OS << uperiodic;
  OS << vperiodic;

  // poles and weights
  Standard_Integer i, j, udegree, vdegree, nbupoles, nbvpoles, nbuknots, nbvknots;
  udegree  = S->UDegree();
  vdegree  = S->VDegree();
  nbupoles = S->NbUPoles();
  nbvpoles = S->NbVPoles();
  nbuknots = S->NbUKnots();
  nbvknots = S->NbVKnots();
  OS << (Standard_ExtCharacter)udegree;
  OS << (Standard_ExtCharacter)vdegree;
  OS << nbupoles;
  OS << nbvpoles;
  OS << nbuknots;
  OS << nbvknots;
  for (i = 1; i <= nbupoles; i++)
  {
    for (j = 1; j <= nbvpoles; j++)
    {
      OS << S->Pole(i, j); // Pnt
      if (urational || vrational)
        OS << S->Weight(i, j); // Real
    }
  }

  for (i = 1; i <= nbuknots; i++)
  {
    OS << S->UKnot(i);
    OS << S->UMultiplicity(i);
  }

  for (i = 1; i <= nbvknots; i++)
  {
    OS << S->VKnot(i);
    OS << S->VMultiplicity(i);
  }
  return OS;
}

//=======================================================================
// function :  operator <<(Geom_RectangularTrimmedSurface)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream&                             OS,
                                    const Handle(Geom_RectangularTrimmedSurface)& S)
{
  OS << (Standard_Byte)RECTANGULAR;
  Standard_Real U1, U2, V1, V2;
  S->Bounds(U1, U2, V1, V2);
  OS << U1 << U2 << V1 << V2;
  SurfaceBinarySet::WriteSurface(S->BasisSurface(), OS);
  return OS;
}

//=======================================================================
// function : operator <<(Geom_OffsetSurface)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_OffsetSurface)& S)
{
  OS << (Standard_Byte)OFFSET;
  OS << S->Offset();
  SurfaceBinarySet::WriteSurface(S->BasisSurface(), OS);
  return OS;
}

//=================================================================================================

void SurfaceBinarySet::WriteSurface(const Handle(GeomSurface)& S, BinaryOutputStream& OS)
{
  Handle(TypeInfo) TheType = S->DynamicType();
  try
  {
    OCC_CATCH_SIGNALS
    if (TheType == STANDARD_TYPE(GeomPlane))
    {
      OS << Handle(GeomPlane)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_CylindricalSurface))
    {
      OS << Handle(Geom_CylindricalSurface)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_ConicalSurface))
    {
      OS << Handle(Geom_ConicalSurface)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_SphericalSurface))
    {
      OS << Handle(Geom_SphericalSurface)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_ToroidalSurface))
    {
      OS << Handle(Geom_ToroidalSurface)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_SurfaceOfLinearExtrusion))
    {
      OS << Handle(Geom_SurfaceOfLinearExtrusion)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_SurfaceOfRevolution))
    {
      OS << Handle(Geom_SurfaceOfRevolution)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_BezierSurface))
    {
      OS << Handle(Geom_BezierSurface)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_BSplineSurface))
    {
      OS << Handle(Geom_BSplineSurface)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
    {
      OS << Handle(Geom_RectangularTrimmedSurface)::DownCast(S);
    }
    else if (TheType == STANDARD_TYPE(Geom_OffsetSurface))
    {
      OS << Handle(Geom_OffsetSurface)::DownCast(S);
    }
    else
    {
      throw ExceptionBase("UNKNOWN SURFACE TYPE");
    }
  }
  catch (ExceptionBase const& anException)
  {
    Standard_SStream aMsg;
    aMsg << "EXCEPTION in SurfaceBinarySet::WriteSurface(..)" << std::endl;
    aMsg << anException << std::endl;
    throw ExceptionBase(aMsg.str().c_str());
  }
}

//=================================================================================================

void SurfaceBinarySet::Write(Standard_OStream& OS, const Message_ProgressRange& theRange) const
{

  Standard_Integer      i, nbsurf = myMap.Extent();
  Message_ProgressScope aPS(theRange, "Writing surfaces", nbsurf);
  OS << "Surfaces " << nbsurf << "\n";
  BinaryOutputStream aStream(OS);
  for (i = 1; i <= nbsurf && aPS.More(); i++, aPS.Next())
  {
    WriteSurface(Handle(GeomSurface)::DownCast(myMap(i)), aStream);
  }
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Point3d& P)
{
  Standard_Real X = 0., Y = 0., Z = 0.;
  BinTools1::GetReal(IS, X);
  BinTools1::GetReal(IS, Y);
  BinTools1::GetReal(IS, Z);
  P.SetCoord(X, Y, Z);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Dir3d& D)
{
  Standard_Real X = 0., Y = 0., Z = 0.;
  BinTools1::GetReal(IS, X);
  BinTools1::GetReal(IS, Y);
  BinTools1::GetReal(IS, Z);
  D.SetCoord(X, Y, Z);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Ax3& A3)
{
  Point3d P(0., 0., 0.);
  Dir3d A(1., 0., 0.), AX(1., 0., 0.), AY(1., 0., 0.);
  IS >> P >> A >> AX >> AY;
  Ax3 ax3(P, A, AX);
  if (AY.DotCross(A, AX) < 0)
    ax3.YReverse();
  A3 = ax3;
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(GeomPlane)& S)
{
  Ax3 A;
  IS >> A;
  S = new GeomPlane(A);
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_CylindricalSurface)& S)
{
  Ax3        A;
  Standard_Real R = 0.;
  IS >> A;
  BinTools1::GetReal(IS, R);
  S = new Geom_CylindricalSurface(A, R);
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_ConicalSurface)& S)
{
  Ax3        A;
  Standard_Real R = 0., Ang = 0.;
  IS >> A;
  BinTools1::GetReal(IS, R);
  BinTools1::GetReal(IS, Ang);
  S = new Geom_ConicalSurface(A, Ang, R);
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_SphericalSurface)& S)
{
  Ax3        A;
  Standard_Real R = 0.;
  IS >> A;
  BinTools1::GetReal(IS, R);
  S = new Geom_SphericalSurface(A, R);
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_ToroidalSurface)& S)
{
  Ax3        A;
  Standard_Real R1 = 0., R2 = 0.;
  IS >> A;
  BinTools1::GetReal(IS, R1);
  BinTools1::GetReal(IS, R2);
  S = new Geom_ToroidalSurface(A, R1, R2);
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_SurfaceOfLinearExtrusion)& S)
{
  Dir3d             D(1., 0., 0.);
  Handle(GeomCurve3d) C;
  IS >> D;
  CurveBinarySet::ReadCurve(IS, C);
  S = new Geom_SurfaceOfLinearExtrusion(C, D);
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_SurfaceOfRevolution)& S)
{
  Point3d             P(0., 0., 0.);
  Dir3d             D(1., 0., 0.);
  Handle(GeomCurve3d) C;
  IS >> P >> D;
  CurveBinarySet::ReadCurve(IS, C);
  S = new Geom_SurfaceOfRevolution(C, Axis3d(P, D));
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_BezierSurface)& S)
{
  //  std::cout << "BezierSurface:" <<std::endl;
  Standard_Boolean urational = Standard_False, vrational = Standard_False;
  BinTools1::GetBool(IS, urational);
  BinTools1::GetBool(IS, vrational);

  //  std::cout << "\turational = " << urational << " vrational = " << vrational<<std::endl;
  Standard_Integer      udegree = 0, vdegree = 0;
  Standard_ExtCharacter aVal = '\0';
  BinTools1::GetExtChar(IS, aVal);

  udegree = (Standard_Integer)aVal;
  BinTools1::GetExtChar(IS, aVal);
  vdegree = (Standard_Integer)aVal;
  //  std::cout << "\ttudegree  = " << udegree << ", vdegree = " << vdegree << std::endl;

  TColgp_Array2OfPnt   poles(1, udegree + 1, 1, vdegree + 1);
  TColStd_Array2OfReal weights(1, udegree + 1, 1, vdegree + 1);

  Standard_Integer i, j;
  for (i = 1; i <= udegree + 1; i++)
  {
    for (j = 1; j <= vdegree + 1; j++)
    {
      IS >> poles(i, j); // Pnt
      //      std::cout <<"Pole X = " <<poles(i,j).X()<< " Y = " <<poles(i,j).Y()<< " Z = " <<
      //      poles(i,j).Z()<<std::endl;
      if (urational || vrational)
        BinTools1::GetReal(IS, weights(i, j));
    }
  }

  if (urational || vrational)
    S = new Geom_BezierSurface(poles, weights);
  else
    S = new Geom_BezierSurface(poles);
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_BSplineSurface)& S)
{
  Standard_Boolean urational = Standard_False, vrational = Standard_False,
                   uperiodic = Standard_False, vperiodic = Standard_False;
  BinTools1::GetBool(IS, urational);
  BinTools1::GetBool(IS, vrational);
  BinTools1::GetBool(IS, uperiodic);
  BinTools1::GetBool(IS, vperiodic);

  Standard_Integer udegree = 0, vdegree = 0, nbupoles = 0, nbvpoles = 0, nbuknots = 0, nbvknots = 0;
  Standard_ExtCharacter aVal = '\0';
  BinTools1::GetExtChar(IS, aVal);
  udegree = (Standard_Integer)aVal;
  BinTools1::GetExtChar(IS, aVal);
  vdegree = (Standard_Integer)aVal;
  BinTools1::GetInteger(IS, nbupoles);
  BinTools1::GetInteger(IS, nbvpoles);
  BinTools1::GetInteger(IS, nbuknots);
  BinTools1::GetInteger(IS, nbvknots);

  TColgp_Array2OfPnt   poles(1, nbupoles, 1, nbvpoles);
  TColStd_Array2OfReal weights(1, nbupoles, 1, nbvpoles);

  Standard_Integer i, j;
  for (i = 1; i <= nbupoles; i++)
  {
    for (j = 1; j <= nbvpoles; j++)
    {
      IS >> poles(i, j); // Pnt
      if (urational || vrational)
        BinTools1::GetReal(IS, weights(i, j));
    }
  }

  TColStd_Array1OfReal    uknots(1, nbuknots);
  TColStd_Array1OfInteger umults(1, nbuknots);
  for (i = 1; i <= nbuknots; i++)
  {
    BinTools1::GetReal(IS, uknots(i));
    BinTools1::GetInteger(IS, umults(i));
  }

  TColStd_Array1OfReal    vknots(1, nbvknots);
  TColStd_Array1OfInteger vmults(1, nbvknots);
  for (i = 1; i <= nbvknots; i++)
  {
    BinTools1::GetReal(IS, vknots(i));
    BinTools1::GetInteger(IS, vmults(i));
  }

  if (urational || vrational)
    S = new Geom_BSplineSurface(poles,
                                weights,
                                uknots,
                                vknots,
                                umults,
                                vmults,
                                udegree,
                                vdegree,
                                uperiodic,
                                vperiodic);
  else
    S = new Geom_BSplineSurface(poles,
                                uknots,
                                vknots,
                                umults,
                                vmults,
                                udegree,
                                vdegree,
                                uperiodic,
                                vperiodic);
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_RectangularTrimmedSurface)& S)
{
  Standard_Real U1 = 0., U2 = 0., V1 = 0., V2 = 0.;
  BinTools1::GetReal(IS, U1);
  BinTools1::GetReal(IS, U2);
  BinTools1::GetReal(IS, V1);
  BinTools1::GetReal(IS, V2);
  Handle(GeomSurface) BS;
  SurfaceBinarySet::ReadSurface(IS, BS);
  S = new Geom_RectangularTrimmedSurface(BS, U1, U2, V1, V2);
  return IS;
}

//=======================================================================
// function : operator>>
// purpose  :
//=======================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_OffsetSurface)& S)
{
  Standard_Real O = 0.;
  BinTools1::GetReal(IS, O);
  Handle(GeomSurface) BS;
  SurfaceBinarySet::ReadSurface(IS, BS);
  S = new Geom_OffsetSurface(BS, O);
  return IS;
}

//=================================================================================================

Standard_IStream& SurfaceBinarySet::ReadSurface(Standard_IStream& IS, Handle(GeomSurface)& S)
{
  try
  {
    OCC_CATCH_SIGNALS
    const Standard_Byte stype = (Standard_Byte)IS.get();
    switch (stype)
    {

      case PLANE: {
        Handle(GeomPlane) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case CYLINDER: {
        Handle(Geom_CylindricalSurface) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case CONE: {
        Handle(Geom_ConicalSurface) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case SPHERE: {
        Handle(Geom_SphericalSurface) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case TORUS: {
        Handle(Geom_ToroidalSurface) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case LINEAREXTRUSION: {
        Handle(Geom_SurfaceOfLinearExtrusion) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case REVOLUTION: {
        Handle(Geom_SurfaceOfRevolution) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case BEZIER: {
        Handle(Geom_BezierSurface) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case BSPLINE: {
        Handle(Geom_BSplineSurface) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case RECTANGULAR: {
        Handle(Geom_RectangularTrimmedSurface) SS;
        IS >> SS;
        S = SS;
      }
      break;

      case OFFSET: {
        Handle(Geom_OffsetSurface) SS;
        IS >> SS;
        S = SS;
      }
      break;

      default: {
        S = NULL;
        throw ExceptionBase("UNKNOWN SURFACE TYPE");
      }
      break;
    }
  }
  catch (ExceptionBase const& anException)
  {
    S = NULL;
    Standard_SStream aMsg;
    aMsg << "EXCEPTION in SurfaceBinarySet::ReadSurface(..)" << std::endl;
    aMsg << anException << std::endl;
    throw ExceptionBase(aMsg.str().c_str());
  }
  return IS;
}

//=================================================================================================

void SurfaceBinarySet::Read(Standard_IStream& IS, const Message_ProgressRange& theRange)
{
  char buffer[255];
  IS >> buffer;
  if (IS.fail() || strcmp(buffer, "Surfaces"))
  {
    Standard_SStream aMsg;
    aMsg << "SurfaceBinarySet::Read:  Not a surface table" << std::endl;
#ifdef OCCT_DEBUG
    std::cout << "SurfaceSet buffer: " << buffer << std::endl;
#endif
    throw ExceptionBase(aMsg.str().c_str());
    return;
  }

  Handle(GeomSurface) S;
  Standard_Integer     i, nbsurf;
  IS >> nbsurf;
  Message_ProgressScope aPS(theRange, "Reading surfaces", nbsurf);
  IS.get(); // remove <lf>
  for (i = 1; i <= nbsurf && aPS.More(); i++, aPS.Next())
  {
    SurfaceBinarySet::ReadSurface(IS, S);
    myMap.Add(S);
  }
}
