//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "machine.h"


State *createState(char *c,int type) {

	State *s, *ns;
	switch(type) {

		case accepting:
			s = (State*)malloc(sizeof(State));
			s->c = accept;
			s->nextState = NULL;
			break;
		case literal:
			s = (State*)malloc(sizeof(State));
			ns = (State*)malloc(sizeof(State));
			s->c = decode(*c); //printf("Decoded :%c\n",decode(*c));
			ns->c = accept;
			ns->nextState = NULL;
			s->nextState = ns;
			break;
		default:
			s = NULL;
			break;

	}
	return s;
}

State *concat(State *s1, State *s2) {
	State *st = s1;
	
	while(st->c != accept)
	  st = st->nextState;
	st->c = e;
	st->nextState = s2;
	return s1;
}

State *alternate(State *s1, State *s2) {
	State *s = (State*)malloc(sizeof(State));
	State *acceptState = createState(NULL,accepting);
	State *st = s1;
	State *tmp, *tmp2;

	while(st->c != accept) { tmp2 = st; st = st->nextState;}
	tmp = st;
	tmp->nextState = acceptState;
  tmp->c = e;

	st = s2;
	while(st->c != accept) { tmp2 = st; st = st->nextState;}
	tmp = st;
	tmp->nextState = acceptState;
  tmp->c = e;

	s->c = E;
	s->nextState = s1;
	s->nxtState = s2;
	return s;
}



char *
re2postfix(char *re) {
  // variables to store the number of atoms (or literals) and alternations
  int natom, nalt;
  char *dst;	// destination pointer
//re = preRegex(re);
  // structure to store the values of nalt and natom at some point in the program
  typedef struct parenthesis {
  int nalt;	// number of alternations so far
  int natom;	// number of literals (or atoms) so far
 } parenthesis;

//character buffer of size 8000 to hold the postfix regular expression
 static char buf[8000];
  // list of parenthesis encountered so far. it's size (for now) is 100
  parenthesis paren[100];
  parenthesis *p;	// pointer to paren

  // initialize
  dst = buf;	// dst points to the begining of buf
  p = paren;	// p points to the beginning of paren
  natom = nalt = 0;

  printf("Converting Infix Regular Expression to Postfix...\n");

//check for size compatibility
  if(strlen(re) + 1 > sizeof buf/2) return NULL;
  // loop through the regex re and handle each character as a separate cases
  for(; *re; re++)
  {
	switch(*re) {

	default:	// when the character is a literal or atom
	  if(natom > 1)
	  {
		--natom;
		*dst++ = '.';
	  }			
	  		//printf("Default: %d\n",*re);
	  *dst++ = *re; 	// store literal into buf
	  ++natom;	// increment the number of atoms
	  break;	// we are done with literals

	case '(':
	  //check to see if list of parenthesis is full
	  if(p >= paren + 100)
	     return NULL;
	  if(natom > 1) {
	      --natom;
	      *dst++ = '.';
	  }
	//save current values of natom and nalt
	p->nalt = nalt;
	p->natom = natom;
	//move list pointer to point to next
	++p;
	// now that we have saved the current state of natom and nalt,
	// reset them to zero
	nalt = natom = 0;
	break;	// that's all

	case ')':
	  // check to see if there are unmatching number of brackets OR empty brackets
	  if(p == paren || natom == 0 )
	     return NULL;
	  //concatenate all literals within brackets
	  while(--natom > 0) *dst++ = '.';
	 // alternate all literals within brackets
	  for(; nalt>0; nalt--) *dst++ = '|';
	// move to previous state of parenthesis
	--p;
	// retore previous state of nalt and natom
	nalt = p->nalt;
	natom = p->natom;
	natom++; // the values within the bracket now form a single atom or literal, and as such we increase the number of atoms
	break;	// that's it

	case '|':
	// check to see if it is not an empty alternation
	if(natom == 0) return NULL;
	//concatenate all literals at the left of |
	while(--natom > 0) *dst++ = '.';
	// finally increase the number of alternation
	++nalt;
	break;	// it's that simple

	case '*':	//zero or more (kleen closure)
	case '+': 	// one or more (positive closure)
	case '?':	//zero or one
	  //check to see if the left argument is empty
	  if(natom == 0) return NULL;
	  //add it to buffer
	  *dst++ = *re;
	  break;

	

     }
   }
  // well, it could happen that the parenthesis still don't match!
  if(p != paren) return NULL;
  // again concatenate and alternate all atoms
  while(--natom > 0) *dst++ = '.';
  for(; nalt>0; nalt--) *dst++ = '|';
  *dst = 0;	// pad buffer with the null string to indicate end of string

printf("Done\n\n");
  return buf;
}


