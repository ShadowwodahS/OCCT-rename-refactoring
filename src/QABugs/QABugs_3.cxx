// Created on: 2002-06-17
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
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS.hxx>
#include <DBRep.hxx>
#include <BRep_Tool.hxx>
#include <GeomInt_IntSS.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <ViewerTest.hxx>
#include <AIS_Shape.hxx>

#include <fstream>

static int BUC60623(DrawInterpreter& di, Standard_Integer argc, const char** a)
{
  if (argc != 4)
  {
    di << "Usage : " << a[0] << " result Shape1 Shape2\n";
    return -1;
  }

  TopLoc_Location      L1;
  TopLoc_Location      L2;
  TopoFace          F1   = TopoDS::Face(DBRep1::Get(a[2], TopAbs_FACE));
  TopoFace          F2   = TopoDS::Face(DBRep1::Get(a[3], TopAbs_FACE));
  Handle(GeomSurface) GSF1 = BRepInspector::Surface(F1, L1);
  Handle(GeomSurface) GSF2 = BRepInspector::Surface(F2, L2);
  GeomInt_IntSS        Inter;
  Inter.Perform(GSF1, GSF2, BRepInspector::Tolerance(F1));
  if (!Inter.IsDone())
  {
    di << "Intersection not done\n";
    return 1;
  }
  Standard_Integer nbsol = Inter.NbLines();
  if (!nbsol)
  {
    di << "The number of solutions is zero!" << "\n";
    return 0;
  }
  Handle(GeomCurve3d) Sol = Inter.Line(1);
  if (!Sol.IsNull())
  {
    DBRep1::Set(a[1], EdgeMaker(Sol));
    return 0;
  }
  else
    di << "The first solution is Null!" << "\n";

  di << "fini\n";
  return 0;
}

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <TCollection_ExtendedString.hxx>
#include <AIS_InteractiveContext.hxx>
#include <PrsDim_LengthDimension.hxx>

static Standard_Integer BUC60632(DrawInterpreter& di, Standard_Integer /*n*/, const char** a)
{

  Handle(VisualContext) myAIScontext = ViewerTest::GetAISContext();
  if (myAIScontext.IsNull())
  {
    di << "use 'vinit' command before " << a[0] << "\n";
    return -1;
  }
  myAIScontext->EraseAll(Standard_False);

  TopoVertex V1 = BRepBuilderAPI_MakeVertex(Point3d(0, 0, 0));
  TopoVertex V2 = BRepBuilderAPI_MakeVertex(Point3d(10, 10, 0));

  Handle(VisualShape) Ve1 = new VisualShape(V1);
  Handle(VisualShape) Ve2 = new VisualShape(V2);

  myAIScontext->Display(Ve1, Standard_False);
  myAIScontext->Display(Ve2, Standard_False);

  Handle(GeomPlane)             Plane1 = new GeomPlane(Point3d(0, 0, 0), Dir3d(0, 0, 1));
  UtfString     Ext1("Dim1");
  Handle(PrsDim_LengthDimension) Dim1 = new PrsDim_LengthDimension(V1, V2, Plane1->Pln());
  Dim1->SetCustomValue(Draw1::Atof(a[2]));

  Handle(Prs3d_DimensionAspect) anAspect = new Prs3d_DimensionAspect();
  anAspect->MakeArrows3d(Standard_False);
  anAspect->MakeText3d(Standard_True);
  anAspect->MakeTextShaded(Standard_True);
  anAspect->TextAspect()->SetHeight(2.5);
  anAspect->ArrowAspect()->SetLength(1.0);
  Dim1->SetDimensionAspect(anAspect);

  myAIScontext->SetDisplayMode(Dim1, Draw1::Atoi(a[1]), Standard_False);
  myAIScontext->Display(Dim1, Standard_True);
  return 0;
}

#include <BRepTools.hxx>

static Standard_Integer BUC60652(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : BUC60652 fase" << "\n";
    return 1;
  }
  TopoShape shape = DBRep1::Get(argv[1]);
  TopoFace  face  = TopoDS::Face(shape);
  TopoWire  ow    = BRepTools1::OuterWire(face);
  DBRep1::Set("w", ow);
  return 0;
}

#include <BRepPrimAPI_MakeBox.hxx>

#include <BRepAlgoAPI_Fuse.hxx>

#include <V3d_View.hxx>

#include <Bnd_BoundSortBox.hxx>
#include <BRepBndLib.hxx>
#include <TopExp_Explorer.hxx>

static Standard_Integer BUC60729(DrawInterpreter& /*di*/,
                                 Standard_Integer /*argc*/,
                                 const char** /*argv*/)
{
  Bnd_Box      aMainBox;
  TopoShape aShape = BoxMaker(1, 1, 1).Solid();

  BRepBndLib::Add(aShape, aMainBox);

  Standard_Integer siMaxNbrBox = 6;
  Bnd_BoundSortBox m_BoundSortBox;
  m_BoundSortBox.Initialize(aMainBox, siMaxNbrBox);
  ShapeExplorer  aExplorer(aShape, TopAbs_FACE);
  Standard_Integer i;

  //  Bnd_Box __emptyBox; // Box is void !
  //  Handle(Bnd_HArray1OfBox) __aSetOfBox = new Bnd_HArray1OfBox( 1, siMaxNbrBox, __emptyBox );

  for (i = 1, aExplorer.ReInit(); aExplorer.More(); aExplorer.Next(), i++)
  {
    const TopoShape& aFace = aExplorer.Current();
    Bnd_Box             aBox;
    BRepBndLib::Add(aFace, aBox);
    m_BoundSortBox.Add(aBox, i);
    //      __aSetOfBox->SetValue( i, aBox );
  }
  //  m_BoundSortBox.Initialize( aMainBox, siMaxNbrBox );

  return 0;
}

static Standard_Integer BUC60724(DrawInterpreter& di,
                                 Standard_Integer /*argc*/,
                                 const char** /*argv*/)
{
  AsciiString1 as1("");
  AsciiString1 as2('\0');
  if (as1.ToCString() == NULL || as1.Length() != 0 || as1.ToCString()[0] != '\0')
    di << "Error : the first string is not zero string : " << as1.ToCString() << "\n";

  if (as2.ToCString() == NULL || as2.Length() != 0 || as2.ToCString()[0] != '\0')
    di << "Error : the second string is not zero string : " << as2.ToCString() << "\n";

  return 0;
}

