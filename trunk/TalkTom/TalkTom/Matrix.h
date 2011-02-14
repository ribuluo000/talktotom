#include "stdafx.h"

#ifndef __CMATRIX__
#define __CMATRIX__

class CMatrix //矩阵类
{
public:
	//构造函数、析构函数、拷贝构造函数、赋值运算符
	CMatrix(int row, int col);
	~CMatrix();
	CMatrix(const CMatrix &rhs);
	CMatrix& operator=(const CMatrix &rhs);

public:
	//填充矩阵
	bool setMatrix(const double *array, int size);
	//获取矩阵中的所有元素
	bool getMatrix(double *array, int size);
	//获取矩阵的行数
	void getRow(int &row);
	//获取矩阵的列数
	void getCol(int &col);
	//显示矩阵
	bool display(void);

public:
	//矩阵相加
	bool Add(const CMatrix &rhs);
	//矩阵相减
	bool Minus(const CMatrix &rhs);
	//矩阵相乘
	bool Multiply(const CMatrix &rhs);
	//常数与矩阵相乘
	bool Multiply(const double constant);


public:
	//矩阵转置，结果保存在rhs中
	bool TransposeMatrix();

	//求逆矩阵，结果保存在rhs中
	bool InverseMatrix();

private:
	bool ContraryMatrix(double *const pMatrix, double *const _pMatrix, const int &dim);
	CMatrix(){}

private:
	double *m_matrix; //指向矩阵的指针
	int    m_row; //矩阵的行数
	int    m_col; //矩阵的列数
};


#endif