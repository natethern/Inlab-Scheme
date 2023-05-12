/* Copyright (c) 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: machine.c,v 4.102 2006/04/29 09:13:22 tommy Exp $
 *
 */

static char * rcsid = "$Id: machine.c,v 4.102 2006/04/29 09:13:22 tommy Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <extern.h>
#include <global.h>
#include <fdecl.h>
#include <heap.h>
#include <assert.h>
#include <signal.h>

/*
 *   Moegliche Sprungziele :
 */

enum {
	EV_EVAL_CONT = 0,
	EV_EVALG_CONT,
	EV_EVALG_EXIT,
	EV_DEFINITION_1,
	EV_ASSIGNMENT_1,
	EV_IF_DECIDE,
	SETUP_NO_ARG_APPLY,
	EVAL_ARGS,
	ACCUMULATE_ARG,
	ACCUMULATE_LAST_ARG,
	EVAL_SEQUENCE_CONTINUE, 
	LOAD_EXEC,
	PRINT_RESULT,
	EV_EVAL_GENERAL_EXIT,
	CONTINUE_FROM_VECTOR_ERROR,
	SYNTAXCONT
};

static char conti;

InitMachine()
{
  SetEnvironmentIn(the_global_env);
  ((Environment*) the_global_env)->variable = nil_constant;
  ((Environment*) the_global_env)->value = nil_constant;
  ((Environment*) the_global_env)->next = nil_constant;
}

static SetCmsg(s)
unsigned char* s;
{
  continuemessage = make_string(strlen((char*) s),' ');
  strcpy((char*) C_STRING(continuemessage),(char*) s);
}

static Pair* GetCurrentMContinuation() {
  Pair* x;
  x = SchemeGetInternalContinuation();
  x->tag = MCONTINUATION_TYPE;  
  return(x);
}

static BuildFrame() {
  /*
   * wenn newenv==false_constant, dann bauen wir alle Ergebnisse in
   * argl auf. Ansonsten wird argl schrittweise reduziert und
   * gleichzeitig der komplette Frame der Zielfunktion aufgebaut.
   */
  if(newenv==false_constant) {
    GiveMeWords(sizeof(Pair)/sizeof(SchemeWord));
    ((Pair*) FreePointer)->tag = PAIR_TYPE;
    ((Pair*) FreePointer)->car = val;
    ((Pair*) FreePointer)->cdr = argl;
    argl= (Pair*) FreePointer;
    FreePointer+=sizeof(Pair) / sizeof(SchemeWord);
    return(SCHEME_OK);
  }
  if(argl->tag == PAIR_TYPE) {
    GiveMeWords(sizeof(Environment)/sizeof(SchemeWord));
    ((Environment*) FreePointer)->tag = ENVIRONMENT_TYPE;
    ((Environment*) FreePointer)->variable = argl->car; 
    ((Atom*) argl->car)->direct_violated = 1;
    ((Environment*) FreePointer)->value = val;
    ((Environment*) FreePointer)->next = newenv;
    newenv = (Pair*) FreePointer;
    FreePointer+=sizeof(Environment) / sizeof(SchemeWord);
    argl = argl->cdr;
    return(SCHEME_OK);
  } else {
    if(argl == nil_constant) {
      SchemePushP(val);
      SchemePrint();
      werri ("too many parameters supplied",expr);
      return(SCHEME_ERROR);
    } else {
      if(argl->tag == SYMBOL_TYPE) {
        if((((Environment*) newenv)->variable) == argl) {
	  /* Platz schon da, konsumieren */
	  GiveMeWords(sizeof(Pair)/sizeof(SchemeWord)); 
	  ((Pair*) FreePointer)->tag = PAIR_TYPE; 
	  ((Pair*) FreePointer)->car = val; 
	  ((Pair*) FreePointer)->cdr = nil_constant; 
          { 
	    register Pair* bla; 
	    bla=((Environment*) (newenv))->value; 
	    for(;;) { 
	      if(bla->cdr == nil_constant) { 
		break; 
	      } else { 
		bla=bla->cdr; 
	      } 
	    } 
	    bla->cdr = (Pair*) FreePointer; 
          } 
	  FreePointer+=sizeof(Pair)/sizeof(SchemeWord); 
	  return(SCHEME_OK);
	} else {
          GiveMeWords(sizeof(Environment)/sizeof(SchemeWord) + 
		      sizeof(Pair)/sizeof(SchemeWord));
          ((Environment*) FreePointer)->tag = ENVIRONMENT_TYPE;
          ((Environment*) FreePointer)->variable = argl; 
          ((Atom*) argl)->direct_violated = 1;
          ((Environment*) FreePointer)->value = 
	    (Pair*) (((Environment*) (FreePointer))+1);
	  ((Environment*) FreePointer)->value->tag = PAIR_TYPE; 
	  ((Environment*) FreePointer)->value->car = val; 
	  ((Environment*) FreePointer)->value->cdr = nil_constant; 
          ((Environment*) FreePointer)->next = newenv;
          newenv = (Pair*) FreePointer;
          FreePointer+=sizeof(Environment) / sizeof(SchemeWord) +
		       sizeof(Pair) / sizeof(SchemeWord);
	}
	return(SCHEME_OK);
      } else {
	fprintf(stderr,"argl->tag=%d\n",argl->tag);
        werri ("invalid argl (eigentlich PANIC)",expr);
	return(SCHEME_ERROR);
      }
    }
  }
}

