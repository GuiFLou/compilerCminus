/****************************************************/
/* File: Scanner.l                                  */
/* Lex specification for C-                         */ 
/*                                                  */
/* Laís Pereira e Guilherme Ferreira                */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"

char tokenString[MAXTOKENLEN+1];
static int yylex(void);
%}

digito       [0-9]
NUM         {digito}+
letra       [a-zA-Z]
ID          {letra}+
newline     [\n]
whitespace  [ \t\r]+
special     [+ - * / < <= > >= == != = ; , () [] {} /* */]
%%
"/*"            { char c;
                  do
                  { c = input();
                    if (c == EOF) break;
                    if (c == '\n') lineno++;
                    if (c == '*') 
                    { 
                      if(input() == '/')
                      break;
                    }   
                  } while (1);
                }
"if"            {return IF;}
"else"          {return ELSE;}
"int"           {return INT;}
"return"        {return RETURN;}
"void"          {return VOID;}
"while"         {return WHILE;}
"="             {return ASSIGN;}
"=="            {return EQ;}
"<"             {return LT;}
">"             {return GT;}
"<="            {return LTE;}
">="            {return GTE;}
"!="            {return NE;}
"["             {return LBRACKET;}
"]"             {return RBRACKET;}
"{"             {return LKEYS;}
"}"             {return RKEYS;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"("             {return LPAREN;}
")"             {return RPAREN;}
";"             {return SEMI;}
","             {return COMMA;}
{NUM}           {return NUM;}
{ID}            {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
.               {return ERROR;}

%%

TokenType getToken(void)
{ 
  static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { 
    firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) 
  {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

int yywrap(void)
{
  return 1;
}
