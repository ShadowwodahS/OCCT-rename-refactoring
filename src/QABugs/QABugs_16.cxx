// Created on: 2002-03-18
// Created by: QA Admin
// Copyright (c) 2002-2014 OPEN CASCADE SAS
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

#include <QABugs.hxx>

#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <DBRep.hxx>
#include <DrawTrSurf.hxx>
#include <AIS_InteractiveContext.hxx>
#include <ViewerTest.hxx>
#include <AIS_Shape.hxx>
#include <TopoDS_Shape.hxx>

#include <BRepGProp.hxx>
#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <gp_Ax2.hxx>
#include <Geom_Circle.hxx>
#include <AIS_Circle.hxx>
#include <gp_Pln.hxx>
#include <PrsDim_AngleDimension.hxx>

#include <Aspect_Window.hxx>
#include <V3d_View.hxx>

#include <TopExp_Explorer.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GC_MakePlane.hxx>
#include <AIS_PlaneTrihedron.hxx>
#include <ViewerTest_DoubleMapOfInteractiveAndName.hxx>

#include <BRep_Tool.hxx>
#include <Geom_BSplineCurve.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Precision.hxx>

#include <GProp_PrincipalProps.hxx>

#include <OSD_Path.hxx>
#include <Standard_ProgramError.hxx>

#include <ShapeFix_Wireframe.hxx>
#include <ShapeBuild_ReShape.hxx>

#include <ViewerTest_EventManager.hxx>

#include <DDocStd.hxx>
#include <TDocStd_Document.hxx>
#include <Standard_ErrorHandler.hxx>

#if !defined(_WIN32)
extern ViewerTest_DoubleMapOfInteractiveAndName& GetMapOfAIS();
#else
Standard_EXPORT ViewerTest_DoubleMapOfInteractiveAndName& GetMapOfAIS();
#endif

static Standard_Integer BUC60848(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " shape \n";
    return 1;
  }
  TopoShape S = DBRep1::Get(argv[1]);
  if (S.IsNull())
  {
    di << "Shape is empty\n";
    return 1;
  }
  GeometricProperties G;
  BRepGProp::VolumeProperties(S, G);
  Standard_Real GRes;
  GRes = G.Mass();
  if (GRes < 0)
  {
    di << "Result is negative : " << GRes << "\n";
    return 1;
  }
  else
  {
    di << "Volume : " << GRes << "\n";
  }

  return 0;
}

static Standard_Integer BUC60828(DrawInterpreter& di,
                                 Standard_Integer /*argc*/,
                                 const char** /*argv*/)
{
  TopoEdge      anEdge = EdgeMaker(Point3d(0., 0., 0.), Point3d(0., 0., 1.));
  Standard_Boolean aValue;
  aValue = anEdge.Infinite();
  di << "Initial flag : " << (Standard_Integer)aValue << "\n";
  anEdge.Infinite(Standard_True);
  Standard_Boolean aValue1;
  aValue1 = anEdge.Infinite();
  di << "Current flag : " << (Standard_Integer)aValue1 << "\n";
  if (aValue1)
    di << "Flag was set properly.\n";
  else
    di << "Faulty : flag was not set properly.\n";
  return 0;
}

static Standard_Integer BUC60814(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 1)
  {
    di << "Usage : " << argv[0] << "\n";
    return 1;
  }

  Handle(VisualContext) myAISContext = ViewerTest::GetAISContext();
  if (myAISContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return 1;
  }

  // TRIHEDRON
  Handle(VisualEntity) aTrihedron;
  Handle(Geom_Axis2Placement)   aTrihedronAxis = new Geom_Axis2Placement(gp1::XOY());
  aTrihedron                                   = new AIS_Trihedron(aTrihedronAxis);
  myAISContext->Display(aTrihedron, Standard_False);

  // Circle
  Point3d P(10, 10, 10);
  Dir3d V(1, 0, 0);
  Frame3d aAx2(P, V);

  Handle(GeomCircle)           ahCircle = new GeomCircle(aAx2, 20);
  Handle(VisualEntity) aCircle  = new AIS_Circle(ahCircle);
  myAISContext->Display(aCircle, Standard_False);

  const Handle(StyleDrawer)& aSelStyle = myAISContext->SelectionStyle();
  aSelStyle->SetColor(Quantity_NOC_BLUE1);

  myAISContext->AddOrRemoveSelected(aTrihedron, Standard_False);
  myAISContext->AddOrRemoveSelected(aCircle, Standard_True);

  return 0;
}

