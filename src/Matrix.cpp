#include "Matrix.h"

Matrix::Matrix(char rows, char columns, std::vector<float> data)
	: matrixData(data)
{
	matrixDimensions = { rows, columns };
}

Matrix::Matrix()
{
}

Matrix::~Matrix()
{
}

// Getter
Dimensions Matrix::getDimensions()
{
	return matrixDimensions;
}

// Override () operator to allow indexing
float Matrix::operator()(char row, char column)
{
	return matrixData[row * matrixDimensions.columns + column];
}

// Set data at specified row and column
void Matrix::setData(char row, char column, float data)
{
	matrixData[row * matrixDimensions.columns + column] = data;
}

// Override * operator with matrix multiplication
Matrix Matrix::operator*(Matrix &otherMatrix)
{
	// Check for valid matrix dimensions
	if (matrixDimensions.columns != otherMatrix.getDimensions().rows)
		throw std::invalid_argument("matrices are of invalid dimensions to be multiplied");

	char thisRows = matrixDimensions.rows;
	char thisColumns = matrixDimensions.columns;
	char otherRows = otherMatrix.getDimensions().rows;
	char otherColumns = otherMatrix.getDimensions().columns;
	std::vector<float> resultData;
	float *currentRow = new float[thisColumns];
	float *currentColumn = new float[otherRows];
	// Multiply matrices
	for (char row = 0; row < thisRows; row++)
	{
		for (char column = 0; column < otherColumns; column++)
		{
			for (char k = 0; k < thisColumns; k++)
			{
				currentRow[k] = matrixData[row * thisColumns + k];
			}
			for (char k = 0; k < otherRows; k++)
			{
				currentColumn[k] = otherMatrix.matrixData[column + otherColumns * k];
			}
			resultData.push_back(dotProduct(currentRow, currentColumn, thisColumns));
		}
	}
	// Free memory
	delete currentRow;
	delete currentColumn;

	return Matrix(thisRows, otherColumns, resultData);
}

// Helper function for matrix multiplication
float dotProduct(float *row, float *column, char size)
{
	float result = 0;
	for (int i = 0; i < size; i++)
	{
		result += row[i] * column[i];
	}
	return result;
}