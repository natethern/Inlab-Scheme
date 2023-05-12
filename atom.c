/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: atom.c,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

static char* rcsid="$Id: atom.c,v 4.102 2006/04/29 09:13:22 tommy Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <extern.h>
#include <global.h>
#include <fdecl.h>
#include <ctype.h>

static Atom*  the_atoms;
static Atom*  freep;		          /* Pointer to next free Atom */
static int    number_of_atoms_allocated;  /* Gesamtzahl allokierter Atome */
static int    number_of_atoms_in_area;	  /* Anzahl pro Allokierung */

InitAtom(int argc, char** argv)
{
  int i;

  static Pair nil_content        = {CONSTANT_TYPE, NULL, NULL};
  static Pair true_content       = {CONSTANT_TYPE, NULL, NULL};
  static Pair false_content      = {CONSTANT_TYPE, NULL, NULL};
  static Pair unassigned_content = {CONSTANT_TYPE, NULL, NULL};

  pstack_o_message = "pstack overflow";
  cstack_o_message = "cstack overflow";

  exit_on_error = 1;
  global_argc = argc;
  global_argv = argv;

  nil_constant = & nil_content;
  true_constant = & true_content;
  false_constant = & false_content;
  unassigned_constant = & unassigned_content;

  if((the_atoms=(Atom*) malloc(SCHEMEInitialAtomSize * sizeof(Atom))) == NULL)
    SchemePanic("cannot allocate atomspace");
  number_of_atoms_allocated = SCHEMEInitialAtomSize ;
  number_of_atoms_in_area = SCHEMEInitialAtomSize ;
  freep = NULL;
  for (i=0; i<SCHEMEInitialAtomSize ; i++) {
    the_atoms[i].next = freep;
    freep = & (the_atoms[i]);
  }
}

Atom* AllocAtom()
{
  Atom *x;
  int i;
  if (freep == NULL) {
    /*					hier wird automatisch neuer Platz fuer
    garbage_collect();			weitere Symbole allokiert und KEINE
    if (freep == NULL)			gc ausgefuehrt (Effizienz...). 
      SchemePanic("cannot allocate atom");
    */
    /* Referenz von the_atoms geht hier verloren, das ist egal ... */
    if((the_atoms = 
	(Atom*) malloc(number_of_atoms_in_area * sizeof(Atom))) == NULL)
      SchemePanic("cannot allocate MORE atomspace");
    number_of_atoms_allocated += number_of_atoms_in_area;
    freep = NULL;
    for (i=0; i<number_of_atoms_in_area; i++) {
      the_atoms[i].next = freep;
      freep = & (the_atoms[i]);
    }
  }
  x = freep;
  freep = x->next;
  x->tag = SYMBOL_TYPE;
  return(x);
}

FreeAtom(Atom* atom)
{
  atom->next = freep;
  freep = atom;
}

GetFreeAtoms() {	/* liefert Anzahl der freien Atome (Symbols) */
  int rc = 0;
  Atom* p;
  p=freep;
  while(p != NULL) {
    rc++;
    p=p->next;
  }
  return(rc);
}

GetTotalAtoms() {	/* liefert Anzahl aller allokierten Atome */
  return(number_of_atoms_allocated);
}

/*
 * Hashing
 */

static Atom** hasharray;
static int    hashsize;

WrapAtom(int (*f)())
{
  Atom* x;
  Atom* the_next;
  int i;

  for (i=0; i<hashsize; i++) {
    x = hasharray[i];
    for(;;) {
      if (x == NULL)
	break;
      else {
	the_next = x->next;
	(*f)(x);
	x = the_next;
      }
    }
  }
}

InitAtomHash(int hsize)
{
  int i;
  if((hasharray = (Atom**) malloc (sizeof(Atom*) * hsize)) == NULL)
    SchemePanic("cannot allocate hashtable");
  for (i=0; i<hsize; i++)
    hasharray[i] = NULL;
  hashsize = hsize;
}

