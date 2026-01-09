// Created on: 2013-09-16
// Copyright (c) 2013-2014 OPEN CASCADE SAS
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

#include <StdPrs_BRepFont.hxx>

#include <BRep_Tool.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <Font_FTLibrary.hxx>
#include <Font_FontMgr.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_Plane.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Line.hxx>
#include <GeomAPI.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomLib.hxx>
#include <gp_Pln.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_DataMapOfShapeSequenceOfShape.hxx>

#ifdef HAVE_FREETYPE
  #include <ft2build.h>
  #include FT_FREETYPE_H
#endif

IMPLEMENT_STANDARD_RTTIEXT(StdPrs_BRepFont, RefObject)

namespace
{
// pre-defined font rendering options
static const unsigned int      THE_FONT_SIZE      = 72;
static const unsigned int      THE_RESOLUTION_DPI = 4800;
static const FTFontParams THE_FONT_PARAMS(THE_FONT_SIZE, THE_RESOLUTION_DPI);

// compute scaling factor for specified font size
inline Standard_Real getScale(const Standard_Real theSize)
{
  return theSize / Standard_Real(THE_FONT_SIZE) * 72.0 / Standard_Real(THE_RESOLUTION_DPI);
}

#ifdef HAVE_FREETYPE
//! Auxiliary method to convert FT_Vector to Coords2d
static Coords2d readFTVec(const FT_Vector&    theVec,
                       const Standard_Real theScaleUnits,
                       const Standard_Real theWidthScaling = 1.0)
{
  return Coords2d(theScaleUnits * Standard_Real(theVec.x) * theWidthScaling / 64.0,
               theScaleUnits * Standard_Real(theVec.y) / 64.0);
}

//! Auxiliary method for classification wire theW2 with respect to wire theW1
static TopAbs_State classifyWW(const TopoWire& theW1,
                               const TopoWire& theW2,
                               const TopoFace& theF)
{
  TopAbs_State aRes = TopAbs_UNKNOWN;

  TopoFace aF = TopoDS::Face(theF.EmptyCopied());
  aF.Orientation(TopAbs_FORWARD);
  ShapeBuilder aB;
  aB.Add(aF, theW1);
  BRepTopAdaptor_FClass2d aClass2d(aF, ::Precision1::PConfusion());
  for (TopoDS_Iterator anEdgeIter(theW2); anEdgeIter.More(); anEdgeIter.Next())
  {
    const TopoEdge&   anEdge  = TopoDS::Edge(anEdgeIter.Value());
    Standard_Real        aPFirst = 0.0, aPLast = 0.0;
    Handle(GeomCurve2d) aCurve2d = BRepInspector::CurveOnSurface(anEdge, theF, aPFirst, aPLast);
    if (aCurve2d.IsNull())
    {
      continue;
    }

    gp_Pnt2d     aPnt2d = aCurve2d->Value((aPFirst + aPLast) / 2.0);
    TopAbs_State aState = aClass2d.Perform(aPnt2d, Standard_False);
    if (aState == TopAbs_OUT || aState == TopAbs_IN)
    {
      if (aRes == TopAbs_UNKNOWN)
      {
        aRes = aState;
      }
      else if (aRes != aState)
      {
        return TopAbs_UNKNOWN;
      }
    }
  }
  return aRes;
}
#endif
} // namespace

//=================================================================================================

StdPrs_BRepFont::StdPrs_BRepFont()
    : myPrecision(Precision1::Confusion()),
      myScaleUnits(1.0),
      myIsCompositeCurve(Standard_False),
      my3Poles(1, 3),
      my4Poles(1, 4)
{
  myFTFont = new Font_FTFont();
  init();
}

//=================================================================================================

void StdPrs_BRepFont::init()
{
  mySurface                              = new GeomPlane(gp_Pln(gp1::XOY()));
  myCurve2dAdaptor                       = new Geom2dAdaptor_Curve();
  Handle(SurfaceAdaptor) aSurfAdaptor = new GeomAdaptor_Surface(mySurface);
  myCurvOnSurf.Load(aSurfAdaptor);
}

