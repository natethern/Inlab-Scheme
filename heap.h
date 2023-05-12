/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: heap.h,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

extern Pair** TheHeapA;

/*
 * Analog zu AllocHeap in heap.c ...
 * Nach GiveMeWords muss der FreePointer um die Anzahl der angeforderten
 * Worte hochgezaehlt werdden...
 */

#define GiveMeWords(n) \
  ((Pair*) (((FreePointer + (n)) >= EndPointer) ? \
   (garbage_collect(), \
     ((FreePointer + (n) >= EndPointer ? \
      ((Pair**) SchemePanic("Out of Heap")) : FreePointer))) : \
      FreePointer))

#define SetPairIn(lv) \
  { \
    if (FreePointer + (sizeof(Pair)/sizeof(SchemeWord)) >= EndPointer) {\
      garbage_collect();\
      if (FreePointer + (sizeof(Pair)/sizeof(SchemeWord)) >= EndPointer) {\
	PanicWithOutOfHeap();\
      } \
    } \
    (lv) = (void*) FreePointer;\
    ((Pair*) FreePointer)->tag = PAIR_TYPE;\
    FreePointer += (sizeof(Pair)/sizeof(SchemeWord));\
  }

#define SetIntegerIn(lv) \
  { \
    if (FreePointer + (sizeof(Integer)/sizeof(SchemeWord)) >= EndPointer) {\
      garbage_collect();\
      if (FreePointer + (sizeof(Integer)/sizeof(SchemeWord)) >= EndPointer) {\
	PanicWithOutOfHeap();\
      } \
    } \
    (lv) = (void*) FreePointer;\
    ((Integer*) FreePointer)->tag = INTEGER_TYPE;\
    FreePointer += (sizeof(Integer)/sizeof(SchemeWord));\
  }

#define MakeInteger(lv,value) \
{ \
  if(value < STATIC_INTEGER_START || \
     value > (STATIC_INTEGER_START+STATIC_INTEGER_SIZE-1)) {\
    SetIntegerIn(lv);\
    ((Pair*) lv)->car = (Pair*) ((SchemeWord) (value));\
  } else {\
    (lv) = (void*) &StaticIntArray[(value)-(STATIC_INTEGER_START)]; \
  }\
}

#define SetEnvironmentIn(lv) \
  { \
    if (FreePointer + (sizeof(Environment)/sizeof(SchemeWord)) >= EndPointer) {\
      garbage_collect();\
      if (FreePointer + (sizeof(Environment)/sizeof(SchemeWord)) >= EndPointer) {\
	PanicWithOutOfHeap();\
      } \
    } \
    (lv) = (void*) FreePointer;\
    ((Pair*) FreePointer)->tag = ENVIRONMENT_TYPE;\
    FreePointer += (sizeof(Environment)/sizeof(SchemeWord));\
  }

#define SetSpecialPairIn(lv,newtag) \
  { \
    if (FreePointer + (sizeof(Pair)/sizeof(SchemeWord)) >= EndPointer) {\
      garbage_collect();\
      if (FreePointer + (sizeof(Pair)/sizeof(SchemeWord)) >= EndPointer) {\
	PanicWithOutOfHeap();\
      } \
    } \
    (lv) = (void*) FreePointer;\
    ((Pair*) FreePointer)->tag = (newtag);\
    FreePointer += (sizeof(Pair)/sizeof(SchemeWord));\
  }

