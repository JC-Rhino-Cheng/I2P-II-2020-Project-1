#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"
#pragma warning (disable: 4996)

int evaluateTree(BTNode *root) {
	int retval = 0, lv = 0, rv = 0;//lv�N��left_value�Arv�N��right_value

	if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);//getval��J�ܼƪ��W��(�r��)�A�æ^�Ǧ��ܼƥثe�x�s����
                break;
            case INT:
                retval = atoi(root->lexeme);//�p�G�onode�������OINT�A�N�������Ѧr�ꦨ���ƭ�
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);//����k��l�𪺭Ⱥ�X�ӡA�����x�s��rv�̭�
                retval = setval(root->left->lexeme, rv);//�]���OASSIGN�A�ҥH�Φ�������: xxxx = 3�C�ҥH�z�Lsetval�禡�A������xxxx�o���ܼƦW�r���L�btable���ĴX�ӽs���A�������x�s���L�C����Areturn�P�@�ӭȡC
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
					if (rv == 0) error(DIVZERO);
                    retval = lv / rv;
                }
                break;
            default:
                retval = 0;
        }
    }

    return retval;
}


void printPrefix(BTNode *root) {
	if (root != NULL) {
		printf("%s ", root->lexeme);
		printPrefix(root->left);
		printPrefix(root->right);
    }

	return;
}