#include <UnitsAPI.hxx>

static Standard_Integer BUC60727(DrawInterpreter& di,
                                 Standard_Integer /*argc*/,
                                 const char** /*argv*/)
{
  di << "Program Test\n";
  UnitsAPI::SetLocalSystem(UnitsAPI_MDTV);                          // length is mm
  di << "AnyToLS (3,mm) = " << UnitsAPI::AnyToLS(3., "mm") << "\n"; // result was WRONG.

  return 0;
}

#include <gp_Circ.hxx>
#include <Geom_Circle.hxx>
#include <GeomAPI.hxx>
#include <Geom2d_CartesianPoint.hxx>
#include <Geom2dGcc_QualifiedCurve.hxx>
#include <Geom2dGcc_Circ2d2TanRad.hxx>
#include <Geom2d_Circle.hxx>
#include <ProjLib.hxx>

static Standard_Integer BUC60792(DrawInterpreter& di, Standard_Integer /*argc*/, const char** argv)
{
  Handle(VisualContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return -1;
  }

  Point3d               pt3d(0, 20, 150);
  Frame3d               anAx2(Point3d(0, 0, 0), Dir3d(1, 0, 0), Dir3d(0, 0, 1));
  gp_Circ              circ(anAx2, 50.0);
  Handle(GeomCircle)  gcir  = new GeomCircle(circ);
  Handle(GeomPlane)   pln   = new GeomPlane(gp_Ax3(Point3d(0, 0, 0), Dir3d(1, 0, 0)));
  Handle(GeomCurve2d) gcir1 = GeomAPI::To2d(gcir, pln->Pln());
  TopoShape         sh1   = EdgeMaker(gcir1, pln).Shape();
  Handle(VisualShape)    ais1  = new VisualShape(sh1);
  aContext->SetColor(ais1, Quantity_NOC_INDIANRED, Standard_False);
  aContext->Display(ais1, Standard_False);
  DBRep1::Set("sh0", sh1);
  gp_Pnt2d thepoint;
  //  local_get_2Dpointfrom3Dpoint(pt3d, pln->Pln(), thepoint);
  thepoint                               = ProjLib::Project(pln->Pln(), pt3d);
  Handle(Geom2d_CartesianPoint) ThePoint = new Geom2d_CartesianPoint(thepoint);
  Geom2dAdaptor_Curve           acur1(gcir1);
  Geom2dGcc_QualifiedCurve      qcur1(acur1, GccEnt_outside);
  Geom2dGcc_Circ2d2TanRad       cirtanrad(qcur1, ThePoint, 200.0, 0.0001);
  printf("\n No. of solutions = %d\n", cirtanrad.NbSolutions());
  Handle(Geom2d_Circle) gccc;
  if (cirtanrad.NbSolutions())
  {
    for (int i = 1; i <= cirtanrad.NbSolutions(); i++)
    {
      gp_Circ2d ccc         = cirtanrad.ThisSolution(i);
      gccc                  = new Geom2d_Circle(ccc);
      TopoShape       sh = EdgeMaker(gccc, pln).Shape();
      Standard_Character aStr[5];
      Sprintf(aStr, "sh%d", i);
      DBRep1::Set(aStr, sh);
      Handle(VisualShape) ais = new VisualShape(sh);
      if (i == 1)
        aContext->SetColor(ais, Quantity_NOC_GREEN, Standard_False);
      if (i == 2)
        aContext->SetColor(ais, Quantity_NOC_HOTPINK, Standard_False);
      aContext->Display(ais, Standard_False);
      Standard_Real ParSol1, ParSol2, ParArg1, ParArg2;
      gp_Pnt2d      PntSol1, PntSol2;
      cirtanrad.Tangency1(i, ParSol1, ParArg1, PntSol1);
      printf("%f\t%f\t\t%f\t%f\n", ParSol1, ParArg1, PntSol1.X(), PntSol1.Y());
      cirtanrad.Tangency2(i, ParSol2, ParArg2, PntSol2);
      printf("%f\t%f\t\t%f\t%f\n", ParSol2, ParArg2, PntSol2.X(), PntSol2.Y());
    }
  }
  aContext->UpdateCurrentViewer();
  return 0;
}

#include <TColgp_Array2OfPnt.hxx>
#include <Geom_BezierSurface.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Geom_OffsetSurface.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <GeomProjLib.hxx>
#include <Geom_TrimmedCurve.hxx>

