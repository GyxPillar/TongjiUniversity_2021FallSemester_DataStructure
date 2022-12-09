/* 2053385 高逸轩 */

#include<iostream>
#include<ctime>
#include<time.h>
using namespace std;

// 程序运行时间
clock_t start_time, end_time;

// 数据范围
int N;

// 记录比较次数
long long int cnt = 0;

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


// 冒泡排序
void BubbleSort(int a[], int begin, int end)
{
	// i记录当前有序的位置
	for (int i = end; i >= begin; i--)
	{
		// j记录当前比较元素
		for (int j = begin; j < i; j++)
		{	// 冒泡
			if (a[j] > a[j + 1])
			{
				swap(a[j], a[j + 1]);
			}
			cnt++;
		}
	}
}

// 选择排序
void SelectSort(int a[], int begin, int end)
{
	for (int i = begin; i < end; i++)
	{
		// temp记录要交换的位置
		int temp = i;
		for (int j = i + 1; j <= end; j++)
		{
			// 更新交换位置的记录
			if (a[j] < a[temp]) temp = j;
			cnt++;
		}
		// 交换
		swap(a[i], a[temp]);
	}
}

// 插入排序
void InsertSort(int a[], int begin, int end)
{
	for (int i = begin + 1; i <= end; i++)
	{
		for (int j = i; j > begin; j--)
		{
			// 从无序区向前比较
			if (a[j] < a[j - 1]) swap(a[j], a[j - 1]);
			cnt++;
		}
	}
}

// 希尔排序
void ShellSort(int a[], int begin, int end)
{
	// gap为分块大小，初始为数据范围的1/2
	int gap = (end - begin + 1) / 2;
	while (gap)
	{
		for (int k = 0; k < gap; k++)
		{
			for (int i = begin + gap + k; i <= end; i += gap)
			{
				for (int j = i; j > begin + k; j -= gap)
				{
					if (a[j] < a[j - gap]) swap(a[j], a[j - gap]);
					cnt++;
				}
			}
		}
		// 缩小gap
		gap /= 2;
	}
}

// 快速排序
void QuickSort(int a[], int begin, int end)
{
	if (begin > end) return;
	// 比较指针
	int i = begin, j = end;

	while (i != j)
	{
		// a[begin]设置为比较基准
		while (a[j] >= a[begin] && j > i) j--;
		while (a[i] <= a[begin] && j > i) i++;
		if (j > i)	swap(a[i], a[j]);
		cnt += 3;
	}

	// 交换指针最终位置和基准位置
	swap(a[i], a[begin]);

	// 以指针最终位置为划分，分为两部分向下递归
	QuickSort(a, begin, i - 1);
	QuickSort(a, i + 1, end);
}


// 堆排序调整函数
void HeapAdjust(int a[], int begin, int i, int length)
{
	// 取出当前元素i，暂存tmp
	int tmp = a[begin + i];

	// 从i结点的左子结点开始,即2i+1处开始
	for (int k = 2 * i + 1; k < length; k = k * 2 + 1)
	{  
		// 如果左子结点小于右子结点，则k指向右子结点(2k+1表示左子结点，2k+2表示右子结点)
		if (k + 1 < length && a[begin + k] < a[begin + k + 1])  k++;

		cnt++;

		// 如果子节点大于父节点,将子节点值赋给父节点(不用进行交换)
		if (a[begin + k] > tmp)
		{
			cnt++;
			a[begin + i] = a[begin + k];
			i = k;// i指向k,即将较小值放在较大值下方
		}
		// 自下而上进行调整,保证下方子树已为大顶堆
		else break;
	}
	//当for循环结束后，将以i为父结点的树的最大值放在局部堆顶
	a[begin + i] = tmp;
}
// 堆排序
void HeapSort(int a[], int begin, int end)
{
	// 长度
	int length = end - begin + 1;
	// 从最后一个有儿子节点的位置(length/2-1)开始，调整关系
	for (int i = length / 2 - 1; i >= 0; i--)
		HeapAdjust(a, begin, i, length);

	// 将大根堆顶与最后一个元素调整位置
	for (int j = length - 1; j > 0; j--)
	{
		swap(a[begin], a[begin + j]);// 将堆顶元素与末尾元素进行交换
		HeapAdjust(a, begin, 0, j);  // 固定尾部位置，重新对堆进行调整
	}
}


// 归并排序
void MergeSort(int a[], int begin, int end)
{
	if (begin == end) return;
	// 找到中点位置
	int mid = (begin + end) / 2;

	// 分为两部分向下递归排序
	MergeSort(a, begin, mid);
	MergeSort(a, mid + 1, end);

	// 定义指针
	int i = begin, j = mid + 1, k = begin;

	// temp用来暂时存储当前部分的元素
	int* temp= new int[100001];

	// 只要两部分都没有走到结尾
	while (i <= mid && j <= end)
	{
		// 从两部分当前指针位置选择较小元素加入temp
		if (a[i] <= a[j]) temp[k++] = a[i++];
		else temp[k++] = a[j++];

		cnt++;
	}

	// 将未选择的元素加入temp
	while (i <= mid) temp[k++] = a[i++];
	while (j <= end) temp[k++] = a[j++];

	// 将排序后temp的信息传递给a
	for (int i = begin; i <= end; i++) a[i] = temp[i];

	delete[]temp;
}


