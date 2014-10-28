/****************************************************************************/
/*  Copyright(C) 2014 Jon Goldstein (ronkuby@brasscube.com)                 *
*                                                                          *
*  This program is free software : you can redistribute it and / or modify *
*  it under the terms of the GNU General Public License as published by    *
*  the Free Software Foundation, either version 3 of the License, or       *
*  (at your option) any later version.                                     *
*                                                                          *
*  This program is distributed in the hope that it will be useful,         *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the             *
*  GNU General Public License for more details.                            *
*                                                                          *
*  You should have received a copy of the GNU General Public License       *
*  along with this program.If not, see <http://www.gnu.org/licenses/>.     *
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include "solver.h"

using namespace std;

bool GetPivots(vector<vector<double> > &simplex, int &pivotCol, int &pivotRow, bool &noSolution) {
	int numRows = simplex.size();
	int numCols = simplex[0].size();
	int numVariables = numCols - numRows - 1;

	noSolution = false;
	double min = 0;
	for (int iCol = 0; iCol < numCols - 2; iCol++) {
		double value = simplex[numRows - 1][iCol];
		if (value < min) {
			pivotCol = iCol;
			min = value;
		}
	}
	if (min == 0) return false;

	double minRatio = 0.0;
	bool first = true;
	for (int iRow = 0; iRow < numRows - 1; iRow++) {
		double value = simplex[iRow][pivotCol];

		if (value > 0.0) {
			double colValue = simplex[iRow][numCols - 1];
			double ratio = colValue / value;

			if ((first || ratio < minRatio) && ratio >= 0.0) {
				minRatio = ratio;
				pivotRow = iRow;
				first = false;
			}
		}
	}
	noSolution = first;

	return !noSolution;
}

vector<double> DoSimplex(vector<vector<double> > &simplex, double &max) {
	int pivotCol, pivotRow;
	int numRows = simplex.size();
	int numCols = simplex[0].size();
	int pivots = 0;

	bool noSolution = false;
	while (GetPivots(simplex, pivotCol, pivotRow, noSolution)) {
		pivots++;
		if (pivots == maxSimplex) {
			noSolution = true;
			break;
		}
		double pivot = simplex[pivotRow][pivotCol];

		for (int iCol = 0; iCol < numCols; iCol++) {
			simplex[pivotRow][iCol] /= pivot;
		}
		for (int iRow = 0; iRow < numRows; iRow++) {
			if (iRow != pivotRow) {
				double ratio = -1 * simplex[iRow][pivotCol];

				for (int iCol = 0; iCol < numCols; iCol++) {
					simplex[iRow][iCol] = simplex[pivotRow][iCol] * ratio + simplex[iRow][iCol];
				}
			}
		}
	}
	if (noSolution) {
		vector<double> vec;
		return vec;
	}

	//optimo!!!
	max = simplex[numRows - 1][numCols - 1];
	int numVariables = numCols - numRows - 1;
	vector<double> x(numVariables, 0);

	for (int iCol = 0; iCol < numVariables; iCol++) {
		bool isUnit = true;
		bool first = true;
		double value;

		for (int j = 0; j < numRows; j++) {
			if (simplex[j][iCol] == 1.0 && first) {
				first = false;
				value = simplex[j][numCols - 1];
			}
			else if (simplex[j][iCol] != 0.0) {
				isUnit = false;
			}
		}
		if (isUnit && !first) x[iCol] = value;
		else x[iCol] = 0.0;
	}

	return x;
}

vector<vector<double> > SetSimplex(vector<double> &maxFunction, vector<vector<double> > &A, vector<double> &b) {
	vector<vector<double> > simplex;
	int numVariables = maxFunction.size();
	int numEquations = A.size();
	int numCols = numVariables + numEquations + 1 + 1;

	for (int iRow = 0; iRow < numEquations; iRow++) {
		vector<double> row(numCols, 0);

		for (int iCol = 0; iCol < numVariables; iCol++) {
			row[iCol] = A[iRow][iCol];
		}
		row[numVariables + iRow] = 1;
		row[numCols - 1] = b[iRow];
		simplex.push_back(row);
	}

	vector<double> lastRow(numCols, 0);
	for (int iVar = 0; iVar < numVariables; iVar++) {
		lastRow[iVar] = 0 - maxFunction[iVar];
	}
	lastRow[numVariables + numEquations] = 1;
	simplex.push_back(lastRow);

	return simplex;
}

vector<vector<double> > Transpose(vector<vector<double> > &M) {
	vector<vector<double> > T;
	int mNumRows = M.size();
	int mNumCols = M[0].size();

	for (int mCol = 0; mCol < mNumCols; mCol++) {
		vector<double> tRow;

		for (int mRow = 0; mRow < mNumRows; mRow++) {
			tRow.push_back(M[mRow][mCol]);
		}
		T.push_back(tRow);
	}

	return T;
}

vector<vector<double> > invTranspose(vector<vector<double> > &M) {
	vector<vector<double> > T;
	int mNumRows = M.size();
	int mNumCols = M[0].size();

	for (int mCol = 0; mCol < mNumCols; mCol++) {
		vector<double> tRow;

		for (int mRow = 0; mRow < mNumRows; mRow++) {
			tRow.push_back(maxEval - M[mRow][mCol]);
		}
		T.push_back(tRow);
	}

	return T;
}

vector<vector<double> > SetLinearProgram(vector<vector<double> > A) {
	vector<double> maxFunc;
	vector<double> b;
	vector<vector<double> > T = Transpose(A);

	for (uint32_t iRow = 0; iRow < T.size(); iRow++) {
		for (uint32_t iCol = 0; iCol < T[iRow].size(); iCol++) {
			T[iRow][iCol] *= -1.0;
		}
		T[iRow].push_back(1.0);
		T[iRow].push_back(-1.0);
	}

	for (uint32_t iRow = 0; iRow < T.size(); iRow++) {
		b.push_back(0.0);
	}

	int rowSize = T[0].size();
	vector<double> rowEq1(rowSize, 1.0);
	rowEq1[rowSize - 2] = rowEq1[rowSize - 1] = 0.0;
	T.push_back(rowEq1);
	b.push_back(1.0);

	vector<double> rowEq2(rowSize, -1.0);
	rowEq2[rowSize - 2] = rowEq2[rowSize - 1] = 0.0;
	T.push_back(rowEq2);
	b.push_back(-1.0);

	for (int i = 0; i < rowSize; i++) {
		if (i < rowSize - 2) maxFunc.push_back(1.0);
		else if (i < rowSize - 1) maxFunc.push_back(1.0);
		else maxFunc.push_back(-1.0);
	}

	return SetSimplex(maxFunc, T, b);
}

Solver::Solver(vector<vector<int32_t > > &value) {
	vector<int> dupR(value.size(), -1), dupC(value[0].size(), -1);

	m_type = MATRIX_BOTH;
	// check for duplicate rows
	for (uint32_t jj = 0; jj < value.size() - 1; jj++) {
		if (dupR[jj] < 0) {
			for (uint32_t kk = jj + 1; kk < value.size(); kk++) {
				bool duplicate = true;
				for (uint32_t ii = 0; ii < value[jj].size(); ii++) {
					if (value[kk][ii] != value[jj][ii]) {
						duplicate = false;
						break;
					}
				}
				if (duplicate) {
					dupR[kk] = jj;
				}
			}
		}
	}
	// check for duplicate cols
	for (uint32_t ii = 0; ii < value[0].size() - 1; ii++) {
		if (dupC[ii] < 0) {
			for (uint32_t kk = ii + 1; kk < value[0].size(); kk++) {
				bool duplicate = true;
				for (uint32_t jj = 0; jj < value.size(); jj++) {
					if (value[jj][kk] != value[jj][ii]) {
						duplicate = false;
						break;
					}
				}
				if (duplicate) {
					dupC[kk] = ii;
				}
			}
		}
	}

	for (uint32_t jj = 0; jj < value.size(); jj++) {
		if (dupR[jj] < 0) {
			vector<double> row;

			for (uint32_t ii = 0; ii < value[jj].size(); ii++) {
				if (dupC[ii] < 0) {
					row.push_back(static_cast<double>(value[jj][ii]));
				}
			}
			m_A.push_back(row);
		}
	}
	m_Ainv = invTranspose(m_A);
	m_dupR = dupR;
	m_dupC = dupC;
}

double Solver::solve(PE_ERROR &err) {
	vector<vector<double> > simplex = SetLinearProgram(m_A);
	double max;
	vector<double> result = DoSimplex(simplex, max);
	int size = result.size();
	if (!size) {
		err = PE_SOLVER_NOSOLUTION;
		return 0.0;
	}
	//printf("Result: Max = %f\n", result[size - 2] - result[size - 1]);

	for (uint32_t i = 0; i < result.size() - 2; i++) {
		printf("x%d = %f ; ", i, result[i]);
	}
	printf("\r\n");

	return result[size - 2] - result[size - 1];
}

double Solver::doubleSolve(PE_ERROR &err, vector<vector<double> > &x) {
	double max;
	vector<vector<double> > simplex = SetLinearProgram(m_A);
	vector<double> result = DoSimplex(simplex, max);
	int size = result.size();
	if (!size) {
		err = PE_SOLVER_NOSOLUTION;
		return 0.0;
	}

	x.resize(PE_SIDES);
	int rdups = 0;
	vector<int> countr(m_dupR.size(), 1);
	for (uint32_t jj = 0; jj < m_dupR.size(); jj++) {
		if (m_dupR[jj] >= 0) {
			countr[m_dupR[jj]]++;
			x[0].push_back(x[0][m_dupR[jj]]);
			rdups++;
		}
		else {
			x[0].push_back(result[jj - rdups]);
		}
	}
	for (uint32_t jj = 0; jj < m_dupR.size(); jj++) {
		if (m_dupR[jj] >= 0) {
			x[0][jj] /= countr[m_dupR[jj]];
		}
		else if (countr[jj] > 1) {
			x[0][jj] /= countr[jj];
		}
	}

	vector<vector<double> > simplex2 = SetLinearProgram(m_Ainv);
	vector<double> result2 = DoSimplex(simplex2, max);
	int size2 = result2.size();

	int cdups = 0;
	vector<int> countc(m_dupC.size(), 1);
	for (uint32_t ii = 0; ii < m_dupC.size(); ii++) {
		if (m_dupC[ii] >= 0) {
			countc[m_dupC[ii]]++;
			x[1].push_back(x[1][m_dupC[ii]]);
			cdups++;
		}
		else {
			x[1].push_back(result2[ii - cdups]);
		}
	}
	for (uint32_t ii = 0; ii < m_dupC.size(); ii++) {
		if (m_dupC[ii] >= 0) {
			x[1][ii] /= countc[m_dupC[ii]];
		}
		else if (countc[ii] > 1) {
			x[1][ii] /= countc[ii];
		}
	}

	/*printf("side 1 ");
	for(int i = 0; i < result2.size() - 2; i++) {
	printf("x%d = %f ", i, result2[i]);
	}
	printf("\r\n");*/

	return result2[size2 - 2] - result2[size2 - 1];
}