static Standard_Integer BUC60811(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc == 4)
  {
    TopLoc_Location      L1;
    TopoEdge          aEdge = TopoDS::Edge(DBRep1::Get(argv[2], TopAbs_EDGE));
    TopoFace          aFace = TopoDS::Face(DBRep1::Get(argv[3], TopAbs_FACE));
    Standard_Real        f = 0.0, l = 0.0;
    Handle(GeomCurve3d)   GC            = BRepInspector::Curve(aEdge, f, l);
    Handle(GeomSurface) GS            = BRepInspector::Surface(aFace, L1);
    GC                                 = new Geom_TrimmedCurve(GC, f, l);
    Handle(GeomCurve3d)       projCurve = GeomProjLib::Project(GC, GS);
    BRepBuilderAPI_MakeWire* myWire;
    myWire = new BRepBuilderAPI_MakeWire();
    myWire->Add((EdgeMaker(projCurve)).Edge());
    DBRep1::Set(argv[1], myWire->Wire());
    return 0;
  }

  Handle(VisualContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return -1;
  }

  // step 1. creating a Bezier Surface and a patch
  TopoFace                FP;
  TopoShape               FP1;
  TopoSolid               solid;
  Handle(VisualShape)          ais1;
  Handle(VisualShape)          ais2;
  Handle(Geom_BezierSurface) BZ1;
  TColgp_Array2OfPnt         array1(1, 3, 1, 3);
  array1.SetValue(1, 1, Point3d(0, 100, 0));
  array1.SetValue(1, 2, Point3d(200, 100, 0));
  array1.SetValue(1, 3, Point3d(400, 100, 0));
  array1.SetValue(2, 1, Point3d(0, 200, 100));
  array1.SetValue(2, 2, Point3d(200, 200, 100));
  array1.SetValue(2, 3, Point3d(400, 200, 100));
  array1.SetValue(3, 1, Point3d(0, 300, 0));
  array1.SetValue(3, 2, Point3d(200, 300, 0));
  array1.SetValue(3, 3, Point3d(400, 300, 0));
  BZ1 = new Geom_BezierSurface(array1);
  FaceMaker bzf1(BZ1, Precision::Confusion());
  TopoFace             F1 = bzf1.Face();
  ais1                       = new VisualShape(F1);
  DBRep1::Set("F1", F1);
  aContext->SetMaterial(ais1, Graphic3d_NameOfMaterial_Aluminum, Standard_False);
  aContext->Display(ais1, Standard_False);
  ShapeBuilder B;
  TopoShell shell;
  B.MakeShell(shell);
  B.Add(shell, bzf1);
  shell.Closed(BRepInspector::IsClosed(shell));
  B.MakeSolid(solid);
  B.Add(solid, shell);
  Dir3d                   D(0, 0, 1.0f);
  BRepBuilderAPI_MakeWire  mkw;
  Point3d                   p1 = Point3d(150., 150.0, 260.);
  Point3d                   p2 = Point3d(350., 150., 260.);
  EdgeMaker* E1 = new EdgeMaker(p1, p2);
  mkw.Add(*E1);
  p1                          = Point3d(350., 150., 260.);
  p2                          = Point3d(350., 250., 260.);
  EdgeMaker* E2 = new EdgeMaker(p1, p2);
  mkw.Add(*E2);
  p1                          = Point3d(350., 250., 260.);
  p2                          = Point3d(300., 250.0, 260.);
  EdgeMaker* E3 = new EdgeMaker(p1, p2);
  mkw.Add(*E3);
  p1                          = Point3d(300., 250.0, 260.);
  p2                          = Point3d(200., 200.0, 260.);
  EdgeMaker* E4 = new EdgeMaker(p1, p2);
  mkw.Add(*E4);
  p1                          = Point3d(200., 200.0, 260.);
  p2                          = Point3d(150., 200.0, 260.);
  EdgeMaker* E5 = new EdgeMaker(p1, p2);
  mkw.Add(*E5);
  p1                          = Point3d(150., 200.0, 260.);
  p2                          = Point3d(150., 150.0, 260.);
  EdgeMaker* E6 = new EdgeMaker(p1, p2);
  mkw.Add(*E6);
  FP   = FaceMaker(mkw.Wire());
  ais2 = new VisualShape(FP);
  aContext->SetMaterial(ais2, Graphic3d_NameOfMaterial_Aluminum, Standard_False);
  aContext->Display(ais2, Standard_False);

  DBRep1::Set("FP", FP);

  // step 2. offsetting the surface.
  Handle(Geom_OffsetSurface) offsurf;
  offsurf = new Geom_OffsetSurface(BZ1, -100);
  FaceMaker bzf2(offsurf, Precision::Confusion());
  const TopoFace&      F2    = bzf2.Face();
  Handle(VisualShape)       ais22 = new VisualShape(F2);
  aContext->Display(ais22, Standard_False);
  DBRep1::Set("F2", F2);

  // step 3. filleting the patch.
  //( I want to project wire of this patch on offsetted surface above)
  BRepFilletAPI_MakeFillet2d fillet(FP);
  ShapeExplorer            Ex;
  Ex.Init(FP, TopAbs_VERTEX);
  TopoVertex v1 = TopoDS::Vertex(Ex.Current());
  fillet.AddFillet(v1, 20);
  di << "\nError is " << fillet.Status() << "\n";
  //  printf("\nError is %d ", fillet.Status());
  Ex.Next();
  TopoVertex V2 = TopoDS::Vertex(Ex.Current());
  fillet.AddFillet(V2, 20);
  di << "\nError is " << fillet.Status() << "\n";
  //  printf("\nError is %d ", fillet.Status());
  fillet.Build();
  FP1  = fillet.Shape();
  ais2 = new VisualShape(FP1);
  aContext->SetMaterial(ais2, Graphic3d_NameOfMaterial_Aluminum, Standard_False);
  aContext->Display(ais2, Standard_False);

  DBRep1::Set("FP1", FP1);

  // step 4. Projecting the wire of this patch on offsetted surface.
  //   ShapeExplorer Ex;
  BRepBuilderAPI_MakeWire* myWire;
  myWire = new BRepBuilderAPI_MakeWire();
  for (Ex.Init(FP1, TopAbs_EDGE); Ex.More(); Ex.Next())
  {
    TopoEdge        e1 = TopoDS::Edge(Ex.Current());
    Standard_Real      f = 0.0, l = 0.0;
    Handle(GeomCurve3d) newBSplin = BRepInspector::Curve(e1, f, l);
    newBSplin                    = new Geom_TrimmedCurve(newBSplin, f, l);
    Handle(GeomCurve3d) projCurve = GeomProjLib::Project(newBSplin, offsurf);
    myWire->Add((EdgeMaker(projCurve)).Edge());
  }
  Handle(VisualShape) ais33 = new VisualShape(myWire->Wire());
  aContext->Display(ais33, Standard_True);

  DBRep1::Set("Wire", myWire->Wire());

  return 0;
}

#include <GeomAPI_ExtremaCurveCurve.hxx>

static int BUC60825(DrawInterpreter& di, Standard_Integer argc, const char** argv)

{
  if (argc < 3)
  {
    printf("Usage: %s edge1 edge2", argv[0]);
    return (-1);
  }

  TopoEdge E1 = TopoDS::Edge(DBRep1::Get(argv[1])), E2 = TopoDS::Edge(DBRep1::Get(argv[2]));

  Standard_Real fp, lp;

  Handle(GeomCurve3d) C1 = BRepInspector::Curve(E1, fp, lp), C2 = BRepInspector::Curve(E2, fp, lp);

  GeomAPI_ExtremaCurveCurve aExt(C1, C2);

  di << "NB RESULTS : " << aExt.NbExtrema() << "\n";

  return 0;
}

#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

