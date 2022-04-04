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
    RET, OPEN, READ, CLOS, PRTF, MALC, FREE, MSET, MCMP, EXIT
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


// ****************************************************************************************************************
// Debug
// ****************************************************************************************************************
void error(char *s) {
    printf("%s\n", s);
    exit(-1);
}
void debug_print_token() {
    printf("Token = %lld, Value = %lld\n", symbol_ptr[Token], symbol_ptr[Value]);
    printf("Name = %s\n", symbol_ptr[Name]);
}
void debug_print_symbol_table() {
    int *tmp_ptr = symbol_table;
    while (tmp_ptr[Token]) {
        printf("Token: %lld\n", tmp_ptr[Token]);
        printf("Name: %.8s\n", tmp_ptr[Name]);
        printf("Value: %lld\n", tmp_ptr[Value]);
        printf("Type: %lld\n\n", tmp_ptr[Type]);
        tmp_ptr = tmp_ptr + SymSize;
    }
}


// ****************************************************************************************************************
// virtual machine
// ****************************************************************************************************************
int VMachine(int argc, char **argv) {
    int op;
    int *tmp;

    sp = (int*)((int)Stack + 256 * 1024);
    bp = sp;
    *--sp = EXIT;
    *--sp = PUSH;
    tmp = sp;
    *--sp = argc;
    *--sp = (int)argv;
    *--sp = (int)tmp;
    if(!(pc = (int*)main_ptr[Value]))
        error("Main function is not defined.");

    while (1) {
        op = *pc++;

        // save & load instructions
        if (op == IMM)
            ax = *pc++;
        else if (op == LEA) {
            ax = (int) (bp + *pc);
            pc++;
        }
        else if (op == LI)
            ax = *(int *) ax;
        else if (op == LC) ax = *(char *) ax;
        else if (op == SI) {
            *(int *) *sp = ax;
            sp++;
        }
        else if (op == SC) {
            *(char *) *sp = ax;
            sp++;
        }
        else if (op == PUSH) {
            sp--;
            *sp = ax;
        }

            // mathematical instructions
        else if (op == OR) {
            ax = *sp | ax;
            sp++;
        }
        else if (op == XOR) {
            ax = *sp ^ ax;
            sp++;
        }
        else if (op == AND) {
            ax = *sp & ax;
            sp++;
        }
        else if (op == EQ) {
            ax = *sp == ax;
            sp++;
        }
        else if (op == NE) {
            ax = *sp != ax;
            sp++;
        }
        else if (op == LT) {
            ax = *sp < ax;
            sp++;
        }
        else if (op == GT) {
            ax = *sp > ax;
            sp++;
        }
        else if (op == LE) {
            ax = *sp <= ax;
            sp++;
        }
        else if (op == GE) {
            ax = *sp >= ax;
            sp++;
        }
        else if (op == SHL) {
            ax = *sp << ax;
            sp++;
        }
        else if (op == SHR) {
            ax = *sp >> ax;
            sp++;
        }
        else if (op == ADD) {
            ax = *sp + ax;
            sp++;
        }
        else if (op == SUB) {
            ax = *sp - ax;
            sp++;
        }
        else if (op == MUL) {
            ax = *sp * ax;
            sp++;
        }
        else if (op == DIV) {
            ax = *sp / ax;
            sp++;
        }
        else if (op == MOD) {
            ax = *sp % ax;
            sp++;
        }

            // jump instructions
        else if (op == JMP) pc = (int *) *pc;
        else if (op == JZ) pc = ax ? pc + 1 : (int *) *pc;
        else if (op == JNZ) pc = ax ? (int *) *pc : pc + 1;

            // instructions for funtion call
        else if (op == CALL) {
            sp--;
            *sp = (int) (pc + 1);
            pc = (int *) *pc;
        } else if (op == NVAR) {
            sp--;
            *sp = (int) bp;
            bp = sp;
            sp = sp - *pc;
            pc++;
        } else if (op == DARG) {
            sp = sp + *pc;
            pc++;
        } else if (op == RET) {
            sp = bp;
            bp = (int *) *sp;
            sp++;
            pc = (int *) *sp;
            sp++;
        }

            // end for call function.
            // native call
        else if (op == OPEN) { ax = open((char *) sp[1], sp[0]); }
        else if (op == CLOS) { ax = close(*sp); }
        else if (op == READ) { ax = read(sp[2], (char *) sp[1], *sp); }
        else if (op == PRTF) {
            tmp = sp + pc[1] - 1;
            ax = printf((char *) tmp[0], tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5]);
        }
        else if (op == MALC) { ax = (int) malloc(*sp); }
        else if (op == FREE) { free((void *) *sp); }
        else if (op == MSET) { ax = (int) memset((char *) sp[2], sp[1], *sp); }
        else if (op == MCMP) { ax = memcmp((char *) sp[2], (char *) sp[1], *sp); }
        else if (op == EXIT) {
            printf("exit(%lld)\n", *sp);
            return *sp;
        }
    }
    return 0;
}

