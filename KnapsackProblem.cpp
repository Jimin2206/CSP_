#include "Functions.h"
#include "Highs.h"
#include <vector>
#include <iostream>
#include <limits>

double SubProblem(double* Dual_Vals, int* NewPattern)
{
#pragma region Fetch Data
	int Log_Length = ProblemData::LL;		// total length of a log
	int nLength = ProblemData::nL;			// number of items
	int* Req_Length = ProblemData::ReqL;	// order lengths
	int* b = ProblemData::b;	// demand

#pragma endregion
	Highs highs;
	HighsModel model;
	model.lp_.sense_ = ObjSense::kMaximize;

	// �����Լ� ��� == Dual_Vals

	model.lp_.num_col_ = nLength;	// ���� ���� ����
	model.lp_.col_cost_.assign(Dual_Vals, Dual_Vals + nLength);	// �����Լ� ��� ���� ����

	// ������ ���� / ���� ����
	model.lp_.col_lower_.assign(nLength, 0.0);
	model.lp_.col_upper_.assign(nLength, std::numeric_limits<double>::infinity());

	// ���� ���� ����
	model.lp_.num_row_ = 1;
	model.lp_.row_lower_.push_back(-std::numeric_limits<double>::infinity());
	model.lp_.row_upper_.push_back(Log_Length);

	// ��� ��� A ����
	vector<int> Astart(nLength + 1);
	vector<int> Aindex;
	vector<double> Avalue;

	Astart[0] = 0;
	for (int j = 0; j < nLength; ++j)
	{
		Aindex.push_back(0);			// �� ���� ���� �ִ� ���� row0
		Avalue.push_back(Req_Length[j]);
		Astart[j + 1] = Aindex.size();	// ���� ���� �����ϴ� ��ġ
	}

	model.lp_.a_matrix_.format_ = MatrixFormat::kColwise;
	model.lp_.a_matrix_.start_ = Astart;
	model.lp_.a_matrix_.index_ = Aindex;
	model.lp_.a_matrix_.value_ = Avalue;

	// ���� ���� ����
	model.lp_.integrality_.assign(nLength, HighsVarType::kInteger);

	// HiGHS ����
	highs.setOptionValue("output_flag", false);	// �ֹ� ��� ��
	highs.passModel(model);
	HighsStatus status = highs.run();

	if (status != HighsStatus::kOk)
	{
		cerr << "SubProblem: HiGHS optimization failed!" << endl;
		throw - 1;
	}

	const vector<double>& sol = highs.getSolution().col_value;	// ���� ���� ��
	double obj = highs.getInfo().objective_function_value;		// ���� �Լ� ��

	cout << "\t New pattern generated: [";
	for (int i = 0; i < nLength; ++i)
	{
		NewPattern[i] = (int)(sol[i] + 0.5);
		cout << " " << NewPattern[i];
	}
	cout << "]" << endl;

	cout << "\n\t\t SP obj is: " << obj << endl;
	cout << "\n\n\n\n";

	return obj;

}