static int OCC10006(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 1)
  {
    di << "Usage : " << argv[0] << "\n";
    return 1;
  }

  double bottompoints1[12] = {10, -10, 0, 100, -10, 0, 100, -100, 0, 10, -100, 0};
  double toppoints1[12]    = {0, 0, 10, 100, 0, 10, 100, -100, 10, 0, -100, 10};
  double bottompoints2[12] = {0, 0, 10.00, 100, 0, 10.00, 100, -100, 10.00, 0, -100, 10.00};
  double toppoints2[12]    = {0, 0, 250, 100, 0, 250, 100, -100, 250, 0, -100, 250};
  BRepBuilderAPI_MakePolygon bottompolygon1, toppolygon1, bottompolygon2, toppolygon2;
  Point3d                     tmppnt;
  for (int i = 0; i < 4; i++)
  {
    tmppnt.SetCoord(bottompoints1[3 * i], bottompoints1[3 * i + 1], bottompoints1[3 * i + 2]);
    bottompolygon1.Add(tmppnt);
    tmppnt.SetCoord(toppoints1[3 * i], toppoints1[3 * i + 1], toppoints1[3 * i + 2]);
    toppolygon1.Add(tmppnt);
    tmppnt.SetCoord(bottompoints2[3 * i], bottompoints2[3 * i + 1], bottompoints2[3 * i + 2]);
    bottompolygon2.Add(tmppnt);
    tmppnt.SetCoord(toppoints2[3 * i], toppoints2[3 * i + 1], toppoints2[3 * i + 2]);
    toppolygon2.Add(tmppnt);
  }
  bottompolygon1.Close();
  DBRep1::Set("B1", bottompolygon1.Shape());
  toppolygon1.Close();
  DBRep1::Set("T1", toppolygon1.Shape());
  bottompolygon2.Close();
  DBRep1::Set("B2", bottompolygon2.Shape());
  toppolygon2.Close();
  DBRep1::Set("T2", toppolygon2.Shape());
  BRepOffsetAPI_ThruSections loft1(Standard_True, Standard_True);
  loft1.AddWire(bottompolygon1.Wire());
  loft1.AddWire(toppolygon1.Wire());
  loft1.Build();
  BRepOffsetAPI_ThruSections loft2(Standard_True, Standard_True);
  loft2.AddWire(bottompolygon2.Wire());
  loft2.AddWire(toppolygon2.Wire());
  loft2.Build();
  if (loft1.Shape().IsNull() || loft2.Shape().IsNull())
    return 1;
  DBRep1::Set("TS1", loft1.Shape());
  DBRep1::Set("TS2", loft2.Shape());

  di << "BooleanFuse result(loft1.Shape(), loft2.Shape())\n";
  BooleanFuse result(loft1.Shape(), loft2.Shape());
  DBRep1::Set("F", result.Shape());

  return 0;
}

#include <GC_MakeTrimmedCone.hxx>

static Standard_Integer BUC60856(DrawInterpreter& di, Standard_Integer /*argc*/, const char** argv)
{
  Handle(VisualContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return -1;
  }

  Frame3d                                 Cone_Ax;
  double                                 R1 = 8, R2 = 16;
  Point3d                                 P0(0, 0, 0), P1(0, 0, 20), P2(0, 0, 45);
  Handle(Geom_RectangularTrimmedSurface) S = GC_MakeTrimmedCone(P1, P2, R1, R2).Value();
  TopoShape      myshape = FaceMaker(S, Precision::Confusion()).Shape();
  Handle(VisualShape) ais1    = new VisualShape(myshape);
  aContext->Display(ais1, Standard_False);
  aContext->SetColor(ais1, Quantity_NOC_BLUE1, Standard_False);

  Handle(Geom_RectangularTrimmedSurface) S2 = GC_MakeTrimmedCone(P1, P2, R1, 0).Value();
  TopoShape      myshape2 = FaceMaker(S2, Precision::Confusion()).Shape();
  Handle(VisualShape) ais2     = new VisualShape(myshape2);
  aContext->Display(ais2, Standard_False);
  aContext->SetColor(ais2, Quantity_NOC_RED, Standard_False);
  return 0;
}

//==========================================================================
// function : CoordLoad
//           chargement d une face dans l explorer.
//==========================================================================
static Standard_Integer coordload(DrawInterpreter& theDi,
                                  Standard_Integer  theArgsNb,
                                  const char**      theArgVec)
{
  if (theArgsNb < 3)
  {
    return 1;
  }

  std::ifstream aFile(theArgVec[2], std::ios::in);
  if (!aFile)
  {
    theDi << "unable to open " << theArgVec[2] << " for input\n";
    return 2;
  }

  char aLine[80];
  memset(aLine, 0, 40);
  aFile.getline(aLine, 80);

  Point3d aPnt(0.0, 0.0, 0.0);
  aLine[40] = '\0';
  aPnt.SetY(Draw1::Atof(&aLine[20]));
  aLine[20] = '\0';
  aPnt.SetX(Draw1::Atof(aLine));
  TopoVertex           aVert1 = BRepBuilderAPI_MakeVertex(aPnt);
  BRepBuilderAPI_MakeWire aMakeWire;
  for (;;)
  {
    memset(aLine, 0, 40);
    aFile.getline(aLine, 80);
    if (!aFile)
    {
      break;
    }

    aLine[40] = '\0';
    aPnt.SetY(Draw1::Atof(&aLine[20]));
    aLine[20] = '\0';
    aPnt.SetX(Draw1::Atof(aLine));
    TopoVertex aVert2 = BRepBuilderAPI_MakeVertex(aPnt);
    aMakeWire.Add(EdgeMaker(aVert1, aVert2));
    aVert1 = aVert2;
  }
  aFile.close();

  if (!aMakeWire.IsDone())
  {
    DBRep1::Set(theArgVec[1], TopoFace());
    return 0;
  }

  FaceMaker aMakeFace(aMakeWire.Wire());
  DBRep1::Set(theArgVec[1], aMakeFace.IsDone() ? aMakeFace.Face() : TopoFace());
  return 0;
}

static Standard_Integer TestMem(DrawInterpreter& /*di*/,
                                Standard_Integer /*nb*/,
                                const char** /*arg*/)
{
  UtfString aString(1024 * 1024, 'A');
  return 0;
}

static Standard_Integer BUC60876_(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  Handle(VisualContext) aContext = ViewerTest::GetAISContext();
  if (aContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return -1;
  }
  if ((argc != 2) && (argc != 3))
  {
    di << "usage : " << argv[0] << " shape [mode==1]\n";
    return -1;
  }
  TopoShape                  aShape = DBRep1::Get(argv[1]);
  Handle(VisualEntity) anIO   = new VisualShape(aShape);
  anIO->SetHilightMode((argc == 3) ? Draw1::Atoi(argv[2]) : 1);
  aContext->Display(anIO, Standard_True);
  return 0;
}

