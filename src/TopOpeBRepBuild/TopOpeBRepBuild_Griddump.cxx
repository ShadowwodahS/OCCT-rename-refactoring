// Created on: 1996-03-07
// Created by: Jean Yves LEBEY
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

#include <TopOpeBRepBuild_define.hxx>
#include <TopOpeBRepBuild_EdgeBuilder.hxx>
#include <TopOpeBRepBuild_FaceBuilder.hxx>
#include <TopOpeBRepBuild_ShapeSet.hxx>
#include <TopOpeBRepBuild_SolidBuilder.hxx>
#include <TopOpeBRepTool_ShapeExplorer.hxx>

#ifdef DRAW
  #include <DBRep.hxx>
#endif

#include <GeomAdaptor_Curve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopOpeBRepBuild_GIter.hxx>
#include <TopOpeBRepDS.hxx>
#include <TopOpeBRepDS_Dumper.hxx>
#include <gp_Pnt.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_Surface.hxx>

#ifdef OCCT_DEBUG
static AsciiString1 PRODINS("dins ");
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpLS(const ShapeList& L) const
{
  TopTools_ListIteratorOfListOfShape it(L);
  for (; it.More(); it.Next())
  {
    const TopoShape& SL = it.Value();
    GdumpSHA(SL);
  }
}
#else
void TopOpeBRepBuild_Builder::GdumpLS(const ShapeList&) const {}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::PrintGeo(const TopoShape& S)
{
  if (S.ShapeType() == TopAbs_VERTEX)
    PrintPnt(TopoDS::Vertex(S));
  else if (S.ShapeType() == TopAbs_EDGE)
    PrintCur(TopoDS::Edge(S));
  else if (S.ShapeType() == TopAbs_FACE)
    PrintSur(TopoDS::Face(S));
}
#else
void TopOpeBRepBuild_Builder::PrintGeo(const TopoShape&) {}
#endif

