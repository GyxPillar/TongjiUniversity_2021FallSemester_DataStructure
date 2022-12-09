/*关于神经网络*/
/*
1.简述
	神经网络的概念来自仿生学，在反向传播算法提出后，以神经网络为主的连接主义逐渐显露头角
	在反向传播之前，对于感知机（神经网络）还有其他的优化方法，如本文所使用的遗传算法

2. 为什么采用卷积神经网络：
	使用卷积神经网络的目的是给ai提供一个能够大概评估每一步走子分数的函数，从而根据这个函数ai可以选择走子的方法并进行决策
3.关于卷积：
	以一维的卷积为例，可以表示成：
	y（x）=	∫f（t）g（x-t）
4.为什么使用卷积：
	通俗的说，卷积可以提取在函数分布中的空间不变性特征
	在图片中，卷积则可以提取图片中某一种的一种分布，而不用考虑分布出现的位置
	棋盘中，一些棋子的分布同样具有一定的不变性，因此通过卷积来提取棋盘中的不变性特征
	同时注意到，连接主义极其神经网络可以对函数有一个较好的拟合，而卷积神经网络可以提取较深层的特征
	因此使用卷积神经网络的模型作为ai为局面打分的函数
5.为什么不使用反向传播进行优化
	反向传播主要用于监督学习，即已有样本的label，根据label和预测值来求lossfuction的值和梯度进行优化
	然而在博弈中我们无法先验地知道每一种局面的label（即便我们人为给每一个棋盘打分也未必合理）
	因此我们难以获得一个有效的lossfuction的梯度，也就无法通过反向传播进行优化捏

	然而通过遗传算法，让各个染色体对应的网络对弈，通过筛选和变异，则能在解空间内挑选出一个能获得相对较好的打分函数的网络
6.网络结构
	本算法的网络结构借鉴了leNet的结构，使用了2层忽略边界的3*3卷积层和2*2max池化层，以及三层全连接层，最后获得一个函数值作为对棋盘局面的打分

*/

/*关于遗传算法*/
/*
关于遗传算法流程和原理不再赘述，详见
https://zh.wikipedia.org/wiki/%E9%81%97%E4%BC%A0%E7%AE%97%E6%B3%95
https://en.wikipedia.org/wiki/Genetic_algorithm
 （来自维基百科）

关于以下代码以及算法：
1：遗传算法中染色体的编码和解码
1.1：染色体编码
	本算法中采用二进制编码

	每一个数字用一个11位的二进制数表示
	其第一位为符号位，1为负0为正
	剩下的十位按顺序转化为int值后，除以1024，使其为一个-1到1的浮点数

1.2：关于染色体长度
	染色体长度为(2 * 5 * 3 * 3			+ 22 * 25		 + 25 * 25      +		25 * 1) * 11;
	其中，11为每个参数的编码长度
	2*5*3*3为卷积层的参数的数目
	22*25、22*25、25*1
	分别为连接层参数的数目
2:关于种群
	4.1染色体变异
		每轮迭代中染色体除最优个体均变异，每个染色体中的每个编码均根据变异率进行一次‘非’运算
	4.2染色体交叉
		对每一个染色体，按一定交换率随机挑选一个染色体进行交换，交换细节详见类chromosome 的Crossover函数
	4.3染色体选择
		让每一个染色体相互对弈获得每个染色体的胜率，按胜率，根据赌徒轮盘法选择染色体，对种群进行迭代（来自csdn）
		https://blog.csdn.net/xuxinrk/article/details/80158786

*/


#define _CRT_SECURE_NO_WARNINGS
#pragma GCC optimize(3)
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <fstream>
#include <mem.h>
#include <cstring>
using namespace std;
#define GENERATION 2000
#define CHROMOSOME_NUMBER  11
#define VARIATION_RATE 0.0005
#define CROSSOVER_RATE 0.4
struct myPair;
using Point = myPair;
using Position = myPair;

template <typename T>
struct Matrix;
struct ConvolutionNeuralNet;
struct ChessAction;
struct ChessActionChooseTreeNode;
struct  ChessMap;
struct ChessGameEstimate;
class chromosome;
template<int size = 3, int step = 1>
struct ConvolutionalKernel;
template<int step = 2>
struct PollingMethod;
//提供一个有序对，可作为坐标，点等//作为demo，没有使用std：：pair减少不必要封装
struct myPair
{
	int x, y;
	myPair(const int x = 0, const int y = 0)
	{
		this->x = x;
		this->y = y;
	}
	myPair(const myPair& toCopy)
	{
		this->x = toCopy.x;
		this->y = toCopy.y;
	}
	myPair& operator =(const myPair& toCopy)
	{
		this->x = toCopy.x;
		this->y = toCopy.y;
		return *this;
	}
	//判断两个点是不是相同
	bool operator ==(const myPair& toCompare)
	{
		return(x == toCompare.x && y == toCompare.y);
	}
	bool operator !=(const myPair& toCompare)
	{
		return !(*this == toCompare);
	}
};

//矩阵，用于神经网络计算
template <typename T>
struct Matrix
{

