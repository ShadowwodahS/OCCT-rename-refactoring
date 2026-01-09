// Copyright (c) 2015 OPEN CASCADE SAS
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

#include <StdPersistent_Naming.hxx>
#include <StdObjMgt_ReadData.hxx>

#include <TNaming_Name.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_Iterator.hxx>

//=======================================================================
// function : Import
// purpose  : Import transient attribute from the persistent data
//=======================================================================
void Naming2::NamedShape1::Import(const Handle(ShapeAttribute)& theAttribute) const
{
  theAttribute->SetVersion(myVersion);

  if (myOldShapes.IsNull() || myNewShapes.IsNull())
    return;

  TNaming_Builder aBuilder(theAttribute->Label());

  HArray1OfShape1::Iterator aOldShapesIter(*myOldShapes->Array());
  HArray1OfShape1::Iterator aNewShapesIter(*myNewShapes->Array());
  for (; aNewShapesIter.More(); aOldShapesIter.Next(), aNewShapesIter.Next())
  {
    TopoShape aOldShape = aOldShapesIter.Value().Import();
    TopoShape aNewShape = aNewShapesIter.Value().Import();

    switch (myShapeStatus)
    {
      case 0:
        aBuilder.Generated(aNewShape);
        break; // PRIMITIVE
      case 1:
        aBuilder.Generated(aOldShape, aNewShape);
        break; // GENERATED
      case 2:
        aBuilder.Modify(aOldShape, aNewShape);
        break; // MODIFY
      case 3:
        aBuilder.Delete(aOldShape);
        break; // DELETE
      case 4:
        aBuilder.Select(aNewShape, aOldShape);
        break; // SELECTED
      case 5:
        aBuilder.Modify(aOldShape, aNewShape);
        break; // REPLACE
    }
  }
}

//=======================================================================
// function : Read
// purpose  : Read persistent data from a file
//=======================================================================
void Naming2::Name::Read(ReadData& theReadData)
{
  theReadData >> myType >> myShapeType >> myArgs >> myStop >> myIndex;
}

//=======================================================================
// function : Write
// purpose  : Write persistent data to a file
//=======================================================================
void Naming2::Name::Write(WriteData& theWriteData) const
{
  theWriteData << myType << myShapeType << myArgs << myStop << myIndex;
}

//=======================================================================
// function : Import
// purpose  : Import transient object from the persistent data
//=======================================================================
void Naming2::Name::Import(TNaming_Name& theName, const Handle(Data2)&) const
{
  theName.Type(static_cast<TNaming_NameType>(myType));
  theName.ShapeType(static_cast<TopAbs_ShapeEnum>(myShapeType));

  if (myArgs)
  {
    StdLPersistent_HArray1OfPersistent::Iterator anIter(*myArgs->Array());
    for (; anIter.More(); anIter.Next())
    {
      const Handle(StdObjMgt_Persistent)& aPersistent = anIter.Value();
      if (aPersistent)
      {
        Handle(TDF_Attribute) anArg = aPersistent->GetAttribute();
        theName.Append(Handle(ShapeAttribute)::DownCast(anArg));
      }
    }
  }

  if (myStop)
  {
    Handle(TDF_Attribute) aStop = myStop->GetAttribute();
    theName.StopNamedShape(Handle(ShapeAttribute)::DownCast(aStop));
  }

  theName.Index(myIndex);
}

//=======================================================================
// function : Read
// purpose  : Read persistent data from a file
//=======================================================================
void Naming2::Name_1::Read(ReadData& theReadData)
{
  Name::Read(theReadData);
  theReadData >> myContextLabel;
}

//=======================================================================
// function : Write
// purpose  : Write persistent data to a file
//=======================================================================
void Naming2::Name_1::Write(WriteData& theWriteData) const
{
  Name::Write(theWriteData);
  theWriteData << myContextLabel;
}

//=======================================================================
// function : Import
// purpose  : Import transient object from the persistent data
//=======================================================================
void Naming2::Name_1::Import(TNaming_Name&           theName,
                                          const Handle(Data2)& theDF) const
{
  Name::Import(theName, theDF);
  if (myContextLabel)
    theName.ContextLabel(myContextLabel->Label(theDF));
}

//=======================================================================
// function : Read
// purpose  : Read persistent data from a file
//=======================================================================
void Naming2::Name_2::Read(ReadData& theReadData)
{
  Name_1::Read(theReadData);
  theReadData >> myOrientation;
}

//=======================================================================
// function : Write
// purpose  : Write persistent data to a file
//=======================================================================
void Naming2::Name_2::Write(WriteData& theWriteData) const
{
  Name_1::Write(theWriteData);
  theWriteData << myOrientation;
}

//=======================================================================
// function : Import
// purpose  : Import transient object from the persistent data
//=======================================================================
void Naming2::Name_2::Import(TNaming_Name&           theName,
                                          const Handle(Data2)& theDF) const
{
  Name_1::Import(theName, theDF);
  theName.Orientation(static_cast<TopAbs_Orientation>(myOrientation));
}

//=======================================================================
// function : ImportAttribute
// purpose  : Import transient attribute from the persistent data
//=======================================================================
void Naming2::Naming1::ImportAttribute()
{
  Handle(Name) aName = Handle(Name)::DownCast(myData);
  if (aName)
  {
    aName->Import(myTransient->ChangeName(), myTransient->Label().Data());
    myData.Nullify();
  }
}

//=======================================================================
// function : ImportAttribute
// purpose  : Import transient attribute from the persistent data
//=======================================================================
void Naming2::Naming_11::ImportAttribute()
{
  Naming1::ImportAttribute();

  Handle(ShapeAttribute) aNamedShape;
  if (myTransient->Label().FindAttribute(ShapeAttribute::GetID(), aNamedShape)
      && aNamedShape->Evolution() == TNaming_SELECTED)
  {
    for (Iterator1 anIter(aNamedShape); anIter.More(); anIter.Next())
    {
      const TopoShape& aOldShape = anIter.OldShape();
      const TopoShape& aNewShape = anIter.NewShape();

      if (!aOldShape.IsNull() && aOldShape.ShapeType() == TopAbs_VERTEX && !aNewShape.IsNull()
          && aNewShape.ShapeType() != TopAbs_VERTEX)
      {
        myTransient->ChangeName().Orientation(aOldShape.Orientation());
      }
    }
  }
}