// ****************************************************************************************************************
// "词法分析"
// Completed: 100%
// ****************************************************************************************************************
// output: symbol_ptr, token, (token_val)
void tokenize() {
    char *ch_ptr;
    while (token = *src++) {
        if (token == '\n') {
            if (debug) {
                printf("%lld: %.*s", line, src - rec, rec);
                rec = src;
                while (Code_rec < Code) {
                    printf("%8.4s", &"IMM ,LEA ,JMP ,JZ  ,JNZ ,NVAR,DARG,CALL,"
                                     "LI  ,LC  ,SI  ,SC  ,PUSH,"
                                     "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                                     "RET ,OPEN,READ,CLOS,PRTF,MALC,FREE,MSET,MCMP,EXIT"[*++Code_rec * 5]);
                    if (*Code_rec <= CALL) printf(" %lld\n", *++Code_rec); else printf("\n");
                }
            }

            line++;
        } else if (token == '#') {
            while (*src && *src != '\n') src++;
        }
            // "只有 Id 对象才会更新 symbol_ptr"
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || token == '_') {
            ch_ptr = src - 1;
            // "先将 token 当成 hash 来用"
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || *src == '_' ||
                   (*src >= '0' && *src <= '9')) {
                token = token * 147 + *src;
                src++;
            }
            token = (token << 6) + src - ch_ptr;
            symbol_ptr = symbol_table;
            while (symbol_ptr[Token]) {

                if (token == symbol_ptr[Hash] && !memcmp((char *) symbol_ptr[Name], ch_ptr, src - ch_ptr)) {
                    token = symbol_ptr[Token];

                    return;
                }
                symbol_ptr = symbol_ptr + SymSize;
            }
            symbol_ptr[Hash] = token;
            token = symbol_ptr[Token] = Id;
            symbol_ptr[Name] = (int) ch_ptr;
            return;
        } else if (token >= '0' && token <= '9') {
            token_val = token - '0';
            if (token == '0' && (*src == 'x' || *src == 'X')) {
                src++;
                while ((*src >= '0' && *src <= '9') || (*src >= 'a' && *src <= 'f') || (*src >= 'A' && *src <= 'F')) {
                    token_val = token_val * 16 + (*src & 0xF) + ((*src >= '0' && *src <= '9') ? 0 : 9);
                    src++;
                }
            }
                // try BIN
            else if (token == '0' && (*src == 'b' || *src == 'B')) {
                src++;
                while (*src >= '0' && *src < '2') {
                    token_val = token_val * 2 + *src - '0';
                    src++;
                }
            } else {
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val * 10 + *src - '0';
                    src++;
                }
            }
            token = Num;
            return;
        } else if (token == '\'' || token == '\"') {
            ch_ptr = Data;
            while (*src && *src != token) {
                token_val = *src++;
                if (token_val == '\\') {
                    if (*src == 'n') {
                        token_val = '\n';
                        src++;
                    }
                }
                if (token == '\"')
                    *Data++ = token_val;
            }
            src++;
            if (token == '\'')
                token = Num;
            else
                token_val = (int) ch_ptr;

            return;
        } else if (token == '/') {
            if (*src == '/') {
                src++;
                while (*src && *src != '\n') src++;
            } else {
                token = Div;
                return;
            }
        } else if (token == '=') {
            if (*src == '=') {
                src++;
                token = Eq;
            } else
                token = Assign;
            return;
        } else if (token == '+') {
            if (*src == '+') {
                src++;
                token = Inc;
            }
            else token = Add;
            return;
        }
        else if (token == '-') {
            if (*src == '-') {
                src++;
                token = Dec;
            }
            else token = Sub;
            return;
        }
        else if (token == '!') {
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        }
        else if (token == '<') {
            if (*src == '=') {
                src++;
                token = Le;
            }
            else if (*src == '<') {
                src++;
                token = Shl;
            }
            else token = Lt;
            return;
        }
        else if (token == '>') {
            if (*src == '=') {
                src++;
                token = Ge;
            }
            else if (*src == '>') {
                src++;
                token = Shr;
            }
            else token = Gt;
            return;
        }
        else if (token == '|') {
            if (*src == '|') {
                src++;
                token = Lor;
            }
            else token = Or;
            return;
        }
        else if (token == '&') {
            if (*src == '&') {
                src++;
                token = Land;
            }
            else token = And;
            return;
        }
        else if (token == '^') {
            token = Xor;
            return;
        }
        else if (token == '%') {
            token = Mod;
            return;
        }
        else if (token == '*') {
            token = Mul;
            return;
        }
        else if (token == '[') {
//            debug_print_symbol_table();
            token = Brak;
            return;
        }
        else if (token == '?') {
            token = Cond;
            return;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' ||
                 token == ']' || token == ',' || token == ':')
            return;
    }
}