//=======================================================================
// function : PrintSur
// purpose  : print the name of a surface
//=======================================================================
#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::PrintSur(const TopoFace& F)
{
  BRepAdaptor_Surface STA_Surface(F);
  GeomAbs_SurfaceType t = STA_Surface.GetType();
  switch (t)
  {
    case GeomAbs_Plane:
      std::cout << "PLANE";
      break;
    case GeomAbs_Cylinder:
      std::cout << "CYLINDER";
      break;
    case GeomAbs_Cone:
      std::cout << "CONE";
      break;
    case GeomAbs_Sphere:
      std::cout << "SPHERE";
      break;
    case GeomAbs_Torus:
      std::cout << "TORUS";
      break;
    case GeomAbs_BezierSurface:
      std::cout << "BEZIERSURFACE";
      break;
    case GeomAbs_BSplineSurface:
      std::cout << "BSPLINESURFACE";
      break;
    case GeomAbs_SurfaceOfRevolution:
      std::cout << "SURFACEOFREVOLUTION";
      break;
    case GeomAbs_SurfaceOfExtrusion:
      std::cout << "SURFACEOFEXTRUSION";
      break;
    case GeomAbs_OtherSurface:
    default:
      std::cout << "OTHERSURFACE";
      break;
  }
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::PrintSur(const TopoFace&) {}
#endif

//=======================================================================
// function : PrintCur
// purpose  : print the name of a curve
//=======================================================================
#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::PrintCur(const TopoEdge& E)
{
  TopLoc_Location    L;
  Standard_Real      f, l;
  Handle(GeomCurve3d) C = BRepInspector::Curve(E, L, f, l);
  if (C.IsNull())
    return;
  GeomAdaptor_Curve GC(C);
  GeomAbs_CurveType t = GC.GetType();

  switch (t)
  {
    case GeomAbs_Line:
      std::cout << "LINE";
      break;
    case GeomAbs_Circle:
      std::cout << "CIRCLE";
      break;
    case GeomAbs_Ellipse:
      std::cout << "ELLIPSE";
      break;
    case GeomAbs_Hyperbola:
      std::cout << "HYPERBOLA";
      break;
    case GeomAbs_Parabola:
      std::cout << "PARABOLA";
      break;
    case GeomAbs_BezierCurve:
      std::cout << "BEZIERCURVE";
      break;
    case GeomAbs_BSplineCurve:
      std::cout << "BSPLINECURVE " << GC.BSpline()->Degree();
      break;
    case GeomAbs_OffsetCurve:
      std::cout << "OFFSETCURVE";
      break;
    case GeomAbs_OtherCurve:
      std::cout << "OTHERCURVE";
      break;
  }
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::PrintCur(const TopoEdge&) {}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::PrintPnt(const TopoVertex& V)
{
  GdumpPNT(BRepInspector::Pnt(V));
}
#else
void TopOpeBRepBuild_Builder::PrintPnt(const TopoVertex&) {}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::PrintOri(const TopoShape& S)
{
  TopAbs1::Print(S.Orientation(), std::cout);
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::PrintOri(const TopoShape& /*S*/) {}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
AsciiString1 TopOpeBRepBuild_Builder::StringState(const TopAbs_State st)
#else
AsciiString1 TopOpeBRepBuild_Builder::StringState(const TopAbs_State)
#endif
{
  AsciiString1 s;
#ifdef OCCT_DEBUG
  switch (st)
  {
    case TopAbs_ON:
      s.AssignCat("ON");
      break;
    case TopAbs_IN:
      s.AssignCat("IN");
      break;
    case TopAbs_OUT:
      s.AssignCat("OUT");
      break;
    case TopAbs_UNKNOWN:
      s.AssignCat("UNKNOWN");
      break;
  }
#endif
  return s;
}

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpPNT(const Point3d& P)
{
  std::cout << P.X() << " " << P.Y() << " " << P.Z();
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpPNT(const Point3d&) {}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpORIPARPNT(const TopAbs_Orientation o,
                                             const Standard_Real      p,
                                             const Point3d&            Pnt)
{
  TopAbs1::Print(o, std::cout);
  std::cout << " " << p << " pnt ";
  GdumpPNT(Pnt);
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpORIPARPNT(const TopAbs_Orientation,
                                             const Standard_Real,
                                             const Point3d&)
{
}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpEDGVER(const TopoShape&    E,
                                          const TopoShape&    V,
                                          const Standard_Address s) const
{
  char* c = (char*)s;
  if (c)
    std::cout << c;
  const TopoEdge&   EE  = TopoDS::Edge(E);
  const TopoVertex& VV  = TopoDS::Vertex(V);
  Standard_Real        par = BRepInspector::Parameter(VV, EE);
  Point3d               P   = BRepInspector::Pnt(VV);
  GdumpORIPARPNT(VV.Orientation(), par, P);
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpEDGVER(const TopoShape&,
                                          const TopoShape&,
                                          const Standard_Address) const
{
}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpEDG(const TopoShape& E, const Standard_Address s) const
{
  char* c = (char*)s;
  if (c)
    std::cout << c;
  const TopoEdge& EE = TopoDS::Edge(E);
  Standard_Integer   n  = 0;
  GdumpSHAORI(E, (char*)"vertices of ");
  std::cout << std::endl;
  TopOpeBRepTool_ShapeExplorer ex(E, TopAbs_VERTEX);
  char                         strpar[256];
  Sprintf(strpar, " #");
  for (; ex.More(); ex.Next())
  {
    const TopoVertex& VV = TopoDS::Vertex(ex.Current());
    TopAbs_Orientation   o  = VV.Orientation();
    std::cout << "vertex v";
    if (o == TopAbs_FORWARD)
      std::cout << "F";
    else if (o == TopAbs_REVERSED)
      std::cout << "R";
    else if (o == TopAbs_INTERNAL)
      std::cout << "I";
    else if (o == TopAbs_EXTERNAL)
      std::cout << "E";
    std::cout << ++n << " ";
    TopOpeBRepBuild_Builder::PrintPnt(VV);
    std::cout << ";";
    Standard_Real par = BRepInspector::Parameter(VV, EE);
    char          spar[255];
    Sprintf(spar, " par%d %f", n, par);
    strcat(strpar, spar);
  }
  if (n)
    std::cout << strpar << std::endl;
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpEDG(const TopoShape&, const Standard_Address) const {}
#endif

//=================================================================================================

void TopOpeBRepBuild_Builder::GdumpSAMDOM(const ShapeList& L,
                                          const Standard_Address      astr) const
{
  TopOpeBRepDS_Dumper Dumper(myDataStructure);
  std::cout << Dumper.SPrintShapeRefOri(L, (char*)astr) << std::endl;
  std::cout.flush();
}

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpSHA(const TopoShape& S, const Standard_Address str) const
{
  char* c = (char*)str;
  if (c)
    std::cout << c;
  if (S.IsNull())
    return;
  TopAbs_ShapeEnum tS = S.ShapeType();
  Standard_Integer iS = 0;
  if (!myDataStructure.IsNull())
    iS = myDataStructure->Shape(S);
  TopOpeBRepDS1::Print(tS, iS, std::cout);
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpSHA(const TopoShape&, const Standard_Address) const {}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpSHAORI(const TopoShape& S, const Standard_Address str) const
{
  char* c = (char*)str;
  if (c)
    std::cout << c;
  GdumpSHA(S, NULL);
  std::cout << ",";
  PrintOri(S);
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpSHAORI(const TopoShape&, const Standard_Address) const {}
#endif
//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpSHAORIGEO(const TopoShape&    S,
                                             const Standard_Address str) const
{
  char* c = (char*)str;
  if (c)
    std::cout << c;
  GdumpSHAORI(S, NULL);
  std::cout << ",";
  PrintGeo(S);
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpSHAORIGEO(const TopoShape&, const Standard_Address) const {}
#endif
//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpSHASTA(const TopoShape&            S,
                                          const TopAbs_State             T,
                                          const AsciiString1& a,
                                          const AsciiString1& b) const
{
  std::cout << a;
  GdumpSHAORIGEO(S, NULL);
  std::cout << "," << StringState(T).ToCString();
  std::cout << b;
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpSHASTA(const TopoShape&,
                                          const TopAbs_State,
                                          const AsciiString1&,
                                          const AsciiString1&) const
{
}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpSHASTA(const Standard_Integer         iS,
                                          const TopAbs_State             T,
                                          const AsciiString1& a,
                                          const AsciiString1& b) const
{
  const TopoShape& S = myDataStructure->Shape(iS);
  GdumpSHASTA(S, T, a, b);
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpSHASTA(const Standard_Integer,
                                          const TopAbs_State,
                                          const AsciiString1&,
                                          const AsciiString1&) const
{
}
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpSHASTA(const Standard_Integer          iS,
                                          const TopAbs_State              T,
                                          const TopOpeBRepBuild_ShapeSet& SS,
                                          const AsciiString1&  a,
                                          const AsciiString1&  b,
                                          const AsciiString1&  c) const
{
  const TopoShape&     S   = myDataStructure->Shape(iS);
  AsciiString1 aib = a + " " + SS.DEBNumber() + " " + b;
  GdumpSHASTA(S, T, aib, c);
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpSHASTA(const Standard_Integer,
                                          const TopAbs_State,
                                          const TopOpeBRepBuild_ShapeSet&,
                                          const AsciiString1&,
                                          const AsciiString1&,
                                          const AsciiString1&) const
{
}
#endif

//=================================================================================================

void TopOpeBRepBuild_Builder::GdumpSHASETreset()
{
#ifdef OCCT_DEBUG
  mySHASETindex = 0;
#endif
}

//=================================================================================================

Standard_Integer TopOpeBRepBuild_Builder::GdumpSHASETindex()
{
  Standard_Integer n = 0;
#ifdef OCCT_DEBUG
  n = ++mySHASETindex;
#endif
  return n;
}

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpEXP(const TopOpeBRepTool_ShapeExplorer& Ex) const
{
  if (!Ex.More())
    return;
  TopAbs_ShapeEnum t = Ex.Current().ShapeType();
  if (t == TopAbs_SOLID)
    std::cout << "";
  else if (t == TopAbs_FACE)
    std::cout << "  ";
  else if (t == TopAbs_EDGE)
    std::cout << "     ";
  else
    std::cout << "??";
  Ex.DumpCurrent(std::cout);
  Standard_Integer I = myDataStructure->Shape(Ex.Current());
  if (I != 0)
    std::cout << " :  shape " << I;
  std::cout << std::endl;
  std::cout.flush();
}
#else
void TopOpeBRepBuild_Builder::GdumpEXP(const TopOpeBRepTool_ShapeExplorer&) const {}
#endif

//=================================================================================================

void TopOpeBRepBuild_Builder::GdumpSOBU(TopOpeBRepBuild_SolidBuilder& /*ME*/) const
{
#ifdef OCCT_DEBUG
#endif
} // GdumpSOBU

#ifdef OCCT_DEBUG
void* GFABUMAKEFACEPWES_DEB = NULL;
#endif

//=================================================================================================

#ifdef OCCT_DEBUG
void TopOpeBRepBuild_Builder::GdumpFABU(TopOpeBRepBuild_FaceBuilder& ME) const
{
  const TopoShape& face = ME.Face();
  Standard_Integer    iF;
  //  Standard_Boolean tSPS =
  GtraceSPS(face, iF);
  TopOpeBRepBuild_WireEdgeSet* PWES = (TopOpeBRepBuild_WireEdgeSet*)GFABUMAKEFACEPWES_DEB;

  Standard_Integer nf, nw, ne;
  ME.InitFace();
  if (ME.MoreFace())
    std::cout << "clear;" << std::endl;
  for (nf = 0; ME.MoreFace(); ME.NextFace())
  {
    nf++;
    std::cout << "# face " << nf << std::endl;
    for (nw = 0, ME.InitWire(); ME.MoreWire(); ME.NextWire())
    {
      nw++;
      Standard_Boolean ow = ME.IsOldWire();
      std::cout << "#  wire " << nw;
      if (ow)
        std::cout << " (old)";
      else
        std::cout << " (new)";
      std::cout << std::endl;
      if (!ow)
      {
        AsciiString1 whatis("whatis");
        for (ne = 0, ME.InitEdge(); ME.MoreEdge(); ME.NextEdge())
        {
          ne++;
          const TopoEdge&      EE = TopoDS::Edge(ME.Edge());
          AsciiString1 Enam("E");
          AsciiString1 VFnam("VF");
          AsciiString1 VRnam("VR");
          Enam = Enam + ne + "NF" + nf + "F" + iF;
          if (PWES)
            Enam = Enam + PWES->DEBName() + PWES->DEBNumber();
          VFnam = VFnam + ne + "NF" + nf + "F" + iF;
          VRnam = VRnam + ne + "NF" + nf + "F" + iF;
          //	  std::cout<<"    puts \"edge "<<ne<<" : "<<Enam<<"\"";std::cout<<"; ";
          TopoVertex VF, VR;
          TopExp1::Vertices(EE, VF, VR);
          if (!VF.IsNull() && !VR.IsNull() && !EE.IsNull())
          {
  #ifdef DRAW
            DBRep1::Set(Enam.ToCString(), EE);
            DBRep1::Set(VFnam.ToCString(), VF);
            DBRep1::Set(VRnam.ToCString(), VR);
  #endif
            std::cout << PRODINS << "-O -p 0.5 " << Enam;
            std::cout << "; ";
            //	    std::cout<<PRODINS<<VFnam; std::cout<<"; ";
            //	    std::cout<<PRODINS<<VRnam; std::cout<<"; ";
            //	    Point3d PF = BRepInspector::Pnt(VF);
            //	    Point3d PR = BRepInspector::Pnt(VR);
            //	    std::cout<<std::endl;
            //	    std::cout<<"# ";
            //	    std::cout<<"dinp "<<VFnam<<"
            //";TopOpeBRepBuild_Builder::PrintPnt(VF);std::cout<<"; "; 	    std::cout<<"dinp
            //"<<VRnam<<"
            //";TopOpeBRepBuild_Builder::PrintPnt(VR);std::cout<<"; ";
            std::cout << std::endl;
            whatis += " ";
            whatis += Enam;
          }
        }
        if (ne)
          std::cout << "    " << whatis << std::endl << std::endl;
      }
    }
  }
  std::cout.flush();
} // GdumpFABU
#else
void TopOpeBRepBuild_Builder::GdumpFABU(TopOpeBRepBuild_FaceBuilder&) const {}
#endif

//=================================================================================================

void TopOpeBRepBuild_Builder::GdumpEDBU(TopOpeBRepBuild_EdgeBuilder& /*ME*/) const
{
#ifdef OCCT_DEBUG
#endif
} // GdumpEDBU

//=================================================================================================

#ifdef OCCT_DEBUG
Standard_Boolean TopOpeBRepBuild_Builder::GtraceSPS(const Standard_Integer iS) const
#else
Standard_Boolean TopOpeBRepBuild_Builder::GtraceSPS(const Standard_Integer) const
#endif
{
  Standard_Boolean b = Standard_False;
#ifdef OCCT_DEBUG
  Standard_Integer ibid;
  b = GtraceSPS(myDataStructure->Shape(iS), ibid);
#endif
  return b;
}

//=================================================================================================

Standard_Boolean TopOpeBRepBuild_Builder::GtraceSPS(const Standard_Integer,
                                                    const Standard_Integer) const
{
  return Standard_False;
}

//=================================================================================================

#ifdef OCCT_DEBUG
Standard_Boolean TopOpeBRepBuild_Builder::GtraceSPS(const TopoShape& S) const
#else
Standard_Boolean TopOpeBRepBuild_Builder::GtraceSPS(const TopoShape&) const
#endif
{
  Standard_Boolean b = Standard_False;
#ifdef OCCT_DEBUG
  Standard_Integer iS;
  b = GtraceSPS(S, iS);
#endif
  return b;
}

//=================================================================================================

Standard_Boolean TopOpeBRepBuild_Builder::GtraceSPS(const TopoShape&, Standard_Integer& IS) const
{
  IS = 0;
  return Standard_False;
}

//=================================================================================================

#ifdef OCCT_DEBUG
Standard_Boolean TopOpeBRepBuild_Builder::GcheckNBOUNDS(const TopoShape& E)
#else
Standard_Boolean TopOpeBRepBuild_Builder::GcheckNBOUNDS(const TopoShape&)
#endif
{
  Standard_Boolean res = Standard_False;
#ifdef OCCT_DEBUG
  Standard_Integer             nf = 0, nr = 0;
  TopOpeBRepTool_ShapeExplorer ex(E, TopAbs_VERTEX);
  for (; ex.More(); ex.Next())
  {
    TopAbs_Orientation o = ex.Current().Orientation();
    if (o == TopAbs_FORWARD)
      nf++;
    if (o == TopAbs_REVERSED)
      nr++;
  }
  if (nf == 1 && nr == 1)
    res = Standard_True;
#endif
  return res;
}