//=================================================================================================

#include <TCollection_HAsciiString.hxx>

static Standard_Integer BUC60773(DrawInterpreter& /*di*/,
                                 Standard_Integer /*n*/,
                                 const char** /*a*/)
{
  Handle(TCollection_HAsciiString) hAscii = new TCollection_HAsciiString();
  Standard_CString                 aStr   = hAscii->ToCString();
  AsciiString1          aAscii(aStr);

  return 0;
}

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>

static int TestCMD(DrawInterpreter& di, Standard_Integer argc, const char** argv)

{
  if (argc != 1)
  {
    di << "Usage : " << argv[0] << "\n";
    return 1;
  }

  // Cylindre 36.085182 20.0 8.431413 88.04671 20.0 38.931416 10.0

  Standard_Real x11    = 36.085182;
  Standard_Real y11    = 20.0;
  Standard_Real z11    = 8.431413;
  Standard_Real x12    = 88.04671;
  Standard_Real y12    = 20.0;
  Standard_Real z12    = 38.931416;
  Standard_Real radius = 10.0;

  Point3d        base1(x11, y11, z11);
  Dir3d        vect1(x12 - x11, y12 - y11, z12 - z11);
  Frame3d        axis1(base1, vect1);
  Standard_Real height1 =
    sqrt(((x12 - x11) * (x12 - x11)) + ((y12 - y11) * (y12 - y11)) + ((z12 - z11) * (z12 - z11)));
  CylinderMaker cylinder(axis1, radius, height1);

  TopoShape SCyl = cylinder.Shape();
  DBRep1::Set("cyl", SCyl);

  // Cone 70.7262 20.0 28.431412 105.36722 20.0 48.431416 6.0 3.0
  Standard_Real x21     = 70.7262;
  Standard_Real y21     = 20.0;
  Standard_Real z21     = 28.431412;
  Standard_Real x22     = 105.36722;
  Standard_Real y22     = 20.0;
  Standard_Real z22     = 48.431416;
  Standard_Real radius1 = 6.0;
  Standard_Real radius2 = 3.0;

  Point3d        base2(x21, y21, z21);
  Dir3d        vect2(x22 - x21, y22 - y21, z22 - z21);
  Frame3d        axis2(base2, vect2);
  Standard_Real height2 =
    sqrt(((x22 - x21) * (x22 - x21)) + ((y22 - y21) * (y22 - y21)) + ((z22 - z21) * (z22 - z21)));
  BRepPrimAPI_MakeCone cone(axis2, radius1, radius2, height2);

  TopoShape SCon = cone.Shape();
  DBRep1::Set("con", SCon);

  di << "BooleanFuse SFuse(SCyl, SCon)\n";
  BooleanFuse SFuse(SCyl, SCon);
  if (!SFuse.IsDone())
  {
    di << "Error: Boolean fuse operation failed !\n";
  }
  else
  {
    const TopoShape& fuse = SFuse.Shape();
    DBRep1::Set("fus", fuse);
  }
  return 0;
}

#include <NCollection_DataMap.hxx>
#include <TColStd_HSequenceOfAsciiString.hxx>
#include <TopExp.hxx>
#include <TopoDS_Iterator.hxx>

//---------------------------------------------------------------------------------------

static Standard_Integer statface(DrawInterpreter& di, Standard_Integer /*argc*/, const char** argv)

{
  TopoShape aShape = DBRep1::Get(argv[1]);
  if (aShape.IsNull())
  {
    di << "Invalid input shape\n";
    return 1;
  }
  NCollection_DataMap<AsciiString1, Standard_Integer> aMap;
  Handle(TColStd_HSequenceOfAsciiString) aSequence = new TColStd_HSequenceOfAsciiString;
  Standard_CString                       aString;
  Standard_Integer                       l = 0;
  ShapeExplorer                        expl;
  Standard_Real                          f3d, l3d;
  for (expl.Init(aShape, TopAbs_FACE); expl.More(); expl.Next())
  {
    // SURFACES
    TopoFace          aFace    = TopoDS::Face(expl.Current());
    Handle(GeomSurface) aSurface = BRepInspector::Surface(aFace);
    aString                       = aSurface->DynamicType()->Name();

    if (aMap.IsBound(aString))
      aMap.ChangeFind(aString)++;
    else
    {
      aMap.Bind(aString, 1);
      aSequence->Append(aString);
    }
  }
  // PCURVES
  for (expl.Init(aShape, TopAbs_FACE); expl.More(); expl.Next())
  {
    TopoFace     aFace = TopoDS::Face(expl.Current());
    TopoDS_Iterator anIt(aFace);
    TopoWire     aWire = TopoDS::Wire(anIt.Value());
    TopoDS_Iterator it(aWire);
    for (; it.More(); it.Next())
    {
      TopoEdge          Edge     = TopoDS::Edge(it.Value());
      Handle(GeomCurve2d) aCurve2d = BRepInspector::CurveOnSurface(Edge, aFace, f3d, l3d);
      aString                       = aCurve2d->DynamicType()->Name();
      if (aMap.IsBound(aString))
        aMap.ChangeFind(aString)++;
      else
      {
        aMap.Bind(aString, 1);
        aSequence->Append(aString);
      }
    }
  }
  // 3d CURVES
  ShapeExplorer exp;
  for (exp.Init(aShape, TopAbs_EDGE); exp.More(); exp.Next())
  {
    TopoEdge        Edge     = TopoDS::Edge(exp.Current());
    Handle(GeomCurve3d) aCurve3d = BRepInspector::Curve(Edge, f3d, l3d);
    if (aCurve3d.IsNull())
    {
      l++;
    }
    else
    {
      aString = aCurve3d->DynamicType()->Name();
      if (aMap.IsBound(aString))
      {
        aMap.ChangeFind(aString)++;
      }
      else
      {
        aMap.Bind(aString, 1);
        aSequence->Append(aString);
      }
    }
  }
  // Output
  di << "\n";

  for (Standard_Integer i = 1; i <= aSequence->Length(); i++)
  {
    di << aMap.Find(aSequence->Value(i)) << "   --   " << aSequence->Value(i).ToCString() << "\n";
  }

  di << "\n";
  di << "Degenerated edges :\n";
  di << l << "   --    Degenerated edges \n";

  return 0;
}

#include <BRepBuilderAPI_Transform.hxx>

