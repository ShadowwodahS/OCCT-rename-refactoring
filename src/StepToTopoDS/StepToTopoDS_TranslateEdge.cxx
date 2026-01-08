// Created on: 1995-01-03
// Created by: Frederic MAUPAS
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

//: o0 abv 16.02.99: POLYLINE allowed as 3d curve of edge
// gka,abv 05.04.99: S4136: improving tolerance management, eliminate BRepAPI::Precision()

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeConstruct_Curve.hxx>
#include <StdFail_NotDone.hxx>
#include <StepData_Factors.hxx>
#include <StepGeom_Pcurve.hxx>
#include <StepGeom_SurfaceCurve.hxx>
#include <StepRepr_DefinitionalRepresentation.hxx>
#include <StepShape_EdgeCurve.hxx>
#include <StepShape_OrientedEdge.hxx>
#include <StepShape_Vertex.hxx>
#include <StepShape_VertexPoint.hxx>
#include <StepToGeom.hxx>
#include <StepToTopoDS.hxx>
#include <StepToTopoDS_GeometricTool.hxx>
#include <StepToTopoDS_NMTool.hxx>
#include <StepToTopoDS_Tool.hxx>
#include <StepToTopoDS_TranslateEdge.hxx>
#include <StepToTopoDS_TranslateVertex.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <Transfer_TransientProcess.hxx>
#include <GeomConvert_Units.hxx>
#include <Standard_Failure.hxx>

// #include <StepGeom_Polyline.hxx>
// #include <TransferBRep.hxx>
//: d8
// Used in I-DEAS-like STP processing (ssv; 15.11.2010)
// #define DEBUG
// ============================================================================
// Method  : DecodeMakeEdgeError
// Purpose :
// ============================================================================
static void DecodeMakeEdgeError(const BRepLib_MakeEdge&           ME,
                                const Handle(RefObject)& orig,
                                const Handle(GeomCurve3d)&         myCurve,
                                const TopoVertex&              V1,
                                const TopoVertex&              V2,
                                const Standard_Real&              U1,
                                const Standard_Real&              U2,
                                StepToTopoDS_Tool&                aTool,
                                const Handle(StepShape_TopologicalRepresentationItem)& /*tobind*/)
{
  (void)U1, (void)U2; // avoid compiler warning

  Handle(Transfer_TransientProcess) TP = aTool.TransientProcess();

#ifdef OCCT_DEBUG
  std::cout << "------------------------------------" << std::endl;
  std::cout << "MakeEdge Error  : " << ME.Error() << " - ";
#endif
  switch (ME.Error())
  {
    case (BRepLib_EdgeDone):
      return;
    case (BRepLib_PointProjectionFailed):
      TP->AddFail(orig, " Point Projection failed");
      break;
    case (BRepLib_ParameterOutOfRange):
      TP->AddFail(orig, " Parameter Out Of Range");
      break;
    case (BRepLib_DifferentPointsOnClosedCurve):
      TP->AddFail(orig, " Different Points on Closed Curve");
      break;
    case (BRepLib_PointWithInfiniteParameter):
      TP->AddFail(orig, " Point with infinite Parameter");
      break;
    case (BRepLib_DifferentsPointAndParameter):
      if (!ShapeConstruct_Curve().AdjustCurve(myCurve,
                                              BRepInspector::Pnt(V1),
                                              BRepInspector::Pnt(V2),
                                              Standard_True,
                                              Standard_True))
        TP->AddFail(orig, " Different Points and Parameters");
      else
        TP->AddWarning(orig, "Different Points and Parameters, adjusted");
      break;
    case (BRepLib_LineThroughIdenticPoints):
      TP->AddFail(orig, " Line through identic Points");
      break;
  }
#ifdef OCCT_DEBUG
  std::cout << "Original Type   : " << orig->DynamicType() << std::endl;
  std::cout << "3D Curve Type   : " << myCurve->DynamicType() << std::endl;
  std::cout << "First Parameter : " << U1 << std::endl;
  Point3d p1 = BRepInspector::Pnt(V1);
  //  std::cout << "First Point     : ";
  std::cout << "First Vertex    : " << p1.X() << "  " << p1.Y() << "  " << p1.Z() << "  ";
  std::cout << "Distance Point - Vertex : ";
  Standard_Real d1 = p1.Distance(myCurve->Value(U1));
  std::cout << d1 << std::endl;
  std::cout << "Last  Parameter : " << U2 << std::endl;
  Point3d p2 = BRepInspector::Pnt(V2);
  //  std::cout << "Last  Point     : ";
  std::cout << "Last  Vertex    : " << p2.X() << "  " << p2.Y() << "  " << p2.Z() << "  ";
  std::cout << "Distance Point - Vertex : ";
  Standard_Real d2 = BRepInspector::Pnt(V2).Distance(myCurve->Value(U2));
  std::cout << d2 << std::endl;
#endif
}