	T** data;
	unsigned int row;
	unsigned int column;
	Matrix(const  unsigned int row = 2, const  unsigned int column = 2, const T x = 0) :
		row(row > 0 ? row : 1),
		column(column > 0 ? column : 1)
	{
		data = new T * [row];
		for (unsigned int i = 0; i < row; i++)
			data[i] = new T[column];
		for (unsigned int i = 0; i < row; i++)
			for (unsigned int k = 0; k < column; k++)
				data[i][k] = x;
	}
	explicit Matrix(const myPair& size) :
		row(size.y),
		column(size.x)
	{
		data = new T * [size.y];
		for (unsigned int i = 0; i < size.y; i++)
			data[i] = new T[size.x];
		for (unsigned int i = 0; i < size.y; i++)
			for (unsigned int k = 0; k < size.x; k++)
				data[i][k] = 0;
	}
	Matrix(const Matrix& oldMatrix) :
		row(oldMatrix.row),
		column(oldMatrix.column)
	{
		data = new T * [row];

		for (unsigned int i = 0; i < row; i++)
			data[i] = new T[column];
		for (unsigned int i = 0; i < row; i++)
			for (unsigned int k = 0; k < column; k++)
			{
				data[i][k] = oldMatrix.data[i][k];
			}
	}
	Matrix& operator =(const Matrix& matrix)
	{

		for (unsigned int i = 0; i < row; i++)
		{
			T* toDELETE = data[i];
			delete[] toDELETE;
		}
		T** toDELETE = data;
		delete[] toDELETE;


		row = matrix.row;
		column = matrix.column;

		data = new T * [row];
		for (unsigned int i = 0; i < row; i++)
			data[i] = new T[column];

		for (unsigned int i = 0; i < row; i++)
			for (unsigned int k = 0; k < column; k++)
				data[i][k] = matrix.data[i][k];

		return *this;
	}
	~Matrix()
	{
		for (unsigned int i = 0; i < row; i++)
		{
			delete[] data[i];
		}

		delete[] data;
	}
	const Matrix operator +(const Matrix& otherMartix)const
	{
		if (this->row != otherMartix.row || this->column != otherMartix.column)
			return Matrix(0, 0);
		Matrix toReturn(*this);
		for (unsigned int i = 0; i < this->row; i++)
			for (unsigned int k = 0; k < this->column; k++)
				toReturn.data[i][k] += otherMartix.data[i][k];
		return toReturn;
	}
	const Matrix operator -(const Matrix& otherMartix)const
	{
		if (this->row != otherMartix.row || this->column != otherMartix.column)
			return Matrix(0, 0);
		Matrix toReturn(*this);
		for (unsigned int i = 0; i < this->row; i++)
			for (unsigned int k = 0; k < this->column; k++)
				toReturn.data[i][k] -= otherMartix.data[i][k];
		return toReturn;
	}
	const Matrix operator *(const Matrix& otherMatrix)const
	{
		if (this->column != otherMatrix.row)
			return Matrix(0, 0);
		Matrix<T> toReturn(this->row, otherMatrix.column, 0);
		for (unsigned int i = 0; i < toReturn.row; i++)
			for (unsigned int k = 0; k < toReturn.column; k++)
				for (unsigned int j = 0; j < this->column; j++)
					toReturn.data[i][k] += this->data[i][j] * otherMatrix.data[j][k];

		return toReturn;
	}
	const Matrix operator *(const double number)const
	{
		Matrix toReturn(*this);
		for (unsigned int i = 0; i < row; i++)
			for (unsigned int k = 0; k < column; k++)
				toReturn.data[i][k] *= number;
		return toReturn;

	}
	const Matrix operator /(const double number)const
	{
		Matrix toReturn(*this);
		for (unsigned int i = 0; i < row; i++)
			for (unsigned int k = 0; k < column; k++)
				toReturn.data[i][k] /= number;
		return toReturn;
	}
	const Matrix hardmord(const Matrix& otherMatrix)const
	{

		Matrix toReturn(*this);

		for (int i = 0; i < row; i++)
			for (int k = 0; k < column; k++)
			{
				toReturn.data[i][k] = data[i][k] * otherMatrix.data[i][k];
			}
		return toReturn;
	}
	friend const Matrix operator * (const double number, const Matrix& matrix)
	{
		Matrix toReturn(matrix);
		for (unsigned int i = 0; i < matrix.row; i++)
			for (unsigned int k = 0; k < matrix.column; k++)
				toReturn.data[i][k] *= T(number);
		return toReturn;
	}
	const Matrix transpose()const
	{

		Matrix toReturn(this->column, this->row);
		for (unsigned int i = 0; i < row; i++)
		{
			for (unsigned int k = 0; k < column; k++)
			{
				toReturn.data[k][i] = this->data[i][k];
			}
		}
		return toReturn;
	}
	T& at(const unsigned int row, const unsigned  int column)//输入错误默认（0，0）
	{
		if (row<0 || row>this->row || column<0 || column>this->column)
			return data[0][0];
		return data[row][column];
	}
	const T& at(const unsigned int row, const unsigned  int column)const//输入错误默认（0，0）
	{
		if (row<0 || row>this->row || column<0 || column>this->column)
			return data[0][0];
		return data[row][column];
	}
	friend std::ostream& operator <<(std::ostream& os, const Matrix& matrix)
	{
		for (unsigned int i = 0; i < matrix.row; i++)
		{
			for (unsigned int j = 0; j < matrix.column; j++)
			{
				os << setw(3) << (fabs(matrix.data[i][j] - 0) < 1e-6 ? 0 : matrix.data[i][j]) << " ";
			}
			os << std::endl;
		}
		return os;
	}
	const T determinant()const
	{
		if (this->row != this->column)
			return 0;
		T returnValue = 0;

		if (this->row == 1 && this->column == 1)
			return this->data[0][0];

		for (int i = 0; i < this->column; i++)
		{
			returnValue += data[0][i] * ((i + 0) % 2 == 0 ? 1 : -1) * this->CovalentFormula(0, i).determinant();
		}
		return returnValue;
	}//行列式

	const Matrix<T> CovalentFormula(const int row, const int column)const//[0][0]对应a（row=0） （column=0）
	{
		Matrix <T> toReturn(this->row - 1, this->column - 1);
		for (int i1 = 0, i2 = 0; i2 < this->row; i1++, i2++)
		{
			if (i2 == row)
				i2++;
			if (i2 == this->row)
				break;
			for (int k1 = 0, k2 = 0; k2 < this->column; k1++, k2++)
			{
				if (k2 == column)
					k2++;
				if (k2 == this->column)
					break;
				toReturn.data[i1][k1] = this->data[i2][k2];
			}
		}
		return toReturn;
	}//余子式

	const Matrix<T> companionMatrix()const
	{
		if (this->column != this->row)
			return Matrix(0, 0);
		Matrix toReturn(*this);
		for (int i = 0; i < toReturn.row; i++)
			for (int k = 0; k < toReturn.column; k++)
			{
				toReturn.data[k][i] = ((i + k) % 2 == 0 ? 1 : -1) * (this->CovalentFormula(i, k).determinant());
			}
		return toReturn;

	}//伴随矩阵

	const Matrix<T> InverseMatrix()
	{
		if (this->column != this->row || fabs(this->determinant() - 0) < 1e-6)
			return Matrix(0, 0);
		return this->companionMatrix() / double(this->determinant());
	}//逆

	void Mrand()
	{
		for (unsigned int i = 0; i < this->row; i++)
		{
			for (unsigned int k = 0; k < this->column; k++)
			{
				data[i][k] = rand() % 80000 / 10000.0 - 4;
			}
		}
	}
};
//以下是可能用到的用于神经网络计算的激活函数、和其梯度
const Matrix<double>log(const Matrix<double>& m)
{
	Matrix<double> toReturn(m);
	for (unsigned int i = 0; i < m.row; i++)
	{
		for (unsigned int k = 0; k < m.column; k++)
		{
			toReturn.data[i][k] = log(m.data[i][k] + 0.0001);
		}
	}
	return toReturn;
}
const Matrix<double> SreLu(const Matrix<double>& m)//lecky relu 
{
	Matrix<double> toReturn(m);
	for (unsigned int i = 0; i < m.row; i++)
	{
		for (unsigned int k = 0; k < m.column; k++)
		{
			toReturn.data[i][k] = m.data[i][k] > 0 ? 1 * m.data[i][k] : 0.1 * m.data[i][k];
		}
	}
	return toReturn;
}
const Matrix<double> differential_SreLu(const Matrix<double>& m)//微分形式
{

	Matrix<double> toReturn(m);
	for (unsigned int i = 0; i < m.row; i++)
	{
		for (unsigned int k = 0; k < m.column; k++)
		{
			toReturn.data[i][k] = m.data[i][k] > 0 ? 1 : 0.1;
		}
	}
	return toReturn;
}
const Matrix<double> sigmoid(const Matrix<double>& m)//logistic 回归用于胜率分析
{
	Matrix<double> toReturn(m);
	for (unsigned int i = 0; i < m.row; i++)
	{
		for (unsigned int k = 0; k < m.column; k++)
		{
			toReturn.data[i][k] = 1.0 / (1.0 + exp(0 - m.data[i][k]));
		}
	}
	return toReturn;
}
const Matrix<double> differential_sigmoid(const Matrix<double>& m)
{
	Matrix<double> toReturn(m);
	for (unsigned int i = 0; i < m.row; i++)
	{
		for (unsigned int k = 0; k < m.column; k++)
		{
			toReturn.data[i][k] =
				(1.0 - 1.0 / (1.0 + exp(0 - m.data[i][k]))) * (1 / (1 + exp(0 - m.data[i][k])));
		}
	}
	return toReturn;
}

//棋子在棋盘上的行为，主要是为了ai进行操作
struct ChessAction
{
	//initial position when a chess act

	myPair initialPosition;

	//aim position when a chess act

	myPair aimPosition;

	// action type of chess   such as: Move   Eat

	// default action type is -1 to express no action

	int actionType;
	//move=0,eat=1;
	static const int Move = 0;
	static const int Eat = 1;

	ChessAction()
	{
		actionType = -1;
	}

	ChessAction(const myPair& initialPosition, const myPair& aimPosition, const int action)
	{
		this->initialPosition = initialPosition;
		this->aimPosition = aimPosition;
		actionType = action;

	}
	friend ostream& operator<<(ostream& os, const ChessAction& ac)
	{
		os << "ini" << ac.initialPosition.x << "," << ac.initialPosition.y << "  aim" << ac.aimPosition.x << "," << ac.aimPosition.y;
		return os;
	}
};

