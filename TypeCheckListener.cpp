
#include "TypeCheckListener.h"

#include "antlr4-runtime.h"

#include "../common/TypesMgr.h"
#include "../common/SymTable.h"
#include "../common/TreeDecoration.h"
#include "../common/SemErrors.h"

#include <iostream>
#include <string>

// uncomment the following line to enable debugging messages with DEBUG*
// #define DEBUG_BUILD
#include "../common/debug.h"

// using namespace std;


// Constructor
TypeCheckListener::TypeCheckListener(TypesMgr       & Types,
				     SymTable       & Symbols,
				     TreeDecoration & Decorations,
				     SemErrors      & Errors) :
  Types{Types},
  Symbols {Symbols},
  Decorations{Decorations},
  Errors{Errors} {
}

void TypeCheckListener::enterProgram(AslParser::ProgramContext *ctx) {
  DEBUG_ENTER();
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);
}
void TypeCheckListener::exitProgram(AslParser::ProgramContext *ctx) {
  if (Symbols.noMainProperlyDeclared())
    Errors.noMainProperlyDeclared(ctx);
  Symbols.popScope();
  Errors.print();
  DEBUG_EXIT();
}

void TypeCheckListener::enterFunction(AslParser::FunctionContext *ctx) {
  DEBUG_ENTER();
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);
  // Symbols.print();
}
void TypeCheckListener::exitFunction(AslParser::FunctionContext *ctx) {
  Symbols.popScope();
  DEBUG_EXIT();
}

