// Created on: 2006-05-25
// Created by: Alexander GRIGORIEV
// Copyright (c) 2006-2014 OPEN CASCADE SAS
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

#include <VrmlData_Scene.hxx>
#include <VrmlData_Coordinate.hxx>
#include <VrmlData_Color.hxx>
#include <VrmlData_Normal.hxx>
#include <VrmlData_TextureCoordinate.hxx>
#include <VrmlData_InBuffer.hxx>
#include <VrmlData_Box.hxx>
#include <VrmlData_Cone.hxx>
#include <VrmlData_Cylinder.hxx>
#include <VrmlData_Sphere.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrim_Cone.hxx>
#include <BRepPrim_Cylinder.hxx>
#include <BRepPrim_Sphere.hxx>
#include <BRepPrim_Builder.hxx>
#include <NCollection_Vector.hxx>
#include <VrmlData_ArrayVec3d.hxx>

IMPLEMENT_STANDARD_RTTIEXT(VrmlData_Geometry, VrmlData_Node)

#ifdef _MSC_VER
  #define _CRT_SECURE_NO_DEPRECATE
  #pragma warning(disable : 4996)
#endif

//=================================================================================================

const Coords3d& VrmlData_ArrayVec3d::Value(const Standard_Size i) const
{
  static Coords3d anOrigin(0., 0., 0.);
  return i < myLength ? myArray[i] : anOrigin;
}

//=================================================================================================

Standard_Boolean VrmlData_ArrayVec3d::AllocateValues(const Standard_Size theLength)
{
  myArray =
    reinterpret_cast<const Coords3d*>(Scene().Allocator()->Allocate(theLength * sizeof(Coords3d)));
  myLength = theLength;
  return (myArray != 0L);
}

//=================================================================================================

const Handle(TopoDS_TShape)& VrmlData_Box::TShape()
{
  if (myIsModified)
  {
    try
    {
      const TopoShell aShell =
        BoxMaker(Point3d(-0.5 * mySize), mySize.X(), mySize.Y(), mySize.Z());
      SetTShape(aShell.TShape());
      myIsModified = Standard_False;
    }
    catch (ExceptionBase const&)
    {
      myTShape.Nullify();
    }
  }
  return myTShape;
}

//=================================================================================================

