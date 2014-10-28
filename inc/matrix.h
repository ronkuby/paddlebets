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

#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>
#include <vector>
#include <assert.h>

const int MATRIX_ROW = 0;
const int MATRIX_COL = 1;
const int MATRIX_BOTH = 2;

using namespace std;

class Matrix {

public:
	Matrix(const int32_t &rows, const int32_t &cols) {
		m_rows = rows;
		m_cols = cols;
		m_dupR = vector<int>(rows, -1);
		m_dupC = vector<int>(cols, -1);

		m_matrix.resize(m_rows);
		for (int jj = 0; jj < m_rows; jj++) {
			m_matrix[jj].resize(m_cols);
		}
	}

	double& operator()(const int row, const int col) {
		int rowp = row, colp = col;

		if (m_dupR[row] >= 0) {
			rowp = m_dupR[row];
		}
		if (m_dupC[col] >= 0) {
			colp = m_dupC[col];
		}

		assert(colp >= 0 && col < m_cols);
		assert(rowp >= 0 && row < m_rows);

		return m_matrix[rowp][colp];
	}

	void set(const int32_t &value) {
		int32_t nValue;

		if (value < 0) nValue = 0;
		else if (value > maxEval) nValue = maxEval;
		else nValue = value;
		for (int jj = 0; jj < m_rows; jj++) {
			for (int ii = 0; ii < m_cols; ii++) {
				m_matrix[jj][ii] = static_cast<double>(nValue);
			}
		}
	}

	vector<vector<double> > *copy() { return &m_matrix; }

	void subrow(const int &index, const double &replace) {
		vector<double> replaces(m_cols, replace);
		int indexp = index;

		if (m_dupR[index] >= 0) {
			indexp = m_dupR[index];
		}

		m_matrix.erase(m_matrix.begin() + indexp);
		m_matrix.push_back(replaces);
	}

	void subcol(const int &index, const double &replace) {
		int indexp = index;

		if (m_dupC[index] >= 0) {
			indexp = m_dupC[index];
		}

		for (int jj = 0; jj < m_rows; jj++) {
			m_matrix[jj].erase(m_matrix[jj].begin() + indexp);
			m_matrix[jj].push_back(replace);
		}
	}

	vector<double> nixrow(const int &index) {
		vector<double> row = m_matrix[index];

		m_matrix.erase(m_matrix.begin() + index);
		m_rows--;

		return row;
	}

	vector<double> nixcol(const int &index) {
		vector<double> col;

		for (int jj = 0; jj < m_rows; jj++) {
			col.push_back(m_matrix[jj][index]);
			m_matrix[jj].erase(m_matrix[jj].begin() + index);
		}
		m_cols--;

		return col;
	}

	void duplicates(const vector<int> &dupR, const vector<int> &dupC) {
		m_dupR = dupR;
		m_dupC = dupC;
	}

	int size(const int &dim) {
		if (dim == 0) {
			return m_rows;
		}
		else {
			return m_cols;
		}
	}

	PE_ERROR solve(int32_t &intValue) {
		double value;

		int domRow = -1, domCol = -1;
		for (int cand = 0; cand < m_rows; cand++) {
			bool dom = true;
			for (int col = 0; col < m_cols; col++) {
				for (int test = 0; test < m_rows; test++) {
					if (m_matrix[test][col] > m_matrix[cand][col]) {
						dom = false;
					}
				}
			}
			if (dom) {
				domRow = cand;
				value = maxEval;
				for (int col = 0; col < m_cols; col++) {
					if (m_matrix[domRow][col] < value) value = m_matrix[domRow][col];
				}
			}
		}
		if (domRow < 0) {
			for (int cand = 0; cand < m_cols; cand++) {
				bool dom = true;
				for (int row = 0; row < m_rows; row++) {
					for (int test = 0; test < m_cols; test++) {
						if (m_matrix[row][test] < m_matrix[row][cand]) {
							dom = false;
						}
					}
				}
				if (dom) {
					domCol = cand;
					value = 0;
					for (int row = 0; row < m_rows; row++) {
						if (m_matrix[row][domCol] > value) value = m_matrix[row][domCol];
					}
				}
			}
		}
		if (domCol == -1 && domRow == -1) {
			return PE_SOLVER_NOSOLUTION;
		}
		else {
			intValue = static_cast<int32_t>(value);
			return PE_OK;
		}
	}