//=================================================================================================

static Standard_Integer BUC60774(DrawInterpreter& theDi,
                                 Standard_Integer  theArgNb,
                                 const char**      theArgv)
{
  if (theArgNb != 1)
  {
    std::cout << "Usage : " << theArgv[0] << "\n";
    return -1;
  }

  const Handle(VisualContext)& anAISContext = ViewerTest::GetAISContext();
  if (anAISContext.IsNull())
  {
    std::cout << "use 'vinit' command before " << theArgv[0] << "\n";
    return -1;
  }

  const Handle(ViewWindow)& aV3dView = ViewerTest::CurrentView();

  Standard_Integer aWinWidth  = 0;
  Standard_Integer aWinHeight = 0;
  aV3dView->Window()->Size(aWinWidth, aWinHeight);

  Standard_Integer aXPixMin = 0;
  Standard_Integer aYPixMin = 0;
  Standard_Integer aXPixMax = aWinWidth;
  Standard_Integer aYPixMax = aWinHeight;

  AIS_StatusOfPick aPickStatus = anAISContext->SelectRectangle(Graphic3d_Vec2i(aXPixMin, aYPixMin),
                                                               Graphic3d_Vec2i(aXPixMax, aYPixMax),
                                                               aV3dView);
  theDi << (aPickStatus == AIS_SOP_NothingSelected
              ? "status = AIS_SOP_NothingSelected : OK"
              : "status = AIS_SOP_NothingSelected : bugged - Faulty ");
  theDi << "\n";

  theDi.Eval("box b 10 10 10");
  theDi.Eval(" vdisplay b");

  aPickStatus = anAISContext->SelectRectangle(Graphic3d_Vec2i(aXPixMin, aYPixMin),
                                              Graphic3d_Vec2i(aXPixMax, aYPixMax),
                                              aV3dView);
  theDi << (aPickStatus == AIS_SOP_OneSelected ? "status = AIS_SOP_OneSelected : OK"
                                               : "status = AIS_SOP_OneSelected : bugged - Faulty ");
  theDi << "\n";

  theDi.Eval("box w 20 20 20 20 20 20");
  theDi.Eval(" vdisplay w");

  aPickStatus = anAISContext->SelectRectangle(Graphic3d_Vec2i(aXPixMin, aYPixMin),
                                              Graphic3d_Vec2i(aXPixMax, aYPixMax),
                                              aV3dView);
  anAISContext->UpdateCurrentViewer();
  theDi << (aPickStatus == AIS_SOP_SeveralSelected
              ? "status = AIS_SOP_SeveralSelected : OK"
              : "status = AIS_SOP_SeveralSelected : bugged - Faulty ");
  theDi << "\n";

  return 0;
}

static Standard_Integer BUC60972(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  Handle(VisualContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return 1;
  }

  if (argc != 6)
  {
    di << "Usage : " << argv[0] << " edge edge plane val text\n";
    return 1;
  }

  TopoEdge        aFirst  = TopoDS::Edge(DBRep1::Get(argv[1], TopAbs_EDGE));
  TopoEdge        aSecond = TopoDS::Edge(DBRep1::Get(argv[2], TopAbs_EDGE));
  Handle(GeomPlane) aPlane  = Handle(GeomPlane)::DownCast(DrawTrSurf1::GetSurface(argv[3]));
  if (aPlane.IsNull())
    return 1;

  di << aPlane->Pln().SquareDistance(Point3d(0, 0, 0)) << "\n";

  UtfString aText(argv[5]);
  // Standard_ExtString ExtString_aText = aText.ToExtString();
  // di << ExtString_aText << " " << Draw1::Atof(argv[4]) << "\n";
  di << argv[5] << " " << Draw1::Atof(argv[4]) << "\n";

  Handle(PrsDim_AngleDimension) aDim = new PrsDim_AngleDimension(aFirst, aSecond);
  aContext->Display(aDim, Standard_True);

  return 0;
}

