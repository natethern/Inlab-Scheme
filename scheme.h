/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: scheme.h,v 4.105 2006/04/29 09:13:22 tommy Exp $
 */

static char* rcs_include_id = "$Id: scheme.h,v 4.105 2006/04/29 09:13:22 tommy Exp $";

typedef long SchemeWord;		
typedef int (*SchemeFptr)();		

typedef struct pair_struct {
  SchemeWord tag;
  struct pair_struct *car;
  struct pair_struct *cdr;
} Pair;

typedef struct {
  SchemeWord tag;
  SchemeWord value;
} Integer;

typedef struct atom_struct {
  SchemeWord  tag;				
  SchemeWord  mark;			/* 1 = used, 0 = unused */
  SchemeWord  special;			/* 1 = special, 0 = normal */
  SchemeWord  hashindex;		/* Index in Hashtable */
  struct atom_struct *prev;	
  struct atom_struct *next;
  Pair*	pname;		
  Pair* direct;	
  SchemeWord  direct_bound;		
  SchemeWord  direct_violated;
  SchemeFptr  primitive;
  SchemeWord  arguments;		/* Anzahl der erwarteten Argumente */
  SchemeWord  special_form;		/* Spezialform, wenn 0 dann keine */
} Atom;

typedef struct port_struct {
  SchemeWord tag;
  FILE* fd;
  SchemeWord line;
  SchemeWord col;
  SchemeWord character;
  struct port_struct* next;
} Port;

typedef struct {
  SchemeWord tag;
  SchemeWord position;	/* Position in String */
  SchemeWord line;
  SchemeWord col;
  SchemeWord character;
  Pair* string;
} InputStringPort;

typedef struct {
  SchemeWord tag;
  SchemeWord dummy;	/* filler */
  SchemeWord line;
  SchemeWord col;
  SchemeWord character;
  Pair* charlist;
} OutputStringPort;

typedef struct {	/* Input-Port fuer einen statischen Bereich in C */
  SchemeWord tag;
  SchemeWord dummy;
  SchemeWord line;
  SchemeWord col;
  SchemeWord character;
  char* current;	/* Pointer auf momentan zu lesenden char ... */
  char* invalid;	/* Pointer auf ersten nicht mehr zu lesenden char ... */
} StaticPort;

/* 
 * Externer Typ
 */

typedef struct external_functions_struct {
  SchemeFptr print;
  SchemeFptr destroy;
  SchemeFptr putchar;
  SchemeFptr putstring;
  SchemeFptr getchar;
  SchemeFptr ungetchar; // !!! ist eigentlich unnoetig,
			// da der Typ Port ja schon 
			// sich daruem kuemmert ...
} ExternalFunctions;

typedef struct external_struct {
  SchemeWord tag;
  ExternalFunctions* functions;
  SchemeWord line;
  SchemeWord col;
  SchemeWord character;
  struct external_struct* next;
  SchemeWord length;
} External;

/**/

typedef struct {
  SchemeWord tag;
  Pair* name;
  Pair* environment;
  Pair* parameters;
  Pair* body;
} Compound;

typedef struct {
  SchemeWord tag;
  Pair* name;
  Pair* environment;
  Pair* parameters;
  Pair* body;
} SyntaxCompound;

typedef struct {
  SchemeWord tag;
  SchemeFptr function;
  SchemeWord arguments;
  Pair* symbol;
} Primitive;

/* Continuations (internal) */

typedef struct {
  SchemeWord tag;
  Pair* pstack;	
  Pair* cstack;		
  Pair* parameters;	/* Pseudo-Parameterliste GENAU AN DIESER STELLE */
  Pair* fr1;
  Pair* fr2;
  Pair* fr3;
  Pair* fr4;
  Pair* pr_a;
  Pair* pr_b;
  Pair* pr_c;
  Pair* pr_d;
  Pair* expr;
  Pair* env;
  Pair* val;
  Pair* fun;
  Pair* argl;
  Pair* newenv;
  Pair* unev;
  Pair* initf;	    /* Init-Functions / dynamic-wind */
  Pair* exitf;      /* exit-Functions */
  Pair* expr_env;   /* expression + env fuer internal-general-eval */
  Pair* callchain;  /* CallChain fuer debugging */
} Continuation;

typedef struct {
  SchemeWord  tag;
  Pair* variable;
  Pair* value;
  Pair* next;
} Environment;

