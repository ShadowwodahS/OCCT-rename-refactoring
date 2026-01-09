// Created by: NW,JPB,CAL
// Copyright (c) 1991-1999 Matra Datavision
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

//-Version
//-Design	Declaration des variables specifiques aux fonds d'ecran.
//-Warning	Un fond d'ecran est defini par une couleur.
//-References
//-Language	C++ 2.0
//-Declarations
// for the class

#include <Aspect_Background.hxx>
#include <Quantity_Color.hxx>
#include <Standard_Dump.hxx>

//-Aliases
//-Global data definitions
//	-- la couleur associee au fond d'ecran
//	MyColor	:	Color;
//-Constructors
//-Destructors
//-Methods, in order
Background1::Background1()
{

  Color1 MatraGray(Quantity_NOC_MATRAGRAY);

  MyColor = MatraGray;
}

Background1::Background1(const Color1& AColor)
{

  MyColor = AColor;
}

void Background1::SetColor(const Color1& AColor)
{

  MyColor = AColor;
}

Color1 Background1::Color() const
{

  return (MyColor);
}

//=================================================================================================

void Background1::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_CLASS_BEGIN(theOStream, Background1)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &MyColor)
}
