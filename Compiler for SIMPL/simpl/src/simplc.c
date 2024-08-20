/**
 * @file    simplc.c
 *
 * A recursive-descent compiler for the SIMPL-2021 language.
 *
 * All scanning errors are handled in the scanner.  Parser errors MUST be
 * handled by the <code>abort_c</code> function.  System and environment errors,
 * for example, running out of memory, MUST be handled in the unit in which they
 * occur.  Transient errors, for example, non-existent files, MUST be reported
 * where they occur.  There are no warnings, which is to say, all errors are
 * fatal and MUST cause compilation to terminate with an abnormal error code.
 *
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2021-08-23
 */

/* TODO: Include the appropriate system and project header file. */
#include <stdlib.h>
#include <string.h>

#include "boolean.h"
#include "codegen.h"
#include "errmsg.h"
#include "error.h"
#include "hashtable.h"
#include "jvm.h"
#include "scanner.h"
#include "symboltable.h"
#include "token.h"
#include "valtypes.h"
/* --- debugging ------------------------------------------------------------ */

/* TODO: Your Makefile has a variable called DFLAGS.  If it is set to contain
 * -DDEBUG_PARSER, it will cause the following prototypes to be included, and
 *  the functions to which they refer (given at the end of this file) will be
 *  compiled.  If, on the other hand, this flag is comment out, by setting
 *  DFLAGS to #-DDEBUG_PARSER, these functions will be excluded.  These
 *  definitions should be used at the start and end of every parse function.
 *  For an example, see the provided parse_program function.
 */

#ifdef DEBUG_PARSER
void debug_start(const char *fmt, ...);
void debug_end(const char *fmt, ...);
void debug_info(const char *fmt, ...);
#define DBG_start(...) debug_start(__VA_ARGS__)
#define DBG_end(...) debug_end(__VA_ARGS__)
#define DBG_info(...) debug_info(__VA_ARGS__)
#else
#define DBG_start(...)
#define DBG_end(...)
#define DBG_info(...)
#endif /* DEBUG_PARSER */

/* --- type definitions ----------------------------------------------------- */

/* TODO: Uncomment the following for use during type checking. */

#if 1
typedef struct variable_s Variable;
struct variable_s {
	char *id;		/**< variable identifier                       */
	ValType type;	/**< variable type                             */
	SourcePos pos;	/**< variable position in the source           */
	Variable *next; /**< pointer to the next variable in the list  */
};
#endif

/* --- global variables ----------------------------------------------------- */

Token token;	/**< the lookahead token.type                  */
FILE *src_file; /**< the source code file                      */
#if 1
ValType return_type; /**< the return type of the current subroutine */
#endif
static int relop;
static int flag = 0;
/*static int index_flag;*/
/* TODO: Uncomment the previous definition for use during type checking. */

/* --- helper macros -------------------------------------------------------- */

#define IS_STATEMENT(toktype)                                         \
	(toktype == TOK_EXIT || toktype == TOK_IF || toktype == TOK_ID || \
	 toktype == TOK_READ || toktype == TOK_WHILE || toktype == TOK_WRITE)

#define STARTS_FACTOR(toktype)                                         \
	(toktype == TOK_ID || toktype == TOK_NUM || toktype == TOK_LPAR || \
	 toktype == TOK_NOT || toktype == TOK_TRUE || toktype == TOK_FALSE)

#define STARTS_EXPR(toktype) (STARTS_FACTOR(toktype) || toktype == TOK_MINUS)
/* TODO */

#define STARTS_EXPR_2(toktype)                                          \
	(toktype == TOK_NUM || toktype == TOK_LPAR || toktype == TOK_NOT || \
	 toktype == TOK_TRUE || toktype == TOK_FALSE || toktype == TOK_MINUS)

#define IS_ADDOP(toktype) (toktype >= TOK_MINUS && toktype <= TOK_PLUS)

#define IS_MULOP(toktype)                                              \
	(toktype == TOK_AND || toktype == TOK_DIV || toktype == TOK_MUL || \
	 toktype == TOK_MOD)
/* TODO */