//Kleen closure
State *kclosure(State *s) {
 //creat two states, one for start and the other accept
   State *start, *accepts;
   start = malloc(sizeof(State));
   accepts = createState(NULL, accepting);
   //find the accepting state of s;
  State *tmp = s;
  while(tmp->c != accept)
    tmp = tmp->nextState;
  //point accepting stateto new accepts state
  tmp->c = E;
  tmp->nextState = accepts;
  tmp->nxtState = s;

  start->c = E;
  start->nextState = s;
  start->nxtState = accepts;

 return start;
}

//Thompson's construction
State *rex2nfa(char *rex) {
	int i=-1;	// string position pointer
	int n = strlen(rex);	//length of string
	//the stack
	State *stack[256];
	int stackptr = -1;
	#define push(s) stack[++stackptr] = s
	#define pop() stack[stackptr--]

 	State *s1, *s2;

 	printf("Converting Postfix Regular Expression to NFA...\n");

	//read character one at a time in sequence
	while(++i < n) {
 		switch(rex[i]) {

		   case '|':
			s1 = pop();
			s2 = pop();
			push(alternate(s2,s1));
			break;

		   case '.':
			s1 = pop();
			s2 = pop();
			push(concat(s2,s1));
		 	break;

		   case '*':
			s1 = pop();
		  	push(kclosure(s1));
			break;

		   default:
			push(createState(&rex[i],literal)); //printf("Def: %d\n",rex[i]);
			break;
		}
	}

	printf("Done\n\n");
	if(stackptr == 0) return pop();

	return NULL;

}



int exists2(Set *set, Sets *U) {
	if(!set || !U) return 0;
  Lists *Elements = U->elements;
  List *elements = set->elements;
  Set *u; List *uelements;
  List *tmp;
  while(Elements) {
    if(equal(set,Elements->set))
		  return 1;
		Elements = Elements->next;
 }
  return 0;
}


int existsc(char c, Setc *set) {
   Listc *tmp = set->elements;
   while(tmp) {
      if(c == tmp->c)
	return 1;
      tmp =tmp->next;
   }
  return 0;
}

int exists(State *q, Set *set) {
   List *tmp = set->elements;
   while(tmp) {
      if(  q == tmp->state)
	return 1;
      tmp =tmp->next;
   }
  return 0;
}

//check for equality of two sets
int equal(Set *s1, Set *s2) {
	if(!s1 && !s2) {  return 1;}
	if(!s1 || !s2) {  return 0;}
	if(s1->size < s2->size || s1->size > s2->size)
	   {  return 0;}
	else {
		List *tmp;
		tmp = s1->elements;
		while(tmp) {
			if(!exists(tmp->state,s2))
			  {    return 0;}
			tmp = tmp->next;
		}
		return 1;
	}
}


void initSet(Set **set) {
 *set = malloc(sizeof(Set));
 (*set)->size = 0;
  (*set)->elements = NULL;
 }

Set *add(State *q, Set *set) {
  if(!exists(q,set)){
    List *elem = malloc(sizeof(List));
    elem->state = q;
    elem->next = set->elements;
    set->elements = elem;
    set->size += 1;
    return set;
  }
  return set;
}



Sets *add2(Set *q, Sets *sets) {
  if(!exists2(q,sets) && q->size > 0){
    Lists *elem = malloc(sizeof(Lists));
    elem->set = q;
    elem->next = sets->elements;
    sets->elements = elem;
    sets->size += 1;
    return sets;
  }
  return sets;
}

Setc *addc(char c, Setc *set) {
  if(!existsc(c,set)){
    Listc *elem = malloc(sizeof(Listc));
    elem->c = c;
    elem->next = set->elements;
    set->elements = elem;
    set->size += 1;
    return set;
  }
  return set;
}

Set *Union(Set *set1, Set *set2) {
  if(!set1 && !set2) return NULL;
 if(!set1) return set2;
 if(!set2) return set1;
 List *tmp = set2->elements;
 while(tmp) {
    if(!exists(tmp->state, set1))
	set1 = add(tmp->state,set1);
    tmp = tmp->next;
 }
 return set1;
}


