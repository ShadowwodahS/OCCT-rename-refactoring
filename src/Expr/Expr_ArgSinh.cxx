// Created on: 1991-05-27
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
#include <Expr_ArgSinh.hxx>
#include <Expr_GeneralExpression.hxx>
#include <Expr_NamedUnknown.hxx>
#include <Expr_Operators.hxx>
#include <Expr_Sinh.hxx>
#include <Expr_Square.hxx>
#include <Expr_SquareRoot.hxx>
#include <Expr_Sum.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Expr_ArgSinh, Expr_UnaryExpression)

Expr_ArgSinh::Expr_ArgSinh(const Handle(Expression1)& exp)
{
  CreateOperand(exp);
}

Handle(Expression1) Expr_ArgSinh::ShallowSimplified() const
{
  Handle(Expression1) op = Operand();
  if (op->IsKind(STANDARD_TYPE(Expr_NumericValue)))
  {
    Handle(Expr_NumericValue) valop = Handle(Expr_NumericValue)::DownCast(op);
    return new Expr_NumericValue(ASinh(valop->GetValue()));
  }
  if (op->IsKind(STANDARD_TYPE(Expr_Sinh)))
  {
    return op->SubExpression(1);
  }
  Handle(Expr_ArgSinh) me = this;
  return me;
}

Handle(Expression1) Expr_ArgSinh::Copy() const
{
  return new Expr_ArgSinh(Expr1::CopyShare(Operand()));
}

Standard_Boolean Expr_ArgSinh::IsIdentical(const Handle(Expression1)& Other) const
{
  if (!Other->IsKind(STANDARD_TYPE(Expr_ArgSinh)))
  {
    return Standard_False;
  }
  Handle(Expression1) op = Operand();
  return op->IsIdentical(Other->SubExpression(1));
}

Standard_Boolean Expr_ArgSinh::IsLinear() const
{
  if (ContainsUnknowns())
  {
    return Standard_False;
  }
  return Standard_True;
}

Handle(Expression1) Expr_ArgSinh::Derivative(const Handle(Expr_NamedUnknown)& X) const
{
  if (!Contains(X))
  {
    return new Expr_NumericValue(0.0);
  }
  Handle(Expression1) op    = Operand();
  Handle(Expression1) derop = op->Derivative(X);

  Handle(Expr_Square) sq = new Expr_Square(Expr1::CopyShare(op));
  // X2 + 1
  Handle(Expr_Sum) thesum = sq->ShallowSimplified() + 1.0;

  // sqrt(X2 + 1)
  Handle(Expr_SquareRoot) theroot = new Expr_SquareRoot(thesum->ShallowSimplified());

  // ArgSinh'(F(X)) = F'(X)/sqrt(F(X)2+1)
  Handle(Expr_Division) thediv = derop / theroot->ShallowSimplified();

  return thediv->ShallowSimplified();
}

Standard_Real Expr_ArgSinh::Evaluate(const Expr_Array1OfNamedUnknown& vars,
                                     const TColStd_Array1OfReal&      vals) const
{
  Standard_Real val = Operand()->Evaluate(vars, vals);
  return ::Log(val + ::Sqrt(::Square(val) + 1.0));
}

AsciiString1 Expr_ArgSinh::String() const
{
  AsciiString1 str("ASinh(");
  str += Operand()->String();
  str += ")";
  return str;
}
