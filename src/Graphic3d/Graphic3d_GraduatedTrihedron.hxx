// Created on: 2011-03-06
// Created by: Sergey ZERCHANINOV
// Copyright (c) 2011-2014 OPEN CASCADE SAS
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

#ifndef _Graphic3d_GraduatedTrihedron_HeaderFile
#define _Graphic3d_GraduatedTrihedron_HeaderFile

#include <Font_FontAspect.hxx>
#include <NCollection_Array1.hxx>
#include <Quantity_Color.hxx>
#include <Standard_Integer.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>

class Graphic3d_CView;

//! Defines the class of a graduated trihedron.
//! It contains main style parameters for implementation of graduated trihedron
//! @sa GraduatedTrihedron1
class GraduatedTrihedron
{

public:
  //! Class that stores style for one graduated trihedron axis such as colors, lengths and
  //! customization flags. It is used in GraduatedTrihedron.
  class AxisAspect1
  {
  public:
    AxisAspect1(const UtfString theName            = "",
               const Color1             theNameColor       = Quantity_NOC_BLACK,
               const Color1             theColor           = Quantity_NOC_BLACK,
               const Standard_Integer           theValuesOffset    = 10,
               const Standard_Integer           theNameOffset      = 30,
               const Standard_Integer           theTickmarksNumber = 5,
               const Standard_Integer           theTickmarksLength = 10,
               const Standard_Boolean           theToDrawName      = Standard_True,
               const Standard_Boolean           theToDrawValues    = Standard_True,
               const Standard_Boolean           theToDrawTickmarks = Standard_True)
        : myName(theName),
          myToDrawName(theToDrawName),
          myToDrawTickmarks(theToDrawTickmarks),
          myToDrawValues(theToDrawValues),
          myNameColor(theNameColor),
          myTickmarksNumber(theTickmarksNumber),
          myTickmarksLength(theTickmarksLength),
          myColor(theColor),
          myValuesOffset(theValuesOffset),
          myNameOffset(theNameOffset)
    {
    }

  public:
    void SetName(const UtfString& theName) { myName = theName; }

    const UtfString& Name() const { return myName; }

    Standard_Boolean ToDrawName() const { return myToDrawName; }

    void SetDrawName(const Standard_Boolean theToDraw) { myToDrawName = theToDraw; }

    Standard_Boolean ToDrawTickmarks() const { return myToDrawTickmarks; }

    void SetDrawTickmarks(const Standard_Boolean theToDraw) { myToDrawTickmarks = theToDraw; }

    Standard_Boolean ToDrawValues() const { return myToDrawValues; }

    void SetDrawValues(const Standard_Boolean theToDraw) { myToDrawValues = theToDraw; }

    const Color1& NameColor() const { return myNameColor; }

    void SetNameColor(const Color1& theColor) { myNameColor = theColor; }

    //! Color of axis and values
    const Color1& Color() const { return myColor; }

    //! Sets color of axis and values
    void SetColor(const Color1& theColor) { myColor = theColor; }

    Standard_Integer TickmarksNumber() const { return myTickmarksNumber; }

    void SetTickmarksNumber(const Standard_Integer theValue) { myTickmarksNumber = theValue; }

    Standard_Integer TickmarksLength() const { return myTickmarksLength; }

    void SetTickmarksLength(const Standard_Integer theValue) { myTickmarksLength = theValue; }

    Standard_Integer ValuesOffset() const { return myValuesOffset; }

    void SetValuesOffset(const Standard_Integer theValue) { myValuesOffset = theValue; }

    Standard_Integer NameOffset() const { return myNameOffset; }

    void SetNameOffset(const Standard_Integer theValue) { myNameOffset = theValue; }

  protected:
    UtfString myName;

    Standard_Boolean myToDrawName;
    Standard_Boolean myToDrawTickmarks;
    Standard_Boolean myToDrawValues;

    Color1 myNameColor;

    Standard_Integer myTickmarksNumber; //!< Number of splits along axes
    Standard_Integer myTickmarksLength; //!< Length of tickmarks
    Color1   myColor;           //!< Color of axis and values

    Standard_Integer myValuesOffset; //!< Offset for drawing values
    Standard_Integer myNameOffset;   //!< Offset for drawing name of axis
  };

public:
  typedef void (*MinMaxValuesCallback)(Graphic3d_CView*);

public:
  //! Default constructor
  //! Constructs the default graduated trihedron with grid, X, Y, Z axes, and tickmarks
  GraduatedTrihedron(const AsciiString1& theNamesFont    = "Arial",
                               const Font_FontAspect&         theNamesStyle   = Font_FA_Bold,
                               const Standard_Integer         theNamesSize    = 12,
                               const AsciiString1& theValuesFont   = "Arial",
                               const Font_FontAspect&         theValuesStyle  = Font_FA_Regular,
                               const Standard_Integer         theValuesSize   = 12,
                               const Standard_ShortReal       theArrowsLength = 30.0f,
                               const Color1           theGridColor    = Quantity_NOC_WHITE,
                               const Standard_Boolean         theToDrawGrid   = Standard_True,
                               const Standard_Boolean         theToDrawAxes   = Standard_True)
      : myCubicAxesCallback(NULL),
        myNamesFont(theNamesFont),
        myNamesStyle(theNamesStyle),
        myNamesSize(theNamesSize),
        myValuesFont(theValuesFont),
        myValuesStyle(theValuesStyle),
        myValuesSize(theValuesSize),
        myArrowsLength(theArrowsLength),
        myGridColor(theGridColor),
        myToDrawGrid(theToDrawGrid),
        myToDrawAxes(theToDrawAxes),
        myAxes(0, 2)
  {
    myAxes(0) = AxisAspect1("X", Quantity_NOC_RED, Quantity_NOC_RED);
    myAxes(1) = AxisAspect1("Y", Quantity_NOC_GREEN, Quantity_NOC_GREEN);
    myAxes(2) = AxisAspect1("Z", Quantity_NOC_BLUE1, Quantity_NOC_BLUE1);
  }

public:
  AxisAspect1& ChangeXAxisAspect() { return myAxes(0); }