Handle(VrmlData_Node) VrmlData_Box::Clone(const Handle(VrmlData_Node)& theOther) const
{
  Handle(VrmlData_Box) aResult = Handle(VrmlData_Box)::DownCast(VrmlData_Node::Clone(theOther));
  if (aResult.IsNull())
    aResult = new VrmlData_Box(theOther.IsNull() ? Scene() : theOther->Scene(), Name());
  aResult->SetSize(mySize);
  return aResult;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Box::Read(InputBuffer& theBuffer)
{
  VrmlData_ErrorStatus aStatus;
  if (OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
  {
    if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "size"))
      aStatus = Scene().ReadXYZ(theBuffer, mySize, Standard_True, Standard_True);
    if (OK(aStatus))
      aStatus = readBrace(theBuffer);
  }
  return aStatus;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Box::Write(const char* thePrefix) const
{
  static char          header[] = "Box1 {";
  VrmlData_ErrorStatus aStatus;
  if (OK(aStatus, Scene().WriteLine(thePrefix, header, GlobalIndent())))
  {
    char buf[128];
    Sprintf(buf, "size %.12g %.12g %.12g", mySize.X(), mySize.Y(), mySize.Z());
    Scene().WriteLine(buf);
    aStatus = WriteClosing();
  }
  return aStatus;
}

//=================================================================================================

const Handle(TopoDS_TShape)& VrmlData_Cone::TShape()
{
  if (myIsModified && (myHasBottom || myHasSide))
  {
    try
    {
      Frame3d        aLocalAxis(Point3d(0., -0.5 * myHeight, 0.), Dir3d(0., 1., 0.));
      BRepPrim_Cone aBuilder(aLocalAxis, myBottomRadius, 0., myHeight);
      if (!myHasBottom)
        myTShape = aBuilder.LateralFace().TShape();
      else if (!myHasSide)
        myTShape = aBuilder.BottomFace().TShape();
      else
        myTShape = aBuilder.Shell().TShape();
      myIsModified = Standard_False;
    }
    catch (ExceptionBase const&)
    {
      myTShape.Nullify();
    }
  }
  return myTShape;
}

//=================================================================================================

Handle(VrmlData_Node) VrmlData_Cone::Clone(const Handle(VrmlData_Node)& theOther) const
{
  Handle(VrmlData_Cone) aResult = Handle(VrmlData_Cone)::DownCast(VrmlData_Node::Clone(theOther));
  if (aResult.IsNull())
    aResult = new VrmlData_Cone(theOther.IsNull() ? Scene() : theOther->Scene(), Name());

  aResult->SetBottomRadius(myBottomRadius);
  aResult->SetHeight(myHeight);
  aResult->SetFaces(myHasBottom, myHasSide);
  return aResult;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Cone::Read(InputBuffer& theBuffer)
{
  VrmlData_ErrorStatus aStatus;
  Standard_Boolean     hasSide(Standard_True), hasBottom(Standard_True);

  while (OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
  {
    if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "bottomRadius"))
      aStatus = Scene().ReadReal(theBuffer, myBottomRadius, Standard_True, Standard_True);
    else if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "height"))
      aStatus = Scene().ReadReal(theBuffer, myHeight, Standard_True, Standard_True);
    else if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "side"))
    {
      if (OK(aStatus, ReadBoolean(theBuffer, hasSide)))
        myHasSide = hasSide;
    }
    else if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "bottom"))
    {
      if (OK(aStatus, ReadBoolean(theBuffer, hasBottom)))
        myHasBottom = hasBottom;
    }
    else
      break;

    if (!OK(aStatus))
      break;
  }
  // Read the terminating (closing) brace
  if (OK(aStatus))
    aStatus = readBrace(theBuffer);
  return aStatus;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Cone::Write(const char* thePrefix) const
{
  static char          header[] = "Cone {";
  VrmlData_ErrorStatus aStatus;
  if (OK(aStatus, Scene().WriteLine(thePrefix, header, GlobalIndent())))
  {
    char buf[128];
    if ((myBottomRadius - 1.) * (myBottomRadius - 1.) > Precision1::Confusion())
    {
      Sprintf(buf, "bottomRadius %.12g", myBottomRadius);
      aStatus = Scene().WriteLine(buf);
    }
    if (OK(aStatus) && (myHeight - 2.) * (myHeight - 2.) > Precision1::Confusion())
    {
      Sprintf(buf, "height       %.12g", myHeight);
      aStatus = Scene().WriteLine(buf);
    }
    if (OK(aStatus) && myHasBottom == Standard_False)
      aStatus = Scene().WriteLine("bottom   FALSE");
    if (OK(aStatus) && myHasSide == Standard_False)
      aStatus = Scene().WriteLine("side     FALSE");

    aStatus = WriteClosing();
  }
  return aStatus;
}

//=================================================================================================

// Standard_Boolean VrmlData_Cone::IsDefault () const
// {
//   return
//     (myHasBottom && myHasSide &&
//      ((myBottomRadius - 1.)*(myBottomRadius-1.) < Precision1::Confusion()) &&
//      ((myHeight - 2.)*(myHeight - 2.) < Precision1::Confusion()));
// }

//=================================================================================================

const Handle(TopoDS_TShape)& VrmlData_Cylinder::TShape()
{
  if (myIsModified && (myHasBottom || myHasSide || myHasTop))
  {
    try
    {
      Frame3d            aLocalAxis(Point3d(0., -0.5 * myHeight, 0.), Dir3d(0., 1., 0.));
      BRepPrim_Cylinder aBuilder(aLocalAxis, myRadius, myHeight);
      BRepPrim_Builder  aShapeBuilder;
      TopoShell      aShell;
      aShapeBuilder.MakeShell(aShell);
      if (myHasSide)
        aShapeBuilder.AddShellFace(aShell, aBuilder.LateralFace());
      if (myHasTop)
        aShapeBuilder.AddShellFace(aShell, aBuilder.TopFace());
      if (myHasBottom)
        aShapeBuilder.AddShellFace(aShell, aBuilder.BottomFace());
      myTShape     = aShell.TShape();
      myIsModified = Standard_False;
    }
    catch (ExceptionBase const&)
    {
      myTShape.Nullify();
    }
  }
  return myTShape;
}

