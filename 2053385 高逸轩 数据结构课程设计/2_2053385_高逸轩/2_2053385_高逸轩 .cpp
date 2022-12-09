/* 2053385 高逸轩 */

#include<iostream>
#include<iomanip>
#include<string>
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

// 节点
struct LinkedListNode
{

	// 构造函数，初始化编号
	LinkedListNode(int i = 0) { num = i; next = NULL; }

	// 输出编号信息
	void Display() { cout << num << endl; }

	int num;              // 编号
	LinkedListNode* next; // 指向下个位置
};

// 循环链表类
class CircleLinkedList
{
	friend void JosephusSolve(CircleLinkedList& circleJosephus, int n, int s, int m, int k);
public:
	// 构造函数，为头结点申请空间，同时构造循环条件
	CircleLinkedList()
	{
		head = NULL;

		// 为头节点申请空间
		head = new LinkedListNode;

		// 若空间申请异常，进行健壮性处理
		if (head == NULL)
		{
			cerr << "内存分配错误！" << endl;
			system("pause");
			exit(1);
		}

		// 初始状态将头结点的next指针指向自己，以构造循环链表
		head->next = head;
	}

	// 析构函数，释放空间
	~CircleLinkedList()
	{
		LinkedListNode* current = head->next;
		while (head->next != head)
		{
			current = head->next;
			head->next = current->next;
			delete current;
		}
		delete head;
	}

	// 计算循环链表长度
	int Length()                         
	{
		int retNum = 0;
		LinkedListNode* current = head->next;
		// 当重新遍历至头节点后，标志着循环链表的遍历结束
		while (current != head)
		{
			current = current->next;
			retNum++;
		}
		return retNum;
	}

	// 定位函数，返回表中第i个元素的地址
	LinkedListNode* Locate(int i)
	{
		// 对头结点进行特判
		if (!i) return head;

		// i不合理，返回NULL保证健壮性
		if (i < 0 || i>Length())return NULL;

		LinkedListNode* current = head->next; int k = 1;
		while (current != head && k < i)
			current = current->next, k++;

		// 返回第i个节点地址
		return current;
	}

	// 寻找链表中是否含有x编号的人，搜索成功返回该节点位置，否则返回-1
	int Search(int x)
	{
		int retNum = 1;
		LinkedListNode* current = head->next;
		while (current != head)
		{
			// 搜索成功，跳出循环
			if (current->num == x) return retNum;
			// 当前节点搜索失败，搜索下一个节点
			else current = current->next; retNum++;
		}
		return -1;
	}

	// 删除第i个元素,x返回该元素的值
	bool Remove(int i, LinkedListNode& x)
	{
		LinkedListNode* current = Locate(i - 1);

		// 删除失败
		if (current == NULL || current->next == head) return false;

		// 重新拉链，将被删节点从链中摘下
		LinkedListNode* del = current->next;
		current->next = del->next;

		// 记录删除节点的信息
		x.num = del->num;
		x.next = del->next;

		delete del;
		return true;
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

	// 将链表中信息依次输出
	void Display()
	{
		LinkedListNode* current = head->next;
		while (current != head)
		{
			cout << setw(7) << setiosflags(ios::left) << current->num;
			current = current->next;
		}
	}

private:
	LinkedListNode* head;           // 链表头指针 
};

void JosephusSolve(CircleLinkedList& circleJosephus, int n, int s, int m, int k)
{
	// 记录初始节点信息
	LinkedListNode* begin = circleJosephus.Locate(s);
	LinkedListNode current = *begin;

	//共杀死n-k个人
	for (int i = 0; i < n - k; i++)
	{
		// 每数m个数杀死一个人
		for (int j = 1; j < m; j++)
		{
			// 若遇到表头（空节点），则指向第一个节点
			if (begin == circleJosephus.head)
				begin = begin->next;

			// 指向下个数字
			begin = begin->next;

			// 若遇到表头（空节点），则指向第一个节点
			if (begin == circleJosephus.head)
				begin = begin->next;
		}

		// 输出死者信息
		cout << "第" << i + 1 << "个死者的位置是：" << begin->num << endl;
		int killed = begin->num;

		// 杀死后，需要将指针指向被杀者的下一个人
		begin = begin->next;

		// 杀死编号为killed的人，用current记录其信息
		circleJosephus.Remove(circleJosephus.Search(killed), current);
	}

	cout << "最终剩下：        " << k << "人" << endl;
	cout << "剩余的生者位置为：";
	// 最后遍历循环链表来输出所有生者信息
	circleJosephus.Display();
}

int main()
{
	int n, s, m, k;
	cout << "现有N人围成一圈，从第S个人开始依次报数，报M的人出局，再由下一人开始报数，如此循环，直至剩下K个人为止 " << endl << endl;

	cout << "请输入生死游戏的总人数N：";
	n = getint(1, INT_MAX, "输入错误，请重新输入生死游戏的总人数N：");

	cout << "请输入游戏开始的位置S：  ";
	s = getint(1, n, "输入错误，请重新输入游戏开始的位置S：");

	cout << "请输入死亡数字M：        ";
	m = getint(1, n, "输入错误，请重新输入死亡数字M：");

	cout << "请输入剩余的生者人数K：  ";
	k = getint(1, n - 1, "输入错误，请重新输入剩余的生者人数K：");
	cout << endl;
	CircleLinkedList circleJosephus;

	// 将所有人插入链表
	for (int i = 0; i < n; i++)
	{
		LinkedListNode current(i + 1);
		circleJosephus.Insert(i, current);
	}

	// 进行约瑟夫环的求解
	JosephusSolve(circleJosephus, n, s, m, k);

	system("pause");
	return 0;
}