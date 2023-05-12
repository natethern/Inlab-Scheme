/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: heap.c,v 4.103 2006/04/29 09:13:22 tommy Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <fdecl.h>
#include <extern.h>
#include <global.h>
#include <heap.h>

#define C_PAIRARRAY(x) ((Pair**) (& ((x)->tag)))  /* fuer Continuations */

static char* rcsid = "$Id: heap.c,v 4.103 2006/04/29 09:13:22 tommy Exp $";
static int ShowGc;

InitStack(int psize,int csize)
{
  int real_psize=psize + SCHEMESTACKSECURITY;
  int real_csize=csize + SCHEMESTACKSECURITY;

  if ((the_pstack = (Pair**) malloc (real_psize * sizeof(Pair*))) == NULL)
    SchemePanic("cannot allocate pstack");
  the_pstackptr = the_pstack;
  the_pstackmax = the_pstack+psize-1;
  the_pstacksize = psize;

  if ((the_cstack = (char*) malloc (real_csize * sizeof(char))) == NULL)
    SchemePanic("cannot allocate cstack");
  the_cstackptr = the_cstack;
  the_cstackmax = the_cstack+csize-1;
  the_cstacksize = csize;
}

ClearStacks()
{
  the_pstackptr = the_pstack;
  the_cstackptr = the_cstack;
}

Pair** TheHeapA;
Pair** TheHeapB;

InitHeap(size)
int size;
{
  int i;
  ScanIndex=0;

  if ((TheHeapA = (Pair**) malloc(size * sizeof(SchemeWord))) == NULL) 
    SchemePanic("cannot allocate heap");
  if ((TheHeapB = (Pair**) malloc(size * sizeof(SchemeWord))) == NULL) 
    SchemePanic("cannot allocate heap");

  HeapSize = size;
  HeapUsed = 0;

  FreePointer = &(TheHeapA[0]);
  EndPointer = FreePointer+HeapSize;

  if(getenv("SCHEME_SHOWGC") != NULL) 
    ShowGc = 1;
  else
    ShowGc = 0;

  for (i=0; i<256; i++) {
    CharacterArray[i].tag = CHAR_TYPE;
    CharacterArray[i].car = (Pair*) ((SchemeWord) i);
  }

  fr1 = nil_constant;
  fr2 = nil_constant;
  fr3 = nil_constant;
  fr4 = nil_constant;

  pr_a = nil_constant;
  pr_b = nil_constant;
  pr_c = nil_constant;
  pr_d = nil_constant;

  expr = nil_constant;
  env = nil_constant;
  newenv = nil_constant;
  val = nil_constant;
  fun = nil_constant;
  argl = nil_constant;
  unev = nil_constant;

  initf = nil_constant;
  exitf = nil_constant;
  expr_env = nil_constant;

  errorcallchain = nil_constant;
  jumpout = nil_constant;
  jumpin = nil_constant;
  continuemessage = nil_constant;
  callchain = nil_constant;

  the_global_env = nil_constant;

  errormessage = nil_constant;
  errorirritant = nil_constant;
  errorexpression = nil_constant;
  errorenvironment = nil_constant;

  DestructiveMacros = 1;	/* Destruktive Macros per default an */

  /* Initialisierung des statischen Int-Arrays */

  for(i=0; i<STATIC_INTEGER_SIZE; i++) {
    StaticIntArray[i].tag = INTEGER_TYPE;
    StaticIntArray[i].value = (SchemeWord) (STATIC_INTEGER_START + i);
  }
}

static int gcflag = 0; 	

Pair** AllocHeap(size)
int size;
{
  Pair ** rc;
  int i;

  if (FreePointer + size >= EndPointer) {
    garbage_collect();
    if (FreePointer + size >= EndPointer) {
        SchemePanic("Out of Heap");
    }
  }
  rc = FreePointer;
  FreePointer+=size;
  return(rc);
}


