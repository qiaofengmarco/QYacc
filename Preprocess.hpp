#ifndef MY_PREPROCESSOR_HPP
#define MY_PREPROCESSOR_HPP

#include "Headers.h"

struct production
{
	string value;//拆开后产生式的字符串值
	int left;//拆开后产生式左部对应的整数值
	vector<int> right;//拆开后产生式右部各项对应的整数值
};

map<string,int> TokenAndLabel;//存token、原始产生式左部和它们对应的整数值
vector<string> originalProduction;//存储原始产生式
vector<production> productions;//存储拆开后的产生式
unordered_map<int, int> firstPos;//原始产生式对应拆开后产生式第一次出现的位置
unordered_map<int, bool> nullable;//原始产生式能否推出空
int start_id = 0;//起始产生式左部对应的整数值
string start_str = "";//起始产生式左部对应的字符串值（%start语句后面的字符串，若没有%start语句则为空）
int tokenAmount = 0;//token个数
int startPos = 130;//标识符开始出现的位置

extern void mapTokenProcess();//读取y.tab.h文件，将token和其对应的整数值存到TokenAndLabel中
extern void yProcess();//处理.y文件，包括找到起始产生式、原始产生式的存储、产生式左部及其对应整数值的存储以及yy.tab.c文件的创建等
extern void pdProcess();//拆分原始产生式并将拆开后产生式相关信息存入productions中，判断原始产生式左部能否推出空

void mapTokenProcess()
{
    ifstream file;
    file.open("./y.tab.h");

    string str = "";
    int i = 0;
    while (i<2)
    {
        getline(file, str);
        if (str[0] == '#')
            i++;
    }

    string tempStr = "";
    string intStr = "";
    int tempInt;
    getline(file, str);
    while (str != "#endif")
    {
        if (str[0] == '#')
        {
            tempStr = "";
            intStr = "";

            int pos;

            for (pos = 8; str[pos] != ' '; pos++)
                tempStr += str[pos];

            for (pos++; str[pos] != '\0'; pos++)
                intStr += str[pos];
            tempInt = atoi(intStr.c_str());

            TokenAndLabel.insert(pair<string, int>(tempStr, tempInt));
        }
        else
        {
        }

        getline(file, str);
    }

    tokenAmount = TokenAndLabel.size() + 130;
    startPos = tokenAmount;
    file.close();
}

void yProcess()
{
    ifstream inFile;
    ofstream outFile;

    inFile.open("./yacc.y");
    outFile.open("./y.tab.c");

    string str = "";

    getline(inFile, str);
    while (str != "%%")
    {
        if (str.length() >= 8)
        {
            string temp = "";
            for (int i = 1; i < 6; i++)
                temp += str[i];
            if (temp == "start")
            {
                int j = 7;
                while (str[j] == ' ' || str[j] == '\t' || str[j] == '\n')
                    j++;
                while (str[j] != ' ' && str[j] != '\t' && str[j] != '\n' && str[j] != '\0')
                {
                    start_str += str[j];
                    j++;
                }
            }
        }

        getline(inFile, str);
    }

    string tempStr = "";
    getline(inFile, str);
    int pos = startPos;
    while (str != "%%")
    {
        if ((str[0] >= 'a'&&str[0] <= 'z') || (str[0] >= 'A'&&str[0] <= 'Z'))
        {
            tempStr = "";
            for (int i = 0; str[i] != '\t' && str[i] != ' ' && str[i] != '\0'; i++)
                tempStr += str[i];
            TokenAndLabel.insert(pair<string, int>(tempStr, pos));

            pos++;
            tempStr = "";
            while (true)
            {
                tempStr += str;
                tempStr += '\n';

                getline(inFile, str);
                int count;
                for (count = 0; (str[count] == ' ' || str[count] == '\t'); count++);
                if (str[count] == ';')
                {
                    tempStr += "\t;";
                    break;
                }
            }
            originalProduction.push_back(tempStr);
        }
        else
        {
        }

        getline(inFile, str);
    }

    if (start_str.empty())
        start_id = startPos;
    else
        start_id = TokenAndLabel.at(start_str);

    while (!inFile.eof())
    {
        getline(inFile, str);
        outFile << str << "\n";
    }

    inFile.close();
    outFile.close();
}

void pdProcess()
{
    production tempPd;
    string str;
    string leftStr = "";
    string tempStr = "";
    string tempValue = "";
    int pos = 0;
    bool emptyFlag = false;

    int amount = originalProduction.size();
    for (int count = 0; count < amount; count++)
    {
        firstPos.insert(pair<int, int>(count + startPos, productions.size()));

        pos = 0;
        leftStr = "";
        str = originalProduction[count];
        emptyFlag = false;

        while (str[pos] != ' '&&str[pos] != '\t'&&str[pos] != '\n')
        {
            leftStr += str[pos];
            pos++;
        }
        tempPd.left = TokenAndLabel.at(leftStr);

        while (str[pos] != '\n')
            pos++;
        pos++;

        while (str[pos] != '\0')
        {
            tempValue = leftStr + " :";
            vector<int> tempRight;

            while (str[pos] != '\n')
            {
                tempStr = "";
                while (str[pos] == ' ' || str[pos] == '\t' || str[pos] == ':' || str[pos] == '|')
                {
                    if (str[pos] == ':' || str[pos] == '|')
                    {
                        bool empty = true;
                        for (int i = pos + 1; str[i] != '\n'; i++)
                        {
                            if (str[i] != ' '&&str[i] != '\t')
                            {
                                empty = false;
                                break;
                            }
                        }

                        if (empty)
                            emptyFlag = true;
                    }
                    pos++;
                }

                if (str[pos] == ';')
                    break;

                while (str[pos] != ' ' && str[pos] != '\t' && str[pos] != '\n')
                {
                    tempStr += str[pos];
                    pos++;
                }
                tempValue = tempValue + " " + tempStr;

                if (tempStr == "")
                    tempRight.push_back(129);
                else if (tempStr[0] != '\'')
                    tempRight.push_back(TokenAndLabel.at(tempStr));
                else
                    tempRight.push_back((int) tempStr[1]);
            }

            if (str[pos] == ';')
            {
                pos++;
                continue;
            }

            tempPd.value = tempValue;
            tempPd.right.assign(tempRight.begin(), tempRight.end());
            productions.push_back(tempPd);
            pos++;
        }

        nullable.insert(pair<int, bool>(count + startPos, emptyFlag));
    }
}

#endif