static Standard_Integer OCC218bug(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  Handle(VisualContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return 1;
  }

  if (argc != 5)
  {
    di << "Usage : " << argv[0] << " name plane Xlabel Ylabel\n";
    return 1;
  }

  TopoShape S = DBRep1::Get(argv[2]);
  if (S.IsNull())
  {
    di << "Shape is empty\n";
    return 1;
  }

  AsciiString1 name(argv[1]);
  AsciiString1 Xlabel(argv[3]);
  AsciiString1 Ylabel(argv[4]);

  // Construction de l'AIS_PlaneTrihedron
  Handle(AIS_PlaneTrihedron) theAISPlaneTri;

  Standard_Boolean IsBound = GetMapOfAIS().IsBound2(name);
  if (IsBound)
  {
    // on recupere la shape dans la map des objets displayes
    Handle(VisualEntity) aShape = GetMapOfAIS().Find2(name);

    // On verifie que l'AIS InteraciveObject est bien
    // un AIS_PlaneTrihedron
    if (aShape->Type() == AIS_KindOfInteractive_Datum && aShape->Signature() == 4)
    {
      // On downcast aShape de VisualEntity a AIS_PlaneTrihedron
      theAISPlaneTri = Handle(AIS_PlaneTrihedron)::DownCast(aShape);

      theAISPlaneTri->SetXLabel(Xlabel);
      theAISPlaneTri->SetYLabel(Ylabel);

      aContext->Redisplay(theAISPlaneTri, Standard_False);
      aContext->UpdateCurrentViewer();
    }
  }
  else
  {
    TopoFace FaceB = TopoDS::Face(S);

    // Construction du Plane1
    // recuperation des edges des faces.
    ShapeExplorer FaceExpB(FaceB, TopAbs_EDGE);

    TopoEdge EdgeB = TopoDS::Edge(FaceExpB.Current());
    // declarations
    Point3d A, B, C;

    // si il y a plusieurs edges
    if (FaceExpB.More())
    {
      FaceExpB.Next();
      TopoEdge       EdgeC = TopoDS::Edge(FaceExpB.Current());
      BRepAdaptor_Curve theCurveB(EdgeB);
      BRepAdaptor_Curve theCurveC(EdgeC);
      A = theCurveC.Value(0.1);
      B = theCurveC.Value(0.9);
      C = theCurveB.Value(0.5);
    }
    else
    {
      // FaceB a 1 unique edge courbe
      BRepAdaptor_Curve theCurveB(EdgeB);
      A = theCurveB.Value(0.1);
      B = theCurveB.Value(0.9);
      C = theCurveB.Value(0.5);
    }
    // Construction du GeomPlane
    GC_MakePlane              MkPlane(A, B, C);
    const Handle(GeomPlane)& theGeomPlane = MkPlane.Value();

    // on le display & bind
    theAISPlaneTri = new AIS_PlaneTrihedron(theGeomPlane);

    theAISPlaneTri->SetXLabel(Xlabel);
    theAISPlaneTri->SetYLabel(Ylabel);

    GetMapOfAIS().Bind(theAISPlaneTri, name);
    aContext->Display(theAISPlaneTri, Standard_True);
  }
  return 0;
}

