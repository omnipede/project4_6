%{
	#include "globals.h"
	#include "util.h"
	#include "scan.h"
	#define YY_NO_UNPUT
	char tokenString[MAXTOKENLEN+1];
%}

digit		[0-9]
number		{digit}+
letter		[a-zA-Z]
identifier	{letter}+
newline		\n
whitespace	[ \t]+

%%
"else"			{return ELSE;}
"if"			{return IF;}
"int"			{return INT;}
"return"		{return RETURN;}
"void"			{return VOID;}
"while"			{return WHILE;}

"+"				{return PLUS;}
"-"				{return MINUS;}
"*"				{return TIMES;}
"/"				{return OVER;}
"<"				{return LT;}
"<="			{return LE;}
">"				{return GT;}
">="			{return GE;}
"=="			{return EQ;}
"!="			{return NE;}
"="				{return ASSIGN;}
";"				{return SEMI;}
","				{return COMMA;}
"("				{return LPAREN;}
")"				{return RPAREN;}
"["				{return LSQUARE;}
"]"				{return RSQUARE;}
"{"				{return LCURLY;}
"}"				{return RCURLY;}

{number}		{return NUM;}
{identifier}	{return ID;}
{newline}		{lineno++;}
{whitespace}	{/* skip whitespace. */}
"/*"			{ 
					int c, temp_lineno;
					temp_lineno = lineno;
					while(1) {
						while( (c = input() ) != '*' && c != EOF){
							if (c == '\n')
								temp_lineno++;
						}
						if ( c == '*' ) {
							while( (c = input()) == '*')
								;
							if (c == '\n')
								temp_lineno++;
							if (c == '/') {
								lineno = temp_lineno;
								break; /* Found the end. */
							}
						}
						if ( c == EOF ) {/* Error occured.*/
							lineno = temp_lineno;
							return ERROR_IN_COMMENT;
						}
					}	
			  	}
<<EOF>>			{return ENDFILE;}
.				{
					return ERROR;
				}
%%

TokenType getToken(void) {
	static int firstTime = TRUE;
	TokenType currentToken;
	if (firstTime) {
		firstTime = FALSE;
		lineno++;
		yyin = source;
		yyout = listing;
	}
	currentToken = yylex();
	strncpy(tokenString, yytext, MAXTOKENLEN);
	if (TraceScan) {
		fprintf(listing,"\t%d\t", lineno);
		printToken(currentToken, tokenString);
	}
	return currentToken;
}

int yywrap (void) {
	return 1;
}