Solver::Solver(Matrix &m, const int &type) {
	vector<vector<double> > *temp = m.copy();
	vector<int> dupR(temp->size(), -1), dupC((*temp)[0].size(), -1);

	// check for duplicate rows
	for (uint32_t jj = 0; jj < temp->size() - 1; jj++) {
		if (dupR[jj] < 0) {
			for (uint32_t kk = jj + 1; kk < temp->size(); kk++) {
				bool duplicate = true;
				for (uint32_t ii = 0; ii < (*temp)[jj].size(); ii++) {
					if ((*temp)[kk][ii] != (*temp)[jj][ii]) {
						duplicate = false;
						break;
					}
				}
				if (duplicate) {
					dupR[kk] = jj;
				}
			}
		}
	}
	// check for duplicate cols
	for (uint32_t ii = 0; ii < (*temp)[0].size() - 1; ii++) {
		if (dupC[ii] < 0) {
			for (uint32_t kk = ii + 1; kk < (*temp)[0].size(); kk++) {
				bool duplicate = true;
				for (uint32_t jj = 0; jj < temp->size(); jj++) {
					if ((*temp)[jj][kk] != (*temp)[jj][ii]) {
						duplicate = false;
						break;
					}
				}
				if (duplicate) {
					dupC[kk] = ii;
				}
			}
		}
	}
	m.duplicates(dupR, dupC);

	for (uint32_t jj = 0; jj < temp->size(); jj++) {
		if (dupR[jj] < 0) {
			vector<double> row;

			for (uint32_t ii = 0; ii < (*temp)[jj].size(); ii++) {
				if (dupC[ii] < 0) {
					row.push_back((*temp)[jj][ii]);
				}
			}
			m_A.push_back(row);
		}
	}
	if (type == MATRIX_COL || MATRIX_BOTH) {
		m_Ainv = invTranspose(m_A);
	}
	m_dupR = dupR;
	m_dupC = dupC;
	m_type = type;
}