#define IS_ORDOP(toktype)                                           \
	(toktype == TOK_GE || toktype == TOK_GT || toktype == TOK_LE || \
	 toktype == TOK_LT)
/* TODO */

#define IS_RELOP(toktype)                                           \
	(toktype == TOK_EQ || toktype == TOK_GE || toktype == TOK_GT || \
	 toktype == TOK_LE || toktype == TOK_LT || toktype == TOK_NE)
/* TODO */

#define IS_TYPE_TOKEN(toktype) \
	(toktype == TOK_BOOLEAN || toktype == TOK_INTEGER)
/* TODO */

/* --- function prototypes: parsing ----------------------------------------- */

void parse_program(void);
void parse_funcdef(void);
void parse_body(void);
void parse_type(ValType *type);
void parse_vardef(void);
void parse_statement(void);
void parse_statements(void);
void parse_exit(void);
void parse_if(void);
void parse_name(void);
void parse_read(void);
void parse_while(void);
void parse_write(void);
void parse_index(char *id);
void parse_simple(ValType *type);
void parse_term(ValType *type);
void parse_factor(ValType *type);
void parse_arglist(char *id, IDprop *prop);
void parse_expr(ValType *type);
/* TODO: Add the prototypes for the rest of the parse function. */

/* --- function prototypes: helpers ----------------------------------------- */

/* TODO: Uncomment the following commented-out prototypes for use during type
 * checking.
 */

#if 1
void check_types(ValType found, ValType expected, SourcePos *pos, ...);
#endif
void expect(TokenType type);
void expect_id(char **id);
#if 1
IDprop *make_idprop(ValType type, unsigned int offset, unsigned int nparams,
					ValType *params);
Variable *make_var(char *id, ValType type, SourcePos pos);
#endif

/* --- function prototypes: error reporting --------------------------------- */

void abort_c(Error err, ...);
void abort_cp(SourcePos *posp, Error err, ...);

/* --- main routine --------------------------------------------------------- */

int main(int argc, char *argv[])
{
#if 1
	char *jasmin_path;
#endif

	/* TODO: Uncomment the previous definition for code generation. */

	/* set up global variables */
	setprogname(argv[0]);

	/* check command-line arguments and environment */
	if (argc != 2) {
		eprintf("usage: %s <filename>", getprogname());
	}

	/* TODO: Uncomment the following for code generation. */
#if 1
	if ((jasmin_path = getenv("JASMIN_JAR")) == NULL) {
		eprintf("JASMIN_JAR environment variable not set");
	}
#endif

	/* open the source file, and report an error if it could not be opened. */
	if ((src_file = fopen(argv[1], "r")) == NULL) {
		eprintf("file '%s' could not be opened:", argv[1]);
	}
	setsrcname(argv[1]);

	/* initialise all compiler units */
	init_scanner(src_file);
	init_symbol_table();
	init_code_generation();

	/* compile */
	get_token(&token);
	parse_program();

	/* produce the object code, and assemble */
	/* TODO: For code generation. */
	make_code_file();
	assemble(jasmin_path);

	/* release allocated resources */
	/* TODO: Release the resources of the symbol table and code generation here.
	 */
	/*for some peculiar reason, it gets a seg fault when I free the symbol table*/
	/*release_symbol_table();*/
	fclose(src_file);
	freeprogname();
	freesrcname();

#ifdef DEBUG_PARSER
	printf("SUCCESS!\n");
#endif

	return EXIT_SUCCESS;
}

/* --- parser routines ------------------------------------------------------ */

/* <program> = "program" <id> { <funcdef> } <body> .
 */
void parse_program(void)
{
	char *class_name;
	DBG_start("<program>");

	/* TODO: For code generation, set the class name inside this function, and
	 * also handle initialising and closing the "main" function.  But from the
	 * perspective of simple parsing, this function is complete.
	 */
	init_subroutine_codegen("main", NULL);
	
	expect(TOK_PROGRAM);
	expect_id(&class_name);
	/* TODO: Set the class name here during code generation. */
	set_class_name(class_name);
	
	while (token.type == TOK_DEFINE) {
		parse_funcdef();
	}
	
	parse_body();
	gen_1(JVM_RETURN);
	close_subroutine_codegen(get_variables_width());
	
	free(class_name);
	
	DBG_end("</program>");
}