Sets *Union2(Sets *sets1, Sets *sets2) {
  if(!sets1 && !sets2) return NULL;
 if(!sets1) return sets2;
 if(!sets2) return sets1;
 Lists *tmp = sets2->elements;
 while(tmp) {
    if(!exists2(tmp->set, sets1))
	sets1 = add2(tmp->set,sets1);
    tmp = tmp->next;
 }
 return sets1;
}

Setc *Unionc(Setc *set1, Setc *set2) {
  if(!set1 && !set2) return NULL;
 if(!set1) return set2;
 if(!set2) return set1;
 Listc *tmp = set2->elements;
 while(tmp) {
    if(!existsc(tmp->c, set1))
	set1 = addc(tmp->c,set1);
    tmp = tmp->next;
 }
 return set1;
}

//transition functions
Set *deta(State *q, char c,Set *Ulist) {
  if(q->c == E) { 
    Set *s1, *s2;
    if(c == e) {
      initSet(&s1);
      s1 = add(q,s1);
      s1 = add(q->nextState,s1);
      s1 = add(q->nxtState,s1);
      return s1;
    }
		if(!exists(q,Ulist)) { 
		  Ulist = add(q,Ulist);
    	s1 = deta(q->nextState, c,Ulist);
    	s2 = deta(q->nxtState,c,Ulist);
			s1 = Union(s1,s2);
    	return s1;
		}
  }
  else if(q->c == e) { 
    if(c == e) {
      Set *s1;
      initSet(&s1);
      s1 = add(q,s1);
      s1 = add(q->nextState,s1);
      return s1;
    }
		if(!exists(q,Ulist)){ 
			Ulist = add(q,Ulist);
    	return deta(q->nextState,c,Ulist);
		}
		//return Ulist;
  }
  else {
    Set *s = NULL;
    if(q->c == c) {
	  	s = malloc(sizeof(Set));
   		Ulist = add(q,Ulist);
			s = add(q->nextState,s);
	  	return s;
   	}
  else if(c == e) {
 	     initSet(&s);
 	     s = add(q,s);
 	     return s;
  }

     return s;
   }
	 return NULL;
}


Set *delta(State *q,char c) {
	Set *Ulist;
	init(Set, Ulist);
	return deta(q,c,Ulist);
}

Set *Delta(Set *set,char c) {
  Set *workList;
  List *elements;
  init(Sets,workList);
  elements = set->elements;
  while(elements) {
    workList = Union(workList,delta(elements->state,c));
    elements = elements->next;
  }
  return workList;
}

// Closure Functions
Set *e_close(State *s) {
  Set *s1, *s2, *s3, *workList;

 s1 = delta(s,e);
 List *elements = (s1 == NULL)?NULL:s1->elements;
 s3 = NULL;
 initSet(&workList);
 while(elements) {
  if(!exists(elements->state,workList)) {
    add(elements->state,workList);
    s2 = delta(elements->state,e);
    s3 = Union(s3,s2);
  }
    elements = elements->next;
    if(!elements){
      (s3==NULL)?elements:(elements = s3->elements);
       s3 = NULL;
     }

 }
 return workList;
}

Set *E_close(Set *set) {
  List *elements = set->elements;
  Set *workList; init(Set,workList);
  while(elements) {
    workList = Union(workList,e_close(elements->state));
    elements = elements->next;
  }
  return workList;
}


Setc *getAlphabets(char *regex) {
	printf("Computing the Set of Characters for this NFA...\n");
	regex = trimSpace(regex);
  Setc *Chars; init(Setc,Chars); 
  int n = strlen(regex);
  int i = 0;
 
  while(i<n) {
    if(isMetaChar(regex[i]))
	{ 
		if(regex[i] == ESCAPE) Chars = addc(regex[++i],Chars); 
		else if(regex[i] == ANYCHAR) { 
			char k = '!'; 
    	 	char j = '~';
    	 	while(k<=j) Chars = addc(k++,Chars);
		}
		++i;
	}
    else {
    	 if(regex[i] == '[' && isCharacterClass(regex[i+1]) && regex[i+2] == '-' && isCharacterClass(regex[i+3]) && regex[i+4] == ']')
    	 {
    	 	char k = regex[i+1]; 
    	 	char j = regex[i+3];
    	 	if(j<k) { k=j; j=regex[i+1];}
    	 	while(k<=j) Chars = addc(k++,Chars);
    	 	i += 5;
    	 }
    	else Chars = addc(regex[i++],Chars);
    }
   
  }

  printf("Done\n\n");
  return Chars;
}


