/* 2053385 高逸轩 */

#include <iostream>
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


// 边
struct Edge
{
	Edge()
	{
		src = dst = -1;
		value = 0;
	}
	int src;          // 起始点编号  
	int dst;          // 目标点序号
	double value;     // 边长度权重
	bool operator<(const Edge& other) { return value < other.value; };// 重载小于比较运算符
	bool operator>(const Edge& other) { return value > other.value; };// 重载大于比较运算符
};

// 点
struct Vertex
{
	Vertex()
	{
		name = "";
		ancestor = -1;
	}
	string name;
	int ancestor;          // 记录在并查集中的祖先
};

// 类模板实现优化后的快速排序，排序区间为[l,r]
template<typename T>
void QuickSort(T a[], int l, int r)
{
	if (l > r)
		return;
	T mid = a[(l + r) / 2];      // 取中间元素
	int i = l, j = r;
	while (i < j)
	{
		while (a[i] < mid) i++;  // 查找左半部分比中间数大的数
		while (a[j] > mid) j--;  // 查找右半部分比中间数小的数
		if (i <= j)              // 如果有一组不满足排序条件（左小右大）的数
		{
			swap(a[i], a[j]);    // 交换
			i++;
			j--;
		}
	}
	if (l < j) QuickSort(a, l, j);// 递归排序左半部分
	if (i < r) QuickSort(a, i, r);// 递归排序右半部分
}

class Graph
{
	template<typename T>
	friend void QuickSort(T a[], int l, int r);
public:
	// 构造函数
	Graph(int n = 0, int m = 0) :vertexNumber(n), edgeNumber(m)
	{
		vertexs = NULL, edges = NULL;
		MST = NULL;
		vertexs = new Vertex[n]; edges = new Edge[m];
	}

	// 析构函数
	~Graph()
	{
		// 分别释放最小生成树、边数组和顶点数组内存，避免内存泄露
		delete[]edges;
		delete[]vertexs;
		delete MST;
	}

	// Kruskal算法求得最小生成树，将信息存储进MST
	bool Kruskal()
	{
		// 快速排序，区间[0,edgeNumber - 1]
		QuickSort(edges, 0, edgeNumber - 1);

		// 删除原来的MST信息
		delete MST;
		MST = NULL;

		// 为MST申请空间
		MST = new Graph(vertexNumber, vertexNumber - 1);

		// cnt记录已选边数量
		int cnt = 0;
		for(int i=0;i<edgeNumber;i++)
		{
			int f1 = FindAncestor(edges[i].src); // f1为边起点的祖先
			int f2 = FindAncestor(edges[i].dst); // f2为边终点的祖先

			// 若两点祖先不同，则分属不同的连通分支，需要合并，将此边加入最小生成树
			if (f1 != f2)
			{
				cnt++;                     // 已选边数+1
				vertexs[f1].ancestor = f2; // f1的祖先设置为f2，两点属于同一个联通分支
				MST->edges[cnt - 1] = edges[i];
			}
			if (cnt == vertexNumber - 1)   // 当最小生成树终已选边数=顶点数-1时，最小生成树构造完成
			{
				cout << endl;
				return true;
			}
		}
		// 若未能选满vertexNumber-1条边，则不能构成最小生成树
		delete MST;
		MST = NULL;
		cout << endl;
		return false;

	}

	// 初始化顶点信息
	void InitVertex()
	{
		// 将图重置
		MakeEmpty();

		// 初始化顶点数量
		cout << "请输入顶点的个数：";
		vertexNumber = getint(1, 1024, "顶点必须是1-1024的数字，请重新输入：");
		vertexs = new Vertex[vertexNumber];

		// 设置顶点名称
		cout << "请依次输入各顶点的名称：" << endl;
		for (int i = 0; i < vertexNumber; ++i)
		{
			string vertexName;
			cin >> vertexName;
			// 判断姓名是否已存在
			if (SearchVertex(vertexName) != -1)
			{
				// 若已经存在，请重新输入
				cout << "顶点" << vertexName << "已存在！请重新输入：";
				i--;
			}
			// 初始化并查集祖先为自己
			else
			{
				vertexs[i].name = vertexName;
				vertexs[i].ancestor = i;
			}
		}
	}