static Standard_Integer BUC60841(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 1)
  {
    di << "Usage : " << argv[0] << "\n";
    return 1;
  }

  Frame3d                   Ax2 = Frame3d(Point3d(0, 621, 78), Dir3d(0, 1, 0));
  CylinderMaker cyl(Ax2, 260, 150);
  // CylinderMaker cyl(Frame3d(Point3d(0, 621, 78), Dir3d(0, 1,0)), 260, 150);

  TopoShape sh1 = cyl.Shape();
  DBRep1::Set("sh1", sh1);
  Transform3d trsf1, trsf2;
  trsf1.SetTranslation(Point3d(0.000000, 700.000000, -170.000000),
                       Point3d(0.000000, 700.000000, -95.000000));
  trsf2.SetRotation(
    Axis3d(Point3d(0.000000, 700.000000, -170.000000), Dir3d(0.000000, 0.000000, 1.000000)),
    0.436111);
  BRepBuilderAPI_Transform trans1(sh1, trsf1);
  TopoShape             sh2 = trans1.Shape();
  DBRep1::Set("sh2", sh2);

  di << "BooleanFuse fuse1(sh1, sh2)\n";
  BooleanFuse fuse1(sh1, sh2);
  TopoShape     fsh1 = fuse1.Shape();
  DBRep1::Set("fsh1", fsh1);

  BRepBuilderAPI_Transform trans2(fsh1, trsf2);
  TopoShape             sh3 = trans2.Shape();
  DBRep1::Set("sh3", sh3);

  di << "BooleanFuse fuse2(fsh1,sh3)\n";
  BooleanFuse    fuse2(fsh1, sh3);
  const TopoShape& fsh2 = fuse2.Shape();
  DBRep1::Set("fsh2", fsh2);

  Handle(VisualShape) aisp1 = new VisualShape(fsh2);
  return 0;
}

#include <ShapeBuild_Edge.hxx>

static Standard_Integer BUC60874(DrawInterpreter& /*di*/,
                                 Standard_Integer /*argc*/,
                                 const char** argv)
{
  TopoEdge e = TopoDS::Edge(DBRep1::Get(argv[1], TopAbs_EDGE));
  ShapeBuild_Edge().BuildCurve3d(e);
  DBRep1::Set("ED", e);
  return 0;
}

#include <TDF_Label.hxx>
#include <TDataStd_TreeNode.hxx>

#include <DDocStd.hxx>

#include <DDF.hxx>

#include <TDocStd_Modified.hxx>
#include <TDocStd_Application.hxx>
#include <TDF_Delta.hxx>
#include <TDataXtd_Constraint.hxx>
#include <TPrsStd_AISPresentation.hxx>
#include <TPrsStd_AISViewer.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_Naming.hxx>
#include <TNaming_NamedShape.hxx>

static int BUC60817(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " D\n";
    di << 1;
    return 0;
  }

  Handle(TDF_Data) DF;
  if (!DDF1::GetDF(argv[1], DF))
  {
    di << 2;
    return 0;
  }

  DataLabel                 L1, L2;
  Handle(TDataStd_TreeNode) TN1, TN2;

  DDF1::AddLabel(DF, "0:2", L1);
  TN1 = TDataStd_TreeNode::Set(L1);

  DDF1::AddLabel(DF, "0:3", L2);
  TN2 = TDataStd_TreeNode::Set(L2);

  TN1->Append(TN2);
  if (!(TN2->IsDescendant(TN1)))
  {
    di << 3;
    return 0;
  }
  if ((TN1->IsDescendant(TN2)))
  {
    di << 4;
    return 0;
  }

  di << 0;
  return 0;
}

static int BUC60831_1(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " D\n";
    di << -1;
    return 0;
  }

  Handle(TDF_Data) DF;
  if (!DDF1::GetDF(argv[1], DF))
  {
    di << -2;
    return 0;
  }

  DataLabel L;
  DDF1::FindLabel(DF, "0:1", L, Standard_False);
  Handle(TDocStd_Modified) MDF;
  if (!L.Root().FindAttribute(TDocStd_Modified::GetID(), MDF))
  {
    MDF = new TDocStd_Modified();
    L.Root().AddAttribute(MDF);
  }

  di << !MDF->IsEmpty();
  return 0;
}

static int BUC60831_2(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 3)
  {
    di << "Usage : " << argv[0] << " D Label\n";
    di << 1;
    return 0;
  }

  Handle(TDF_Data) DF;
  if (!DDF1::GetDF(argv[1], DF))
  {
    di << 2;
    return 0;
  }

  DataLabel L;
  DDF1::FindLabel(DF, argv[2], L, Standard_False);

  TDocStd_Modified::Add(L);

  di << 0;
  return 0;
}

static int BUC60836(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " D\n";
    di << 1;
    return 0;
  }

  Handle(TDF_Data) aDF;
  if (!DDF1::GetDF(argv[1], aDF))
  {
    di << 2;
    return 0;
  }

  Handle(AppDocument) aDocument;
  if (!DDocStd1::GetDocument(argv[1], aDocument))
  {
    di << 3;
    return 0;
  }

  DataLabel                 L;
  Handle(TDataStd_TreeNode) TN;

  aDocument->NewCommand();
  DDF1::AddLabel(aDF, "0:2", L);
  TN = TDataStd_TreeNode::Set(L);

  aDocument->NewCommand();
  DDF1::AddLabel(aDF, "0:3", L);
  TN = TDataStd_TreeNode::Set(L);

  aDocument->NewCommand();
  DDF1::AddLabel(aDF, "0:4", L);
  TN = TDataStd_TreeNode::Set(L);
  aDocument->NewCommand();

  TDF_DeltaList Us, Rs;
  Us = aDocument->GetUndos();
  Rs = aDocument->GetUndos();

  Standard_Integer i;
  char             Names[10][5] = {"n1", "n2", "n3", "n4", "n5", "n6", "n7", "n8", "n9", "n10"};

  TDF_ListIteratorOfDeltaList IDL;
  for (IDL.Initialize(Us), i = 1; IDL.More(); IDL.Next(), i++)
  {
    Handle(TDF_Delta)          D = IDL.Value();
    UtfString S(Names[i - 1]);
    D->SetName(S);
    //    std::cout<<" U"<<i<<"="<<D->Name()<<std::endl;
  }

  aDocument->Undo();
  aDocument->Undo();

  Us = aDocument->GetUndos();
  Rs = aDocument->GetRedos();

  for (IDL.Initialize(Us), i = 1; IDL.More(); IDL.Next(), i++)
  {
    Handle(TDF_Delta) D = IDL.Value();
    //    std::cout<<" U"<<i<<"="<<D->Name()<<std::endl;
  }

  UtfString n2name("n2");
  for (IDL.Initialize(Rs), i = 1; IDL.More(); IDL.Next(), i++)
  {
    Handle(TDF_Delta) D = IDL.Value();
    if (i == 1 && !D->Name().IsEqual(n2name))
    {
      di << 4;
      return 0;
    }
  }

  di << 0;
  return 0;
}

