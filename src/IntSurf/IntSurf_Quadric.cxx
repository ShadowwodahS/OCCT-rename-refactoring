// Copyright (c) 1995-1999 Matra Datavision
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

#include <ElCLib.hxx>
#include <ElSLib.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <gp_Vec.hxx>
#include <IntSurf_Quadric.hxx>
#include <StdFail_NotDone.hxx>

// ============================================================
Quadric1::Quadric1()
    : typ(GeomAbs_OtherSurface),
      prm1(0.),
      prm2(0.),
      prm3(0.),
      prm4(0.),
      ax3direc(Standard_False)
{
}

// ============================================================
Quadric1::Quadric1(const gp_Pln& P)
    : ax3(P.Position()),
      typ(GeomAbs_Plane)
{
  ax3direc = ax3.Direct();
  P.Coefficients(prm1, prm2, prm3, prm4);
}

// ============================================================
Quadric1::Quadric1(const Cylinder1& C)
    :

      ax3(C.Position()),
      lin(ax3.Axis()),
      typ(GeomAbs_Cylinder)
{
  prm2 = prm3 = prm4 = 0.0;
  ax3direc           = ax3.Direct();
  prm1               = C.Radius();
}

// ============================================================
Quadric1::Quadric1(const Sphere3& S)
    :

      ax3(S.Position()),
      lin(ax3.Axis()),
      typ(GeomAbs_Sphere)
{
  prm2 = prm3 = prm4 = 0.0;
  ax3direc           = ax3.Direct();
  prm1               = S.Radius();
}

// ============================================================
Quadric1::Quadric1(const Cone1& C)
    :

      ax3(C.Position()),
      typ(GeomAbs_Cone)
{
  ax3direc = ax3.Direct();
  lin.SetPosition(ax3.Axis());
  prm1 = C.RefRadius();
  prm2 = C.SemiAngle();
  prm3 = Cos(prm2);
  prm4 = 0.0;
}

// ============================================================
Quadric1::Quadric1(const gp_Torus& T)
    :

      ax3(T.Position()),
      typ(GeomAbs_Torus)
{
  ax3direc = ax3.Direct();
  lin.SetPosition(ax3.Axis());
  prm1 = T.MajorRadius();
  prm2 = T.MinorRadius();
  prm3 = 0.0;
  prm4 = 0.0;
}

// ============================================================
void Quadric1::SetValue(const gp_Pln& P)
{
  typ      = GeomAbs_Plane;
  ax3      = P.Position();
  ax3direc = ax3.Direct();
  P.Coefficients(prm1, prm2, prm3, prm4);
}

// ============================================================
void Quadric1::SetValue(const Cylinder1& C)
{
  typ      = GeomAbs_Cylinder;
  ax3      = C.Position();
  ax3direc = ax3.Direct();
  lin.SetPosition(ax3.Axis());
  prm1 = C.Radius();
  prm2 = prm3 = prm4 = 0.0;
}

// ============================================================
void Quadric1::SetValue(const Sphere3& S)
{
  typ      = GeomAbs_Sphere;
  ax3      = S.Position();
  ax3direc = ax3.Direct();
  lin.SetPosition(ax3.Axis());
  prm1 = S.Radius();
  prm2 = prm3 = prm4 = 0.0;
}

// ============================================================
void Quadric1::SetValue(const Cone1& C)
{
  typ      = GeomAbs_Cone;
  ax3      = C.Position();
  ax3direc = ax3.Direct();
  lin.SetPosition(ax3.Axis());
  prm1 = C.RefRadius();
  prm2 = C.SemiAngle();
  prm3 = Cos(prm2);
  prm4 = 0.0;
}

// ============================================================
void Quadric1::SetValue(const gp_Torus& T)
{
  typ      = GeomAbs_Torus;
  ax3      = T.Position();
  ax3direc = ax3.Direct();
  lin.SetPosition(ax3.Axis());
  prm1 = T.MajorRadius();
  prm2 = T.MinorRadius();
  prm3 = 0.0;
  prm4 = 0.0;
}

