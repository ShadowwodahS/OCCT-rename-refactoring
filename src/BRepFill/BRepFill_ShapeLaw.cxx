// Created on: 1998-08-17
// Created by: Philippe MANGIN
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

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFill_ShapeLaw.hxx>
#include <BRepLProp.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GeomFill_EvolvedSection.hxx>
#include <GeomFill_HArray1OfSectionLaw.hxx>
#include <GeomFill_SectionLaw.hxx>
#include <GeomFill_UniformSection.hxx>
#include <Law_Function.hxx>
#include <Precision.hxx>
#include <Standard_Type.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColStd_HArray1OfInteger.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepFill_ShapeLaw, BRepFill_SectionLaw)

//=======================================================================
// function : Create
// purpose  : Process the case of Vertex by constructing a line
//           with the vertex in the origin
//=======================================================================
BRepFill_ShapeLaw::BRepFill_ShapeLaw(const TopoVertex& V, const Standard_Boolean Build)
    : vertex(Standard_True),
      myShape(V)
{
  TheLaw.Nullify();
  uclosed = Standard_False;
  vclosed = Standard_True; // constant law
  myEdges = new (HArray1OfShape)(1, 1);
  myEdges->SetValue(1, V);

  if (Build)
  {
    myLaws = new (SectionLawArray)(1, 1);
    //    Point3d Origine;
    Dir3d                    D(1, 0, 0); // Following the normal
    Handle(GeomLine)         L    = new (GeomLine)(BRepInspector::Pnt(V), D);
    Standard_Real             Last = 2 * BRepInspector::Tolerance(V) + Precision1::PConfusion();
    Handle(Geom_TrimmedCurve) TC   = new (Geom_TrimmedCurve)(L, 0, Last);

    myLaws->ChangeValue(1) = new (GeomFill_UniformSection)(TC);
  }
  myDone = Standard_True;
}

//=================================================================================================

BRepFill_ShapeLaw::BRepFill_ShapeLaw(const TopoWire& W, const Standard_Boolean Build)
    : vertex(Standard_False),
      myShape(W)

{
  TheLaw.Nullify();
  Init(Build);
  myDone = Standard_True;
}

//=======================================================================
// function : Create
// purpose  : Evolutive Wire
//=======================================================================

BRepFill_ShapeLaw::BRepFill_ShapeLaw(const TopoWire&          W,
                                     const Handle(Function2)& L,
                                     const Standard_Boolean      Build)
    : vertex(Standard_False),
      myShape(W)

{
  TheLaw = L;
  Init(Build);
  myDone = Standard_True;
}

//=======================================================================
// function : Init
// purpose  : Case of the wire : Create a table of SectionLaw
//=======================================================================
void BRepFill_ShapeLaw::Init(const Standard_Boolean Build)
{
  vclosed = Standard_True;
  BRepTools_WireExplorer wexp;
  TopoEdge            E;
  Standard_Integer       NbEdge, ii;
  Standard_Real          First, Last;
  TopoWire            W;
  W = TopoDS::Wire(myShape);

  for (NbEdge = 0, wexp.Init(W); wexp.More(); wexp.Next())
  {
    E = wexp.Current();
    if (!E.IsNull() && !BRepInspector::Degenerated(E))
    {
      Handle(GeomCurve3d) C = BRepInspector::Curve(E, First, Last);
      if (!C.IsNull())
      {
        NbEdge++;
      }
    }
  }

  myLaws  = new SectionLawArray(1, NbEdge);
  myEdges = new HArray1OfShape(1, NbEdge);

  ii = 1;

  for (wexp.Init(W); wexp.More(); wexp.Next())
  {
    E = wexp.Current();
    if (!E.IsNull() && !BRepInspector::Degenerated(wexp.Current()))
    {
      Handle(GeomCurve3d) C = BRepInspector::Curve(E, First, Last);
      if (!C.IsNull())
      {
        myEdges->SetValue(ii, E);
        myIndices.Bind(E, ii);
        if (Build)
        {
          // Handle(GeomCurve3d) C = BRepInspector::Curve(E,First,Last);
          if (E.Orientation() == TopAbs_REVERSED)
          {
            Standard_Real      aux;
            Handle(GeomCurve3d) CBis;
            CBis  = C->Reversed(); // To avoid the deterioration of the topology
            aux   = C->ReversedParameter(First);
            First = C->ReversedParameter(Last);
            Last  = aux;
            C     = CBis;
          }

          Standard_Boolean IsClosed = BRepInspector::IsClosed(E);
          if (IsClosed && Abs(C->FirstParameter() - First) > Precision1::PConfusion())
            IsClosed = Standard_False; // trimmed curve differs

          if ((ii > 1) || !IsClosed)
          { // Trim C
            Handle(Geom_TrimmedCurve) TC = new Geom_TrimmedCurve(C, First, Last);
            C                            = TC;
          }
          // otherwise preserve the integrity of the curve
          if (TheLaw.IsNull())
          {
            myLaws->ChangeValue(ii) = new GeomFill_UniformSection(C);
          }
          else
          {
            myLaws->ChangeValue(ii) = new GeomFill_EvolvedSection(C, TheLaw);
          }
        }
        ii++;
      }
    }
  }

  //  std::cout << "new law" << std::endl;

  //  Is the law closed by U ?
  uclosed = W.Closed();
  if (!uclosed)
  {
    // if not sure about the flag, make check
    TopoEdge   Edge1, Edge2;
    TopoVertex V1, V2;
    Edge1 = TopoDS::Edge(myEdges->Value(myEdges->Length()));
    Edge2 = TopoDS::Edge(myEdges->Value(1));

    if (Edge1.Orientation() == TopAbs_REVERSED)
    {
      V1 = TopExp1::FirstVertex(Edge1);
    }
    else
    {
      V1 = TopExp1::LastVertex(Edge1);
    }

    if (Edge2.Orientation() == TopAbs_REVERSED)
    {
      V2 = TopExp1::LastVertex(Edge2);
    }
    else
    {
      V2 = TopExp1::FirstVertex(Edge2);
    }
    if (V1.IsSame(V2))
    {
      uclosed = Standard_True;
    }
    else
    {
      BRepAdaptor_Curve Curve1(Edge1);
      BRepAdaptor_Curve Curve2(Edge2);
      Standard_Real     U1  = BRepInspector::Parameter(V1, Edge1);
      Standard_Real     U2  = BRepInspector::Parameter(V2, Edge2);
      Standard_Real     Eps = BRepInspector::Tolerance(V2) + BRepInspector::Tolerance(V1);

      uclosed = Curve1.Value(U1).IsEqual(Curve2.Value(U2), Eps);
    }
  }
}