// ============================================================================
// Method  : StepToTopoDS_TranslateEdge::StepToTopoDS_TranslateEdge
// Purpose : Empty Constructor
// ============================================================================

static Handle(GeomCurve3d) MakeCurve(const Handle(StepGeom_Curve)&            C1,
                                    const Handle(Transfer_TransientProcess)& TP,
                                    const ConversionFactors&                  theLocalFactors)
{
  Handle(GeomCurve3d) C2 = Handle(GeomCurve3d)::DownCast(TP->FindTransient(C1));
  if (!C2.IsNull())
    return C2;
  C2 = StepToGeom1::MakeCurve(C1, theLocalFactors);
  if (!C2.IsNull())
    TP->BindTransient(C1, C2);
  return C2;
}

static TopoEdge MakeEdge(const Handle(GeomCurve3d)& C3D,
                            const TopoVertex&      V1,
                            const TopoVertex&      V2,
                            const Standard_Real       U1,
                            const Standard_Real       U2)
{
  ShapeBuilder B;
  TopoEdge  E;
  B.MakeEdge(E, C3D, Precision::Confusion());
  B.Add(E, V1);
  B.Add(E, V2);
  B.UpdateVertex(V1, U1, E, 0.);
  B.UpdateVertex(V2, U2, E, 0.);
  return E;
}

// ============================================================================
// Method  : StepToTopoDS_TranslateEdge::StepToTopoDS_TranslateEdge()
// Purpose :
// ============================================================================

StepToTopoDS_TranslateEdge::StepToTopoDS_TranslateEdge()
    : myError(StepToTopoDS_TranslateEdgeOther)
{
  done = Standard_False;
}

// ============================================================================
// Method  : StepToTopoDS_TranslateEdge::StepToTopoDS_TranslateEdge()
// Purpose : Constructor with an Edge and a Tool
// ============================================================================

StepToTopoDS_TranslateEdge::StepToTopoDS_TranslateEdge(const Handle(StepShape_Edge)& E,
                                                       StepToTopoDS_Tool&            T,
                                                       NamingTool2&          NMTool,
                                                       const ConversionFactors& theLocalFactors)
{
  Init(E, T, NMTool, theLocalFactors);
}

// ============================================================================
// Method  : Init
// Purpose : Init with an Edge and a Tool.
//           This method builds an Edge With 2 Vertices and 2 Parameters.
//           The Edge is always build like FORWARD (BRepLib_MakeEdge)
// ============================================================================