void parse_funcdef(void)
{
	char *func_name;
	DBG_start("<funcdef>");
	ValType type;
	Variable *v;
	Variable *head = NULL;
	IDprop *prop;
	int nparams = 0;
	ValType *params;
	expect(TOK_DEFINE);
	expect_id(&func_name);
	char *function_name = func_name;
	expect(TOK_LPAR);
	if (IS_TYPE_TOKEN(token.type)) {
		parse_type(&type);
		expect_id(&func_name);
		v = make_var(func_name, type, position);
		head = v;
		nparams = nparams + 1;

		while (token.type == TOK_COMMA) {
			expect(TOK_COMMA);
			parse_type(&type);

			expect_id(&func_name);
			v->next = make_var(func_name, type, position);
			v = v->next;

			nparams = nparams + 1;
		}
	}
	expect(TOK_RPAR);
	if (token.type == TOK_TO) {
		expect(TOK_TO);
		parse_type(&type);
	}
	params = (ValType *)malloc(sizeof(ValType) * nparams);
	int i;
	params[0] = head->type;
	v = head;
	for (i = 1; i < nparams; i++) {
		v = v->next;
		params[i] = v->type;
	}
	prop = make_idprop(TYPE_CALLABLE, get_variables_width(), nparams, params);
	gen_call(function_name, prop);
	open_subroutine(function_name, prop);
	init_subroutine_codegen(function_name, prop);
	IDprop *idprop;
	Variable *var;
	v = head;
	for (i = 0; i < nparams; i++) {
		var = make_var(v->id, params[i], position);
		idprop = make_idprop(var->type, get_variables_width(), 0, NULL);
		insert_name(var->id, idprop);
		v = v->next;
	}

	parse_body();
	gen_1(JVM_RETURN);
	close_subroutine_codegen(get_variables_width());
	close_subroutine();
	DBG_end("</funcdef>");
}

void parse_body(void)
{
	DBG_start("<body>");
	expect(TOK_BEGIN);
	if (IS_TYPE_TOKEN(token.type)) {
		while (IS_TYPE_TOKEN(token.type)) {
			parse_vardef();
		}
	}
	parse_statements();
	expect(TOK_END);

	DBG_end("</body>");
}

void parse_type(ValType *type)
{
	DBG_start("<type>");
	if (token.type == TOK_BOOLEAN) {
		expect(TOK_BOOLEAN);
		*type = TYPE_BOOLEAN;
		if (token.type == TOK_ARRAY) {
			expect(TOK_ARRAY);
			SET_AS_ARRAY(*type);
		}
	} else if (token.type == TOK_INTEGER) {
		expect(TOK_INTEGER);
		*type = TYPE_INTEGER;
		if (token.type == TOK_ARRAY) {
			expect(TOK_ARRAY);
			SET_AS_ARRAY(*type);
		}
	} else {
		abort_c(ERR_TYPE_EXPECTED, token.type);
	}

	DBG_end("</type>");
}

void parse_vardef(void)
{
	DBG_start("<vardef>");
	ValType type;
	Variable *v;
	IDprop *prop;
	char *vname;
	parse_type(&type);
	expect_id(&vname);
	v = make_var(vname, type, position);
	prop = make_idprop(v->type, get_variables_width(), 0, NULL);
	if (find_name(vname, &prop) == FALSE) {
		insert_name(vname, prop);
	} else {
		abort_c(ERR_MULTIPLE_DEFINITION, vname);
	}
	while (token.type == TOK_COMMA) {
		expect(TOK_COMMA);
		expect_id(&vname);
		v = make_var(vname, type, position);
		prop = make_idprop(v->type, get_variables_width(), 0, NULL);
		if (find_name(vname, &prop) == FALSE) {
			insert_name(vname, prop);
		} else {
			abort_c(ERR_MULTIPLE_DEFINITION, vname);
		}
	}
	
	expect(TOK_SEMICOLON);
	DBG_end("</vardef>");
}

