/*
 * @file	scanner.c
 * @brief	The scanner for SIMPL-2021.
 * @author	W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date	2021-08-23
 */

#include "scanner.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"
#include "error.h"
#include "token.h"

/* --- type definitions and constants --------------------------------------- */

typedef struct {
	char *word;		/* the reserved word, i.e., the lexeme */
	TokenType type; /* the associated token type		   */
} ReservedWord;

/* -------------------------------------------------------------------------- */

static FILE *src_file;	  /* the source file pointer			 */
static int ch;			  /* the next source character			 */
static int column_number; /* the current column number			 */

/* reserved words */
static ReservedWord reserved[] = {
	{"and", TOK_AND}, {"array", TOK_ARRAY},
	{"begin", TOK_BEGIN}, {"boolean", TOK_BOOLEAN},
	{"chill", TOK_CHILL}, {"define", TOK_DEFINE},
	{"do", TOK_DO},	{"else", TOK_ELSE},
	{"elsif", TOK_ELSIF}, {"end", TOK_END},
	{"exit", TOK_EXIT}, {"false", TOK_FALSE},
	{"if", TOK_IF},	{"integer", TOK_INTEGER},
	{"mod", TOK_MOD}, {"not", TOK_NOT},
	{"or", TOK_OR},	{"program", TOK_PROGRAM},
	{"read", TOK_READ}, {"then", TOK_THEN},
	{"true", TOK_TRUE}, {"while", TOK_WHILE},
	{"write", TOK_WRITE}
	/* TODO: Populate this array with the appropriate pairs of reserved word
	 * strings and token types, sorted alphabetically by reserved word string.
	 */
};

#define NUM_RESERVED_WORDS (sizeof(reserved) / sizeof(ReservedWord))
#define MAX_INITIAL_STRLEN (1024)

/* --- function prototypes -------------------------------------------------- */

static void next_char(void);
static void process_number(Token *token);
static void process_string(Token *token);
static void process_word(Token *token);
static void skip_comment(void);
void append(char* s, char ch);

/* --- scanner interface ---------------------------------------------------- */

void init_scanner(FILE *in_file)
{
	src_file = in_file;
	position.line = 1;
	position.col = column_number = 0;
	next_char();
}

void get_token(Token *token)
{
	/* remove whitespace */
	/* TODO: Skip all whitespace characters before the start of the token. */
	if (isspace(ch)) {
		while (isspace(ch)) {
			next_char();
		}
	}
	/* remember token start */
	position.col = column_number;
	/* get next token*/
	if (ch != EOF) {
		if (isalpha(ch) || ch == '_') {
			/* process a word */
			process_word(token);

		} else if (isdigit(ch)) {
			/* process a number */
			process_number(token);

		} else
			switch (ch) {
				/* process a string */
				case '"':
					position.col = column_number;
					next_char();
					process_string(token);
					break;

				/* TODO: Process the other tokens, and trigger comment skipping.
				 */
				case '=':
					next_char();
					token->type = TOK_EQ;
					break;
				case '>':
					next_char();
					if (ch == '=') {
						next_char();
						token->type = TOK_GE;
						break;
					} else {
						token->type = TOK_GT;
						break;
					}
				case '<':
					next_char();
					if (ch == '=') {
						next_char();
						token->type = TOK_LE;
						break;
					} else if (ch == '-') {
						next_char();
						token->type = TOK_GETS;
						break;
					} else {
						token->type = TOK_LT;
						break;
					}
				case '#':
					next_char();
					token->type = TOK_NE;
					break;
				case '-':
					next_char();
					if (ch == '>') {
						next_char();
						token->type = TOK_TO;
						break;
					} else {
						token->type = TOK_MINUS;
						break;
					}
				case '+':
					next_char();
					token->type = TOK_PLUS;
					break;
				case '/':
					next_char();
					token->type = TOK_DIV;
					break;
				case '*':
					next_char();
					token->type = TOK_MUL;
					break;
				case '%':
					next_char();
					token->type = TOK_MOD;
					break;
				case '&':
					next_char();
					token->type = TOK_AMPERSAND;
					break;
				case '[':
					next_char();
					token->type = TOK_LBRACK;
					break;
				case ']':
					next_char();
					token->type = TOK_RBRACK;
					break;
				case ',':
					next_char();
					token->type = TOK_COMMA;
					break;
				case '(':
					position.col = column_number;
					next_char();
					if (ch == '*') {
						skip_comment();
						get_token(token);
					} else {
						token->type = TOK_LPAR;
					}
					break;
				case ')':
					next_char();
					token->type = TOK_RPAR;
					break;
				case ';':
					next_char();
					token->type = TOK_SEMICOLON;
					break;
				default:
					leprintf("illegal character '%c' (ASCII #%d)", ch, ch);
			}

	} else {
		token->type = TOK_EOF;
	}
}

/* --- utility functions ---------------------------------------------------- */

void next_char(void)
{
	/*static char last_read = '\0';*/
	static char new_line;
	ch = getc(src_file);

	if (new_line == '\n') {
		position.line++;
		column_number = 1;
		new_line = 'a';
	}
	column_number++;
	if (ch == '\n') {
		new_line = ch;
	}

	/* TODO:
	 *	Get the next character from the source file.
	 * - Increment the line number if the previous character is EOL.
	 * - Reset the global column number when necessary.
	 * - DO NOT USE feof!!!
	 */
}

