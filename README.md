# CL-Practica
## Grau-CL (Compiladors) ~ 2018/2019-Q2

+ **Nil Vilas**
+ **Pau Núñez**

### Listeners

* SymbolsListener: Variable declaration and fill up Symbols Table
* TypeCheckListener: Semantic type-check, compile time errors (typing)
* CodeGenListener: Three-address code generation. (t-code due to high temporal register usage)

### Main

1. Lexer (char stream -> token stream)
2. Parser (parses token stream)
3. Generate parse tree
4. Check for lexical or syntactical errors
5. Walker (checking variable types or generating code)
6. Listener that looks for var and func declarations in the tree and stores required information **(SymbolsListener)**
7. Traverse the tree using this listener, to collect information about declared identifiers
8. Listener that will perform type checkings wherever it is needed (on expressions, assignments, parameter passing, etc) **(TypeCheckListener)**
9. Traverse the tree using this listener, so all types are checked
10. Listener that will generate code for each part of the tree **(CodeGenListener)**
11. Traverse the tree using this listener, so code is generated and stored in 'mycode'
12. Print generated code