//=================================================================================================

Handle(VrmlData_Node) VrmlData_Cylinder::Clone(const Handle(VrmlData_Node)& theOther) const
{
  Handle(VrmlData_Cylinder) aResult =
    Handle(VrmlData_Cylinder)::DownCast(VrmlData_Node::Clone(theOther));
  if (aResult.IsNull())
    aResult = new VrmlData_Cylinder(theOther.IsNull() ? Scene() : theOther->Scene(), Name());
  aResult->SetRadius(myRadius);
  aResult->SetHeight(myHeight);
  aResult->SetFaces(myHasBottom, myHasSide, myHasTop);
  return aResult;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Cylinder::Read(InputBuffer& theBuffer)
{
  VrmlData_ErrorStatus aStatus;
  Standard_Boolean     hasSide(Standard_True), hasBottom(Standard_True);
  Standard_Boolean     hasTop(Standard_True);

  while (OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
  {
    if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "radius"))
      aStatus = Scene().ReadReal(theBuffer, myRadius, Standard_True, Standard_True);
    else if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "height"))
      aStatus = Scene().ReadReal(theBuffer, myHeight, Standard_True, Standard_True);
    else if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "top"))
    {
      if (OK(aStatus, ReadBoolean(theBuffer, hasTop)))
        myHasTop = hasTop;
    }
    else if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "side"))
    {
      if (OK(aStatus, ReadBoolean(theBuffer, hasSide)))
        myHasSide = hasSide;
    }
    else if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "bottom"))
    {
      if (OK(aStatus, ReadBoolean(theBuffer, hasBottom)))
        myHasBottom = hasBottom;
    }
    else
      break;

    if (!OK(aStatus))
      break;
  }

  // Read the terminating (closing) brace
  if (OK(aStatus))
    aStatus = readBrace(theBuffer);
  return aStatus;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Cylinder::Write(const char* thePrefix) const
{
  static char          header[] = "Cylinder {";
  VrmlData_ErrorStatus aStatus;
  if (OK(aStatus, Scene().WriteLine(thePrefix, header, GlobalIndent())))
  {
    char buf[128];
    if ((myRadius - 1.) * (myRadius - 1.) > Precision1::Confusion())
    {
      Sprintf(buf, "radius   %.12g", myRadius);
      aStatus = Scene().WriteLine(buf);
    }
    if (OK(aStatus) && (myHeight - 2.) * (myHeight - 2.) > Precision1::Confusion())
    {
      Sprintf(buf, "height   %.12g", myHeight);
      aStatus = Scene().WriteLine(buf);
    }
    if (OK(aStatus) && myHasBottom == Standard_False)
      aStatus = Scene().WriteLine("bottom   FALSE");
    if (OK(aStatus) && myHasSide == Standard_False)
      aStatus = Scene().WriteLine("side     FALSE");
    if (OK(aStatus) && myHasTop == Standard_False)
      aStatus = Scene().WriteLine("top      FALSE");

    aStatus = WriteClosing();
  }
  return aStatus;
}

//=================================================================================================

// Standard_Boolean VrmlData_Cylinder::IsDefault () const
// {
//   return
//     (myHasBottom && myHasSide && myHasTop &&
//      ((myRadius - 1.)*(myRadius - 1.) < Precision1::Confusion()) &&
//      ((myHeight - 2.)*(myHeight - 2.) < Precision1::Confusion()));
// }

//=================================================================================================

const Handle(TopoDS_TShape)& VrmlData_Sphere::TShape()
{
  if (myIsModified)
  {
    try
    {
      myTShape     = BRepPrim_Sphere(myRadius).Shell().TShape();
      myIsModified = Standard_False;
    }
    catch (ExceptionBase const&)
    {
      myTShape.Nullify();
    }
  }
  return myTShape;
}

//=================================================================================================

