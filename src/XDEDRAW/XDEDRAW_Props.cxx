// Created on: 2000-08-04
// Created by: Pavel TELKOV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

// OCC532 sln 24.07.2002. Add epsilon parameter to SetProps function

#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <Draw.hxx>
#include <gp_Pln.hxx>
#include <GProp_GProps.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <Standard_Stream.hxx>
#include <TColgp_Array1OfXYZ.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Document.hxx>
#include <TNaming_NamedShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_Volume.hxx>
#include <XDEDRAW_Props.hxx>

#include <stdio.h>

// --------------------- VolumeFix Begin ---
//=================================================================================================

static double TetraVol(Point3d RefPoint, Point3d Som1, Point3d Som2, Point3d Som3)
{
  double curVolume = 0;
  Dir3d Line12;
  gp_Pln Plane123;
  Vector3d N;

  {
    try
    {
      OCC_CATCH_SIGNALS
      Line12 = Dir3d(Vector3d(Som1, Som2));
      Vector3d v1(Som1, Som2);
      Vector3d v2(Som2, Som3);
      N        = v1 ^ v2;
      Plane123 = gp_Pln(Som1, Dir3d(N));
    }
    catch (ExceptionBase const&)
    {
      return (0.);
    }
  }
  double L1, L2, L3;
  L1 = Som1.Distance(Som2);
  L2 = gp_Lin(Som1, Line12).Distance(Som3);
  L3 = Plane123.Distance(RefPoint);

  curVolume = ((L1 * L2) / 2) * (L3 / 3);

  Vector3d Rad(RefPoint, Som1);

  if ((Rad * N) > 0)
    return (curVolume);
  else
    return (-curVolume);
}

//=================================================================================================

static gp_XYZ TetraCen(const Point3d& RefPoint,
                       const Point3d& Som1,
                       const Point3d& Som2,
                       const Point3d& Som3)
{
  gp_XYZ curCentr, plnPnt;
  plnPnt   = (Som1.XYZ() + Som2.XYZ() + Som3.XYZ()) / 3;
  curCentr = plnPnt + (RefPoint.XYZ() - plnPnt) / 4;
  return curCentr;
}

//=================================================================================================

static Standard_Real CalculVolume(const TopoShape& So,
                                  Point3d&             aRefPoint,
                                  Standard_Real       tol,
                                  Standard_Boolean    withForce,
                                  DrawInterpreter&   di)
{
  Standard_Real    myVolume = 0, curVolume = 0;
  gp_XYZ           localCentroid(0, 0, 0), curCentroid(0, 0, 0);
  Standard_Boolean haveVertex = Standard_False;
  for (ShapeExplorer ex(So, TopAbs_FACE); ex.More(); ex.Next())
  {
    TopoFace     F = TopoDS::Face(ex.Current());
    TopLoc_Location L;
    if (!haveVertex)
      for (ShapeExplorer Vex(F, TopAbs_VERTEX); Vex.More(); Vex.Next())
      {
        TopoVertex v = TopoDS::Vertex(Vex.Current());
        if (!v.IsNull())
        {
          aRefPoint  = BRepInspector::Pnt(v);
          haveVertex = Standard_True;
          break;
        }
      }

    Handle(MeshTriangulation) facing = BRepInspector::Triangulation(F, L);
    if (facing.IsNull() || withForce)
    {
      MeshGenerator MESH(F, tol);

      facing = BRepInspector::Triangulation(F, L);
    }

    for (Standard_Integer i = 1; i <= (facing->NbTriangles()); i++)
    {

      Poly_Triangle    trian = facing->Triangle1(i);
      Standard_Integer index1, index2, index3; // M,N;
      if (F.Orientation() == TopAbs_REVERSED)
        trian.Get(index1, index3, index2);
      else
        trian.Get(index1, index2, index3);
      curVolume =
        TetraVol(aRefPoint, facing->Node(index1), facing->Node(index2), facing->Node(index3));
      myVolume += curVolume;
      curCentroid =
        TetraCen(aRefPoint, facing->Node(index1), facing->Node(index2), facing->Node(index3));

      localCentroid = localCentroid + curCentroid * curVolume;
    }
  }

  localCentroid = localCentroid * (1. / myVolume);

  di << "Centroid:\n";
  di << "X=\t" << localCentroid.X() << "\n";
  di << "Y=\t" << localCentroid.Y() << "\n";
  di << "Z=\t" << localCentroid.Z() << "\n";
  return (myVolume);
}

