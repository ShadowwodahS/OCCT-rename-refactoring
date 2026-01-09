// Created on: 1991-06-10
// Created by: Arnaud BOUZY
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

#include <Expr.hxx>
#include <Expr_Equal.hxx>
#include <Expr_GeneralExpression.hxx>
#include <Expr_GeneralRelation.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Expr_Equal, Expr_SingleRelation)

Expr_Equal::Expr_Equal(const Handle(Expression1)& exp1,
                       const Handle(Expression1)& exp2)
{
  SetFirstMember(exp1);
  SetSecondMember(exp2);
}

Standard_Boolean Expr_Equal::IsSatisfied() const
{
  Handle(Expression1) fm = FirstMember();
  Handle(Expression1) sm = SecondMember();
  fm                                = fm->Simplified();
  sm                                = sm->Simplified();
  return (fm->IsIdentical(sm));
}

Handle(Expr_GeneralRelation) Expr_Equal::Simplified() const
{
  Handle(Expression1) fm = FirstMember();
  Handle(Expression1) sm = SecondMember();
  return new Expr_Equal(fm->Simplified(), sm->Simplified());
}

void Expr_Equal::Simplify()
{
  Handle(Expression1) fm = FirstMember();
  Handle(Expression1) sm = SecondMember();
  SetFirstMember(fm->Simplified());
  SetSecondMember(sm->Simplified());
}

Handle(Expr_GeneralRelation) Expr_Equal::Copy() const
{
  return new Expr_Equal(Expr1::CopyShare(FirstMember()), Expr1::CopyShare(SecondMember()));
}

AsciiString1 Expr_Equal::String() const
{
  return FirstMember()->String() + " = " + SecondMember()->String();
}
