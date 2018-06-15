#ifndef MY_PREPROCESSOR_HPP
#define MY_PREPROCESSOR_HPP

#include "Headers.h"

struct production
{
	string value;//�𿪺����ʽ���ַ���ֵ
	int left;//�𿪺����ʽ�󲿶�Ӧ������ֵ
	vector<int> right;//�𿪺����ʽ�Ҳ������Ӧ������ֵ
};

map<string,int> TokenAndLabel;//��token��ԭʼ����ʽ�󲿺����Ƕ�Ӧ������ֵ
vector<string> originalProduction;//�洢ԭʼ����ʽ
vector<production> productions;//�洢�𿪺�Ĳ���ʽ
unordered_map<int, int> firstPos;//ԭʼ����ʽ��Ӧ�𿪺����ʽ��һ�γ��ֵ�λ��
unordered_map<int, bool> nullable;//ԭʼ����ʽ�ܷ��Ƴ���
int start_id = 0;//��ʼ����ʽ�󲿶�Ӧ������ֵ
string start_str = "";//��ʼ����ʽ�󲿶�Ӧ���ַ���ֵ��%start��������ַ�������û��%start�����Ϊ�գ�
int tokenAmount = 0;//token����
int startPos = 130;//��ʶ����ʼ���ֵ�λ��

extern void mapTokenProcess();//��ȡy.tab.h�ļ�����token�����Ӧ������ֵ�浽TokenAndLabel��
extern void yProcess();//����.y�ļ��������ҵ���ʼ����ʽ��ԭʼ����ʽ�Ĵ洢������ʽ�󲿼����Ӧ����ֵ�Ĵ洢�Լ�yy.tab.c�ļ��Ĵ�����
extern void pdProcess();//���ԭʼ����ʽ�����𿪺����ʽ�����Ϣ����productions�У��ж�ԭʼ����ʽ���ܷ��Ƴ���

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