void TypeCheckListener::enterDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterVariable_decl(AslParser::Variable_declContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitVariable_decl(AslParser::Variable_declContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterType(AslParser::TypeContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitType(AslParser::TypeContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterStatements(AslParser::StatementsContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitStatements(AslParser::StatementsContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterAssignStmt(AslParser::AssignStmtContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitAssignStmt(AslParser::AssignStmtContext *ctx) {

  TypesMgr::TypeId t1 = getTypeDecor(ctx->left_expr());
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr());

  // typeTo (t1) = TypeFrom (t2) | t2 cannot be copied to t1
  if ((not Types.isErrorTy(t1)) and (not Types.isErrorTy(t2)) and
      (not Types.copyableTypes(t1, t2)))
    Errors.incompatibleAssignment(ctx->ASSIGN());

  // t1 is not an lValue, assignment not possible
  if ((not Types.isErrorTy(t1)) and (not getIsLValueDecor(ctx->left_expr())))
    Errors.nonReferenceableLeftExpr(ctx->left_expr());

  DEBUG_EXIT();
}

void TypeCheckListener::enterIfStmt(AslParser::IfStmtContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitIfStmt(AslParser::IfStmtContext *ctx) {

  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());

  // expr is not a bool
  if ((not Types.isErrorTy(t1)) and (not Types.isBooleanTy(t1)))
    Errors.booleanRequired(ctx);
  DEBUG_EXIT();
}

void TypeCheckListener::enterWhileStmt(AslParser::WhileStmtContext * ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitWhileStmt(AslParser::WhileStmtContext * ctx) {

  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());

  //expr is not a bool
  if ((not Types.isErrorTy(t1)) and (not Types.isBooleanTy(t1)))
    Errors.booleanRequired(ctx);
  DEBUG_EXIT();
}


void TypeCheckListener::enterProcCall(AslParser::ProcCallContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitProcCall(AslParser::ProcCallContext *ctx) {
  
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  // t1 is not a function, so it is not callable
  if (not Types.isFunctionTy(t1) and not Types.isErrorTy(t1)) {
    Errors.isNotCallable(ctx->ident());
  }
  DEBUG_EXIT();
}

void TypeCheckListener::enterReturnStmt(AslParser::ReturnStmtContext * ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitReturnStmt(AslParser::ReturnStmtContext * ctx) {

  if (ctx->expr() != NULL) {

    TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
    
    // return type is Non-Primitive
    if (not Types.isErrorTy(t1) and not Types.isPrimitiveNonVoidTy(t1))
      Errors.incompatibleReturn(ctx->RETURN());

    // return type does not match

    // function was void
  }

  DEBUG_EXIT();
}


void TypeCheckListener::enterReadStmt(AslParser::ReadStmtContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitReadStmt(AslParser::ReadStmtContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->left_expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isPrimitiveTy(t1)) and
      (not Types.isFunctionTy(t1)))
    Errors.readWriteRequireBasic(ctx);
  if ((not Types.isErrorTy(t1)) and (not getIsLValueDecor(ctx->left_expr())))
    Errors.nonReferenceableExpression(ctx);
  DEBUG_EXIT();
}

void TypeCheckListener::enterWriteExpr(AslParser::WriteExprContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitWriteExpr(AslParser::WriteExprContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isPrimitiveTy(t1)))
    Errors.readWriteRequireBasic(ctx);
  DEBUG_EXIT();
}

void TypeCheckListener::enterWriteString(AslParser::WriteStringContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitWriteString(AslParser::WriteStringContext *ctx) {
  DEBUG_EXIT();
}

void TypeCheckListener::enterLeft_expr(AslParser::Left_exprContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitLeft_expr(AslParser::Left_exprContext *ctx) {

  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());

  // Left Expr is an array access 
  if (ctx->expr() != NULL) {

  }

  putTypeDecor(ctx, t1);
  bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, b);

  DEBUG_EXIT();
}


void TypeCheckListener::enterArrayIndex(AslParser::ArrayIndexContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitArrayIndex(AslParser::ArrayIndexContext * ctx) {
  // Declarem un Error i en cas que tot estigui en ordre prendra valor igual
  // al tipus de l'array per fer el put
  TypesMgr::TypeId tArr = Types.createErrorTy();

  if (not Symbols.findInCurrentScope(ctx->ID()->getText()))
    Errors.undeclaredIdent(ctx->ID());
  
  else {
    
    TypesMgr::TypeId t = getTypeDecor(ctx->expr());
    TypesMgr::TypeId tID = Symbols.getType(ctx->ID()->getText());
    
    if (not Types.isArrayTy(tID))
      Errors.nonArrayInArrayAccess(ctx);
    
    // tArr pren per valor el tipus de l'array en questio
    else tArr = Types.getArrayElemType(tID); 
      
    if (not Types.isIntegerTy(t))
      Errors.nonIntegerIndexInArrayAccess(ctx->expr());
  }

  putTypeDecor(ctx, tArr);
  putIsLValueDecor(ctx, true);
  DEBUG_EXIT();
}


void TypeCheckListener::enterFuncCall(AslParser::FuncCallContext * ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitFuncCall(AslParser::FuncCallContext * ctx){
  DEBUG_EXIT();
}


void TypeCheckListener::enterUnary(AslParser::UnaryContext * ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitUnary(AslParser::UnaryContext * ctx) {
  
  TypesMgr::TypeId t = getTypeDecor(ctx->expr());

  if (ctx->NOT()) {
    if (not Types.isErrorTy(t) and not Types.isBooleanTy(t))
      Errors.incompatibleOperator(ctx->op);
    t = Types.createBooleanTy();
    putTypeDecor(ctx, t);
  }

  else {
    if (not Types.isErrorTy(t) and (not Types.isIntegerTy(t)) and (not Types.isFloatTy(t)))
      Errors.incompatibleOperator(ctx->op);
    if (Types.isFloatTy(t))
      putTypeDecor(ctx,t);
    else {
      t = Types.createIntegerTy();
      putTypeDecor(ctx,t);
    }
  }

  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
}



void TypeCheckListener::enterArithmetic(AslParser::ArithmeticContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitArithmetic(AslParser::ArithmeticContext *ctx) {

  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  TypesMgr::TypeId t;

  if (((not Types.isErrorTy(t1)) and (not Types.isNumericTy(t1))) or
      ((not Types.isErrorTy(t2)) and (not Types.isNumericTy(t2))))
    Errors.incompatibleOperator(ctx->op);
  
  if (Types.isFloatTy(t1) or Types.isFloatTy(t2)) t = Types.createFloatTy();
  else t = Types.createIntegerTy();
  
  putTypeDecor(ctx, t); // t is either a Float or an Int always
  putIsLValueDecor(ctx, false); // this node is not an lValue
  DEBUG_EXIT();
}

void TypeCheckListener::enterRelational(AslParser::RelationalContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitRelational(AslParser::RelationalContext *ctx) {
  
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  std::string oper = ctx->op->getText();

  if ((not Types.isErrorTy(t1)) and (not Types.isErrorTy(t2)) and
      (not Types.comparableTypes(t1, t2, oper)))
    Errors.incompatibleOperator(ctx->op);

  TypesMgr::TypeId t = Types.createBooleanTy();
  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
}

void TypeCheckListener::enterLogical(AslParser::LogicalContext * ctx) {
  DEBUG_ENTER();
}

void TypeCheckListener::exitLogical(AslParser::LogicalContext * ctx) {
  
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));

  if ((not Types.isErrorTy(t1)) and (not Types.isErrorTy(t2))) {
    if ((not Types.isBooleanTy(t1)) or (not Types.isBooleanTy(t2))) {
      Errors.incompatibleOperator(ctx->op);
    }
  }

  TypesMgr::TypeId t = Types.createBooleanTy();
  putTypeDecor(ctx, t); // This node is a bool
  putIsLValueDecor(ctx, false); // This node is not an lValue
  DEBUG_EXIT();
}



void TypeCheckListener::enterValue(AslParser::ValueContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitValue(AslParser::ValueContext *ctx) {

  TypesMgr::TypeId t;
  
  if (ctx->INTVAL())        t = Types.createIntegerTy();
  else if (ctx->FLOATVAL()) t = Types.createFloatTy();
  else if (ctx->BOOLVAL()) t = Types.createBooleanTy();
  else if (ctx->CHARVAL()) t = Types.createCharacterTy();
  else t = Types.createErrorTy();

  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
}

void TypeCheckListener::enterExprIdent(AslParser::ExprIdentContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitExprIdent(AslParser::ExprIdentContext *ctx) {
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  putTypeDecor(ctx, t1);
  bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, b);
  DEBUG_EXIT();
}

void TypeCheckListener::enterIdent(AslParser::IdentContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitIdent(AslParser::IdentContext *ctx) {
  std::string ident = ctx->getText();
  if (Symbols.findInStack(ident) == -1) {
    Errors.undeclaredIdent(ctx->ID());
    TypesMgr::TypeId te = Types.createErrorTy();
    putTypeDecor(ctx, te);
    putIsLValueDecor(ctx, true);
  }
  else {
    TypesMgr::TypeId t1 = Symbols.getType(ident);
    putTypeDecor(ctx, t1);
    if (Symbols.isFunctionClass(ident))
      putIsLValueDecor(ctx, false);
    else
      putIsLValueDecor(ctx, true);
  }
  DEBUG_EXIT();
}

// void TypeCheckListener::enterEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_ENTER();
// }
// void TypeCheckListener::exitEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_EXIT();
// }
// void TypeCheckListener::visitTerminal(antlr4::tree::TerminalNode *node) {
//   DEBUG("visitTerminal");
// }
// void TypeCheckListener::visitErrorNode(antlr4::tree::ErrorNode *node) {
// }


// Getters for the necessary tree node atributes:
//   Scope, Type ans IsLValue
SymTable::ScopeId TypeCheckListener::getScopeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getScope(ctx);
}
TypesMgr::TypeId TypeCheckListener::getTypeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getType(ctx);
}
bool TypeCheckListener::getIsLValueDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getIsLValue(ctx);
}

// Setters for the necessary tree node attributes:
//   Scope, Type ans IsLValue
void TypeCheckListener::putScopeDecor(antlr4::ParserRuleContext *ctx, SymTable::ScopeId s) {
  Decorations.putScope(ctx, s);
}
void TypeCheckListener::putTypeDecor(antlr4::ParserRuleContext *ctx, TypesMgr::TypeId t) {
  Decorations.putType(ctx, t);
}
void TypeCheckListener::putIsLValueDecor(antlr4::ParserRuleContext *ctx, bool b) {
  Decorations.putIsLValue(ctx, b);
}
