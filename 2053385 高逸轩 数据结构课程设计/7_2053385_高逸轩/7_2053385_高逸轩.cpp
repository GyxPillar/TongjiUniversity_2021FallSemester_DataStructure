/* 2053385 高逸轩 */

#include<iostream>
#include<iomanip>
#include<string>

using namespace std;

// 记录节点记录的是数字还是运算符
enum { isNumber, isOperation };

// 每个节点的信息
struct info
{
	info()
	{
		value = 0.0;
		operation = '\0';
		infoType = 0;
	}
	double value;                                    // 数字的值 
	char operation;                                  // 运算符
	bool infoType;                                   // 记录节点信息
};

// 二叉树节点
struct BinaryTreeNode
{
	// 构造函数，为左右儿子赋值
	BinaryTreeNode(BinaryTreeNode* lc = NULL, BinaryTreeNode* rc = NULL) :leftChild(lc), rightChild(rc) {};

	// double to BinaryTreeNode 显式转换
	BinaryTreeNode(double v) { data.value = v; data.infoType = isNumber; }

	// char to BinaryTreeNode 显式转换
	BinaryTreeNode(char o) { data.operation = o; data.infoType = isOperation; }

	BinaryTreeNode* leftChild = NULL, * rightChild = NULL;// 左右儿子
	info data;                                            // 节点存储信息
};
// 重载输出运算符
ostream& operator<<(ostream& os, const BinaryTreeNode& x)
{
	// 根据该节点存储的数据类型进行输出
	if (x.data.infoType == isOperation) os << x.data.operation;
	if (x.data.infoType == isNumber) os << x.data.value;
	return os;
}

// 二叉树
class BinaryTree
{
public:
	// 构造函数：利用输入的字符串表达式s建立表达式树
	// 思路：先将原表达式转化为逆波兰式，再将逆波兰式转化为表达式树
	BinaryTree(string& s)
	{
		/*将中缀表达式转换为逆波兰表达式*/

		char stack1[1024];                // 存放运算符的栈 
		int top1 = 0;                     // 栈顶
		int lengthRPN = 0;                // 逆波兰表达式长度
		BinaryTreeNode RPN[1024];         // 存放逆波兰表达式
		// temp用以提取expression中的高位数字和小数

		string temp = "";

		// 将表达式整体套上括号
		s += ')';
		stack1[top1++] = '(';

		// 遍历表达式
		for(int i=0;i<s.length();i++)
		{
			// 表达式该位置为一个数字，记录进temp
			if (isdigit(s[i]))
				temp += s[i];

			// 小数点直接记入temp
			else if (s[i] == '.')
				temp += s[i];

			// 表达式该位置为运算符
			else
			{
				// 若temp不为空，需要先将现在的temp转化为数字节点后加入逆波兰式，并将temp重置
				if (temp != "")
				{
					// 利用atof函数和c_str 实现 char* to double 与 string to char*
					RPN[lengthRPN++] = BinaryTreeNode(atof(temp.c_str()));
					temp = "";
				}

				// 遇到左括号直接入栈
				if (s[i] == '(') stack1[top1++] = s[i];

				// 遇到右括号则弹出栈中运算符并进行运算，直到遇到左括号为止
				else if (s[i] == ')')
				{
					// 栈为空，则直接进入下一次循环，不必进行左括号匹配
					if (!top1) continue;

					// 弹出栈内运算符直到匹配到左括号
					while (stack1[top1 - 1] != '(')
						// 将栈顶运算符转换为节点后加入逆波兰式，同时弹出栈顶元素
						RPN[lengthRPN++] = BinaryTreeNode(stack1[--top1]);

					top1--; // 左括号出栈
				}

				else if (CompareOperator(stack1[top1 - 1], s[i]) == 0) stack1[top1++] = s[i];// 栈顶元素优先级小于当前元素，当前元素入栈
				else if (CompareOperator(stack1[top1 - 1], s[i]) == 1)                       // 栈顶元素优先级大于当前元素，出栈
				{
					RPN[lengthRPN++] = BinaryTreeNode(stack1[--top1]);                       // 将栈顶运算符转换为节点后加入逆波兰式，同时弹出栈顶元素
					i--;                                                                     // i--，重新对此运算符重复上述操作，直到遇到发现更低优先级的元素(或者栈为空)为止
				}
			}
		}





		/*将逆波兰式转换为表达式树*/

		BinaryTreeNode stack2[1024];     // 存放节点的栈
		int top2 = 0;                    // 栈顶 

		// 遍历逆波兰式
		for(int i=0;i<lengthRPN;i++)
		{
			// 遇到数字，直接入栈
			if (RPN[i].data.infoType == isNumber) stack2[top2++] = RPN[i];

			// 遇到运算符
			if (RPN[i].data.infoType == isOperation)
			{
				BinaryTreeNode* rc = new BinaryTreeNode(stack2[--top2]);  // 取栈顶元素为rightChild
				BinaryTreeNode* lc = new BinaryTreeNode(stack2[--top2]);  // 取次顶元素为leftChild
				BinaryTreeNode* parent = new BinaryTreeNode(RPN[i]);      // 记录父节点为当前运算符节点
				MergeTree(*lc, *rc, *parent);                             // 构建父子关系
				stack2[top2++] = *parent;                                 // parent入栈
				root = parent;                                            // 更新根节点
			}
		}
	}

	// 合并bt1 bt2两节点至parent节点
	void MergeTree(BinaryTreeNode& bt1, BinaryTreeNode& bt2, BinaryTreeNode& parent)
	{
		// 左右子树位置设置
		parent.leftChild = &bt1, parent.rightChild = &bt2;
	}

	// 以下为三种遍历输出的方式，为保证输出后可以分辨小数、多位整数，每个元素之间多输出了一个空格以分割
	// 前序遍历
	void preOrder(BinaryTreeNode* current)
	{
		if (current == NULL) return;
		cout << *current << ' ';
		preOrder(current->leftChild);
		preOrder(current->rightChild);
	}
	// 中序遍历
	void inOrder(BinaryTreeNode* current)
	{
		if (current == NULL) return;
		inOrder(current->leftChild);
		cout << *current << ' ';
		inOrder(current->rightChild);
	}
	// 后序遍历
	void postOrder(BinaryTreeNode* current)
	{
		if (current == NULL) return;
		postOrder(current->leftChild);
		postOrder(current->rightChild);
		cout << *current << ' ';
	}

	BinaryTreeNode* Root() { return root; }
private:
	BinaryTreeNode* root;

	// 运算符优先级比较
	bool CompareOperator(const char a, const char b)
	{
		if (a == '*' || a == '/')
		{
			if (b == '(') return 0;
			else return 1;
		}
		else if (a == '+' || a == '-')
		{
			if (b == '+' || b == '-' || b == ')') return 1;
			else return 0;
		}
		else if (a == '(')  return 0;
	}
};

int main()
{
	cout << "本程序支持输入小数、多位整数，为方便分辨数字，在输出时相邻元素之间有空格" << endl;
	cout << "请输入表达式(请使用英文括号)： ";
	string expression;
	cin >> expression;
	BinaryTree expressionTree(expression);

	cout << "波兰表达式： ";
	expressionTree.preOrder(expressionTree.Root());
	cout << endl << "中缀表达式： ";
	expressionTree.inOrder(expressionTree.Root());
	cout << endl << "逆波兰表达式：";
	expressionTree.postOrder(expressionTree.Root());
	cout << endl << endl;
	system("pause");
	return 0;
}