void parse_statement(void)
{
	DBG_start("<statement>");

	switch (token.type) {
		case TOK_EXIT:
			parse_exit();
			break;
		case TOK_IF:
			parse_if();
			break;
		case TOK_ID:
			parse_name();
			break;
		case TOK_READ:
			parse_read();
			break;
		case TOK_WHILE:
			parse_while();
			break;
		case TOK_WRITE:
			parse_write();
			break;
		default:
			abort_c(ERR_STATEMENT_EXPECTED, token.type);
	}

	DBG_end("</statement>");
}

void parse_statements(void)
{
	DBG_start("<statements>");

	if (IS_STATEMENT(token.type)) {
		parse_statement();
		while (token.type == TOK_SEMICOLON) {
			expect(TOK_SEMICOLON);
			parse_statement();
		}
	} else if (!IS_STATEMENT(token.type) && token.type != TOK_CHILL) {
		abort_c(ERR_STATEMENT_EXPECTED, token.type);
	} else if (token.type == TOK_CHILL) {
		expect(TOK_CHILL);
	}

	DBG_end("</statements>");
}

void parse_exit(void)
{
	DBG_start("<exit>");
	ValType type;
	expect(TOK_EXIT);
	if (STARTS_EXPR(token.type)) {
		parse_expr(&type);
	}
	DBG_end("</exit>");
}

void parse_if(void)
{
	DBG_start("<if>");
	ValType type;
	expect(TOK_IF);
	parse_expr(&type);
	check_types(type, TYPE_BOOLEAN, &position, "for 'if' guard");
	expect(TOK_THEN);
	parse_statements();
	while (token.type == TOK_ELSIF) {
		expect(TOK_ELSIF);
		parse_expr(&type);
		check_types(type, TYPE_BOOLEAN, &position, "for 'elsif' guard");
		expect(TOK_THEN);
		parse_statements();
	}
	if (token.type == TOK_ELSE) {
		expect(TOK_ELSE);
		parse_statements();
	}
	expect(TOK_END);
	DBG_end("</if>");
}

void parse_name(void)
{
	DBG_start("<name>");
	ValType type;
	IDprop *prop;
	char *name;
	expect_id(&name);
	int offset = 0;
	if (find_name(name, &prop) == TRUE) {
		type = prop->type;
		offset = prop->offset;
	} else {
		abort_c(ERR_UNKNOWN_IDENTIFIER, name);
	}
	
	if (token.type == TOK_LPAR) {
		parse_arglist(name, prop);
		if (token.type == TOK_SEMICOLON) {
			expect(TOK_SEMICOLON);
			parse_statement();
		} else if (token.type == TOK_GETS) {
			expect(TOK_GETS);
			if (STARTS_EXPR(token.type)) {
				expect(token.type);
				parse_expr(&type);
			} else if (token.type == TOK_ARRAY) {
				expect(TOK_ARRAY);
				parse_simple(&type);
				check_types(type, TYPE_INTEGER, &position,
							"for array size of '%s'", name);
			} else {
				abort_c(ERR_ARRAY_ALLOCATION_OR_EXPRESSION_EXPECTED,
						token.type);
			}
			gen_2(JVM_ISTORE, offset);
		}
	} else if (token.type == TOK_LBRACK) {
		gen_2(JVM_ALOAD, offset);
		parse_index(name);
		expect(TOK_GETS);
		
		/*if (type == TYPE_INTEGER + TYPE_ARRAY) {
			gen_newarray(T_INT);
			gen_2(JVM_ASTORE, offset);
		} else if (type == TYPE_BOOLEAN + TYPE_ARRAY) {
			gen_newarray(T_BOOLEAN);
			gen_2(JVM_ASTORE, offset);
		}*/
		
		if (STARTS_EXPR(token.type)) {
			if (STARTS_EXPR(token.type)) {
				parse_expr(&type);
			}
			if (IS_ADDOP(token.type)) {
				expect(token.type);
				parse_expr(&type);
			}
			gen_2(JVM_IASTORE, offset);
		} else if (token.type == TOK_ARRAY) {
			expect(TOK_ARRAY);
			parse_simple(&type);
			check_types(type, TYPE_INTEGER, &position, "for array size of '%s'",
						name);
		} else {
			abort_c(ERR_ARRAY_ALLOCATION_OR_EXPRESSION_EXPECTED, token.type);
		}
		/*gen_2(JVM_ISTORE, offset);*/
	} else if (token.type == TOK_GETS) {
		if (type == TYPE_INTEGER + TYPE_ARRAY) {
			gen_2(JVM_ASTORE, offset);
		}
		expect(TOK_GETS);
		if (STARTS_EXPR(token.type)) {
			parse_expr(&type);
			gen_2(JVM_ISTORE, offset);
		} else if (token.type == TOK_ARRAY) {
			expect(TOK_ARRAY);
			parse_simple(&type);
			check_types(type, TYPE_INTEGER, &position, "for array size of '%s'",
						name);
		} else {
			position.col = position.col - 1;
			abort_c(ERR_ARRAY_ALLOCATION_OR_EXPRESSION_EXPECTED, token.type);
		}
		/*gen_2(JVM_ISTORE, offset);*/
	} else {
		abort_c(ERR_ARGUMENT_LIST_OR_VARIABLE_ASSIGNMENT_EXPECTED, token.type);
	}
	
	DBG_end("</name>");
}