//根据跳棋规则，需要有吃则吃，且必须走吃最多的那一种走子方式，该类主要是用于寻找吃最多的路线
struct ChessActionChooseTreeNode
{
	//节点在树中的层数
	int level;

	//每个节点在被搜索时的优先级
	int priority;

	//是不是末端节点
	int isOver;

	//从parent节点对应的map到这个节点对应的map 需要的action
	ChessAction parentsToThis;

	//指向父节点的地址
	ChessActionChooseTreeNode* parent;

	//指向子节点的指针
	std::vector< ChessActionChooseTreeNode*> children;

public:

	//默认层数，优先级均为0
	ChessActionChooseTreeNode()
	{
		level = 0;
		isOver = 0;
		priority = 0;
		parent = NULL;
	}

	ChessActionChooseTreeNode* const Parent()
	{
		return this->parent;
	}

	ChessActionChooseTreeNode* const Child(const int i)
	{
		return this->children.at(i);
	}

	//在children中加入child节点，并将child的父节点设为该节点
	void setChild(ChessActionChooseTreeNode* const child)
	{
		children.push_back(child);
		children[children.size() - 1]->parent = this;
	}

	/// 将这个节点中的action换为指定action，不安全
	void setAction(const ChessAction& action)
	{
		parentsToThis = action;
	}

	//将parent设为该节点的父节点，并在parent的children中插入该节点
	void setParent(ChessActionChooseTreeNode* const parent)
	{
		parent->children.push_back(this);
		this->parent = parent;
	}

	//设置该节点的优先级
	void setLevel(const int level)
	{
		this->level = level;
	}

	/*---------------------------------------------------------------------------
	函数名	：backPropogation
	功能	：由当前节点开始，找到底层节点，
			  令每一级节点的父节点的优先级为其子节点的最大值，
			  从而使有最长路径的节点的一枝上的节点都为最大值，
			  从而方便寻找搜索最长路径
	参数	：
	返回值	：void
	说明	：更新每个节点的优先级是为了mostEatSearch中找到最大路径
	---------------------------------------------------------------------------*/
	void backPropagation()
	{
		if (isOver)
			return;

		for (unsigned int i = 0; i < children.size(); i++)
		{
			children[i]->backPropagation();
			priority = max(priority, children[i]->priority);
		}
	}

	/*---------------------------------------------------------------------------
	函数名	：findLastNodeInLongestRoad
	功能	：由当前节点开始，找到层数最高的节点（层数相同时都返回到vector中）
	参数	：
	返回值	：vector< ChessActionChooseTreeNode* >
	说明	：返回层数最高那一层的所有节点,必须是在backpropagation后调用
	---------------------------------------------------------------------------*/
	std::vector< ChessActionChooseTreeNode* > findLastNodeInLongestRoad(const int priority)
	{
		//先声明一个将要返回的vector
		std::vector< ChessActionChooseTreeNode* > toReturn;

		//反向传播后，最高层的优先级等于层数
		if (this->priority == priority && this->priority == this->level && this->level != 0)
		{
			//在vector中加入本节点的action并返回
			toReturn.push_back(this);
			return toReturn;
		}
		//将当前节点的所有子节点中返回的所有vector中的节点地址加入当前vector，并返回
		for (unsigned int i = 0; i < children.size(); i++)
		{
			for (unsigned int k = 0; k < children[i]->findLastNodeInLongestRoad(priority).size(); k++)
				toReturn.push_back(children[i]->findLastNodeInLongestRoad(priority).at(k));
		}
		return toReturn;
	}

	/*---------------------------------------------------------------------------
	函数名	：findLongestChessRoadFromLeaf
	功能	：由当前叶节点开始，向上回溯，到根节点，记录每一个点的action
			  返回从根节点到叶节点的所有action
	参数	：ChessActionChooseTreeNode* leafNode
	返回值	：vector< ChessAction >
	说明	：返回层数最高那一层的所有节点,必须是在backpropagation后调用
	---------------------------------------------------------------------------*/
	friend 	std::vector< ChessAction> findLongestChessRoadFromLeaf(ChessActionChooseTreeNode* leafNode)
	{

		std::vector<ChessAction> toReturn;

		while (leafNode->level != 0)
		{
			std::vector<ChessAction>::iterator it = toReturn.begin();
			toReturn.insert(it, leafNode->parentsToThis);
			leafNode = leafNode->parent;
		}

		return toReturn;
	}

	ChessActionChooseTreeNode(const ChessActionChooseTreeNode& Node)
	{
		this->isOver = Node.isOver;

		this->level = Node.isOver;

		this->priority = Node.priority;

		this->parentsToThis = Node.parentsToThis;

		this->parent = NULL;

		if (!this->isOver)
		{
			for (unsigned int i = 0; i < Node.children.size(); i++)
			{
				ChessActionChooseTreeNode* childrenToPushback = new ChessActionChooseTreeNode(*Node.children[i]);
				this->setChild(childrenToPushback);
			}
		}
	}

	ChessActionChooseTreeNode& operator =(const ChessActionChooseTreeNode& Node)
	{

		this->isOver = Node.isOver;

		this->level = Node.isOver;

		this->priority = Node.priority;

		this->parentsToThis = Node.parentsToThis;

		this->parent = NULL;

		if (!this->isOver)
		{
			for (unsigned int i = 0; i < Node.children.size(); i++)
			{
				ChessActionChooseTreeNode* childrenToPushback = new ChessActionChooseTreeNode(*Node.children[i]);
				this->setChild(childrenToPushback);
			}
		}

		return *this;
	}

	~ChessActionChooseTreeNode()
	{

#ifdef DEBUG

		//debug时方便看节点有没有成功释放
		std::cout << "DEBUG:释放<ChessActionChooseTreeNode>----------" << endl << endl;
		std::cout << "当前地址:" << (int*)(this) << endl;;
		std::cout << "层数:" << this->level << endl;
		std::cout << "优先级:" << this->priority << endl;
		std::cout << "执行的操作:" << [](int type)->string
		{
			switch (type)
			{
				case -1:
					return "末端节点";
				case 0:
					return "Move";
				case 1:
					return "Jump";
			}
			return "wrong type";
		}(this->parentsToThis.actionType);

		if (parentsToThis.actionType != -1)
		{
			std::cout << "(" << this->parentsToThis.initialPosition.x << "," << parentsToThis.initialPosition.y << ") to";
			std::cout << "(" << this->parentsToThis.aimPosition.x << "," << parentsToThis.aimPosition.y << ")" << endl;
		}

		else cout << endl;

		std::cout << "子节点:" << endl;

		for (unsigned int i = 0; i < children.size(); i++)
		{
			cout << (int*)children[i] << " ";
		}

		cout << endl << endl;



#endif // !DEBUG

		if (this->parent != NULL)
		{
			ChessActionChooseTreeNode* parentPoint = this->parent;
			for (std::vector< ChessActionChooseTreeNode* >::iterator it = parentPoint->children.begin(); it != parentPoint->children.end(); it++)
			{
				if (*it == this)
				{
					parentPoint->children.erase(it);
					break;
				}

				if (parentPoint->children.size() == 0)
					break;
			}
		}

		this->parent = NULL;

		for (unsigned int i = 0; i < this->children.size(); )
		{
			delete children[i];
		}
	}
};

//棋盘
struct  ChessMap
{
public:
	static const int noChess = 0, normalWhite = 1, normalBlack = 2, kingWhite = 3, kingBlack = 4;