static Standard_Integer OCC295(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Usage : " << argv[0] << " edge_result edge1 edge2\n";
    return 1;
  }

  TopoShape Sh1 = DBRep1::Get(argv[2]);
  TopoShape Sh2 = DBRep1::Get(argv[3]);
  if (Sh1.IsNull() || Sh2.IsNull())
    return 1;
  if (Sh1.ShapeType() != TopAbs_EDGE || Sh2.ShapeType() != TopAbs_EDGE)
    return 1;
  TopoEdge               e1 = TopoDS::Edge(Sh1);
  TopoEdge               e2 = TopoDS::Edge(Sh2);
  Standard_Real             f1, l1, f2, l2;
  Standard_Boolean          After  = Standard_True;
  Handle(GeomCurve3d)        ac1    = BRepInspector::Curve(e1, f1, l1);
  Handle(GeomCurve3d)        ac2    = BRepInspector::Curve(e2, f2, l2);
  Handle(BSplineCurve3d) bsplc1 = Handle(BSplineCurve3d)::DownCast(ac1);
  Handle(BSplineCurve3d) bsplc2 = Handle(BSplineCurve3d)::DownCast(ac2);
  if (bsplc1.IsNull() || bsplc2.IsNull())
    return 1;
  Point3d pmid = 0.5 * (bsplc1->Pole(bsplc1->NbPoles()).XYZ() + bsplc2->Pole(1).XYZ());
  bsplc1->SetPole(bsplc1->NbPoles(), pmid);
  bsplc2->SetPole(1, pmid);
  GeomConvert_CompCurveToBSplineCurve connect3d(bsplc1);
  if (!connect3d.Add(bsplc2, Precision::Confusion(), After, Standard_False))
    return 1;
  EdgeMaker MkEdge(connect3d.BSplineCurve());
  if (MkEdge.IsDone())
  {
    TopoEdge nedge = MkEdge.Edge();
    DBRep1::Set(argv[1], nedge);
    return 0;
  }
  else
    return 1;
}

static Standard_Integer OCC49(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{

  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " name\n";
    return 1;
  }

  TopoShape S = DBRep1::Get(argv[1]);
  if (S.IsNull())
    return 0;

  GeometricProperties G;
  BRepGProp::VolumeProperties(S, G);
  GProp_PrincipalProps Pr     = G.PrincipalProperties();
  Standard_Boolean     Result = Pr.HasSymmetryAxis();
  if (Result)
  {
    di << "1\n";
  }
  else
  {
    di << "0\n";
  }
  return 0;
}

static Standard_Integer OCC132(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  /*
     OCC132:
     =======

     ... the validation of the name of files in Analyse_DOS and Analyse_UNIX is :

     characters not allowed in DOS/WNT names are
     /
     :
     *
     ?
     "
     <
     >
     |
     and  more than one dot in filename.
     */

  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " DependentName\n";
    return 1;
  }

  OSD_SysType SysType1 = OSD_OS2;
  OSD_SysType SysType2 = OSD_WindowsNT;

  {
    try
    {
      OCC_CATCH_SIGNALS
      SystemPath Path(argv[1], SysType1);
    }
    catch (Standard_ProgramError const&)
    {
      di << "1\n";
      return 0;
    }
  }

  {
    try
    {
      OCC_CATCH_SIGNALS
      SystemPath Path(argv[1], SysType2);
    }
    catch (Standard_ProgramError const&)
    {
      di << "2\n";
      return 0;
    }
  }

  di << "0\n";
  return 0;
}