#define STATIC_INTEGER_START (-100)
#define STATIC_INTEGER_SIZE  (201)

#define PSTACKO pstack_o_message
#define CSTACKO cstack_o_message

#define SCHEMEInitialAtomSize 1024
#define SCHEME_ERROR (-1)
#define SCHEME_OK (0)
#define SCHEME_TRUE (1)
#define SCHEME_FALSE (0)

/*
 * THE TYPES
 *
 */

enum {
PAIR_TYPE = 1,
SYMBOL_TYPE,
INTEGER_TYPE,
CHAR_TYPE,
STRING_TYPE, 
VECTOR_TYPE,
FLOAT_TYPE,
PRIMITIVE_TYPE,  	 
COMPOUND_TYPE,   	 
SYNTAXCOMPOUND_TYPE,   	 
CONSTANT_TYPE,		 
BROKENHEART_TYPE, 	 
INPUTPORT_TYPE,		 
OUTPUTPORT_TYPE,		 
USER_TYPE,		     /* User-defined type (Scheme-Seite) */
EXTERNAL_TYPE,		     /* neuer externer Typ mit Zeiger auf SchemeFptr-Array */
CONTINUATION_TYPE,	     /* Internal Continuations */
ENVIRONMENT_TYPE,	     /* Environment-Pair */
POPENR_TYPE,		     /* popen-input-port */
POPENW_TYPE,		     /* popen-output-port */
MCONTINUATION_TYPE,	     /* Continuation in die Maschine / errorh */
INPUTSTRINGPORT_TYPE,	     /* Input aus einem String */
OUTPUTSTRINGPORT_TYPE,	     /* Output in einen String */
STATICPORT_TYPE,	     /* Input aus static-Bereich in C */

SPECIAL_NULL_TYPE,	     /* nur in Symbol, keine SF zugeordnet */
SPECIAL_START_TYPE,	     /* Pseudo-Typ (Start der special-pairs) */
SPECIAL_DEFINE_TYPE = SPECIAL_START_TYPE,
SPECIAL_SET_TYPE,	     /* set! */
SPECIAL_LAMBDA_TYPE,  	     /* internal-named-lambda */
SPECIAL_SYNTAXLAMBDA_TYPE,   /* syntax-lambda */
SPECIAL_IF_TYPE,	     /* internal-if */
SPECIAL_EVAL_TYPE,	     /* eval */
SPECIAL_BEGIN_TYPE,	     /* begin */
SPECIAL_QUOTE_TYPE,	     /* quote */
SPECIAL_EVALG_TYPE,	     /* eval-in-global-environment */
SPECIAL_EVALGENERAL_TYPE,    /* general-eval */
SPECIAL_APPLICATION_TYPE,    /* Application */
SPECIAL_NOARGS_TYPE,         /* Application ohne Argumente */
SPECIAL_END_TYPE = SPECIAL_NOARGS_TYPE
};

#define SchemeINITFILE	"init"

/*
 * Tokens, die intern vom 'Scanner' geliefert werden:
 */

enum {
  SchemeTokenLPAR=1,
  SchemeTokenRPAR,
  SchemeTokenDOT,
  SchemeTokenQUOTE,
  SchemeTokenSYMBOL,
  SchemeTokenCHAR,
  SchemeTokenFLOAT,
  SchemeTokenSTRING,
  SchemeTokenVECTOR,
  SchemeTokenINTEGER,
  SchemeTokenQQUOTE,
  SchemeTokenUNQUOTE,
  SchemeTokenUNQUOTES,
  SchemeTokenCONSTANT
};

#define SchemeMAXTOKENLEN 10240
#define SchemeLISTLIMIT 200000

#define BOOT(pname,function) SchemeInstallPrimitive(function,pname,0)
#define PrimitiveProcedure(function,name,arguments) void function() 

#define ConsOnStack() \
{\
   GiveMeWords(3);\
   *(FreePointer+0) = (Pair*) PAIR_TYPE;\
   *(FreePointer+1) = *(the_pstackptr-2);\
   *(FreePointer+2) = *(the_pstackptr-1);\
   *(the_pstackptr-2) = (Pair*) FreePointer;\
   FreePointer+=3;\
   the_pstackptr-=1;\
}

#define SCHEMESTACKSECURITY 1024