	//private:
	int chess[8][8];
public:
	//用于判断当前棋局结束时谁胜谁负
	double judge()
	{
		int blackNumber = 0;
		int whiteNumber = 0;
		//记录场上棋子数量，多的获胜
		for (int i = 0; i < 8; i++)
		{
			for (int k = 0; k < 8; k++)
			{
				if (chess[i][k] == 1 || chess[i][k] == 3)
				{
					whiteNumber++;

				}
				if (chess[i][k] == 2 || chess[i][k] == 4)
				{
					blackNumber++;

				}
			}
		}
		
		if (blackNumber > whiteNumber)
			return 1.0;
		else if (blackNumber == whiteNumber)
			return 0.5;
		else return 0;
	}
	bool operator==(const ChessMap& toCompare)const
	{
		for (int i = 0; i < 8; i++)
		{
			for (int k = 0; k < 8; k++)
			{
				if (this->chess[i][k] != toCompare.chess[i][k])
					return 0;
			}
		}
		return 1;
	}
	ChessMap& operator=(const ChessMap& map)
	{
		for (int row = 0; row < 8; row++)
			for (int column = 0; column < 8; column++)
			{
				this->chess[row][column] = map.chess[row][column];
			}
		return *this;
	}

	ChessMap()
	{
		start();
	}

	ChessMap(const ChessMap& map)
	{
		for (int row = 0; row < 8; row++)
			for (int column = 0; column < 8; column++)
			{
				this->chess[row][column] = map.chess[row][column];
			}
	}

