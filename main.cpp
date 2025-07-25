/*
	File name: main.cpp
	Description: Column generation to solve the CSP
*/

/*
	start with a small number of patterns (columns),
	generating additional patterns as needed by solving a subproblem (knapsack problem)
*/

// L: total length of each roll
// I: set of order lengths
// P: set of patterns
// a_ip: number of pieces of length i cut in pattern p
// b_i: demand for length i
// X_p: number of logs cut using pattern p

#include "ProblemData.h";
#include "Functions.h"
#include "Highs.h"
#include <vector>
#include <iostream>

using namespace std;

int main()
{
#pragma region Fetch Data and Initialize the feasible patterns
	int Log_Length = ProblemData::LL;
	int nLength = ProblemData::nL;
	int* Req_Length = ProblemData::ReqL;
	int* b = ProblemData::b;

	// generate initial patterns for RMP
	vector<int*> Pattern;
	for (int i = 0; i < nLength; i++)
	{
		int* Pat0 = new int[nLength]();
		Pat0[i] = std::floor(Log_Length / Req_Length[i]);

		Pattern.push_back(Pat0);
	}

	// print initial patterns
	cout << "Initial patterns" << endl;
	for (int p = 0; p < Pattern.size(); p++)
	{
		cout << "Pattern " << p << " [";
		for (int i = 0; i < nLength; i++)
		{
			cout << " " << Pattern[p][i];
		}
		cout << " ]" << endl;
	}

	// vector to save final solution (optimal number of patterns used)
	vector<double> final_sol;

#pragma endregion

	int iter_count = 0;

	while (true)
	{
		iter_count++;
		// Solve the restricted master problem (RMP) using HiGHS

		Highs highs;
		HighsModel model;
		model.lp_.sense_ = ObjSense::kMinimize;

		int num_vars = Pattern.size();
		int num_cons = nLength;

		// objective coefficients
		model.lp_.col_cost_.assign(num_vars, 1.0);

		// variable range
		model.lp_.col_lower_.assign(num_vars, 0.0);
		model.lp_.col_upper_.assign(num_vars, std::numeric_limits<double>::infinity());

		// constraints
		model.lp_.row_lower_.assign(b, b + nLength);
		model.lp_.row_upper_.assign(num_cons, std::numeric_limits<double>::infinity());

		// coefficient matrix A
		vector<int> Astart(num_vars + 1);
		vector<int> Aindex;
		vector<double> Avalue;

		int nnz = 0;	// number of nonzeros in each column
		for (int p = 0; p < num_vars; ++p)
		{
			Astart[p] = nnz;
			for (int i = 0; i < nLength; ++i)
			{
				if (Pattern[p][i] > 0)
				{
					Aindex.push_back(i);
					Avalue.push_back(Pattern[p][i]);
					nnz++;
				}
			}
		}
		Astart[num_vars] = nnz;

		model.lp_.num_col_ = num_vars;
		model.lp_.num_row_ = num_cons;
		model.lp_.a_matrix_.format_ = MatrixFormat::kColwise;
		model.lp_.a_matrix_.start_ = Astart;
		model.lp_.a_matrix_.index_ = Aindex;
		model.lp_.a_matrix_.value_ = Avalue;

		highs.passModel(model);
		highs.setOptionValue("output_flag", false);
		HighsStatus status = highs.run();

		if (status != HighsStatus::kOk)
		{
			cerr << "RMP: HiGHS optimization failed!" << endl;
			throw - 1;
		}

		const vector<double>& sol = highs.getSolution().col_value;	// optimal variable values
		double obj = highs.getInfo().objective_function_value;		// optimal objective value

		final_sol = sol;

		cout << "\n\t Iteration: " << iter_count << "\t Required Number of logs: " << obj << endl;	// first iteration: Required number of logs: 23.4167

		// get the dual variables from the RMP
		double* Duals = new double[nLength]();
		const vector<double>& duals = highs.getSolution().row_dual;
		
		for (int i = 0; i < nLength; i++)
		{
			Duals[i] = duals[i];	// dual variable for each demand constraint
			//cout << Duals[i] << endl;	// 0.1, 0.05, 0.166667, 0.2, 0.2, 0.333333
		}

		//int jj = 0;
		// generate a new pattern from the subproblem (knapsack problem)
		int* NewPat = new int[nLength]();
		double SP_obj = SubProblem(Duals, NewPat);

		// check the optimality condition/ add the new patterns
		if (1 - SP_obj >= 0) { break; }
		else
		{
			Pattern.push_back(NewPat);
		}
	}


	cout << "Final patterns" << endl;
	for (int p = 0; p < Pattern.size(); p++)
	{
		cout << "Pattern " << p << " [";
		for (int i = 0; i < nLength; i++)
		{
			cout << " " << Pattern[p][i];
		}
		cout << " ]: used " << final_sol[p] << " times" << endl;
	}
	return 0;

	/*
		Final patterns
		Pattern 0[10 0 0 0 0 0]: used 2 times
		Pattern 1[0 20 0 0 0 0] : used 0.633333 times
		Pattern 2[0 0 6 0 0 0] : used 0 times
		Pattern 3[0 0 0 5 0 0] : used 0 times
		Pattern 4[0 0 0 0 5 0] : used 1.8 times
		Pattern 5[0 0 0 0 0 3] : used 0 times
		Pattern 6[0 0 0 4 0 1] : used 3 times
		Pattern 7[0 2 0 0 0 3] : used 10.3333 times
		Pattern 8[0 2 6 0 0 0] : used 3.83333 times

		-> not an integer solution
		-> Branch and Price algorithm
	*/
}