#ifndef _MACHINE_H
#define _MACHINE_H


#define accepting 0
#define literal 1
#define alternation 2
#define concatenation 3
#define kleenclosure 4

#define accept 256
#define E 31
#define e 30

//some preProcessor constants for common metaCharacters
#define PLUS 43
#define ANYCHAR 63
#define ZERO_OR_MORE 42
#define CLOSE_PAREN 41
#define OPEN_PAREN 40
#define START_CHAR_CLASS 91
#define END_CHAR_CLASS   93
#define OR 124
#define AND 46
#define ESCAPE 35
#define METASIZE 8


typedef struct State State;

 struct State {
	int c;		//character for this state
	 State *nextState;
	 State *nxtState;	// for alternation
};

//Data structures

typedef struct list List;
typedef struct set Set;
typedef struct listc Listc;
typedef struct setc Setc;
typedef union data Data;
typedef struct lists Lists;
typedef struct sets Sets;

typedef struct transition Transition;
typedef struct transTable TransTable;
typedef struct dfa DFA;

typedef struct queueC QueueC;

/*
#define add(x,y) _Generic((x), Set *:add2, State*:add1)(x,y)
#define exists(x,y) _Generic((x), Set * :exists2, State*:exists1)(x,y)
#define Union(x,y) _Generic((x), Sets *:Union2, Set:Union1)(x,y)
*/
#define init(type,var) var = malloc(sizeof(type)); var->elements=NULL; var->size=0


union data {
 State *state;
 Set *set;
};

//list of states
struct list {
   State *state;
   List *next;
};

//A set of states
struct set {
   List *elements;
   int size;
};


//List of characters
struct listc {
  char c;
  Listc *next;
};

//A set of characters for the NFA
struct setc {
  Listc *elements;
  int size;
};


//list of set sets
struct lists {
   Set *set;
   Lists *next;
};

//A set of sets
struct sets {
   Lists *elements;
   int size;
};



/*Section for DFA STates */
// DFA Data structures
struct transition{
	Set *q;
  int c;
  Set *t;
  Transition *next;
};

struct transTable {
  Transition *T;
  int size;
};

struct dfa {
  Sets *D;
  Setc *C;
  TransTable *table;
  Set *start;
  Sets *A;
};


//Data structure for Regex PreProcessing

//determine if a character is a meta character
char metaChar[METASIZE] = {'*','?','|','+','.','(',')','#'};

struct queueC {
  Listc *head;
  Listc *tail;
  int size;
};

QueueC *enqueueC(char,QueueC *);
char dequeueC(QueueC *);
char *trimSpace(char *);
int isMetaChar(char);
int isCharacterClass(char);
char decode(char);
char *PreRegex(char *);


#endif