static collect(x)
Pair **x;
{
  static int lasttype;
  Pair* new_pointer;
  Pair* old_pointer;

  switch((int) ((*x)->tag)) {
    case BROKENHEART_TYPE:
      *x = (*x)->car;
      break;
    case PAIR_TYPE:
      old_pointer = (*x);
      SetPairIn(new_pointer);
      new_pointer->car = (*x)->car;
      new_pointer->cdr = (*x)->cdr;
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case USER_TYPE:
      old_pointer = (*x);
      SetPairIn(new_pointer);
      new_pointer->tag = USER_TYPE;
      new_pointer->car = (*x)->car;
      new_pointer->cdr = (*x)->cdr;
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case SYMBOL_TYPE:
      ((Atom*) (*x))->mark = 1;
      break;
    case INTEGER_TYPE:
      if (((Integer*) (*x)) >= &StaticIntArray[0] &&
	  ((Integer*) (*x)) <= &StaticIntArray[STATIC_INTEGER_SIZE-1]) {
	break;
      } else {
        old_pointer = (*x);
        new_pointer = AllocInteger();
        new_pointer->car = (*x)->car;
        (*x) = new_pointer;
        old_pointer->tag = BROKENHEART_TYPE;
        old_pointer->car = new_pointer;
        break;
      }
    case STRING_TYPE:
      old_pointer = (*x);
      new_pointer = AllocString((int) ((*x)->car));
      new_pointer->car = (*x)->car;  /* strlen */
      memcpy(& new_pointer->cdr, & old_pointer->cdr, (int) (old_pointer->car));
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case VECTOR_TYPE:
      old_pointer = (*x);
      new_pointer = AllocVector((SchemeWord) ((*x)->car));
      new_pointer->car = (*x)->car;  
      memcpy(& new_pointer->cdr, & old_pointer->cdr, 
         ((SchemeWord) (old_pointer->car)) * sizeof(SchemeWord));
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case CONTINUATION_TYPE:
    case MCONTINUATION_TYPE:
      old_pointer = (*x);
      new_pointer = AllocContinuation();
      memcpy(& new_pointer->car, & old_pointer->car,
	     sizeof(Continuation)-sizeof(SchemeWord));
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;

    case EXTERNAL_TYPE:
      old_pointer = (*x);
      new_pointer = AllocExternalType ((int) (((External*) (*x))->length));

      ((External*) new_pointer)->functions = ((External*) old_pointer)->functions;
      ((External*) new_pointer)->length = ((External*) old_pointer)->length;

      memcpy(((char*) new_pointer)+sizeof(External), 
              ((char*) old_pointer)+sizeof(External), 
             ((int) (((External*) new_pointer)->length)) * sizeof(SchemeWord));
      
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
   
    case FLOAT_TYPE:		/* assuming 64-bit float */
      old_pointer = (*x);
      new_pointer = AllocFloat();
      new_pointer->car = (*x)->car;
      new_pointer->cdr = (*x)->cdr;
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case PRIMITIVE_TYPE:
      old_pointer = (*x);
      new_pointer = AllocPrimitive();
      memcpy(& new_pointer->car, & old_pointer->car,
	     sizeof(Primitive)-sizeof(SchemeWord));
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case COMPOUND_TYPE:
      old_pointer = (*x);
      new_pointer = AllocCompound();
      memcpy(& new_pointer->car, & old_pointer->car,
	     sizeof(Compound)-sizeof(SchemeWord));
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case SYNTAXCOMPOUND_TYPE:
      old_pointer = (*x);
      new_pointer = AllocSyntaxCompound();
      memcpy(& new_pointer->car, & old_pointer->car,
	     sizeof(SyntaxCompound)-sizeof(SchemeWord));
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case INPUTSTRINGPORT_TYPE:
      old_pointer = (*x);
      new_pointer = AllocInputStringPort();
      memcpy(& new_pointer->car, & old_pointer->car,
	     sizeof(InputStringPort)-sizeof(SchemeWord));
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case STATICPORT_TYPE:
      old_pointer = (*x);
      new_pointer = AllocStaticPort();
      memcpy(& new_pointer->car, & old_pointer->car,
	     sizeof(StaticPort)-sizeof(SchemeWord));
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case OUTPUTSTRINGPORT_TYPE:
      old_pointer = (*x);
      new_pointer = AllocOutputStringPort();
      memcpy(& new_pointer->car, & old_pointer->car,
	     sizeof(OutputStringPort)-sizeof(SchemeWord));
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case POPENR_TYPE:
      old_pointer = (*x);
      new_pointer = AllocPopenrPort();
      goto commonaction;
    case POPENW_TYPE:
      old_pointer = (*x);
      new_pointer = AllocPopenwPort();
      goto commonaction;
    case INPUTPORT_TYPE:
      old_pointer = (*x);
      new_pointer = AllocInputPort();
      goto commonaction;
      /* break;  not reached */
    case OUTPUTPORT_TYPE:
      old_pointer = (*x);
      new_pointer = AllocOutputPort();
commonaction:
      ((Port*) new_pointer)->fd = ((Port*) old_pointer)->fd;
      ((Port*) new_pointer)->col = ((Port*) old_pointer)->col;
      ((Port*) new_pointer)->line = ((Port*) old_pointer)->line;
      ((Port*) new_pointer)->character =
	 ((Port*) old_pointer)->character;

      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case CHAR_TYPE:
    case CONSTANT_TYPE:
      break;
    case ENVIRONMENT_TYPE:
      old_pointer = (*x);
      SetEnvironmentIn(new_pointer);
      ((Environment *) new_pointer)->variable = ((Environment*) (*x))->variable;
      ((Environment *) new_pointer)->value = ((Environment*) (*x))->value;
      ((Environment *) new_pointer)->next = ((Environment*) (*x))->next;
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;
    case SPECIAL_DEFINE_TYPE:
    case SPECIAL_SET_TYPE:
    case SPECIAL_LAMBDA_TYPE:
    case SPECIAL_SYNTAXLAMBDA_TYPE:
    case SPECIAL_IF_TYPE:
    case SPECIAL_EVAL_TYPE:
    case SPECIAL_BEGIN_TYPE:
    case SPECIAL_QUOTE_TYPE:
    case SPECIAL_EVALG_TYPE:
    case SPECIAL_EVALGENERAL_TYPE:
    case SPECIAL_APPLICATION_TYPE:
    case SPECIAL_NOARGS_TYPE:
      old_pointer = (*x);
      SetSpecialPairIn(new_pointer,((*x)->tag));
      new_pointer->car = (*x)->car;
      new_pointer->cdr = (*x)->cdr;
      (*x) = new_pointer;
      old_pointer->tag = BROKENHEART_TYPE;
      old_pointer->car = new_pointer;
      break;

    default:
      HeapInfo(((int) ((*x)->tag)));
      fprintf(stderr,"Last Type Collected is [%d]\n",lasttype);
      fflush(stderr);
      SchemePanic("unknown type / collect");
      break;
  }
  lasttype=((int) ((*x)->tag)); 
}