void parse_read(void)
{
	DBG_start("<read>");
	char *read_name;
	int offset = 0;
	ValType type;
	IDprop *prop;
	expect(TOK_READ);
	expect_id(&read_name);
	
	if (token.type == TOK_LBRACK) {
		parse_index(read_name);
	} else {
		if (find_name(read_name, &prop) == TRUE) {
			type = prop->type;
			offset = prop->offset;
			gen_read(type);
			gen_2(JVM_ISTORE, offset);
		}
	}
	DBG_end("</read>");
}

void parse_while(void)
{
	DBG_start("<while>");
	ValType type;
	expect(TOK_WHILE);
	parse_expr(&type);
	check_types(type, TYPE_BOOLEAN, &position, "for 'while' guard");
	expect(TOK_DO);
	parse_statements();
	expect(TOK_END);
	DBG_end("</while>");
}

void parse_write(void)
{
	DBG_start("<write>");
	ValType type;
	expect(TOK_WRITE);
	if (token.type == TOK_STR) {
		gen_print_string(token.string);
		expect(TOK_STR);
		if (token.type == TOK_AMPERSAND) {
			while (token.type == TOK_AMPERSAND) {
				expect(TOK_AMPERSAND);
				if (token.type == TOK_STR) {
					gen_print_string(token.string);
					expect(TOK_STR);
				} else if (STARTS_EXPR(token.type)) {
					parse_expr(&type);
					gen_print(type);
				}
			}
		}
	} else if (STARTS_EXPR(token.type)) {
		parse_expr(&type);
		if (type == TYPE_INTEGER + TYPE_ARRAY) {
			gen_print(TYPE_INTEGER);
		} else if (type == TYPE_BOOLEAN + TYPE_ARRAY) {
			gen_print(TYPE_BOOLEAN);
		} else {
			gen_print(type);
		}
		/*gen_print(type);*/
		if (token.type == TOK_AMPERSAND) {
			while (token.type == TOK_AMPERSAND) {
				expect(TOK_AMPERSAND);
				if (token.type == TOK_STR) {
					gen_print_string(token.string);
					expect(TOK_STR);
				} else if (STARTS_EXPR(token.type)) {
					parse_expr(&type);
					gen_print(type);
				}
			}
		}
	} else {
		abort_c(ERR_EXPRESSION_OR_STRING_EXPECTED, token.type);
	}
	DBG_end("</write>");
}

void parse_index(char *id)
{
	DBG_start("<index>");
	ValType type;
	expect(TOK_LBRACK);
	parse_simple(&type);
	check_types(type, TYPE_INTEGER, &position, "for array index of '%s'", id);
	expect(TOK_RBRACK);
	DBG_end("</index>");
}

