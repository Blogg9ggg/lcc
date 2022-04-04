//
// Created by blog on 2022/4/4.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define int long long

char *src, *rec;

// registers
int ax;
int *pc, *sp, *bp;

// segments
int *Code, *Stack;
char *Data;

int ibp;
int *symbol_table, *symbol_ptr, *main_ptr;
int token, token_val;

// debug
int line;
int debug;
int *Code_rec;

enum instructions {
    IMM, LEA, JMP, JZ, JNZ, NVAR, DARG, CALL,    // 7
    LI, LC, SI, SC, PUSH,                   // 12
    OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,//28
    RET,
    OPEN, READ, CLOS, PRTF, MALC, FREE, MSET, MCMP, EXIT
};

// fields of symbol_table
enum fields { Token, Hash, Name, Class, Type, Value, GClass, GType, GValue, SymSize };
// types of variables & functions in symbol_table
enum types { CHAR, INT, PTR };
// classes/keywords, Do not support for.
enum {
    // class
    Num = 128, Fun, Sys, Glo, Loc,  //132
    // token
    Id, Char, Int, Enum, If, Else, Return, Sizeof, While,   //141
    // operators in precedence order.
    Assign, Cond, Lor, Land, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge,  //154
    Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

void bug0() {
    char *ch_ptr;

    while (symbol_ptr[Token]) {
        symbol_ptr = symbol_ptr + SymSize;
    }
    token = symbol_ptr[Token] = Id;
    symbol_ptr[Name] = (int) ch_ptr;
}

void bug1() {   // solved
    if(ax == IMM)   ax = *(int *) ax;
    else
    if(ax == PUSH)  ax = *(int *) ax;
}