// --------------------- VolumeFix End   ---

//=======================================================================
// Section: Work with val props
//=======================================================================

//=================================================================================================

static Standard_Integer SetProps(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " DocName {Shape|Label} [epsilon = 0.001]\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  Standard_Real Vres, Ares;

  DataLabel aLabel;
  Tool3::Label(Doc->GetData(), argv[2], aLabel);
  TopoShape aShape;
  if (aLabel.IsNull())
  {
    aShape = DBRep1::Get(argv[2]);
    if (!aShape.IsNull())
    {
      Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
      aLabel                          = STool->FindShape(aShape);
    }
  }
  else
  {
    aShape = XCAFDoc_ShapeTool::GetShape(aLabel);
  }
  if (!aLabel.IsNull())
  {

    // retrieve epsilon
    Standard_Real anEps;
    if (argc > 3)
      anEps = Draw1::Atof(argv[3]);
    else
      anEps = 0.001;

    GeometricProperties G;
    BRepGProp::VolumeProperties(aShape, G, anEps, Standard_True);
    Vres                           = G.Mass();
    Handle(XCAFDoc_Volume) aVolume = new XCAFDoc_Volume;
    if (!aLabel.FindAttribute(XCAFDoc_Volume::GetID(), aVolume))
      aLabel.AddAttribute(aVolume);
    aVolume->Set(Vres);

    Point3d                   aPoint    = G.CentreOfMass();
    Handle(XCAFDoc_Centroid) aCentroid = new XCAFDoc_Centroid;
    if (!aLabel.FindAttribute(XCAFDoc_Centroid::GetID(), aCentroid))
      aLabel.AddAttribute(aCentroid);
    aCentroid->Set(aPoint);

    BRepGProp::SurfaceProperties(aShape, G, anEps);
    Ares                       = G.Mass();
    Handle(XCAFDoc_Area) aArea = new XCAFDoc_Area;
    if (!aLabel.FindAttribute(XCAFDoc_Area::GetID(), aArea))
      aLabel.AddAttribute(aArea);
    aArea->Set(Ares);

    di << argv[2] << ": Volume = " << Vres << ", Area = " << Ares << ", Centroid is (" << aPoint.X()
       << ", " << aPoint.Y() << ", " << aPoint.Z() << ")";
  }
  return 0;
}

//=================================================================================================

static Standard_Integer SetVolume(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Use: " << argv[0] << " DocName {Label|Shape} volume\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  Standard_Real res = 0.;

  DataLabel aLabel;
  Tool3::Label(Doc->GetData(), argv[2], aLabel);
  if (aLabel.IsNull())
  {
    TopoShape aShape = DBRep1::Get(argv[2]);
    if (!aShape.IsNull())
    {
      Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
      aLabel                          = STool->FindShape(aShape);
    }
  }
  if (!aLabel.IsNull())
  {
    res                            = Draw1::Atof(argv[3]);
    Handle(XCAFDoc_Volume) aVolume = new XCAFDoc_Volume;
    if (!aLabel.FindAttribute(XCAFDoc_Volume::GetID(), aVolume))
      aLabel.AddAttribute(aVolume);
    aVolume->Set(res);
  }

  di << res;
  return 0;
}

//=================================================================================================

static Standard_Integer SetArea(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Use: " << argv[0] << " DocName {Label|Shape} area\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  Standard_Real res = 0.;

  DataLabel aLabel;
  Tool3::Label(Doc->GetData(), argv[2], aLabel);
  if (aLabel.IsNull())
  {
    TopoShape aShape = DBRep1::Get(argv[2]);
    if (!aShape.IsNull())
    {
      Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
      aLabel                          = STool->FindShape(aShape);
    }
  }
  if (!aLabel.IsNull())
  {
    res                        = Draw1::Atof(argv[3]);
    Handle(XCAFDoc_Area) aArea = new XCAFDoc_Area;
    if (!aLabel.FindAttribute(XCAFDoc_Area::GetID(), aArea))
      aLabel.AddAttribute(aArea);
    aArea->Set(res);
  }
  di << res;
  return 0;
}

