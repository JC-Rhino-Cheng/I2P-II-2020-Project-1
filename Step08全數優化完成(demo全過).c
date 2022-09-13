//line330以前的內容都不要改動。只要改動line330以後的即可(PS: line330的330已經是個不正確的數字了)

//reg_max_len記得改為8，PRINTERR改成0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#pragma warning (disable: 4996)

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define CCC 0//CCC refers to Calculate_Clock_Cycle
#define PRINTERR 0
#define TBLSIZE 64
#define MAXLEN 256
#define reg_max_len 8
int flag_now_expr_error = 0;
int flag_now_expr_exists_assignment = 0;
int count_num_of_assignments = 0;
int count_num_of_divs = 0;
int flag_exists_variable = 0;
int flag_is_reg_idx_available[reg_max_len];

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
	LPAREN, RPAREN,
	DISCARDED_CZ_PROCESSED,
} TokenSet;

typedef enum {
	UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR, RUNOUTOFREG
} ErrorType;
void err(ErrorType errorNum);// Print error message and exit the program

typedef struct {
	int val;
	char name[MAXLEN];
} Symbol;

typedef struct _Node {
	TokenSet data;
	int val;
	char lexeme[MAXLEN];
	struct _Node *left;
	struct _Node *right;
	int reg_num;
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
void output_assembly(BTNode *root, BTNode *parent);
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
		//因為r0、r1、r2常駐x、y、z的值，所以不用再搬動
		/*printf("MOV r0 [0]\n");
		printf("MOV r1 [4]\n");
		printf("MOV r2 [8]\n");*/
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
							evaluateTree(retp);//執行這個只是想利用這裡面的某些行來正確把新的未知數填入table，這樣下一行的output_assembly才能夠使用。
							output_assembly(assignments[i], NULL);
						}
					}
				}
				else {
					printf("EXIT 1\n");
					error(DIVZERO);
					//system("pause");
				}
			}
			else {
				if (flag_now_expr_exists_assignment) {
					for (int i = 0; i < count_num_of_assignments; i++) {
						evaluateTree(retp);//執行這個只是想利用這裡面的某些行來正確把新的未知數填入table，這樣下一行的output_assembly才能夠使用。
						output_assembly(assignments[i], NULL);
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
			error(SYNTAXERR);
		}
	}
}

void err(ErrorType errorNum) {
	if (PRINTERR) {
		fprintf(stderr, "error: ");
		switch (errorNum) {
		case MISPAREN:
			fprintf(stderr, "mismatched parenthesis\n");
			//system("pause");
			break;
		case NOTNUMID:
			fprintf(stderr, "number or identifier expected\n");
			//system("pause");
			break;
		case NOTFOUND:
			fprintf(stderr, "variable not defined\n");
			//system("pause");
			break;
		case RUNOUT:
			fprintf(stderr, "out of memory\n");
			//system("pause");
			break;
		case NOTLVAL:
			fprintf(stderr, "lvalue required as an operand\n");
			//system("pause");
			break;
		case DIVZERO:
			fprintf(stderr, "divide by constant zero\n");
			//system("pause");
			break;
		case SYNTAXERR:
			fprintf(stderr, "syntax error\n");
			//system("pause");
			break;
		case RUNOUTOFREG:
			fprintf(stderr, "run out of register\n");
			//system("pause");
			break;
		default:
			fprintf(stderr, "undefined error\n");
			//system("pause");
			break;
		}
	}
	exit(0);
}


int getval(char *str) {
	for (int i = 0; i < sbcount; i++)
		if (strcmp(str, table[i].name) == 0)
			return table[i].val;

	//如果建樹完成後，在執行evaluateTree時到了這邊，要找變數編號找不到，代表在此expr之前沒有任何expr宣告了這個新變數，所以要印出EXIT 1。
	printf("EXIT 1\n");
	error(NOTFOUND);
	return 0;
}


