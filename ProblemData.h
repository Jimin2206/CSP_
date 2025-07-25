#pragma once
#include <iostream>
#include <random>
#include <fstream>
#include <chrono>
#include "Highs.h"

using namespace std;

// 문제에 필요한 데이터를 한 곳에서 관리하고,
// 다른 모듈에서 공유하기 위한 class
class ProblemData
{
public:
	static int LL;
	static int nL;
	static int* ReqL;
	static int* b;

	//static void PopulateParamts();
};
