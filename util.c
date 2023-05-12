/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: util.c,v 4.103 2006/04/29 09:13:22 tommy Exp $
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <limits.h>
#include <scheme.h>
#include <extern.h>
#include <global.h>
#include <fdecl.h>
#include <heap.h>

char *strchr();

#define ERROR_EOF	1
#define ERROR_PARSE	2

static char * rcsid="$Id: util.c,v 4.103 2006/04/29 09:13:22 tommy Exp $";

static int  token_length;
static Pair*  last_printed;
static jmp_buf error_env;
int    werr();

static p_readl();
static p_read_head();
static p_read_tail();
static p_read_cdr();
static int get_token(char**);
static p_print_it();
static p_prin0();
static p_printbody();
static p_print_item();
static make_symbol_or_number();
static make_long();
static make_constant();
static make_char();
static make_stringt();
static print_pname();
static print_pname_e();
static print_integer();
static print_float();
static print_primitive();
static print_char();
static print_string();
static handle_hash();
static handle_string();
static unget_c(char);

Pair* make_string(int, unsigned char);

static long the_long;
static double the_double;
static int display_flag = 0;
static int printout_flag = 0;

InitUtil() {
  the_console_input_port = createstdin();
  the_console_output_port = createstdout();
  the_console_error_port = createstderr();
  the_current_input_port = the_console_input_port;
  the_current_output_port = the_console_output_port;
  the_current_error_port = the_console_error_port;
}

#ifdef NDEBUG
#else
static TokenTest(int x, char* y) {
/*
  fprintf(stderr,"%d[%s]\n",x,y);
*/
}
#endif

/*
 * wird im Fehlerfall aufgerufen,
 * alle Ports schliessen, Standardports setzen.
 *
 */

cop_prints(s)
char* s;
{
  char *p = s;
  while (*p != '\000') {
    writechar(the_current_output_port,*p);
    p++;
  }
}

cop_printc(c)
int c;
{
  writechar(the_current_output_port,c);
}

cep_prints(s)
char* s;
{
  char *p = s;
  while (*p != '\000') {
    writechar(the_current_error_port,*p);
    p++;
  }
}

cleanup_at_error() {
  the_current_input_port = the_console_input_port;
  the_current_output_port = the_console_output_port;
  the_current_error_port = the_console_error_port;
}

/*
 *   neue Utility-Functions
 */

static char* Scheme_strtof(char* p, double *res)
{
    double acc;
    int sign;
    double fpos;
    int exp;
    int exps;
    acc = 0.0;
    sign = 1;
    exp = 0;
    exps = 1;
    if (*p == '+')
        p++;
    else if (*p == '-') {
        p++;
        sign = -1;
    }
    while ((*p) >= '0' && (*p) <= '9') {
        acc = acc * 10.0 + (double)(*p - '0');
        p++;
    }
    if (*p == 'e' || *p == 'E') {
	    p++;
        if (*p == '+')
	    p++;
        else if (*p == '-') {
	    p++;
	    exps = -1;
        }
        while((*p) >= '0' && (*p) <= '9') {
	    exp = exp * 10 + (*p - '0');
	    p++;
        }
    }
    if (*p == '.') {
	fpos = 1.0/10.0;
	p++;
	while((*p) >= '0' && (*p) <= '9'){
	    acc += (*p - '0') * fpos;
	    fpos *= 1.0/10.0;
	    p++;
	}
    }
    if (*p == 'e' || *p == 'E' || *p == 's' || *p == 'S' ||
        *p == 'f' || *p == 'F' || *p == 'd' || *p == 'D' ||
	*p == 'l' || *p == 'L') {   /* IEEE-Exponent Markers */
	exp = 0;
	exps = 1;
        p++;
	if (*p == '+')
	    p++;
	else if (*p == '-') {
	    p++;
	    exps = -1;
	}
	while((*p) >= '0' && (*p) <= '9'){
	    exp = exp * 10 + (*p - '0');
	    p++;
	}
    }
    if (exp) {
	if (exps > 0)
	    while (exp--)
		acc *= 10.0;
	else
	    while (exp--)
		acc *= 1.0/10.0;
    }
    if (sign > 0)
        *res = acc;
    else
	*res = -acc;
    return(p);
}

/*
 * int (*errf)();   <=>  Error-Function receiving char* 
 *
 */