void StepToTopoDS_TranslateEdge::Init(const Handle(StepShape_Edge)& aEdge,
                                      StepToTopoDS_Tool&            aTool,
                                      NamingTool2&          NMTool,
                                      const ConversionFactors&       theLocalFactors)
{
  Handle(Transfer_TransientProcess) TP = aTool.TransientProcess();

  Handle(StepShape_OrientedEdge) OE    = Handle(StepShape_OrientedEdge)::DownCast(aEdge);
  Handle(StepShape_Edge)         wEdge = aEdge;
  if (!OE.IsNull())
    wEdge = OE->EdgeElement();
  Handle(StepShape_EdgeCurve) EC = Handle(StepShape_EdgeCurve)::DownCast(wEdge);

  if (aTool.IsBound(EC))
  {
    myResult = aTool.Find(EC);
    if (BRepInspector::Degenerated(TopoDS::Edge(myResult)))
    {
      TP->AddWarning(EC, "Degenerated Edge in several faces : transferred for each face");
    }
    else
    {
      myError = StepToTopoDS_TranslateEdgeDone;
      done    = Standard_True;
      //: S4136      B.SameRange(TopoDS::Edge(myResult), Standard_False);    //:a5 abv 11 Feb 98
      //: S4136      B.SameParameter(TopoDS::Edge(myResult), Standard_False);//:a5
      return;
    }
  }

  // [BEGIN] Proceed with non-manifold cases (ssv; 12.11.2010)
  if (NMTool.IsActive() && NMTool.IsBound(EC))
  {
    TopoShape existingShape = NMTool.Find(EC);
    // Reverse shape's orientation if needed
    if (!OE->Orientation())
      existingShape.Reverse();
    myResult = existingShape;
    myError  = StepToTopoDS_TranslateEdgeDone;
    done     = Standard_True;
    return;
  }
  // [END] Proceed with non-manifold cases (ssv; 12.11.2010)

  // [BEGIN] Proceed with I-DEAS-like STP (ssv; 15.11.2010)
  const Handle(TCollection_HAsciiString) anECName = EC->Name();
  if (NMTool.IsIDEASCase() && !anECName.IsNull() && !anECName->IsEmpty()
      && NMTool.IsBound(anECName->String()))
  {
    TopoShape existingShape = NMTool.Find(anECName->String());
    // Reverse shape's orientation if needed
    if (!OE->Orientation())
      existingShape.Reverse();
    // Register Edge for final processing (I-DEAS case)
    NMTool.RegisterNMEdge(existingShape);
    myResult = existingShape;
    myError  = StepToTopoDS_TranslateEdgeDone;
    done     = Standard_True;
    return;
  }
  // [END] Proceed with I-DEAS-like STP (ssv; 15.11.2010)

  ShapeBuilder B;

  Handle(StepGeom_Curve) C = EC->EdgeGeometry();
  if (C.IsNull())
  {
    TP->AddFail(EC, " Geom Curve in EdgeCurve is equal to 0");
    myError = StepToTopoDS_TranslateEdgeOther;
    done    = Standard_False;
    return;
  }
  TopoEdge              E;
  Handle(StepShape_Vertex) Vstart, Vend;

  // -----------------------------------------------------------
  // Extract the start and end Vertices corresponding to FORWARD
  // (following the geometrical sense)
  // -----------------------------------------------------------

  Standard_Boolean EdgeCurveSameSense = EC->SameSense();

  if (EdgeCurveSameSense)
  {
    Vstart = EC->EdgeStart();
    Vend   = EC->EdgeEnd();
  }
  else
  {
    Vend   = EC->EdgeStart();
    Vstart = EC->EdgeEnd();
  }

  TopoVertex V1, V2;

  StepToTopoDS_TranslateVertex myTranVertex1(Vstart, aTool, NMTool, theLocalFactors);
  StepToTopoDS_TranslateVertex myTranVertex2(Vend, aTool, NMTool, theLocalFactors);

  if (myTranVertex1.IsDone())
  {
    V1 = TopoDS::Vertex(myTranVertex1.Value());
    V1.Orientation(TopAbs_FORWARD);
  }
  if (Vend == Vstart)
  {
    V2 = V1;
    V2.Orientation(TopAbs_REVERSED);
  }
  else if (myTranVertex2.IsDone())
  {
    V2 = TopoDS::Vertex(myTranVertex2.Value());
    V2.Orientation(TopAbs_REVERSED);
  }
  done = Standard_True;

  // ----------------------------------------------------------
  // --- The EdgeCurve Geometry is of StepGeom_Curve Type
  // --- It can be : * a Pcurve : no 3D curve is constructed
  // ---             * a Surface Curve, Intersection Curve
  // ---               or a Seam Curve
  // ---             * a 3D Curve
  // ----------------------------------------------------------

  if (C->IsKind(STANDARD_TYPE(StepGeom_Pcurve)))
  {
    B.MakeEdge(E);
    //: S4136    B.UpdateEdge (E,preci);
    B.Add(E, V1);
    B.Add(E, V2);
  }
  else if (C->IsKind(STANDARD_TYPE(StepGeom_SurfaceCurve)))
  {
    // For SeamCurve and IntersectionCurve types
    // --- The Edge Geometry is a Surface Curve ---
    // ---     (3d + 2 Pcurve Or Surface)       ---
    Handle(StepGeom_SurfaceCurve) Sc = Handle(StepGeom_SurfaceCurve)::DownCast(C);
    Handle(StepGeom_Curve)        C1 = Sc->Curve3d();
    MakeFromCurve3D(C1, EC, Vend, Precision(), E, V1, V2, aTool, theLocalFactors);
  }
  else
  {
    // --- The Edge Geometry is a Single 3d Curve ---
    MakeFromCurve3D(C, EC, Vend, Precision(), E, V1, V2, aTool, theLocalFactors);
  }
  // Force set flags SameRange and SameParameter to Standard_False
  if (done)
  {
    //: S4136    B.SameRange(E, Standard_False);
    //: S4136    B.SameParameter(E, Standard_False);
    aTool.Bind(EC, E);

    // Bind Edge in NM tool (ssv; 15.11.2010)
    if (NMTool.IsActive())
    {
      NMTool.Bind(EC, E);
      if (NMTool.IsIDEASCase() && !anECName.IsNull() && !anECName->IsEmpty())
        NMTool.Bind(anECName->String(), E);
    }

    myResult = E;
    myError  = StepToTopoDS_TranslateEdgeDone;
  }
}