	PE_ERROR pure(int32_t &index, const int &type) {

		int domRow = -1, domCol = -1;
		for (int cand = 0; cand < m_rows; cand++) {
			bool dom = true;
			for (int col = 0; col < m_cols; col++) {
				for (int test = 0; test < m_rows; test++) {
					if (m_matrix[test][col] > m_matrix[cand][col]) {
						dom = false;
					}
				}
			}
			if (dom) {
				domRow = cand;
				if (type == MATRIX_ROW) {
					index = domRow;
					return PE_OK;
				}
				else {
					domCol = 0;
					for (int col = 1; col < m_cols; col++) {
						if (m_matrix[domRow][col] < m_matrix[domRow][domCol]) domCol = col;
					}
					index = domCol;
					return PE_OK;
				}
			}
		}
		if (domRow < 0) {
			for (int cand = 0; cand < m_cols; cand++) {
				bool dom = true;
				for (int row = 0; row < m_rows; row++) {
					for (int test = 0; test < m_cols; test++) {
						if (m_matrix[row][test] < m_matrix[row][cand]) {
							dom = false;
						}
					}
				}
				if (dom) {
					domCol = cand;
					if (type == MATRIX_COL) {
						index = domCol;
						return PE_OK;
					}
					else {
						domRow = 0;
						for (int row = 1; row < m_rows; row++) {
							if (m_matrix[row][domCol] > m_matrix[domRow][domCol]) domRow = row;
						}
						index = domRow;
						return PE_OK;
					}
				}
			}
		}
		return PE_SOLVER_NOSOLUTION;
	}

	PE_ERROR pure(int32_t &rowIndex, int32_t &colIndex, const int &type) {

		if (type != MATRIX_BOTH) {
			return PE_OUT_OF_RANGE;
		}

		rowIndex = -1;
		colIndex = -1;
		for (int cand = 0; cand < m_rows; cand++) {
			bool dom = true;
			for (int col = 0; col < m_cols; col++) {
				for (int test = 0; test < m_rows; test++) {
					if (m_matrix[test][col] > m_matrix[cand][col]) {
						dom = false;
					}
				}
			}
			if (dom) {
				rowIndex = cand;
				colIndex = 0;
				for (int col = 1; col < m_cols; col++) {
					if (m_matrix[rowIndex][col] < m_matrix[rowIndex][colIndex]) colIndex = col;
				}
				return PE_OK;
			}
		}
		if (rowIndex < 0) {
			for (int cand = 0; cand < m_cols; cand++) {
				bool dom = true;
				for (int row = 0; row < m_rows; row++) {
					for (int test = 0; test < m_cols; test++) {
						if (m_matrix[row][test] < m_matrix[row][cand]) {
							dom = false;
						}
					}
				}
				if (dom) {
					colIndex = cand;
					rowIndex = 0;
					for (int row = 1; row < m_rows; row++) {
						if (m_matrix[row][colIndex] > m_matrix[rowIndex][colIndex]) rowIndex = row;
					}
					return PE_OK;
				}
			}
		}
		return PE_SOLVER_NOSOLUTION;
	}

	PE_ERROR solve(int32_t &intValue, const int &solve) {
		double value;

		int domRow = -1, domCol = -1;
		for (int cand = 0; cand < m_rows; cand++) {
			bool dom = true;
			for (int col = 0; col < m_cols; col++) {
				for (int test = 0; test < m_rows; test++) {
					if (m_matrix[test][col] > m_matrix[cand][col]) {
						dom = false;
					}
				}
			}
			if (dom) {
				domRow = cand;
				value = maxEval;
				for (int col = 0; col < m_cols; col++) {
					if (m_matrix[domRow][col] < value) value = m_matrix[domRow][col];
				}
			}
		}
		if (domRow < 0) {
			for (int cand = 0; cand < m_cols; cand++) {
				bool dom = true;
				for (int row = 0; row < m_rows; row++) {
					for (int test = 0; test < m_cols; test++) {
						if (m_matrix[row][test] < m_matrix[row][cand]) {
							dom = false;
						}
					}
				}
				if (dom) {
					domCol = cand;
					value = 0;
					for (int row = 0; row < m_rows; row++) {
						if (m_matrix[row][domCol] > value) value = m_matrix[row][domCol];
					}
				}
			}
		}
		if (domCol == -1 && domRow == -1) {
			return PE_SOLVER_NOSOLUTION;
		}
		else {
			intValue = static_cast<int32_t>(value);
			return PE_OK;
		}
	}

	double average() {

		double aveVal = 0.0;
		for (int row = 0; row < m_rows; row++) {
			for (int col = 0; col < m_cols; col++) {
				aveVal += m_matrix[row][col];
			}
		}
		aveVal /= m_rows*m_cols;
		return aveVal;
	}

	void stabalize() {

		for (int row = 0; row < m_rows; row++) {
			for (int col = 0; col < m_cols; col++) {
				m_matrix[row][col] = floor(m_matrix[row][col] + 0.5);
			}
		}
	}

	void dump() {
		for (int row = 0; row < m_rows; row++) {
			printf("   ");
			for (int col = 0; col < m_cols; col++) {
				printf("%7.2f ", m_matrix[row][col]);
			}
			printf("\r\n");
		}
	}

private:
	vector<int> m_dupR, m_dupC;
	int32_t m_rows, m_cols;
	vector<vector<double> > m_matrix;
};

#endif