void process_number(Token *token)
{
	int i, v, x;
	char d1[12];
	char d2[2];
	int *sum = (int *)malloc(sizeof(int));
	/*if (ch == '-') {
		append(d1, ch);
		next_char();
	}*/
	i = ch - '0';
	sprintf(d1, "%i", i);	
	while (isdigit(ch)) {
		v = atoi(d1);
		next_char();
		/*if (!isdigit(ch)) {
			break;
		}*/
		i = ch - '0';
		*sum = v + i;
		if (v > 0 && i > 0 && *sum < 0){
			x = -1;
		}
		if (v < 0 && i < 0 && *sum > 0) {
			x = -1;
		} else {
			x = 0;
		}
		if (x == -1){
			if (position.line > 2) {
				SourcePos start_pos = {position.line - 1, position.col - 1};
				position = start_pos;
			} else {
				if (position.col > 1) {
					SourcePos start_pos = {position.line, position.col - 1};
					position = start_pos;
				} else {
					SourcePos start_pos = {position.line, position.col};
                                        position = start_pos;
				}
			}
			leprintf("number too large");
		}
		if (!isdigit(ch)) {
			break;
		}
		sprintf(d2, "%i", i);
		strcat(d1, d2);
	}

	i = atoi(d1);
	token->value = i;
	token->type = TOK_NUM;
	/* TODO:
	 * - Build number up to the specified maximum magnitude.
	 * - Store the value in the appropriatetoken field.
	 * - Set the appropriate token type.
	 * - "Remember" the correct column number globally.
	 */
}

void process_string(Token *token)
{
	size_t j, nstring = MAX_INITIAL_STRLEN;
	token->string = (char *)malloc(nstring);
	j = 0;
	int line = position.line;

	while (ch != '"') {
		if (ch == '"') {
			break;
		}
		if (ch < 32 && ch != -1 && ch != '\\') {
			if (position.line > 1) {
                        	SourcePos start_pos = {line, column_number - 1};
                        	position = start_pos;
			} else {
				SourcePos start_pos = {line, column_number};
                                position = start_pos;
			}
                        leprintf("non-printable character (ASCII #%d) in string", ch);
                }
		if (ch == EOF) {
			int pos;
			if (position.col == 1) {
				pos = 1;
			} else {
				pos = position.col - 1;
			}
			SourcePos start = {line, pos};
			position = start;
			leprintf("string not closed");
		}
		j++;
		if (j > nstring) {
			token->string = (char *)realloc(token->string, 2 * nstring);
		}
		if (ch == '\\') {
			append(token->string, ch);
			char ec[3];
			append(ec, ch);
			next_char();
			j++;
			if (ch != 'n' && ch != 't' && ch != '"' && ch != '\\') {
				append(ec, ch);
				SourcePos start = {line, column_number - 1};
				position = start;
				leprintf("illegal escape code '%s' in string", ec);
			}
		}
		append(token->string, ch);
		next_char();
	}

	next_char();
	token->type = TOK_STR;
	/* TODO:
	 * - Allocate heap space of the size of the maximum initial string length.
	 * - If a string is *about* to overflow while scanning it, double the amount
	 *	 of space available.
	 * - *Only* printable ASCII characters are allowed; see man 3 isalpha.
	 * - Check the legality of escape codes.
	 * - Set the appropriate token type.
	 */
}

void process_word(Token *token)
{
	char lexeme[MAX_ID_LENGTH + 1] = "";
	int cmp, low, mid, high;
	cmp = -1;
	low = 0;
	high = NUM_RESERVED_WORDS;

	int length = 0;
	while (isalpha(ch) || ch == '_' || isdigit(ch)) {
		length++;
		/* check that the id length is less than the maximum */
		/* TODO */
		if (length > MAX_ID_LENGTH) {
			if (position.line > 1) {
				SourcePos start_pos = {position.line, position.col - 1};
				position = start_pos;
			} else {
				SourcePos start_pos = {position.line, position.col};
				position = start_pos;
			}
			leprintf("identifier too long");
		}
		if (!isalpha(ch) && ch != '_' && !isdigit(ch)) {
			break;
		}
		append(lexeme, ch);
		next_char();
	}

	/* do a binary search through the array of reserved words */
	/* TODO */
	while (low <= high) {
		mid = (low + high) / 2;
		if (strcmp(lexeme, reserved[mid].word) == 0) {
			cmp = 0;
			break;
		} else if (strcmp(lexeme, reserved[mid].word) > 0) {
			low = mid + 1;
		} else {
			high = mid - 1;
		}
	}

	/* if id was not recognised as a reserved word, it is an identifier */
	if (cmp == 0) {
		token->type = reserved[mid].type;
	} else {
		token->type = TOK_ID;
	}
	
	strcpy(token->lexeme, lexeme);
}

void skip_comment(void)
{
	SourcePos start_pos = {position.line, column_number - 2};
	while (ch != EOF) {
        	if (ch == '(') {
        		next_char();
			if (ch == '*') {
				skip_comment();
			}
		} else if (ch == '*') {
			next_char();
			if (ch == ')') {
				next_char();
				return;
			}
		} else {
			next_char();
        	}
	}
	position = start_pos;
	leprintf("comment not closed");
	/* TODO:
	 * - Skip nested comments *recursively*, which is to say, counting
	 *	 strategies are not allowed.
	 * - Terminate with an error if comments are not nested properly.
	 */

	/* force the line number of error reporting */
}

void append(char* s, char ch)
{
        int len = strlen(s);
        s[len] = ch;
        s[len+1] = '\0';
}
