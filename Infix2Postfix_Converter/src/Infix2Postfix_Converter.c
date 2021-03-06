/*
 ============================================================================
 Name        : Infix2Postfix_Converter.c
 Author      : Michael A. Morris
 Version     : 0.0.1
 Copyright   : Copyright 2017, GPLv3
 Description : Infix To Postfix Converter in C, ANSI-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "error.h"
#include "ScannerPascal.h"

/*
 * Externals
 */

extern TOKEN_CODE token;
extern char       token_string[];

/*
 * Globals
 */

char postfix[MAX_PRINT_LINE_LENGTH];
char *pp;

/*
 * Function prototypes
 */

void expression(void);
void simple_expression(void);
void term(void);
void factor(void);
void output_postfix(char *);


int main(int argc, char *argv[])
{
	/*
	 * Initialize Scanner
	 */

	init_scanner(argv[1]);

	/*
	 * Process tokens using by calling expression parsing function until
	 * a PERIOD or the end of file is found.
	 */

	do {
		strcpy(postfix, ">> ");
		pp = postfix + strlen(postfix);

		get_token(); expression();

		output_postfix("\n");
		print_line(postfix);


		/*
		 * After an expression, there should be a semicolon, a period, or
		 * an EOF. If not, skip tokens until there is such a token.
		 */

		while(   (SEMICOLON != token)
			  && (PERIOD != token)
			  && (END_OF_FILE != token) ) {
			error(INVALID_EXPRESSION);
			get_token();
		}
	} while((PERIOD != token) && (END_OF_FILE != token));

	return EXIT_SUCCESS;
}

/*
 * void expression(void)    Process an expression, which is just a simple
 *                          expression at this time.
 */

void expression(void)
{
	simple_expression();
}

/*
 * void simple_expression(void)     Process a simple expression consisting
 *                                  terms separated by '+' or '-' operators.
 *
 *  Syntax diagram for a simple expression:
 *
 *      term {{+, -} term}
 */

void simple_expression(void)
{
	TOKEN_CODE op;
	char       *op_string;

	term();     // simple expressions start with a term.

	/*
	 * Loop to process subsequent terms separated by operators.
	 */

	while((PLUS == token) || (MINUS == token)) {
		op = token;     // remember the operator

		get_token(); term();

		switch(op) {
		case PLUS :
			op_string = "+";
			break;

		case MINUS :
			op_string = "-";
			break;

		default :
			break;
		}
		output_postfix(op_string);
	}
}

/*
 * void term(void)      Process a term consisting of factors separated by
 *                      operators '*' or '/'.
 *
 *  Syntax diagram for a term:
 *
 *      factor {{*, /} factor}
 */

void term(void)
{
	TOKEN_CODE op;
	char       *op_string;

	factor();

	/*
	 * Loop to process subsequent factors separated by operators.
	 */

	while((STAR == token) || (SLASH == token)) {
		op = token;     // remember the operator

		get_token(); factor();

		switch(op) {
		case STAR :
			op_string = "*";
			break;

		case SLASH :
			op_string = "/";
			break;

		default :
			break;
		}
		output_postfix(op_string);
	}
}

/*
 * void factor(void)    Process a factor, which is an identifier, a number,
 *                      or another expression inside of parenthesis.
 */

void factor(void)
{
	if((IDENTIFIER == token) || (NUMBER == token)) {
		output_postfix(token_string);
		get_token();
	} else if(LPAREN == token) {
		get_token(); expression();  // recursive call to expression()

		if(RPAREN == token) {
			get_token();
		} else {
			error(MISSING_RPAREN);
		}
	} else {
		error(INVALID_EXPRESSION);
	}
}

/*
 * void output_postfix(char *)  Append the input string, preceded by a
 *                              blank to the postfix buffer.
 */

void output_postfix(char *string)
{
	*pp++ = ' ';
	*pp   = '\0';
	strcat(pp, string);
	pp += strlen(string);
}
