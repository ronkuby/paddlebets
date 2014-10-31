/***************************************************************************
*  Adapted from code written by Mario Consuegra                            *
*  https://plus.google.com/111411198512679603620/about                     *
/***************************************************************************/


#ifndef SOLVER_H
#define SOLVER_H

#include <stdint.h>
#include <vector>
#include "constants.h"
#include "matrix.h"

using namespace std;

class Solver {

public:
	Solver(Matrix &m, const int &type);
	Solver(vector<vector<int32_t > > &value);
	double solve(PE_ERROR &err);
	double doubleSolve(PE_ERROR &err, vector<vector<double> > &x);
	double newDoubleSolve(PE_ERROR &err, vector<vector<double>>  &x);
	double newSolve(PE_ERROR &err, vector<double> &x);

private:
	int m_type;
	vector<int> m_dupR, m_dupC;
	vector<vector<double> > m_A;
	vector<vector<double> > m_Ainv;
};

#endif
