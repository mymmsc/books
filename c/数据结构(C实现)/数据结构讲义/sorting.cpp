#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <iostream.h>
#define  N   25000     // 待排序元素的个数

void insertsort(int R[N+1])  // 直接插入排序
{
	int i,j;
	for (i=2; i<=N; i++) {
		R[0]=R[i];    // 设置监视哨
		j=i-1;
		while (R[0]<R[j]) {
			R[j+1]=R[j];
			j--;
		}
		R[j+1]=R[0];
	}
}

void shellsort(int R[N+1])  // 希尔排序
{
	int i,j,gap;
	int x;
	gap=N/2;   // 设置初始增量
	while (gap>0) {
		for (i=gap+1; i<=N; i++) {
			j=i-gap;
			while (j>0) 
				if(R[j]>R[j+gap]) {
					x=R[j];
					R[j]=R[j+gap];
					R[j+gap]=x;
					j=j-gap;
				}
				else 
					j=0;
		}
		gap=gap/2;   // 减小增量
	}
}

void bubblesort(int R[N+1])  // 起泡排序
{
	int i,j,noswap;
	int temp;
	for (i=1; i<=N-1; i++) {
		noswap=1;
		for (j=N; j>=i+1; j--) 
			if(R[j]<R[j-1]) {
				temp=R[j];
				R[j]=R[j-1];
				R[j-1]=temp;
				noswap=0;
			}
		if(noswap)
			break;
	}
}

int partition(int R[N+1],int low,int high)  // 快速排序的子函数（取定枢轴元素）
{
	int i,j;
	i=low;
	j=high;
	R[0]=R[low];   // 取定枢轴元素
	do {  // 从表的两端交替地向中间扫描
		while ((j>i) && (R[j]>=R[0]))
			j--;
		if(i<j) {
			R[i]=R[j];
			i++;
		}
		while ((i<j) && (R[i]<=R[0]))
			i++;
		if(i<j) {
			R[j]=R[i];
			j--;
		}
	} while (i<j);
	R[i]=R[0];  // 枢轴元素到位
	return i;   // 返回枢轴位置
}

void quicksort(int R[N+1],int low,int high)  // 快速排序
{
	int i;
	if(low<high) {
		i=partition(R,low,high);  // 将表R一分为二
		quicksort(R,low,i-1);     // 对低子表递归排序
		quicksort(R,i+1,high);    // 对高子表递归排序
	}
}

void selectsort(int R[N+1])    // 直接选择排序
{
	int i,j,k;
	int temp;
	for (i=1; i<=N-1; i++) {
		k=i;
		for (j=i+1; j<=N; j++)
			if(R[j]<R[k])
				k=j;   // 用k指出每趟在无序区间段的最小元素
			if(k!=i) {
				temp=R[i];
				R[i]=R[k];
				R[k]=temp;
			}
	}
}

void sift(int R[N+1],int s,int m)  // 堆排序的子函数（筛选算法，使R[s..m]成为一个大根堆）
{
	int i,j;
	int temp;
	temp=R[s];
	i=s; 
	j=2*i;   // R[j]是R[i]的左孩子
	while (j<=m) {
		if((j<m) && (R[j]<R[j+1]))
			j++;  // 若右孩子较大，则把j修改为右孩子的下标
		if(temp<R[j]) {
			R[i]=R[j];  // 将R[j]调到父亲的位置上
			i=j;
			j=2*i;      // 修改i和j的值，以便继续向下筛选
		}
		else 
			break;      // 筛选完成，终止循环
	}
	R[i]=temp;          // 被筛结点的值放入最终位置
}

void heapsort(int R[N+1])  // 堆排序
{
	int i;
	int temp;
	for (i=N/2; i>=1; i--)
		sift(R,i,N);       // 建立初始堆
	for (i=N; i>=2; i--) { // 进行N-1次循环，完成堆排序
		temp=R[1];
		R[1]=R[i];
		R[i]=temp;         // 将第一个元素同当前区间内最后一个元素对换
		sift(R,1,i-1);     // 筛选R[1]结点，得到(N-1)个结点的堆
	}
}

