/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald 
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: fdecl.h,v 4.102 2006/04/29 09:13:22 tommy Exp $
 * 
 */

static char* rcsid_fdecl_h = "$Id: fdecl.h,v 4.102 2006/04/29 09:13:22 tommy Exp $";

extern Pair* SchemeGetInternalContinuation();
extern Atom* SearchAtom();
extern Port* createoutputstringport();
extern Port* createinputstringport();
extern Port* createstaticport();
extern Port* CreateStaticPortByName();
extern Port* openoutputfile();
extern Port* openoutputfileappend();
extern Port* openinputfile();
extern Port* openpopenread();
extern Port* openpopenwrite();
extern Port* openinitfile();
extern Pair* AllocExternalType();
extern Pair* AllocXCtype();
extern Pair* AllocCtype();
extern Atom* AllocAtom();
extern Atom* CreateAtom();
extern Atom* CreateCAtom();
extern Atom* GetAtom();
extern Atom* GetCAtom();
extern Pair* AllocCompound();
extern Pair* AllocSyntaxCompound();
extern Pair* AllocFloat();
extern Pair* AllocInputPort();
extern Pair* AllocStaticPort();
extern Pair* AllocPopenwPort();
extern Pair* AllocPopenrPort();
extern Pair* AllocInputStringPort();
extern Pair* AllocOutputStringPort();
extern Pair* AllocInteger();
extern Pair* AllocOutputPort();
extern Pair* AllocPair();
extern Pair* AllocEnvironment();
extern Pair* AllocPrimitive();
extern Pair* AllocString();
extern Pair* AllocVector();
extern Pair* AllocContinuation();
extern Pair* binding_in_env();
extern Pair* binding_in_frame();
extern Pair* define_vaiable();
extern Pair* list_to_string();
extern Pair* list_to_vector();
extern Pair* make_string(int, unsigned char);
extern Pair* make_vector();
extern Pair* string_to_list();
extern Pair* vector_to_list();
extern Pair** AllocHeap();
extern Port* createstderr();
extern Port* createstdin();
extern Port* createstdout();
extern char* getenv();
extern char* readuntilchar();