static unmark_all_atoms(a) 
Atom* a;
{
  a->mark = 0;
}

static wrap_direct(a)
Atom* a;
{
  collect(& (a->direct));
}

static wrap_all_atoms(a)
Atom* a;
{
  if (a->mark || a->special || a->direct_bound) {
    collect(& (a->pname));
  } else {
    DestroyAtom(a);
  }
}

#define SCANADD(x) { ScanIndex += (x); }
#define SCANDEREF  (&((TheHeapA)[ScanIndex]))

garbage_collect()
{
  int i;
  Pair **p;
  Pair **temporary;
  Port* the_old_ports;

  if(ShowGc) {
    fprintf(stderr,"%s","[GC --->");
    fflush(stderr);
  }

  gcflag = 1;		/* AllocHeap und andere Allocs nutzbar */
  the_old_ports = the_ports;
  the_ports = NULL;
  WrapAtom(unmark_all_atoms);

  /* 
   *    noch ungenutzer Bereich B wird zum Bereich A und umgekehrt.
   *   AllocXXX allokiert immer im Bereich A.
   */

  temporary = TheHeapB;
  TheHeapB = TheHeapA;
  TheHeapA = temporary;

  ScanIndex       = 0;
  FreePointer = &(TheHeapA[0]);
  EndPointer = FreePointer+HeapSize;

  collect(&fr1);
  collect(&fr2);
  collect(&fr3);
  collect(&fr4);

  collect(&pr_a);
  collect(&pr_b);
  collect(&pr_c);
  collect(&pr_d);

  collect(& expr);
  collect(& env);
  collect(& newenv);
  collect(& val);
  collect(& fun);
  collect(& argl);
  collect(& unev);

  collect(& initf);
  collect(& exitf);
  collect(& expr_env);

  collect(& errorcallchain);
  collect(& jumpout);
  collect(& jumpin);
  collect(& continuemessage);
  collect(& callchain);

  collect(& the_global_env);

  collect(& errormessage);
  collect(& errorirritant);
  collect(& errorenvironment);
  collect(& errorexpression);

  collect (& the_current_input_port);
  collect (& the_current_output_port);
  collect (& the_current_error_port);

  collect (& the_console_input_port);
  collect (& the_console_output_port);
  collect (& the_console_error_port);

  for (p = the_pstack; p<the_pstackptr; p++)
    collect(& (*p));

  WrapAtom(wrap_direct);

  while (FreePointer != &(TheHeapA[ScanIndex])) {

    switch((int) (((Pair*) (SCANDEREF))->tag)) {
      case PAIR_TYPE:
      case USER_TYPE:
      case SPECIAL_DEFINE_TYPE:
      case SPECIAL_SET_TYPE:
      case SPECIAL_LAMBDA_TYPE:
      case SPECIAL_SYNTAXLAMBDA_TYPE:
      case SPECIAL_IF_TYPE:
      case SPECIAL_EVAL_TYPE:
      case SPECIAL_BEGIN_TYPE:
      case SPECIAL_QUOTE_TYPE:
      case SPECIAL_EVALG_TYPE:
      case SPECIAL_EVALGENERAL_TYPE:
      case SPECIAL_APPLICATION_TYPE:
      case SPECIAL_NOARGS_TYPE:
	collect(& (((Pair*) (SCANDEREF))->car));
	collect(& (((Pair*) (SCANDEREF))->cdr));
	SCANADD(sizeof(Pair)/sizeof(SchemeWord));
	break;
      case ENVIRONMENT_TYPE:
	collect(& (((Environment*) (SCANDEREF))->variable));
	collect(& (((Environment*) (SCANDEREF))->value));
	collect(& (((Environment*) (SCANDEREF))->next));
	SCANADD(sizeof(Environment)/sizeof(SchemeWord));
	break;
      case SYMBOL_TYPE:
	SchemePanic("SYMBOL in heap / scanning");
	break;
      case INTEGER_TYPE:
	SCANADD(sizeof(Integer)/sizeof(SchemeWord));
	break;
      case CHAR_TYPE:
	SchemePanic("CHARTYPE in heap ....");
	break;
      case STRING_TYPE:
        /*
	SCANADD((((C_STRINGLEN((Pair*) (SCANDEREF))+3+1)/sizeof(SchemeWord))+2));
        */
	SCANADD((((C_STRINGLEN((Pair*) (SCANDEREF))+(sizeof(SchemeWord)-1)+1)/sizeof(SchemeWord))+2));
	break;
      case EXTERNAL_TYPE:
	SCANADD((int) ((sizeof(External) / sizeof(SchemeWord)) +
		       ((int) (((External*)(SCANDEREF))->length))));
        break;
      case VECTOR_TYPE:
	for (i=0; i<C_VECTORLEN((Pair*) (SCANDEREF)); i++) {
	  collect(& (C_VECTOR((Pair*) (SCANDEREF))[i]));
	}
	SCANADD((C_VECTORLEN((Pair*) (SCANDEREF)) + 2));
	break;
      case CONTINUATION_TYPE:
      case MCONTINUATION_TYPE:
	for(i=1; i<((int)(sizeof(Continuation)/sizeof(SchemeWord))); i++) {
	  collect(& (C_PAIRARRAY((Pair*) (SCANDEREF))[i]));
	}
	SCANADD(sizeof(Continuation)/sizeof(SchemeWord));
	break;
      case FLOAT_TYPE:
	SCANADD(3);
	break;
      case PRIMITIVE_TYPE:
	collect(& (((Primitive*) (SCANDEREF))->symbol)); 
	SCANADD(sizeof(Primitive)/sizeof(SchemeWord));
	break;
      case COMPOUND_TYPE:
	collect(& (((Compound*) (SCANDEREF))->name));
	collect(& (((Compound*) (SCANDEREF))->environment));
	collect(& (((Compound*) (SCANDEREF))->parameters));
	collect(& (((Compound*) (SCANDEREF))->body));
	SCANADD(sizeof(Compound)/sizeof(SchemeWord));
	break;
      case SYNTAXCOMPOUND_TYPE:
	collect(& (((SyntaxCompound*) (SCANDEREF))->name));
	collect(& (((SyntaxCompound*) (SCANDEREF))->environment));
	collect(& (((SyntaxCompound*) (SCANDEREF))->parameters));
	collect(& (((SyntaxCompound*) (SCANDEREF))->body));
	SCANADD(sizeof(SyntaxCompound)/sizeof(SchemeWord));
	break;
      case INPUTSTRINGPORT_TYPE:
	collect(& (((InputStringPort*) (SCANDEREF))->string));
	SCANADD(sizeof(InputStringPort)/sizeof(SchemeWord));
	break;
      case STATICPORT_TYPE:
	SCANADD(sizeof(StaticPort)/sizeof(SchemeWord));
	break;
      case OUTPUTSTRINGPORT_TYPE:
	collect(& (((OutputStringPort*) (SCANDEREF))->charlist));
	SCANADD(sizeof(OutputStringPort)/sizeof(SchemeWord));
	break;
      case CONSTANT_TYPE:
	SchemePanic("CONSTANT in heap");
	break;
      case BROKENHEART_TYPE:
	SchemePanic("BROKENHEART in Heap");
	break;
      case INPUTPORT_TYPE:
      case OUTPUTPORT_TYPE:
      case POPENR_TYPE:
      case POPENW_TYPE:
	SCANADD(sizeof(Port)/sizeof(SchemeWord));
	break;
      default:
        HeapInfo(((int) (((Pair*) (SCANDEREF))->tag)));
	SchemePanic("unknown type / scanning (gc)");
	break;
    }
  }

  WrapAtom(wrap_all_atoms);

  for(;;) {
    if(the_old_ports == NULL)
      break;
    else {
      if(the_old_ports->tag == INPUTPORT_TYPE ||
	the_old_ports->tag == OUTPUTPORT_TYPE ||
	the_old_ports->tag == POPENR_TYPE ||
	the_old_ports->tag == POPENW_TYPE ||
        the_old_ports->tag == EXTERNAL_TYPE) { /* noch im alten heap ! */
	if(the_old_ports->tag == EXTERNAL_TYPE) {
          if(((External*) the_old_ports)->functions != NULL) {
            if(((External*) the_old_ports)->functions->destroy != NULL) {
	      (*((External*) the_old_ports)->functions->destroy) (((External*) the_old_ports));
	    }
          } 
	} else {
          if(the_old_ports->fd != NULL) {
	    closeport(the_old_ports);
	  }
	}
      }
    }
    /*
     * Das TAG ist hier ungueltig, da bei einem gebrauchten EXTERNAL-Type
     * immer BROKENHEART als tag vorgefunden wird (an dieser Stelle) Mist 
     * Die Struktur fuer EXTERNAL-Types ist jetzt durch dummies dergestalt,
     * dass der next-Pointer an der gleichen Stelle sitzt wie der der Ports....
     */
    the_old_ports = the_old_ports->next;
    /*
    if (the_old_ports->tag == EXTERNAL_TYPE) {
      the_old_ports = (Port*) ((External*) the_old_ports)->next;
    } else {
      the_old_ports = the_old_ports->next;
    }
    */
  }
  gcflag = 0;
  HeapUsed = FreePointer - &(TheHeapA[0]);
  if(ShowGc) {
    fprintf(stderr,"%s","<--- GC]");
    fflush(stderr);
  }
}

