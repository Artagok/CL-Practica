
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

  TypesMgr::TypeId tRet; // RETURN type
  if (ctx->basic_type() != NULL)
    tRet = getTypeDecor(ctx->basic_type());
  else 
    tRet = Types.createVoidTy();
  Symbols.setCurrentFunctionTy(tRet);

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

  // Already checked if ID is undeclared in exitIdent
  TypesMgr::TypeId tID = getTypeDecor(ctx->ident());

  // Check if it is callable or not
  if (not Types.isErrorTy(tID) and not Types.isFunctionTy(tID)) {
    Errors.isNotCallable(ctx->ident());
  }

  // OK, tID is callable
  else if (not Types.isErrorTy(tID)) {

    // Check if #params in call matches function definition
    if (Types.getNumOfParameters(tID) != (std::size_t) (ctx->expr()).size()) {
      Errors.numberOfParameters(ctx->ident());
    }

    // Check if params are all of the expected type
    else {

      auto params = Types.getFuncParamsTypes(tID);

      for (uint i = 0; i < params.size(); ++i) {
        // We found a param that has different type
        if (not Types.equalTypes(params[i], getTypeDecor(ctx->expr(i)))) {
          if (not (Types.isFloatTy(params[i]) and Types.isIntegerTy(getTypeDecor(ctx->expr(i)))))
            Errors.incompatibleParameter(ctx->expr(i), i+1, ctx);
        }
        /* Maybe they were vectors, check their elem type to be equal as well || En principi aquest bloc no cal, equalTypes compara be arrays
        else if (Types.isArrayTy(params[i]) and not Types.equalTypes(Types.getArrayElemType(params[i]), 
                                                                     Types.getArrayElemType(Types.getParameterType(tID,i))))
        {
          Errors.incompatibleParameter(ctx->expr(i), i+1, ctx); // I assumed this error!
        }*/
      }
    }
  }
  
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
}