//=================================================================================================

StdPrs_BRepFont::StdPrs_BRepFont(const NCollection_String& theFontPath,
                                 const Standard_Real       theSize,
                                 const Standard_Integer    theFaceId)
    : myPrecision(Precision1::Confusion()),
      myScaleUnits(1.0),
      myIsCompositeCurve(Standard_False),
      my3Poles(1, 3),
      my4Poles(1, 4)
{
  init();
  if (theSize <= myPrecision * 100.0)
  {
    return;
  }

  myScaleUnits = getScale(theSize);
  myFTFont     = new Font_FTFont();
  myFTFont->Init(theFontPath.ToCString(), THE_FONT_PARAMS, theFaceId);
}

//=================================================================================================

StdPrs_BRepFont::StdPrs_BRepFont(const NCollection_String& theFontName,
                                 const Font_FontAspect     theFontAspect,
                                 const Standard_Real       theSize,
                                 const Font_StrictLevel    theStrictLevel)
    : myPrecision(Precision1::Confusion()),
      myScaleUnits(1.0),
      myIsCompositeCurve(Standard_False),
      my3Poles(1, 3),
      my4Poles(1, 4)
{
  init();
  if (theSize <= myPrecision * 100.0)
  {
    return;
  }

  myScaleUnits = getScale(theSize);
  myFTFont     = new Font_FTFont();
  myFTFont->FindAndInit(theFontName.ToCString(), theFontAspect, THE_FONT_PARAMS, theStrictLevel);
}

//=================================================================================================

void StdPrs_BRepFont::Release()
{
  myCache.Clear();
  myFTFont->Release();
}

//=================================================================================================

Handle(StdPrs_BRepFont) StdPrs_BRepFont::FindAndCreate(const AsciiString1& theFontName,
                                                       const Font_FontAspect          theFontAspect,
                                                       const Standard_Real            theSize,
                                                       const Font_StrictLevel theStrictLevel)
{
  Handle(StdPrs_BRepFont) aFont = new StdPrs_BRepFont();

  if (aFont->FindAndInit(theFontName, theFontAspect, theSize, theStrictLevel))
    return aFont;

  return Handle(StdPrs_BRepFont)();
}

//=================================================================================================

void StdPrs_BRepFont::SetCompositeCurveMode(const Standard_Boolean theToConcatenate)
{
  if (myIsCompositeCurve != theToConcatenate)
  {
    myIsCompositeCurve = theToConcatenate;
    myCache.Clear();
  }
}

//=================================================================================================

bool StdPrs_BRepFont::Init(const NCollection_String& theFontPath,
                           const Standard_Real       theSize,
                           const Standard_Integer    theFaceId)
{
  if (theSize <= myPrecision * 100.0)
  {
    return false;
  }

  myScaleUnits = getScale(theSize);
  myCache.Clear();
  return myFTFont->Init(theFontPath.ToCString(), THE_FONT_PARAMS, theFaceId);
}

//=================================================================================================

bool StdPrs_BRepFont::FindAndInit(const AsciiString1& theFontName,
                                  const Font_FontAspect          theFontAspect,
                                  const Standard_Real            theSize,
                                  const Font_StrictLevel         theStrictLevel)
{
  if (theSize <= myPrecision * 100.0)
  {
    return false;
  }

  myScaleUnits = getScale(theSize);
  myCache.Clear();
  return myFTFont->FindAndInit(theFontName.ToCString(),
                               theFontAspect,
                               THE_FONT_PARAMS,
                               theStrictLevel);
}

//=================================================================================================

TopoShape StdPrs_BRepFont::RenderGlyph(const Standard_Utf32Char& theChar)
{
  TopoShape           aShape;
  Standard_Mutex::Sentry aSentry(myMutex);
  renderGlyph(theChar, aShape);
  return aShape;
}