static Standard_Integer OCC405(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Usage : " << argv[0] << " edge_result edge1 edge2; merge two edges\n";
    return 1;
  }

  TopoShape Sh1 = DBRep1::Get(argv[2]);
  TopoShape Sh2 = DBRep1::Get(argv[3]);
  if (Sh1.IsNull() || Sh2.IsNull())
    return 1;
  if (Sh1.ShapeType() != TopAbs_EDGE || Sh2.ShapeType() != TopAbs_EDGE)
    return 1;
  TopoEdge        e1 = TopoDS::Edge(Sh1);
  TopoEdge        e2 = TopoDS::Edge(Sh2);
  Standard_Real      f1, l1, f2, l2;
  Standard_Boolean   After = Standard_True;
  Handle(GeomCurve3d) ac1   = BRepInspector::Curve(e1, f1, l1);
  Handle(GeomCurve3d) ac2   = BRepInspector::Curve(e2, f2, l2);
  if (e1.Orientation() == TopAbs_REVERSED)
  {
    Standard_Real cf = f1;
    f1               = ac1->ReversedParameter(l1);
    l1               = ac1->ReversedParameter(cf);
    ac1              = ac1->Reversed();
  }
  if (e2.Orientation() == TopAbs_REVERSED)
  {
    Standard_Real cf = f2;
    f2               = ac2->ReversedParameter(l2);
    l2               = ac2->ReversedParameter(cf);
    ac2              = ac2->Reversed();
  }
  Handle(BSplineCurve3d) bsplc1 = Handle(BSplineCurve3d)::DownCast(ac1);
  Handle(BSplineCurve3d) bsplc2 = Handle(BSplineCurve3d)::DownCast(ac2);
  if (bsplc1.IsNull() || bsplc2.IsNull())
    return 1;
  if (bsplc1->FirstParameter() < f1 - Precision::PConfusion()
      || bsplc1->LastParameter() > l1 + Precision::PConfusion())
  {
    Handle(BSplineCurve3d) aBstmp = Handle(BSplineCurve3d)::DownCast(bsplc1->Copy());
    aBstmp->Segment1(f1, l1);
    bsplc1 = aBstmp;
  }
  if (bsplc2->FirstParameter() < f2 - Precision::PConfusion()
      || bsplc2->LastParameter() > l2 + Precision::PConfusion())
  {
    Handle(BSplineCurve3d) aBstmp = Handle(BSplineCurve3d)::DownCast(bsplc2->Copy());
    aBstmp->Segment1(f2, l2);
    bsplc2 = aBstmp;
  }
  Point3d pmid = 0.5 * (bsplc1->Pole(bsplc1->NbPoles()).XYZ() + bsplc2->Pole(1).XYZ());
  bsplc1->SetPole(bsplc1->NbPoles(), pmid);
  bsplc2->SetPole(1, pmid);
  GeomConvert_CompCurveToBSplineCurve connect3d(bsplc1);
  if (!connect3d.Add(bsplc2, Precision::Confusion(), After, Standard_False))
    return 1;
  EdgeMaker MkEdge(connect3d.BSplineCurve());
  if (MkEdge.IsDone())
  {
    TopoEdge nedge = MkEdge.Edge();
    DBRep1::Set(argv[1], nedge);
    return 0;
  }
  else
    return 1;
}

static Standard_Integer OCC395(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Usage : " << argv[0] << " edge_result edge1 edge2\n";
    return 1;
  }
  // AsciiString1 fnom(a[1]);
  // Standard_Boolean modfic = XSDRAW::FileAndVar(a[1],a[2],a[3],"IGES",fnom,rnom,resnom);
  TopoShape Sh1 = DBRep1::Get(argv[2]);
  TopoShape Sh2 = DBRep1::Get(argv[3]);
  if (Sh1.IsNull() || Sh2.IsNull())
    return 1;
  if (Sh1.ShapeType() != TopAbs_EDGE || Sh2.ShapeType() != TopAbs_EDGE)
    return 1;
  TopoEdge        e1 = TopoDS::Edge(Sh1);
  TopoEdge        e2 = TopoDS::Edge(Sh2);
  Standard_Real      f1, l1, f2, l2;
  Standard_Boolean   After = Standard_True;
  Handle(GeomCurve3d) ac1   = BRepInspector::Curve(e1, f1, l1);
  Handle(GeomCurve3d) ac2   = BRepInspector::Curve(e2, f2, l2);
  if (e1.Orientation() == TopAbs_REVERSED)
  {
    // Standard_Real cf = cf1;
    // cf1 = ac1->ReversedParameter ( cl1 );
    // cl1 = ac1->ReversedParameter ( cf );
    ac1 = ac1->Reversed();
  }
  if (e2.Orientation() == TopAbs_REVERSED)
  {
    // Standard_Real cf = cf2;
    // ac2 = ac2->ReversedParameter ( cl2 );
    // ac2 = ac2->ReversedParameter ( cf );
    ac2 = ac2->Reversed();
  }
  Handle(BSplineCurve3d) bsplc1 = Handle(BSplineCurve3d)::DownCast(ac1);
  Handle(BSplineCurve3d) bsplc2 = Handle(BSplineCurve3d)::DownCast(ac2);
  if (bsplc1.IsNull() || bsplc2.IsNull())
    return 1;
  Point3d pmid = 0.5 * (bsplc1->Pole(bsplc1->NbPoles()).XYZ() + bsplc2->Pole(1).XYZ());
  bsplc1->SetPole(bsplc1->NbPoles(), pmid);
  bsplc2->SetPole(1, pmid);
  GeomConvert_CompCurveToBSplineCurve connect3d(bsplc1);
  if (!connect3d.Add(bsplc2, Precision::Confusion(), After, Standard_False))
    return 1;
  EdgeMaker MkEdge(connect3d.BSplineCurve());
  if (MkEdge.IsDone())
  {
    TopoEdge nedge = MkEdge.Edge();
    DBRep1::Set(argv[1], nedge);
    return 0;
  }
  else
    return 1;
}