// ============================================================
Standard_Real Quadric1::Distance(const Point3d& P) const
{
  switch (typ)
  {
    case GeomAbs_Plane: // plan
      return prm1 * P.X() + prm2 * P.Y() + prm3 * P.Z() + prm4;
    case GeomAbs_Cylinder: // cylindre
      return (lin.Distance(P) - prm1);
    case GeomAbs_Sphere: // sphere
      return (lin.Location().Distance(P) - prm1);
    case GeomAbs_Cone: // cone
    {
      Standard_Real dist = lin.Distance(P);
      Standard_Real U, V;
      ElSLib1::ConeParameters(ax3, prm1, prm2, P, U, V);
      Point3d        Pp    = ElSLib1::ConeValue(U, V, ax3, prm1, prm2);
      Standard_Real distp = lin.Distance(Pp);
      dist                = (dist - distp) / prm3;
      return (dist);
    }
    case GeomAbs_Torus: // torus
    {
      Point3d O, Pp, PT;
      //
      O = ax3.Location();
      Vector3d OZ(ax3.Direction());
      Pp = P.Translated(OZ.Multiplied(-(Vector3d(O, P).Dot(ax3.Direction()))));
      //
      Dir3d DOPp = (O.SquareDistance(Pp) < 1e-14) ? ax3.XDirection() : Dir3d(Vector3d(O, Pp));
      PT.SetXYZ(O.XYZ() + DOPp.XYZ() * prm1);
      //
      Standard_Real dist = P.Distance(PT) - prm2;
      return dist;
    }
    default: {
    }
    break;
  }
  return (0.0);
}

// ============================================================
Vector3d Quadric1::Gradient(const Point3d& P) const
{
  Vector3d grad;
  switch (typ)
  {
    case GeomAbs_Plane: // plan
      grad.SetCoord(prm1, prm2, prm3);
      break;
    case GeomAbs_Cylinder: // cylindre
    {
      Coords3d PP(lin.Location().XYZ());
      PP.Add(ElCLib1::Parameter(lin, P) * lin.Direction().XYZ());
      grad.SetXYZ(P.XYZ() - PP);
      Standard_Real N = grad.Magnitude();
      if (N > 1e-14)
      {
        grad.Divide(N);
      }
      else
      {
        grad.SetCoord(0.0, 0.0, 0.0);
      }
    }
    break;
    case GeomAbs_Sphere: // sphere
    {
      Coords3d PP(P.XYZ());
      grad.SetXYZ((PP - lin.Location().XYZ()));
      Standard_Real N = grad.Magnitude();
      if (N > 1e-14)
      {
        grad.Divide(N);
      }
      else
      {
        grad.SetCoord(0.0, 0.0, 0.0);
      }
    }
    break;
    case GeomAbs_Cone: // cone
    {
      Standard_Real U, V;
      ElSLib1::ConeParameters(ax3, prm1, prm2, P, U, V);
      Point3d Pp = ElSLib1::ConeValue(U, V, ax3, prm1, prm2);
      Vector3d D1u, D1v;
      ElSLib1::ConeD1(U, V, ax3, prm1, prm2, Pp, D1u, D1v);
      grad = D1u.Crossed(D1v);
      if (ax3direc == Standard_False)
      {
        grad.Reverse();
      }
      grad.Normalize();
    }
    break;
    case GeomAbs_Torus: // torus
    {
      Point3d O, Pp, PT;
      //
      O = ax3.Location();
      Vector3d OZ(ax3.Direction());
      Pp = P.Translated(OZ.Multiplied(-(Vector3d(O, P).Dot(ax3.Direction()))));
      //
      Dir3d DOPp = (O.SquareDistance(Pp) < 1e-14) ? ax3.XDirection() : Dir3d(Vector3d(O, Pp));
      PT.SetXYZ(O.XYZ() + DOPp.XYZ() * prm1);
      //
      grad.SetXYZ(P.XYZ() - PT.XYZ());
      Standard_Real N = grad.Magnitude();
      if (N > 1e-14)
      {
        grad.Divide(N);
      }
      else
      {
        grad.SetCoord(0., 0., 0.);
      }
    }
    break;
    default: {
    }
    break;
  }
  return grad;
}