get_hashtable_size() {
  return(hashsize);
}

get_hashtable_used() {
  int rc = 0;
  int i;
  for (i=0; i<hashsize; i++) {
    if(hasharray[i] != NULL)
      rc++;
  }
  return(rc);
}

get_hashtable_max() {
  int rc = 0;
  int trc;
  int i;
  Atom* p;
  for (i=0; i<hashsize; i++) {
    if (hasharray[i] != NULL) {
      trc=1;
      p=(hasharray[i])->next;
      while(p != NULL) {
	trc++;
	p=p->next;
      }
      if (trc>rc)
	rc=trc;
    }
  }
  return rc;
}

/*
 * Externe Interfacefunktionen
 * + Nachbildung von rand und srand ...
 */

static int fold(char *s,int len)
{
  unsigned int rc = 0;
  int i;
  for (i=0; i<len; i++)
    rc = s[i] + 31 * rc;
#ifdef NDEBUG
#else
  /*
  fprintf(stderr,"%d:[%s]\n",rc % hashsize, s);
  */
#endif
  return(rc % hashsize);
}

DestroyAtom(Atom* a)
{
  /*
  printf("destroying atom ");
  print_pname(a);
  printf("\n");
  */

  if (a->prev == NULL) {
    if(a->next == NULL) {
      hasharray[a->hashindex] = NULL; 
    } else { 
      /* a hat Nachfolger aber keinen Vorgaenger ... */
      a->next->prev = NULL;
      if (hasharray[a->hashindex] == a)
        hasharray[a->hashindex] = a->next;
    }
  } else {
    if(a->next == NULL) {
      /* a hat Vorgaenger aber keinen Nachfolger ... */
      a->prev->next = NULL;
      if (hasharray[a->hashindex] == a)
	hasharray[a->hashindex] = a->prev;
    } else {
      a->prev->next = a->next;
      a->next->prev = a->prev;
      if (hasharray[a->hashindex] == a)
	hasharray[a->hashindex] = a->prev;
    }
  }
  FreeAtom(a);
}

Atom* CreateAtom(SchemeFptr function,char *pname,int length)
{
  Atom *a;
  Pair *pstring;
  int hindex;

  a = AllocAtom();
  hindex = fold(pname,length);
  pstring = AllocString(length);
  memcpy(C_STRING(pstring),pname,length);

  a->hashindex = hindex;
  a->pname = pstring;
  a->direct = nil_constant;
  a->direct_bound = 0;
  a->direct_violated = 0;
  a->primitive = function;
  a->arguments = 0;
  a->special_form = SPECIAL_NULL_TYPE;
  if (a->primitive != NULL) {
    a->special = 1;
  }

  if(hasharray[hindex] == NULL) {
    a->prev = NULL;
    a->next = NULL;
  } else {
    a->prev = NULL;
    a->next = hasharray[hindex];
    hasharray[hindex]->prev = a;
  }
  hasharray[hindex] = a;
  return(a);
}

Atom *CreateCAtom(SchemeFptr function, char* pname)
{
  return(CreateAtom(function,pname,strlen(pname)));
}

SchemeInstallPrimitive(SchemeFptr function, char* pname, int arguments)
{
  Atom* a;
  a=GetAtom(pname,strlen(pname));
  if(a->direct != nil_constant) {
    fprintf(stderr,"WARNING: symbol %s already owns primitive\n",
	    C_STRING(((Pair*) a->pname)));
  }
  a->direct_bound = 1;
  a->direct_violated = 0;
  a->direct = (Pair*) AllocPrimitive();
  ((Primitive*) a->direct)->function = function;
  ((Primitive*) a->direct)->arguments = arguments;
  ((Primitive*) a->direct)->symbol = (Pair*) a;
  a->arguments=arguments;
}