SchemeRunMachine()
{
  goto load_init;

conti_dispatch:

  if (global_errorflag) {	/* ERRORCHECK ... */
    goto error_with_no_return; 
  }

  switch(conti) {
    case EV_EVAL_CONT:	
      goto ev_eval_cont;
    case EV_EVALG_CONT:
      goto ev_evalg_cont;
    case EV_EVALG_EXIT:
      goto ev_evalg_exit;
    case EV_DEFINITION_1:
      goto ev_definition_1;
    case EV_ASSIGNMENT_1:
      goto ev_assignment_1;
    case EV_IF_DECIDE:	
      goto ev_if_decide;
    case SETUP_NO_ARG_APPLY:
      goto setup_no_arg_apply;
    case EVAL_ARGS:	
      goto eval_args;
    case ACCUMULATE_ARG:
      goto accumulate_arg;
    case ACCUMULATE_LAST_ARG:
      goto accumulate_last_arg;
    case EVAL_SEQUENCE_CONTINUE:
      goto eval_sequence_continue;
    case LOAD_EXEC:	
      goto load_exec;
    case PRINT_RESULT:
      goto print_result;
    case EV_EVAL_GENERAL_EXIT:
      goto ev_eval_general_exit;
    case CONTINUE_FROM_VECTOR_ERROR:
      goto continue_from_vector_error;
    case SYNTAXCONT:
      goto syntaxcont;
    default:
      SchemePanic("invalid destination / conti_dispatch");
      break;
  }

error_condition:

  if (global_errorflag) {

    flushport(the_current_error_port);
    flushport(the_current_output_port);

    cep_prints("error: ");
    cep_prints(C_STRING(errormessage));
    cep_prints("\n");

    cep_prints("irrit: ");

    SchemePushP((Pair*) the_current_output_port);
    the_current_output_port=the_current_error_port;
    SchemePushP(errorirritant);
    SchemePrint();		
    the_current_output_port = (Port*) SchemePopP();

    if(exit_on_error) {
      flushport(the_current_error_port);
      exit(2);
    }

    global_errorflag = 0;
    cleanup_at_error();
  }
  goto read_eval_print;

load_init:  

  the_current_input_port = openinitfile();
  if(global_errorflag) {
    SchemePanic("cannot load init.scm");
  }
  ClearStacks();
  goto load_exec;  

load_exec: 

  /*
  ClearStacks();
  */
  global_errorflag = 0;
  SchemeRead();
  expr = SchemePopP();
  if (expr == end_of_file_atom) 
    goto load_exit;   
  env = the_global_env;
  conti = LOAD_EXEC;
  goto eval_dispatch;    

load_exit: 

  closeport(the_current_input_port);
  the_current_input_port = the_console_input_port;
  goto read_eval_print;  

read_eval_print: 

  ClearStacks();		/* alle Stacks initialisieren */
  global_errorflag = 0;   	/* errorflag initialisieren */
  cop_prints (": ");
  SchemeRead();
  if (global_errorflag) {	/* ERRORCHECK ... */
    goto error_with_no_return; 
  }
  expr = SchemePopP();
  if (expr == end_of_file_atom) {
    goto read_eval_print_exit;   
  }
  env = the_global_env;
  conti = PRINT_RESULT;
  goto eval_dispatch;   

read_eval_print_exit:

  exit(0);

print_result:  

  SchemePushP(val);
  SchemePrint();
  goto read_eval_print;      

/*
 * THE DISPATCHER
 *
 */

eval_dispatch:  

  /*
  SchemePushP(expr);
  SchemePrint();
  */

  switch((int) (expr->tag)) {
    /*
     * dies sind die self-evaluating types ...
     */
    case CONSTANT_TYPE:
    case INTEGER_TYPE:
    case FLOAT_TYPE:
    case CHAR_TYPE:
    case STRING_TYPE:
    case PRIMITIVE_TYPE:
    case COMPOUND_TYPE:
    case INPUTPORT_TYPE:
    case OUTPUTPORT_TYPE:
    case CONTINUATION_TYPE:
    case MCONTINUATION_TYPE:
    case INPUTSTRINGPORT_TYPE:
    case OUTPUTSTRINGPORT_TYPE:
    case STATICPORT_TYPE:
      val = expr;
      goto conti_dispatch;

    /*
     * Vektoren sind nicht self-evaluating ...
     */
    case VECTOR_TYPE:
      werri("cannot evaluate vectors",expr);
      val = expr;
      errorcallchain   = callchain;
      errorexpression  = expr;
      errorenvironment = env;
      SchemePushC(conti)
      SchemePushC(CONTINUE_FROM_VECTOR_ERROR);
      jumpin = GetCurrentMContinuation();
      SetCmsg("treating vector as self-evaluating / value ignored");
      goto error_handler_entry; 

continue_from_vector_error:

      conti=SchemePopC();
      val = expr;
      goto conti_dispatch;

    /*
     * lookup symbol in environment ...
     */
    case SYMBOL_TYPE:
#ifdef NDEBUG
#else
      if(((Environment*) env)->variable != nil_constant ||
	 ((Environment*) env)->value != nil_constant) {
	fprintf(stderr,"WARNING: FRAME ERROR\n");
      }
#endif
      {
	 if(((Atom*) expr)->direct_bound && 
	    !(((Atom*) expr)->direct_violated)) {
	   val=((Atom*) expr)->direct;
           if(val == unassigned_constant) {
	     werri("variable unassigned", expr);
	     goto error_in_lookup_variable;
	   } else {
	     goto conti_dispatch;
	   }
	 } else {
	   register Pair* search_env;
	   search_env=env;
	   while(search_env != nil_constant) {
	     if(((Environment*) search_env)->variable == expr) {
	       val=((Environment*) search_env)->value;
	       if(val==unassigned_constant) {
	         werri("variable unassigned", expr);
	         goto error_in_lookup_variable;
	       } else {
	         goto conti_dispatch;
	       }
	     }
	     search_env=(Pair*) (((Environment*) search_env)->next);
	   }
	 }
      }
      if(((Atom*) expr)->direct_bound) {
	val=((Atom*) expr)->direct;
        if(val==unassigned_constant) {
          werri("variable unassigned", expr);
          goto error_in_lookup_variable;
        } else {
          goto conti_dispatch;
        }
      } else {
        werri("variable not found", expr);
        goto error_in_lookup_variable;
      }

error_in_lookup_variable:

      errorcallchain   = callchain;
      errorexpression  = expr;
      errorenvironment = env;
      SchemePushC(conti);
      jumpin = GetCurrentMContinuation();
      SetCmsg("continue with value as obtained from variable");
      goto error_handler_entry; 

    case PAIR_TYPE:
      if (expr->car->tag == SYMBOL_TYPE) {
	switch (((Atom*) (expr->car))->special_form) {
	  case SPECIAL_NULL_TYPE: 	/* keine Spezialform */
	    break;
	  case SPECIAL_DEFINE_TYPE:
            expr->tag=SPECIAL_DEFINE_TYPE;
	    goto ev_definition;   
	  case SPECIAL_SET_TYPE:
            expr->tag=SPECIAL_SET_TYPE;
	    goto ev_assignment;   
	  case SPECIAL_LAMBDA_TYPE:
            expr->tag=SPECIAL_LAMBDA_TYPE;
	    goto ev_lambda;   
	  case SPECIAL_SYNTAXLAMBDA_TYPE:
            expr->tag=SPECIAL_SYNTAXLAMBDA_TYPE;
	    goto ev_syntaxlambda;   
	  case SPECIAL_IF_TYPE:
            expr->tag=SPECIAL_IF_TYPE;
	    goto ev_if;   
	  case SPECIAL_EVAL_TYPE:
            expr->tag=SPECIAL_EVAL_TYPE;
	    goto ev_eval;   
	  case SPECIAL_BEGIN_TYPE:
            expr->tag=SPECIAL_BEGIN_TYPE;
	    goto ev_seq;    
	  case SPECIAL_QUOTE_TYPE:
            expr->tag=SPECIAL_QUOTE_TYPE;
	    val = expr->cdr->car;
	    goto conti_dispatch; 
          case SPECIAL_EVALG_TYPE:
            expr->tag=SPECIAL_EVALG_TYPE;
	    goto ev_evalg;   
	  case SPECIAL_EVALGENERAL_TYPE:
	    /*
	    ... wegen syntax-lambda !
            expr->tag=SPECIAL_EVALGENERAL_TYPE;
            */
	    goto ev_eval_general;
	  default:
	    fprintf(stderr,"special form: %d\n",
		    (((Atom*) (expr->car))->special_form));
	    fflush(stderr);
	    SchemePanic("INTERNAL ERROR: invalid special form");
	    break;
	} /* Ende Special-Form-Switch .... */
      }  /* Ende der Behandlung von Symbolen in expr->car */

      if (IS_NIL(expr->cdr)) {
        expr->tag=SPECIAL_NOARGS_TYPE;
        goto ev_no_args;   
      }
      /*
      expr->tag = SPECIAL_APPLICATION_TYPE;
      */
      goto ev_application;    

    case SPECIAL_DEFINE_TYPE:
      goto ev_definition;
    case SPECIAL_SET_TYPE:
      goto ev_assignment;
    case SPECIAL_LAMBDA_TYPE:
      goto ev_lambda;
      
    case SPECIAL_SYNTAXLAMBDA_TYPE:
      goto ev_syntaxlambda;
    case SPECIAL_IF_TYPE:
      goto ev_if;   
    case SPECIAL_EVAL_TYPE:
      goto ev_eval;
    case SPECIAL_BEGIN_TYPE:
      goto ev_seq;
    case SPECIAL_QUOTE_TYPE:
      val = expr->cdr->car;
      goto conti_dispatch; 
    case SPECIAL_EVALG_TYPE:
      goto ev_evalg;
    case SPECIAL_EVALGENERAL_TYPE:
      goto ev_eval_general;
    case SPECIAL_APPLICATION_TYPE:
      goto ev_application;    
    case SPECIAL_NOARGS_TYPE:
      goto ev_no_args;

    default:
      fprintf(stderr,"TYP: %d\n", ((int) (expr->tag)));
      SchemePanic("invalid type / eval_dispatch");
      break;
  }

ev_seq:  

  val = nil_constant;
  unev = expr->cdr;
  SchemePushC(conti);
  goto eval_sequence;     

/*
 * eval
 *
 */

ev_eval:  

  expr = expr->cdr;
  if (! IS_PAIR(expr)) {
    werri ("syntax-error / eval",expr);
    goto error_with_no_return;
  }
  if (IS_NIL(expr)) {
    werr ("syntax-error (no expression) / eval");
    goto error_with_no_return;
  }
  expr = expr->car;
  SchemePushP(env);
  SchemePushC(conti);
  conti = EV_EVAL_CONT;
  goto eval_dispatch;    

ev_eval_cont:  

  conti = SchemePopC();
  env = SchemePopP();
  expr = val;
  goto eval_dispatch;     

/*
 * eval-in-global-environment
 *
 */

ev_evalg: 

  expr = expr->cdr;
  if (! IS_PAIR(expr)) {
    werri ("syntax-error / eval",expr);
    goto error_with_no_return;
  }
  if (IS_NIL(expr)) {
    werr ("syntax-error (no expression) / eval");
    goto error_with_no_return;
  }
  expr = expr->car;
  SchemePushP(env);
  SchemePushC(conti);
  conti = EV_EVALG_CONT;
  goto eval_dispatch;    

ev_evalg_cont:  

  conti = EV_EVALG_EXIT;
  env = the_global_env;
  expr = val;
  goto eval_dispatch;    


ev_evalg_exit: 

  conti = SchemePopC();
  env = SchemePopP();
  goto conti_dispatch;

/* 
 * internal-eval-general
 *
 */

ev_eval_general:

  SchemePushP(env);
  SchemePushC(conti);
  conti = EV_EVAL_GENERAL_EXIT;
  env = expr_env->cdr;
  expr = expr_env->car;
  goto eval_dispatch;    

ev_eval_general_exit:

  conti = SchemePopC();
  env = SchemePopP();
  goto conti_dispatch;

/*
 * define
 *
 */

ev_definition:  

  /*
   * Variablenname in unev ...
   * vorher definition_variable in util.c ...
   */

  if (!IS_PAIR(expr->cdr)) {
    werri ("too few arguments / define",expr);
    goto error_with_no_return;
  }
  if (IS_PAIR(expr->cdr->car)) { 
     unev=expr->cdr->car->car;
  } else {
    if ( !IS_SYMBOL(expr->cdr->car)) {
      werri ("invalid variable-name / define",expr->cdr->car);
      goto error_with_no_return;
    }
    unev=expr->cdr->car;
  }

  SchemePushP(unev);

  /*
   *  definition_value in expr ...
   *  (Syntaktische Pruefungen sind bereits erfolgt)
   *
   *  (define (a x) x)  oder
   *  (define a)	oder
   *  (define a 1)
   *
   *  (named-lambda (a x) x)
   */

  if (IS_PAIR(expr->cdr->car)) {		/* (define (a x) x) */
    GiveMeWords(3*3); 				/* drei Pairs */
    *(FreePointer+0) = (Pair*) PAIR_TYPE;
    *(FreePointer+3) = (Pair*) PAIR_TYPE;
    *(FreePointer+6) = (Pair*) PAIR_TYPE;
    *(FreePointer+1) = lambda_atom;
    *(FreePointer+2) = (Pair*) (FreePointer+3);
    *(FreePointer+4) = (Pair*) (FreePointer+6);
    
    *(FreePointer+7) = unev;
    *(FreePointer+8) = expr->cdr->car->cdr;
    *(FreePointer+5) = expr->cdr->cdr;
    expr = (Pair*) FreePointer; 
    FreePointer += (3*(sizeof(Pair)/sizeof(SchemeWord)));
  } else {
    if (expr->cdr->cdr == nil_constant) {
      expr=nil_constant;
    } else {
      if(IS_PAIR(expr->cdr->cdr)) {
        expr=expr->cdr->cdr->car;
      } else {
        werri ("syntax error / define", expr);
        goto error_with_no_return;
      }
    }
  }

  SchemePushP(env);
  SchemePushC(conti);

  conti = EV_DEFINITION_1;
  goto eval_dispatch;    

ev_definition_1:  

  conti = SchemePopC();
  env = SchemePopP();
  unev = SchemePopP();

  /* Definition einer Variablen, unev=Symbol, val=Erwuenschter Wert */

  if(env == the_global_env) {
    if(((Atom*) unev)->direct_bound) {
      ((Atom*) unev)->direct = val;    	
      val = unev;
      goto conti_dispatch; 
    } else {
      ((Atom*) unev)->direct = val;  
      ((Atom*) unev)->direct_bound = 1;
      val = unev;
      goto conti_dispatch; 
    }
  } else {
    register Environment* search;
    search=(Environment*) (((Environment*) env)->next);
    while(search != (Environment*) nil_constant && 
      search->value != (Pair*) nil_constant) {
      if(search->variable == unev) {
	search->value = val;
	val = unev;
	goto conti_dispatch; 
      }
      search=(Environment*) (search->next);
    }
    GiveMeWords(4);
    ((Environment*) FreePointer)->tag = ENVIRONMENT_TYPE;
    ((Environment*) FreePointer)->variable = unev;
    ((Environment*) FreePointer)->value = val;
    ((Environment*) FreePointer)->next = ((Environment*) env)->next;
    ((Environment*) env)->next = ((Pair*) FreePointer);

    ((Atom*) unev)->direct_violated = 1;

    FreePointer+=4;
    val = unev;
    goto conti_dispatch;
  }

ev_assignment:  

  /*
   * Syntax-Check fuer set!
   *
   */

  if(expr->cdr->tag != PAIR_TYPE)
    goto assignment_syntax_error;
  if(expr->cdr->car->tag != SYMBOL_TYPE) 
    goto assignment_syntax_error;
  if(expr->cdr->cdr->tag != PAIR_TYPE &&
     (expr->cdr->cdr->tag < SPECIAL_START_TYPE ||
     expr->cdr->cdr->tag > SPECIAL_END_TYPE))
    goto assignment_syntax_error;

  unev = expr->cdr->car;		/* Symbol in unev */
  SchemePushP(unev);
  expr = expr->cdr->cdr->car;

  SchemePushP(env);
  SchemePushC(conti);

  conti = EV_ASSIGNMENT_1;
  goto eval_dispatch;    

assignment_syntax_error:

  werri("syntax-error / set!",expr);
  goto error_with_no_return;

ev_assignment_1:  

  conti = SchemePopC();			
  env = SchemePopP();
  unev = SchemePopP();

  /*
   * env  = Environment
   * unev = Symbol
   * val  = Sollwert
   *
   */

  if(((Atom*) unev)->direct_bound &&
     !((Atom*) unev)->direct_violated) {
    ((Atom*) unev)->direct = val;
    val = unev;
    goto conti_dispatch;
  } else {
    register Environment* search;
    search=(Environment*) (((Environment*) env)->next);
    while(search != (Environment*) nil_constant) {
      if(search->variable == unev) {
	search->value = val;
	val = unev;
	goto conti_dispatch; 
      }
      search=(Environment*) (search->next);
    }
  }

  if(((Atom*) unev)->direct_bound) {
    ((Atom*) unev)->direct = val;
    val = unev;
    goto conti_dispatch; 
  } else {
    werri ("variable not found / set!",unev);
    goto error_with_no_return;
  }

  /* 
   *  (named-lambda (name <parameters>) <body>)
   */

ev_lambda:  

  if (!IS_PAIR(expr->cdr) || !IS_PAIR(expr->cdr->car)) {
    werri ("syntax-error / named-lambda",expr);
    goto error_with_no_return;
  }
  val = AllocCompound();
  ((Compound*) val)->name = expr->cdr->car->car;
  ((Compound*) val)->parameters = expr->cdr->car->cdr;
  ((Compound*) val)->environment = env;
  ((Compound*) val)->body = expr->cdr->cdr;
  goto conti_dispatch;

ev_syntaxlambda:  

  if (!IS_PAIR(expr->cdr) || !IS_PAIR(expr->cdr->car)) {
    werri ("syntax-error / syntax-lambda",expr);
    goto error_with_no_return;
  }
  val = AllocSyntaxCompound();
  ((Compound*) val)->name = expr->cdr->car->car;
  ((Compound*) val)->parameters = expr->cdr->car->cdr;
  ((Compound*) val)->environment = env;
  ((Compound*) val)->body = expr->cdr->cdr;
  goto conti_dispatch;

ev_if:   

  if (!IS_PAIR(expr->cdr) || IS_NIL(expr->cdr->cdr)) {
    werri ("syntax-error / if",expr);
    goto error_with_no_return;
  }
  SchemePushC(conti);
  unev = expr->cdr->cdr;
  expr = expr->cdr->car;	/* Praedikat in expr */
  conti = EV_IF_DECIDE;
  SchemePushP(env)
  SchemePushP(unev);
  goto eval_dispatch;    

ev_if_decide:  

  unev = SchemePopP();
  env  = SchemePopP();
  if (val != false_constant) {
    expr = unev->car;		/* wahr */
    conti = SchemePopC();
    goto eval_dispatch;    
  } else {
    unev = unev->cdr;
    if (unev == nil_constant) {
      val = nil_constant;	/* falsch, keine Alternative */
      conti = SchemePopC();
      goto conti_dispatch;  
    } else {
      expr = unev->car;		/* falsch und Alternative */
      conti = SchemePopC();
      goto eval_dispatch;    
    }
  }

ev_no_args:  

  expr = expr->car;		/* procedure ermitteln ... */
  SchemePushC(conti);
  conti = SETUP_NO_ARG_APPLY;       
  goto eval_dispatch;    

setup_no_arg_apply:  

  fun = val;

  if(fun->tag == PRIMITIVE_TYPE) {
    if(((Primitive*)fun)->arguments == 0) {
      (*(((Primitive*)fun)->function))();
      goto primitive_exit;
    } else {
      werri ("no arguments supplied",fun);
      goto error_with_no_return;
    }
  }

  if(fun->tag==SYNTAXCOMPOUND_TYPE) {
    unev = nil_constant;
    /*
    SchemePushC(conti);
    SchemePushC(conti);
    */
    SetPairIn(expr);
    expr->car=((Compound*) fun)->name;
    expr->cdr = nil_constant;
    goto syntaxcompound_apply;
  }

  argl=((Compound*) fun)->parameters;
  newenv=((Compound*) fun)->environment;

  goto apply_dispatch;

  /*
   * Auswertung des Funktionsarguments
   */

ev_application:  

  SchemePushP(expr); 
  unev = expr->cdr;
  expr = expr->car;
  SchemePushC(conti);
  SchemePushP(env)
  SchemePushP(unev)
  conti = EVAL_ARGS;
  goto eval_dispatch;    

  /*
   * Auswertung der Argumente, sammeln in argl 
   */

eval_args:  

  unev = SchemePopP();
  env = SchemePopP();
  expr = SchemePopP(); 

  fun = val;			       	/* hier ist das Ziel bekannt ! */

  switch(fun->tag) {
    case SYNTAXCOMPOUND_TYPE:
      goto syntaxcompound_apply;
    case PRIMITIVE_TYPE:
      SchemePushP(fun);
      argl=nil_constant;
      newenv=false_constant;		/* Markierung fuer speziellen Sammelmodus
					 * fuer Primitives. Siehe BuildFrame !
					 */
      goto eval_arg_loop;
    case COMPOUND_TYPE:
    case CONTINUATION_TYPE:
    case MCONTINUATION_TYPE:
      SchemePushP(fun);
      argl=((Compound*) fun)->parameters;
      newenv=((Compound*) fun)->environment;
      goto eval_arg_loop;   		
    default:           
      werri ("unknown type of procedure",fun);
      goto error_with_no_return;
  }

primitive_exit:

  if (global_errorflag) {
    errorcallchain = callchain;
    errorexpression = expr;
    errorirritant = fun;
    errorenvironment = env;
    jumpin = GetCurrentMContinuation();
    SetCmsg("continue with value as value from primitive");
    goto error_handler_entry; 
  }
  conti = SchemePopC();
  goto conti_dispatch;

eval_arg_loop:  

  SchemePushP(argl); 
  SchemePushP(newenv);

  expr = unev->car;
  if (IS_NIL(unev->cdr)) {
     goto eval_last_arg;    
  }
  SchemePushP(env);
  SchemePushP(unev);
  conti = ACCUMULATE_ARG;
  goto eval_dispatch;    

accumulate_arg:  

  unev = SchemePopP();
  env = SchemePopP();

  newenv=SchemePopP(); 
  argl=SchemePopP();

  /* CollectInArgl(val);  */		/* nicht das letzte Argument ... */

  if(BuildFrame() == SCHEME_ERROR)
    goto error_with_no_return;
  unev = unev->cdr;
  goto eval_arg_loop;    

eval_last_arg:   

  conti = ACCUMULATE_LAST_ARG;
  goto eval_dispatch;     

accumulate_last_arg:  

  newenv=SchemePopP(); 
  argl=SchemePopP();

  /* CollectInArgl(val); */		/* Das letzte Argument */
  if(BuildFrame() == SCHEME_ERROR)
    goto error_with_no_return;
  fun = SchemePopP();

  goto apply_dispatch;    

apply_dispatch:  

  switch((int)(fun->tag)) {
    case COMPOUND_TYPE:
      goto compound_apply;     

    /* 
     * HMMM...
     *
     */

    case SYNTAXCOMPOUND_TYPE:
      goto compound_apply;     

    case PRIMITIVE_TYPE:
      goto primitive_apply;    
    case CONTINUATION_TYPE:
      SetPairIn(val);
      val->car = nil_constant;		/* temporaer */
      val->cdr = ((Environment*) newenv)->value;
      {
        static Pair* tmp;
        SetPairIn(tmp);
        val->car = tmp;
        val->car->car = initf;
        val->car->cdr = exitf;
      }
      SchemeSetInternalContinuation(fun);
      goto continuation_entry;
    case MCONTINUATION_TYPE:
      val = ((Environment*) newenv)->value;
      SchemeSetInternalContinuation(fun);
      goto continuation_entry;
    default:
      werri ("unknown type of procedure",fun);
      goto error_with_no_return;
      /* break; */
  }

primitive_apply:  

  if((((Primitive*) fun)->arguments) == list_length(argl)) {
    (*(((Primitive*)fun)->function))();
  } else {
    werri ("invalid number of arguments",fun);
    goto error_with_no_return;
  }
  goto primitive_exit;

continuation_entry:
  conti = SchemePopC();
  goto conti_dispatch;

/*
 * Einsprung ueber Continuation in neuen Error-Handler.
 * Hier Default-Einsprung.
 *
 */

error_handler_entry:

  if (jumpout == nil_constant)
    goto error_condition;
  global_errorflag = 0;		/* wichtig ! */
  SetPairIn(val);
  val->car = nil_constant;		/* temporaer */
  val->cdr = nil_constant;		/* egal ... */
  {
    Pair* tmp;
    SetPairIn(tmp);
    val->car=tmp;
    val->car->car = initf;
    val->car->cdr = exitf;
  }
  SchemeSetInternalContinuation(jumpout);
  goto continuation_entry;

/*
 */

syntaxcompound_apply:

  /*
   *   fun: Syntax-Funktion ...
   *   expr:
   */

  SchemePushP(argl); 
  SchemePushP(newenv);

  SetEnvironmentIn(newenv);		
  ((Environment*) newenv)->variable = ((Compound*) fun)->parameters->car;
  ((Atom*) ((Compound*) fun)->parameters->car)->direct_violated = 1;
  assert(((Environment*) newenv)->variable != NULL);
  ((Environment*) newenv)->value = nil_constant;
  ((Environment*) newenv)->next  = nil_constant;

  /*
   * Zusammenbasteln des urspruenglichen Ausdrucks
   * in argl...
   */

  SetPairIn(argl);

  /*
   * Expression sollte hier aber ein pair sein ...
   */

  argl->car = expr->car;
  argl->cdr = unev;

  ((Environment*) newenv)->value = argl;

  SetEnvironmentIn(argl);				/* temporaer */
  ((Environment*) newenv)->next = argl;

  /* TEST */
  ((Environment*) argl)->variable = ((Compound*) fun)->parameters->cdr->car;
  ((Atom*) ((Compound*) fun)->parameters->cdr->car)->direct_violated = 1;
  assert(((Environment*) argl)->variable != NULL);
  ((Environment*) argl)->value = env;
  ((Environment*) argl)->next = ((Compound*) fun)->environment;

  argl = nil_constant;

  SchemePushC(conti);
  SchemePushC(SYNTAXCONT);
  SchemePushP(env);
  SchemePushP(expr);
  goto apply_dispatch;     

syntaxcont:
  expr = SchemePopP();
  env = SchemePopP();

  /*
  printf("EXPR: (%d) ",expr->tag);
  SchemePushP(expr); 
  SchemePrint();
  printf("VAL: (%d) ",val->tag);
  SchemePushP(val);
  SchemePrint();
  */

  if(DestructiveMacros) {
    if (IS_PAIR(val) && IS_PAIR(expr)) {
      expr->car = val->car;		/* Destruktive Expansion */
      expr->cdr = val->cdr; 
    } else {
      expr = val;			/* wenn kein pair ... */
    }
  } else {
    expr = val;				/* nicht destr. Expansion */
  }
  conti = SchemePopC();        
  conti = SchemePopC();         	/* WICHTIG */

  newenv=SchemePopP(); 
  argl=SchemePopP();

  goto eval_dispatch;    

compound_apply:  

  /*
   * argl    =   Argument-Vektor      
   * fun     =   Prozedur
   *
   */

  if(argl->tag == SYMBOL_TYPE) {
    if((((Environment*) newenv)->variable) != argl) {
      GiveMeWords(sizeof(Environment)/sizeof(SchemeWord));
      ((Environment*) FreePointer)->tag = ENVIRONMENT_TYPE;
      ((Environment*) FreePointer)->variable = argl; 
      ((Atom*) argl)->direct_violated = 1;
      ((Environment*) FreePointer)->value = nil_constant;
      ((Environment*) FreePointer)->next = newenv;
      newenv = (Pair*) FreePointer;
      FreePointer+=(sizeof(Environment)/sizeof(SchemeWord));
    }
    argl = nil_constant;
  }

  if(argl != nil_constant) {
    werri ("too few parameters supplied",expr);
    goto error_with_no_return;
  }

  SetEnvironmentIn(env);
  ((Environment*) env)->variable = nil_constant;
  ((Environment*) env)->value = nil_constant;
  ((Environment*) env)->next = newenv;
  
  unev=(((Compound*) fun)->body);
  goto eval_sequence;

/* 
 * Achtung: EVAL-SEQUENCE muss die Continuation auf dem Stack haben !
 */

eval_sequence:  
  if (IS_NIL(unev)) {			/* last expression */
    conti = SchemePopC();
    goto conti_dispatch;
  }
  expr = unev->car;
  if (IS_NIL(unev->cdr)) { 
    conti = SchemePopC();
    goto eval_dispatch;    
  }
  SchemePushP(unev);
  SchemePushP(env);	
  conti = EVAL_SEQUENCE_CONTINUE;
  goto eval_dispatch;    

eval_sequence_continue:  
  env  = SchemePopP();
  unev = SchemePopP();
  unev = unev->cdr;
  goto eval_sequence;    

error_with_no_return:
  errorcallchain   = callchain;
  errorexpression  = expr;
  errorenvironment = env;
  jumpin = nil_constant;		/* no return ... */
  SetCmsg("this message should not appear");
  goto error_handler_entry; 
}