/*
   allocation of types
   each allocation-function initializes the tag (type) field in
   the heap and leaves the data-area of the specified type uninitialized.
*/

Pair* AllocPair()
{
  Pair* rc;
  rc = (Pair*) AllocHeap(3);
  rc->tag = PAIR_TYPE;
  return rc;
}

Pair* AllocChar()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (2);
  rc->tag = CHAR_TYPE;
  return(rc);
}

Pair* AllocString(len)
int len;
{
  Pair* rc;
  /*
  rc = (Pair*) AllocHeap ((((len+1) +3) / sizeof(SchemeWord)) + 2); 
  */
  rc = (Pair*) AllocHeap ((((len+1) +(sizeof(SchemeWord)-1)) / sizeof(SchemeWord)) + 2); /* NULLBYTE !! */

  rc->tag = STRING_TYPE;
  rc->car = (Pair*) ((SchemeWord) len);
  C_STRING(rc)[len] = '\000';
  return(rc);
}

Pair* AllocVector(len)
SchemeWord len;
{
  Pair* rc;
  rc = (Pair*) AllocHeap (((int) len) + 2); 
  rc->tag = VECTOR_TYPE;
  rc->car = (Pair*) ((SchemeWord) len);
  return(rc);
}

Pair* AllocContinuation()
{
  Pair* rc;
  Pair** vec;
  int i;

  rc = (Pair*) AllocHeap((sizeof(Continuation)/sizeof(SchemeWord)));
  rc->tag = CONTINUATION_TYPE;

  /* Ausnahme: Alles in Continuation ist 'clean' */

  vec=(Pair**) rc;

  for(i=1; i<((int)(sizeof(Continuation)/sizeof(SchemeWord))); i++) {
    vec[i]=nil_constant;
  }

  return(rc);
}

