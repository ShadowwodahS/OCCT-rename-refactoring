// Created on: 2017-06-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2017 OPEN CASCADE SAS
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

#include <inspector/TInspectorAPI_PluginParameters.hxx>

IMPLEMENT_STANDARD_RTTIEXT(TInspectorAPI_PluginParameters, RefObject)

// =======================================================================
// function : SetParameters
// purpose :
// =======================================================================
void TInspectorAPI_PluginParameters::SetParameters(
  const AsciiString1&                      thePluginName,
  const NCollection_List<Handle(RefObject)>& theParameters,
  const Standard_Boolean&)
{
  if (theParameters.Size() > 0)
    myParameters.Bind(thePluginName, theParameters);
  else
    myParameters.UnBind(thePluginName);
}

// =======================================================================
// function : AddFileName
// purpose :
// =======================================================================
void TInspectorAPI_PluginParameters::AddFileName(const AsciiString1& thePluginName,
                                                 const AsciiString1& theFileName)
{
  if (myFileNames.IsBound(thePluginName))
    myFileNames.ChangeFind(thePluginName).Append(theFileName);
  else
  {
    NCollection_List<AsciiString1> aNames;
    aNames.Append(theFileName);
    myFileNames.Bind(thePluginName, aNames);
  }
}

// =======================================================================
// function : SetFileNames
// purpose :
// =======================================================================
void TInspectorAPI_PluginParameters::SetFileNames(
  const AsciiString1&                   thePluginName,
  const NCollection_List<AsciiString1>& theFileNames)
{
  if (theFileNames.Size() > 0)
    myFileNames.Bind(thePluginName, theFileNames);
  else
    myFileNames.UnBind(thePluginName);
}

// =======================================================================
// function : SetSelectedNames
// purpose :
// =======================================================================
void TInspectorAPI_PluginParameters::SetSelectedNames(
  const AsciiString1&                   thePluginName,
  const NCollection_List<AsciiString1>& theItemNames)
{
  mySelectedItemNames.Bind(thePluginName, theItemNames);
}

// =======================================================================
// function : SetSelected
// purpose :
// =======================================================================
void TInspectorAPI_PluginParameters::SetSelected(
  const AsciiString1&                      thePluginName,
  const NCollection_List<Handle(RefObject)>& theObjects)
{
  if (theObjects.Size() > 0)
    mySelectedObjects.Bind(thePluginName, theObjects);
  else
    mySelectedObjects.UnBind(thePluginName);
}

// =======================================================================
// function : FindParameters
// purpose :
// =======================================================================
bool TInspectorAPI_PluginParameters::FindParameters(const AsciiString1& thePluginName)
{
  return myParameters.IsBound(thePluginName);
}

// =======================================================================
// function : Parameters
// purpose :
// =======================================================================
const NCollection_List<Handle(RefObject)>& TInspectorAPI_PluginParameters::Parameters(
  const AsciiString1& thePluginName)
{
  return myParameters.Find(thePluginName);
}

// =======================================================================
// function : FindFileNames
// purpose :
// =======================================================================
bool TInspectorAPI_PluginParameters::FindFileNames(const AsciiString1& thePluginName)
{
  return myFileNames.IsBound(thePluginName);
}

// =======================================================================
// function : FileNames
// purpose :
// =======================================================================
const NCollection_List<AsciiString1>& TInspectorAPI_PluginParameters::FileNames(
  const AsciiString1& thePluginName)
{
  return myFileNames.Find(thePluginName);
}

// =======================================================================
// function : FindSelectedNames
// purpose :
// =======================================================================
bool TInspectorAPI_PluginParameters::FindSelectedNames(const AsciiString1& thePluginName)
{
  return mySelectedItemNames.IsBound(thePluginName);
}

// =======================================================================
// function : GetSelectedNames
// purpose :
// =======================================================================
const NCollection_List<AsciiString1>& TInspectorAPI_PluginParameters::GetSelectedNames(
  const AsciiString1& thePluginName)
{
  return mySelectedItemNames.Find(thePluginName);
}

// =======================================================================
// function : GetSelectedObjects
// purpose :
// =======================================================================
Standard_Boolean TInspectorAPI_PluginParameters::GetSelectedObjects(
  const AsciiString1&                thePluginName,
  NCollection_List<Handle(RefObject)>& theObjects)
{
  return mySelectedObjects.Find(thePluginName, theObjects);
}