SchemePanicprint() {
  flushport(the_current_error_port);
  flushport(the_current_output_port);
  cep_prints("errormessage@panic=");
  cep_prints(C_STRING(errormessage));
  cep_prints("\n");
  cep_prints("irritant@panic=");
  SchemePushP((Pair*) the_current_output_port);
  the_current_output_port=the_current_error_port;
  SchemePushP(errorirritant);
  SchemePrint();
  the_current_output_port = (Port*) SchemePopP();
  flushport(the_current_error_port);
  flushport(the_current_output_port);
}

/*
 * Signalbehandlung
 */

static void handler (s)
int s;
{
  switch(s) {
  case SIGINT:
  case SIGQUIT:
    werr ("**** BREAK ****");
    break;
  case SIGFPE:
    werr("floating point exception");
    break;
  default:
    werr("unexected signal received");
    break;
  }
  signal (s,handler);
}

SignalInit()
{
  signal (SIGINT, handler);
  signal (SIGFPE, handler);
}

SignalDisableInterrupts()
{
  signal (SIGINT, SIG_IGN);
  /*
  signal (SIGQUIT, SIG_IGN);
  */
}

SignalEnableInterrupts()
{
  signal (SIGINT, handler);
  /*
  signal (SIGQUIT, handler);
  */
}

SchemePanic(char* s)
{
  fprintf(stderr,"\nFATAL ERROR: %s\n\n",s);
  fflush(stderr);
  SchemePanicprint();
  fprintf(stderr,"dumping core ...\n");
  fflush(stderr);
  kill(getpid(),10);
  exit(1);
}


