#include "stdafx.h"
#include <iostream>
#include "Matrix.h"
#include <math.h>

using namespace std;


//一般构造函数
CMatrix::CMatrix(int row, int col)
:m_matrix(NULL), m_row(0), m_col(0)
{
	int length = 0;

	//判断行数与列数是否合乎规定
	if ((row < 1) || (col < 1))
	{
		return;
	}

	length = row * col;

	m_matrix = new double[length]; //为矩阵分配内存空间
	if (NULL == m_matrix)
	{
		return;
	}
	else
	{
		m_row = row;
		m_col = col;

		//首先用0来填充矩阵里面的所有元素
		memset(m_matrix, 0, length * sizeof(double));
	}

}

//析构函数
CMatrix::~CMatrix()
{
	if (NULL != m_matrix)
	{
		delete [] m_matrix; //释放矩阵所占用的内存
	}
}

//拷贝构造函数
CMatrix::CMatrix(const CMatrix &rhs)
:m_matrix(NULL), m_row(rhs.m_row), m_col(rhs.m_col)
{
	int length = 0;

	if (NULL == rhs.m_matrix) //矩阵rhs为空
	{
		return;
	}

	length = m_row * m_col;

	m_matrix = new double[length]; //为矩阵分配内存空间
	if (NULL == m_matrix)
	{
		m_row = 0;
		m_col = 0;

		return;
	}
	else
	{
		//用rhs矩阵里面的元素来填充本矩阵
		memcpy(m_matrix, rhs.m_matrix, length * sizeof(double) );
	}

}

//赋值运算符
CMatrix& CMatrix::operator=(const CMatrix &rhs)
{
	int length = 0;

	//判断是否自赋值
	if (this != &rhs)
	{
		//释放原矩阵所占用的内存
		delete [] m_matrix;
		m_matrix = NULL;
		m_row = 0;
		m_col = 0;

		//矩阵rhs是否为空
		if (rhs.m_matrix != NULL)
		{
			length = rhs.m_row * rhs.m_col;

			m_matrix = new double [length]; //为矩阵分配内存空间
			if (m_matrix != NULL)
			{
				m_row = rhs.m_row;
				m_col = rhs.m_col;

				//用rhs矩阵里面的元素来填充本矩阵
				memcpy(m_matrix, rhs.m_matrix, length * sizeof(double) );
			}

		} //if ((rhs.m_row > 0) && (rhs.m_col > 0))

	}

	return *this; //返回本对象的引用

}

//填充矩阵
bool CMatrix::setMatrix(const double *array, int size)
{
	if ((NULL == m_matrix) || (NULL == array))
	{
		return false;
	}

	if (size != (m_row * m_col)) //长度不相符
	{
		return false;
	}
	else
	{
		//用数组array里面的值来填充本矩阵
		memcpy(m_matrix, array, size * sizeof(double));

		return true;

	}

}

//获取矩阵中的所有元素
bool CMatrix::getMatrix(double *array, int size)
{
	int length = 0;

	if (    (NULL == m_matrix) 
		||  (NULL == array)
		||  (size != m_col * m_row))
	{
		return false;
	}
	else
	{
		length = m_row * m_col;

		//用数组array来返回本矩阵中所有元素值
		memcpy(array, m_matrix, length * sizeof(double) );

		return true;

	}

}

//获取矩阵的行数
void CMatrix::getRow(int &row)
{
	row = m_row;
}

//获取矩阵的列数
void CMatrix::getCol(int &col)
{
	col = m_col;
}

//显示矩阵
bool CMatrix::display(void)
{
	if (NULL == m_matrix) //本矩阵为空
	{
		return false;
	}
	else
	{
		//按行输出矩阵
		for (int row = 0; row < m_row; ++row)
		{
			for (int col = 0; col < m_col; ++col)
			{
				printf("%5f\t",m_matrix[row * m_col + col]); //同一行中的矩阵元素之间用一个tab 隔开
			}

			cout << '\n'; //准备输出下一行

		}

		return true;

	}

}

//矩阵相加
bool CMatrix::Add(const CMatrix &rhs)
{
	bool b = false;

	//判断两个矩阵的行数与列数是否分别相等
	if ((m_row == rhs.m_row) && (m_col == rhs.m_col))
	{
		if (NULL == m_matrix)
		{
			return b; 
		}
		else
		{
			b = true;

			int length = m_row * m_col;

			for (int index = 0; index < length; ++index)
			{
				m_matrix[index] = m_matrix[index] + rhs.m_matrix[index]; //相加
			}

		} //if ((m_row < 1) || (m_col < 1))

	}

	return b;
}