void parse_arglist(char *id, IDprop *prop)
{
	DBG_start("<arglist>");
	id = "";
	relop = 0;
	ValType type;
	expect(TOK_LPAR);
	if (token.type == TOK_RPAR && flag == 1) {
		abort_c(ERR_FACTOR_EXPECTED);
	}
	if (STARTS_EXPR(token.type)) {
		parse_expr(&type);
		check_types(type, prop->type, &position);
		while (token.type == TOK_COMMA) {
			expect(TOK_COMMA);
			parse_expr(&type);
			check_types(type, prop->type, &position);
		}
	}
	expect(TOK_RPAR);
	DBG_end("</arglist>");
}

void parse_simple(ValType *type)
{
	DBG_start("<simple>");
	ValType t1;
	if (token.type == TOK_MINUS) {
		expect(TOK_MINUS);
	}
	parse_term(type);
	
	while (IS_ADDOP(token.type)) {
		if (token.type == TOK_PLUS) {
			expect(token.type);		
			parse_term(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IADD);
		} else if (token.type == TOK_MINUS) {
			expect(token.type);
			parse_term(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_ISUB);
		} else if (token.type == TOK_OR) {
			expect(token.type);
			parse_term(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IOR);
		}
	}
	DBG_end("</simple>");
}

void parse_term(ValType *type)
{
	DBG_start("<term>");
	ValType t1;
	parse_factor(type);
	while (IS_MULOP(token.type)) {
		if (token.type == TOK_MUL) {
			expect(token.type);
			parse_factor(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IMUL);
		} else if (token.type == TOK_DIV) {
			expect(token.type);
			parse_factor(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IDIV);
		} else if (token.type == TOK_MOD) {
			expect(token.type);
			parse_factor(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IREM);
		} else if (token.type == TOK_AND) {
			expect(token.type);
			parse_factor(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IAND);
		}
	}
	DBG_end("</term>");
}

void parse_factor(ValType *type)
{
	char *factor_name = NULL;
	DBG_start("<factor>");
	ValType t1;
	IDprop *prop = NULL;
	int offset = 0;
	if (token.type == TOK_ID) {
		expect_id(&factor_name);
		if (find_name(factor_name, &prop) == TRUE) {
			*type = prop->type;
			offset = prop->offset;
			if (*type == TYPE_INTEGER + TYPE_ARRAY
			 || *type == TYPE_BOOLEAN + TYPE_ARRAY) {
				gen_2(JVM_IALOAD, offset);
			} else {
				gen_2(JVM_ILOAD, offset);
			}
		} else {
			abort_c(ERR_UNKNOWN_IDENTIFIER, factor_name);
		}
		if (token.type == TOK_LBRACK) {
			parse_index(factor_name);
		} else if (token.type == TOK_LPAR) {
			parse_arglist(factor_name, prop);
		}
	} else if (token.type == TOK_LBRACK) {
		parse_index(factor_name);
	} else if (token.type == TOK_LPAR) {
		flag = 1;
		parse_arglist(factor_name, prop);
		flag = 0;
	} else if (token.type == TOK_NUM) {
		gen_2(JVM_LDC, token.value);
		expect(TOK_NUM);
		*type = TYPE_INTEGER;
	} else if (IS_RELOP(token.type)) {
		relop++;
		if (relop > 1) {
			abort_c(ERR_FACTOR_EXPECTED);
		}
	} else
		switch (token.type) {
			case TOK_NOT:
				expect(TOK_NOT);
				parse_factor(&t1);
				check_types(t1, *type, &position);
				gen_2(JVM_IXOR, 1);
				break;
			case TOK_TRUE:
				expect(TOK_TRUE);
				gen_2(JVM_LDC, 1);
				*type = TYPE_BOOLEAN;
				break;
			case TOK_FALSE:
				expect(TOK_FALSE);
				gen_2(JVM_LDC, 0);
				*type = TYPE_BOOLEAN;
				break;
			default:
				abort_c(ERR_FACTOR_EXPECTED);
		}
	DBG_end("</factor>");
}

