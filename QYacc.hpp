#ifndef MYQYACC_HPP
#define MYQYACC_HPP

#include "Headers.h"
#include "Preprocess.hpp"
#include "LR_DFA.hpp"

class QYacc
{
public:
    QYacc()
    {
        DFA.construct();
    }
    void GenerateTable()
    {
        DFA.printTable();
    }
    void GenerateRemainingCode()
    {
        ofstream out("./y.tab.c", ios::app);
        cout << "Start generating utility code..." << endl;
        out << "struct Stack {" << endl;
        out << "    int *stack;" << endl;
        out << "    int capacity;" << endl;
        out << "    int top;" << endl;
        out << "};" << endl;
        out << "void StackInit(struct Stack *s, int capacity) {" << endl;
        out << "    s->capacity = capacity;" << endl;
        out << "    s->top = -1;" << endl;
        out << "    s->stack = (int*) malloc(sizeof(int) * capacity);" << endl;
        out << "}" << endl;
        out << "int StackTop(const struct Stack* const s) {" << endl;
        out << "    if (s->top >= 0)" << endl;
        out << "        return s->stack[s->top];" << endl;
        out << "}" << endl;
        out << "void StackPop(struct Stack *s) {" << endl;
        out << "    if (s->top >= 0) {" << endl;
        out << "        s->stack[s->top] = -1;" << endl;
        out << "        s->top--;" << endl;
        out << "    }" << endl;
        out << "}" << endl;
        out << "void StackExtend(struct Stack* s, int capacity) {" << endl;
        out << "    int this_capacity = s->capacity;" << endl;
        out << "    if (this_capacity >= capacity)" << endl;
        out << "        return;" << endl;
        out << "    int *temp = (int*) malloc(sizeof(int) * capacity);" << endl;
        out << "    memcpy(temp, s->stack, sizeof(int) * this_capacity);" << endl;
        out << "    free(s->stack);" << endl;
        out << "    s->stack = (int*) malloc(sizeof(int) * capacity);" << endl;
        out << "    memcpy(s->stack, temp, sizeof(int) * this_capacity);" << endl;
        out << "    s->capacity = capacity;" << endl;
        out << "    free(temp);" << endl;
        out << "    temp = NULL;" << endl;
        out << "}" << endl;
        out << "void StackPush(struct Stack* s, int a) {" << endl;
        out << "    s->top++;" << endl;
        out << "    if (s->top == s->capacity)" << endl;
        out << "        StackExtend(s, s->capacity + 100);" << endl;
        out << "    s->stack[s->top] = a;" << endl;
        out << "}" << endl;
        cout << "Finished generating utility code." << endl;
        cout << "Start generating remaining code..." << endl;
        out << "int main(int argc, char *argv[]) {" << endl;
        out << "    int cur_state, i, j, next_state, action, reduce_pid, A, flag, flag1, flag2;" << endl;
        out << "    struct Stack s;" << endl;
        out << "    StackInit(&s, 100);" << endl;
        out << "    table_action_init();" << endl;
        out << "    if ((argc < 2) || (argc > 3)) {" << endl;
        out << "        printf(\"Parameters Error.\\n\");" << endl;
        out << "        return 1;" << endl;
        out << "    }" << endl;
        out << "    else if (argc == 2)" << endl;
        out << "        yyout = fopen(\"./Result.txt\", \"w\");" << endl;
        out << "    else" << endl;
        out << "        yyout = fopen(argv[2], \"w\");" << endl;
        out << "    yyin = fopen(argv[1], \"r\");" << endl;
        out << "    fprintf(yyout, \"Line 1:\");" << endl;
        out << "    next = fgetc(yyin);" << endl;
        out << "    yyleng = 0;" << endl;
        out << "    cur_state = 0;" << endl;
        out << "    StackPush(&s, cur_state);" << endl;
        out << "    do {" << endl;
        out << "        if (yyleng == 0)" << endl;
        out << "            action = yylex(next);" << endl;
        out << "        if (action == -1) {" << endl;
        out << "            yyleng = 1;" << endl;
        out << "            yytext[0] = \'$\';" << endl;
        out << "            yytext[1] = \'\\0\';" << endl;
        out << "            action = \'$\';" << endl;
        out << "            next = \'$\';" << endl;
        out << "        }" << endl;
        out << "        flag1 = 0;" << endl;
        out << "        for (i = 0; i < parser_action_count[cur_state]; i++)" << endl;
        out << "            if (action == parser_action[cur_state][i]) {" << endl;
        out << "                flag1 = 1;" << endl;
        out << "                break;" << endl;
        out << "            }" << endl;
        out << "        if (flag1) {" << endl;
        out << "            next_state = parser_table[cur_state][i];" << endl;
        out << "            if (next_state < parser_state_num) {" << endl;
        out << "                cur_state = next_state;" << endl;
        out << "                StackPush(&s, cur_state);" << endl;
        out << "                yytext[0] = \'\\0\';" << endl;
        out << "                yyleng = 0;" << endl;
        out << "            }" << endl;
        out << "            else {" << endl;
        out << "                reduce_pid = next_state - parser_state_num;" << endl;
        out << "                for (j = 0; j < production_label_count[reduce_pid]; j++)" << endl;
        out << "                    StackPop(&s);" << endl;
        out << "                cur_state = StackTop(&s);" << endl;
        out << "                A = production_left[reduce_pid];" << endl;
        out << "                fprintf(yyout, \"\\n%s\", productions[reduce_pid]);" << endl;
        out << "                flag2 = 0;" << endl;
        out << "                for (j = 0; j < parser_action_count[cur_state]; j++)" << endl;
        out << "                    if (A == parser_action[cur_state][j]) {" << endl;
        out << "                        flag2 = 1;" << endl;
        out << "                        break;" << endl;
        out << "                    }" << endl;
        out << "                if (flag2) {" << endl;
        out << "                    cur_state = parser_table[cur_state][j];" << endl;
        out << "                    StackPush(&s, cur_state);" << endl;
        out << "                }" << endl;
        out << "                else {" << endl;
        out << "                    yyerror(\"Error\");" << endl;
        out << "                    exit(1);" << endl;
        out << "                }" << endl;
        out << "            }" << endl;
        out << "        }" << endl;
        out << "        else {" << endl;
        out << "            if (next == \'$\')" << endl;
        out << "                break;" << endl;
        out << "            yyerror(\"Error\");" << endl;
        out << "            exit(1);" << endl;
        out << "        }" << endl;
        out << "    } while (1);" << endl;
        out << "    fclose(yyin);" << endl;
        out << "    fclose(yyout);" << endl;
        out << "    return 0;" << endl;
        out << "}" << endl;
        cout << "Finished generating remaining code." << endl;
        out.close();
    }
private:
    LR_DFA DFA; 
};


#endif