	/*---------------------------------------------------------------------------
	函数名	：start
	功能	：初始化棋盘
	参数	：void
	返回值	：void
	说明	：按照题目要求，上区为白，下为黑
	---------------------------------------------------------------------------*/
	void start()
	{
		for (int i = 0; i < 8; i++)
			for (int k = 0; k < 8; k++)
			{
				chess[i][k] = 0;
			}
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 8; j += 2)
			{
				chess[i][j + (i + 1) % 2] = normalWhite;
			}
		}
		for (int i = 5; i < 8; i++)
		{
			for (int j = 0; j < 8; j += 2)
			{
				chess[i][j + (i + 1) % 2] = normalBlack;
			}
		}
	}

	/*---------------------------------------------------------------------------
	函数名	：print
	功能	：打印当前的棋盘
	参数	：void
	返回值	：void
	说明	：不清屏
	---------------------------------------------------------------------------*/
	void print()const
	{
		cout << endl;
		for (int i = 0; i < 8; i++)
		{
			for (int k = 0; k < 8; k++)
			{
				if (this->chess[i][k] == 1)
					cout << setw(2) << -1 << " ";
				else if (this->chess[i][k] == 2)
					cout << setw(2) << 1 << " ";
				else if (this->chess[i][k] == 3)
					cout << setw(2) << -2 << " ";
				else if (this->chess[i][k] == 4)
					cout << setw(2) << 2 << " ";
				else cout << setw(2) << 0 << " ";
			}
			cout << endl;
		}
	}

	/*---------------------------------------------------------------------------
	函数名	：at
	功能	：返回ChessMode& Position 位置上的棋子的引用
	参数	：const Position& position
	返回值	：ChessMode& Position 位置上的棋子的引用
	说明	：返回引用，会修改棋盘值
	---------------------------------------------------------------------------*/
	int& at(const Position& position)
	{
		return chess[position.y][position.x];
	}

	/*---------------------------------------------------------------------------
	函数名	：findOnlyMove
	功能	：遍历当前棋盘，找到所有的可移动棋子的Action
			  加入vector中，最后返回整个vector，
	参数	：const int player （当前的玩家的颜色）
	返回值	：std::vector<ChessAction> 返回所有的可移动棋子ChessAction
	说明	：在没有可吃子时使用
	---------------------------------------------------------------------------*/
	vector<ChessAction> findOnlyMove(const int playerNow)
	{
		//创建一个空的vector，准备放入当前棋盘可吃子的action
		typename std::vector<ChessAction > toReturn;
		//遍历整个棋盘，寻找可以吃子的action
		for (int row = 0; row < 8; row++)
			for (int column = 0; column < 8; column++)
			{

				//遍历到的棋子，如果是空，就继续寻找
				if (chess[row][column] == noChess)
					continue;

				//如果当前下棋的人是白方
				if (playerNow == 1)
				{
					//普通棋子
					if (chess[row][column] == normalWhite)
					{
						//看是不是左下或右下有其他棋子
						for (int deltaX = -1; deltaX < 2; deltaX += 2)
						{
							//不能越界
							if (row >= 0 && row + 1 < 8 && deltaX + column > -1 && deltaX + column < 8)
							{
								//满足可以吃的条件，吃之
								if (chess[row + 1][column + deltaX] == noChess)
								{
									toReturn.push_back(ChessAction(Position(column, row), Position(column + deltaX, row + 1), ChessAction::Move));
								}
							}
						}
					}
					//king子
					else if (chess[row][column] == kingWhite)
					{
						// 看是不是四角有其他棋子
						for (int deltaY = -1; deltaY < 2; deltaY += 2)
							for (int deltaX = -1; deltaX < 2; deltaX += 2)
							{
								//不能越界
								if (row + deltaY > -1 && row + deltaY  < 8 && deltaX + column > -1 && deltaX + column < 8)
								{
									//满足可以吃的条件，吃之
									if (chess[row + deltaY][column + deltaX] == noChess)
									{
										toReturn.push_back(ChessAction(Position(column, row), Position(column + deltaX, row + deltaY), ChessAction::Move));
									}
								}
							}
					}
				}
				//黑棋，同上
				else if (playerNow == 2)
				{
					if (chess[row][column] == normalBlack)
					{
						for (int deltaX = -1; deltaX < 2; deltaX += 2)
						{
							if (row > 0 && row < 8 && deltaX + column>-1 && deltaX + column < 8)
							{
								if (chess[row - 1][column + deltaX] == noChess)
								{
									toReturn.push_back(ChessAction(Position(column, row), Position(column + deltaX, row - 1), ChessAction::Move));
								}
							}
						}
					}
					else if (chess[row][column] == kingBlack)
					{
						for (int deltaY = -1; deltaY < 2; deltaY += 2)
							for (int deltaX = -1; deltaX < 2; deltaX += 2)
							{
								if (row + deltaY > -1 && row + deltaY  < 8 && deltaX + column > -1 && deltaX + column < 8)
								{
									if (chess[row + deltaY][column + deltaX] == noChess)
									{
										toReturn.push_back(ChessAction(Position(column, row), Position(column + deltaX, row + deltaY), ChessAction::Move));
									}
								}
							}
					}
				}
			}
		//遍历结束后，所有可移动的步骤都会放在toReturn 中
		return toReturn;
	}

	/*---------------------------------------------------------------------------
	函数名	：changeChess
	功能	：直接将position处的棋子改为mode
	参数	：
	--const Position& position（要改变的位置）
	--ChessMode mode（要将当前位置更改为什么）
	返回值	：void
	说明	：直接改变当前map的值
	---------------------------------------------------------------------------*/
	void changeChess(const Position& position, char mode)
	{
		chess[position.y][position.x] = mode;
	}

	/*---------------------------------------------------------------------------
	函数名	： actChess
	功能	：返回当前棋盘在经历ChessAction后的新棋盘
	参数	：const ChessAction& action 要对当前棋盘操作的棋盘
	返回值	：ChessMap 经历action后的新棋盘
	说明	： 不改变源对象的值
	---------------------------------------------------------------------------*/

	ChessMap actChess(const ChessAction& action)const
	{
		//复制当前的对象，方便修改，同时不改变源对象
		ChessMap toReturn = *this;
		//如果要进行的操作是 move 移动棋子
		if (int(action.actionType) == ChessAction::Move)
		{
			//将 新棋盘 目标位置的棋子换为 原棋盘 起始位置的棋子
			toReturn.changeChess(action.aimPosition, chess[action.initialPosition.y][action.initialPosition.x]);
			//将 新棋盘 目标位置的棋子换为noChess
			toReturn.changeChess(action.initialPosition, ChessMap::noChess);
		}
		else if (int(action.actionType) == ChessAction::Eat)
		{
			//将 新棋盘 目标位置的棋子换为 原棋盘 起始位置的棋子
			toReturn.changeChess(action.aimPosition, chess[action.initialPosition.y][action.initialPosition.x]);
			//将 新棋盘 目标位置的棋子换为noChess
			toReturn.changeChess(action.initialPosition, ChessMap::noChess);
			//吃掉中间的棋子
			toReturn.changeChess(Position((action.initialPosition.x + action.aimPosition.x) / 2, (action.initialPosition.y + action.aimPosition.y) / 2), ChessMap::noChess);
		}
		return toReturn;
	}

	/*---------------------------------------------------------------------------
	函数名	：findEating
	功能	：遍历当前棋盘，找到所有的可吃子的 某次ChessAction
			  （指在当前局面下，进行ChessAction，可以吃子）
			  加入vector中，最后返回整个vector，
	参数	：const int player （当前的玩家的颜色）
	返回值	：std::vector<ChessAction> 返回所有的可吃子的ChessAction
	说明	：仅返回单次棋盘上的情况，不模拟走棋后可能连吃，连吃交给其他函数
	---------------------------------------------------------------------------*/

	typename std::vector<ChessAction > findEating(const int playerNow)const
	{
		//创建一个空的vector，准备放入当前棋盘可吃子的action
		typename std::vector<ChessAction > toReturn;
		//遍历整个棋盘，寻找可以吃子的action
		for (int row = 0; row < 8; row++)
			for (int column = 0; column < 8; column++)
			{
				//遍历到的棋子，如果是空，就继续寻找
				if (chess[row][column] == noChess)
					continue;
				//如果当前下棋的人是白方
				if (playerNow == 1)
				{

					//king子
					if (chess[row][column] == kingWhite || chess[row][column] == normalWhite)
					{
						// 看是不是四角有其他棋子
						for (int deltaY = -1; deltaY < 2; deltaY += 2)
							for (int deltaX = -1; deltaX < 2; deltaX += 2)
							{
								//不能越界
								if (row + deltaY * 2 > -1 && row + deltaY * 2 < 8 && deltaX * 2 + column > -1 && deltaX * 2 + column < 8
									&& row + deltaY > -1 && row + deltaY  < 8 && deltaX + column > -1 && deltaX + column < 8)
								{
									//满足可以吃的条件，吃之
									if ((chess[row + deltaY][column + deltaX] == normalBlack || chess[row + deltaY][column + deltaX] == kingBlack) && chess[row + deltaY * 2][column + deltaX * 2] == noChess)
									{
										toReturn.push_back(ChessAction(Position(column, row), Position(column + deltaX * 2, row + deltaY * 2), ChessAction::Eat));
									}
								}
							}
					}
				}
				else if (playerNow == 2)
				{
					if (chess[row][column] == kingBlack || chess[row][column] == normalBlack)
					{
						for (int deltaY = -1; deltaY < 2; deltaY += 2)
							for (int deltaX = -1; deltaX < 2; deltaX += 2)
							{
								if (row + deltaY * 2 > -1 && row + deltaY * 2 < 8 && deltaX * 2 + column > -1 && deltaX * 2 + column < 8
									&& row + deltaY > -1 && row + deltaY  < 8 && deltaX + column > -1 && deltaX + column < 8)
								{
									if ((chess[row + deltaY][column + deltaX] == normalWhite || chess[row + deltaY][column + deltaX] == kingWhite) && chess[row + deltaY * 2][column + deltaX * 2] == noChess)
									{
										toReturn.push_back(ChessAction(Position(column, row), Position(column + deltaX * 2, row + deltaY * 2), ChessAction::Eat));
									}
								}
							}
					}
				}
			}
		return toReturn;
	}


	/*---------------------------------------------------------------------------
	函数名	：chessRoadSearch
	功能	：寻找所有当前玩家可行的棋路，建立路径树
			  如果路径树的根节点只有一个子节点，说明没有可吃的子
	参数	：const int player （当前的玩家的颜色）,const int level（要计算的节点在树中的高度）
	返回值	：ChessActionChooseTreeNode* 返回当前的节点
	说明	：通过递归，将当前节点插入上一节点，最终插入root节点
	---------------------------------------------------------------------------*/

	ChessActionChooseTreeNode* const chessRoadSearch(const int playerNow, const Position& childInitialPosition, const int level)
	{
		//创建一个节点，最后返回
		ChessActionChooseTreeNode* Node = new ChessActionChooseTreeNode;

		//由于每次搜索时，树深度增加，故令优先级和层数为当前层数
		Node->level = level;
		Node->priority = level;

		//寻找当前局面下的所有的可吃子的步骤
		std::vector<ChessAction> feasibleAction = this->findEating(playerNow);
		//不能吃子则结束
		if (feasibleAction.size() < 1)
		{
			ChessActionChooseTreeNode* childNode = new ChessActionChooseTreeNode;
			childNode->isOver = 1;
			childNode->level = level + 1;
			childNode->priority = level;
			Node->setChild(childNode);
		}
		//寻找能否吃子，能则在树上添加子节点
		for (unsigned int i = 0; i < feasibleAction.size(); i++)
		{

			ChessMap afterAction = this->actChess(feasibleAction.at(i));
			ChessActionChooseTreeNode* childNode = afterAction.chessRoadSearch(playerNow, feasibleAction.at(i).aimPosition, level + 1);
			childNode->parentsToThis = feasibleAction.at(i);
			Node->setChild(childNode);
			if (level != 0 && Node->children[Node->children.size() - 1]->parentsToThis.initialPosition != childInitialPosition)
				delete childNode;
		}
		return Node;
	}

	/*-----------------------------------------------------------------------
	函数名	：mostEatRoad
	功能	：如果有可吃的子，返回当前ChessMap中能吃最多子的路线
	参数	：ChessActionChooseTreeNode  root  参数是之前寻找的可执行的路线树
	返回值	：std::vector<ChessAction>
	说明	：
	------------------------------------------------------------------------*/
	typename std::vector<std::vector<ChessAction> >  mostEatRoad(ChessActionChooseTreeNode* root)
	{
		int max = root->priority;
		typename	std::vector<std::vector<ChessAction> > toReturn;
		std::vector<ChessActionChooseTreeNode*> allTheLeaves = root->findLastNodeInLongestRoad(max);
		for (unsigned int i = 0; i < allTheLeaves.size(); i++)
		{
			toReturn.push_back(findLongestChessRoadFromLeaf(allTheLeaves.at(i)));
		}

		return toReturn;
	}
	/*-----------------------------------------------------------------------
函数名	：check
功能	：检查棋子到对方边界则升级
参数	：void
返回值	：std::vector<ChessAction>
说明	：
------------------------------------------------------------------------*/
	void check()
	{
		for (int i = 0; i < 8; i += 7)
		{
			for (int k = 0; k < 8; k++)
			{
				if (i == 0 && chess[i][k] == normalBlack)
				{
					chess[i][k] = kingBlack;
				}
				else if (i == 7 && chess[i][k] == normalWhite)
				{
					chess[i][k] = kingWhite;
				}
			}
		}
	}
};

//染色体，用于遗传算法中使用
class chromosome
{
public:
	double winrate = 0;
	bool* content;
	static const int maxContent = (2 * 5 * 3 * 3 + 22 * 25 + 25 * 25 + 25) * 11;
	static const int numberSize = 11;
	static const int linkedLevelContent_1 = (2 * 5 * 3 * 3) * 11;
	static const int linkedLevelContent_2 = 2 * 5 * 3 * 3 * 11 + 22 * 25 * 11;
	static const int linkedLevelContent_3 = 2 * 5 * 3 * 3 * 11 + 22 * 25 * 11 + 25 * 25 * 11;


public:
	/*-----------------------------------------------------------------------
	函数名	：getNumber
	功能	：从start开始根据编码规则将编码转换为参数
	参数	：const int start(起始点的位置)
	返回值	：double(转换后的编码)
	说明	：
	每个数字以11位的bool值储存
	第一位为 符号位
	其他位 按二进制 转换为int 后除以1024，保证转换的数字为精度为10e-3的[-1 --- 1]的数字
	------------------------------------------------------------------------*/
	double getNumber(const int& start)const
	{

		int c = content[start + 1];
		for (int i = 2; i < 11; i++)
			c = (c << 1) + content[start + i];
		if (content[start] == 1)
		{
			c = -c;
		}
		return c / 1024.0;
	}