void parse_expr(ValType *type)
{
	DBG_start("<expr>");
	ValType t1;
	parse_simple(type);
	if (IS_RELOP(token.type)) {
		relop++;
		if (token.type == TOK_EQ) {
			expect(token.type);
			parse_simple(&t1);
			check_types(*type, t1, &position);
			gen_1(JVM_IF_ICMPEQ);
			/**type = TYPE_BOOLEAN;*/
		} else if (token.type == TOK_NE) {
			expect(token.type);
			parse_simple(&t1);
			check_types(*type, t1, &position);
			gen_1(JVM_IF_ICMPNE);
			/**type = TYPE_BOOLEAN;*/
		} else if (token.type == TOK_GE) {
			expect(token.type);
			parse_simple(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IF_ICMPGE);
		} else if (token.type == TOK_GT) {
			expect(token.type);
			parse_simple(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IF_ICMPGT);
		} else if (token.type == TOK_LE) {
			expect(token.type);
			parse_simple(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IF_ICMPLE);
		} else if (token.type == TOK_LT) {
			expect(token.type);
			parse_simple(&t1);
			check_types(t1, *type, &position);
			gen_1(JVM_IF_ICMPLT);
		}
	}
	DBG_end("</expr>");
}
/* TODO: Turn the EBNF into a program by writing one parse function for those
 * productions as instructed in the specification.  I suggest you use the
 * production as comment to the function.  Also, you may only report errors
 * through the abort_c and abort_cp functions.  You must figure out what
 * arguments you should pass for each particular error.  Keep to the *exact*
 * error messages given in the specification.
 */

/* --- helper routines ------------------------------------------------------ */

#define MAX_MESSAGE_LENGTH 256

/* TODO: Uncomment the following function for use during type checking. */

#if 1
void check_types(ValType found, ValType expected, SourcePos *pos, ...)
{
	char buf[MAX_MESSAGE_LENGTH], *s;
	va_list ap;

	if (found != expected) {
		buf[0] = '\0';
		va_start(ap, pos);
		s = va_arg(ap, char *);
		vsnprintf(buf, MAX_MESSAGE_LENGTH, s, ap);
		va_end(ap);
		if (pos != NULL) {
			position = *pos;
		}
		leprintf("incompatible types (expected %s, found %s) %s",
				 get_valtype_string(expected), get_valtype_string(found), buf);
	}
}
#endif

void expect(TokenType type)
{
	if (token.type == type) {
		get_token(&token);
	} else {
		abort_c(ERR_EXPECT, type);
	}
}

void expect_id(char **id)
{
	if (token.type == TOK_ID) {
		*id = strdup(token.lexeme);
		get_token(&token);
	} else {
		abort_c(ERR_EXPECT, TOK_ID);
	}
}

/* TODO: Uncomment the following two functions for use during type checking. */

#if 1
IDprop *make_idprop(ValType type, unsigned int offset, unsigned int nparams,
					ValType *params)
{
	IDprop *ip;

	ip = emalloc(sizeof(IDprop));
	ip->type = type;
	ip->offset = offset;
	ip->nparams = nparams;
	ip->params = params;

	return ip;
}

Variable *make_var(char *id, ValType type, SourcePos pos)
{
	Variable *vp;

	vp = emalloc(sizeof(Variable));
	vp->id = id;
	vp->type = type;
	vp->pos = pos;
	vp->next = NULL;

	return vp;
}
#endif

/* --- error reporting routines --------------------------------------------- */

void _abort_compile(SourcePos *posp, Error err, va_list args);

void abort_c(Error err, ...)
{
	va_list args;

	va_start(args, err);
	_abort_compile(NULL, err, args);
	va_end(args);
}

void abort_cp(SourcePos *posp, Error err, ...)
{
	va_list args;

	va_start(args, err);
	_abort_compile(posp, err, args);
	va_end(args);
}