// ****************************************************************************************************************
// "语法分析"
// ****************************************************************************************************************
// TODO: 把这些 assert 的命名整一下
void assert(int expec) {
//    debug_print_token();
    if(token != expec) {
        printf("Line #%lld: expected token: %lld(%c), obtained token: %lld(%c).\n", line, expec, (char)expec, token, (char)token);
        exit(-1);
    }
}
void t2assert(int expec) {
    tokenize();
    assert(expec);
}


// Char -> CHAR, Int -> INT
int token2type() {
    if(token == Char)
        return CHAR;
    else if(token == Int)
        return INT;
    else{
        printf("Line #%lld: expected token: Char/Int, obtained token: %lld(%c).\n", line, token, (char)token);
        exit(-1);
    }
}


void parse_enum() {
    int num = 0;
    while (token != '}') {
        t2assert(Id);
        tokenize();
        if (token == Assign) {
            t2assert(Num);
            num = token_val;
            tokenize();
        }
        // TODO: check (token == '}' || token == ',')
        symbol_ptr[Class] = Num;
        symbol_ptr[Type] = INT;
        symbol_ptr[Value] = num++;
    }
}


void cover_global() {
    symbol_ptr[GClass] = symbol_ptr[Class];
    symbol_ptr[GType] = symbol_ptr[Type];
    symbol_ptr[GValue] = symbol_ptr[Value];
}
void recover_global() {
    symbol_ptr[Class] = symbol_ptr[GClass];
    symbol_ptr[Type] = symbol_ptr[GType];
    symbol_ptr[Value] = symbol_ptr[GValue];
}


void parse_param() {
    int type, i = 0;

    tokenize();
    while (token != ')') {
        type = token2type();
        tokenize();
        while (token == Mul) {
            type = type + PTR;
            tokenize();
        }
        assert(Id);
        if (symbol_ptr[Class] == Glo)
            cover_global();

        symbol_ptr[Class] = Loc;
        symbol_ptr[Type] = type;
        symbol_ptr[Value] = i++;

        // ',' or ')'
        tokenize();
        if (token == ',')
            tokenize();
        // TODO: assert(token != ')')
    }
    // 用 ibp - symbol_ptr[Value] 定位函数的参数
    ibp = ++i;
}