	chromosome()
	{
		content = new bool[maxContent];
		for (int i = 0; i < maxContent; i++)
		{
			content[i] = rand() % 2;
		}
	}
	chromosome& operator=(const chromosome& c)
	{
		winrate = 0;
		bool* toDelete = this->content;
		content = new bool[maxContent];
		memcpy(content, c.content, maxContent);
		delete[]toDelete;
		return *this;
	}
	chromosome(const chromosome& c)
	{
		winrate = 0;
		content = new bool[maxContent];
		memcpy(content, c.content, maxContent);
	}


	/*-----------------------------------------------------------------------
	函数名	：variation
	功能	：染色体的变异
	参数	：const double& rate
	返回值	：void
	说明	：
	按照预设的变异率，对染色体中每一个编码进行变异
	------------------------------------------------------------------------*/
	void variation(const double& rate)//更改编码不会导致重复
	{
		for (int i = 0; i < maxContent; i++)
			if (rand() % 10000 / 10000.0 < rate)
			{
#ifdef DEBUG
				cout << "variation" << endl;
#endif // DEBUG
				content[i] = !content[i];
			}
	}
	//直接交叉不会导致缺失和重复


		/*-----------------------------------------------------------------------
	函数名	：crossOver
	功能	：染色体的交换
	参数	：const chromosome&ch ，int start ，int length
	返回值	：void
	说明	：
	按照预设的交换率，对染色体中每一个编码进行变异
	------------------------------------------------------------------------*/
	void crossOver(chromosome& ch, int start, int length)
	{
		//保证start和length不超出染色体范围
		start = start < 0 ? 0 : start;
		length = length < 0 ? 0 : length;
		start = start > ch.maxContent ? maxContent / 2 : start;
		length = length > ch.maxContent ? maxContent / 2 : length;
		//从start开始 源染色体和ch染色体交换length长度的编码
		bool* n = new bool[length + start > maxContent ? maxContent - start - 1 : length];
		bool* ch1Start = content + start;
		bool* ch2Start = ch.content + start;

		memcpy(n, ch2Start, length + start > maxContent ? maxContent - start - 1 : length);
		memcpy(ch2Start, ch1Start, length + start > maxContent ? maxContent - start - 1 : length);
		memcpy(ch1Start, n, length + start > maxContent ? maxContent - start - 1 : length);
		delete[]n;
	}
	~chromosome()
	{
		delete[] content;
	}
};

//卷积核，对所选矩阵进行卷积操作
template<int/*大小*/ size, /*步长*/ int step>
struct ConvolutionalKernel 
{
	//默认卷积层为size*size的矩阵
	Matrix<double> kernel = Matrix<double>(size, size);
public:


	/*-----------------------------------------------------------------------
	函数名	：convolution
	功能	：对指定矩阵按特定步长和大小进行卷积
	参数	：const Matrix<double>& toConvolution
	返回值	：	Matrix<double> 得到的矩阵
	说明	：
	//进行不补充边界的卷积（考虑的棋盘的边界比较特殊）
	------------------------------------------------------------------------*/
	Matrix<double> convolution(const Matrix<double>& toConvolution)
	{
		Matrix<double> convolutioned(toConvolution.row - 2, toConvolution.column - 2);
		//对所选矩阵遍历卷积
		for (int row = 1; row < toConvolution.row - 1; row += step)
		{
			for (int column = 1; column < toConvolution.column - 1; column += step)
			{
				double total = 0;
				for (int i = 0 - (size / 2); i < size - (size / 2); i++)
				{
					for (int k = 0 - (size / 2); k < size - (size / 2); k++)
					{
						if (i + row < 0 || i + row >= toConvolution.row)
							continue;
						if (k + column < 0 || k + column >= toConvolution.column)
							continue;
						total += kernel.at(i + (size / 2), k + (size / 2)) * toConvolution.at(i + row, k + column);

					}
				}
				convolutioned.at(row - 1, column - 1) = total / 9.0;
			}
		}

		return convolutioned;
	}
	//对目标矩阵进行卷积
	Matrix<double> operator()(const Matrix<double>& toConvolution)
	{
		return this->convolution(toConvolution);
	}
};

//用于对所选矩阵进行池化操作
template<int/*池化步长*/step>
struct PollingMethod//按步长池化
{
	static const int maxPol = 1;//max池化//更加凸显纹理特征
	static const int avePol = 2;//平均值池化
	Matrix<double> operator()(const Matrix<double>& m, const int type = 1)
	{
		Matrix<double> toReturn(m.row - step + 1, m.column - step + 1);
		if (type == avePol)
		{
			for (int i = 0; i < m.row - step + 1; i++)
			{
				for (int k = 0; k < m.column - step + 1; k++)
				{
					toReturn.at(i, k) = [&m](int r, int c)->double
					{
						double value = 0;
						for (int i = 0; i < step; i++)
						{
							for (int k = 0; k < step; k++)
							{
								value += m.at(r + i, k + c);
							}
						}
						value /= (step * step);
						return value;
					}(i, k);

				}
			}
		}
		else if (type == maxPol)
		{
			for (int i = 0; i < m.row - step + 1; i++)
			{
				for (int k = 0; k < m.column - step + 1; k++)
				{
					toReturn.at(i, k) = [&m](int r, int c)->double
					{
						double maxValue = 0;
						for (int i = 0; i < step; i++)
						{
							for (int k = 0; k < step; k++)
							{
								maxValue = max(m.at(r + i, k + c), maxValue);
							}
						}
						return maxValue;
					}(i, k);

				}
			}
		}
		return toReturn;
	}
};

//卷积网络
struct ConvolutionNeuralNet
{
	const chromosome* chr;
	ConvolutionalKernel<3> kernel[2][5]; //3*5*3*3  range(-9 ~9)  2*16
	PollingMethod<2> p[2][5];
	Matrix<double> m[3] = { Matrix<double>(5 * 4 + 2,25) ,Matrix<double>(25,25),Matrix<double>(25,1) };
	//初始化，将染色体中的编码转化为参数
	ConvolutionNeuralNet(const chromosome& ch)
	{
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 5; j++)
			{
				int start = 3 * 3 * ch.numberSize * (i * 5 + j);

				for (int k = 0; k < 3; k++)

					for (int l = 0; l < 3; l++)
					{
						kernel[i][j].kernel.at(k, l) = ch.getNumber(start + (k * 3 + l) * ch.numberSize);
					}
			}

		for (int i = 0; i < 22; i++)
			for (int k = 0; k < 25; k++)
				m[0].at(i, k) = ch.getNumber(ch.linkedLevelContent_1 + (i * 22 + k) * ch.numberSize);

		for (int i = 0; i < 25; i++)
			for (int k = 0; k < 25; k++)
				m[1].at(i, k) = ch.getNumber(ch.linkedLevelContent_2 + (i * 25 + k) * ch.numberSize);

		for (int i = 0; i < 25; i++)
			m[2].at(i, 0) = ch.getNumber(ch.linkedLevelContent_3 + i * ch.numberSize);