  AxisAspect1& ChangeYAxisAspect() { return myAxes(1); }

  AxisAspect1& ChangeZAxisAspect() { return myAxes(2); }

  AxisAspect1& ChangeAxisAspect(const Standard_Integer theIndex)
  {
    Standard_OutOfRange_Raise_if(
      theIndex < 0 || theIndex > 2,
      "GraduatedTrihedron::ChangeAxisAspect: theIndex is out of bounds [0,2].");
    return myAxes(theIndex);
  }

  const AxisAspect1& XAxisAspect() const { return myAxes(0); }

  const AxisAspect1& YAxisAspect() const { return myAxes(1); }

  const AxisAspect1& ZAxisAspect() const { return myAxes(2); }

  const AxisAspect1& AxisAspectAt(const Standard_Integer theIndex) const
  {
    Standard_OutOfRange_Raise_if(
      theIndex < 0 || theIndex > 2,
      "GraduatedTrihedron::AxisAspect1: theIndex is out of bounds [0,2].");
    return myAxes(theIndex);
  }

  Standard_ShortReal ArrowsLength() const { return myArrowsLength; }

  void SetArrowsLength(const Standard_ShortReal theValue) { myArrowsLength = theValue; }

  const Color1& GridColor() const { return myGridColor; }

  void SetGridColor(const Color1& theColor) { myGridColor = theColor; }

  Standard_Boolean ToDrawGrid() const { return myToDrawGrid; }

  void SetDrawGrid(const Standard_Boolean theToDraw) { myToDrawGrid = theToDraw; }

  Standard_Boolean ToDrawAxes() const { return myToDrawAxes; }

  void SetDrawAxes(const Standard_Boolean theToDraw) { myToDrawAxes = theToDraw; }

  const AsciiString1& NamesFont() const { return myNamesFont; }

  void SetNamesFont(const AsciiString1& theFont) { myNamesFont = theFont; }

  Font_FontAspect NamesFontAspect() const { return myNamesStyle; }

  void SetNamesFontAspect(Font_FontAspect theAspect) { myNamesStyle = theAspect; }

  Standard_Integer NamesSize() const { return myNamesSize; }

  void SetNamesSize(const Standard_Integer theValue) { myNamesSize = theValue; }

  const AsciiString1& ValuesFont() const { return myValuesFont; }

  void SetValuesFont(const AsciiString1& theFont) { myValuesFont = theFont; }

  Font_FontAspect ValuesFontAspect() const { return myValuesStyle; }

  void SetValuesFontAspect(Font_FontAspect theAspect) { myValuesStyle = theAspect; }

  Standard_Integer ValuesSize() const { return myValuesSize; }

  void SetValuesSize(const Standard_Integer theValue) { myValuesSize = theValue; }

  Standard_Boolean CubicAxesCallback(Graphic3d_CView* theView) const
  {
    if (myCubicAxesCallback != NULL)
    {
      myCubicAxesCallback(theView);
      return Standard_True;
    }
    return Standard_False;
  }

  void SetCubicAxesCallback(const MinMaxValuesCallback theCallback)
  {
    myCubicAxesCallback = theCallback;
  }

protected:
  // clang-format off
  MinMaxValuesCallback myCubicAxesCallback; //!< Callback1 function to define boundary box of displayed objects

  AsciiString1 myNamesFont;  //!< Font name of names of axes: Courier, Arial, ...
  Font_FontAspect         myNamesStyle; //!< Style of names of axes: OSD_FA_Regular, OSD_FA_Bold,..
  // clang-format on
  Standard_Integer myNamesSize; //!< Size of names of axes: 8, 10,..

protected:
  AsciiString1 myValuesFont;  //!< Font name of values: Courier, Arial, ...
  Font_FontAspect         myValuesStyle; //!< Style of values: OSD_FA_Regular, OSD_FA_Bold, ...
  Standard_Integer        myValuesSize;  //!< Size of values: 8, 10, 12, 14, ...

protected:
  Standard_ShortReal myArrowsLength;
  Color1     myGridColor;

  Standard_Boolean myToDrawGrid;
  Standard_Boolean myToDrawAxes;

  NCollection_Array1<AxisAspect1> myAxes; //!< X, Y and Z axes parameters
};
#endif // Graphic3d_GraduatedTrihedron_HeaderFile