// 基数排序子函数
void _RadixSort(int a[], int begin, int end, int exp)
{
	int* result = new int[100001];    //存放桶中收集数据后的临时数组

	int bucket[10] = { 0 };    //初始化10个桶

	// 遍历A，将数据出现的次数存储在桶bucket中
	for (int i = begin; i <= end; i++)
	{
		bucket[(a[i] / exp) % 10]++;
		cnt++;
	}

	// 调整bucket各元素的值，调整后的值就是A中元素在result中的位置
	for (int i = 1; i < 10; i++)
		bucket[i] = bucket[i] + bucket[i - 1];

	// 将a中的元素填充到result中（从后往前排，先入后出）
	for (int i = end; i >= begin; i--) 
	{
		int iexp = (a[i] / exp) % 10;
		result[bucket[iexp] - 1] = a[i];
		bucket[iexp]--;
		cnt++;
	}
	// 将排序好的数组result复制回A中
	for (int i = begin; i <= end; i++)
	{
		a[i] = result[i - begin];
		cnt++;
	}

	delete[]result;
}
// 基数排序
void RadixSort(int a[], int begin, int end)
{
	// 获取数组中的最大值
	int infoMax = 0;     
	for (int i = begin; i <= end; i++) infoMax = infoMax > a[i] ? infoMax : a[i];

	// 从个位开始，对数组a按位进行基数排序
	for (int iexp = 1; infoMax / iexp > 0; iexp *= 10)
		_RadixSort(a, begin, end, iexp);    
}


int main()
{
	srand((unsigned int)(time(0)));

	cout << "**        排序算法        **" << endl;
	cout << "============================" << endl;
	cout << "**       1.冒泡排序       **" << endl;
	cout << "**       2.选择排序       **" << endl;
	cout << "**       3.插入排序       **" << endl;
	cout << "**       4.希尔排序       **" << endl;
	cout << "**       5.快速排序       **" << endl;
	cout << "**       6.堆排序         **" << endl;
	cout << "**       7.归并排序       **" << endl;
	cout << "**       8.基数排序       **" << endl;
	cout << "============================" << endl;

	cout << endl << "请输入产生随机数的个数(1-10000)：";
	N = getint(1, 10000, "输入错误，请重新输入：");

	int a[10001];


	// 生成随机数
	for (int i = 0; i < N; i++)
		a[i] = rand();
	// 记录开始时间
	start_time = clock();
	BubbleSort(a, 0, N - 1);
	// 记录运行时间
	end_time = clock();
	cout << "The run time is: " << double(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
	cout << "冒泡排序比较次数: " << cnt << endl << endl;
	cnt = 0;


	// 生成随机数
	for (int i = 0; i < N; i++)
		a[i] = rand();
	// 记录开始时间
	start_time = clock();
	SelectSort(a, 0, N - 1);
	// 记录运行时间
	end_time = clock();
	cout << "The run time is: " << double(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
	cout << "选择排序比较次数: " << cnt << endl << endl;
	cnt = 0;


	// 生成随机数
	for (int i = 0; i < N; i++)
		a[i] = rand();
	// 记录开始时间
	start_time = clock();
	InsertSort(a, 0, N - 1);
	// 记录运行时间
	end_time = clock();
	cout << "The run time is: " << double(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
	cout << "插入排序比较次数: " << cnt << endl << endl;
	cnt = 0;


	// 生成随机数
	for (int i = 0; i < N; i++)
		a[i] = rand();
	// 记录开始时间
	start_time = clock();
	ShellSort(a, 0, N - 1);
	// 记录运行时间
	end_time = clock();
	cout << "The run time is: " << double(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
	cout << "希尔排序比较次数: " << cnt << endl << endl;
	cnt = 0;


	// 生成随机数
	for (int i = 0; i < N; i++)
		a[i] = rand();
	// 记录开始时间
	start_time = clock();
	QuickSort(a, 0, N - 1);
	// 记录运行时间
	end_time = clock();
	cout << "The run time is: " << double(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
	cout << "快速排序比较次数: " << cnt << endl << endl;
	cnt = 0;


	// 生成随机数
	for (int i = 0; i < N; i++)
		a[i] = rand();
	// 记录开始时间
	start_time = clock();
	HeapSort(a, 0, N - 1);
	// 记录运行时间
	end_time = clock();
	cout << "The run time is: " << double(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
	cout << "堆排序比较次数: " << cnt << endl << endl;
	cnt = 0;


	// 生成随机数
	for (int i = 0; i < N; i++)
		a[i] = rand();
	// 记录开始时间
	start_time = clock();
	MergeSort(a, 0, N - 1);
	// 记录运行时间
	end_time = clock();
	cout << "The run time is: " << double(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
	cout << "归并排序比较次数: " << cnt << endl << endl;
	cnt = 0;


	// 生成随机数
	for (int i = 0; i < N; i++)
		a[i] = rand();
	// 记录开始时间
	start_time = clock();
	RadixSort(a, 0, N - 1);
	// 记录运行时间
	end_time = clock();
	cout << "The run time is: " << double(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;
	cout << "基数排序比较次数: " << cnt << endl << endl;
	cnt = 0;

	
	system("pause");
	return 0;
}
 