#define SchemePushP(x) \
{ \
  if (the_pstackptr >= the_pstackmax) { \
    if (the_pstackptr < the_pstackmax + SCHEMESTACKSECURITY) { \
      werr(PSTACKO); \
      *(the_pstackptr++) = (x); \
    } else { \
      werr(PSTACKO); \
      SchemePanic("unrecovered p-stack overflow"); \
    } \
  } else { \
    *(the_pstackptr++) = (x); \
  } \
}

#define SchemePopP() \
  (*(--the_pstackptr))

#define SchemeInspectP() \
  (*(the_pstackptr-1))

#define SchemePushC(x) \
{ \
  if (the_cstackptr >= the_cstackmax) { \
    if (the_cstackptr < the_cstackmax + SCHEMESTACKSECURITY) { \
      werr(CSTACKO); \
      *(the_cstackptr++) = (x); \
    } else { \
      werr(CSTACKO); \
      SchemePanic("unrecovered c-stack overflow"); \
    } \
  } else { \
    *(the_cstackptr++) = (x); \
  } \
}

#define SchemePopC() \
  (*(--the_cstackptr))

/*
 * type conversions
 */

#define C_INTEGER(x) ((SchemeWord) ((x)->car)) /* indizes usw. */
#define C_LONG(x) ((SchemeWord) ((x)->car))
#define C_FLOAT(x) (*((double*) (& ((x)->car))))
#define C_STRING(x) ((unsigned char*) (& ((x)->cdr)))
#define C_STRINGLEN(x) ((SchemeWord) ((x)->car))
#define C_CHAR(x) ((SchemeWord) ((x)->car))

#define C_VECTOR(x) ((Pair**) (& ((x)->cdr)))
#define C_VECTORLEN(x) ((SchemeWord) ((x)->car))

#define S_CHAR(x) (& CharacterArray[(x)])

/*
 * type-checking
 */

#define IS_NIL(x) ((x) == nil_constant)
#define IS_PAIR(x) (((x)->tag) == PAIR_TYPE)
#define IS_SPECIALPAIR(x) (((x)->tag) >= SPECIAL_START_TYPE && \
			    ((x)->tag) <= SPECIAL_END_TYPE)
#define IS_VECTOR(x) (((x)->tag) == VECTOR_TYPE)
#define IS_SYMBOL(x) (((x)->tag) == SYMBOL_TYPE)
#define IS_INTEGER(x) (((x)->tag) == INTEGER_TYPE)
#define IS_FLOAT(x) (((x)->tag) == FLOAT_TYPE)
#define IS_CHAR(x) (((x)->tag) == CHAR_TYPE)
#define IS_STRING(x) (((x)->tag) == STRING_TYPE)

#define IS_INPUTPORT(x) (((x)->tag) == INPUTPORT_TYPE || \
			 ((x)->tag) == POPENR_TYPE || \
			 ((x)->tag) == INPUTSTRINGPORT_TYPE || \
			 ((x)->tag) == STATICPORT_TYPE || \
			 ((x)->tag) == EXTERNAL_TYPE) 
#define IS_OUTPUTPORT(x) (((x)->tag) == OUTPUTPORT_TYPE ||\
			  ((x)->tag) == POPENW_TYPE ||\
			  ((x)->tag) == OUTPUTSTRINGPORT_TYPE || \
			  ((x)->tag) == EXTERNAL_TYPE) 

#define MAXERRLEN 512	/* max. Laenge Errorstring in error */


#define ARG(n) (n==0?argl->car:(n==1?argl->cdr->car:(n==2?argl->cdr->cdr->car:(n==3?argl->cdr->cdr->cdr->car:(n==4?argl->cdr->cdr->cdr->cdr->car:(n==5?argl->cdr->cdr->cdr->cdr->cdr->car:nil_constant))))))

/*
 *   Returncodes numerischer Funktionen
 */

enum {
  SchemeNONUMBER,
  SchemeLONG,
  SchemeDOUBLE
};

// ***** ES-Port fuer Embedded Scheme 

#define ES_STATE_START		1
#define ES_STATE_DISPLAY	2
#define ES_STATE_ACTIVE		3
#define ES_STATE_EOF 		4
#define ES_MAXESSTACK		32
#define ES_MAXINDISPLAY    	1024 

typedef struct {
  FILE *fd;			// input file descriptor
  int  state;			// current state
  int  bytecount;		// bytecount im STATE_DISPLAY 
  int  esstack[ES_MAXESSTACK];	// buffer 
  int  esstackptr;		// pointer...
  int  uc;			// char fuer ungetchar...
} ES;
