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

// 采用孩子兄弟表示法，每一个树上节点包含两个指针域，分别指向第一个儿子和下一个兄弟
// 家族树节点
struct TreeNode
{
	// 构造函数，根据所给数据初始化数据
	TreeNode(string s = "", TreeNode* fc = NULL, TreeNode* ns = NULL) :name(s), firstChild(fc), nextSibling(ns) { father = NULL; };

	TreeNode* firstChild;  // 指向第一个孩子
	TreeNode* nextSibling; // 指向下一个兄弟
	TreeNode* father;      // 指向父节点
	string name;           // 名字
};

// 家族树
class Tree
{
public:
	// 构造函数
	Tree() { root = NULL; }

	// 析构函数，释放空间
	~Tree() { DeleteTree(root); }

	// 建立根节点，即家族的祖先
	void BuildRoot()
	{
		// 得到祖先姓名
		cout << "首先建立一个家谱！" << endl;
		cout << "请输入祖先的姓名：";
		string name;
		cin >> name;

		// 为root分配空间
		root = new TreeNode(name);

		// 若失败则给出提示，保证健壮性
		if (root == NULL)
		{
			cerr << "内存分配错误！" << endl;
			system("pause");
			exit(1);
		}

		// 输出祖先名字
		cout << "此家谱的祖先是：" << root->name << endl;
	}

	// 建立某个子树
	void BuildTree()
	{
		// 得到建立者姓名
		cout << "请输入要建立家庭的人的姓名：";
		string name;
		cin >> name;

		// 用parent来Search找到建立者的位置
		TreeNode* parent = Search(name);

		// 错误输入处理
		if (parent == NULL)
		{
			cout << "本家谱内不存在该家庭成员！" << endl;
			return;
		}

		// 若当前人已经有了孩子，说明已经建立过家庭，则不能再次建立家庭，给出提示
		if (parent->firstChild != NULL)
		{
			cout << "该成员已经建立过家庭！" << endl;
			return;
		}

		// 利用getint函数得到儿女个数，保证健壮性
		cout << "请输入" << name << "的儿女个数：";
		int childNum = getint(0, INT_MAX, "儿女个数必须为正整数，请重新输入！");

		// 为parent插入孩子
		cout << "请依次输入" << name << "的儿女的姓名:";

		// 为parent插入firstChild
		while (1)
		{
			cin >> name;

			// 该名称未被使用，可以插入
			if (Search(name) == NULL)
			{
				// 更新儿子
				parent->firstChild = new TreeNode(name);

				// 对内存管理进行错误处理，保证健壮性
				if (parent->firstChild == NULL)
				{
					cerr << "内存分配错误！" << endl;
					system("pause");
					exit(1);
				}

				// 更新儿子节点的父亲为parent
				parent->firstChild->father = parent;
				break;
			}

			// 该名称已经使用过了，需要重新输入
			else
			{
				cout << "姓名为" << name << "的成员已存在！请重新输入！" << endl;
				cin.clear();

				// 清除缓存区，避免后续错误输入产生后效性
				cin.ignore(1024, '\n');
			}
		}


		// 更新其他子女信息
		for (int i = 1; i < childNum; i++)
		{
			while (1)
			{
				cin >> name;

				// 该名称未被使用，可以插入
				if (Search(name) == NULL)
				{
					TreeNode* p = parent->firstChild;

					// 将p指针移到parent的最后一个孩子处
					while (p->nextSibling != NULL)
						p = p->nextSibling;

					// 为最后一个孩子更新兄弟
					p->nextSibling = new TreeNode(name);

					// 对内存管理进行错误处理，保证健壮性
					if (p->nextSibling == NULL)
					{
						cerr << "内存分配错误！" << endl;
						system("pause");
						exit(1);
					}

					// 对最后一个孩子的father设置为parent
					p->nextSibling->father = parent;
					break;
				}

				// 该名称已经使用过了，需要重新输入
				else
				{
					cout << "姓名为" << name << "的成员已存在！请重新输入！" << endl;
					cin.clear();

					// 清除缓存区，避免后续错误输入产生后效性
					cin.ignore(1024, '\n');
				}
			}
		}

		// 展示parent节点的孩子
		ShowTree(parent->name);
	}