Handle(VrmlData_Node) VrmlData_Sphere::Clone(const Handle(VrmlData_Node)& theOther) const
{
  Handle(VrmlData_Sphere) aResult =
    Handle(VrmlData_Sphere)::DownCast(VrmlData_Node::Clone(theOther));
  if (aResult.IsNull())
    aResult = new VrmlData_Sphere(theOther.IsNull() ? Scene() : theOther->Scene(), Name());
  aResult->SetRadius(myRadius);
  return aResult;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Sphere::Read(InputBuffer& theBuffer)
{
  VrmlData_ErrorStatus aStatus;
  while (OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
    if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "radius"))
      aStatus = Scene().ReadReal(theBuffer, myRadius, Standard_True, Standard_True);
    else
      break;

  // Read the terminating (closing) brace
  if (OK(aStatus))
    aStatus = readBrace(theBuffer);
  return aStatus;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Sphere::Write(const char* thePrefix) const
{
  static char          header[] = "Sphere {";
  VrmlData_ErrorStatus aStatus;
  if (OK(aStatus, Scene().WriteLine(thePrefix, header, GlobalIndent())))
  {
    char buf[128];
    Sprintf(buf, "radius   %.12g", myRadius);
    Scene().WriteLine(buf);
    aStatus = WriteClosing();
  }
  return aStatus;
}

//=================================================================================================

// Standard_Boolean VrmlData_Sphere::IsDefault () const
// {
//   return ((myRadius - 1.)*(myRadius - 1.) < Precision1::Confusion())
// }

//=================================================================================================

Standard_Boolean VrmlData_TextureCoordinate::AllocateValues(const Standard_Size theLength)
{
  myPoints =
    reinterpret_cast<const Coords2d*>(Scene().Allocator()->Allocate(theLength * sizeof(Coords2d)));
  myLength = theLength;
  return (myPoints != 0L);
}

//=================================================================================================

Handle(VrmlData_Node) VrmlData_TextureCoordinate::Clone(const Handle(VrmlData_Node)& theOther) const
{
  Handle(VrmlData_TextureCoordinate) aResult =
    Handle(VrmlData_TextureCoordinate)::DownCast(VrmlData_Node::Clone(theOther));
  if (aResult.IsNull())
    aResult =
      new VrmlData_TextureCoordinate(theOther.IsNull() ? Scene() : theOther->Scene(), Name());
  if (&aResult->Scene() == &Scene())
    aResult->SetPoints(myLength, myPoints);
  else
  {
    aResult->AllocateValues(myLength);
    for (Standard_Size i = 0; i < myLength; i++)
      const_cast<Coords2d&>(aResult->myPoints[i]) = myPoints[i];
  }
  return aResult;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_TextureCoordinate::Read(InputBuffer& theBuffer)
{
  VrmlData_ErrorStatus      aStatus;
  NCollection_Vector<Coords2d> vecValues;
  if (OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
  {
    // Match the name with the current word in the stream
    if (VRMLDATA_LCOMPARE(theBuffer.LinePtr, "point"))
      // Read the body of the data node (comma-separated list of duplets)
      if (OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
      {
        if (theBuffer.LinePtr[0] != '[') // opening bracket
          aStatus = VrmlData_VrmlFormatError;
        else
        {
          theBuffer.LinePtr++;
          for (;;)
          {
            Coords2d anXY;
            if (!OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
              break;
            // closing bracket, in case that it follows a comma
            if (theBuffer.LinePtr[0] == ']')
            {
              theBuffer.LinePtr++;
              break;
            }
            if (!OK(aStatus, Scene().ReadXY(theBuffer, anXY, Standard_False, Standard_False)))
              break;
            vecValues.Append(anXY);
            if (!OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
              break;
            if (theBuffer.LinePtr[0] == ',')
            {
              theBuffer.LinePtr++;
            }
            else if (theBuffer.LinePtr[0] == ']')
            { // closing bracket
              theBuffer.LinePtr++;
              break;
            }
          }
        }
      }
    if (OK(aStatus) && OK(aStatus, readBrace(theBuffer)))
    {
      myLength = vecValues.Length();
      if (myLength > 0)
      {
        Coords2d* aPoints =
          reinterpret_cast<Coords2d*>(Scene().Allocator()->Allocate(myLength * sizeof(Coords2d)));
        myPoints = aPoints;
        for (Standard_Integer i = 0; i < Standard_Integer(myLength); i++)
          aPoints[i] = vecValues(i);
      }
    }
  }
  return aStatus;
}

//=================================================================================================

// Handle(VrmlData_Node) VrmlData_ArrayVec3d::Clone
//                                 (const Handle(VrmlData_Node)& theOther) const
// {
//   VrmlData_Node::Clone (theOther);
//   const Handle(VrmlData_ArrayVec3d) anArrayNode =
//     Handle(VrmlData_ArrayVec3d)::DownCast (theOther);
//   if (anArrayNode.IsNull() == Standard_False)
//     anArrayNode->SetValues (myLength, myArray);
//   return theOther;
// }

//=================================================================================================

VrmlData_ErrorStatus VrmlData_ArrayVec3d::ReadArray(InputBuffer&     theBuffer,
                                                    const char*            theName,
                                                    const Standard_Boolean isScale)
{
  VrmlData_ErrorStatus       aStatus;
  NCollection_Vector<Coords3d> vecValues;
  if (OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
  {
    // Match the name with the current word in the stream
    if (theName)
    {
      const Standard_Size aNameLen = strlen(theName);
      if (strncmp(theBuffer.LinePtr, theName, aNameLen))
        aStatus = VrmlData_VrmlFormatError;
      else
        theBuffer.LinePtr += aNameLen;
    }
    else
    {
      // Skip the word in the input
      while (theBuffer.LinePtr[0] != ' ' && theBuffer.LinePtr[0] != ','
             && theBuffer.LinePtr[0] != '\t' && theBuffer.LinePtr[0] != '\n'
             && theBuffer.LinePtr[0] != '\r' && theBuffer.LinePtr[0] != '\0')
        theBuffer.LinePtr++;
    }
    // Read the body of the data node (list of triplets)
    if (OK(aStatus) && OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
    {
      if (theBuffer.LinePtr[0] != '[') // opening bracket
      {
        // Handle case when brackets are omitted for single element of array
        Coords3d anXYZ;
        // Read three numbers (XYZ value)
        if (!OK(aStatus, Scene().ReadXYZ(theBuffer, anXYZ, isScale, Standard_False)))
          aStatus = VrmlData_VrmlFormatError;
        else
          vecValues.Append(anXYZ);
      }
      else
      {
        theBuffer.LinePtr++;
        for (;;)
        {
          Coords3d anXYZ;
          if (!OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
            break;
          // closing bracket, in case that it follows a comma
          if (theBuffer.LinePtr[0] == ']')
          {
            theBuffer.LinePtr++;
            break;
          }
          // Read three numbers (XYZ value)
          if (!OK(aStatus, Scene().ReadXYZ(theBuffer, anXYZ, isScale, Standard_False)))
            break;
          vecValues.Append(anXYZ);
          if (!OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
            break;
          if (theBuffer.LinePtr[0] == ']')
          { // closing bracket
            theBuffer.LinePtr++;
            break;
          }
        }
      }
    }
    if (OK(aStatus) && OK(aStatus, readBrace(theBuffer)))
    {
      myLength = vecValues.Length();
      if (myLength > 0)
      {
        Coords3d* anArray =
          reinterpret_cast<Coords3d*>(Scene().Allocator()->Allocate(myLength * sizeof(Coords3d)));
        myArray = anArray;
        for (Standard_Integer i = 0; i < Standard_Integer(myLength); i++)
          anArray[i] = vecValues(i);
      }
    }
  }
  return aStatus;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_ArrayVec3d::WriteArray(const char*            theName,
                                                     const Standard_Boolean isScale) const
{
  VrmlData_ErrorStatus aStatus(VrmlData_StatusOK);
  if (myLength > 0)
  {
    aStatus = Scene().WriteLine(theName, "[", 2 * GlobalIndent());
    if (OK(aStatus))
    {
      for (Standard_Size i = 0; i < myLength - 1; i++)
        if (!OK(aStatus, Scene().WriteXYZ(myArray[i], isScale, ",")))
          break;
      if (OK(aStatus))
        aStatus = Scene().WriteXYZ(myArray[myLength - 1], isScale);
    }
    if (aStatus == VrmlData_StatusOK)
      aStatus = Scene().WriteLine("]", 0L, -2 * GlobalIndent());
  }
  return aStatus;
}

//=================================================================================================

Standard_Boolean VrmlData_ArrayVec3d::IsDefault() const
{
  return myLength == 0;
}

//=================================================================================================

Handle(VrmlData_Node) VrmlData_Coordinate::Clone(const Handle(VrmlData_Node)& theOther) const
{
  Handle(VrmlData_Coordinate) aResult =
    Handle(VrmlData_Coordinate)::DownCast(VrmlData_Node::Clone(theOther));
  if (aResult.IsNull())
    aResult = new VrmlData_Coordinate(theOther.IsNull() ? Scene() : theOther->Scene(), Name());
  if (&aResult->Scene() == &Scene())
    aResult->SetValues(Length(), Values());
  else
  {
    aResult->AllocateValues(Length());
    for (Standard_Size i = 0; i < Length(); i++)
      const_cast<Coords3d&>(aResult->Values()[i]) = Values()[i];
  }
  return aResult;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Coordinate::Read(InputBuffer& theBuffer)
{
  return VrmlData_ArrayVec3d::ReadArray(theBuffer, "point", Standard_True);
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Coordinate::Write(const char* thePrefix) const
{
  static char          header[] = "Coordinate {";
  VrmlData_ErrorStatus aStatus;
  if (OK(aStatus, Scene().WriteLine(thePrefix, header, GlobalIndent())))
  {
    WriteArray("point", Standard_True);
    aStatus = WriteClosing();
  }
  return aStatus;
}

//=================================================================================================

Handle(VrmlData_Node) VrmlData_Color::Clone(const Handle(VrmlData_Node)& theOther) const
{
  Handle(VrmlData_Color) aResult = Handle(VrmlData_Color)::DownCast(VrmlData_Node::Clone(theOther));
  if (aResult.IsNull())
    aResult = new VrmlData_Color(theOther.IsNull() ? Scene() : theOther->Scene(), Name());
  if (&aResult->Scene() == &Scene())
    aResult->SetValues(Length(), Values());
  else
  {
    aResult->AllocateValues(Length());
    for (Standard_Size i = 0; i < Length(); i++)
      const_cast<Coords3d&>(aResult->Values()[i]) = Values()[i];
  }
  return aResult;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Color::Read(InputBuffer& theBuffer)
{
  return ReadArray(theBuffer, "color", Standard_False);
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Color::Write(const char* thePrefix) const
{
  static char          header[] = "Color {";
  VrmlData_ErrorStatus aStatus;
  if (OK(aStatus, Scene().WriteLine(thePrefix, header, GlobalIndent())))
  {
    WriteArray("color", Standard_False);
    aStatus = WriteClosing();
  }
  return aStatus;
}

//=================================================================================================

Handle(VrmlData_Node) VrmlData_Normal::Clone(const Handle(VrmlData_Node)& theOther) const
{
  Handle(VrmlData_Normal) aResult =
    Handle(VrmlData_Normal)::DownCast(VrmlData_Node::Clone(theOther));
  if (aResult.IsNull())
    aResult = new VrmlData_Normal(theOther.IsNull() ? Scene() : theOther->Scene(), Name());
  if (&aResult->Scene() == &Scene())
    aResult->SetValues(Length(), Values());
  else
  {
    aResult->AllocateValues(Length());
    for (Standard_Size i = 0; i < Length(); i++)
      const_cast<Coords3d&>(aResult->Values()[i]) = Values()[i];
  }
  return aResult;
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Normal::Read(InputBuffer& theBuffer)
{
  return VrmlData_ArrayVec3d::ReadArray(theBuffer, "vector", Standard_False);
}

//=================================================================================================

VrmlData_ErrorStatus VrmlData_Normal::Write(const char* thePrefix) const
{
  static char          header[] = "Normal {";
  VrmlData_ErrorStatus aStatus;
  if (OK(aStatus, Scene().WriteLine(thePrefix, header, GlobalIndent())))
  {
    WriteArray("vector", Standard_False);
    aStatus = WriteClosing();
  }
  return aStatus;
}
