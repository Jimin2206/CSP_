#pragma once
#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <chrono>
#include "Highs.h"

//using NumVar2D = vector<vector<double>>;
using NumVar1D = vector<double>;
using Range2D = vector<vector<double>>;
using NumVar3D = vector<vector<vector<double>>>;

using namespace std;

// ������ �ʿ��� �����͸� �� ������ �����ϰ�,
// �ٸ� ��⿡�� �����ϱ� ���� class
class ProblemData
{
public:
	static int LL;
	static int nL;
	static int* ReqL;
	static int* b;

	//static void PopulateParamts();
};