//DFA implementation
TransTable *T(Set *q, char c, Set *t, TransTable *table) {
	if(t == NULL || t->size <= 0) return table;
	Transition *trans = malloc(sizeof(Transition));
	trans->q = q; trans->c = c; trans->t=t;
	trans->next=table->T;
	table->T = trans;
	table->size +=1;
	return table;
}

Set *removeNextSet(Sets *workList) {
	if(workList == NULL || workList->size == 0) return NULL;
  Set *s =  workList->elements->set;
  workList->elements = workList->elements->next;
	workList->size -= 1;
	return s;
}

int empty(Sets *sets ) {
	if(sets->size == 0 || sets == NULL)
	  return 1;
	return 0;
}


//The subset Construction of DFA from NFA
DFA *subSetCon(State *start, Setc *Characters) {
	Sets *Q, *workList;
	Listc *Alphabets = Characters->elements;
	TransTable *table;
	table = malloc(sizeof(TransTable)); table->size = 0; table->T = NULL;
	init(Sets,Q); init(Sets,workList);
	Set *q, *t; init(Set,q);
	q = add(start,q);
	q = E_close(q);
 Set *S = q;	//save the start state of the DFA for later use
	Q = add2(q,Q);  workList = add2(q,workList);

	printf("Converting NFA to DFA...\n");

	while(!empty(workList)) {
		q = removeNextSet(workList);
		while(Alphabets) {
			t = E_close(Delta(q,Alphabets->c));
			//check whether the set t is in Q
			if(exists2(t,Q)) {
				Lists *tmp= Q->elements;
				for(; tmp; tmp=tmp->next)
				 if(equal(t,tmp->set)) t = tmp->set;	//then use the existing set
			}
			//store t in the transition table using the transition function T
			T(q,Alphabets->c,t,table);
			if(!exists2(t,Q)) {
				Q = add2(t,Q);
				workList = add2(t,workList);
			}
			Alphabets = Alphabets->next;
		}
		Alphabets = Characters->elements;
	}

	printf("Done\n\n");

	//Find all accepting state of the NFA
	Sets *A; init(Sets,A);
	Lists *tmp = Q->elements;
	List *tmpState;
	for(; tmp; tmp=tmp->next) {
		tmpState = tmp->set->elements;
		 for(; tmpState; tmpState=tmpState->next)
		 		if(tmpState->state->c == accept)
			 		A = add2(tmp->set,A);
	}

printf("*********************DFA Specification******************************\n");
printf("workList's size = %d\n",workList->size);
printf("Total Number of states: %d\n",Q->size);
printf("Total Number of Transitions: %d \n",table->size);
printf("Number Of Accepting States: %d \n",A->size);
printf("*****************************End***********************************\n\n");

//create the DFA
DFA *dfa = malloc(sizeof(DFA));
 dfa->D = Q; dfa->C = Characters;
 dfa->table = table;
 dfa->start = S;
 dfa->A = A;

 return dfa;
}


Set *d(Set *state,char c,DFA *dfa) {
	Transition *t = dfa->table->T;
	for(; t; t = t->next)
		if(t->q == state && t->c == c)
		{  return t->t;}
	return NULL;
}


//some Printing Functions for Debuging...
void printSet(Set *set) {
	if(!set) {printf("NULL Set\n");return;}
	List *element = set->elements;
	printf("\nset:>>>>>>>>>>>>>>>>>>>>>>>>>>>:\n");
	while(element) {
		printf("c: %d ID: %p\n",element->state->c,element->state);
		element=element->next;
	}
}

void printSets(Sets *set) {
	Lists *elements = set->elements;
	printf("\n\n:>>>>>>>>>>>>>>PRINTING SETS>>>>>>>>>>>>>:\n\n");
	while(elements) {
		printSet(elements->set);
		elements = elements->next;
	}
	printf("\n\n:>>>>>>>>>>>>>>END>>>>>>>>>>>>>:\n\n");
}

printTransitionTable(TransTable *table) {
	Transition *tr = table->T;
	printf("\n=====printing the transition table===\n");
	while(tr) {
		printf("%p ===> %d ===> %p\n",tr->q,tr->c,tr->t);
		tr= tr->next;
	}
	printf("===finished===\n");
}


//Regex PreProcessing...
QueueC *enqueueC(char c, QueueC *Q) {
	Listc *C  = malloc(sizeof(Listc));
	C->c = c; C->next = NULL;
	if(Q->size == 0) {
		Q->head = C; Q->size += 1;
		Q->tail = C;
	}
	else {
		Q->tail->next = C;
		Q->tail = C;
		Q->size += 1;
	}
	return Q;
}