static Standard_Integer OCC394(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Usage : " << argv[0] << " edge_result edge [tol [mode [tolang]]]\n";
    return 1;
  }
  TopoShape Sh = DBRep1::Get(argv[2]);

  Standard_Integer k      = 3;
  Standard_Real    tol    = 100000;
  Standard_Integer mode   = 2;
  Standard_Real    tolang = M_PI / 2;
  if (argc > k)
    tol = Draw1::Atof(argv[k++]);

  if (argc > k)
    mode = Draw1::Atoi(argv[k++]);

  if (argc > k)
    tolang = Draw1::Atof(argv[k++]);

  Handle(ShapeFix_Wireframe) aSfwr    = new ShapeFix_Wireframe();
  Handle(ShapeBuild_ReShape) aReShape = new ShapeBuild_ReShape;
  aSfwr->SetContext(aReShape);
  aSfwr->Load(Sh);
  aSfwr->SetPrecision(tol);
  Standard_Boolean aModeDrop = Standard_True;
  if (mode == 2)
    aModeDrop = Standard_False;

  TopTools_MapOfShape                theSmallEdges, theMultyEdges;
  TopTools_DataMapOfShapeListOfShape theEdgeToFaces, theFaceWithSmall;
  aSfwr->CheckSmallEdges(theSmallEdges, theEdgeToFaces, theFaceWithSmall, theMultyEdges);
  aSfwr->MergeSmallEdges(theSmallEdges,
                         theEdgeToFaces,
                         theFaceWithSmall,
                         theMultyEdges,
                         aModeDrop,
                         tolang);
  // aSfwr->FixSmallEdges();
  TopoShape resShape = aSfwr->Shape();
  DBRep1::Set(argv[1], resShape);
  return 0;
}

static Standard_Integer OCC301(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  Handle(VisualContext) context = ViewerTest::GetAISContext();
  if (context.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return 1;
  }
  if (argc != 3)
  {
    di << "Usage : " << argv[0] << " ArcRadius ArrowSize\n";
    return 1;
  }

  Standard_Real aRadius     = Draw1::Atof(argv[1]);
  Standard_Real anArrowSize = Draw1::Atof(argv[2]);

  Point3d p1 = Point3d(10., 10., 0.);
  Point3d p2 = Point3d(50., 10., 0.);
  Point3d p3 = Point3d(50., 50., 0.);

  TopoEdge E1 = EdgeMaker(p1, p2);
  TopoEdge E2 = EdgeMaker(p2, p3);

  context->Display(new VisualShape(E1), Standard_False);
  context->Display(new VisualShape(E2), Standard_True);

  Point3d             plnpt(0, 0, 0);
  Dir3d             plndir(0, 0, 1);
  Handle(GeomPlane) pln = new GeomPlane(plnpt, plndir);

  Handle(PrsDim_AngleDimension) anAngleDimension =
    new PrsDim_AngleDimension(p1.Mirrored(p2), p2, p3);

  Handle(Prs3d_DimensionAspect) anAspect = new Prs3d_DimensionAspect;
  anAspect->MakeArrows3d(Standard_True);
  anAspect->ArrowAspect()->SetLength(anArrowSize);
  anAspect->SetTextHorizontalPosition(Prs3d_DTHP_Right);
  anAspect->TextAspect()->SetColor(Quantity_NOC_YELLOW);
  anAngleDimension->SetDimensionAspect(anAspect);
  // Another position of dimension
  anAngleDimension->SetFlyout(aRadius);
  context->Display(anAngleDimension, 0);
  return 0;
}