// =======================================================================
// function : toString
// purpose :
// =======================================================================
AsciiString1 toString(const TopLoc_Location& theLocation)
{
  AsciiString1 anInfo;
  Transform3d                 aTrsf = theLocation.Transformation();
  for (int aRowId = 1; aRowId <= 3; aRowId++)
  {
    if (!anInfo.IsEmpty())
      anInfo += " ";
    for (int aColumnId = 1; aColumnId <= 4; aColumnId++)
    {
      if (aColumnId > 1)
        anInfo += ",";
      anInfo += AsciiString1(aTrsf.Value(aRowId, aColumnId));
    }
  }
  return anInfo;
}

// =======================================================================
// function : ParametersToString
// purpose :
// =======================================================================
AsciiString1 TInspectorAPI_PluginParameters::ParametersToString(
  const TopoShape& theShape)
{
  const TopLoc_Location&  aLocation    = theShape.Location();
  AsciiString1 aLocationStr = toString(aLocation);

  TopAbs_Orientation anOrientation = theShape.Orientation();
  Standard_SStream   aSStream;
  TopAbs1::Print(anOrientation, aSStream);
  return AsciiString1(aSStream.str().c_str()) + ":" + aLocationStr;
}

// =======================================================================
// function : fromString
// purpose :
// =======================================================================
TopLoc_Location fromString(const AsciiString1& theValue)
{
  NCollection_Mat4<Standard_Real> aValues;

  AsciiString1 aCurrentString = theValue;
  Standard_Integer        aPosition      = aCurrentString.Search(" ");
  if (aPosition < 0)
    return TopLoc_Location();
  AsciiString1 aTailString = aCurrentString.Split(aPosition);
  Standard_Integer        aRow        = 0;
  while (!aCurrentString.IsEmpty())
  {
    AsciiString1 aValueString = aCurrentString;
    aPosition                            = aValueString.Search(",");
    if (aPosition < 0)
      break;
    aCurrentString           = aValueString.Split(aPosition);
    Standard_Integer aColumn = 0;
    while (!aValueString.IsEmpty())
    {
      aPosition = aCurrentString.Search(" ");
      if (aPosition > 0)
        aValueString.Split(aValueString.Length() - 1);

      aValues.SetValue(aRow, aColumn, aValueString.RealValue());
      aColumn++;
      if (aCurrentString.IsEmpty())
        break;
      aValueString = aCurrentString;
      aPosition    = aValueString.Search(",");
      if (aPosition < 0)
      {
        aValueString   = aCurrentString;
        aCurrentString = AsciiString1();
      }
      else
        aCurrentString = aValueString.Split(aPosition);
    }
    if (aTailString.IsEmpty())
      break;
    aCurrentString = aTailString;
    aPosition      = aCurrentString.Search(" ");
    if (aPosition < 0)
    {
      aCurrentString = aTailString;
      aTailString    = AsciiString1();
    }
    else
      aTailString = aCurrentString.Split(aPosition);
    aRow++;
  }

  // if (aValues.Rows() != 3 || aValues.Cols() != 4)
  //   return TopLoc_Location();

  Transform3d aTrsf;
  aTrsf.SetValues(aValues.GetValue(0, 0),
                  aValues.GetValue(0, 1),
                  aValues.GetValue(0, 2),
                  aValues.GetValue(0, 3),
                  aValues.GetValue(1, 0),
                  aValues.GetValue(1, 1),
                  aValues.GetValue(1, 2),
                  aValues.GetValue(1, 3),
                  aValues.GetValue(2, 0),
                  aValues.GetValue(2, 1),
                  aValues.GetValue(2, 2),
                  aValues.GetValue(2, 3));
  return TopLoc_Location(aTrsf);
}

// =======================================================================
// function : ParametersToShape
// purpose :
// =======================================================================
void TInspectorAPI_PluginParameters::ParametersToShape(const AsciiString1& theValue,
                                                       TopoShape&                  theShape)
{
  int                     aSeparatorPos    = theValue.Search(":");
  AsciiString1 anOrientationStr = theValue;
  AsciiString1 aLocationStr     = anOrientationStr.Split(aSeparatorPos);
  // orientation
  if (anOrientationStr.Length() < 2)
    return;
  anOrientationStr.Split(anOrientationStr.Length() - 1);

  TopAbs_Orientation anOrientation;
  if (!TopAbs1::ShapeOrientationFromString(anOrientationStr.ToCString(), anOrientation))
    return;
  // location
  TopLoc_Location aLocation = fromString(aLocationStr);

  theShape.Location(aLocation);
  theShape.Orientation(anOrientation);
}