//=================================================================================================

static Standard_Integer SetCentroid(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 6)
  {
    di << "Use: " << argv[0] << " DocName {Label|Shape} x y z\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  Point3d aPoint;

  DataLabel aLabel;
  Tool3::Label(Doc->GetData(), argv[2], aLabel);
  if (aLabel.IsNull())
  {
    TopoShape aShape = DBRep1::Get(argv[2]);
    if (!aShape.IsNull())
    {
      Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
      aLabel                          = STool->FindShape(aShape);
    }
  }
  if (!aLabel.IsNull())
  {
    aPoint.SetX(Draw1::Atof(argv[3]));
    aPoint.SetY(Draw1::Atof(argv[4]));
    aPoint.SetZ(Draw1::Atof(argv[5]));
    Handle(XCAFDoc_Centroid) aCentroid = new XCAFDoc_Centroid;
    if (!aLabel.FindAttribute(XCAFDoc_Centroid::GetID(), aCentroid))
      aLabel.AddAttribute(aCentroid);
    aCentroid->Set(aPoint);
    di << Draw1::Atof(argv[3]) << " " << Draw1::Atof(argv[4]) << " " << Draw1::Atof(argv[5]);
  }
  return 0;
}

//=================================================================================================

static Standard_Integer GetVolume(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 3)
  {
    di << "Use: " << argv[0] << " DocName {Shape|Label}\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  Tool3::Label(Doc->GetData(), argv[2], aLabel);
  if (aLabel.IsNull())
  {
    TopoShape aShape = DBRep1::Get(argv[2]);
    if (!aShape.IsNull())
    {
      Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
      aLabel                          = STool->FindShape(aShape);
    }
  }
  if (!aLabel.IsNull())
  {
    //    Handle(XCAFDoc_Volume) aVolume = new (XCAFDoc_Volume);
    //    if (aLabel.FindAttribute (XCAFDoc_Volume::GetID(), aVolume)) di << aVolume->Get();
    // another case
    Standard_Real aVol;
    if (XCAFDoc_Volume::Get(aLabel, aVol))
      di << aVol;
  }
  return 0;
}

//=================================================================================================

static Standard_Integer GetArea(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 3)
  {
    di << "Use: " << argv[0] << " DocName {Shape|Label}\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  Tool3::Label(Doc->GetData(), argv[2], aLabel);
  if (aLabel.IsNull())
  {
    TopoShape aShape = DBRep1::Get(argv[2]);
    if (!aShape.IsNull())
    {
      Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
      aLabel                          = STool->FindShape(aShape);
    }
  }
  if (!aLabel.IsNull())
  {
    //     Handle(XCAFDoc_Area) aArea = new (XCAFDoc_Area);
    //     if (aLabel.FindAttribute (XCAFDoc_Area::GetID(), aArea)) di << aArea->Get();
    // another case
    Standard_Real anA;
    if (XCAFDoc_Area::Get(aLabel, anA))
      di << anA;
  }
  return 0;
}

//=================================================================================================

