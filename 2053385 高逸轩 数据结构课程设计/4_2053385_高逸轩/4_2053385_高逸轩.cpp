/* 2053385 高逸轩 */

#include<iostream>
#include<cstdlib>
#include<cstdio>
using namespace std;

// 得到一个[ minNum，maxNum ]的数字，进行了输入错误处理，保证健壮性
int getint(const int minNum = INT_MIN, const int maxNum = INT_MAX, const string errorTips = "")
{
	int retNum = INT_MIN;
	while (1)
	{
		cin >> retNum;
		if (cin.fail() || retNum < minNum || retNum > maxNum) // 若输入变量类型不同或数字不在要求范围内
		{
			cin.clear();
			cin.ignore(1024, '\n');                           // 清除缓存区
			cout << errorTips;                                // 给出错误输入的提示
			continue;
		}
		break;
	}
	return retNum;
}

// 棋盘类
class ChessBoard
{
	friend void SetQueue(ChessBoard& queueProblem, const int currentLine);
public:

	// 构造函数，设置数据成员信息
	ChessBoard(const int n = 0)
	{
		size = counter = 0;
		line = NULL;
		col = leftDiagonal = rightDiagonal = NULL;

		// 申请数组空间
		// 行和列从1-n 编号，故空间多申请一个
		line = new int[n + 1], col = new bool[n + 1];
		// 左对角线和右对角线同样根据大小申请空间
		leftDiagonal = new bool[2 * n + 1], rightDiagonal = new bool[2 * n + 1];

		if (line == NULL || col == NULL || leftDiagonal == NULL || rightDiagonal == NULL)
		{
			cerr << "内存分配错误！" << endl;
			system("pause");
			exit(1);
		}

		// 初始化数组信息
		for (int i = 0; i < n + 1; i++)
			line[i] = col[i] = 0;

		for (int i = 0; i < 2 * n + 1; i++)
			leftDiagonal[i] = rightDiagonal[i] = 0;

		// 棋盘大小初始化
		size = n;
	}

	// 析构函数，释放申请的空间
	~ChessBoard() { delete[]line, delete[]col, delete[]leftDiagonal, delete[]rightDiagonal; }

	// 输出当前棋盘信息
	void Display()
	{
		for (int i = 1; i < size + 1; i++)
		{
			for (int j = 1; j < size + 1; j++)
			{
				if (j == line[i])	cout << 'X' << " ";
				else cout << 0 << " ";
			}
			cout << endl;
		}
		cout << endl;
	}

	// 得到答案计数器的值
	int GetCounter()const { return counter; }

private:
	int size;           // 记录棋盘大小
	int* line;          // 行
	bool* col;          // 列
	bool* leftDiagonal; // 左对角线
	bool* rightDiagonal;// 右对角线
	int counter;        // 记录答案总数
};

// 递归放置皇后棋
void SetQueue(ChessBoard& queueProblem, const int currentLine)  // 当前在currentLine行
{
	// 如果当前行已经超出棋盘，说明整个棋盘已经设置完成，找到一组解
	if (currentLine > queueProblem.size)
	{
		// 将这组解输出
		queueProblem.Display();
		// 计数器更新
		queueProblem.counter++;
		return;
	}

	for(int i=1;i<queueProblem.size+1;i++) // 当前在i列
	{
		// 对第currentLine行第i列进行分析，若当前列、左右对角线上均未放置棋子，则可以放置皇后
		if (*(queueProblem.col + i) == 0 && *(queueProblem.rightDiagonal + currentLine + i) == 0 && *(queueProblem.leftDiagonal + currentLine - i + queueProblem.size) == 0)
		{
			// 第currentLine行被占领
			*(queueProblem.line + currentLine) = i;
			// 第col列被占领
			*(queueProblem.col + i) = true;
			// 第currentLine+i条右对角线被占领
			*(queueProblem.rightDiagonal + currentLine + i) = true;
			// 第currentLine-i+n条左对角线被占领
			*(queueProblem.leftDiagonal + currentLine - i + queueProblem.size) = true;
			// 设置下一行的棋子摆放位置
			SetQueue(queueProblem, currentLine + 1);
			// 此种情况搜索结束后，为在当前行上重新进行放置，需要将当前位置的状态重新置为无棋子
			*(queueProblem.col + i) = false;
			*(queueProblem.rightDiagonal + currentLine + i) = false;
			*(queueProblem.leftDiagonal + currentLine - i + queueProblem.size) = false;
		}
	}
}

int main()
{
	// 给出输入提示
	cout << "现有N×N的棋盘，放入N个皇后，要求所有皇后不在同一行、列、对角线上！" << endl << endl;
	cout << "请输入皇后的个数（6-15）：";

	// 利用getint函数得到棋盘大小，保证健壮性，若n>15，则程序效率无法保证
	int n = getint(6, 15, "皇后个数输入错误，请重新输入：");

	// 利用ChessBoard类进行游戏
	ChessBoard queueProblem(n);
	cout << endl << "皇后摆法" << endl;

	// 输出棋盘情况
	SetQueue(queueProblem, 1);

	// 输出解法总数
	cout << "共有" << queueProblem.GetCounter() << "种解法！" << endl;

	system("pause");

	return 0;
}