// have 1st token
// get next token
int parse_expr(int precd) {
    int type, new_type, i;
    int *tmp_ptr;
    if (token == Num) {
        *++Code = IMM;
        *++Code = token_val;

        type = INT;
        tokenize();
    } else if (token == '\"') {
        *++Code = IMM;
        *++Code = token_val;
        while (token == '\"')
            tokenize();
        *Data++ = 0;
        Data = ((int) (Data + 7) & (-8));

        type = PTR;
    } else if (token == Sizeof) {
        // sizeof([Id]) or sizeof(int[*] / char[*])
        t2assert('(');
        tokenize();
        new_type = token2type();
        tokenize();
        while (token == Mul) {
            new_type = new_type + PTR;
            tokenize();
        }
        assert(')');
        *++Code = IMM;
        *++Code = new_type == CHAR ? 1 : 8;

        type = INT;
        tokenize();
    } else if (token == Id) {
        tmp_ptr = symbol_ptr;
        if (symbol_ptr[Class] == Num) {
            *++Code = IMM;
            *++Code = token_val;
        } else if (symbol_ptr[Class] == Fun) {
            i = 0;
            t2assert('(');
            tokenize();
            while (token != ')') {
                parse_expr(Assign);
                *++Code = PUSH;
                i++;
                if (token == ',')
                    tokenize();
            }
            *++Code = CALL;
            *++Code = tmp_ptr[Value];
            if (i) {
                *++Code = DARG;
                *++Code = i;
            }
        } else if (symbol_ptr[Class] == Loc) {
            *++Code = LEA;
            *++Code = ibp - symbol_ptr[Value];
        } else if (symbol_ptr[Class] == Glo) {
            *++Code = LEA;
            *++Code = symbol_ptr[Value];
        } else if (symbol_ptr[Class] == Sys) {
            i = 0;
            t2assert('(');
            tokenize();
            while (token != ')') {
                parse_expr(Assign);
                *++Code = PUSH;
                i++;
                if (token == ',')
                    tokenize();
            }
            *++Code = tmp_ptr[Value];
            if (i) {
                *++Code = DARG;
                *++Code = i;
            }
        } else {
            printf("Line #%lld: invalid variable.\n", line);
            exit(-1);
        }

        type = tmp_ptr[Type];
        if (tmp_ptr[Class] == Loc || tmp_ptr[Class] == Glo)
            *++Code = (type == CHAR) ? LC : LI;
        tokenize();
    } else if (token == Inc || token == Dec) {
        tokenize();
        new_type = parse_expr(Inc);
        if (*Code == LC) {
            *Code = PUSH;
            *++Code = LC;
        } else if (*Code == LI) {
            *Code = PUSH;
            *++Code = LI;
        } else {
            printf("Line #%lld: invalid ++ or --.\n", line);
            exit(-1);
        }
        *Code = PUSH;
        *++Code = IMM;
        *++Code = (new_type > PTR) ? 8 : 1;
        *++Code = (type == Inc) ? ADD : SUB;
        *++Code = (new_type == CHAR) ? SC : SI;
        type = new_type;
    } else if (token == Add || token == Sub) {
        type = token;
        tokenize();
        new_type = parse_expr(Inc);
        // TODO: assert(new_type == INT)
        if (type == Sub) {
            *++Code = PUSH;
            *++Code = IMM;
            *++Code = -1;
            *++Code = MUL;
        }
        type = INT;
    } else if (token == '~') {
        tokenize();
        new_type = parse_expr(Inc);
        // TODO: assert(new_type == INT)
        *++Code = PUSH;
        *++Code = IMM;
        *++Code = -1;
        *++Code = XOR;
        type = INT;
    } else if (token == '!') {
        tokenize();
        new_type = parse_expr(Inc);
        // TODO: assert(new_type == INT)
        *++Code = PUSH;
        *++Code = IMM;
        *++Code = 0;
        *++Code = EQ;
        type = INT;
    } else if (token == Mul) {
        tokenize();
        type = parse_expr(Inc);
        // TODO: assert(变量)
        if (type >= PTR)
            type -= PTR;
        else {
            printf("Line #%lld: invalid dereference.\n", line);
            exit(-1);
        }
        *++Code = (type == CHAR) ? LC : LI;
    } else if (token == And) {
        tokenize();
        type = parse_expr(Inc);
        if (*Code == LC || *Code == LI)
            Code--; // rollback load by addr
        else {
            printf("Line #%lld: invalid reference.\n", line);
            exit(-1);
        }
        type = type + PTR;
    } else if (token == '(') {
        tokenize();
        // "类型转换"
        if (token == Char || token == Int) {
            type = token2type();
            tokenize();
            while (token == Mul) {
                type = type + PTR;
                tokenize();
            }
            assert(')');
            tokenize();
            new_type = parse_expr(Inc);
        } else {
            type = parse_expr(Assign);
            assert(')');
            tokenize();
        }

    }

    while (token >= precd) {
        if (token == Assign) {
            if (*Code == LC || *Code == LI) *Code = PUSH;
            else {
                printf("Line #%lld: invalid assignment.\n", line);
                exit(-1);
            }
            tokenize();
            new_type = parse_expr(Assign);
            *++Code = (new_type == CHAR) ? SC : SI;
        } else if (token == Add) {
            *++Code = PUSH;
            tokenize();
            // "由于同等级的运算符遵循从左到右的顺序，故这里的 '+' 相对于后面的 '+' 优先级是更高的, 所以递归的时候给他适当地升级"
            new_type = parse_expr(Mul);
            if (type > PTR) {
                *++Code = PUSH;
                *++Code = IMM;
                *++Code = 8;
                *++Code = MUL;
            }
            *++Code = ADD;
        } else if (token == Sub) {
            *++Code = PUSH;
            tokenize();
            new_type = parse_expr(Mul);
            if (new_type == type && type > PTR) {
                *++Code = SUB;

                *++Code = PUSH;
                *++Code = IMM;
                *++Code = 8;
                *++Code = DIV;

                type = INT;
            } else if (type > PTR) {
                // assert: new_type == INT
                *++Code = PUSH;
                *++Code = IMM;
                *++Code = 8;
                *++Code = MUL;

                *++Code = SUB;
            } else    // assert: type == INT && new_type == INT
                *++Code = SUB;
        } else if (token == Cond) {
            tokenize();
            *++Code = JZ;
            type = ++Code;
            new_type = parse_expr(Cond);  // parse_expr(Assign)?
            assert(':');
            *++Code = JMP;
            new_type = ++Code;
            *(int *) type = (int) (Code + 1);
            tokenize();
            type = parse_expr(Cond);
            *(int *) new_type = (int) (Code + 1);
        } else if (token == Lor) {
            *++Code = JNZ;
            new_type = ++Code;

            tokenize();
            type = parse_expr(Land);

            *(int *) new_type = (int) (Code + 1);
            type = INT;
        } else if (token == Land) {
            *++Code = JZ;
            new_type = ++Code;

            tokenize();
            type = parse_expr(Or);

            *(int *) new_type = (int) (Code + 1);
            type = INT;
        } else if (token == Or) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Xor);
            *++Code = OR;
            type = INT;
        } else if (token == Xor) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(And);
            *++Code = XOR;
            type = INT;
        } else if (token == And) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Eq);
            *++Code = AND;
            type = INT;
        } else if (token == Eq) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Lt);
            *++Code = EQ;
            type = INT;
        } else if (token == Ne) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Lt);
            *++Code = NE;
            type = INT;
        } else if (token == Lt) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Shl);
            *++Code = LT;
            type = INT;
        } else if (token == Gt) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Shl);
            *++Code = GT;
            type = INT;
        } else if (token == Le) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Shl);
            *++Code = LE;
            type = INT;
        } else if (token == Ge) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Shl);
            *++Code = GE;
            type = INT;
        } else if (token == Shl) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Add);
            *++Code = SHL;
            type = INT;
        } else if (token == Shr) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Add);
            *++Code = SHR;
            type = INT;
        } else if (token == Mul) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Inc);
            *++Code = MUL;
            type = INT;
        } else if (token == Div) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Inc);
            *++Code = DIV;
            type = INT;
        } else if (token == Mod) {
            *++Code = PUSH;
            tokenize();
            type = parse_expr(Inc);
            *++Code = MOD;
            type = INT;
        } else if (token == Inc || token == Dec) { // high level!
            if (*Code == LC) {
                *Code = PUSH;
                *++Code = LC;
            } else if (*Code == LI) {
                *Code = PUSH;
                *++Code = LI;
            } else {
                printf("Line #%lld: invalid ++ or --.\n", line);
                exit(-1);
            }
            *++Code = PUSH;
            *++Code = IMM;
            *++Code = (type > PTR) ? 8 : 1;
            *++Code = (token == Inc) ? ADD : SUB;
            *++Code = (type == CHAR) ? SC : SI;
            *++Code = PUSH;
            *++Code = IMM;
            *++Code = (type > PTR) ? 8 : 1;
            *++Code = (token == Inc) ? SUB : ADD;
            tokenize();
        } else if (token == Brak) {
            if (type < PTR) {
                printf("Line #%lld: invalid index op.\n", line);
                exit(-1);
            }
            *++Code = PUSH;
            tokenize();
            new_type = parse_expr(Assign);
            assert(']');
            *++Code = IMM;
            *++Code = (type == PTR) ? 1 : 8;
            *++Code = MUL;
            *++Code = ADD;
            type = type - PTR;
            *++Code = (type == CHAR) ? LC : LI;
            tokenize();
        }
    }

    return type;
}