Pair* AllocInteger(x)
int x;
{
  Pair* rc;
  rc = (Pair*) AllocHeap (2);
  rc->tag = INTEGER_TYPE;
  return(rc);
}

Pair* AllocInputPort(x)
int x;
{
  Pair* rc;
  rc = (Pair*) AllocHeap (6);
  rc->tag = INPUTPORT_TYPE;
  ((Port*) rc)->next = the_ports;
  the_ports = ((Port*) rc);
  return(rc);
}

Pair* AllocOutputPort(x)
int x;
{
  Pair* rc;
  rc = (Pair*) AllocHeap (6);
  rc->tag = OUTPUTPORT_TYPE;
  ((Port*) rc)->next = the_ports;
  the_ports = ((Port*) rc);
  return(rc);
}

Pair* AllocPopenwPort()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (6);
  rc->tag = POPENW_TYPE;
  ((Port*) rc)->next = the_ports;
  the_ports = ((Port*) rc);
  return(rc);
}

Pair* AllocPopenrPort()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (6);
  rc->tag = POPENR_TYPE;
  ((Port*) rc)->next = the_ports;
  the_ports = ((Port*) rc);
  return(rc);
}

Pair* AllocExternalType(int len) {
  Pair* rc;
  rc=(Pair*) AllocHeap ((sizeof(External)/sizeof(SchemeWord)) + len);
  rc->tag=EXTERNAL_TYPE;
  ((External*) rc)->length = len;
  ((External*) rc)->character = -1; 			/* External-Types sind nun auch */
							/* Input-Ports ! */
  ((External*) rc)->next = (External*) the_ports;	/* Einklinken in die Ports-Liste */
  the_ports = ((Port*) rc);
  return(rc);
}