static int BUC60847(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 3)
  {
    di << "Usage : " << argv[0] << " D Shape\n";
    di << 1;
    return 0;
  }

  Handle(TDF_Data) aDF;
  if (!DDF1::GetDF(argv[1], aDF))
  {
    di << 2;
    return 0;
  }

  TopoShape s = DBRep1::Get(argv[2]);
  if (s.IsNull())
  {
    di << "shape not found\n";
    di << 3;
    return 0;
  }
  DataLabel L;
  DDF1::AddLabel(aDF, "0:2", L);
  TNaming_Builder SI(L);
  SI.Generated(s);

  Handle(ShapeAttribute) NS = new ShapeAttribute;

  TNaming_Naming aNN;
  NS = aNN.Name(L, s, s);
  //  if (!NS->IsEmpty()) {di<<3;return 0;}
  if (NS->IsEmpty())
  {
    di << 4;
    return 0;
  }
  di << 0;
  return 0;
}

static int BUC60862(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 3)
  {
    di << "Usage : " << argv[0] << " D Shape\n";
    di << 1;
    return 0;
  }

  Handle(TDF_Data) aDF;
  if (!DDF1::GetDF(argv[1], aDF))
  {
    di << 2;
    return 0;
  }

  TopoShape s = DBRep1::Get(argv[2]);
  if (s.IsNull())
  {
    di << "shape not found\n";
    di << 3;
    return 0;
  }
  DataLabel L;
  DDF1::AddLabel(aDF, "0:2", L);
  TNaming_Builder SI(L);
  SI.Generated(s);

  Handle(ShapeAttribute) NS = new ShapeAttribute;

  TNaming_Naming aNN;
  NS = aNN.Name(L, s, s);
  if (NS->IsEmpty())
  {
    di << 4;
    return 0;
  }
  di << 0;
  return 0;
}

static int BUC60867(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc == 2)
  {
    UtfString  path(argv[1]);
    Handle(AppManager) A = DDocStd1::GetApplication();
    Handle(AppDocument)    D;
    Standard_Integer            insession = A->IsInSession(path);
    if (insession > 0)
    {
      di << "document " << insession << "  is already in session\n";
      di << 2;
      return 0;
    }
    PCDM_ReaderStatus Result = A->Open(path, D);
    if (Result == PCDM_RS_OK)
    {
      di << 0;
      return 0;
    }
  }
  di << 3;
  return 0;
}

static int BUC60910(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " D\n";
    di << 1;
    return 0;
  }

  Handle(TDF_Data) aDF;
  if (!DDF1::GetDF(argv[1], aDF))
  {
    di << 2;
    return 0;
  }

  DataLabel L;
  DDF1::AddLabel(aDF, "0:2", L);

  Handle(TPrsStd_AISPresentation) AISP =
    TPrsStd_AISPresentation::Set(L, TDataXtd_Constraint::GetID());

  if (AISP->HasOwnMode())
  {
    di << 3;
    return 0;
  }
  AISP->SetMode(3);
  Standard_Integer Mode = AISP->Mode();
  if (Mode != 3)
  {
    di << 4;
    return 0;
  }
  if (!AISP->HasOwnMode())
  {
    di << 5;
    return 0;
  }
  AISP->UnsetMode();
  if (AISP->HasOwnMode())
  {
    di << 6;
    return 0;
  }
  di << 0;
  return 0;
}

static int BUC60925(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " D\n";
    di << 1;
    return 0;
  }

  Handle(TDF_Data) aDF;
  if (!DDF1::GetDF(argv[1], aDF))
  {
    di << 2;
    return 0;
  }

  DataLabel L;
  DDF1::AddLabel(aDF, "0:2", L);
  TDF_LabelMap LM;
  LM.Add(L);

  Handle(ShapeAttribute) NS = new ShapeAttribute;
  //  Handle(TNaming_Name) NN = new TNaming_Name;
  TNaming_Name NN;

  NN.Type(TNaming_IDENTITY);
  NN.Append(NS);
  Standard_Boolean Res = NN.Solve(L, LM);

  if (Res != Standard_False)
  {
    di << 3;
    return 0;
  }
  di << 0;
  return 0;
}

static int BUC60932(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " D\n";
    di << 1;
    return 0;
  }

  Handle(AppDocument) aDocument;
  if (!DDocStd1::GetDocument(argv[1], aDocument))
  {
    di << 2;
    return 0;
  }

  if (!aDocument->InitDeltaCompaction())
  {
    di << 3;
    return 0;
  }
  if (!aDocument->PerformDeltaCompaction())
  {
    di << 4;
    return 0;
  }

  di << 0;
  return 0;
}

//=======================================================================
// function : AISWidth
// purpose  : AISWidth (DOC,entry,[width])
// abv: testing command for checking bug BUC60917 in TPrsStd_AISPresentation
//=======================================================================