//=================================================================================================

bool StdPrs_BRepFont::to3d(const Handle(GeomCurve2d)& theCurve2d,
                           const GeomAbs_Shape         theContinuity,
                           Handle(GeomCurve3d)&         theCurve3d)
{
  Standard_Real aMaxDeviation   = 0.0;
  Standard_Real anAverDeviation = 0.0;
  myCurve2dAdaptor->Load(theCurve2d);
  const Handle(Adaptor2d_Curve2d)& aCurve = myCurve2dAdaptor; // to avoid ambiguity
  myCurvOnSurf.Load(aCurve);
  GeomLib1::BuildCurve3d(myPrecision,
                        myCurvOnSurf,
                        myCurve2dAdaptor->FirstParameter(),
                        myCurve2dAdaptor->LastParameter(),
                        theCurve3d,
                        aMaxDeviation,
                        anAverDeviation,
                        theContinuity);
  return !theCurve3d.IsNull();
}

//=================================================================================================

Standard_Boolean StdPrs_BRepFont::buildFaces(const NCollection_Sequence<TopoWire>& theWires,
                                             TopoShape&                            theRes)
{
#ifdef HAVE_FREETYPE
  // classify wires
  NCollection_DataMap<TopoShape, NCollection_Sequence<TopoWire>, ShapeHasher>
                                 aMapOutInts;
  TopTools_DataMapOfShapeInteger aMapNbOuts;
  TopoFace                    aF;
  myBuilder.MakeFace(aF, mySurface, myPrecision);
  Standard_Integer aWireIter1Index = 1;
  for (NCollection_Sequence<TopoWire>::Iterator aWireIter1(theWires); aWireIter1.More();
       ++aWireIter1Index, aWireIter1.Next())
  {
    const TopoWire& aW1 = aWireIter1.Value();
    if (!aMapNbOuts.IsBound(aW1))
    {
      const Standard_Integer aNbOuts = 0;
      aMapNbOuts.Bind(aW1, aNbOuts);
    }

    NCollection_Sequence<TopoWire>* anIntWs =
      aMapOutInts.Bound(aW1, NCollection_Sequence<TopoWire>());
    Standard_Integer aWireIter2Index = 1;
    for (NCollection_Sequence<TopoWire>::Iterator aWireIter2(theWires); aWireIter2.More();
         ++aWireIter2Index, aWireIter2.Next())
    {
      if (aWireIter1Index == aWireIter2Index)
      {
        continue;
      }

      const TopoWire& aW2    = aWireIter2.Value();
      const TopAbs_State aClass = classifyWW(aW1, aW2, aF);
      if (aClass == TopAbs_IN)
      {
        anIntWs->Append(aW2);
        if (Standard_Integer* aNbOutsPtr = aMapNbOuts.ChangeSeek(aW2))
        {
          ++(*aNbOutsPtr);
        }
        else
        {
          const Standard_Integer aNbOuts = 1;
          aMapNbOuts.Bind(aW2, aNbOuts);
        }
      }
    }
  }

  // check out wires and remove "not out" wires from maps
  for (TopTools_DataMapIteratorOfDataMapOfShapeInteger anOutIter(aMapNbOuts); anOutIter.More();
       anOutIter.Next())
  {
    const Standard_Integer aTmp = anOutIter.Value() % 2;
    if (aTmp > 0)
    {
      // not out wire
      aMapOutInts.UnBind(anOutIter.Key1());
    }
  }

  // create faces for out wires
  TopTools_MapOfShape anUsedShapes;
  TopoCompound     aFaceComp;
  myBuilder.MakeCompound(aFaceComp);
  for (; !aMapOutInts.IsEmpty();)
  {
    // find out wire with max number of outs
    TopoShape     aW;
    Standard_Integer aMaxNbOuts = -1;
    for (NCollection_DataMap<TopoShape,
                             NCollection_Sequence<TopoWire>,
                             ShapeHasher>::Iterator itMOI(aMapOutInts);
         itMOI.More();
         itMOI.Next())
    {
      const TopoShape&    aKey    = itMOI.Key1();
      const Standard_Integer aNbOuts = aMapNbOuts.Find(aKey);
      if (aNbOuts > aMaxNbOuts)
      {
        aMaxNbOuts = aNbOuts;
        aW         = aKey;
      }
    }

    // create face for selected wire
    TopoFace aNewF;
    myBuilder.MakeFace(aNewF, mySurface, myPrecision);
    myBuilder.Add(aNewF, aW);
    anUsedShapes.Add(aW);
    const NCollection_Sequence<TopoWire>& anIns = aMapOutInts.Find(aW);
    for (NCollection_Sequence<TopoWire>::Iterator aWireIter(anIns); aWireIter.More();
         aWireIter.Next())
    {
      TopoWire aWin = aWireIter.Value();
      if (anUsedShapes.Contains(aWin))
      {
        continue;
      }

      aWin.Reverse();
      myBuilder.Add(aNewF, aWin);
      anUsedShapes.Add(aWin);
    }

    myBuilder.Add(aFaceComp, aNewF);
    aMapOutInts.UnBind(aW);
  }

  if (aFaceComp.NbChildren() == 0)
  {
    return Standard_False;
  }

  if (aFaceComp.NbChildren() == 1)
  {
    theRes = TopoDS_Iterator(aFaceComp).Value();
  }
  else
  {
    theRes = aFaceComp;
  }
  return Standard_True;
#else
  (void)theWires;
  (void)theRes;
  return Standard_False;
#endif
}