Pair* AllocFloat()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (3);
  rc->tag = FLOAT_TYPE;
  return(rc);
}

Pair* AllocPrimitive()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (sizeof(Primitive)/sizeof(SchemeWord));
  rc->tag = PRIMITIVE_TYPE;
  return(rc);
}

Pair* AllocCompound()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (sizeof(Compound)/sizeof(SchemeWord));
  rc->tag = COMPOUND_TYPE;
  return(rc);
}

Pair* AllocSyntaxCompound()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (sizeof(SyntaxCompound)/sizeof(SchemeWord));
  rc->tag = SYNTAXCOMPOUND_TYPE;
  return(rc);
}

Pair* AllocInputStringPort()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (sizeof(InputStringPort)/sizeof(SchemeWord));
  rc->tag = INPUTSTRINGPORT_TYPE;
  return(rc);
}

/* Input-Port direkt aus Datenbereich in C */

Pair* AllocStaticPort()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (sizeof(StaticPort)/sizeof(SchemeWord));
  rc->tag = STATICPORT_TYPE;
  return(rc);
}

Pair* AllocOutputStringPort()
{
  Pair* rc;
  rc = (Pair*) AllocHeap (sizeof(OutputStringPort)/sizeof(SchemeWord));
  rc->tag = OUTPUTSTRINGPORT_TYPE;
  return(rc);
}

/* 
 * fuer SetXIn-Macros :
 */

PanicWithOutOfHeap() {
  SchemePanic("Out of Heap");
}

HeapInfo (tag) 
int tag;
{
  fprintf(stderr,"\n");
  fprintf(stderr,"HEAPINFO ....\n");
  fprintf(stderr,"Tagfield         : %d\n", tag);
  fprintf(stderr,"ScanIndex        : %d\n", ScanIndex);
  fprintf(stderr,"gcflag           : %d\n", gcflag);
  fflush(stderr);
}