// ============================================================
void Quadric1::ValAndGrad(const Point3d& P, Standard_Real& Dist, Vector3d& Grad) const
{

  switch (typ)
  {
    case GeomAbs_Plane: {
      Dist = prm1 * P.X() + prm2 * P.Y() + prm3 * P.Z() + prm4;
      Grad.SetCoord(prm1, prm2, prm3);
    }
    break;
    case GeomAbs_Cylinder: {
      Dist = lin.Distance(P) - prm1;
      Coords3d PP(lin.Location().XYZ());
      PP.Add(ElCLib1::Parameter(lin, P) * lin.Direction().XYZ());
      Grad.SetXYZ((P.XYZ() - PP));
      Standard_Real N = Grad.Magnitude();
      if (N > 1e-14)
      {
        Grad.Divide(N);
      }
      else
      {
        Grad.SetCoord(0.0, 0.0, 0.0);
      }
    }
    break;
    case GeomAbs_Sphere: {
      Dist = lin.Location().Distance(P) - prm1;
      Coords3d PP(P.XYZ());
      Grad.SetXYZ((PP - lin.Location().XYZ()));
      Standard_Real N = Grad.Magnitude();
      if (N > 1e-14)
      {
        Grad.Divide(N);
      }
      else
      {
        Grad.SetCoord(0.0, 0.0, 0.0);
      }
    }
    break;
    case GeomAbs_Cone: {
      Standard_Real dist = lin.Distance(P);
      Standard_Real U, V;
      Vector3d        D1u, D1v;
      Point3d        Pp;
      ElSLib1::ConeParameters(ax3, prm1, prm2, P, U, V);
      ElSLib1::ConeD1(U, V, ax3, prm1, prm2, Pp, D1u, D1v);
      Standard_Real distp = lin.Distance(Pp);
      dist                = (dist - distp) / prm3;
      Dist                = dist;
      Grad                = D1u.Crossed(D1v);
      if (ax3direc == Standard_False)
      {
        Grad.Reverse();
      }
      //-- lbr le 7 mars 96
      //-- Si le gardient est nul, on est sur l axe
      //-- et dans ce cas dist vaut 0
      //-- On peut donc renvoyer une valeur quelconque.
      if (Grad.X() > 1e-13 || Grad.Y() > 1e-13 || Grad.Z() > 1e-13)
      {
        Grad.Normalize();
      }
    }
    break;
    case GeomAbs_Torus: {
      Point3d O, Pp, PT;
      //
      O = ax3.Location();
      Vector3d OZ(ax3.Direction());
      Pp = P.Translated(OZ.Multiplied(-(Vector3d(O, P).Dot(ax3.Direction()))));
      //
      Dir3d DOPp = (O.SquareDistance(Pp) < 1e-14) ? ax3.XDirection() : Dir3d(Vector3d(O, Pp));
      PT.SetXYZ(O.XYZ() + DOPp.XYZ() * prm1);
      //
      Dist = P.Distance(PT) - prm2;
      //
      Grad.SetXYZ(P.XYZ() - PT.XYZ());
      Standard_Real N = Grad.Magnitude();
      if (N > 1e-14)
      {
        Grad.Divide(N);
      }
      else
      {
        Grad.SetCoord(0., 0., 0.);
      }
    }
    break;
    default: {
    }
    break;
  }
}

// ============================================================
Point3d Quadric1::Value(const Standard_Real U, const Standard_Real V) const
{
  switch (typ)
  {

    case GeomAbs_Plane:
      return ElSLib1::PlaneValue(U, V, ax3);
    case GeomAbs_Cylinder:
      return ElSLib1::CylinderValue(U, V, ax3, prm1);
    case GeomAbs_Sphere:
      return ElSLib1::SphereValue(U, V, ax3, prm1);
    case GeomAbs_Cone:
      return ElSLib1::ConeValue(U, V, ax3, prm1, prm2);
    case GeomAbs_Torus:
      return ElSLib1::TorusValue(U, V, ax3, prm1, prm2);
    default: {
      Point3d p(0, 0, 0);
      return (p);
    }
      // break;
  }
  // pop : pour NT
  //  return Point3d(0,0,0);
}

// ============================================================
void Quadric1::D1(const Standard_Real U,
                         const Standard_Real V,
                         Point3d&             P,
                         Vector3d&             D1U,
                         Vector3d&             D1V) const
{
  switch (typ)
  {
    case GeomAbs_Plane:
      ElSLib1::PlaneD1(U, V, ax3, P, D1U, D1V);
      break;
    case GeomAbs_Cylinder:
      ElSLib1::CylinderD1(U, V, ax3, prm1, P, D1U, D1V);
      break;
    case GeomAbs_Sphere:
      ElSLib1::SphereD1(U, V, ax3, prm1, P, D1U, D1V);
      break;
    case GeomAbs_Cone:
      ElSLib1::ConeD1(U, V, ax3, prm1, prm2, P, D1U, D1V);
      break;
    case GeomAbs_Torus:
      ElSLib1::TorusD1(U, V, ax3, prm1, prm2, P, D1U, D1V);
      break;
    default: {
    }
    break;
  }
}