// ============================================================================
// Method  : MakeFromCurve3D
// Purpose : case of a Curve 3D (alone or in SurfaceCurve)
// ============================================================================

// auxiliary function
//: e6 abv 16 Apr 98: ProSTEP TR8, r0601_sy.stp, #14907
static void GetCartesianPoints(const Handle(StepShape_EdgeCurve)& EC,
                               Point3d&                            P1,
                               Point3d&                            P2,
                               const ConversionFactors&            theLocalFactors)
{
  for (Standard_Integer i = 1; i <= 2; i++)
  {
    const Handle(StepShape_Vertex) V =
      ((i == 1) == EC->SameSense() ? EC->EdgeStart() : EC->EdgeEnd());
    const Handle(StepShape_VertexPoint) VP = Handle(StepShape_VertexPoint)::DownCast(V);
    if (VP.IsNull())
      continue;
    const Handle(StepGeom_CartesianPoint) P =
      Handle(StepGeom_CartesianPoint)::DownCast(VP->VertexGeometry());
    Handle(Geom_CartesianPoint) CP = StepToGeom1::MakeCartesianPoint(P, theLocalFactors);
    (i == 1 ? P1 : P2)             = CP->Pnt();
  }
}

// ============================================================================
// Method  : StepToTopoDS_TranslateEdge::MakeFromCurve3D()
// Purpose :
// ============================================================================