	// 添加某个节点
	void AddTreeNode()
	{
		// 得到添加者姓名
		cout << "请输入要添加儿子（或女儿）的人的姓名：";
		string name;
		cin >> name;

		// 用parent记录添加者的位置
		TreeNode* parent = Search(name);

		// 错误输入处理
		if (parent == NULL)
		{
			cout << "本家谱内不存在该家庭成员！" << endl;
			return;
		}

		// 得到新添加的节点的姓名
		cout << "请输入" << name << "新添加儿子（或女儿）的姓名：";

		// 更新firstChild
		if (parent->firstChild == NULL)
		{
			while (1)
			{
				cin >> name;

				// 该名称未被使用，可以插入
				if (Search(name) == NULL)
				{
					// 更新firstChild
					parent->firstChild = new TreeNode(name);

					// 对内存进行处理，保证健壮性
					if (parent->firstChild == NULL)
					{
						cerr << "内存分配错误！" << endl;
						system("pause");
						exit(1);
					}

					// 将firstChild的father设置为parent
					parent->firstChild->father = parent;
					break;
				}

				// 该名称已经使用过了，需要重新输入
				else
				{
					cout << "姓名为" << name << "的成员已存在！请重新输入！" << endl;
					cin.clear();

					// 清除缓存区，避免后续错误输入产生后效性
					cin.ignore(1024, '\n');
				}
			}
		}

		// 更新孩子节点的nextSibling
		else
		{
			while (1)
			{
				cin >> name;

				// 该名称未被使用，可以插入
				if (Search(name) == NULL)
				{
					TreeNode* p = parent->firstChild;

					// 将p指针移到parent的最后一个孩子处
					while (p->nextSibling != NULL)
						p = p->nextSibling;

					// 更新兄弟
					p->nextSibling = new TreeNode(name);

					// 对内存进行处理，保证健壮性
					if (p->nextSibling == NULL)
					{
						cerr << "内存分配错误！" << endl;
						system("pause");
						exit(1);
					}

					// 设置当前最后一个孩子的father为parent
					p->nextSibling->father = parent;
					break;
				}

				// 该名称已经使用过了，需要重新输入
				else
				{
					cout << "姓名为" << name << "的成员已存在！请重新输入！" << endl;
					cin.clear();

					// 清除缓存区，避免后续错误输入产生后效性
					cin.ignore(1024, '\n');
				}
			}
		}
		// 展示parent节点的孩子
		ShowTree(parent->name);
	}

	// 删除某个子树
	void DeleteTree()
	{
		// 得到解散者的姓名
		cout << "请输入要解散家庭的人的姓名:";
		string name;
		cin >> name;

		// 利用parent得到解散者的位置
		TreeNode* parent = Search(name);
		if (parent == NULL)
		{
			cout << "本家谱内不存在该家庭成员！" << endl;
			return;
		}
		cout << "要解散家庭的人是:" << name << endl;

		// 在解散前首先展示当前解散者的孩子
		ShowTree(parent->name);

		// 若解散者不是根节点，需要进行以下操作将以parent为根节点的树分离出，维护家族树仍然有序：
		if (parent != root)
		{
			// 如果解散者是其父节点的第一个儿子，则其父节点的firstChild指针需要指向解散者的下一个兄弟
			if (parent->father->firstChild == parent) parent->father->firstChild = parent->nextSibling;

			// 如果解散者不是其父节点的第一个儿子，则它的上一个兄弟节点的nextSibling指针需要指向解散者的下一个兄弟
			else
			{
				// 用p来寻找解散者的上一个兄弟节点
				TreeNode* p = parent->father->firstChild;
				while (1)
				{
					if (p->nextSibling == parent) break;
					p = p->nextSibling;
				}

				//上一个兄弟节点的nextSibling指针需要指向解散者的下一个兄弟
				p->nextSibling = parent->nextSibling;
			}

			// 将以parent为根的树分离出后，再释放内存
			DeleteTree(parent);
		}
		else
			// 若解散者恰为根节点，不需要分离，直接释放内存即可
			DeleteTree(parent);
	}