int StringToNumber(char *s,long *l,double *d,int radix,int (*errf)())
{
  char *x;
  int exact=1;
  int sign=1;
  int i;

  if(strlen(s) >= 1) {
    if ((*s >= 'a' && *s <='z')) {
      if (radix == 16) {
	if (*s == 'a' || *s == 'b' || *s == 'c' || *s == 'd' ||
	    *s == 'e' || *s == 'f') {
	  goto restart;
	} else {
	  return(SchemeNONUMBER);
	}
      } else {
        return(SchemeNONUMBER);
      }
    }
    if ((*s >= 'A' && *s <='Z')) {
      if (radix == 16) {
	if (*s == 'A' || *s == 'B' || *s == 'C' || *s == 'D' ||
	    *s == 'E' || *s == 'F') {
	  goto restart;
	} else {
	  return(SchemeNONUMBER);
	}
      } else {
        return(SchemeNONUMBER);
      }
    }
  }
restart:
  if(strlen(s) == 1 && ((*s) < '0' || (*s) > '9'))
    return(SchemeNONUMBER);
  if (*s == '#') {
    s++;
    switch(*s) {
      case 'e': s++; exact = 1; goto restart;
      case 'i': s++; exact = 0; goto restart;
      case 'd': s++; radix = 10; goto restart;
      case 'o': s++; radix = 8; goto restart;
      case 'x': s++; radix = 16; goto restart;
      case 'b': s++; radix = 2; goto restart;
      default: return(SchemeNONUMBER); 
    }
  }
  if (radix == 10) {
    if (strchr(s,'e') != NULL || strchr(s,'E') != NULL ||
	strchr(s,'.') != NULL || strchr(s,'s') != NULL ||
	strchr(s,'S') != NULL || strchr(s,'f') != NULL ||
	strchr(s,'F') != NULL || strchr(s,'l') != NULL ||
	strchr(s,'L') != NULL || strchr(s,'#') != NULL)
      exact = 0;
    x = Scheme_strtof(s,d);
    if (*x != '\000') {
      return(SchemeNONUMBER);
    } else {
      if (exact) {
	if (*d > LONG_MAX || *d < LONG_MIN) { 
	  /*
	  (*errf)("number not representable in exact");
	  return(SchemeNONUMBER);
	  */
	  return(SchemeDOUBLE);
	}
	*l = *d;
	return(SchemeLONG);
      } else {
	return(SchemeDOUBLE);
      }
    }
  } else {                
    if (*s == '-') {
      sign = -1;
      s++;
    } else {
      if (*s == '+') {
	sign = 1;
	s++;
      }
    }
    *d = 0.0;

    for(;;) {
      switch(radix) {
        case 2:
	  (*d) = (*d)  * 2;
          switch(*s) {
	    case '1':
	      (*d) = (*d) + 1;
	      break;
	    case '0':
	      break;
	    default:
	      (*errf)("invalid number radix 2");
	      return(SchemeNONUMBER);
	  }
	  break;
        case 8:
	  (*d) = (*d) * 8;
	  if ((*s) >= '0' && (*s) <= '7') {
	    (*d) = (*d) + ((*s) - '0');
	  } else {
	    (*errf)("invalid number radix 8");
	    return(SchemeNONUMBER);
	  }
	  break;
        case 16:
	  if ((*s) >= 'a' && (*s) <= 'f') {
	    i = (*s) - 'a' + 10;
	  } else if ((*s) >= 'A' && (*s) <= 'F') {
	    i = (*s) - 'A' + 10;
	  } else if ((*s) >= '0' && (*s) <= '9') {
	    i = (*s) - '0';
	  } else {
	    (*errf)("invalid number radix 16");
	    return(SchemeNONUMBER);
	  }
	  (*d) = (*d) * 16;
	  (*d) = (*d) + i; 
	  break;
        case 10:
	  (*d) = (*d) * 10;
	  if ((*s) >= '0' && (*s) <= '9') {
	    (*d) = (*d) + ((*s) - '0');
	  } else {
	    (*errf)("invalid number radix 10");
	    return(SchemeNONUMBER);
	  }
	  break;
        default:
	  (*errf)("INTERNAL ERROR / invalid radix");
	  return(SchemeNONUMBER);
      }
      if(*(++s) == '\000') 
	break;
    } 
    *d = *d * sign;
    if (*d > LONG_MAX || *d < LONG_MIN) { 
      /*
      (*errf)("number not representable in exact");
      return(SchemeNONUMBER);
      */
      return(SchemeDOUBLE);
    }
    *l = *d;
    return(SchemeLONG);
  }
}

