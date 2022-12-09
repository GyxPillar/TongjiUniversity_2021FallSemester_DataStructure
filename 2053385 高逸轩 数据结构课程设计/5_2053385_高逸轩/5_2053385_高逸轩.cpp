/* 2053385 高逸轩 */
#define _CRT_SECURE_NO_WARNINGS

#include<iostream>
#include<iomanip>
#include<cstdlib>
#include<algorithm>
#include<string>
#include<ctype.h>

using namespace std;

// 字典树节点
struct TrieNode
{
    TrieNode() 
    {
        wordNum = 0;
        for (int i = 0; i < 26; i++)
            next[i] = NULL;
    }
    int wordNum;                  // 单词频数，若为0说明当前位置不在词尾
    TrieNode* next[26];           // 指向26个可能的后继节点（26个字母）
};

// 字典树 Trie树
class Trie {

public:

    // 构造函数，为根节点申请空间
    Trie()
    {
        root = NULL;

        root = new TrieNode;

        // 若空间分配错误，进行错误提示
        if (root == NULL)
        {
            cerr << "内存分配错误！" << endl;
            system("pause");
            exit(1);
        }
    }

    // 向字典树插入单词word
    void insert(string word)
    {
        TrieNode* current = root;

        // 按照word的字符，从根节点开始，一直向下走
        for (int i = 0; i < word.length(); i++)
        {
            char c = word[i];
            // 如果遇到NULL，就new出新节点
            if (current->next[c - 'a'] == nullptr)
            {
                current->next[c - 'a'] = new TrieNode;

                // 若空间分配错误，进行错误提示
                if (current->next[c - 'a'] == NULL)
                {
                    cerr << "内存分配错误！" << endl;
                    system("pause");
                    exit(1);
                }
            }
            // 如果节点已经存在，current顺着往下走就可以
            current = current->next[c - 'a'];
        }
        // 词频增加
        current->wordNum++;
    }

    // 搜索word出现的词频
    int search(string word)
    {
        TrieNode* current = root;
        // current从根节点开始，按照word的字符一直尝试向下走
        for (int i = 0; i < word.length(); i++)
        {
            char c = word[i];
            // 如果走到了NULL，说明这个word不是前缀树的任何一条路径，返回0
            if (current->next[c - 'a'] == nullptr)  return 0;
            current = current->next[c - 'a'];
        }
        //  如果按照word顺利的走完，就要判断此时current是否为单词尾端：如果是，返回wordNum；如果不是，说明word仅仅是一个前缀，并不完整，返回单词中间节点的记录值0
        return current->wordNum;
    }

private:
    TrieNode* root;      // 根节点
};

int main()
{
    cout << "文件检索系统" << endl;

    // 得到文件名称
    FILE* fp;
    char fileName[1024];
    cout << "请输入文件名：";
    cin.getline(fileName, 1024);

    // 打开文件
    fp = fopen(fileName, "w");

    // 对文件处理保证健壮性
    if (fp == NULL)
    {
        cout << "文件打开失败" << endl;
        system("pause");
        exit(-1);
    }

    // 得到文件内容
    cout << "请输入一段英文：" << endl;
    char fileContent[1024];
    cin.getline(fileContent, 1024);

    // 将文件内容写入文件
    fputs(fileContent, fp);

    cout << "本段文本已经保存在文本文件“" << fileName << "”中。";

    // 关闭文件
    fclose(fp);

    // 输入检索单词
    string keyWord;
    cout << "请输入要检索的关键字：";
    cin >> keyWord;

    // 展示文件内容
    cout << endl << "显示源文件“" << fileName << "”：" << endl;
    fp = fopen(fileName, "r");
    fgets(fileContent, 1024, fp);
    cout << fileContent;

    // 将文本中的单词全部以小写单词形式插入字典树，同时统计词频
    Trie trie;
    int i = 0;
    cout << endl;
    while (fileContent[i] != '\0')
    {
        // 若当前位置不是字母，则向后寻找
        if (!isalpha(fileContent[i]))
        {
            i++;
            continue;
        }
        // current用来记录当前单词
        string current = "";
        while (isalpha(fileContent[i]))
        {
            // 保证currrnt插入的都为小写字母
            current += tolower(fileContent[i]);
            i++;
        }
        // 若current不为空，插入字典树
        if (current != " ")  trie.insert(current);
    }

    // 用_keyWord记录全为小写的关键词
    string _keyWord = keyWord;

    // 利用transform进行转换
    transform(keyWord.begin(), keyWord.end(), _keyWord.begin(), ::tolower);

    // 利用Trie得到关键词词频
    cout << endl << "在源文件中共检索到 ：" << trie.search(_keyWord) << "个关键字“" << keyWord << "”" << endl;
    system("pause");
    return 0;

}