//矩阵相减
bool CMatrix::Minus(const CMatrix &rhs)
{
	CMatrix temp = rhs;

	temp.Multiply(-1);

	return this->Add(temp);
}

//矩阵相乘
bool CMatrix::Multiply(const CMatrix &rhs)
{
	bool b = false;

	if (m_col == rhs.m_row) //第一个矩阵的列数与第二个矩阵的行数相等
	{
		if (NULL == m_matrix)
		{
			return b;
		}
		else
		{
			b = true;

			// 原来的矩阵变成 this->m_row 行, rhs.m_col列
			CMatrix tempMatrix(m_row, rhs.m_col);

			for (int row = 0; row < m_row; ++row) //行
			{
				for (int col = 0; col < rhs.m_col; ++col) //列
				{
					for (int index = 0; index < m_col; ++index)
					{
						tempMatrix.m_matrix[row*rhs.m_col+col] += 
							(m_matrix[row*m_col+index] * rhs.m_matrix[index*rhs.m_col+col]);
					}

				}
			}

			// 处理原来数据，包括行与列
			delete [] this->m_matrix;
			
			m_col = rhs.m_col;

			m_matrix = new double [m_row*m_col];

			memcpy(m_matrix, tempMatrix.m_matrix, m_col * m_row * sizeof(double) );

		} //if (NULL == m_matrix)

	}

	return b;

}

//常数与矩阵相乘
bool CMatrix::Multiply(const double constant)
{
	bool b = false;

	//本矩阵是否为空
	if (NULL == m_matrix)
	{
		return b;
	}
	else
	{
		b = true;

		int length = m_row * m_col;

		for (int index = 0; index < length; ++index)
		{
			this->m_matrix[index] = m_matrix[index] * constant; //矩阵元素与常数相乘
		}
	}

	return b;
}


//矩阵转置，结果保存在rhs中
bool CMatrix::TransposeMatrix()
{
	if (this->m_matrix == NULL)
	{
		return false;
	}

	CMatrix temp(m_row, m_col);

	for (int i = 0; i < m_row; i++)
	{
		for (int j = 0; j < m_col;j++)
		{
			*(temp.m_matrix + j * m_col + i) = *(m_matrix + i * m_col + j);
		}
	}

	memcpy( this->m_matrix, temp.m_matrix, m_row * m_col * sizeof(double) );

	return true;
}

//求逆矩阵，结果保存在rhs中
bool CMatrix::InverseMatrix()
{
	bool b = false;
	if (m_col != m_row)
	{
		// 不是方阵
		return b;
	}

	b = ContraryMatrix(this->m_matrix, this->m_matrix, m_col);

	return b;
}

//求pMatrix的逆矩阵，并存结果于矩阵_pMatrix中
bool CMatrix::ContraryMatrix(double *const pMatrix, double *const _pMatrix, const int &dim)
{ 
	bool b = true;

	double *tMatrix = new double[2*dim*dim];
	for (int i=0; i<dim; i++){
		for (int j=0; j<dim; j++)
			tMatrix[i*dim*2+j] = pMatrix[i*dim+j];        
	}
	for (int i=0; i<dim; i++){
		for (int j=dim; j<dim*2; j++)
			tMatrix[i*dim*2+j] = 0.0;
		tMatrix[i*dim*2+dim+i]  = 1.0;        
	}
	//Initialization over!
	for (int i=0; i<dim; i++)//Process Cols
	{
		double base = tMatrix[i*dim*2+i];
		if (fabs(base) < 1E-300){
			cout << endl << "zero is divied!" << endl;
			b = false;
			return b;
		}
		for (int j=0; j<dim; j++)//row
		{
			if (j == i) continue;
			double times = tMatrix[j*dim*2+i]/base;
			for (int k=0; k<dim*2; k++)//col
			{        
				tMatrix[j*dim*2+k] = tMatrix[j*dim*2+k] - times*tMatrix[i*dim*2+k];
			}
		}        
		for (int k=0; k<dim*2; k++){
			tMatrix[i*dim*2+k] /= base;
		}
	}
	for (int i=0; i<dim; i++)
	{
		for (int j=0; j<dim; j++)
			_pMatrix[i*dim+j] = tMatrix[i*dim*2+j+dim];        
	}    
	delete[] tMatrix;

	return b;
}

////////////////////////////////////////////////////////////////