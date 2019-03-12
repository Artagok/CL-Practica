/*============================================*/
/* ASL (A Simple Language) grammar file       */
/* Nil Vilas & Pau Nunez | 2018/2019-Q2       */
/*============================================*/

grammar Asl;

/*============================================*/
/* Parser Rules                               */
/*============================================*/

// A program is a list of functions
program : function+ EOF
        ;

// A function has a name, a list of parameters and a list of statements
function
        : FUNC ID '(' func_decl_params ')' ( | ':' basic_type) declarations statements ENDFUNC
        ;

func_decl_params
        : ( | ID ':' type (',' ID ':' type)* )
        ;

declarations
        : (variable_decl)*
        ;

array_decl
        : ARRAY '[' INTVAL ']' 'of' basic_type
        ;

variable_decl
        : VAR ID (',' ID)* ':' type
        ;

type    : basic_type | array_decl
        ;

basic_type 
        : INT | FLOAT | BOOL | CHAR
        ;

statements
        : (statement)*
        ;

// The different types of instructions
statement
          // Assignment
        : left_expr ASSIGN expr ';'                           # assignStmt
          // if-then-else statement (else is optional)
        | IF expr THEN statements (ELSE statements)? ENDIF    # ifStmt
          // while-do-endwhile statement
        | WHILE expr 'do' statements ENDWHILE                 # whileStmt
          // A function/procedure call has a list of arguments in parenthesis (possibly empty)
        | ident '(' (expr (',' expr)*)? ')' ';'               # procCall
          // Return statement
        | RETURN expr? ';'                                    # returnStmt
          // Read a variable
        | READ left_expr ';'                                  # readStmt
          // Write an expression
        | WRITE expr ';'                                      # writeExpr
          // Write a string
        | WRITE STRING ';'                                    # writeString
        ;
// Grammar for left expressions (l-values in C++)
left_expr
        : ident ('[' expr ']')?
        ;

// Grammar for expressions with boolean, relational and arithmetic operators
expr    : '(' expr ')'                                            # parenthesis
        | ID '[' expr ']'                                         # arrayIndex
        | ID '(' (expr (',' expr)*)? ')'                          # funcCall
        | op=(NOT|ADD|SUB) expr                                   # unary
        | expr op=(MUL|DIV|MOD) expr                              # arithmetic
        | expr op=(ADD|SUB) expr                                  # arithmetic
        | expr op=(EQ|NEQ|GT|GTE|LT|LTE) expr                     # relational
        | expr op=AND expr                                        # logical
        | expr op=OR expr                                         # logical
        | (INTVAL|FLOATVAL|BOOLVAL|CHARVAL)                       # value
        | ident                                                   # exprIdent
        ;

ident   : ID
        ;

/*============================================*/
/* Lexer Rules                                */
/*============================================*/

ASSIGN    : '=' ;

/*=== Arithmetic ===*/
ADD       : '+' ;
SUB       : '-' ;
MUL       : '*' ;
DIV       : '/' ;
MOD       : '%' ;

/*=== Relational ===*/
EQ        : '==' ;
NEQ       : '!=' ;
GT        : '>' ;
GTE       : '>=' ;
LT        : '<' ;
LTE       : '<=' ;

/*=== Logical ===*/
AND       : 'and' ;
OR        : 'or' ;
NOT       : 'not' ;
TRUE      : 'true' ;
FALSE     : 'false' ;

/*=== Var & Types ===*/
VAR       : 'var' ;
INT       : 'int' ;
FLOAT     : 'float' ;
BOOL      : 'bool' ;
CHAR      : 'char' ;
ARRAY     : 'array' ;

/*=== Control Struct ===*/
IF        : 'if' ;
THEN      : 'then' ;
ELSE      : 'else' ;
ENDIF     : 'endif' ;
WHILE     : 'while' ;
ENDWHILE  : 'endwhile' ;

FUNC      : 'func' ;
ENDFUNC   : 'endfunc' ;
RETURN    : 'return' ;

READ      : 'read' ;
WRITE     : 'write' ;

ID        : ('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'_'|'0'..'9')* ;

/*=== Literal Values ===*/
INTVAL    : ('0'..'9')+ ;
FLOATVAL  : ('0'..'9')+ '.' ('0'..'9')+ ;
BOOLVAL   : TRUE | FALSE ;
CHARVAL   : '\'' ~('\'' | '\\') '\'' ;


// Strings (in quotes) with escape sequences
STRING    : '"' ( ESC_SEQ | ~('\\'|'"') )* '"' ;

fragment
ESC_SEQ   : '\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\') ;

// Comments (inline C++-style)
COMMENT   : '//' ~('\n'|'\r')* '\r'? '\n' -> skip ;

// White spaces
WS        : (' '|'\t'|'\r'|'\n')+ -> skip ;
// Alternative description
// WS        : [ \t\r\n]+ -> skip ;