//=================================================================================================

Standard_Boolean BRepFill_ShapeLaw::IsVertex() const
{
  return vertex;
}

//=================================================================================================

Standard_Boolean BRepFill_ShapeLaw::IsConstant() const
{
  return TheLaw.IsNull();
}

//=================================================================================================

TopoVertex BRepFill_ShapeLaw::Vertex(const Standard_Integer Index,
                                        const Standard_Real    Param) const
{
  TopoEdge   E;
  TopoVertex V;
  if (Index <= myEdges->Length())
  {
    E = TopoDS::Edge(myEdges->Value(Index));
    if (E.Orientation() == TopAbs_REVERSED)
      V = TopExp1::LastVertex(E);
    else
      V = TopExp1::FirstVertex(E);
  }
  else if (Index == myEdges->Length() + 1)
  {
    E = TopoDS::Edge(myEdges->Value(Index - 1));
    if (E.Orientation() == TopAbs_REVERSED)
      V = TopExp1::FirstVertex(E);
    else
      V = TopExp1::LastVertex(E);
  }

  if (!TheLaw.IsNull())
  {
    Transform3d T;
    T.SetScale(Point3d(0, 0, 0), TheLaw->Value(Param));
    // TopLoc_Location L(T);
    // V.Move(L);
    V = TopoDS::Vertex(BRepBuilderAPI_Transform(V, T));
  }
  return V;
}

///=======================================================================
// function : VertexTol
// purpose  : Evaluate the hole between 2 edges of the section
//=======================================================================
Standard_Real BRepFill_ShapeLaw::VertexTol(const Standard_Integer Index,
                                           const Standard_Real    Param) const
{
  Standard_Real    Tol = Precision1::Confusion();
  Standard_Integer I1, I2;
  if ((Index == 0) || (Index == myEdges->Length()))
  {
    if (!uclosed)
      return Tol; // The least possible error
    I1 = myEdges->Length();
    I2 = 1;
  }
  else
  {
    I1 = Index;
    I2 = I1 + 1;
  }

  Handle(SectionLaw)      Loi;
  Standard_Integer                 NbPoles, NbKnots, Degree;
  Handle(PointArray1)      Poles;
  Handle(TColStd_HArray1OfReal)    Knots, Weigth;
  Handle(TColStd_HArray1OfInteger) Mults;
  Handle(BSplineCurve3d)        BS;
  Point3d                           PFirst;

  Loi = myLaws->Value(I1);
  Loi->SectionShape(NbPoles, NbKnots, Degree);
  Poles  = new (PointArray1)(1, NbPoles);
  Weigth = new (TColStd_HArray1OfReal)(1, NbPoles);
  Loi->D0(Param, Poles->ChangeArray1(), Weigth->ChangeArray1());
  Knots = new (TColStd_HArray1OfReal)(1, NbKnots);
  Loi->Knots(Knots->ChangeArray1());
  Mults = new (TColStd_HArray1OfInteger)(1, NbKnots);
  Loi->Mults(Mults->ChangeArray1());
  BS     = new (BSplineCurve3d)(Poles->Array1(),
                               Weigth->Array1(),
                               Knots->Array1(),
                               Mults->Array1(),
                               Degree,
                               Loi->IsUPeriodic());
  PFirst = BS->Value(Knots->Value(Knots->Length()));

  Loi = myLaws->Value(I2);
  Loi->SectionShape(NbPoles, NbKnots, Degree);
  Poles  = new (PointArray1)(1, NbPoles);
  Weigth = new (TColStd_HArray1OfReal)(1, NbPoles);
  Loi->D0(Param, Poles->ChangeArray1(), Weigth->ChangeArray1());
  Knots = new (TColStd_HArray1OfReal)(1, NbKnots);
  Loi->Knots(Knots->ChangeArray1());
  Mults = new (TColStd_HArray1OfInteger)(1, NbKnots);
  Loi->Mults(Mults->ChangeArray1());
  BS = new (BSplineCurve3d)(Poles->Array1(),
                               Weigth->Array1(),
                               Knots->Array1(),
                               Mults->Array1(),
                               Degree,
                               Loi->IsUPeriodic());
  Tol += PFirst.Distance(BS->Value(Knots->Value(1)));
  return Tol;
}