		chr = &ch;
	}
	//通过卷积神经网络对棋盘进行卷积，并且参考player的颜色，给出当前player当前局面的打分
	double operator()(const ChessMap& map, const int player)//1==white ,-1==black
	{
		Matrix<double> mat(8, 8);
		for (int i = 0; i < 8; i++)
			for (int k = 0; k < 8; k++)
				mat.at(i, k) = [=]()->int 
			{
				switch (map.chess[i][k])
				{
				
					case 1:
						return -1;
					case 2:
						return 1;
					case 3:
						return -2;
					case 4:
						return 2;
					default:
						return 0;
					}
			}();
		Matrix<double> linkLever[5];
		for (int i = 0; i < 2; i++)
		{
			for (int k = 0; k < 5; k++)
			{
				if (i == 0)
					linkLever[k] = p[i][k](kernel[i][k](mat));

				else
					linkLever[k] = p[i][k](kernel[i][k](linkLever[k]));
			}
		}
		Matrix <double> X(1, 5 * linkLever[0].row * linkLever[0].column + 2);
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < linkLever[0].row; j++)
				for (int k = 0; k < linkLever[0].column; k++)
				{
					X.at(0, k + j * linkLever[0].column + i * linkLever[0].column * linkLever[0].row) = linkLever[i].at(j, k);
				}
		X.at(0, 5 * linkLever[0].row * linkLever[0].column) = player;
		X.at(0, 5 * linkLever[0].row * linkLever[0].column + 1) = 1;
		X = sigmoid(SreLu(SreLu(X * this->m[0]) * this->m[1]) * this->m[2]);
		return X.at(0, 0);
	}
};

//用于让两个神经网络一黑一白走子，到一定步数结束并判断胜负
//主要为了种群内进行迭代所用
struct ChessGameEstimate
{
	ChessMap map;
	struct Player
	{
		int color;
		enum PlayerColor { white = 1, black = 2 };
		Player(const int color, ConvolutionNeuralNet* playerNet)
		{
			this->color = color;
			this->playerNet = playerNet;
		}
		Player()
		{

		}
		ConvolutionNeuralNet* playerNet;
	};
	Player p1, p2;

	ChessGameEstimate(ConvolutionNeuralNet* p1, ConvolutionNeuralNet* p2)
	{
		this->p1.playerNet = p1;
		this->p1.color = Player::PlayerColor::black;
		this->p2.playerNet = p2;
		this->p2.color = Player::PlayerColor::white;
		map.start();
	}

	int Estimate()
	{
		for (int playTimes = 0; playTimes < 100; playTimes++)
		{
			if (playTimes % 2 == 0)//black
			{
				//创建一个路径寻找树寻找可吃子的路径

				ChessActionChooseTreeNode* ChessActionChooseTreeRoot = this->map.chessRoadSearch(2, Position(0, 0), 0);

				//反向传播，修改树中各个节点的优先级，便于找到最长的路径
				ChessActionChooseTreeRoot->backPropagation();

				vector<vector<ChessAction> > possibleActions;

				possibleActions = this->map.mostEatRoad(ChessActionChooseTreeRoot);

				delete ChessActionChooseTreeRoot;

				if (possibleActions.size() > 0)
					;

				else
				{
					vector<ChessAction> moves = map.findOnlyMove(2);

					for (unsigned int i = 0; i < moves.size(); i++)
					{
						vector<ChessAction> moveAction;
						moveAction.push_back(moves[i]);
						possibleActions.push_back(moveAction);
					}
				}
				if (possibleActions.size() == 0)
				{
					return 1;
				}
				double maxValue = 0;
				int max = 0;
				for (unsigned int i = 0; i < possibleActions.size(); i++)
				{
					ChessMap toAct = map;
					for (int k = 0; k < possibleActions.at(i).size(); k++)
					{
						toAct = toAct.actChess(possibleActions.at(i).at(k));

					}
					toAct.check();
					double value = p1.playerNet->operator()(toAct, 1);
					if (value > maxValue)
					{
						max = i;
						maxValue = value;
					}
				}
				for (int k = 0; k < possibleActions.at(max).size(); k++)
				{
					map = map.actChess(possibleActions.at(max).at(k));
				}
				map.check();
				//map.print();
			}
			else
			{
				//创建一个路径寻找树寻找可吃子的路径

				ChessActionChooseTreeNode* ChessActionChooseTreeRoot = this->map.chessRoadSearch(1, Position(0, 0), 0);

				//反向传播，修改树中各个节点的优先级，便于找到最长的路径
				ChessActionChooseTreeRoot->backPropagation();

				vector<vector<ChessAction> > possibleActions;

				possibleActions = this->map.mostEatRoad(ChessActionChooseTreeRoot);

				delete ChessActionChooseTreeRoot;

				if (possibleActions.size() > 0)
					;

				else
				{
					vector<ChessAction> moves = map.findOnlyMove(1);

					for (unsigned int i = 0; i < moves.size(); i++)
					{
						vector<ChessAction> moveAction;
						moveAction.push_back(moves[i]);
						possibleActions.push_back(moveAction);
					}
				}
				if (possibleActions.size() == 0)
				{
					return 2;
				}
				double maxValue = 0;
				int max = 0;
				for (unsigned int i = 0; i < possibleActions.size(); i++)
				{
					ChessMap toAct = map;
					for (int k = 0; k < possibleActions.at(i).size(); k++)
					{
						toAct = toAct.actChess(possibleActions.at(i).at(k));
					}
					toAct.check();
					double value = 1 - (*p2.playerNet).operator()(toAct, -1);
					if (value > maxValue)
					{
						max = i;
						maxValue = value;
					}
				}
				for (int k = 0; k < possibleActions.at(max).size(); k++)
				{
					map = map.actChess(possibleActions.at(max).at(k));
				}
				map.check();
				//map.print();

			}
		}
		return 0;
	}

};

//遗传算法所用 种群
//在种群中不断迭代寻找最优的染色体，让其作为ai进行互动
class population 

{
public:
	static const int chrNum = CHROMOSOME_NUMBER;//种群个数
	double crossOverRate = CROSSOVER_RATE;//交叉率
	double variationRate = VARIATION_RATE;//变异率
	int maxR = 0;//一轮过后fit最大的编号
	double maxF = 0;//一轮过后最大的fit值
	chromosome maxC;//最优的染色体
	chromosome content[chrNum]{};//种群