// ============================================================
Vector3d Quadric1::DN(const Standard_Real    U,
                           const Standard_Real    V,
                           const Standard_Integer Nu,
                           const Standard_Integer Nv) const
{
  switch (typ)
  {
    case GeomAbs_Plane:
      return ElSLib1::PlaneDN(U, V, ax3, Nu, Nv);
    case GeomAbs_Cylinder:
      return ElSLib1::CylinderDN(U, V, ax3, prm1, Nu, Nv);
    case GeomAbs_Sphere:
      return ElSLib1::SphereDN(U, V, ax3, prm1, Nu, Nv);
    case GeomAbs_Cone:
      return ElSLib1::ConeDN(U, V, ax3, prm1, prm2, Nu, Nv);
    case GeomAbs_Torus:
      return ElSLib1::TorusDN(U, V, ax3, prm1, prm2, Nu, Nv);
    default: {
      Vector3d v(0, 0, 0);
      return (v);
    }
      // break;
  }
  // pop : pour NT
  //  return Vector3d(0,0,0);
}

// ============================================================
Vector3d Quadric1::Normale(const Standard_Real U, const Standard_Real V) const
{
  switch (typ)
  {
    case GeomAbs_Plane:
      if (ax3direc)
        return ax3.Direction();
      else
        return ax3.Direction().Reversed();
    case GeomAbs_Cylinder:
      return Normale(Value(U, V));
    case GeomAbs_Sphere:
      return Normale(Value(U, V));
    case GeomAbs_Cone: {
      Point3d P;
      Vector3d D1u, D1v;
      ElSLib1::ConeD1(U, V, ax3, prm1, prm2, P, D1u, D1v);
      if (D1u.Magnitude() < 0.0000001)
      {
        Vector3d Vn(0.0, 0.0, 0.0);
        return (Vn);
      }
      return (D1u.Crossed(D1v));
    }
    case GeomAbs_Torus:
      return Normale(Value(U, V));
    default: {
      Vector3d v(0, 0, 0);
      return (v);
    }
      //  break;
  }
  // pop : pour NT
  //  return Vector3d(0,0,0);
}

// ============================================================
Vector3d Quadric1::Normale(const Point3d& P) const
{
  switch (typ)
  {
    case GeomAbs_Plane:
      if (ax3direc)
        return ax3.Direction();
      else
        return ax3.Direction().Reversed();
    case GeomAbs_Cylinder: {
      if (ax3direc)
      {
        return lin.Normal(P).Direction();
      }
      else
      {
        Dir3d D(lin.Normal(P).Direction());
        D.Reverse();
        return (D);
      }
    }
    case GeomAbs_Sphere: {
      if (ax3direc)
      {
        Vector3d ax3P(ax3.Location(), P);
        return Dir3d(ax3P);
      }
      else
      {
        Vector3d Pax3(P, ax3.Location());
        return Dir3d(Pax3);
      }
    }
    case GeomAbs_Cone: {
      Standard_Real U, V;
      ElSLib1::ConeParameters(ax3, prm1, prm2, P, U, V);
      return Normale(U, V);
    }
    case GeomAbs_Torus: {
      Point3d O, Pp, PT;
      //
      O = ax3.Location();
      Vector3d OZ(ax3.Direction());
      Pp = P.Translated(OZ.Multiplied(-(Vector3d(O, P).Dot(ax3.Direction()))));
      //
      Dir3d DOPp = (O.SquareDistance(Pp) < 1e-14) ? ax3.XDirection() : Dir3d(Vector3d(O, Pp));
      PT.SetXYZ(O.XYZ() + DOPp.XYZ() * prm1);
      if (PT.SquareDistance(P) < 1e-14)
      {
        return Dir3d(OZ);
      }
      Dir3d aD(ax3direc ? Vector3d(PT, P) : Vector3d(P, PT));
      return aD;
    }
    default: {
      Vector3d v(0, 0, 0);
      return (v);
    } //    break;
  }
}

// ============================================================
void Quadric1::Parameters(const Point3d& P, Standard_Real& U, Standard_Real& V) const
{
  switch (typ)
  {
    case GeomAbs_Plane:
      ElSLib1::PlaneParameters(ax3, P, U, V);
      break;
    case GeomAbs_Cylinder:
      ElSLib1::CylinderParameters(ax3, prm1, P, U, V);
      break;
    case GeomAbs_Sphere:
      ElSLib1::SphereParameters(ax3, prm1, P, U, V);
      break;
    case GeomAbs_Cone:
      ElSLib1::ConeParameters(ax3, prm1, prm2, P, U, V);
      break;
    case GeomAbs_Torus:
      ElSLib1::TorusParameters(ax3, prm1, prm2, P, U, V);
      break;
    default:
      break;
  }
}

// ============================================================