static int AISWidth(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{

  if (argc >= 3)
  {
    Handle(AppDocument) D;
    if (!DDocStd1::GetDocument(argv[1], D))
    {
      di << (-1);
      return 0;
    }
    DataLabel L;
    if (!DDF1::FindLabel(D->GetData(), argv[2], L))
    {
      di << (-2);
      return 0;
    }

    Handle(TPrsStd_AISViewer) viewer;
    if (!TPrsStd_AISViewer::Find(L, viewer))
    {
      di << (-3);
      return 0;
    }

    Handle(TPrsStd_AISPresentation) prs;
    if (L.FindAttribute(TPrsStd_AISPresentation::GetID(), prs))
    {
      if (argc == 4)
      {
        prs->SetWidth(Draw1::Atof(argv[3]));
        TPrsStd_AISViewer::Update(L);
      }
      else
      {
        if (prs->HasOwnWidth())
        {
          //         std::cout << "Width = " << prs->Width() << std::endl;
          di << prs->Width();
        }
        else
        {
          di << "AISWidth: Warning : Width wasn't set\n";
          di << (-4);
        }
      }
      return 0;
    }
  }
  di << "AISWidth : Error" << "\n";
  di << (-5);
  return 0;
}

//=======================================================================
// function : BUC60921 ( & BUC60954 )
// purpose  : Test memory allocation of OCAF in Undo/Redo operations
//=======================================================================

static Standard_Integer BUC60921(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  if (nb >= 4)
  {
    Handle(AppDocument) D;
    if (!DDocStd1::GetDocument(arg[1], D))
    {
      di << 1;
      return 0;
    }
    DataLabel L;
    DDF1::AddLabel(D->GetData(), arg[2], L);

    ShapeBuilder B;
    TopoShape S;
    BRepTools1::Read(S, arg[3], B);

    TNaming_Builder tnBuild(L);
    tnBuild.Generated(S);
    //    di << "File " << arg[3] << " added";
    di << 0;
    return 0;
  }
  di << "BUC60921 Doc label brep_file: directly read brep file and put shape to the label" << "\n";
  di << 2;
  return 0;
}

#include <IGESControl_Reader.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>

static Standard_Integer BUC60951_(DrawInterpreter& di, Standard_Integer argc, const char** a)
{
  if (argc != 2)
  {
    di << "Usage : " << a[0] << " file.igs\n";
    return 1;
  }

  Handle(VisualContext) myContext = ViewerTest::GetAISContext();

  if (myContext.IsNull())
  {
    di << "use 'vinit' command before " << a[0] << "\n";
    return -1;
  }

  //  IGESControlStd_Reader reader;
  IgesFileReader reader;
  reader.ReadFile(a[1]);
  reader.TransferRoots();
  TopoShape shape = reader.OneShape();
  printf("\n iges1 shape type = %d", shape.ShapeType());
  TopTools_IndexedMapOfShape list;
  TopExp1::MapShapes(shape, TopAbs_FACE, list);
  printf("\n No. of faces = %d", list.Extent());

  TopoShell shell;
  ShapeBuilder builder;
  builder.MakeShell(shell);
  for (int i = 1; i <= list.Extent(); i++)
  {
    TopoFace face = TopoDS::Face(list.FindKey(i));
    builder.Add(shell, face);
  }
  shell.Closed(BRepInspector::IsClosed(shell));

  BRepPrimAPI_MakeHalfSpace half(shell, Point3d(0, 0, 20));
  const TopoSolid&       sol = half.Solid();
  Frame3d                    anAx2(Point3d(-800.0, 0.0, 0), Dir3d(0, 0, -1));
  CylinderMaker  cyl(anAx2, 50, 300);
  TopoShape              sh = cyl.Shape();

  di << "BooleanFuse fuse(sol, sh)\n";
  BooleanFuse fuse(sol, sh);
  sh = fuse.Shape();

  Handle(VisualShape) res = new VisualShape(sh);
  myContext->Display(res, Standard_True);
  return 0;
}

void QABugs::Commands_3(DrawInterpreter& theCommands)
{
  const char* group = "QABugs";

  theCommands.Add("BUC60623", "BUC60623 result Shape1 Shape2", __FILE__, BUC60623, group);
  theCommands.Add("BUC60632", "BUC60632 mode length", __FILE__, BUC60632, group);
  theCommands.Add("BUC60652", "BUC60652 face", __FILE__, BUC60652, group);

  theCommands.Add("BUC60729", "BUC60729", __FILE__, BUC60729, group);
  theCommands.Add("BUC60724", "BUC60724", __FILE__, BUC60724, group);
  theCommands.Add("BUC60727", "BUC60727", __FILE__, BUC60727, group);
  theCommands.Add("BUC60792", "BUC60792", __FILE__, BUC60792, group);
  theCommands.Add("BUC60811", "BUC60811", __FILE__, BUC60811, group);

  theCommands.Add("BUC60825", "BUC60825", __FILE__, BUC60825, group);

  theCommands.Add("OCC10006", "OCC10006", __FILE__, OCC10006, group);

  theCommands.Add("BUC60856", "BUC60856", __FILE__, BUC60856, group);

  theCommands.Add("coordload", "load coord from file", __FILE__, coordload, group);

  theCommands.Add("TestMem", "TestMem", __FILE__, TestMem, group);
  theCommands.Add("BUC60945", "BUC60945", __FILE__, TestMem, group);
  theCommands.Add("BUC60876", "BUC60876 shape", __FILE__, BUC60876_, group);
  theCommands.Add("BUC60773", "BUC60773", __FILE__, BUC60773, group);

  theCommands.Add("TestCMD", "TestCMD", __FILE__, TestCMD, group);

  theCommands.Add("statface", "statface face", __FILE__, statface, group);

  theCommands.Add("BUC60841", "BUC60841", __FILE__, BUC60841, group);

  theCommands.Add("BUC60874", "BUC60874", __FILE__, BUC60874, group);

  theCommands.Add("BUC60817", "BUC60817 D", __FILE__, BUC60817, group);
  theCommands.Add("BUC60831_1", "BUC60831_1 D", __FILE__, BUC60831_1, group);
  theCommands.Add("BUC60831_2", "BUC60831_2 D Label", __FILE__, BUC60831_2, group);
  theCommands.Add("BUC60836", "BUC60836 D", __FILE__, BUC60836, group);
  theCommands.Add("BUC60847", "BUC60847 D Shape", __FILE__, BUC60847, group);
  theCommands.Add("BUC60862", "BUC60862 D Shape", __FILE__, BUC60862, group);
  theCommands.Add("BUC60867", "BUC60867", __FILE__, BUC60867, group);
  theCommands.Add("BUC60910", "BUC60910 D", __FILE__, BUC60910, group);
  theCommands.Add("BUC60925", "BUC60925 D", __FILE__, BUC60925, group);
  theCommands.Add("BUC60932", "BUC60932 D", __FILE__, BUC60932, group);
  theCommands.Add("AISWidth", "AISWidth (DOC,entry,[width])", __FILE__, AISWidth, group);
  theCommands.Add("BUC60921", "BUC60921 Doc label brep_file", __FILE__, BUC60921, group);

  theCommands.Add("BUC60951", "BUC60951 file.igs", __FILE__, BUC60951_, group);
}