// have 1st token
// get next token
void parse_stmt() {
    int *addr1, *addr2;
    if (token == If) {
        t2assert('(');
        tokenize();
        parse_expr(Assign);
        assert(')');

        *++Code = JZ;
        addr1 = ++Code;
        tokenize();
        parse_stmt();
        if(token == ';')    // 临时
            tokenize();
        if (token == Else) {
            *++Code = JMP;
            addr2 = ++Code;
            *addr1 = Code + 1;
            tokenize();
            parse_stmt();
            *addr2 = Code + 1;
        } else {
            *addr1 = Code + 1;
        }
    } else if (token == While) {
        addr1 = Code + 1;
        t2assert('(');
        tokenize();
        parse_expr(Assign);
        assert(')');

        *++Code = JZ;
        addr2 = ++Code;
        tokenize();
        if(token == '{')
            tokenize();
        parse_stmt();
        *++Code = JMP;
        *++Code = addr1;
        *addr2 = Code + 1;
    } else if (token == Return) {
        tokenize();
        while (token != ';') parse_expr(Assign);
        *++Code = RET;
    } else if (token == '{') {
        tokenize();
        while (token != '}') {
            parse_stmt();
            tokenize();
        }

        tokenize();
    } else if (token == ';')
        tokenize();
    else
        parse_expr(Assign);

//    if(token == ';')
//        tokenize();
}