	population() {}
	//按二进制储存训练结果（保存最优的染色体）
	void saveBestChromosome() {
		FILE* f = fopen("ChessAI.dat", "wb");
		fwrite(this->maxC.content, sizeof(bool), chromosome::maxContent, f);
		fclose(f);
	}
	//读取二进制文件，获得迭代后的最优染色体
	bool readChromosome()
	{
		FILE* f = fopen("ChessAI.dat", "rb");
		if (f == NULL)
			return 0;
		fread(this->maxC.content, sizeof(bool), chromosome::maxContent, f);
		fclose(f);
		return 1;
	}
	//模拟生物染色体交叉
	void crossOver()
	{
		//std 随机种子
		std::random_device rd{};
		std::mt19937 gen{ rd() };

		// 值最可能接近平均
		// 标准差影响生成的值距离平均数的分散
		std::normal_distribution<> length{ chromosome::maxContent / 2,chromosome::maxContent };
		std::normal_distribution<> d{ chromosome::maxContent / 2,chromosome::maxContent / 2 };//交叉的长度正态分布

		for (int i = 0; i < chrNum; i++)
		{
			double rate = (rand() % 1000) / 1000.0;
			if (i == maxR)
				continue;
			if (rate < crossOverRate)
			{
#ifdef DEBUG

				cout << "crossover" << endl;
#endif // DEBUG

				//随机和某一个进行交叉
				int toCrossOver = rand() % chrNum;
				if (toCrossOver == maxR)
					continue;
				content[i].crossOver(content[toCrossOver], std::round(d(gen)), std::round(length(gen)));
			}
		}
	}
	//模拟生物种群的选择
	void select()
	{
#ifdef DEBUG
		cout << "select" << endl;
#endif // DEBUG

		//让染色体中的每一个染色体创建一个卷积网络，让他们通过给局面打分
		//进行决策走子，将其胜率保存
		for (int i = 0; i < CHROMOSOME_NUMBER; i++)
		{
			for (int k = 0; k < CHROMOSOME_NUMBER; k++)
			{
				if (k == i)
					continue;
				ConvolutionNeuralNet a(content[i]);
				ConvolutionNeuralNet b(content[k]);
				ChessGameEstimate estimate(&a, &b);
				int result = estimate.Estimate();
				switch (result)
				{
					case 0:
						break;
					case 1:
						content[k].winrate += 1;
						break;
					case 2:
						content[i].winrate += 1;
						break;
					default:
						break;
				}

			}
		}
		//Fit函数为染色体胜率
		double totalFit = 0;
		//选择到每一个的概率
		double rate[chrNum]{ 0 };
		//新的content
		chromosome newContent[chrNum];
		//按赌徒轮盘法进行选择
		int maxRank = 0;
		double maxFit = 0;
		for (int i = 0; i < chrNum; i++)
		{
			double fit = this->content[i].winrate * this->content[i].winrate;
			totalFit += fit;

			i == 0 ? rate[i] = fit : rate[i] = rate[i - 1] + fit;
			if (fit > maxFit)
			{
				maxFit = fit;
				maxRank = i;
			}
		}
		maxR = maxRank;
		maxF = maxFit;
		//让种群最优和当前找到的全局最优下棋
		ConvolutionNeuralNet a(content[maxRank]);
		ConvolutionNeuralNet b(maxC);

		ChessGameEstimate estimate(&a, &b);
		
		//如果种群中最优不能战胜全程最优，则将全程最优保存进种群
		//之后根据胜率进行选择，更新种群
		
		if (estimate.Estimate() == 2)
		{
			maxC = content[maxRank];
		}
		else
		{
			int r = rand() % chrNum;
			content[r] = maxC;
			rate[r] = maxFit;
			maxR = r;
		}

	
		for (int i = 0 ; i < chrNum ; i++)
		{
			rate[i] = rate[i] / totalFit;
		}
		
		//轮盘法
		double pointer = rand() % 10000 / 10000.0;
		for (int k = 0; k < chrNum; k++)
		{
			for (int i = 0; i < chrNum; i++)
			{
				if (i == chrNum - 1)
				{
					newContent[k] = this->content[i];
					break;
				}

				if (pointer > rate[i])
					continue;

				newContent[k] = this->content[i];
				break;
			}
		}
		//将新content复制给population
		for (int i = 0; i < chrNum; i++)
		{
			this->content[i] = newContent[i];
		}
		this->content[maxR] = maxC;
		return;
	}
	//模拟生物染色体变异
	void variation()
	{
		//让种群中每个染色体根据变异率进行变异
		for (int i = 0; i < chrNum; i++)
		{
			if (i == maxR)//种群最优不变异
				continue;
			content[i].variation(variationRate);
		}
	}
};
//将cnn封装成能够走子的ai
struct AIplayer
{
	ConvolutionNeuralNet* cnn;
	enum aiColor { white = 1, black = 2 };
	aiColor color;
	void setConvolutionNeuralNet(ConvolutionNeuralNet* cnn)
	{
		this->cnn = cnn;
	}
	void setAIcolor(aiColor c)//设置ai执什么色的棋子
	{
		color = c;
	}
	int AiPlayerOneStep(ChessMap* map)//ai根据打分走子
	{
		//找吃子的走子
		ChessActionChooseTreeNode* ChessActionChooseTreeRoot = map->chessRoadSearch(color, Position(0, 0), 0);
		ChessActionChooseTreeRoot->backPropagation();
		vector<vector<ChessAction> > possibleActions;
		possibleActions = map->mostEatRoad(ChessActionChooseTreeRoot);

		//可走子的方法为PossibleActions，为一个向量
		delete ChessActionChooseTreeRoot;

		if (possibleActions.size() > 0)
			;
		//如果没有可吃子的走法
		else
		{
			//找走子的走法
			vector<ChessAction> moves = map->findOnlyMove(color);

			for (unsigned int i = 0; i < moves.size(); i++)
			{
				vector<ChessAction> moveAction;
				moveAction.push_back(moves[i]);
				possibleActions.push_back(moveAction);
			}
		}
		//五路可走，则返回1
		if (possibleActions.size() == 0)
		{
			return 1;
		}

		double maxValue = 0;
		int max = 0;
		//找最大评分的走子
		for (unsigned int i = 0; i < possibleActions.size(); i++)
		{
			ChessMap toAct = *map;
			for (int k = 0; k < possibleActions.at(i).size(); k++)
			{
				toAct = toAct.actChess(possibleActions.at(i).at(k));

			}
			toAct.check();
			double value = color == black ? cnn->operator()(toAct, 1) : 1 - cnn->operator()(toAct, -1);
			if (value > maxValue)
			{
				max = i;
				maxValue = value;
			}

		}
		for (int k = 0; k < possibleActions.at(max).size(); k++)
		{
			cout << possibleActions.at(max).at(k) << endl;
			*map = map->actChess(possibleActions.at(max).at(k));

		}
		map->check();
		return 0;
	}
};

//用于提供人机交互接口
struct ChessGame
{
	AIplayer* ai;
	ChessMap map;
	void Play()
	{
		int color;
		cout << "设置ai走子颜色 \n 1为白色（棋盘上表现为负数），其他颜色为黑色";
		cin >> color;
		if (color == 1)
			ai->setAIcolor(AIplayer::white);
		else
			ai->setAIcolor(AIplayer::black);

		for (int i = 0;; i++)
		{
			if (color == 1 && i % 2 == 1 || color == 2 && i % 2 == 0)
			{
				system("cls");
				cout << "ai走子" << endl;
				if (ai->AiPlayerOneStep(&map))
					return;


				map.print();
			}
			else
			{
				cout << "到你的回合" << endl;
				while (1)
				{
					ChessAction act;
					int x; int y;
					cout << "起始点";
					cin >> x >> y;
					act.initialPosition.x = x;
					act.initialPosition.y = y;
					cout << "终点";
					cin >> x >> y;
					act.aimPosition.x = x;
					act.aimPosition.y = y;
					cout << "吃子还是移动（吃子1，移动0）";
					int actType;
					cin >> actType;
					act.actionType = actType;
					map = map.actChess(act);
					map.check();
					system("cls");
					map.print();
					int type;
					cout << "输入-1输入停止,输入-2结束，输入其他  数字  继续输入";
					cin >> type;
					if (type == -1)
						break;
					else if (type == -2)
						return;
				}
			}
		}
	}

};
int main()
{
	population p;
	//默认先读取数据，从而保证之前训练数据不丢失 
	p.readChromosome();
	while (1)
	{
		cout << "输入1进行训练\n输入3读取数据\n输入0结束\n其他数字人机交互,";
		int type;
		cin >> type;
		if (type == 1)
		{
			for (int i = 0; i < GENERATION; i++)
			{
				cerr << "GENERATION :" << i << "   " ;
				p.select();
				p.crossOver();
				p.variation();
				cerr << "BEST CHROMOSOME :" << p.maxR << "   WIN RATE IN POPULATIONS: " << sqrt(p.maxF) / (CHROMOSOME_NUMBER - 1) / 2 << endl;
				p.saveBestChromosome();
			}

			cerr << "共进行了   " << GENERATION << "代" << endl;
			cerr << "种群大小 : " << CHROMOSOME_NUMBER << endl;
			cerr << "变异率为 : " << VARIATION_RATE << endl;
			cerr << "交叉率为 : " << CROSSOVER_RATE << endl;

			system("pause");
		}
		else if (type == 3)		p.readChromosome();
		else if (type == 0)		break;
		else
		{
			//从种群中挑选最优的染色体生成ai以及初始化
			AIplayer AI;
			AI.setAIcolor(AIplayer::black);
			ConvolutionNeuralNet CNN(p.maxC);
			AI.setConvolutionNeuralNet(&CNN);
			ChessGame game;
			game.ai = &AI;
			game.map.start();

			//交互
			game.Play();
		}

	}
	return 0;
}



