int setval(char *str, int val) {
	if (sbcount >= TBLSIZE)
		error(RUNOUT);

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

	printf("MOV r0 [0]\n");
	printf("MOV r1 [4]\n");
	printf("MOV r2 [8]\n");

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
	if (CCC)freopen("input.txt", "w", stdout);
	initTable();
	//printf(">> ");
	while (!flag_now_expr_error) {
		memset(assignments, NULL, sizeof(assignments));
		for (int i = 0; i < reg_max_len; i++)flag_is_reg_idx_available[i] = 1;
		flag_is_reg_idx_available[0] = flag_is_reg_idx_available[1] = flag_is_reg_idx_available[2] = 0;//將r[0~2]常駐給x、y、z使用

		count_num_of_assignments = 0;
		flag_now_expr_exists_assignment = 0;
		count_num_of_divs = 0;
		flag_exists_variable = 0;
		flag_now_expr_error = 0;

		statement();
	}
	/*
	//system("pause");
	//system("pause");
	//system("pause");
	//system("pause");
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
				error(MISPAREN);
			}
		}
		else {

			flag_now_expr_error = 1;
			printf("EXIT 1\n");
			error(NOTNUMID);
		}
	}
	else if (match(LPAREN)) {
		advance();
		retp = expr();
		if (match(RPAREN)) advance();
		else {

			flag_now_expr_error = 1;
			printf("EXIT 1\n");
			error(MISPAREN);
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
			retp->right->left = makeNode(ID, name_of_variable);
			retp->right->right = makeNode(INT, "1");

			advance();

		}
		else {

			flag_now_expr_error = 1;
			printf("EXIT 1\n");
			error(SYNTAXERR);
		}
	}
	else {

		flag_now_expr_error = 1;
		printf("EXIT 1\n");
		error(NOTNUMID);
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
			root->val = retval;
			break;
		case INT:
			retval = atoi(root->lexeme);
			root->val = retval;
			break;
		case ASSIGN:
			rv = evaluateTree(root->right);
			root->val = rv;
			retval = setval(root->left->lexeme, rv);
			root->left->val = retval;
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
				//if (rv == 0)error(DIVZERO);
				retval = lv / rv;
			}
			root->val = retval;
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
			root->val = retval;
			break;
		default:
			retval = 0;
		}
	}

	return retval;
}


void output_assembly(BTNode *root, BTNode *parent) {
	if (root == NULL)return;
	if (root->data == INT || root->data == ID) return;

	if (root->data != ASSIGN && root->data != DISCARDED_CZ_PROCESSED && root->data != DISCARDED_CZ_PROCESSED)output_assembly(root->left, root);
	output_assembly(root->right, root);

	if (root->data == ADDSUB || root->data == MULDIV || root->data == LOGICAL) {
		//找尋左邊的可用reg編號
		int left_reg_idx = 0;
		char temp_for_cmp[MAXLEN];
		strcpy(temp_for_cmp, root->left->lexeme);
		int flag_reg_allocated_to_left = 0;//紀錄左邊是否要了reg。

		if (root->left->data == ID || root->left->data == INT) {
			/*
			//看看left的lexeme是不是x或y或z。如果是，因為是常駐在r[0~2]所以就不分配。如果不是，才分配reg。
			if (strcmp(temp_for_cmp, "x") == 0) {
				left_reg_idx = 0;
				root->left->reg_num = left_reg_idx;
				flag_reg_allocated_to_left = 0;
			}
			else if (strcmp(temp_for_cmp, "y") == 0) {
				left_reg_idx = 1;
				root->left->reg_num = left_reg_idx;
				flag_reg_allocated_to_left = 0;
			}
			else if (strcmp(temp_for_cmp, "z") == 0) {
				left_reg_idx = 2;
				root->left->reg_num = left_reg_idx;
				flag_reg_allocated_to_left = 0;
			}
			else {
				for (int i = 0; i < reg_max_len; i++) {
					if (flag_is_reg_idx_available[i]) {
						left_reg_idx = i;
						root->left->reg_num = i;
						flag_is_reg_idx_available[i] = 0;
						flag_reg_allocated_to_left = 1;
						break;
					}
					else if (i == reg_max_len - 1) {
						err(RUNOUTOFREG);
						//system("pause");
					}
				}
			}
			*/
			//上面那段大錯特錯，因為如果x、y、z在等號右邊參與計算的話，如果x是在一組(3個node為一組)的左邊的話，你這樣設計會導致x的常駐r0值被改動。下面那段尋找右邊可用reg也是如此，但我就只保留這個錯的在找尋左邊可用reg這段，以示後人。
			for (int i = 0; i < reg_max_len; i++) {
				if (flag_is_reg_idx_available[i]) {
					left_reg_idx = i;
					root->left->reg_num = i;
					flag_is_reg_idx_available[i] = 0;
					flag_reg_allocated_to_left = 1;
					break;
				}
				else if (i == reg_max_len - 1) {
					err(RUNOUTOFREG);
					//system("pause");
				}
			}
		}
		else {//如果左邊是ADDSUB或MULDIV或LOGICAL或DISCARDED
			left_reg_idx = root->left->reg_num;
		}

		//找尋右邊的可用reg編號
		int right_reg_idx = 0;
		//char temp_for_cmp[MAXLEN];//redefinition
		strcpy(temp_for_cmp, root->right->lexeme);
		int flag_reg_allocated_to_right = 0;//紀錄右邊是否要了reg。

		if (root->right->data == ID || root->right->data == INT) {
			for (int i = 0; i < reg_max_len; i++) {
				if (flag_is_reg_idx_available[i]) {
					right_reg_idx = i;
					root->right->reg_num = i;
					flag_is_reg_idx_available[i] = 0;
					flag_reg_allocated_to_right = 1;
					break;
				}
				else if (i == reg_max_len - 1) {
					err(RUNOUTOFREG);
					//system("pause");
				}
			}
		}
		else {//如果左邊是ADDSUB或MULDIV或LOGICAL或DISCARDED
			left_reg_idx = root->left->reg_num;
		}

		//開始找尋左右兩邊的變數編號。如果不是ID而是(1)INT或者(2)ADDSUBorMULDIVorLOGICALorASSIGN，也沒關係，只是棄置不用。
		int left_memory_idx, right_memory_idx;
		for (int i = 0; i < TBLSIZE; i++) {
			if (root->left->data == ID)
				if (strcmp(root->left->lexeme, table[i].name) == 0)left_memory_idx = i;
			if (root->right->data == ID)
				if (strcmp(root->right->lexeme, table[i].name) == 0)right_memory_idx = i;
		}

		//先輸出左邊
		switch (root->left->data) {
		case ID: {
			if (strcmp(root->left->lexeme, "x") != 0 && strcmp(root->left->lexeme, "y") != 0 && strcmp(root->left->lexeme, "z") != 0) {//如果左邊是個非x，也非y，也非z的變數，ex: Neilson。那麼，需要MOV 
				printf("MOV r%d [%d]\n", left_reg_idx, 4 * left_memory_idx); break;
			}
			else {//如果左邊是x，y，或z
				//先找到到底是x還是y還是z
				int idx;
				if (strcmp(root->left->lexeme, "x") == 0)idx = 0;
				else if (strcmp(root->left->lexeme, "y") == 0)idx = 1;
				else if (strcmp(root->left->lexeme, "z") == 0)idx = 2;

				printf("MOV r%d r%d\n", left_reg_idx, idx);
				break;
			}
		}
		case INT: {printf("MOV r%d %d\n", left_reg_idx, root->left->val); break; }
		case ADDSUB:
		case MULDIV:
		case LOGICAL:
		case DISCARDED_CZ_PROCESSED: {break;	}//這四種情形已經的reg_num都已經有了，就不用動了
		}

		//再輸出右邊
		switch (root->right->data) {
		case ID: {
			if (strcmp(root->right->lexeme, "x") != 0 && strcmp(root->right->lexeme, "y") != 0 && strcmp(root->right->lexeme, "z") != 0) {//如果左邊是個非x，也非y，也非z的變數，ex: Neilson。那麼，需要MOV 
				printf("MOV r%d [%d]\n", right_reg_idx, 4 * right_memory_idx); break;
			}
			else {//如果左邊是x，y，或z
				//先找到到底是x還是y還是z
				int idx;
				if (strcmp(root->right->lexeme, "x") == 0)idx = 0;
				else if (strcmp(root->right->lexeme, "y") == 0)idx = 1;
				else if (strcmp(root->right->lexeme, "z") == 0)idx = 2;

				printf("MOV r%d r%d\n", right_reg_idx, idx);
				break;
			}
		}
		case INT: {printf("MOV r%d %d\n", right_reg_idx, root->right->val); break; }
		case ADDSUB:
		case MULDIV:
		case LOGICAL:
		case DISCARDED_CZ_PROCESSED: {break;	}//這四種情形已經的reg_num都已經有了，就不用動了
		}


		root->data = root->right->data = root->left->data = DISCARDED_CZ_PROCESSED;//因為如果改成ID或者INT，之後明明已經輸出完畢了還是會因為判斷式子而進來。雖然他進來之後只會進行冗餘的指令，並不影響正確性，不過乾脆就不讓他進來，比較簡單，time clock cycle也比較俐落。
		root->reg_num = root->left->reg_num;

		switch (root->lexeme[0]) {
			case '+': {printf("ADD r%d r%d\n", root->left->reg_num, root->right->reg_num); break; }
			case '-': {printf("SUB r%d r%d\n", root->left->reg_num, root->right->reg_num); break; }
			case '*': {printf("MUL r%d r%d\n", root->left->reg_num, root->right->reg_num); break; }
			case '/': {printf("DIV r%d r%d\n", root->left->reg_num, root->right->reg_num); break; }
			case '&': {printf("AND r%d r%d\n", root->left->reg_num, root->right->reg_num); break; }
			case '|': {printf("OR r%d r%d\n", root->left->reg_num, root->right->reg_num); break; }
			case '^': {printf("XOR r%d r%d\n", root->left->reg_num, root->right->reg_num); break; }
		}
		if (root->right->reg_num != 0 && root->right->reg_num != 1 && root->right->reg_num != 2)//只有在reg編號不為0或1或2時，才可以釋放
			flag_is_reg_idx_available[root->right->reg_num] = 1;
		
	}
	else if (root->data == ASSIGN) {
		root->data = DISCARDED_CZ_PROCESSED;

		//先找出左邊的變數的編號
		int idx_of_variable_of_root_of_left_subtree = -1;//因為0、1、2分別是x、y、z的變數，所以預設值就不能用0、1、2
		for (int i = 0; i < TBLSIZE; i++) {
			if (strcmp(table[i].name, root->left->lexeme) == 0) {
				idx_of_variable_of_root_of_left_subtree = i;
				break;
			}
		}

		//再找出右邊的變數的編號，如果右邊是變數的話
		int idx_of_variable_of_root_of_right_subtree = -1;//因為0、1、2分別是x、y、z的變數，所以預設值就不能用0、1、2
		if (root->right->data == ID) {
			for (int i = 0; i < TBLSIZE; i++) {
				if (strcmp(table[i].name, root->right->lexeme) == 0) {
					idx_of_variable_of_root_of_right_subtree = i;
					break;
				}
			}
		}

		//再找出右邊的reg編號。這只有INT或「非x也非y也非z」的ID才需要
		int right_reg_idx = 0;
		char temp_for_cmp[MAXLEN];
		strcpy(temp_for_cmp, root->right->lexeme);
		int flag_reg_allocated_to_right = 0;//紀錄右邊是否要了reg。

		if (root->right->data == ID || root->right->data == INT) {
			//看看right的lexeme是不是x或y或z。如果是，因為是常駐在r[0~2]所以就不分配。如果不是，才分配reg。
			if (strcmp(temp_for_cmp, "x") == 0) {
				right_reg_idx = 0;
				root->right->reg_num = right_reg_idx;
				flag_reg_allocated_to_right = 0;
			}
			else if (strcmp(temp_for_cmp, "y") == 0) {
				right_reg_idx = 1;
				root->right->reg_num = right_reg_idx;
				flag_reg_allocated_to_right = 0;
			}
			else if (strcmp(temp_for_cmp, "z") == 0) {
				right_reg_idx = 2;
				root->right->reg_num = right_reg_idx;
				flag_reg_allocated_to_right = 0;
			}
			else {
				for (int i = 0; i < reg_max_len; i++) {
					if (flag_is_reg_idx_available[i]) {
						right_reg_idx = i;
						root->right->reg_num = i;
						flag_is_reg_idx_available[i] = 0;
						flag_reg_allocated_to_right = 1;
						break;
					}
					else if (i == reg_max_len - 1) {
						err(RUNOUTOFREG);
						//system("pause");
					}
				}
			}
		}
		//其實如果右邊是ADDSUB或MULDIV或LOGICAL(以上三種是在最底層的狀態取用)或DISCARDED_CZ_PROCESSED(此種是在root->right原本是ASSIGN的情形下，已經執行完畢)，也要取用他們自己的reg編號
		else if(root->right->data==ADDSUB||root->right->data==MULDIV||root->right->data==LOGICAL||root->right->data==DISCARDED_CZ_PROCESSED){
			right_reg_idx = root->right->reg_num;
		}

		//開始進行ASSIGN的運作
		if (idx_of_variable_of_root_of_left_subtree == 0 || idx_of_variable_of_root_of_left_subtree == 1 || idx_of_variable_of_root_of_left_subtree == 2) {//如果左邊變數是x、y、z其中一個
			switch (root->right->data) {
				case ID: {
					if (idx_of_variable_of_root_of_right_subtree == 0 || idx_of_variable_of_root_of_right_subtree == 1 || idx_of_variable_of_root_of_right_subtree == 2) {//如果右邊變數是x、y、z其中一個
						if(idx_of_variable_of_root_of_left_subtree!= idx_of_variable_of_root_of_right_subtree)//如果兩個變數不是同樣的話，才需要assign
							printf("MOV r%d r%d\n", idx_of_variable_of_root_of_left_subtree, idx_of_variable_of_root_of_right_subtree);//因為x、y、z的變數編號和reg編號一樣，故借用
					}
					else {//如果右邊變數不是x、y、z其中任何一個
						printf("MOV r%d [%d]\n", idx_of_variable_of_root_of_left_subtree, 4 * idx_of_variable_of_root_of_right_subtree);
					}
					break;
				}
				case INT: {
					printf("MOV r%d %d\n", right_reg_idx, root->right->val);
					printf("MOV r%d r%d\n", idx_of_variable_of_root_of_left_subtree, right_reg_idx);

					break; 
				}
				case ADDSUB:
				case MULDIV:
				case LOGICAL:
				case DISCARDED_CZ_PROCESSED: {printf("MOV r%d r%d\n", idx_of_variable_of_root_of_left_subtree, right_reg_idx); break; }
			}
		}
		else {//如果左邊變數不是是x、y、z其中任何一個
			switch (root->right->data) {
			case ID: {
				if (idx_of_variable_of_root_of_right_subtree == 0 || idx_of_variable_of_root_of_right_subtree == 1 || idx_of_variable_of_root_of_right_subtree == 2) {//如果右邊變數是x、y、z其中一個
					printf("MOV [%d] r%d\n", 4 * idx_of_variable_of_root_of_left_subtree, idx_of_variable_of_root_of_right_subtree);
				}
				else {//如果右邊變數不是x、y、z其中任何一個
					printf("MOV r%d [%d]\n", right_reg_idx, 4 * idx_of_variable_of_root_of_right_subtree);
					printf("MOV [%d] r%d\n", 4 * idx_of_variable_of_root_of_left_subtree, right_reg_idx);
				}
				break;
			}
			case INT: {
				printf("MOV r%d %d\n", right_reg_idx, root->right->val);
				printf("MOV [%d] r%d\n", 4 * idx_of_variable_of_root_of_left_subtree, right_reg_idx);
				break; 
			}
			case ADDSUB:
			case MULDIV:
			case LOGICAL:
			case DISCARDED_CZ_PROCESSED: {printf("MOV [%d] r%d\n", 4*idx_of_variable_of_root_of_left_subtree, right_reg_idx); break; }
			}
		}//ASSIGN完成

		//開始收尾
		root->right->data = DISCARDED_CZ_PROCESSED;
		root->reg_num = root->right->reg_num;
		//不能釋放reg編號，因為可能上面還有ASSIGN或ADDSUB或MULDIV或LOGICAL運算
		return;
	}
}