//=================================================================================================

Handle(SectionLaw) BRepFill_ShapeLaw::ConcatenedLaw() const
{
  Handle(SectionLaw) Law1;
  if (myLaws->Length() == 1)
    return myLaws->Value(1);
  else
  {
    TopoWire   W;
    TopoVertex V;
    W = TopoDS::Wire(myShape);
    if (!W.IsNull())
    {
      //  Concatenation of edges
      Standard_Integer          ii;
      Standard_Real             epsV, f, l;
      Standard_Boolean          Bof;
      Handle(GeomCurve3d)        Composite;
      Handle(Geom_TrimmedCurve) TC;
      Composite = BRepInspector::Curve(Edge(1), f, l);
      TC        = new (Geom_TrimmedCurve)(Composite, f, l);
      GeomConvert_CompCurveToBSplineCurve Concat(TC);

      for (ii = 2, Bof = Standard_True; ii <= myEdges->Length() && Bof; ii++)
      {
        Composite = BRepInspector::Curve(Edge(ii), f, l);
        TC        = new (Geom_TrimmedCurve)(Composite, f, l);
        Bof       = TopExp1::CommonVertex(Edge(ii - 1), Edge(ii), V);
        if (Bof)
        {
          epsV = BRepInspector::Tolerance(V);
        }
        else
          epsV = 10 * Precision1::PConfusion();
        Bof = Concat.Add(TC, epsV, Standard_True, Standard_False, 20);
        if (!Bof)
          Bof = Concat.Add(TC, 200 * epsV, Standard_True, Standard_False, 20);
#ifdef OCCT_DEBUG
        if (!Bof)
          std::cout << "BRepFill_ShapeLaw::ConcatenedLaw INCOMPLET !!!" << std::endl;
#endif
      }
      Composite = Concat.BSplineCurve();

      if (TheLaw.IsNull())
      {
        Law1 = new (GeomFill_UniformSection)(Composite);
      }
      else
      {
        Law1 = new (GeomFill_EvolvedSection)(Composite, TheLaw);
      }
    }
  }
  return Law1;
}

//=================================================================================================

GeomAbs_Shape BRepFill_ShapeLaw::Continuity(const Standard_Integer Index,
                                            const Standard_Real    TolAngular) const
{

  TopoEdge Edge1, Edge2;
  if ((Index == 0) || (Index == myEdges->Length()))
  {
    if (!uclosed)
      return GeomAbs_C0; // The least possible error

    Edge1 = TopoDS::Edge(myEdges->Value(myEdges->Length()));
    Edge2 = TopoDS::Edge(myEdges->Value(1));
  }
  else
  {
    Edge1 = TopoDS::Edge(myEdges->Value(Index));
    Edge2 = TopoDS::Edge(myEdges->Value(Index + 1));
  }

  TopoVertex V1, V2; // common vertex
  TopoVertex vv1, vv2, vv3, vv4;
  TopExp1::Vertices(Edge1, vv1, vv2);
  TopExp1::Vertices(Edge2, vv3, vv4);
  if (vv1.IsSame(vv3))
  {
    V1 = vv1;
    V2 = vv3;
  }
  else if (vv1.IsSame(vv4))
  {
    V1 = vv1;
    V2 = vv4;
  }
  else if (vv2.IsSame(vv3))
  {
    V1 = vv2;
    V2 = vv3;
  }
  else
  {
    V1 = vv2;
    V2 = vv4;
  }

  Standard_Real     U1 = BRepInspector::Parameter(V1, Edge1);
  Standard_Real     U2 = BRepInspector::Parameter(V2, Edge2);
  BRepAdaptor_Curve Curve1(Edge1);
  BRepAdaptor_Curve Curve2(Edge2);
  Standard_Real     Eps = BRepInspector::Tolerance(V2) + BRepInspector::Tolerance(V1);
  GeomAbs_Shape     cont;
  cont = BRepLProp1::Continuity(Curve1, Curve2, U1, U2, Eps, TolAngular);

  return cont;
}

//=================================================================================================

void BRepFill_ShapeLaw::D0(const Standard_Real U, TopoShape& S)
{
  S = myShape;
  if (!TheLaw.IsNull())
  {
    Transform3d T;
    T.SetScale(Point3d(0, 0, 0), TheLaw->Value(U));
    // TopLoc_Location L(T);
    // S.Move(L);
    S = BRepBuilderAPI_Transform(S, T);
  }
}