void TypeCheckListener::enterReturnStmt(AslParser::ReturnStmtContext * ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitReturnStmt(AslParser::ReturnStmtContext * ctx) {

  // Returning SOMETHING
  if (ctx->expr() != NULL) {
   // std::cout << "Func type: " << Types.to_string(Symbols.getCurrentFunctionTy()) << std::endl;
    TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
   // std::cout << "Expr type: " << Types.to_string(t1) << std::endl;
  
    // return type is Non-Primitive
    if (not Types.isErrorTy(t1) and not Types.isPrimitiveNonVoidTy(t1))
      Errors.incompatibleReturn(ctx->RETURN());

    // return type does not match
    else if (not Types.isErrorTy(t1) and not Types.equalTypes(t1, Symbols.getCurrentFunctionTy())) {
      if (not (Types.equalTypes(Types.createFloatTy(), Symbols.getCurrentFunctionTy()) and 
               Types.equalTypes(Types.createIntegerTy(), t1)))
        Errors.incompatibleReturn(ctx->RETURN());
    }

    // function was void
    else if (not Types.isErrorTy(t1) and Types.equalTypes(Types.createVoidTy(),Symbols.getCurrentFunctionTy()))
      Errors.incompatibleReturn(ctx->RETURN());
  }

  // Returning VOID
  else {
    
    if (not Types.equalTypes(Types.createVoidTy(), Symbols.getCurrentFunctionTy()))
      Errors.incompatibleReturn(ctx->RETURN());
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

  //TypesMgr::TypeId tArr = Types.createErrorTy();
  TypesMgr::TypeId t;
  TypesMgr::TypeId tID = getTypeDecor(ctx->ident());
  bool b = getIsLValueDecor(ctx->ident());

  // Left Expr is an array access 
  if (ctx->expr() != NULL) {
    
    t = getTypeDecor(ctx->expr());
    bool synt_valid = not Types.isErrorTy(tID);

    // tID is not an array
    if (not Types.isErrorTy(tID) and not Types.isArrayTy(tID)) {
      Errors.nonArrayInArrayAccess(ctx);
      tID = Types.createErrorTy();
      b = false;
      synt_valid = false;
    }

    // indexing with a non-int
    if (not Types.isErrorTy(t) and not Types.isIntegerTy(t)) {
      Errors.nonIntegerIndexInArrayAccess(ctx->expr());
      synt_valid = false;
      // I assume this is not a reason to carry up an error
    }

    // Sintactically valid array access [not necessarily sintactically valid]
    if (synt_valid) {
      tID = Types.getArrayElemType(tID);
      b = true;
    }
  }

  // Left Expr is NOT an array access
  else {
  }

  putTypeDecor(ctx, tID);
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

  TypesMgr::TypeId t = getTypeDecor(ctx->expr());
  TypesMgr::TypeId tID = getTypeDecor(ctx->ident());
  
  if (not Types.isErrorTy(tID)) {  
  
    if (not Types.isArrayTy(tID))
      Errors.nonArrayInArrayAccess(ctx);  
    // tArr pren per valor el tipus de l'array en questio
    else tArr = Types.getArrayElemType(tID); 
  }

  if (not Types.isErrorTy(t) and not Types.isIntegerTy(t))
    Errors.nonIntegerIndexInArrayAccess(ctx->expr());  

  putTypeDecor(ctx, tArr);
  putIsLValueDecor(ctx, true);
  DEBUG_EXIT();
}


void TypeCheckListener::enterFuncCall(AslParser::FuncCallContext * ctx) {
  DEBUG_ENTER();
}
// funcCall is an expr, meaning is a function not an action, so it RETURNS some value
void TypeCheckListener::exitFuncCall(AslParser::FuncCallContext * ctx) {

  // Already checked if ID is undeclared in exitIdent || Aqui potser falta comprovar que tID no sigui error (com es fa a procCall)
  TypesMgr::TypeId tID = getTypeDecor(ctx->ident());
  // Si no es funcio farem put de Error
  TypesMgr::TypeId t = Types.createErrorTy();

  // Check if it is callable or not
  if (not Types.isFunctionTy(tID)) {
    Errors.isNotCallable(ctx->ident());
  }

  // Callable
  else {

    t = Types.getFuncReturnType(tID);

    // Check if it is a procedure (action), meaning it cannot return
    if (Types.isVoidFunction(tID)) {
      Errors.isNotFunction(ctx->ident());
      t = Types.createErrorTy();
    }

    // Check if #params in call matches function definition
    if (Types.getNumOfParameters(tID) != (std::size_t) (ctx->expr()).size()) {
      Errors.numberOfParameters(ctx->ident());
    }

    // Check if params are all of the expected type
    else {

      auto params = Types.getFuncParamsTypes(tID);

      for (uint i = 0; i < params.size(); ++i) {
        // We found a param that has different type
        if (not Types.equalTypes(params[i], getTypeDecor(ctx->expr(i)))) {
          if (not (Types.isFloatTy(params[i]) and Types.isIntegerTy(getTypeDecor(ctx->expr(i)))))
            Errors.incompatibleParameter(ctx->expr(i), i+1, ctx);
        }
        /* Maybe they were vectors, check their elem type to be equal as well || aquest bloc no cal, en principi equalTypes compara be arrays
        else if (Types.isArrayTy(params[i]) and not Types.equalTypes(Types.getArrayElemType(params[i]), 
                                                                     Types.getArrayElemType(Types.getParameterType(tID,i))))
        {
          Errors.incompatibleParameter(ctx->expr(i), i+1, ctx); // I assumed this error!
        }*/
      }
    }
  }
  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
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
  TypesMgr::TypeId t = Types.createIntegerTy();

// Special case for % (mod) operation, t1 & t2 must be integers
  if (ctx->MOD()) {
    if ((not Types.isErrorTy(t1) and not Types.isIntegerTy(t1)) or 
        (not Types.isErrorTy(t2) and not Types.isIntegerTy(t2)))
      Errors.incompatibleOperator(ctx->op);
  }
  // MUL, DIV, ADD, SUB
  else {
    if (((not Types.isErrorTy(t1)) and (not Types.isNumericTy(t1))) or
        ((not Types.isErrorTy(t2)) and (not Types.isNumericTy(t2))))
      Errors.incompatibleOperator(ctx->op);
    if (Types.isFloatTy(t1) or Types.isFloatTy(t2)) t = Types.createFloatTy();
  }
  
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

void TypeCheckListener::enterParenthesis(AslParser::ParenthesisContext * ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitParenthesis(AslParser::ParenthesisContext * ctx) {
  
  putTypeDecor(ctx, getTypeDecor(ctx->expr()));
  putIsLValueDecor(ctx, getIsLValueDecor(ctx->expr()));
  
  DEBUG_EXIT();
}

void TypeCheckListener::enterValue(AslParser::ValueContext *ctx) {
  DEBUG_ENTER();
}
void TypeCheckListener::exitValue(AslParser::ValueContext *ctx) {

  TypesMgr::TypeId t;
  
  if (ctx->INTVAL())        t = Types.createIntegerTy();
  else if (ctx->FLOATVAL()) t = Types.createFloatTy();
  else if (ctx->BOOLVAL())  t = Types.createBooleanTy();
  else if (ctx->CHARVAL())  t = Types.createCharacterTy();
  else                      t = Types.createErrorTy();

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