void StepToTopoDS_TranslateEdge::MakeFromCurve3D(const Handle(StepGeom_Curve)&      C3D,
                                                 const Handle(StepShape_EdgeCurve)& EC,
                                                 const Handle(StepShape_Vertex)&    Vend,
                                                 const Standard_Real                preci,
                                                 TopoEdge&                       E,
                                                 TopoVertex&                     V1,
                                                 TopoVertex&                     V2,
                                                 StepToTopoDS_Tool&                 aTool,
                                                 const ConversionFactors&            theLocalFactors)
{
  Handle(Transfer_TransientProcess) TP = aTool.TransientProcess();
  Handle(GeomCurve3d)                C1 = MakeCurve(C3D, TP, theLocalFactors);
  if (C1.IsNull())
  {
    TP->AddFail(C3D, " Make GeomCurve3d (3D) failed");
    myError = StepToTopoDS_TranslateEdgeOther;
    done    = Standard_False;
    return;
  }
  // -- Statistics -- -> No Warning message
  aTool.AddContinuity(C1);
  ShapeBuilder  B;
  Standard_Real temp1, temp2, U1, U2;
  Point3d        pproj;
  Point3d        pv1 = BRepInspector::Pnt(V1);
  Point3d        pv2 = BRepInspector::Pnt(V2);

  //: e6 abv
  Point3d pnt1 = pv1, pnt2 = pv2;
  if (V1.IsSame(V2))
    GetCartesianPoints(EC, pnt1, pnt2, theLocalFactors);
  Curve2 sac;
  temp1 = sac.Project(C1, pnt1, preci, pproj, U1, Standard_False);
  temp2 = sac.Project(C1, pnt2, preci, pproj, U2, Standard_False);

  if (!GeometricTool::UpdateParam3d(C1, U1, U2, preci))
    TP->AddWarning(C3D, "Update of 3D-Parameters has failed");

  //: d5: instead of AdjustCurve above which is incorrect if U1 and U2 are not ends
  GeomAdaptor_Curve aCA(C1);
  Point3d            pU1 = aCA.Value(U1), pU2 = aCA.Value(U2);
  temp1 = pU1.Distance(pv1);
  temp2 = pU2.Distance(pv2);
  if (temp1 > preci || temp2 > preci)
  {
    // #25415: handling of special case found on some STEP files produced by FPX Expert 2013 (PCB
    // design system): edge curve is line displaced from its true position but with correct
    // direction; we can shift the line in this case so that it passes through vertices correctly
    if (Abs(temp1 - temp2) < preci && Abs(U2 - U1 - pnt1.Distance(pnt2)) < Precision::Confusion()
        && C1->IsKind(STANDARD_TYPE(GeomLine)))
    {
      Handle(GeomLine) aLine    = Handle(GeomLine)::DownCast(C1);
      gp_Lin            aLin     = aLine->Lin();
      Point3d            anOrigin = pnt1.XYZ() - aLin.Position().Direction().XYZ() * U1;
      aLin.SetLocation(anOrigin);
      C1 = new GeomLine(aLin);

      TP->AddWarning(C3D, "Poor result from projection vertex / line 3d, line shifted");
    }
    else
    {
      TP->AddWarning(C3D, "Poor result from projection vertex / curve 3d");
    }
  }
  B.UpdateVertex(V1, 1.000001 * temp1); //: h6 abv 14 Jul 98: PRO8845 #2746: *=1.0001
  B.UpdateVertex(V2, 1.000001 * temp2); //: h6

  BRepLib_MakeEdge ME(C1, V1, V2, U1, U2);
  if (ME.IsDone())
  {
    E = ME.Edge();
    B.Range(E, U1, U2); // abv 14 Mar 00: trj3_pm1-ug.stp #91739, edge 2
  }
  else
  {
    if (ME.Error() == BRepLib_DifferentPointsOnClosedCurve)
    {
      // The Edge could be closed and trimmed by 2 Different vertices
      if (C1->IsClosed())
      {
        // Attention : topology updating
        aTool.Bind(Vend, V1);
        TopoShape aLocalShape = V1.Reversed();
        V2                       = TopoDS::Vertex(aLocalShape);
        ME.Init(C1, V1, V2, U1, U2);
        if (ME.IsDone())
        {
          TP->AddWarning(EC, "Wrong topology corrected : Closed Edge with TWO different Vertices");
          E = ME.Edge();
        }
        else
        {
          DecodeMakeEdgeError(ME, C3D, C1, V1, V2, U1, U2, aTool, EC);
          E       = MakeEdge(C1, V1, V2, U1, U2);
          myError = StepToTopoDS_TranslateEdgeDone;
          done    = Standard_True;
          //		return;
        }
      }
      else
      {
        // Then, this is should be coded as degenerated
        // To be performed later !!!
        myError = StepToTopoDS_TranslateEdgeDone;
        //  Bon, on la fait cette petite edge, mais faudra repasser
        //  pour l enlever ET FUSIONNER LES VERTEX, pour tout le shell !
        //  courbe trop petite pour etre mise -> fait planter
        done = Standard_True;
        if (!V1.IsSame(V2))
        {
          TP->AddFail(EC, "This edge has null arc length");
          Point3d P1 = BRepInspector::Pnt(V1);
          Point3d P2 = BRepInspector::Pnt(V2);
          Vector3d avec(P1, P2);
          Dir3d adir(avec);
          gp_Lin alin(P1, adir);
          C1 = new GeomLine(alin);
          U1 = 0.;
          U2 = P1.Distance(P2);
          E  = MakeEdge(C1, V1, V2, U1, U2); //,preci
        }
        else
        {
          TP->AddFail(EC, "NULL EDGE, SKIPPED");
          myResult.Nullify();
          return;
        }
      }
    }
    else
    {
      DecodeMakeEdgeError(ME, C3D, C1, V1, V2, U1, U2, aTool, EC);
      E       = MakeEdge(C1, V1, V2, U1, U2);
      myError = StepToTopoDS_TranslateEdgeDone;
      done    = Standard_True;
    }
  }
}