//=================================================================================================

Standard_Boolean StdPrs_BRepFont::renderGlyph(const Standard_Utf32Char theChar,
                                              TopoShape&            theShape)
{
  theShape.Nullify();
#ifdef HAVE_FREETYPE
  const FT_Outline* anOutline = myFTFont->renderGlyphOutline(theChar);
  if (!anOutline)
  {
    return Standard_False;
  }
  else if (myCache.Find(theChar, theShape))
  {
    return !theShape.IsNull();
  }

  if (!anOutline->n_contours)
    return Standard_False;

  TopLoc_Location                   aLoc;
  NCollection_Sequence<TopoWire> aWires;
  TopoCompound                   aFaceCompDraft;

  // Get orientation is useless since it doesn't retrieve any in-font information and just computes
  // orientation. Because it fails in some cases - leave this to ShapeFix1.
  // const FT_Orientation anOrient = FT_Outline_Get_Orientation (&anOutline);
  for (short aContour = 0, aStartIndex = 0; aContour < anOutline->n_contours; ++aContour)
  {
    const FT_Vector* aPntList   = &anOutline->points[aStartIndex];
    const auto*      aTags      = &anOutline->tags[aStartIndex];
    const short      anEndIndex = anOutline->contours[aContour];
    const short      aPntsNb    = (anEndIndex - aStartIndex) + 1;
    aStartIndex                 = anEndIndex + 1;
    if (aPntsNb < 3 && !myFTFont->IsSingleStrokeFont())
    {
      // closed contour can not be constructed from < 3 points
      continue;
    }

    BRepBuilderAPI_MakeWire aWireMaker;

    Coords2d aPntPrev;
    Coords2d aPntCurr = readFTVec(aPntList[aPntsNb - 1], myScaleUnits, myFTFont->WidthScaling());
    Coords2d aPntNext = readFTVec(aPntList[0], myScaleUnits, myFTFont->WidthScaling());

    bool isLineSeg =
      !myFTFont->IsSingleStrokeFont() && FT_CURVE_TAG(aTags[aPntsNb - 1]) == FT_Curve_Tag_On;
    Coords2d aPntLine1 = aPntCurr;

    // see http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
    // for a full description of FreeType tags.
    for (short aPntId = 0; aPntId < aPntsNb; ++aPntId)
    {
      aPntPrev = aPntCurr;
      aPntCurr = aPntNext;
      aPntNext =
        readFTVec(aPntList[(aPntId + 1) % aPntsNb], myScaleUnits, myFTFont->WidthScaling());

      // process tags
      if (FT_CURVE_TAG(aTags[aPntId]) == FT_Curve_Tag_On)
      {
        if (!isLineSeg)
        {
          aPntLine1 = aPntCurr;
          isLineSeg = true;
          continue;
        }

        const Coords2d         aDirVec = aPntCurr - aPntLine1;
        const Standard_Real aLen    = aDirVec.Modulus();
        if (aLen <= myPrecision)
        {
          aPntLine1 = aPntCurr;
          isLineSeg = true;
          continue;
        }

        if (myIsCompositeCurve)
        {
          Handle(Geom2d_TrimmedCurve) aLine =
            GCE2d_MakeSegment(gp_Pnt2d(aPntLine1), gp_Pnt2d(aPntCurr));
          myConcatMaker.Add(aLine, myPrecision);
        }
        else
        {
          Handle(GeomCurve3d)  aCurve3d;
          Handle(Geom2d_Line) aCurve2d = new Geom2d_Line(gp_Pnt2d(aPntLine1), gp_Dir2d(aDirVec));
          if (to3d(aCurve2d, GeomAbs_C1, aCurve3d))
          {
            TopoEdge anEdge = BRepLib_MakeEdge(aCurve3d, 0.0, aLen);
            myBuilder.UpdateEdge(anEdge, aCurve2d, mySurface, aLoc, myPrecision);
            aWireMaker.Add(anEdge);
          }
        }
        aPntLine1 = aPntCurr;
      }
      else if (FT_CURVE_TAG(aTags[aPntId]) == FT_Curve_Tag_Conic)
      {
        isLineSeg       = false;
        Coords2d aPntPrev2 = aPntPrev;
        Coords2d aPntNext2 = aPntNext;

        // previous point is either the real previous point (an "on" point),
        // or the midpoint between the current one and the previous "conic off" point
        if (FT_CURVE_TAG(aTags[(aPntId - 1 + aPntsNb) % aPntsNb]) == FT_Curve_Tag_Conic)
        {
          aPntPrev2 = (aPntCurr + aPntPrev) * 0.5;
        }

        // next point is either the real next point or the midpoint
        if (FT_CURVE_TAG(aTags[(aPntId + 1) % aPntsNb]) == FT_Curve_Tag_Conic)
        {
          aPntNext2 = (aPntCurr + aPntNext) * 0.5;
        }

        my3Poles.SetValue(1, aPntPrev2);
        my3Poles.SetValue(2, aPntCurr);
        my3Poles.SetValue(3, aPntNext2);
        Handle(Geom2d_BezierCurve) aBezierArc = new Geom2d_BezierCurve(my3Poles);
        if (myIsCompositeCurve)
        {
          myConcatMaker.Add(aBezierArc, myPrecision);
        }
        else
        {
          Handle(GeomCurve3d) aCurve3d;
          if (to3d(aBezierArc, GeomAbs_C1, aCurve3d))
          {
            TopoEdge anEdge = BRepLib_MakeEdge(aCurve3d);
            myBuilder.UpdateEdge(anEdge, aBezierArc, mySurface, aLoc, myPrecision);
            aWireMaker.Add(anEdge);
          }
        }
      }
      else if (FT_CURVE_TAG(aTags[aPntId]) == FT_Curve_Tag_Cubic
               && FT_CURVE_TAG(aTags[(aPntId + 1) % aPntsNb]) == FT_Curve_Tag_Cubic)
      {
        isLineSeg = false;
        my4Poles.SetValue(1, aPntPrev);
        my4Poles.SetValue(2, aPntCurr);
        my4Poles.SetValue(3, aPntNext);
        my4Poles.SetValue(
          4,
          gp_Pnt2d(
            readFTVec(aPntList[(aPntId + 2) % aPntsNb], myScaleUnits, myFTFont->WidthScaling())));
        Handle(Geom2d_BezierCurve) aBezier = new Geom2d_BezierCurve(my4Poles);
        if (myIsCompositeCurve)
        {
          myConcatMaker.Add(aBezier, myPrecision);
        }
        else
        {
          Handle(GeomCurve3d) aCurve3d;
          if (to3d(aBezier, GeomAbs_C1, aCurve3d))
          {
            TopoEdge anEdge = BRepLib_MakeEdge(aCurve3d);
            myBuilder.UpdateEdge(anEdge, aBezier, mySurface, aLoc, myPrecision);
            aWireMaker.Add(anEdge);
          }
        }
      }
    }

    if (myIsCompositeCurve)
    {
      Handle(Geom2d_BSplineCurve) aDraft2d = myConcatMaker.BSplineCurve();
      if (aDraft2d.IsNull())
      {
        continue;
      }

      const gp_Pnt2d aFirstPnt = aDraft2d->StartPoint();
      const gp_Pnt2d aLastPnt  = aDraft2d->EndPoint();
      if (!myFTFont->IsSingleStrokeFont() && !aFirstPnt.IsEqual(aLastPnt, myPrecision))
      {
        Handle(Geom2d_TrimmedCurve) aLine = GCE2d_MakeSegment(aLastPnt, aFirstPnt);
        myConcatMaker.Add(aLine, myPrecision);
      }

      Handle(Geom2d_BSplineCurve) aCurve2d = myConcatMaker.BSplineCurve();
      Handle(GeomCurve3d)          aCurve3d;
      if (to3d(aCurve2d, GeomAbs_C0, aCurve3d))
      {
        TopoEdge anEdge = BRepLib_MakeEdge(aCurve3d);
        myBuilder.UpdateEdge(anEdge, aCurve2d, mySurface, aLoc, myPrecision);
        aWireMaker.Add(anEdge);
      }
      myConcatMaker.Clear();
    }
    else
    {
      if (!aWireMaker.IsDone())
      {
        continue;
      }

      TopoVertex aFirstV, aLastV;
      TopExp1::Vertices(aWireMaker.Wire(), aFirstV, aLastV);
      Point3d aFirstPoint = BRepInspector::Pnt(aFirstV);
      Point3d aLastPoint  = BRepInspector::Pnt(aLastV);
      if (!myFTFont->IsSingleStrokeFont() && !aFirstPoint.IsEqual(aLastPoint, myPrecision))
      {
        aWireMaker.Add(BRepLib_MakeEdge(aFirstV, aLastV));
      }
    }

    if (!aWireMaker.IsDone())
    {
      continue;
    }

    TopoWire aWireDraft = aWireMaker.Wire();
    if (!myFTFont->IsSingleStrokeFont())
    {
      // collect all wires and set CCW orientation
      TopoFace aFace;
      myBuilder.MakeFace(aFace, mySurface, myPrecision);
      myBuilder.Add(aFace, aWireDraft);
      BRepTopAdaptor_FClass2d aClass2d(aFace, ::Precision1::PConfusion());
      TopAbs_State            aState = aClass2d.PerformInfinitePoint();
      if (aState != TopAbs_OUT)
      {
        // need to reverse
        aWireDraft.Reverse();
      }
      aWires.Append(aWireDraft);
    }
    else
    {
      if (aFaceCompDraft.IsNull())
      {
        myBuilder.MakeCompound(aFaceCompDraft);
      }
      myBuilder.Add(aFaceCompDraft, aWireDraft);
    }
  }

  if (!aWires.IsEmpty())
  {
    buildFaces(aWires, theShape);
  }
  else if (!aFaceCompDraft.IsNull())
  {
    theShape = aFaceCompDraft;
  }
#else
  (void)theChar;
#endif
  myCache.Bind(theChar, theShape);
  return !theShape.IsNull();
}
