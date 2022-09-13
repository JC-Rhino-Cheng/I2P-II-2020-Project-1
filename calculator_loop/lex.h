//lex�γ~: �y�y���ѡC²��ӻ��N�O��ѥy�l����k
#ifndef __LEX__
#define __LEX__

#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN, END, ENDFILE, 
    INT, ID,//INT�O����ª���ơAID�O���ܼƦW��
    ADDSUB, MULDIV, LOGICAL,
    INCDEC, ASSIGN, 
    LPAREN, RPAREN
} TokenSet;

// Test if a token matches the current token 
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

#endif // __LEX__