double Solver::newSolve(PE_ERROR &err, vector<double> &x) {
	double max;
	vector<double> result;
	int size;
	double value;


	if (m_type == MATRIX_ROW) {
		vector<vector<double> > simplex = SetLinearProgram(m_A);
		result = DoSimplex(simplex, max);
		size = result.size();

		if (!size) {
			err = PE_SOLVER_NOSOLUTION;
			return 0.0;
		}

		int rdups = 0;
		vector<int> countr(m_dupR.size(), 1);
		for (uint32_t jj = 0; jj < m_dupR.size(); jj++) {
			if (m_dupR[jj] >= 0) {
				countr[m_dupR[jj]]++;
				x.push_back(x[m_dupR[jj]]);
				rdups++;
			}
			else {
				x.push_back(result[jj - rdups]);
			}
		}
		for (uint32_t jj = 0; jj < m_dupR.size(); jj++) {
			if (m_dupR[jj] >= 0) {
				x[jj] /= countr[m_dupR[jj]];
			}
			else if (countr[jj] > 1) {
				x[jj] /= countr[jj];
			}
		}
		value = result[size - 2] - result[size - 1];
	}
	else if (m_type == MATRIX_COL) {

		vector<vector<double> > simplex = SetLinearProgram(m_Ainv);
		result = DoSimplex(simplex, max);
		size = result.size();

		if (!size) {
			err = PE_SOLVER_NOSOLUTION;
			return 0.0;
		}

		int cdups = 0;
		vector<int> countc(m_dupC.size(), 1);
		for (uint32_t ii = 0; ii < m_dupC.size(); ii++) {
			if (m_dupC[ii] >= 0) {
				countc[m_dupC[ii]]++;
				x.push_back(x[m_dupC[ii]]);
				cdups++;
			}
			else {
				x.push_back(result[ii - cdups]);
			}
		}
		for (uint32_t ii = 0; ii < m_dupC.size(); ii++) {
			if (m_dupC[ii] >= 0) {
				x[ii] /= countc[m_dupC[ii]];
			}
			else if (countc[ii] > 1) {
				x[ii] /= countc[ii];
			}
		}
		value = maxEval - (result[size - 2] - result[size - 1]);
	}

	return value;
}

