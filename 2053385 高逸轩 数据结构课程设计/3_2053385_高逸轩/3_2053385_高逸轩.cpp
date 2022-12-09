/* 2053385 高逸轩 */

#include<iostream>
#include<iomanip>
using namespace std;

// 地图的最大行坐标、列坐标，可自行修改
const int c_xMAX = 6;
const int c_yMAX = 6;

// 辅助进行坐标变化，分别代表左、右、上、下
const int dx[] = { 0,0,-1,1 };
const int dy[] = { -1,1,0,0 };

// 地图的初始状态，#代表障碍，0代表可行走，可自行修改，注意与c_xMAX和c_yMAX匹配
char c_map[c_xMAX + 1][c_yMAX + 1] = {
'#', '#', '#', '#', '#', '#', '#',
'#', '0', '#', '0', '0', '0', '#',
'#', '0', '#', '0', '#', '#', '#',
'#', '0', '0', '0', '#', '0', '#',
'#', '0', '#', '0', '0', '0', '#',
'#', '0', '#', '0', '#', '0', '#',
'#', '#', '#', '#', '#', '#', '#' };

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

// 节点
struct LinkedListNode
{
	// 构造函数，初始化坐标
	LinkedListNode(int xx = 0, int yy = 0) :x(xx), y(yy) { next = NULL; flag = 0; };
	// 展示该节点坐标
	void Display() { cout << '<' << x << ',' << y << '>'; }

	// 记录行坐标、列坐标
	int x, y;
	// 记录搜索时是否以及被经过
	bool flag;
	// 记录搜索时的下一个点
	LinkedListNode* next;
};

class Game;

// 链表类
class LinkedList
{
	friend Game;

public:
	// 构造函数，为head指针分配初始空间
	LinkedList()
	{
		head = NULL;
		// 为头指针分配空间
		head = new LinkedListNode;
		// 若失败则给出提示，保证健壮性
		if (head == NULL)
		{
			cerr << "内存分配错误！" << endl;
			system("pause");
			exit(1);
		}
		head->next = NULL;
	};

	// 析构函数，释放空间
	~LinkedList()
	{
		LinkedListNode* current = head->next;
		while (head->next != NULL)
		{
			current = head->next;
			head->next = current->next;
			delete current;
		}
		delete head;
	}

	// 计算带附加头结点的单链表的长度
	int Length()
	{
		// retNum为长度
		int retNum = 0; LinkedListNode* current = head->next;

		// 循链扫描，寻找链尾
		while (current != NULL)
		{
			current = current->next; retNum++;
		}

		return retNum;
	}

	// 定位函数，返回表中第i个元素的地址
	LinkedListNode* Locate(int i)
	{
		// i不合理，返回NULL保证健壮性
		if (i < 0)return NULL;
		LinkedListNode* current = head; int k = 0;
		while (current != NULL && k < i)
			current = current->next, k++;
		// 返回第i个节点地址，若返回NULL,则代表i值太大
		return current;
	}

	// 将新元素x插入到第i个节点后
	bool Insert(int i, LinkedListNode& x)
	{
		LinkedListNode* current = Locate(i);

		// 插入不成功
		if (current == NULL)return false;

		LinkedListNode* newNode = new LinkedListNode(x);
		// 空间申请异常
		if (newNode == NULL)
		{
			cerr << "内存分配错误！" << endl;
			system("pause");
			exit(1);
		}

		// 链接在current之后
		newNode->next = current->next;
		current->next = newNode;

		// 插入成功
		return true;
	}

	// 输出链表信息
	void Display()
	{
		LinkedListNode* now = head->next;
		while (now != NULL)
		{
			now->Display();
			now = now->next;
			if (now != NULL)
				cout << " --> ";
		}
	}

private:
	LinkedListNode* head;           // 链表头指针 
};

// 游戏主功能
class Game
{
public:

	// 构造函数，初始化坐标信息
	Game()
	{
		for (int i = 0; i < c_xMAX + 1; i++)
			for (int j = 0; j < c_yMAX + 1; j++)
				node[i][j].x = i, node[i][j].y = j;
	}

	// 深度优先搜索，寻路，若可达返回true并记录路径，不可达返回false
	bool FindPath(int nowX, int nowY, int dstX, int dstY)
	{
		// 当前位置越界或有障碍物，不可达
		if (nowX<0 || nowX>c_xMAX || nowY<0 || nowY>c_yMAX || node[nowX][nowY].flag || c_map[nowX][nowY] == '#')
			return false;

		// 标记当前位置已经被经过
		node[nowX][nowY].flag = true;

		// 如果当前位置为终点
		if (nowX == dstX && nowY == dstY)
		{
			// 在地图上标记此点被经过
			c_map[nowX][nowY] = 'x';
			// 将此点插入到表示路径的链表中
			path.Insert(path.Length(), node[nowX][nowY]);
			return true;
		}

		// 向四周搜索
		for (int i = 0; i < 4; i++)
		{
			// 如果搜索的点可达
			if (FindPath(nowX + dx[i], nowY + dy[i], dstX, dstY))
			{
				// 将当前点在地图上记录为在路径上
				c_map[nowX][nowY] = 'x';
				// 将当前的点插入到表示路径的链表中
				path.Insert(path.Length(), node[nowX][nowY]);
				return true;
			}
		}
		return false;
	}

	// 输出迷宫路径
	void Display()
	{
		cout << endl << "迷宫路径" << endl << endl;
		path.Display();
	}

	bool GetNodeFlag(int x, int y) { return node[x][y].flag; }

private:
	LinkedListNode node[c_xMAX + 1][c_yMAX + 1];
	LinkedList path;
};

int main()
{
	// 输出地图初始信息
	cout << "初始地图" << endl << endl;
	cout << "        ";
	for (int i = 0; i < c_yMAX + 1; i++)
		cout << i << "列        ";
	cout << endl;

	for (int i = 0; i < c_xMAX + 1; i++)
	{
		cout << i << "行     ";
		for (int j = 0; j < c_yMAX + 1; j++)
			cout << c_map[i][j] << "          ";
		cout << endl;
	}

	// 利用getint函数得到起点、终点信息，同时保证健壮性
	cout << "请输入起点x坐标：";
	int srcX = getint(0, c_xMAX, "起点x坐标输入错误，请重新输入：");
	cout << "请输入起点y坐标：";
	int srcY = getint(0, c_yMAX, "起点y坐标输入错误，请重新输入：");
	cout << "请输入终点x坐标：";
	int dstX = getint(0, c_xMAX, "终点x坐标输入错误，请重新输入：");
	cout << "请输入终点y坐标：";
	int dstY = getint(0, c_yMAX, "终点y坐标输入错误，请重新输入：");

	// 起始点错误
	if (c_map[srcX][srcY] == '#' || c_map[dstX][dstY] == '#')
	{
		cout << "无法到达" << endl;
		return 0;
	}

	Game game;

	// 从起点无法到达终点
	if (!game.FindPath(dstX, dstY, srcX, srcY))
	{
		cout << "无法到达" << endl;
		system("pause");
		return 0;
	}

	// 能够到达，输出路径地图
	cout << endl << "路径地图" << endl << endl;

	// 数组展示地图路径
	for (int i = 0; i < c_yMAX + 1; i++)
		cout << i << "列        ";
	cout << endl;

	for (int i = 0; i < c_xMAX + 1; i++)
	{
		cout << i << "行     ";
		for (int j = 0; j < c_yMAX + 1; j++)
			cout << c_map[i][j] << "          ";
		cout << endl;
	}
	// 文字展示地图路径
	game.Display();
	cout << endl;
	system("pause");
	return 0;
}