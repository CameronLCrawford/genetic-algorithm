#pragma once
#include <vector>

struct Dimensions
{
	char rows;
	char columns;
};

float dotProduct(float *, float *, char);

class Matrix
{
public:

	Matrix(char, char, std::vector<float>);

	Matrix();

	~Matrix();

	Dimensions getDimensions();

	float operator ()(char row, char column);

	void setData(char, char, float);

	Matrix operator*(Matrix &);

private:
	std::vector<float> matrixData;
	Dimensions matrixDimensions;

};