	// 更改某个节点信息
	void ResetTreeNode()
	{
		// 得到改名人的信息
		cout << "请输入要更改姓名的人的目前姓名:";
		string name;
		cin >> name;

		// 用current记录改名者的位置
		TreeNode* current = Search(name);

		// 对内存管理进行错误处理，保证健壮性
		if (current == NULL)
		{
			cout << "本家谱内不存在该家庭成员！" << endl;
			return;
		}

		// 得到更改后的姓名
		cout << "请输入更改后的姓名：";
		while (1)
		{
			cin >> name;

			// 若更改后的姓名没有出现过，可以改名
			if (Search(name) == NULL)
			{
				cout << current->name << "已更名为" << name << endl;
				current->name = name;
				break;
			}

			// 该名称已经使用过了，需要重新输入
			else
			{
				cout << "姓名为" << name << "的成员已存在！请重新输入！" << endl;
				cin.clear();

				// 清除缓存区，避免后续错误输入产生后效性
				cin.ignore(1024, '\n');
			}
		}

	}

	// 展示某个节点子女
	void ShowTree(const string& name)
	{
		// 用current定位name姓名人的位置
		TreeNode* current = Search(name);
		if (current == NULL)
		{
			cout << endl;
			return;
		}

		// 没有子女处理
		current = current->firstChild;
		if (current == NULL)
		{
			cout << name << "没有子女" << endl;
			return;
		}

		// 有子女处理
		else
		{
			// 依次输出子女姓名
			cout << name << "的第一代子女有：";
			cout << current->name << " ";
			while (current->nextSibling != NULL)
			{
				current = current->nextSibling;
				cout << current->name << ' ';
			}
			cout << endl;
		}
	}

	// 搜索树中是否有名字为name的家族成员，查找成功返回对应地址的值，失败则返回NULL
	TreeNode* Search(const string& name)const
	{
		if (root == NULL)
			return NULL;
		else
			return Search(root, name);
	}

private:
	TreeNode* root;   // 家族树根节点

	// 查看当前结点是否为所需要查找的家族成员，查找成功返回对应地址的值，失败则返回NULL
	TreeNode* Search(TreeNode* current, const string& name)const
	{
		// 如果当前节点符合
		if (current->name == name)
			return current;

		// 遍历当前节点的儿子节点进行递归搜索
		TreeNode* findResult = NULL;
		for (TreeNode* p = current->firstChild; p != NULL; p = p->nextSibling)
		{
			findResult = Search(p, name);

			// 儿子节点搜索成功
			if (findResult != NULL)
				return findResult;
		}

		// 儿子节点搜索失败
		return NULL;
	}

	// 删除以当前结点为树根的子树
	void DeleteTree(TreeNode* current)
	{
		// 遇到空节点回溯
		if (current == NULL) return;

		// 当前节点有子女，向下递归
		if (current->firstChild != NULL)
		{
			// 先删除第一个儿子
			DeleteTree(current->firstChild);
			TreeNode* p = current->firstChild;

			// 再依次删除其兄弟
			while (p->nextSibling != NULL)
			{
				DeleteTree(p->nextSibling);
				p = p->nextSibling;
			}

			// 释放内存
			delete current;
		}
	}
};

// 得到A-E的字符，进行了错误输入处理
char Choice()
{
	cout << endl << "请选择要执行的操作：";
	char ret = '\0';

	// 如果输入的字符不在A-E，要求用户继续输入
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
	printf("**           家谱管理系统            **\n");
	printf("=======================================\n");
	printf("**        请选择要执行的选择:        **\n");
	printf("**           A --- 完善家谱          **\n");
	printf("**           B --- 添加家庭成员      **\n");
	printf("**           C --- 解散局部家庭      **\n");
	printf("**           D --- 更改家庭成员姓名  **\n");
	printf("**           E --- 退出程序          **\n");
	printf("***************************************\n");

	Tree tree;
	tree.BuildRoot();
	while (1)
	{
		char ch = Choice();
		switch (ch)
		{
		case 'A':
			// 以已经存在的节点为根建立一棵树
			tree.BuildTree();
			break;
		case 'B':
			// 为已经存在的节点添加一个儿子
			tree.AddTreeNode();
			break;
		case 'C':
			// 删除已经存在的节点为根的一棵树
			tree.DeleteTree();
			break;
		case 'D':
			// 为已经存在的节点更改信息
			tree.ResetTreeNode();
			break;
		case 'E':
			// 退出程序
			cout << endl;
			system("pause");
			return 0;
		}
	}
}