	// 初始化边信息
	void InitEdge()
	{
		// 删除原来的边集信息
		delete[]edges;

		cin.clear();
		cin.ignore(1024, '\n');                           // 清除缓存区

		// 根据题目信息，边最多有 n*(n-1)/2 条
		cout << "请输入边的数目：";
		edgeNumber = getint(1, vertexNumber * (vertexNumber - 1) / 2, "根据题目信息，边最多有 n*(n-1)/2 条，请重新输入：");
		edges = new Edge[edgeNumber];

		// 输入边集信息，通过getint函数处理了错误输入，保证了健壮性
		cout << "请依次输入" << edgeNumber << "条边的起点、终点、权重：" << endl;

		for(int i=0;i<edgeNumber;i++)
		{
			// 设置起点终点
			cout << "请输入第" << i + 1 << "条边的信息：";
			string src, dst;
			double value;
			cin >> src >> dst;
			edges[i].src = SearchVertex(src);
			edges[i].dst = SearchVertex(dst);
			// 信息异常处理
			if (edges[i].src == -1 || edges[i].dst == -1)
			{
				cout << "点信息输入错误，请重新输入：" << endl;
				cin.clear();
				cin.ignore(1024, '\n');                           // 清除缓存区
				i--;
				continue;
			}

			// 设置边权
			cin >> value;
			// 信息异常处理
			if (cin.fail() || value <= 0)
			{
				cout << "点信息输入错误，请重新输入：" << endl;
				cin.clear();
				cin.ignore(1024, '\n');                           // 清除缓存区
				i--;
				continue;
			}
			edges[i].value = value;
		}
	}

	// 打印最小生成树
	void printMST()
	{
		// 未生成成功
		if (MST == NULL)
		{
			cout << "未生成最小生成树，请先生成！" << endl << endl;
			return;
		}


		// 生成成功
		cout << "最小生成树信息如下：" << endl;
		// 输出MST中的信息
		for(int i=0;i<MST->edgeNumber;i++)
			cout << "选边信息：起点<" << vertexs[MST->edges[i].src].name << ">  终点<" << vertexs[MST->edges[i].dst].name << ">  权值：" << MST->edges[i].value << endl;

		cout << endl;

	}


private:
	// 将图重置
	void MakeEmpty()
	{
		vertexNumber = 0, edgeNumber = 0;
		if (edges != NULL)	delete[]edges;
		if (vertexs != NULL)delete[]vertexs;
		edges = NULL, vertexs = NULL;
	}

	// 寻找在图中是否有名字为s的顶点
	int SearchVertex(const string& s)
	{
		for (int i = 0; i < vertexNumber; i++)
		{
			// 如果有，返回下标
			if (vertexs[i].name == s)
				return i;
		}
		// 如果没有，返回-1
		return -1;
	}

	// 并查集寻父节点，采用记忆化搜索，进行路径压缩优化
	int FindAncestor(int i)
	{
		if (i == vertexs[i].ancestor) return i;
		return vertexs[i].ancestor = FindAncestor(vertexs[i].ancestor);
	}

	Graph* MST;					  // 最小生成树
	int vertexNumber, edgeNumber; // 记录顶点数、边数
	Edge* edges;                  // 顶点数组
	Vertex* vertexs;              // 边数组
};

// 得到A-E的字符，进行了错误输入处理
char Choice()
{
	cout << endl << "请选择操作：";
	char ret = '\0';

	while (1)
	{
		cin >> ret;
		if (cin.fail() || ret < 'A' || ret > 'E') // 若输入变量类型不同或数字不在要求范围内
		{
			cin.clear();
			cin.ignore(1024, '\n');                           // 清除缓存区
			cout << "输入错误，请重新输入：";                                // 给出错误输入的提示
			continue;
		}
		break;
	}
	return ret;
}

int main()
{
	cout << "**    电网造价模拟系统    **" << endl;
	cout << "============================" << endl;
	cout << "**    A---创建电网顶点    **" << endl;
	cout << "**    B---添加电网的边    **" << endl;
	cout << "**    C---构建最小生成树  **" << endl;
	cout << "**    D---显示最小生成树  **" << endl;
	cout << "**    E---退出    程序    **" << endl;
	cout << "============================" << endl;

	Graph graph;
	while (1)
	{
		char ch = Choice();
		switch (ch)
		{
		case 'A':
			graph.InitVertex();
			break;
		case 'B':
			graph.InitEdge();
			break;
		case 'C':
			if (graph.Kruskal()) cout << "Kruskal最小生成树生成完毕！" << endl;
			else cout << "无法生成最小生成树！";
			break;
		case 'D':
			graph.printMST();
			break;
		case 'E':
			system("pause");
			return 0;
		}
	}
}