//line330以前的內容都不要改動。只要改動line330以後的即可
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#pragma warning (disable: 4996)

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 1
#define TBLSIZE 64
#define MAXLEN 256
int flag_now_expr_error = 0;
int flag_now_expr_exists_assignment = 0;
int count_num_of_assignments = 0;
int count_num_of_divs = 0;
int flag_exists_variable = 0;

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum); \
}

typedef enum {
	UNKNOWN, END, ENDFILE,
	INT, ID,
	ADDSUB, MULDIV, LOGICAL,
	INCDEC, ASSIGN,
	LPAREN, RPAREN
} TokenSet;

typedef enum {
	UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;
void err(ErrorType errorNum);// Print error message and exit the program

typedef struct {
	int val;
	char name[MAXLEN];
} Symbol;

typedef struct _Node{
	TokenSet data;
	int val;
	char lexeme[MAXLEN];
	struct _Node *left;
	struct _Node *right;
	char assembly_alias[7];
} BTNode;
BTNode *assignments[MAXLEN];//紀錄等號所在的node的ptr
void record_assignments_ptr(BTNode *);
BTNode *Div[MAXLEN];//紀錄除號所在的node的ptr
void record_div_ptr(BTNode *);

Symbol table[TBLSIZE];
int sbcount = 0;

int match(TokenSet token);// Test if a token matches the current token 
void advance(void);// Get the next token
char *getLexeme(void);// Get the lexeme of the current token

int evaluateTree(BTNode *root);// Evaluate the syntax tree
void output_assembly(BTNode *root, int reg_idx);
void printPrefix(BTNode *root);// Print the syntax tree in prefix

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

void initTable(void);// Initialize the symbol table with builtin variables
int getval(char *str);// Get the value of a variable
int setval(char *str, int val);// Set the value of a variable

BTNode *makeNode(TokenSet tok, const char *lexe);
void freeTree(BTNode *root);

BTNode *factor(void);
BTNode *term(void);
BTNode *term_tail(BTNode *left);
BTNode *expr(void);
BTNode *expr_tail(BTNode *left);
void statement(void);

void record_assignments_ptr(BTNode *ptr) {
	assignments[count_num_of_assignments++] = ptr;
	
	return;
}

void record_div_ptr(BTNode *ptr) {
	Div[count_num_of_divs++] = ptr;

	return;
}


// term := factor term_tail
BTNode *term(void) {
	BTNode *node = factor();
	return term_tail(node);
}

// expr := term expr_tail
BTNode *expr(void) {
	BTNode *node = term();
	return expr_tail(node);
}


// statement := ENDFILE | END | expr END
void statement(void) {
	BTNode *retp = NULL;

	if (match(ENDFILE)) { 
		printf("MOV r0 [0]\n");
		printf("MOV r1 [4]\n");
		printf("MOV r2 [8]\n");
		printf("EXIT 0\n"); 
		//system("pause"); 
		exit(0);
	}
	else if (match(END)) {
		//printf(">> ");
		advance();
	}
	else {
		retp = expr();
		if (match(END)) {
			if (count_num_of_divs) {
				int flag_is_all_OK = 1;

				for (int i = 0; i < count_num_of_divs; i++) {
					if (evaluateTree(Div[i]->right) == 0) {
						if (!flag_exists_variable) flag_is_all_OK = 0;
					}
				}

				if (flag_is_all_OK) {
					if (flag_now_expr_exists_assignment) {
						for (int i = 0; i < count_num_of_assignments; i++) {
							output_assembly(assignments[i], 0);
						}
					}
				}
				else {
					printf("EXIT 1\n");
					//error(DIVZERO);
					//system("pause");
				}
			}
			else {
				if (flag_now_expr_exists_assignment) {
					for (int i = 0; i < count_num_of_assignments; i++) {
						output_assembly(assignments[i], 0);
					}
				}
			}

			freeTree(retp);

			//printf(">> ");
			advance();
		}
		else {
			 flag_now_expr_error = 1;
			 printf("EXIT 1\n");
			//error(SYNTAXERR);
		} 
	}
}

void err(ErrorType errorNum) {
	if (PRINTERR) {
		fprintf(stderr, "error: ");
		switch (errorNum) {
		case MISPAREN:
			fprintf(stderr, "mismatched parenthesis\n");
			system("pause");
			break;
		case NOTNUMID:
			fprintf(stderr, "number or identifier expected\n");
			system("pause");
			break;
		case NOTFOUND:
			fprintf(stderr, "variable not defined\n");
			system("pause");
			break;
		case RUNOUT:
			fprintf(stderr, "out of memory\n");
			system("pause");
			break;
		case NOTLVAL:
			fprintf(stderr, "lvalue required as an operand\n");
			system("pause");
			break;
		case DIVZERO:
			fprintf(stderr, "divide by constant zero\n");
			system("pause");
			break;
		case SYNTAXERR:
			fprintf(stderr, "syntax error\n");
			system("pause");
			break;
		default:
			fprintf(stderr, "undefined error\n");
			system("pause");
			break;
		}
	}
	exit(0);
}


int getval(char *str) {
	for (int i = 0; i < sbcount; i++)
		if (strcmp(str, table[i].name) == 0)
			return table[i].val;

	//error(NOTFOUND);
	return 0;
}


int setval(char *str, int val) {
	if (sbcount >= TBLSIZE)
		//error(RUNOUT);

	for (int i = 0; i < sbcount; i++) {
		if (strcmp(str, table[i].name) == 0) {
			table[i].val = val;
			return val;
		}
	}

	strcpy(table[sbcount].name, str);
	table[sbcount++].val = val;
	return val;
}


BTNode *makeNode(TokenSet tok, const char *lexe) {
	BTNode *node = (BTNode *)malloc(sizeof(BTNode));
	strcpy(node->lexeme, lexe);
	if (strcmp(lexe, "/") == 0) record_div_ptr(node);

	node->data = tok;
	node->val = 0;
	node->left = NULL;
	node->right = NULL;

	return node;
}


void freeTree(BTNode *root) {
	if (root != NULL) {
		freeTree(root->left);
		freeTree(root->right);
		free(root);
	}
}


void printPrefix(BTNode *root) {
	if (root != NULL) {
		printf("%s ", root->lexeme);
		printPrefix(root->left);
		printPrefix(root->right);
	}
}


void initTable(void) {
	strcpy(table[0].name, "x");	 table[0].val = 0;
	strcpy(table[1].name, "y"); table[1].val = 0;
	strcpy(table[2].name, "z");	 table[2].val = 0;

	sbcount = 3;

	return;
}


TokenSet getToken(void) {
	char c = '\0';

	while ((c = fgetc(stdin)) == ' ' || c == '\t');

	if (isdigit(c)) {
		lexeme[0] = c;
		c = fgetc(stdin);
		int i = 1;
		while (isdigit(c) && i < MAXLEN) {
			lexeme[i] = c;
			++i;
			c = fgetc(stdin);
		}
		ungetc(c, stdin);
		lexeme[i] = '\0';
		return INT;
	}
	else if (c == '+' || c == '-') {
		lexeme[0] = c;
		c = fgetc(stdin);
		if (c == lexeme[0]) {
			lexeme[1] = c;
			lexeme[2] = '\0';
			return INCDEC;
		}
		else {
			ungetc(c, stdin);
			lexeme[1] = '\0';
			return ADDSUB;
		}
	}
	else if (c == '&' || c == '|' || c == '^') {
		lexeme[0] = c;
		lexeme[1] = '\0';
		return LOGICAL;
	}
	else if (c == '*' || c == '/') {
		lexeme[0] = c;
		lexeme[1] = '\0';
		return MULDIV;
	}
	else if (c == '\n') {
		lexeme[0] = '\0';
		return END;
	}
	else if (c == '=') {
		strcpy(lexeme, "=");
		return ASSIGN;
	}
	else if (c == '(') {
		strcpy(lexeme, "(");
		return LPAREN;
	}
	else if (c == ')') {
		strcpy(lexeme, ")");
		return RPAREN;
	}
	else if (isalpha(c) || c == '_') {
		lexeme[0] = c;
		c = fgetc(stdin);
		int i = 1;
		while (isalpha(c) || isdigit(c) || c == '_') {
			lexeme[i] = c;
			++i;
			c = fgetc(stdin);
		}
		ungetc(c, stdin);
		lexeme[i] = '\0';
		return ID;
	}
	else if (c == EOF) return ENDFILE;
	else return UNKNOWN;
}


void advance(void) {
	curToken = getToken();

	return;
}


int match(TokenSet token) {
	if (curToken == UNKNOWN) advance();
	return token == curToken;
}


char *getLexeme(void) {
	return lexeme;
}


int main() {
	initTable();
	//printf(">> ");
	while (!flag_now_expr_error) { 
		memset(assignments, NULL, sizeof(assignments));
		count_num_of_assignments = 0;
		flag_now_expr_exists_assignment = 0;
		count_num_of_divs = 0;
		flag_exists_variable = 0;
		flag_now_expr_error = 0;
		
		statement(); 
	}
	/*
	system("pause");
	system("pause");
	system("pause");
	system("pause");
	*/
}



// factor := /*INT*/ | /*ADDSUB INT/* |
//		   	 /*ID*/  | /*ADDSUB ID*/  | 
//		   	 /*ID ASSIGN expr*/ |
//		   	 /*LPAREN expr RPAREN*/ |
//		   	 /*ADDSUB LPAREN expr RPAREN*/ |
//         INCDEC ID
BTNode *factor(void) {
	BTNode *retp = NULL, *left = NULL;

	if (match(INT)) {
		retp = makeNode(INT, getLexeme());
		advance();
	}
	else if (match(ID)) {
		if (count_num_of_divs)flag_exists_variable = 1;

		left = makeNode(ID, getLexeme());
		advance();
		if (!match(ASSIGN)) retp = left;
		else {
			flag_now_expr_exists_assignment = 1;

			retp = makeNode(ASSIGN, getLexeme());
			record_assignments_ptr(retp);
			advance();
			retp->left = left;
			retp->right = expr();
		}
	}
	else if (match(ADDSUB)) {
		retp = makeNode(ADDSUB, getLexeme());
		retp->left = makeNode(INT, "0");
		advance();
		if (match(INT)) {
			retp->right = makeNode(INT, getLexeme());
			advance();
		}
		else if (match(ID)) {
			retp->right = makeNode(ID, getLexeme());
			advance();
		}
		else if (match(LPAREN)) {
			advance();
			retp->right = expr();
			if (match(RPAREN)) advance();
			else {
				
				flag_now_expr_error = 1;
				printf("EXIT 1\n");
				//error(MISPAREN);
			}
		}
		else {
			
			flag_now_expr_error = 1;
			printf("EXIT 1\n");
			//error(NOTNUMID);
		}
	}
	else if (match(LPAREN)) {
		advance();
		retp = expr();
		if (match(RPAREN)) advance();
		else {
			
			flag_now_expr_error = 1;
			printf("EXIT 1\n");
			//error(MISPAREN);
		}

	}
	else if (match(INCDEC)) {
		char type[3];
		strcpy(type, getLexeme());//把到底是++?還是--?先儲存起來
		char plus_or_minus[2]; plus_or_minus[0] = type[0]; plus_or_minus[1] = '\0';
		advance();

		if (match(ID)) {//只有後面是變數的才會正確，如果是++6，或者++(x+1)，都是不正確。
			char name_of_variable[MAXLEN];
			strcpy(name_of_variable, getLexeme());

			retp = makeNode(ASSIGN, "=");
			record_assignments_ptr(retp);

			flag_now_expr_exists_assignment = 1;

			retp->left = makeNode(ID, name_of_variable);
			retp->right = makeNode(ADDSUB, plus_or_minus);
			retp->right->left= makeNode(ID, name_of_variable);
			retp->right->right = makeNode(INT, "1");

			advance();

		}
		else {
			
			flag_now_expr_error = 1;
			printf("EXIT 1\n");
			//error(SYNTAXERR);
		}
	}
	else {
		
		 flag_now_expr_error = 1;
		 printf("EXIT 1\n");
		//error(NOTNUMID);
	}

	return retp;
}


// term_tail := MULDIV factor term_tail | NiL
BTNode *term_tail(BTNode *left) {
	BTNode *node = NULL;

	if (match(MULDIV)) {
		node = makeNode(MULDIV, getLexeme());
		advance();
		node->left = left;
		node->right = factor();
		return term_tail(node);
	}
	else return left;
}


// expr_tail := ADDSUB term expr_tail | NiL
BTNode *expr_tail(BTNode *left) {
	BTNode *node = NULL;

	if (match(ADDSUB)) {
		node = makeNode(ADDSUB, getLexeme());
		advance();
		node->left = left;
		node->right = term();
		return expr_tail(node);
	}
	else if (match(LOGICAL)) {
		node = makeNode(LOGICAL, getLexeme());
		advance();
		node->left = left;
		node->right = term();
		return expr_tail(node);
	}
	else return left;
}

int evaluateTree(BTNode *root) {
	int retval = 0, lv = 0, rv = 0;

	if (root != NULL) {
		switch (root->data) {
		case ID:
			retval = getval(root->lexeme);
			break;
		case INT:
			retval = atoi(root->lexeme);
			break;
		case ASSIGN:
			rv = evaluateTree(root->right);
			retval = setval(root->left->lexeme, rv);
			break;
		case ADDSUB:
		case MULDIV:
			lv = evaluateTree(root->left);
			rv = evaluateTree(root->right);
			if (strcmp(root->lexeme, "+") == 0)
				retval = lv + rv;
			else if (strcmp(root->lexeme, "-") == 0)
				retval = lv - rv;
			else if (strcmp(root->lexeme, "*") == 0)
				retval = lv * rv;
			else if (strcmp(root->lexeme, "/") == 0) {
				if (rv == 0);//error(DIVZERO);
				retval = lv / rv;
			}
			break;
		case LOGICAL:
			lv = evaluateTree(root->left);
			rv = evaluateTree(root->right);
			if (strcmp(root->lexeme, "&") == 0)
				retval = lv & rv;
			else if (strcmp(root->lexeme, "|") == 0)
				retval = lv | rv;
			else if (strcmp(root->lexeme, "^") == 0)
				retval = lv ^ rv;
			break;
		default:
			retval = 0;
		}
	}

	return retval;
}


void output_assembly(BTNode *root, int reg_idx) {
	if (root->data == ASSIGN) {
		output_assembly(root->right, reg_idx);


		if (root->right->data == ID || root->right->data == INT || root->right->data == ASSIGN) {
			char temp[7], temp_next[7];
			if (reg_idx / 10 == 0) {
				temp[0] = 'r'; temp[1] = reg_idx + '0'; temp[2] = '\0';
			}
			else {
				temp[0] = 'r'; temp[1] = (reg_idx / 10) + '0'; temp[2] = (reg_idx % 10) + '0'; temp[3] = '\0';
			}

			int reg_idx_next = reg_idx + 1;
			if (reg_idx_next / 10 == 0) {
				temp_next[0] = 'r'; temp_next[1] = reg_idx_next + '0'; temp_next[2] = '\0';
			}
			else {
				temp_next[0] = 'r'; temp_next[1] = (reg_idx_next / 10) + '0'; temp_next[2] = (reg_idx_next % 10) + '0'; temp_next[3] = '\0';
			}

			int idx = 0;
			if (root->right->data == ID) {
				
				for (int i = 0; i < sbcount; i++) {
					if (strcmp(table[i].name, root->right->lexeme) == 0) {
						printf("MOV %s [%d]\n", temp_next, 4 * i);
					}
				}

				for (int i = 0; i < sbcount; i++) {
					if (strcmp(table[i].name, root->left->lexeme) == 0) {
						printf("MOV [%d] %s\n", 4 * i, temp_next);
						idx = i;
					}
				}
			}
			else if (root->right->data == INT) {
				printf("MOV %s %s\n", temp_next, root->right->lexeme);

				for (int i = 0; i < sbcount; i++) {
					if (strcmp(table[i].name, root->left->lexeme) == 0) {
						printf("MOV [%d] %s\n", 4 * i, temp_next);
						idx = i;
					}
				}
			}
			else if (root->right->data == ASSIGN) {
				printf("MOV %s %s\n", temp_next, root->right->assembly_alias);

				for (int i = 0; i < sbcount; i++) {
					if (strcmp(table[i].name, root->left->lexeme) == 0) {
						printf("MOV [%d] %s\n", 4 * i, temp_next);
						idx = i;
					}
				}
			}

			idx *= 4;
			//char temp[7];//Redefinition
			if (idx / 100 == 0) {
				if (idx / 10 == 0) {
					temp[0] = '['; temp[1] = idx + '0'; temp[2] = ']'; temp[3] = '\0';
				}
				else { temp[0] = '['; temp[1] = (idx / 10) + '0'; temp[2] = (idx % 10) + '0'; temp[3] = ']'; temp[4] = '\0'; }
			}
			else { temp[0] = '['; temp[1] = (idx / 100) + '0'; temp[2] = ((idx % 100) / 10) + '0'; temp[3] = (idx / 10) + '0'; temp[4] = ']'; temp[5] = '\0'; }

			strcpy(root->left->assembly_alias, temp);
			strcpy(root->assembly_alias, root->left->assembly_alias);
		}
		else {
			strcpy(root->left->assembly_alias, root->right->assembly_alias);

			int idx = 0;
			for (int i = 0; i < sbcount; i++) {
				if (strcmp(table[i].name, root->left->lexeme) == 0) {
					printf("MOV [%d] %s\n", 4 * (i), root->left->assembly_alias);
					idx = i;
					break;
				}
			}

			idx *= 4;
			char temp[7];
			if (idx / 100 == 0) {
				if (idx / 10 == 0) {
					temp[0] = '['; temp[1] = idx + '0'; temp[2] = ']'; temp[3] = '\0';
				}
				else { temp[0] = '['; temp[1] = (idx / 10) + '0'; temp[2] = (idx % 10) + '0'; temp[3] = ']'; temp[4] = '\0'; }
			}
			else { temp[0] = '['; temp[1] = (idx / 100) + '0'; temp[2] = ((idx % 100) / 10) + '0'; temp[3] = (idx / 10) + '0'; temp[4] = ']'; temp[5] = '\0'; }

			strcpy(root->assembly_alias, temp);
		}
		
		
	}
	if (root->data == ADDSUB || root->data == MULDIV || root->data == LOGICAL) {
		output_assembly(root->left, reg_idx);
		output_assembly(root->right, reg_idx+1);


		char temp[7], temp_next[7];
		if (reg_idx / 10 == 0) {
			temp[0] = 'r'; temp[1] = reg_idx + '0'; temp[2] = '\0';
		}
		else {
			temp[0] = 'r'; temp[1] = (reg_idx / 10) + '0'; temp[2] = (reg_idx % 10) + '0'; temp[3] = '\0';
		}

		int reg_idx_next = reg_idx + 1;
		if (reg_idx_next / 10 == 0) {
			temp_next[0] = 'r'; temp_next[1] = reg_idx_next + '0'; temp_next[2] = '\0';
		}
		else {
			temp_next[0] = 'r'; temp_next[1] = (reg_idx_next / 10) + '0'; temp_next[2] = (reg_idx_next % 10) + '0'; temp_next[3] = '\0';
		}
		strcpy(root->left->assembly_alias, temp);
		strcpy(root->right->assembly_alias, temp_next);
		if (root->left->data == INT && root->right->data==INT) {
			printf("MOV %s %s\n", temp, root->left->lexeme);
			printf("MOV %s %s\n", temp_next, root->right->lexeme);
		}
		else if (root->left->data == INT && root->right->data == ID) {
			printf("MOV %s %s\n", temp, root->left->lexeme);

			for (int i = 0; i < sbcount; i++) {
				if (strcmp(table[i].name, root->right->lexeme) == 0) {
					printf("MOV %s [%d]\n", root->right->assembly_alias, 4 * i);
				}
			}
		}
		else if (root->left->data == ID && root->right->data == ID) {
			for (int i = 0; i < sbcount; i++) {
				if (strcmp(table[i].name, root->left->lexeme) == 0) {
					printf("MOV %s [%d]\n", root->left->assembly_alias, 4 * i);
				}
			}

			for (int i = 0; i < sbcount; i++) {
				if (strcmp(table[i].name, root->right->lexeme) == 0) {
					printf("MOV %s [%d]\n", root->right->assembly_alias, 4 * i);
				}
			}
		}
		else if (root->left->data == ID && root->right->data == INT) {
			for (int i = 0; i < sbcount; i++) {
				if (strcmp(table[i].name, root->left->lexeme) == 0) {
					printf("MOV %s [%d]\n", root->left->assembly_alias, 4 * i);
				}
			}

			printf("MOV %s %s\n", temp, root->right->lexeme);
		}


		strcpy(root->assembly_alias, root->left->assembly_alias);
		switch (root->lexeme[0]) {
		case'+':{	printf("ADD %s %s\n", root->left->assembly_alias, root->right->assembly_alias); break;	}
		case'-': {	printf("SUB %s %s\n", root->left->assembly_alias, root->right->assembly_alias); break;	}
		case'*': {	printf("MUL %s %s\n", root->left->assembly_alias, root->right->assembly_alias); break;	}
		case'/': {	printf("DIV %s %s\n", root->left->assembly_alias, root->right->assembly_alias); break;	}
		case'&': {printf("AND %s %s\n", root->left->assembly_alias, root->right->assembly_alias); break;	}
		case'|': {	printf("OR %s %s\n", root->left->assembly_alias, root->right->assembly_alias); break;	}
		case'^': {printf("XOR %s %s\n", root->left->assembly_alias, root->right->assembly_alias); break;	}
		}
	}
	if (root->data == INT || root->data == ID)return;
}