static Standard_Integer GetCentroid(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 3)
  {
    di << "Use: " << argv[0] << " DocName {Shape|Label} \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  Point3d aPoint;

  DataLabel aLabel;
  Tool3::Label(Doc->GetData(), argv[2], aLabel);
  if (aLabel.IsNull())
  {
    TopoShape aShape = DBRep1::Get(argv[2]);
    if (!aShape.IsNull())
    {
      Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
      aLabel                          = STool->FindShape(aShape);
    }
  }
  if (!aLabel.IsNull())
  {
    Handle(XCAFDoc_Centroid) aCentroid = new (XCAFDoc_Centroid);
    if (aLabel.FindAttribute(XCAFDoc_Centroid::GetID(), aCentroid))
    {
      //       aPoint = aCentroid->Get();
      //       di << aPoint.X()<<" "<<aPoint.Y()<<" "<<aPoint.Z();
      // another case
      if (XCAFDoc_Centroid::Get(aLabel, aPoint))
        di << aPoint.X() << " " << aPoint.Y() << " " << aPoint.Z();
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer CheckProps(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 2)
  {
    di << "Use: " << argv[0] << " DocName [ 0|deflection [Shape|Label] ]\n";
    di << "     If second argument is 0, standard CADCADE tool is used for\n";
    di << "     computation of volume and CG.\n";
    di << "     If second argument is not 0, it is treated as deflection\n";
    di << "     and computation is done by triangulations\n";
    di << "     If the second argument is negative, meshing is forced\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Standard_Boolean withVolFix = Standard_False;
  if (argc > 2 && Draw1::Atof(argv[2]) != 0)
    withVolFix = Standard_True;
  Standard_Boolean  wholeDoc = (argc < 4);
  TDF_LabelSequence seq;
  if (!wholeDoc)
  {
    DataLabel aLabel;
    Tool3::Label(Doc->GetData(), argv[3], aLabel);
    TopoShape aShape;
    if (aLabel.IsNull())
    {
      aShape = DBRep1::Get(argv[3]);
      if (!aShape.IsNull())
      {
        Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
        aLabel                          = STool->FindShape(aShape);
      }
    }
    if (aLabel.IsNull())
      return 1;
    seq.Append(aLabel);
  }
  else
  {
    Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
    STool->GetShapes(seq);
  }
  if (wholeDoc)
  {
    di << "Label            Area defect   Volume defect    dX      dY      dZ    Name\n";
  }
  for (Standard_Integer i = 1; i <= seq.Length(); i++)
  {
    DataLabel aLabel = seq(i);

    // add instance labels to sequence to process them as well
    if (XCAFDoc_ShapeTool::IsAssembly(aLabel))
    {
      TDF_LabelSequence comp;
      XCAFDoc_ShapeTool::GetComponents(aLabel, comp);
      Standard_Integer m = i;
      for (Standard_Integer k = 1; k <= comp.Length(); k++)
      {
        DataLabel                lab = comp(k);
        Handle(XCAFDoc_Volume)   aVolume;
        Handle(XCAFDoc_Area)     aArea;
        Handle(XCAFDoc_Centroid) aCentroid;
        if (!lab.FindAttribute(XCAFDoc_Volume::GetID(), aVolume)
            && !lab.FindAttribute(XCAFDoc_Area::GetID(), aArea)
            && !lab.FindAttribute(XCAFDoc_Centroid::GetID(), aCentroid))
          continue;
        seq.InsertAfter(m++, lab);
      }
    }

    AsciiString1 str;
    Tool3::Entry(aLabel, str);
    // printf ( "%s%-12.12s", ( wholeDoc ? "" : "Label " ), str.ToCString() );
    // fflush ( stdout );
    char string1[260];
    Sprintf(string1, "%s%-12.12s", (wholeDoc ? "" : "Label "), str.ToCString());
    di << string1;
    Handle(NameAttribute) N;
    if (aLabel.FindAttribute(NameAttribute::GetID(), N) && !wholeDoc)
    {
      di << " \"" << N->Get() << "\"";
    }
    if (!wholeDoc)
      di << "\n";

    Handle(XCAFDoc_Volume)   aVolume;
    Handle(XCAFDoc_Area)     aArea;
    Handle(XCAFDoc_Centroid) aCentroid;
    aLabel.FindAttribute(XCAFDoc_Volume::GetID(), aVolume);
    aLabel.FindAttribute(XCAFDoc_Area::GetID(), aArea);
    aLabel.FindAttribute(XCAFDoc_Centroid::GetID(), aCentroid);
    GeometricProperties G;

    TopoShape aShape = XCAFDoc_ShapeTool::GetShape(aLabel);
    if (!aArea.IsNull())
    {
      try
      {
        OCC_CATCH_SIGNALS
        BRepGProp::SurfaceProperties(aShape, G, 0.001);
        // printf ("%s%9.1f (%3d%%)%s", ( wholeDoc ? "" : "  Area defect:   " ),
        //	aArea->Get() - G.Mass(),
        //	(Standard_Integer)( Abs ( G.Mass() ) > 1e-10 ? 100. * ( aArea->Get() - G.Mass() ) /
        // G.Mass() : 999. ), 	( wholeDoc ? "" : "\n" ));
        char string2[260];
        Sprintf(string2,
                "%s%9.1f (%3d%%)%s",
                (wholeDoc ? "" : "  Area defect:   "),
                aArea->Get() - G.Mass(),
                (Standard_Integer)(Abs(G.Mass()) > 1e-10
                                     ? 100. * (aArea->Get() - G.Mass()) / G.Mass()
                                     : 999.),
                (wholeDoc ? "" : "\n"));
        di << string2;
      }
      catch (ExceptionBase const&)
      {
        // printf ( "%-16.16s", "exception" );
        char string3[260];
        Sprintf(string3, "%-16.16s", "exception");
        di << string3;
      }
    }
    else if (wholeDoc)
    {
      // printf ( "%16.16s", "" );
      char string4[260];
      Sprintf(string4, "%16.16s", "");
      di << string4;
    }

    if (!aVolume.IsNull() || !aCentroid.IsNull())
    {
      try
      {
        OCC_CATCH_SIGNALS
        // Added for check Volume. PTV 08 Nov 2000.
        Standard_Real localVolume;
        Point3d        pcg(0, 0, 0);
        if (withVolFix)
        {
          Standard_Real    tol       = Draw1::Atof(argv[2]);
          Standard_Boolean withForce = Standard_False;
          if (tol < 0)
          {
            withForce = Standard_True;
            tol       = -tol;
          }
          localVolume = CalculVolume(aShape, pcg, tol, withForce, di);
        }
        else
        {
          BRepGProp::VolumeProperties(aShape, G, 0.001, Standard_True);
          localVolume = G.Mass();
          pcg         = G.CentreOfMass();
        }

        if (!aVolume.IsNull())
        {
          // printf ("%s%9.1f (%3d%%)%s", ( wholeDoc ? "" : "  Volume defect: " ),
          //   aVolume->Get() - localVolume,
          //   (Standard_Integer)( Abs ( localVolume ) > 1e-10 ? 100. * ( aVolume->Get() -
          //   localVolume ) / localVolume : 999. ), ( wholeDoc ? "" : "\n" ));
          char string5[260];
          Sprintf(string5,
                  "%s%9.1f (%3d%%)%s",
                  (wholeDoc ? "" : "  Volume defect: "),
                  aVolume->Get() - localVolume,
                  (Standard_Integer)(Abs(localVolume) > 1e-10
                                       ? 100. * (aVolume->Get() - localVolume) / localVolume
                                       : 999.),
                  (wholeDoc ? "" : "\n"));
          di << string5;
        }
        else if (wholeDoc)
        {
          // printf ( "%16.16s", "" );
          char string6[260];
          Sprintf(string6, "%16.16s", "");
          di << string6;
        }

        if (!aCentroid.IsNull())
        {
          Point3d p = aCentroid->Get();
          char   string7[260];
          if (wholeDoc)
          {
            // printf ( " %7.2f %7.2f %7.2f",
            //     p.X() - pcg.X(), p.Y() - pcg.Y(), p.Z() - pcg.Z() );
            Sprintf(string7,
                    " %7.2f %7.2f %7.2f",
                    p.X() - pcg.X(),
                    p.Y() - pcg.Y(),
                    p.Z() - pcg.Z());
          }
          else
          {
            // printf ( "  CG defect: dX=%.3f, dY=%.3f, dZ=%.3f\n",
            //     p.X() - pcg.X(), p.Y() - pcg.Y(), p.Z() - pcg.Z() );
            Sprintf(string7,
                    "  CG defect: dX=%.3f, dY=%.3f, dZ=%.3f\n",
                    p.X() - pcg.X(),
                    p.Y() - pcg.Y(),
                    p.Z() - pcg.Z());
          }
          di << string7;
        }
        else if (wholeDoc)
        {
          // printf ( "%24.24s", "" );
          char string8[260];
          Sprintf(string8, "%24.24s", "");
          di << string8;
        }
      }
      catch (ExceptionBase const& anException)
      {
        // printf ( "%40.40s", "exception" );
        char string9[260];
        Sprintf(string9, "%40.40s", "exception");
        di << string9;
#ifdef OCCT_DEBUG
        // fflush ( stdout );
        di << ": ";
        di << anException.GetMessageString();
        di << " ** Skip\n";
#endif
        (void)anException;
      }
    }
    else if (wholeDoc)
    {
      // printf ( "%40.40s", "" );
      char string10[260];
      Sprintf(string10, "%40.40s", "");
      di << string10;
    }
    // fflush ( stdout );

    if (wholeDoc)
    {
      if (!N.IsNull())
      {
        di << "  \"" << N->Get() << "\"";
      }
      di << "\n";
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer ShapeVolume(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 3)
  {
    di << "Use: " << argv[0] << " Shape deflection \n";
    return 1;
  }
  TopoShape aShape = DBRep1::Get(argv[1]);
  if (aShape.IsNull())
    return 1;
  Point3d           aPoint(0, 0, 0);
  Standard_Real    localVolume;
  Standard_Real    tol       = Draw1::Atof(argv[2]);
  Standard_Boolean withForce = Standard_False;
  if (tol < 0)
  {
    withForce = Standard_True;
    tol       = -tol;
  }
  localVolume = CalculVolume(aShape, aPoint, tol, withForce, di);
  // std::cout << "Volume : " << std::setw(15) << localVolume << "\n" << std::endl;
  Standard_SStream aSStream;
  aSStream << "Volume : " << std::setw(15) << localVolume << "\n";
  di << aSStream;
  return 0;
}

//=======================================================================
// function : GetMassProps
// purpose  : auxiliary for ShapeMassProps
//=======================================================================

static Standard_Boolean GetMassProps(const DataLabel&    aLabel,
                                     gp_XYZ&             theCenterGravity,
                                     Standard_Real&      theMassVal,
                                     const Standard_Real thetol)
{
  Standard_Real aDensity = XCAFDoc_MaterialTool::GetDensityForShape(aLabel);

  if (aDensity > 0)
  {
    TopoShape aShape = XCAFDoc_ShapeTool::GetShape(aLabel);
    GeometricProperties G;
    BRepGProp::VolumeProperties(aShape, G, 0.001, Standard_True);
    Standard_Real localVolume = G.Mass();
    theMassVal                = aDensity * localVolume;
    theCenterGravity          = G.CentreOfMass().XYZ();
    return Standard_True;
  }

  if (aDensity == 0)
  {
    Handle(ShapeAttribute) NS;
    if (aLabel.FindAttribute(ShapeAttribute::GetID(), NS))
    {
      // S = Tool11::GetShape(NS);
      TopoShape aSh = NS->Get();
      if (aSh.ShapeType() == TopAbs_SOLID)
        return Standard_False;
    }

    // TopoShape aSh = XCAFDoc_ShapeTool::GetShape(aLabel);
    // if(aSh.ShapeType()==TopAbs_SOLID) return Standard_False;

    Handle(TDataStd_TreeNode) Node;
    if (aLabel.FindAttribute(XCAFDoc::ShapeRefGUID(), Node) && Node->HasFather())
    {
      DataLabel SubL = Node->Father()->Label();
      if (GetMassProps(SubL, theCenterGravity, theMassVal, thetol))
      {
        Handle(XCAFDoc_Location) LocationAttribute;
        if (aLabel.FindAttribute(XCAFDoc_Location::GetID(), LocationAttribute))
        {
          gp_XYZ tmp = LocationAttribute->Get().Transformation().TranslationPart();
          theCenterGravity += tmp;
        }
        return Standard_True;
      }
      else
        return Standard_False;
    }
    else
    {
      // calculate for components
      TDF_LabelSequence comp;
      XCAFDoc_ShapeTool::GetComponents(aLabel, comp);
      if (!comp.Length())
        return Standard_False;

      TColgp_Array1OfXYZ   anArrCentres(1, comp.Length());
      TColStd_Array1OfReal anArrMass(1, comp.Length());
      anArrMass.Init(0.0);
      Standard_Real    aTotalMass = 0.0;
      Standard_Integer k          = 1;
      for (; k <= comp.Length(); k++)
      {
        DataLabel     lab = comp(k);
        gp_XYZ        aCenterGravity(0.0, 0.0, 0.0);
        Standard_Real aMassVal = 0.0;
        if (GetMassProps(lab, aCenterGravity, aMassVal, thetol))
        {
          anArrCentres.SetValue(k, aCenterGravity);
          anArrMass.SetValue(k, aMassVal);
          aTotalMass += aMassVal;
        }
      }
      if (aTotalMass > 0)
      {
        Standard_Real x = 0.0, y = 0.0, z = 0.0;
        for (k = 1; k <= anArrMass.Length(); k++)
        {
          Standard_Real koef = anArrMass.Value(k) / aTotalMass;
          x += (anArrCentres.Value(k).X() * koef);
          y += (anArrCentres.Value(k).Y() * koef);
          z += (anArrCentres.Value(k).Z() * koef);
        }
        theMassVal = aTotalMass;
        theCenterGravity.SetCoord(x, y, z);
      }
    }
  }
  return Standard_True;
}

//=================================================================================================

static Standard_Integer ShapeMassProps(DrawInterpreter& di,
                                       Standard_Integer  argc,
                                       const char**      argv)
{

  if (argc < 2)
  {
    di << "Use: " << argv[0] << " DocName [deflection [Shape|Label] ]\n";
    di << "     If second argument is 0, standard CADCADE tool is used for\n";
    di << "     computation of volume and CG.\n";
    di << "     If second argument is not 0, it is treated as deflection\n";
    di << "     and computation is done by triangulations\n";
    di << "     If the second argument is negative, meshing is forced\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  Standard_Real atol = Precision::Confusion();
  if (argc > 2)
    atol = Draw1::Atof(argv[2]);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Standard_Boolean  wholeDoc = (argc < 4);
  TDF_LabelSequence seq;
  if (!wholeDoc)
  {
    DataLabel aLabel;
    Tool3::Label(Doc->GetData(), argv[3], aLabel);
    TopoShape aShape;
    if (aLabel.IsNull())
    {
      aShape = DBRep1::Get(argv[3]);
      if (!aShape.IsNull())
      {
        Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
        aLabel                          = STool->FindShape(aShape);
      }
    }
    if (aLabel.IsNull())
      return 1;
    seq.Append(aLabel);
  }
  else
  {
    Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
    STool->GetShapes(seq);
  }
  // if ( wholeDoc ) {
  //   di << "Label            Area defect   Volume defect    dX      dY      dZ    Name\n";
  // }
  gp_XYZ        aCenterGravity(0.0, 0.0, 0.0);
  Standard_Real aMassVal = 0.0;
  for (Standard_Integer i = 1; i <= seq.Length(); i++)
  {
    DataLabel aLabel = seq(i);
    GetMassProps(aLabel, aCenterGravity, aMassVal, atol);
    //    if(GetMassProps(aLabel,aCenterGravity,aMassVal,atol))
    //    {
    AsciiString1 str;
    Tool3::Entry(aLabel, str);
    if (aMassVal > 0)
    {
      di << "Shape from label : " << str.ToCString() << "\n";
      di << "Mass = " << aMassVal << "\n";
      di << "CenterOfGravity X = " << aCenterGravity.X() << ",Y = " << aCenterGravity.Y()
         << ",Z = " << aCenterGravity.Z() << "\n";
      di << "\n";
    }
    else
    {
      //      di<<"For one component density is absent\n";
      di << "Shape from label : " << str.ToCString() << " not have a mass\n";
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer SetMaterial(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 5)
  {
    di << "Use: " << argv[0] << " Doc {Label|Shape} name density(g/cu sm) \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  Tool3::Label(Doc->GetData(), argv[2], aLabel);

  Handle(XCAFDoc_MaterialTool) MatTool = XCAFDoc_DocumentTool::MaterialTool(Doc->Main());

  MatTool->SetMaterial(aLabel,
                       new TCollection_HAsciiString(argv[3]),
                       new TCollection_HAsciiString(""),
                       Draw1::Atof(argv[4]),
                       new TCollection_HAsciiString("density measure"),
                       new TCollection_HAsciiString("POSITIVE_RATIO_MEASURE"));

  return 0;
}

static Standard_Integer GetValidationProps(DrawInterpreter& di,
                                           Standard_Integer  argc,
                                           const char**      argv)
{
  if (argc < 2)
  {
    di << "Use: " << argv[0] << " Doc {Label|Shape}\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  TDF_LabelSequence         aLabels;
  if (argc == 2)
    STool->GetShapes(aLabels);
  else
  {
    DataLabel    aLabel;
    TopoShape aShape = DBRep1::Get(argv[2]);
    if (aShape.IsNull())
    {
      Tool3::Label(Doc->GetData(), argv[2], aLabel);
    }
    else
    {
      aLabel = STool->FindShape(aShape);
    }
    if (!aLabel.IsNull())
      aLabels.Append(aLabel);
  }

  enum
  {
    Vol      = 0,
    Area     = 1,
    Centroid = 2
  };

  Standard_Integer nbProps[3];
  Standard_Integer j = 0;
  for (; j <= Centroid; j++)
    nbProps[j] = 0;
  Standard_Integer i = 1;
  for (; i <= aLabels.Length(); i++)
  {
    DataLabel aLabel = aLabels(i);

    Standard_Real aProp[2];

    aProp[0] = 0.;
    aProp[1] = 0.;

    Point3d aP(Precision::Infinite(), Precision::Infinite(), Precision::Infinite());
    XCAFDoc_Volume::Get(aLabel, aProp[Vol]);
    XCAFDoc_Area::Get(aLabel, aProp[Area]);

    Handle(XCAFDoc_Centroid) aCentroid = new (XCAFDoc_Centroid);
    if (aLabel.FindAttribute(XCAFDoc_Centroid::GetID(), aCentroid))
      XCAFDoc_Centroid::Get(aLabel, aP);

    if (aProp[Vol] > 0 || aProp[Area] > 0 || !Precision::IsInfinite(aP.X()))
    {
      AsciiString1 str;
      Tool3::Entry(aLabel, str);
      di << "Label : " << str;

      for (j = 0; j <= Area; j++)
      {
        if (aProp[j] > 0)
        {
          di << (j == Vol ? "; Volume - " : "; Area - ") << aProp[j];
          nbProps[j]++;
        }
      }

      if (!Precision::IsInfinite(aP.X()))
      {
        di << "; Centroid -  " << aP.X() << " " << aP.Y() << " " << aP.Z();
        nbProps[Centroid]++;
      }
      di << "\n";
    }
  }
  di << "=========================================================" << "\n";
  di << "Number of the validation properties : ";
  for (i = Vol; i <= Centroid; i++)
    di << (i == Vol ? "Volume" : (i == Area ? "Area" : "Centroid")) << " : " << nbProps[i] << " ; ";

  di << "\n";
  return 0;
}

//=================================================================================================

void XDEDRAW_Props::InitCommands(DrawInterpreter& di)
{
  static Standard_Boolean initactor = Standard_False;
  if (initactor)
  {
    return;
  }
  initactor = Standard_True;

  Standard_CString g = "XDE property's commands";

  di.Add("XSetVolume",
         "DocName {Label|Shape} volume \t: Setting volume to shape",
         __FILE__,
         SetVolume,
         g);

  di.Add("XGetVolume", "DocName {Shape|Label} \t: Getting volume of shape", __FILE__, GetVolume, g);

  di.Add("XSetArea", "DocName {Label|Shape} area \t: Setting area to shape", __FILE__, SetArea, g);

  di.Add("XGetArea", "DocName {Shape|Label} \t: Getting area of shape", __FILE__, GetArea, g);

  di.Add("XSetCentroid",
         "DocName  {Label|Shape} x y z \t: Setting centroid to shape",
         __FILE__,
         SetCentroid,
         g);

  di.Add("XGetCentroid",
         "DocName {Shape|Label} \t: Getting centroid of shape ",
         __FILE__,
         GetCentroid,
         g);

  di.Add("XSetProps",
         "DocName {Shape|Label} [epsilon = 0.001] \t: Compute properties for shape ",
         __FILE__,
         SetProps,
         g);

  di.Add("XCheckProps",
         "DocName [ 0|deflection [Shape|Label] ]\t: Check properties recorded for shape ",
         __FILE__,
         CheckProps,
         g);
  di.Add("XShapeVolume", "Shape \t: Calculating volume of shape", __FILE__, ShapeVolume, g);
  di.Add("XShapeMassProps",
         "XShapeMassProps DocName [deflection [Shape|Label] ]\t: Get mass value and center of "
         "gravity for shape ",
         __FILE__,
         ShapeMassProps,
         g);
  di.Add("XSetMaterial",
         "Doc {Label|Shape} name density(g/cu sm) \t: Set material to shape given by Label",
         __FILE__,
         SetMaterial,
         g);
  di.Add("XGetValProps", "Doc {Label|Shape}", __FILE__, GetValidationProps, g);
}
