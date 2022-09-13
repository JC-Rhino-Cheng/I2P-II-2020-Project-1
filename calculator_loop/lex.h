//lex用途: 語句辨識。簡單來說就是拆解句子的文法
#ifndef __LEX__
#define __LEX__

#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN, END, ENDFILE, 
    INT, ID,//INT是指單純的整數，ID是指變數名稱
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
