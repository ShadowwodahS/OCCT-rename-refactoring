// Created on: 1996-02-15
// Created by: Jacques GOUSSARD
// Copyright (c) 1996-1999 Matra Datavision
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

#include <BRep_Tool.hxx>
#include <ElCLib.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <LocOpe_FindEdges.hxx>
#include <Precision.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>

//=================================================================================================

void LocOpe_FindEdges::Set(const TopoShape& FFrom, const TopoShape& FTo)
{
  myFFrom = FFrom;
  myFTo   = FTo;
  myLFrom.Clear();
  myLTo.Clear();

  ShapeExplorer       expf, expt;
  Handle(GeomCurve3d)    Cf, Ct;
  TopLoc_Location       Loc;
  Standard_Real         ff, lf, ft, lt;
  Handle(TypeInfo) Tf, Tt;

  for (expf.Init(myFFrom, TopAbs_EDGE); expf.More(); expf.Next())
  {
    const TopoEdge& edgf = TopoDS::Edge(expf.Current());
    Cf                      = BRepInspector::Curve(edgf, Loc, ff, lf);
    if (!Loc.IsIdentity())
    {
      Handle(Geom_Geometry) GGf = Cf->Transformed(Loc.Transformation());
      Cf                        = Handle(GeomCurve3d)::DownCast(GGf);
    }
    Tf = Cf->DynamicType();
    if (Tf == STANDARD_TYPE(Geom_TrimmedCurve))
    {
      Cf = Handle(Geom_TrimmedCurve)::DownCast(Cf)->BasisCurve();
      Tf = Cf->DynamicType();
    }
    if (Tf != STANDARD_TYPE(GeomLine) && Tf != STANDARD_TYPE(GeomCircle)
        && Tf != STANDARD_TYPE(Geom_Ellipse) && Tf != STANDARD_TYPE(BSplineCurve3d)
        && Tf != STANDARD_TYPE(BezierCurve3d))
    {
      continue;
    }
    for (expt.Init(myFTo, TopAbs_EDGE); expt.More(); expt.Next())
    {
      const TopoEdge& edgt = TopoDS::Edge(expt.Current());
      Ct                      = BRepInspector::Curve(edgt, Loc, ft, lt);
      if (!Loc.IsIdentity())
      {
        Handle(Geom_Geometry) GGt = Ct->Transformed(Loc.Transformation());
        Ct                        = Handle(GeomCurve3d)::DownCast(GGt);
      }
      Tt = Ct->DynamicType();
      if (Tt == STANDARD_TYPE(Geom_TrimmedCurve))
      {
        Ct = Handle(Geom_TrimmedCurve)::DownCast(Ct)->BasisCurve();
        Tt = Ct->DynamicType();
      }
      if (Tt != Tf)
      {
        continue;
      }
      // On a presomption de confusion
      Standard_Real Tol = Precision::Confusion();
      if (Tt == STANDARD_TYPE(GeomLine))
      {
        gp_Lin        lif  = Handle(GeomLine)::DownCast(Cf)->Lin();
        gp_Lin        lit  = Handle(GeomLine)::DownCast(Ct)->Lin();
        Point3d        p1   = ElCLib1::Value(ff, lif);
        Point3d        p2   = ElCLib1::Value(lf, lif);
        Standard_Real prm1 = ElCLib1::Parameter(lit, p1);
        Standard_Real prm2 = ElCLib1::Parameter(lit, p2);
        if (prm1 >= ft - Tol && prm1 <= lt + Tol && prm2 >= ft - Tol && prm2 <= lt + Tol)
        {
          Tol *= Tol;
          Point3d pt = ElCLib1::Value(prm1, lit);
          if (pt.SquareDistance(p1) <= Tol)
          {
            pt = ElCLib1::Value(prm2, lit);
            if (pt.SquareDistance(p2) <= Tol)
            {
              myLFrom.Append(edgf);
              myLTo.Append(edgt);
              break;
            }
          }
        }
      }
      else if (Tt == STANDARD_TYPE(GeomCircle))
      {
        gp_Circ cif = Handle(GeomCircle)::DownCast(Cf)->Circ();
        gp_Circ cit = Handle(GeomCircle)::DownCast(Ct)->Circ();
        if (Abs(cif.Radius() - cit.Radius()) <= Tol
            && cif.Location().SquareDistance(cit.Location()) <= Tol * Tol)
        {
          // Point debut, calage dans periode, et detection meme sens

          Point3d p1, p2;
          Vector3d tgf, tgt;
          ElCLib1::D1(ff, cif, p1, tgf);
          p2 = ElCLib1::Value(lf, cif);

          Standard_Real prm1  = ElCLib1::Parameter(cit, p1);
          Standard_Real Tol2d = Precision::PConfusion();
          if (Abs(prm1 - ft) <= Tol2d)
            prm1 = ft;
          prm1 = ElCLib1::InPeriod(prm1, ft, ft + 2. * M_PI);
          ElCLib1::D1(prm1, cit, p1, tgt);

          Standard_Real prm2 = ElCLib1::Parameter(cit, p2);
          if (tgt.Dot(tgf) > 0.)
          { // meme sens
            while (prm2 <= prm1)
            {
              prm2 += 2. * M_PI;
            }
          }
          else
          {
            if (Abs(prm1 - ft) <= Precision::Angular())
            {
              prm1 += 2. * M_PI;
            }
            while (prm2 >= prm1)
            {
              prm2 -= 2. * M_PI;
            }
          }

          if (prm1 >= ft - Tol && prm1 <= lt + Tol && prm2 >= ft - Tol && prm2 <= lt + Tol)
          {
            myLFrom.Append(edgf);
            myLTo.Append(edgt);
            break;
          }
          else
          {
            // Cas non traite : on est a cheval
#ifdef OCCT_DEBUG
            std::cout << " cas a cheval." << std::endl;
#endif

            //	    myLFrom.Append(edgf);
            //	    myLTo.Append(edgt);
            //	    break;
          }
        }
      }
      else if (Tt == STANDARD_TYPE(Geom_Ellipse))
      {
        gp_Elips cif = Handle(Geom_Ellipse)::DownCast(Cf)->Elips();
        gp_Elips cit = Handle(Geom_Ellipse)::DownCast(Ct)->Elips();

        if (Abs(cif.MajorRadius() - cit.MajorRadius()) <= Tol
            && Abs(cif.MinorRadius() - cit.MinorRadius()) <= Tol
            && cif.Location().SquareDistance(cit.Location()) <= Tol * Tol)
        {
          // Point debut, calage dans periode, et detection meme sens

          Point3d p1, p2;
          Vector3d tgf, tgt;
          ElCLib1::D1(ff, cif, p1, tgf);
          p2 = ElCLib1::Value(lf, cif);

          Standard_Real prm1 = ElCLib1::Parameter(cit, p1);
          prm1               = ElCLib1::InPeriod(prm1, ft, ft + 2. * M_PI);
          ElCLib1::D1(prm1, cit, p1, tgt);

          Standard_Real prm2 = ElCLib1::Parameter(cit, p2);
          if (tgt.Dot(tgf) > 0.)
          { // meme sens
            while (prm2 <= prm1)
            {
              prm2 += 2. * M_PI;
            }
          }
          else
          {
            if (Abs(prm1 - ft) <= Precision::Angular())
            {
              prm1 += 2. * M_PI;
            }
            while (prm2 >= prm1)
            {
              prm2 -= 2. * M_PI;
            }
          }

          if (prm1 >= ft - Tol && prm1 <= lt + Tol && prm2 >= ft - Tol && prm2 <= lt + Tol)
          {
            myLFrom.Append(edgf);
            myLTo.Append(edgt);
            break;
          }
          else
          {
            // Cas non traite : on est a cheval
#ifdef OCCT_DEBUG
            std::cout << " cas a cheval." << std::endl;
#endif
            //	    myLFrom.Append(edgf);
            //	    myLTo.Append(edgt);
          }
        }
      }
      else if (Tt == STANDARD_TYPE(BSplineCurve3d))
      {
        Handle(BSplineCurve3d) Bf = Handle(BSplineCurve3d)::DownCast(Cf);
        Handle(BSplineCurve3d) Bt = Handle(BSplineCurve3d)::DownCast(Ct);

        Standard_Boolean IsSame = Standard_True;

        Standard_Integer nbpoles = Bf->NbPoles();
        if (nbpoles != Bt->NbPoles())
        {
          IsSame = Standard_False;
        }

        if (IsSame)
        {
          Standard_Integer nbknots = Bf->NbKnots();
          if (nbknots != Bt->NbKnots())
          {
            IsSame = Standard_False;
          }

          if (IsSame)
          {
            TColgp_Array1OfPnt Pf(1, nbpoles), Pt(1, nbpoles);
            Bf->Poles(Pf);
            Bt->Poles(Pt);

            Standard_Real tol3d = BRepInspector::Tolerance(edgt);
            for (Standard_Integer p = 1; p <= nbpoles; p++)
            {
              if ((Pf(p)).Distance(Pt(p)) > tol3d)
              {
                IsSame = Standard_False;
                break;
              }
            }

            if (IsSame)
            {
              TColStd_Array1OfReal Kf(1, nbknots), Kt(1, nbknots);
              Bf->Knots(Kf);
              Bt->Knots(Kt);

              TColStd_Array1OfInteger Mf(1, nbknots), Mt(1, nbknots);
              Bf->Multiplicities(Mf);
              Bt->Multiplicities(Mt);

              for (Standard_Integer k = 1; k <= nbknots; k++)
              {
                if ((Kf(k) - Kt(k)) > Tol)
                {
                  IsSame = Standard_False;
                  break;
                }
                if (Abs(Mf(k) - Mt(k)) > Tol)
                {
                  IsSame = Standard_False;
                  break;
                }
              }

              if (!Bf->IsRational())
              {
                if (Bt->IsRational())
                {
                  IsSame = Standard_False;
                }
              }
              else
              {
                if (!Bt->IsRational())
                {
                  IsSame = Standard_False;
                }
              }

              if (IsSame && Bf->IsRational())
              {
                TColStd_Array1OfReal Wf(1, nbpoles), Wt(1, nbpoles);
                Bf->Weights(Wf);
                Bt->Weights(Wt);

                for (Standard_Integer w = 1; w <= nbpoles; w++)
                {
                  if (Abs(Wf(w) - Wt(w)) > Tol)
                  {
                    IsSame = Standard_False;
                    break;
                  }
                }
              }

              if (IsSame)
              {
#ifdef OCCT_DEBUG
                std::cout << "memes bsplines." << std::endl;
#endif
                myLFrom.Append(edgf);
                myLTo.Append(edgt);
                break;
              }
            }
          }
        }
      }
      else if (Tt == STANDARD_TYPE(BezierCurve3d))
      {
        Handle(BezierCurve3d) Bf = Handle(BezierCurve3d)::DownCast(Cf);
        Handle(BezierCurve3d) Bt = Handle(BezierCurve3d)::DownCast(Ct);

        Standard_Boolean IsSame = Standard_True;

        Standard_Integer nbpoles = Bf->NbPoles();
        if (nbpoles != Bt->NbPoles())
        {
          IsSame = Standard_False;
        }

        if (IsSame)
        {
          TColgp_Array1OfPnt Pf(1, nbpoles), Pt(1, nbpoles);
          Bf->Poles(Pf);
          Bt->Poles(Pt);

          for (Standard_Integer p = 1; p <= nbpoles; p++)
          {
            if ((Pf(p)).Distance(Pt(p)) > Tol)
            {
              IsSame = Standard_False;
              break;
            }
          }

          if (IsSame)
          {
            if (!Bf->IsRational())
            {
              if (Bt->IsRational())
              {
                IsSame = Standard_False;
              }
            }
            else
            {
              if (!Bt->IsRational())
              {
                IsSame = Standard_False;
              }
            }

            if (IsSame && Bf->IsRational())
            {
              TColStd_Array1OfReal Wf(1, nbpoles), Wt(1, nbpoles);
              Bf->Weights(Wf);
              Bt->Weights(Wt);

              for (Standard_Integer w = 1; w <= nbpoles; w++)
              {
                if (Abs(Wf(w) - Wt(w)) > Tol)
                {
                  IsSame = Standard_False;
                  break;
                }
              }
            }

            if (IsSame)
            {
#ifdef OCCT_DEBUG
              std::cout << "memes beziers." << std::endl;
#endif
              myLFrom.Append(edgf);
              myLTo.Append(edgt);
              break;
            }
          }
        }
      }
    }
  }
}
