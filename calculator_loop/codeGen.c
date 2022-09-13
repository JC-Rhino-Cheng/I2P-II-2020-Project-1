#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"
#pragma warning (disable: 4996)

int evaluateTree(BTNode *root) {
	int retval = 0, lv = 0, rv = 0;//lv代表left_value，rv代表right_value

	if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);//getval輸入變數的名稱(字串)，並回傳此變數目前儲存的值
                break;
            case INT:
                retval = atoi(root->lexeme);//如果這node的類型是INT，就直接辨識字串成為數值
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);//先把右邊子樹的值算出來，之後儲存到rv裡面
                retval = setval(root->left->lexeme, rv);//因為是ASSIGN，所以形式類似於: xxxx = 3。所以透過setval函式，先找到用xxxx這個變數名字找到他在table的第幾個編號，之後把值儲存給他。之後再return同一個值。
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