/*
 * 
 */

SchemeRead()
{
  int value;
  Pair** p_stack_pointer = the_pstackptr;
  Pair* t;

  if ((value = setjmp(error_env)) == 0) {
    p_readl();
  } else {
    switch (value) {
      case ERROR_EOF:
        the_pstackptr = p_stack_pointer;	
        SchemePushP(end_of_file_atom); 
	break;
      case ERROR_PARSE:
        the_pstackptr = p_stack_pointer;	
        SchemePushP(nil_constant); 
	break;
      default:
	SchemePanic("setjmp returns invalid value");
	break;
    }
  }
}

/*
 *
 * READ-Functions
 * 
 */

static p_readl()
{
  char  *token_text;
  Pair  *ppp;
  int   x;

  x=get_token(& token_text);
#ifdef NDEBUG
#else
  TokenTest(x,token_text);
#endif
  switch (x) {

    case SchemeTokenLPAR:  
      p_read_head();
      break;

    case SchemeTokenVECTOR:			/* ok */
      p_read_head();
      ppp = SchemePopP();
      SchemePushP(list_to_vector(ppp));
      break;

    case SchemeTokenSYMBOL:  
      make_symbol_or_number(token_text);
      break;

    case SchemeTokenINTEGER:
      make_integer (token_text);
      break;

    case SchemeTokenCONSTANT:
      make_constant(token_text);
      break;

    case SchemeTokenSTRING:
      make_stringt (token_text);
      break;

    case SchemeTokenCHAR:
      make_char (token_text);
      break;

    case SchemeTokenQUOTE: 
      SchemePushP(quote_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      break;

    case SchemeTokenQQUOTE: 
      SchemePushP(quasiquote_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      break;

    case SchemeTokenUNQUOTE: 
      SchemePushP(unquote_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      break;

    case SchemeTokenUNQUOTES: 
      SchemePushP(unquote_splicing_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      break;

    default:    
      werr ("error during read");
      SchemePushP(nil_constant);
      longjmp(error_env, ERROR_PARSE);    
      break;
  }
}

static p_read_head()
{
  char * token_text;
  Pair * ppp;
  int  x;

  x=get_token(& token_text);
#ifdef NDEBUG
#else
  TokenTest(x,token_text);
#endif
  switch (x) {

    case SchemeTokenSYMBOL: 
      make_symbol_or_number(token_text);
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenINTEGER:
      make_integer(token_text);
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenCHAR:
      make_char(token_text);
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenSTRING:
      make_stringt (token_text);
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenLPAR: 
      p_read_head();
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenVECTOR: 
      p_read_head();
      ppp = SchemePopP();
      SchemePushP(list_to_vector(ppp));
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenCONSTANT:
      make_constant(token_text);
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenDOT:  
      p_read_cdr();
      break;

    case SchemeTokenRPAR: 
      SchemePushP(nil_constant);     
      break;

    case SchemeTokenQUOTE:
      SchemePushP(quote_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenQQUOTE:
      SchemePushP(quasiquote_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenUNQUOTE:
      SchemePushP(unquote_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenUNQUOTES:
      SchemePushP(unquote_splicing_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      p_read_tail();
      ConsOnStack();
      break;

    default:   
      werr ("error during read / read_head");
      SchemePushP(nil_constant);    
      longjmp(error_env, ERROR_PARSE);    
      break;
  }
}

static p_read_tail()
{
  char * token_text;
  Pair * ppp;
  int  x;

  x=get_token(& token_text);
#ifdef NDEBUG
#else
  TokenTest(x,token_text);
#endif
  switch (x) {

    case SchemeTokenSYMBOL:  
      make_symbol_or_number(token_text);
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenSTRING:
      make_stringt (token_text);
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenINTEGER:  
      make_integer (token_text);
      p_read_tail();
      ConsOnStack();
      break;

   case SchemeTokenCHAR:
      make_char (token_text);
      p_read_tail();
      ConsOnStack();
      break;

   case SchemeTokenCONSTANT:
      make_constant(token_text);
      p_read_tail();
      ConsOnStack();
      break;

   case SchemeTokenLPAR:  
      p_read_head();
      p_read_tail();
      ConsOnStack();
      break;

   case SchemeTokenVECTOR:  
      p_read_head();
      ppp = SchemePopP();
      SchemePushP(list_to_vector(ppp));
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenDOT:   
      p_read_cdr();
      break;

    case SchemeTokenRPAR:  
      SchemePushP(nil_constant);    
      break;

    case SchemeTokenQUOTE: 
      SchemePushP(quote_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenQQUOTE: 
      SchemePushP(quasiquote_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenUNQUOTE: 
      SchemePushP(unquote_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      p_read_tail();
      ConsOnStack();
      break;

    case SchemeTokenUNQUOTES: 
      SchemePushP(unquote_splicing_atom);
      p_readl();
      SchemePushP(nil_constant);
      ConsOnStack();
      ConsOnStack();
      p_read_tail();
      ConsOnStack();
      break;

    default:    
      werr ("error during read / read_tail");
      SchemePushP(nil_constant);    
      longjmp(error_env, ERROR_PARSE);    
      break;
  }
}

static p_read_cdr()
{
  char * token_text;
  int  x;

  p_readl();

  x=get_token(& token_text);
#ifdef NDEBUG
#else
  TokenTest(x,token_text);
#endif
  switch (x) {

    case SchemeTokenRPAR:  
      break;

    default:    
      werr ("error during read / read_cdr");
      longjmp(error_env, ERROR_PARSE);    
      break;
  }
}

/*
 * print
 *
 */

SchemeDisplay()
{
  display_flag = 1;
  printout_flag = 0;
  p_print_it();
}

SchemePrint()		
{
  display_flag = 0;
  printout_flag = 0;
  p_print_it();
}

SchemeWrite()		/* wie Print nur ohne Newline ! */
{
  display_flag = 0;
  printout_flag = 1;
  p_print_it();
}

static p_print_it()
{
  last_printed=lpar_atom;
  p_prin0();
  if(display_flag == 0) {
    if(printout_flag == 0) {
      cop_prints("\n");
    }
  }
}

static p_prin0()
{
  switch((int) (SchemeInspectP())->tag) {
    case VECTOR_TYPE:
      /* SchemePushP(vector_to_list(SchemePopP())); */
      *(the_pstackptr-1) = vector_to_list(*(the_pstackptr-1));
      p_print_item(vector_atom);
      p_prin0();
      break;
    case PAIR_TYPE:
      p_print_item(lpar_atom);
      p_printbody();
      break;
    default:
      if(IS_SPECIALPAIR(SchemeInspectP())) {
        p_print_item(lpar_atom);
        p_printbody();
      } else {
        p_print_item(SchemeInspectP());
        SchemePopP();
      }
      break;
  }
}

static p_printbody()
{
  Pair *tmp;
  SchemePushP(fr1);

  tmp = *(the_pstackptr-2);			/* Tausch auf dem Stack ... */
  *(the_pstackptr-2) = *(the_pstackptr-1);
  *(the_pstackptr-1) = tmp;
  
  fr1 = SchemePopP();
  SchemePushP(fr1->car);

  p_prin0();

  if(fr1->cdr == nil_constant) {
    p_print_item(rpar_atom);
    fr1 = SchemePopP();
    return;
  }

  /*
   * (a . #(a b c)) NEU
   *
   */

  if (!IS_PAIR(fr1->cdr)) {
    p_print_item (dot_atom);
    if(fr1->cdr->tag == VECTOR_TYPE) {
      SchemePushP(fr1->cdr);
      *(the_pstackptr-1) = vector_to_list(*(the_pstackptr-1));
      p_print_item(vector_atom);
      p_prin0();
    } else {
      p_print_item (fr1->cdr);
    }
    p_print_item (rpar_atom); 
    fr1 = SchemePopP();
    return;
  }

  SchemePushP(fr1->cdr);
  p_printbody();
  fr1 =  SchemePopP();
}

/* 
 * Hier sind die Funktionen, die die Aufgabe haben den uebergebenen 
 * Text umzuwandeln
 */

make_double(d)
double d;
{
  Pair* i;
  i = AllocFloat();
  C_FLOAT(i) = d;
  SchemePushP(i);
}

static make_long(l) 
long l;
{
  Pair* i;
  i = AllocInteger();
  i->car = (Pair*) l;
  SchemePushP(i);
}

static make_symbol_or_number(text)
char * text;
{
  switch(StringToNumber(text,&the_long,&the_double,10,werr)) {
  case SchemeNONUMBER:
    SchemePushP((Pair*) GetCAtom(text));
    break;
  case SchemeLONG:
    make_long(the_long);
    break;
  case SchemeDOUBLE:
    make_double(the_double);
    break;
  default:
    SchemePanic("INTERNAL ERROR make_symbol_or_number");
    break;
  }
}

static make_constant(t)
char *t;
{
  if(t[1] == 't') {
    SchemePushP(true_constant);
  } else if (t[1] == 'f') {
    SchemePushP(false_constant);
  } else if (t[1] == 'u') {
    SchemePushP(unassigned_constant);
  } else {
    SchemePanic("invalid tokentext / make_constant");
  }
}


make_integer(text)
char * text;
{
  Pair* i;
  werr("make_integer called !!");
  i = AllocInteger();
  i->car = (Pair*) atoi(text);
  SchemePushP(i);
}

static make_char(text)
char * text;
{
  int char_value;
  if (!strcmp(text,"#\\space") || !strcmp(text,"#\\Space") || !strcmp(text,"#\\")) {
    char_value = ' ';
  } else if (!strcmp(text, "#\\newline")) {
    char_value = '\n';
  } else {
    if (strlen(text) != 3) {
      if (strlen(text) == 5) {
	if (text[2] >= '0' && text[2] <= '9' &&
	    text[3] >= '0' && text[3] <= '9' &&
	    text[4] >= '0' && text[4] <= '9') {
	  char_value = atoi(text+2);
	  if (char_value < 0 || char_value > 255) {
	    werr("invalid decimal value after \\ in char / read");
	    char_value = ' ';
	  }
	} else {
          werr("invalid character constant / read");
          char_value = ' ';
	}
      } else {
        werr("invalid character constant / read");
        char_value = ' ';
      }
    } else {
      char_value = text[2];
    }
  }
  SchemePushP(S_CHAR(char_value));
}

static make_stringt (text)
char* text;
{
  int i;
  unsigned char *p;
  SchemePushP(fr1);
  SchemePushP(fr1);
  fr1 = make_string(token_length, ' ');
  p = (unsigned char*) C_STRING(fr1);
  for (i=0; i<token_length; i++)
    *p++ = text[i];
  *(the_pstackptr-2) = fr1; 
  fr1 = SchemePopP();
}

static print_pname(atom)
Atom* atom;
{
  int i;
  int l;
  unsigned char* s;
  l = C_STRINGLEN(atom->pname);
  s = C_STRING(atom->pname);
  for (i=0; i<l; i++)
    writechar(the_current_output_port,s[i]);
}

static print_pname_e(atom)
Atom* atom;
{
  int i;
  int l;
  unsigned char* s;
  l = C_STRINGLEN(atom->pname);
  s = C_STRING(atom->pname);
  for (i=0; i<l; i++)
    writechar(the_current_error_port,s[i]);
}

static p_print_item (atom)
Pair* atom;
{
  if (last_printed != lpar_atom &&
      last_printed != vector_atom &&
      atom != rpar_atom)
    writechar(the_current_output_port,' ');

  switch ((int) (atom->tag)) {
  case USER_TYPE:
    cop_prints("#<");
    print_pname(atom->car);
    cop_prints(">");
    break;
  case EXTERNAL_TYPE:
    if ((((External*) atom)->functions) == NULL) {
      cop_prints("#<EXTERNAL_TYPE with no functions-pointer>");
    } else {
      if ((((External*) atom)->functions->print) == NULL) {
	cop_prints("#<EXTERNAL_TYPE with no print-function>");
      } else {
	(*(((External*) atom)->functions->print)) (the_current_output_port,(External*) atom);
      }
    }
    break;
  case SYMBOL_TYPE:
    print_pname(atom);
    break;
  case CONSTANT_TYPE:
    if (atom == nil_constant) {
      cop_prints("()");
    } else if (atom == true_constant) {
      cop_prints("#t");
    } else if (atom == false_constant) {
      cop_prints("#f");
    } else if (atom == unassigned_constant) {
      cop_prints("#u");
    } else {
      SchemePanic("unknown constant / p_print_item");
    }
    break;
  case INTEGER_TYPE:
    print_integer(atom);
    break;
  case FLOAT_TYPE:
    print_float(atom);
    break;
  case PRIMITIVE_TYPE:
    print_primitive(atom);
    break;
  case COMPOUND_TYPE:
    cop_prints("#<compound-procedure");
    if (((Compound*) atom)->name != nil_constant) {
      cop_prints(" ");
      print_pname(((Compound*) atom)->name); 
    }
    cop_prints(">");
    break;
  case SYNTAXCOMPOUND_TYPE:
    cop_prints("#<syntax-procedure");
    if (((SyntaxCompound*) atom)->name != nil_constant) {
      cop_prints(" ");
      print_pname(((Compound*) atom)->name); 
    }
    cop_prints(">");
    break;
  case CONTINUATION_TYPE:
    cop_prints("#<continuation>");
    break;
  case MCONTINUATION_TYPE:
    cop_prints("#<mcontinuation>");
    break;
  case ENVIRONMENT_TYPE:
    cop_prints("#<environment>");
    break;
  case CHAR_TYPE:
    print_char(atom);
    break;
  case STRING_TYPE:
    print_string(atom);
    break;
  case INPUTPORT_TYPE:
  case POPENR_TYPE:
  case INPUTSTRINGPORT_TYPE:
  case STATICPORT_TYPE:
    cop_prints("#<input-port>");
    break;
  case OUTPUTPORT_TYPE:
  case POPENW_TYPE:
  case OUTPUTSTRINGPORT_TYPE:
    cop_prints("#<output-port>");
    break;
  default:
    cop_prints("#<unknown>");
    break;
    /*
     * fprintf(stderr,"tag=%d\n",((int) (atom->tag))); 
     * fflush(stderr);
     * SchemePanic("invalid atom-type / p_print_item");
     * break;
     */
  }
  last_printed=atom;
}

static print_integer(Pair* atom)
{
  char buf[32];
  sprintf(buf, "%ld",C_LONG(atom)); 
  cop_prints(buf);
}

static print_float(Pair* atom)
{
  char buf[64];
  sprintf(buf, "%g", C_FLOAT(atom));
  cop_prints(buf);
  if (strchr(buf,'.') == NULL)
    cop_prints(".0");
}

static print_primitive(Pair* atom)
{
  int old_display_flag = display_flag;
  cop_prints("#<primitive-procedure ");
  display_flag=1;
  /*
   * print_string(atom->cdr);
   */
  print_pname(((Primitive*) atom)->symbol);
  display_flag = old_display_flag;
  cop_prints(">");
}

static print_char(Pair* atom)
{
  int x;
  char buf[8];
  x = C_CHAR(atom);

  if (display_flag) {
    writechar(the_current_output_port,x);
  } else {
    cop_prints("#\\");
    if (x == ' ') {
      cop_prints("space");
    } else if (x == '\n') {
      cop_prints("newline");
    } else if (x >= 32 && x < 127) {
      cop_printc(x);
    } else {
      sprintf(buf,"%03d",x);
      cop_prints(buf);
    }
  }
}

static print_string(Pair *atom)
{
  unsigned char *p;
  int  i,l;
  char buf[8];

  p = (unsigned char*) C_STRING(atom);
  l = C_STRINGLEN(atom);

  if(!display_flag)
    cop_prints("\"");
  for (i = 0; i<l; i++) {
    if (display_flag) {
      cop_printc(p[i]);
    } else {
      if (p[i] < 32 || p[i] >= 127) {
        cop_printc('\\');
        sprintf(buf,"%03d",p[i]);
	cop_prints(buf);
      } else {
        switch (p[i]) {
        case '\"':
	  cop_printc('\\');
	  cop_printc('\"');
	  break;
        case '\\':
	  cop_printc('\\');
	  cop_printc('\\');
	  break;
        default:
	  cop_printc(p[i]);
	  break;
        }
      }
    }
  }
  if(!display_flag)
    cop_prints("\"");
}

/*
 * zentrale Scannerfunktion
 *  get_c und unget_c stehen zur Verfuegung.
 *
 */

static char token_text [SchemeMAXTOKENLEN];
static char *tp;

static int get_token(char **v)
{
  int token;
  int c,cc;

  token_length=0; 		// 
  tp = token_text;

  /*
   * skip white-space
   */

  again:

  for (;;) {
    c = get_c();
    if (c != ' ' && c != '	' && c != '\n')
      break;
  }

  /*
   *  skip comment ...
   * 
   */

  if (c == ';') {
    for(;;) {
      c = get_c();
      if (c == '\n' || c == EOF)
	break;
    }
    goto again;
  }

  switch(c) {

    case '`':
      return SchemeTokenQQUOTE;
    case ',':
      cc = get_c();
      if (cc == '@') {
	return SchemeTokenUNQUOTES;
      } else {
	unget_c((char) cc);
        return SchemeTokenUNQUOTE;
      }

    case '(':
      return SchemeTokenLPAR;

    case ')':
      return SchemeTokenRPAR;

    case '\'':
      return SchemeTokenQUOTE;

    case '#':			/* char, true, false, vector */
      unget_c((char) c);
      return(handle_hash(v));

    case '\"':
      unget_c((char) c);
      return(handle_string(v));

   case EOF:
      longjmp(error_env, ERROR_EOF);      
      break;

   default:
      /*
       * nur ein einzelner Punkt liefert SchemeTokenDOT
       * so ist der 'peculiar identifier' ... moeglich ...
       */
      if(c == '.') {
	int d;
	d = get_c();
	/*
	 * Die bash expandiert evtl. bei #! zu ./, also
	 * muss auch bei / ein symbol generiert werden !
	 */
	if (d != '.' && d != '/' && !(d >= '0' && d <= '9')) {
	  unget_c((char) d);
	  return SchemeTokenDOT;
	} else {
	  unget_c((char) d);
	}
      }
      *tp++ = c;
      token_length++;

      for (;;) {

        if (token_length >= SchemeMAXTOKENLEN) {
          werr("maximal length of token exceeded");
          break;
        }

	c = get_c();
	/*
         * delimiters 
	 */
	if (c == ' ' || c == '	' || c == '\n' || c == ')' || c == ',' ||
	    c == '(' || c == ';' || c == '\"' || c == EOF) {
	  unget_c((char) c);
	  break;
	} else {
	  *tp++ = c;
          token_length++;
        }
      }
      *tp = '\000';
      token = SchemeTokenSYMBOL;
      break;
  }
  *v = token_text;
  return(token);    
}


static handle_hash(char **v)
{
  int c;
  *tp++ = get_c();		/* hash in buffer */

  switch (c = get_c()) {
  case 'F':
  case 'T':
    c = c - 'A' + 'a';
    goto clabel;
  case 'f':
  case 't':
  case 'u':	/* unassigned_constant */
clabel:
    *tp++ = c;
    *tp = '\000';
    *v = token_text;
    return(SchemeTokenCONSTANT);
  case '(':
    return(SchemeTokenVECTOR);

  case '\\':
      *tp++ = c;
      for (;;) {
	c = get_c();
	if (c == ' ' || c == '	' || c == '\n' || c == ')' || c == EOF) {
	  unget_c((char) c);
	  break;
	} else
	  *tp++ = c;
      }
      *tp = '\000';
      *v = token_text;
      return (SchemeTokenCHAR);
  default:
      *tp++ = c;
      for (;;) {
	c = get_c();
	if (c == ' ' || c == '	' || c == '\n' || c == ')' || c == EOF) {
	  unget_c((char) c);
	  break;
	} else
	  *tp++ = c;
      }
      *tp = '\000';
      *v = token_text;
      return (SchemeTokenSYMBOL);
  }
}

static handle_string(char **v)
{
  int c,d,e,f,g;
  char vbuf[4];

  token_length = 0;
  tp = token_text;

  c = get_c();		/* erstes dquote weg ... */
  for (;;) {
    if (token_length >= SchemeMAXTOKENLEN) {
      werr("maximal length of token exceeded");
      break;
    }
    c = get_c();
    switch(c) {
    case '\"':
      break;

    case '\\':
      d = get_c();
      switch(d) {
      case '\"':
        *tp++ = d;
	token_length++;
	break;
      case '\\':
	*tp++ = d;
	token_length++;
	break;
      default:
	if (d >= '0' && d <= '9') {
	  e = get_c();
	  f = get_c();
	  if ((e >= '0' && e <= '9') && (f >= '0' && f <= '9')) {
	    vbuf[0] =d; vbuf[1] = e; vbuf[2] = f; vbuf[3] = '\000';
	    g = atoi(vbuf);
	    if (g > 255) {
	      werr("char in sting exceeds 255 (decimal)");
	    }
	    *tp++ = atoi(vbuf);
	    token_length++;
	  } else {
	    werr ("invalid backslash-notation in string");
	  }
	} else {
	  werr ("invalid backslash-notation in string");
	  *tp++ = d;
	  token_length++;
	}
	break;
      }
      break;
    case EOF:
      werr("unexpected EOF in string");
      break;
    default:
      *tp++ = c;
      token_length++;
    }
    if (c == '\"' || c == EOF)
      break;
  }
  *v = token_text;
  return(SchemeTokenSTRING);
}

get_c()
{
  return(readchar(the_current_input_port));
}

unget_c(char c)
{
  ungetchar(the_current_input_port,c);
}

werr(char* s)
{
  /*
  printf("%s\n",s);
  */
  global_errorflag = 1;
  errormessage = AllocString(strlen(s));
  strcpy((char*) C_STRING(errormessage),(char*) s);
  errorirritant = nil_constant;
}

werri(char* s, Pair* irritant)
{
  global_errorflag = 1;
  errorirritant = irritant;
  errormessage = AllocString(strlen(s));
  strcpy((char*) C_STRING(errormessage),(char*) s);
}

/*
 * Funktionen fuer Vektoren, Strings usw...
 *  
 */

Pair* make_vector(int len, Pair* fill)
{
  int i;
  Pair* rpt;
  Pair** vec;

  SchemePushP(fill);
  rpt = AllocVector(len);
  fill = SchemePopP();

  vec = C_VECTOR(rpt);

  for (i=0; i<len; i++)
    vec[i] = fill;

  return(rpt);
}

Pair* make_string(int len, unsigned char fillchar)
{
  Pair* rpair;
  unsigned char* ptr;
  int i;

  rpair = AllocString(len);
  ptr = C_STRING(rpair);
  for (i=0; i<len; i++)
    ptr[i] = fillchar;

  return(rpair);
}

int list_length(Pair* l)
{
  int n = 0;
  for (;;) {
    if (IS_NIL(l))
      return(n);
    else {
      if (IS_PAIR(l)) {
        n++;
	l = l->cdr;
        if (n > SchemeLISTLIMIT) {
	  werr("aborted / circular structure assumed");
	  return(n);
        }
      } else {
	werr("invalid list / list_length");
	return(n);
      }
    }
  }
}

Pair* vector_to_list(Pair* vector)
{
  int vl,i,pi;
  Pair* rpair;

  SchemePushP(fr1);
  SchemePushP(fr2);
  SchemePushP(fr3);

  vl = C_VECTORLEN(vector);
  fr1 = nil_constant;
  fr3 = vector;

  if (vl != 0) {
    for (i=vl-1; i>=0; i--) {
      SetPairIn(fr2);
      fr2->car = (C_VECTOR(fr3))[i]; 
      fr2->cdr = fr1;
      fr1 = fr2;
    }
  }
  rpair = fr1;
  fr3 = SchemePopP();
  fr2 = SchemePopP();
  fr1 = SchemePopP();
  return(rpair);
}

Pair* list_to_vector(Pair* list)
{
  int laenge;
  int i;
  Pair** vector;
  Pair*  rvec;
  int index;

  SchemePushP(fr1);
  fr1 = list;	/* protect list */
  laenge = list_length(list);
  rvec = AllocVector(laenge);
  vector = C_VECTOR(rvec);
  list = fr1;
  fr1 = SchemePopP();

  for (i=0; i<laenge; i++) {
    vector[i] = list->car;
    list = list->cdr;
  }
  return(rvec);
}

Pair* string_to_list(Pair* string)
{
  int sl,i;
  Pair* rpair;
  unsigned char* sp;

  SchemePushP(fr1);
  SchemePushP(fr2);
  SchemePushP(fr3);

  sl = C_STRINGLEN(string);        
  fr1 = nil_constant;	/* fr1 enthaelt immer die Rueckabeliste */
  fr3 = string; 

  if (sl > 0) {
    for (i=sl-1; i>=0; i--) {
      SetPairIn(fr2);
      fr2->car = S_CHAR(C_STRING(fr3)[i]);
      fr2->cdr = fr1;
      fr1 = fr2;
    }
  }

  rpair = fr1;
  fr3 = SchemePopP();
  fr2 = SchemePopP();
  fr1 = SchemePopP();
  return(rpair);
}

Pair* list_to_string(Pair* list)
{
  int laenge;
  int i;
  Pair* rstr;
  unsigned char* sp;

  SchemePushP(fr1);
  fr1 = list;	/* protect list */
  laenge = list_length(list);
  rstr = make_string(laenge,' ');
  list = fr1;
  fr1 = SchemePopP();

  sp = (unsigned char*) C_STRING(rstr);

  for (i=0; i<laenge; i++) {
    if (!IS_CHAR(list->car)) {
      werr("only chars supported / list->string");
      break;
    }
    sp[i] = C_CHAR(list->car);
    list = list->cdr;
  }
  return(rstr);
}