char dequeueC(QueueC *Q) {
	if(Q == NULL || Q->size == 0) return ' ';
	char c = Q->head->c;
	Listc *tmp = Q->head;
	Q->head = Q->head->next;
	free((void*)tmp);
	Q->size -= 1;
	return c;
}

//function to trim off all spaces within regex
char *trimSpace(char*regex) {
	QueueC *Q = malloc(sizeof(Q));
	Q->size = 0;
	for(; *regex; regex++)
		if(*regex != ' ')
			Q = enqueueC(*regex,Q);

	//recconstruct the  regex
	char *regx = malloc(sizeof(char)*Q->size);
	int i=0;
	for(; Q->size;)
		regx[i++] = dequeueC(Q);
	return regx;
}


int isMetaChar(char c) {
	int i = 0;
	while(i < METASIZE)
	  if(c == metaChar[i++]) return 1;
	return 0;
}

//Determine if a character is in the range of a character class
int isCharacterClass(char c) {
	/*if(c >= 'a' && c <= 'z' || \
	 	 c >= 'A' && c <= 'Z' || \
		 c >= '0' && c <= '9' )*/
	if(c>='!' && c<='~')
		 return 1;
	return 0;
}

char encode(char c) {
	switch(c){
	case '*':
		return 10;
	  break;
	case '?':
		return 11;
	  break;
	case '+':
	  return 12;
	  break;
	case '|':
	   return 13;
	   break;
	case '.':
	   return 14;
	   break;
	case '(':
	   return 15;
	   break;
	case ')':
	  return 16;
	  break;
	/*case '#':
	   return 17;
	  break; */
	 case '[':
	 	return 20;
	 	break;
	 case ']':
	 	return 21;
	 	break;
	default:
	return c;
	break;
}

}

char decode(char c) {
	switch(c) {
		default:
			return c;
			break;
		case 10:
		  return ZERO_OR_MORE;
			break;
		case 12:
			return PLUS;
			break;
		case 11:
		return ANYCHAR;
		break;
		case 14:
		 return AND;
		 break;
		case 13:
		 return OR;
		break;
		case 15:
		 return OPEN_PAREN;
		 break;
		case 16:
		 return CLOSE_PAREN;
		 break;
		/* case 17:
		 return ESCAPE;
		 break; */
		 case 20:
		 return START_CHAR_CLASS;
		 break;
		 case 21:
		   return END_CHAR_CLASS;
		   break;

	}
}

//converts a queue to a string
char *Q2String(QueueC *Q) {

	char *str = malloc(sizeof(char)*Q->size);
	Listc *data;
	int  i=0;

	for(data = Q->head; data; data = data->next) 
		str[i++] = data->c;

	//append the null character to indicate the end of string
	str[i]=0; 

	return str;
}

char *scanForCharClass(char *regex) {

	int i;
	QueueC *Q = malloc(sizeof(QueueC));
	Q->size = 0;
	int done=0;

  for(i=0; i<strlen(regex); i++) {

	if(regex[i++] == '[' && !(done)) {
		  if( isCharacterClass(regex[i]) )
			{
				   char k = regex[i];
			  if(regex[++i] == '-') {
				  if(isCharacterClass(regex[++i]) && regex[i+1] ==']' )
					{ 
						char j = regex[i]; 
						if(j<k) { j=k; k=regex[i];}
						i +=2; //since strlen("[x-y]") = 5
						 done = 1;	// since we are done with this current bracket
						 Q = enqueueC('(',Q);
						 while(k <= j){ 
							 Q = enqueueC(k++,Q);
							 if(k<=j)
							 	Q = enqueueC('|',Q);
						 }
						 Q = enqueueC(')',Q);
					} else { i -=3; }
				} else { i -=2; }
			} else {--(i); }
		} else {i -=1; }
	 
	 if(i<strlen(regex))
			if(regex[i] == '[' && done) { //still watching go back and expand that...
				--i; 
				done = 0;
			}	
			else  Q = enqueueC(regex[i],Q); // otherwise enqueue...	
	}

	// return the string representation of Q

	return Q2String(Q);
}

char *scanForEscapeChar(char *regex) {

	int i;
	QueueC *Q = malloc(sizeof(QueueC));
	Q->size = 0;

	for(i=0; i<strlen(regex); i++) {
	
			if(regex[i] == ESCAPE) { 
				 Q = enqueueC(encode(regex[++i]),Q);  // escaping meta characters
			}
			
			else  Q = enqueueC(regex[i],Q);
	}

	//return the string representation of Q
	return Q2String(Q);
}


