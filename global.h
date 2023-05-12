/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: global.h,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

GLOBAL Pair **the_pstack;
GLOBAL Pair **the_pstackptr;
GLOBAL Pair **the_pstackmax;
GLOBAL int    the_pstacksize;
GLOBAL char *the_cstack;
GLOBAL char *the_cstackptr;
GLOBAL char *the_cstackmax;
GLOBAL int   the_cstacksize;

GLOBAL Pair* nil_constant;
GLOBAL Pair* true_constant;
GLOBAL Pair* false_constant;
GLOBAL Pair* unassigned_constant;

GLOBAL int global_errorflag;			

GLOBAL int ScanIndex;
GLOBAL Pair** FreePointer;	/* Zeigt auf naechstes freie Wort */
GLOBAL Pair** EndPointer;	/* Zeigt auf das erste ungueltige Wort */

/*
 * Register der Maschine
 *
 */

GLOBAL Pair *fr1, *fr2, *fr3, *fr4, *pr_a, *pr_b, *pr_c, *pr_d;
GLOBAL Pair *expr, *env, *newenv, *val, *fun, *argl, *unev, *the_global_env;

/* initf und exitf enthalten jeweils eine Liste von Pairs. car von
 * einem Listenelement ist die init-Funktion eines dynamic-wind, cdr
 * die exit-Funktion. Bei einem call/cc werden die Elemente von 
 * initf und exitf auf eq? verglichen, und bei Gleichheit die 
 * Ausfuehrung unterdrueckt. 
 */

GLOBAL Pair *initf;	/* Init-Functions fuer Protected call/cc */
GLOBAL Pair *exitf;	/* Exit-Functions */
GLOBAL Pair *expr_env;  /* expression/env-Pair fuer general-eval */

/*
 *   NEU:
 */

GLOBAL Pair* errormessage;	/* Error-Message (GET) */
GLOBAL Pair* errorirritant;	/* was war fehlerhaft ? (GET) */
GLOBAL Pair* errorcallchain;	/* Call-Chain des Fehlers (GET) */
GLOBAL Pair* errorexpression;	/* expr currently evaluated */
GLOBAL Pair* errorenvironment;  /* environment in affect */
GLOBAL Pair* jumpout;		/* cuc-Continuation (SET!) */
GLOBAL Pair* jumpin;		/* Continuation fuer evtl. Continue (GET) */
GLOBAL Pair* continuemessage;   /* Was passiert bei Continue ? (GET) */
GLOBAL Pair* callchain;		/* interne callchain (Bestandteil der cont.) */

GLOBAL Pair* cond_atom;
GLOBAL Pair* quote_atom;
GLOBAL Pair* quasiquote_atom;
GLOBAL Pair* unquote_atom;
GLOBAL Pair* unquote_splicing_atom;
GLOBAL Pair* broken_heart;
GLOBAL Pair* lpar_atom;
GLOBAL Pair* rpar_atom;
GLOBAL Pair* dot_atom;
GLOBAL Pair* end_of_file_atom;
GLOBAL Pair* vector_atom;
GLOBAL Pair* string_atom;
GLOBAL Pair* float_atom;
GLOBAL Pair* pseudo_lambda_atom;
GLOBAL Pair* syntax_lambda_atom;

GLOBAL Pair* ok_atom;
GLOBAL Pair* else_atom;
GLOBAL Pair* load_atom;
GLOBAL Pair* eval_atom;
GLOBAL Pair* evalg_atom;
GLOBAL Pair* eval_general_atom;
GLOBAL Pair* begin_atom;
GLOBAL Pair* lambda_atom;

GLOBAL Pair* define_atom;
GLOBAL Pair* if_atom;
GLOBAL Pair* set_atom;

GLOBAL Pair  CharacterArray[256];

GLOBAL Pair* the_global_env;
GLOBAL char* pstack_o_message;
GLOBAL char* cstack_o_message;

GLOBAL Port* the_ports;

GLOBAL Port* the_current_input_port;
GLOBAL Port* the_current_output_port;
GLOBAL Port* the_current_error_port;

GLOBAL Port* the_console_input_port;
GLOBAL Port* the_console_output_port;
GLOBAL Port* the_console_error_port;

GLOBAL int global_argc;
GLOBAL char** global_argv;

GLOBAL int HeapSize;		/* Size of Heap in Words */
GLOBAL int HeapUsed;		/* # of Words used (last gc) */

GLOBAL int DestructiveMacros;   /* 1 = Destruktiv an,
				   0 = Destruktiv aus ... */

GLOBAL Integer StaticIntArray[STATIC_INTEGER_SIZE];
GLOBAL int SchemeAuxmemMax;

GLOBAL int exit_on_error;