void main()
{
	int R[N+1],RR[N+1];     // 待排序的元素组
	clock_t start,finish;   // 用于函数运行的记时
	double duration;
	int i;

	cout<<"The initial data: "<<endl;
	for (i=1; i<=N; i++) {
		R[i]=rand()%5001;
		RR[i]=R[i];
		cout<<R[i]<<"   ";
		if(i%10==0)
			cout<<endl;
	}

///////////////////////////////////////////////////////////
	getchar();        // 直接插入排序
	start = clock();  // 记时开始
	insertsort(RR);
	finish = clock();   // 记时结束
	duration = (double)(finish-start) / CLOCKS_PER_SEC;
	cout<<"The result based on the insert sort is :"<<endl;
	for (i=1; i<=N; i++) {
		cout<<RR[i]<<"    ";
		if(i%10==0)
			cout<<endl;
	}
	cout<<"The Run Time is: "<<duration<<" seconds"<<endl;

///////////////////////////////////////////////////////////
    getchar();        // 希尔排序
	for (i=1; i<=N; i++) {
		RR[i]=R[i];
	}
	start = clock();  // 记时开始
	shellsort(RR);
	finish = clock();   // 记时结束
	duration = (double)(finish-start) / CLOCKS_PER_SEC;
	cout<<"The result based on the shell sort is :"<<endl;
	for (i=1; i<=N; i++) {
		cout<<RR[i]<<"    ";
		if(i%10==0)
			cout<<endl;
	}
	cout<<"The Run Time is: "<<duration<<" seconds"<<endl;

///////////////////////////////////////////////////////////
    getchar();        // 起泡排序
	for (i=1; i<=N; i++) {
		RR[i]=R[i];
	}
	start = clock();  // 记时开始
	bubblesort(RR);
	finish = clock();   // 记时结束
	duration = (double)(finish-start) / CLOCKS_PER_SEC;
	cout<<"The result based on the shell sort is :"<<endl;
	for (i=1; i<=N; i++) {
		cout<<RR[i]<<"    ";
		if(i%10==0)
			cout<<endl;
	}
	cout<<"The Run Time is: "<<duration<<" seconds"<<endl;

///////////////////////////////////////////////////////////
    getchar();        //  快速排序
	for (i=1; i<=N; i++) {
		RR[i]=R[i];
	}
	start = clock();  // 记时开始
	quicksort(RR,1,N);
	finish = clock();   // 记时结束
	duration = (double)(finish-start) / CLOCKS_PER_SEC;
	cout<<"The result based on the shell sort is :"<<endl;
	for (i=1; i<=N; i++) {
		cout<<RR[i]<<"    ";
		if(i%10==0)
			cout<<endl;
	}
	cout<<"The Run Time is: "<<duration<<" seconds"<<endl;

///////////////////////////////////////////////////////////
    getchar();        //  直接选择排序
	for (i=1; i<=N; i++) {
		RR[i]=R[i];
	}
	start = clock();  // 记时开始
	selectsort(RR);
	finish = clock();   // 记时结束
	duration = (double)(finish-start) / CLOCKS_PER_SEC;
	cout<<"The result based on the shell sort is :"<<endl;
	for (i=1; i<=N; i++) {
		cout<<RR[i]<<"    ";
		if(i%10==0)
			cout<<endl;
	}
	cout<<"The Run Time is: "<<duration<<" seconds"<<endl;

///////////////////////////////////////////////////////////
    getchar();        //  堆排序
	for (i=1; i<=N; i++) {
		RR[i]=R[i];
	}
	start = clock();  // 记时开始
	heapsort(RR);
	finish = clock();   // 记时结束
	duration = (double)(finish-start) / CLOCKS_PER_SEC;
	cout<<"The result based on the shell sort is :"<<endl;
	for (i=1; i<=N; i++) {
		cout<<RR[i]<<"    ";
		if(i%10==0)
			cout<<endl;
	}
	cout<<"The Run Time is: "<<duration<<" seconds"<<endl;

///////////////////////////////////////////////////////////

}