char *scanForAnyChar(char *regex) {
	int i;
	QueueC *Q = malloc(sizeof(QueueC)); Q->size = 0;

	for(i=0; i<strlen(regex); i++) {
	 	if(regex[i] == ANYCHAR) {
			char k = '!'; 
			Q = enqueueC('(',Q);
			while(k<='~') { 
				Q = enqueueC(encode(k++),Q);
				if(k<='~') Q=enqueueC('|',Q);
			}
			enqueueC(')',Q);
		}
		else  Q = enqueueC(regex[i],Q);
	}

 //return the string represention of Q
	return Q2String(Q);
}


char *preRegex(char *regex) {
	printf("PreProcessing Regular Expression....\n");
	regex = trimSpace(regex);
	regex = scanForEscapeChar(regex);
	regex = scanForCharClass(regex);
	regex = scanForAnyChar(regex);
	printf("Done\n\n");
	return regex;
}



//main

int main() {

 char *regex = "r[0-9][0-9]*";

int i=0;

char *pp = preRegex(regex); //printf("Preprocessed regex: %s\n",pp); 
Setc *charSet = getAlphabets(regex);
DFA *newDFA = subSetCon(rex2nfa(re2postfix(pp)),charSet);

char input[256];
Set *state = NULL;

printf("Starting Shell...\n");
while(1) {
	printf(">> ");
	fscanf(stdin,"%s",input);

	if(strcmp(input,"REGEX") == 0) {
		//free((void*)regex);
		///free((void*)newDFA);
		printf("ENTER NEW REGEX: ");
		regex = malloc(sizeof(char)*256);
		scanf("%s",regex);
		charSet = getAlphabets(regex);
		newDFA = subSetCon(rex2nfa(re2postfix(preRegex(regex))),charSet);
		printf("Regular Expression Changed...\n\n");

		//printf(">> ");
		//scanf("%s",input);

	}
	else if(strcmp(input,"EXIT") == 0)
	  break;
	else if(strcmp(input,"PRINT_R") == 0) printf("%s\n",regex);
	else if(strcmp(input,"HELP") == 0) {
		printf("****************************HELP SCREEN*******************************\n\n");
		printf("**************Shell Commands***************\n");
		printf("HELP ........... displays this help screen\n");
		printf("PRINT_R ........ prints the current regular expression \n");
		printf("PRINT_A ........ prints the alphabetic characters of current regex\n");
		printf("PRINT_P ........ prints the filters for this regular expression\n");
		printf("REGEX .......... enter new Regular expression\n");
		printf("*************************End of Shell Commands*************************\n\n");

		printf("**************Regular Expression Language***************\n");
		printf("# ........... Escape character\n");
		printf("* ........... Matches zero or more of previous character (Kleen closure)\n");
		printf("? ........... Matches any (printable) character\n");
		printf("| ........... The OR (or Alternation) operator\n");
		printf(". ........... The dot (or AND or Concatenation) operator\n");
		printf("[] .......... The character class operator e.g [a-z], [0-9] etc\n");
		printf("*************************End of Regular Expression Language*************************\n\n");

		printf("****************************END OF HELP SCREEN*******************************\n");
	}
	else if(strcmp(input,"PRINT_P") == 0) {
		char *esc = scanForEscapeChar(regex); printf("PASS ESCAPE Char: %s\n",esc);
		esc = scanForCharClass(esc);	printf("PASS CHAR CLASS: %s\n",esc);
		esc = scanForAnyChar(esc); 	printf("PASS ANY CHAR: %s\n",esc);
	}
	else if(strcmp(input,"PRINT_A") == 0) {	
 		Listc *tmp = charSet->elements;
 		for(; tmp; tmp=tmp->next) printf("%c  ",tmp->c); 
 		printf("\n");
	} 
	else {
	state = d(newDFA->start,input[i],newDFA);
	do {
		if(state == NULL) {printf("%s : Rejected \n",input); break;}
		if(exists2(state,newDFA->A)) {
			if(i==(strlen(input)-1))
				printf("%s : Accepted\n",input);
		//	else {printf("Rejected\n"); break;}
		}
		else if(state && i==(strlen(input)-1)) {printf("%s : Rejected\n",input); break;}
			if(++i < strlen(input))
			state = d(state,input[i],newDFA);
	} while(i<strlen(input));
	i=0;
  }
}
	return 0;
}