Atom *SearchAtom(char* pname,int length)
{
  Atom* current;
  int hindex = fold(pname,length);
  current = hasharray[hindex];
  for(;;) {
    if (current == NULL) {
      return(NULL);
    } else {
      if (length != C_STRINGLEN(current->pname)) {
	current = current->next;
	continue;
      }
      if (!memcmp(pname,C_STRING(current->pname),length)) {
	return(current);
      } else {
	current = current->next;
	continue;
      }
    }
  }
}

Atom* GetAtom(char* pname,int length)
{
  Atom* a;
  if ((a = SearchAtom(pname,length)) == NULL)
    return(CreateAtom(NULL,pname,length));
  else
    return(a);
}

/*
 * GetCAtom wird nur von read() aus aktiviert, pname darf in Abhaengigkeit
 * von SCHEMEIGNORECASE destruktiv veraendert werden ...
 */

Atom* GetCAtom(char* pname)
{
  static char* i;
  for(i=pname; *i!='\000'; i++) {
    if(isupper(*i)) {
      *i = tolower(*i);
    }
  }
  return(GetAtom(pname,strlen(pname)));
}

Atom* GetCSpecialAtom(char* pname)
{
  Atom *a;
  a = GetAtom(pname,strlen(pname));
  a->special = 1;
  return(a);
}

Atom* GetCSpecialSAtom(char* pname,int sform)
{
  Atom *a;
  a = GetAtom(pname,strlen(pname));
  a->special = 1;
  a->special_form = sform;
  return(a);
}

#define INSTALL_ATOM(x) ((Pair*) GetCSpecialAtom((x)))
#define INSTALL_SATOM(x,y) ((Pair*) GetCSpecialSAtom((x),(y)))

/*
 * Definition der von Anfang an existierenden Symbole. Wenn das entsprechende Symbol
 * eine Spezialform ist, dann Bindung an SPECIAL-Type.
 *
 */

InitStandardAtoms()
{
  quote_atom			= INSTALL_SATOM("quote",SPECIAL_QUOTE_TYPE);
  broken_heart   		= INSTALL_ATOM("broken-heart");
  lambda_atom    		= INSTALL_SATOM("named-lambda",SPECIAL_LAMBDA_TYPE);
  syntax_lambda_atom		= INSTALL_SATOM("named-syntax-lambda", SPECIAL_SYNTAXLAMBDA_TYPE);
  pseudo_lambda_atom    	= INSTALL_ATOM("lambda");
  lpar_atom	 		= INSTALL_ATOM("(");
  rpar_atom	 		= INSTALL_ATOM(")");
  dot_atom	 		= INSTALL_ATOM(".");
  define_atom    		= INSTALL_SATOM("define",SPECIAL_DEFINE_TYPE);
  if_atom			= INSTALL_SATOM("if",SPECIAL_IF_TYPE);
  set_atom	 		= INSTALL_SATOM("set!",SPECIAL_SET_TYPE);
  ok_atom	 		= INSTALL_ATOM("ok");
  else_atom	 		= INSTALL_ATOM("else");
  load_atom	 		= INSTALL_ATOM("load");
  eval_atom	 		= INSTALL_SATOM("internal-simple-eval", SPECIAL_EVAL_TYPE);
  evalg_atom			= INSTALL_SATOM("eval-in-global-environment", SPECIAL_EVALG_TYPE);
  eval_general_atom		= INSTALL_SATOM("internal-general-eval", SPECIAL_EVALGENERAL_TYPE);
  begin_atom	 		= INSTALL_SATOM("begin",SPECIAL_BEGIN_TYPE);
  end_of_file_atom 		= INSTALL_ATOM("#<end-of-file>");
  quasiquote_atom		= INSTALL_ATOM("quasiquote");
  unquote_atom			= INSTALL_ATOM("unquote");
  unquote_splicing_atom 	= INSTALL_ATOM("unquote-splicing");
  vector_atom			= INSTALL_ATOM("#");
}