// ============================================================================
// Method  : MakePCurve
// Purpose : Computes an individual pcurve (i.e. curve 2d)
// ============================================================================
Handle(GeomCurve2d) StepToTopoDS_TranslateEdge::MakePCurve(
  const Handle(StepGeom_Pcurve)& PCU,
  const Handle(GeomSurface)&    ConvSurf,
  const ConversionFactors&        theLocalFactors) const
{
  Handle(GeomCurve2d)                              C2d;
  const Handle(StepRepr_DefinitionalRepresentation) DRI = PCU->ReferenceToCurve();
  if (DRI.IsNull())
    return C2d;
  const Handle(StepGeom_Curve) StepCurve = Handle(StepGeom_Curve)::DownCast(DRI->ItemsValue(1));
  try
  {
    C2d = StepToGeom1::MakeCurve2d(StepCurve, theLocalFactors);
    if (!C2d.IsNull())
    {
      // -- if the surface is a RectangularTrimmedSurface,
      // -- send the BasisSurface.
      C2d = Units1::DegreeToRadian(C2d,
                                              ConvSurf,
                                              theLocalFactors.LengthFactor(),
                                              theLocalFactors.FactorDegreeRadian());
    }
  }
  catch (ExceptionBase const&)
  {
    return C2d;
  }
  return C2d;
}

// ============================================================================
// Method  : Value
// Purpose : Returns the mapped edge
// ============================================================================

const TopoShape& StepToTopoDS_TranslateEdge::Value() const
{
  StdFail_NotDone_Raise_if(!done, "StepToTopoDS_TranslateEdge::Value() - no result");
  return myResult;
}

// ============================================================================
// Method  : Error
// Purpose : Returns the error code
// ============================================================================

StepToTopoDS_TranslateEdgeError StepToTopoDS_TranslateEdge::Error() const
{
  return myError;
}