double Solver::newDoubleSolve(PE_ERROR &err, vector<vector<double>> &x) {
	double max;
	vector<double> result;
	int size;

	vector<vector<double> > simplex = SetLinearProgram(m_A);
	result = DoSimplex(simplex, max);
	size = result.size();

	if (!size) {
		err = PE_SOLVER_NOSOLUTION;
		return 0.0;
	}
	x.resize(PE_SIDES);

	int rdups = 0;
	vector<int> countr(m_dupR.size(), 1);
	for (uint32_t jj = 0; jj < m_dupR.size(); jj++) {
		if (m_dupR[jj] >= 0) {
			countr[m_dupR[jj]]++;
			x[0].push_back(x[0][m_dupR[jj]]);
			rdups++;
		}
		else {
			x[0].push_back(result[jj - rdups]);
		}
	}
	for (uint32_t jj = 0; jj < m_dupR.size(); jj++) {
		if (m_dupR[jj] >= 0) {
			x[0][jj] /= countr[m_dupR[jj]];
		}
		else if (countr[jj] > 1) {
			x[0][jj] /= countr[jj];
		}
	}

	vector<double> result2;
	int size2;

	vector<vector<double> > simplex2 = SetLinearProgram(m_Ainv);
	result2 = DoSimplex(simplex2, max);
	size2 = result2.size();

	if (!size2) {
		err = PE_SOLVER_NOSOLUTION;
		return 0.0;
	}

	int cdups = 0;
	vector<int> countc(m_dupC.size(), 1);
	for (uint32_t ii = 0; ii < m_dupC.size(); ii++) {
		if (m_dupC[ii] >= 0) {
			countc[m_dupC[ii]]++;
			x[1].push_back(x[1][m_dupC[ii]]);
			cdups++;
		}
		else {
			x[1].push_back(result2[ii - cdups]);
		}
	}
	for (uint32_t ii = 0; ii < m_dupC.size(); ii++) {
		if (m_dupC[ii] >= 0) {
			x[1][ii] /= countc[m_dupC[ii]];
		}
		else if (countc[ii] > 1) {
			x[1][ii] /= countc[ii];
		}
	}

	return result[size - 2] - result[size - 1];
}