static Standard_Integer OCC261(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " Doc\n";
    return 1;
  }

  Handle(AppDocument) Doc;
  if (DDocStd1::GetDocument(argv[1], Doc))
  {
    Doc->ClearRedos();
    return 0;
  }
  else
    return 1;
}

#include <OSD_File.hxx>

static Standard_Integer OCC710(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " path\n";
  }

  AsciiString1 in(argv[1]);
  SystemFile*               aFile    = new SystemFile(in);
  Standard_Boolean        anExists = aFile->Exists();
  if (anExists == Standard_True)
    di << "1\n";
  else
    di << "0\n";
  return 0;
}

#include <ShapeFix_Shell.hxx>
#include <AIS_InteractiveObject.hxx>

static Standard_Integer OCC904(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Usage : " << argv[0] << " result shape nonmanifoldmode(0/1)\n";
  }
  TopoShape S = DBRep1::Get(argv[2]);
  if (S.IsNull() || S.ShapeType() != TopAbs_SHELL)
  {
    di << " Shape is null\n";
    return 1;
  }
  Standard_Boolean       nonmanifmode = (Draw1::Atoi(argv[3]) != 0);
  Handle(ShapeFix_Shell) SFSh         = new ShapeFix_Shell;
  SFSh->FixFaceOrientation(TopoDS::Shell(S), Standard_True, nonmanifmode);
  DBRep1::Set(argv[1], SFSh->Shape());
  return 0;
}

void QABugs1::Commands_16(DrawInterpreter& theCommands)
{
  const char* group = "QABugs1";

  theCommands.Add("BUC60848", "BUC60848 shape", __FILE__, BUC60848, group);
  theCommands.Add("BUC60828", "BUC60828", __FILE__, BUC60828, group);
  theCommands.Add("BUC60814", "BUC60814", __FILE__, BUC60814, group);
  theCommands.Add("BUC60774", "BUC60774", __FILE__, BUC60774, group);
  theCommands.Add("BUC60972", "BUC60972 edge edge plane val text ", __FILE__, BUC60972, group);
  theCommands.Add("OCC218", "OCC218 name plane Xlabel Ylabel", __FILE__, OCC218bug, group);
  theCommands.Add("OCC295", "OCC295 edge_result edge1 edge2", __FILE__, OCC295, group);
  theCommands.Add("OCC49", "OCC49 name", __FILE__, OCC49, group);
  theCommands.Add("OCC132", "OCC132 DependentName", __FILE__, OCC132, group);
  theCommands.Add("OCC405",
                  "OCC405 edge_result edge1 edge2; merge two edges",
                  __FILE__,
                  OCC405,
                  group);
  theCommands.Add("OCC395", "OCC395 edge_result edge1 edge2", __FILE__, OCC395, group);
  theCommands.Add("OCC394",
                  "OCC394 edge_result edge [tol [mode [tolang]]]",
                  __FILE__,
                  OCC394,
                  group);
  theCommands.Add("OCC301", "OCC301 ArcRadius ArrowSize", __FILE__, OCC301, group);
  theCommands.Add("OCC261", "OCC261 Doc", __FILE__, OCC261, group);
  theCommands.Add("OCC710", "OCC710 path", __FILE__, OCC710, group);
  theCommands.Add("OCC904", "OCC904 result shape nonmanifoldmode(0/1)", __FILE__, OCC904, group);

  return;
}