void parse_fun() {
    int type;
    int i = ibp;
    tokenize();
    while (token == Char || token == Int) {
        type = token2type();

        while (token != ';') {
            tokenize();
            while (token == Mul) {
                type = type + PTR;
                tokenize();
            }
            assert(Id);

            if (symbol_ptr[Class] == Glo)
                cover_global();

            symbol_ptr[Class] = Loc;
            symbol_ptr[Type] = type;
            symbol_ptr[Value] = ++i;

            tokenize();
            if(token == ',')
                tokenize();
        }
        tokenize();
    }

    *++Code = NVAR;
    *++Code = (i - ibp);
    while (token != '}') parse_stmt();
    if (*Code != RET) *++Code = RET;

    symbol_ptr = symbol_table;
    while (symbol_ptr[Token]) {
        if (symbol_ptr[Class] == Loc)
            recover_global();
        symbol_ptr = symbol_ptr + SymSize;
    }
}


void parse() {
    int base_type, type;
    token = 1;
    line = 1;
    while (token) {
        tokenize();
        if (token == Enum) {
            tokenize();
            if (token == Id)
                tokenize();
            assert('{');
            parse_enum();
            assert('}');
        } else if (token == Char || token == Int) {
            base_type = token2type();
            tokenize();
            while (token != ';' && token != '}') {
                type = base_type;
                while (token == Mul) {
                    type = type + PTR;
                    tokenize();
                }
                assert(Id);
                symbol_ptr[Type] = type;
                tokenize();
                // ”全局变量“
                if (token == ',' || token == ';') {
                    symbol_ptr[Class] = Glo;
                    symbol_ptr[Value] = (int) Data;
                    Data = Data + 8;
                    if (token == ',')
                        tokenize();
                } else if (token == '(') {
                    symbol_ptr[Class] = Fun;
                    symbol_ptr[Value] = (int) (Code + 1);
                    parse_param();
                    assert(')');
                    t2assert('{');
                    parse_fun();
                }
            }
        }
    }
    // TODO: else {}
}


void keyword() {
    int i;
    src = "char int enum if else return sizeof while "
          "open read close printf malloc free memset memcmp exit void main";
    // add keywords to symbol table
    i = Char;
    while (i <= While) {
        tokenize();
        symbol_ptr[Token] = i++;
    }
    // add Native CALL to symbol table
    i = OPEN;
    while (i <= EXIT) {
        tokenize();
        symbol_ptr[Class] = Sys;
        symbol_ptr[Type] = INT;
        symbol_ptr[Value] = i++;
    }
    tokenize();
    symbol_ptr[Token] = Char; // handle void type
    tokenize();
    main_ptr = symbol_ptr; // keep track of main

    symbol_ptr = symbol_table;
    while (symbol_ptr[Token]) {
        symbol_ptr = symbol_ptr + SymSize;
    }
}


int main(int argc, char **argv) {
    int fd;
    int poolsz = 256 * 1024;

    debug = 0;
    if ((fd = open(*(argv + 1), 0)) < 0)
        error("Could not open source code.");

    Code_rec = Code = malloc(poolsz);
    Stack = malloc(poolsz);
    memset(Stack, 0, poolsz);
    Data = malloc(poolsz);
    memset(Data, 0, poolsz);
    symbol_ptr = symbol_table = malloc(poolsz);
    memset(symbol_table, 0, poolsz);

    src = rec = malloc(poolsz);
    keyword();

    read(fd, rec, poolsz - 1);
    src = rec;

    parse();

    VMachine(--argc, ++argv);
    return 0;
}
