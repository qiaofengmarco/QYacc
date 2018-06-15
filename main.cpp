#include "QYacc.hpp"

int main()
{
    QYacc Yacc;
    Yacc.GenerateTable();
    Yacc.GenerateRemainingCode();
    system("pause");
    return 0;
}