void _abort_compile(SourcePos *posp, Error err, va_list args)
{
	char expstr[MAX_MESSAGE_LENGTH], *s /*, *t*/;
	int tok;

	if (posp) {
		position = *posp;
	}

	snprintf(expstr, MAX_MESSAGE_LENGTH, "expected %%s, but found %s",
			 get_token_string(token.type));

	switch (err) {
		case ERR_ARGUMENT_LIST_OR_VARIABLE_ASSIGNMENT_EXPECTED:
		case ERR_ARRAY_ALLOCATION_OR_EXPRESSION_EXPECTED:
		case ERR_EXIT_EXPRESSION_NOT_ALLOWED_FOR_PROCEDURE:
		case ERR_EXPECT:
		case ERR_EXPRESSION_OR_STRING_EXPECTED:
		case ERR_FACTOR_EXPECTED:
		case ERR_MISSING_EXIT_EXPRESSION_FOR_FUNCTION:
		case ERR_STATEMENT_EXPECTED:
		case ERR_TYPE_EXPECTED:
		case ERR_UNKNOWN_IDENTIFIER:
		case ERR_MULTIPLE_DEFINITION:
			break;
		default:
			s = va_arg(args, char *);
			break;
	}

	switch (err) {
			/* TODO: Add additional cases here as is necessary, referring to
			 * errmsg.h for all possible errors.  Some errors only become
			 * possible to recognise once we add type checking.  Until you get
			 * to type checking, you can handle such errors by adding the
			 * default case. However, your final submission *must* handle all
			 * cases explicitly.
			 */

		case ERR_EXPECT:
			position.col = position.col - 1;
			tok = va_arg(args, int);
			leprintf(expstr, get_token_string(tok));
			break;

		case ERR_FACTOR_EXPECTED:
			position.col = position.col - 1;
			leprintf(expstr, "factor");
			break;

		case ERR_UNREACHABLE:
			leprintf("unreachable: %s", s);
			break;

		case ERR_STATEMENT_EXPECTED:
			position.col = position.col - 1;
			leprintf("expected statement, but found %s",
					 get_token_string(token.type));
			break;

		case ERR_TYPE_EXPECTED:
			position.col = position.col - 1;
			leprintf("expected type, but found %s",
					 get_token_string(token.type));
			break;

		case ERR_ARGUMENT_LIST_OR_VARIABLE_ASSIGNMENT_EXPECTED:
			position.col = position.col - 1;
			leprintf(
				"expected argument list or variable assignment, but found %s",
				get_token_string(token.type));
			break;

		case ERR_ARRAY_ALLOCATION_OR_EXPRESSION_EXPECTED:
			leprintf("expected array allocation or expression, but found %s",
					 get_token_string(token.type));
			break;

			/*case ERR_EXIT_EXPRESSION_NOT_ALLOWED_FOR_PROCEDURE:


			case ERR_MISSING_EXIT_EXPRESSION_FOR_FUNCTION:*/

		case ERR_EXPRESSION_OR_STRING_EXPECTED:
			position.col = position.col - 1;
			leprintf("expected expression or string, but found %s",
					 get_token_string(token.type));
			break;

		case ERR_UNKNOWN_IDENTIFIER:
			leprintf("unknown identifier '%s'", s);
			break;

		case ERR_MULTIPLE_DEFINITION:
			leprintf("multiple definition of '%s'", s);
			break;

		default:
			break;
	}
}

/* --- debugging output routines -------------------------------------------- */

#ifdef DEBUG_PARSER

static int indent = 0;

void debug_start(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	debug_info(fmt, ap);
	va_end(ap);
	indent += 2;
}

void debug_end(const char *fmt, ...)
{
	va_list ap;

	indent -= 2;
	va_start(ap, fmt);
	debug_info(fmt, ap);
	va_end(ap);
}

void debug_info(const char *fmt, ...)
{
	int i;
	char buf[MAX_MESSAGE_LENGTH], *buf_ptr;
	va_list ap;

	buf_ptr = buf;

	va_start(ap, fmt);

	for (i = 0; i < indent; i++) {
		*buf_ptr++ = ' ';
	}
	vsprintf(buf_ptr, fmt, ap);

	buf_ptr += strlen(buf_ptr);
	snprintf(buf_ptr, MAX_MESSAGE_LENGTH, " in line %d.\n", position.line);
	fflush(stdout);
	fputs(buf, stdout);
	fflush(NULL);

	va_end(ap);
}

#endif /* DEBUG_PARSER */
