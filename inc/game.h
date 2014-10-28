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

#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <sys/timeb.h>
#include <vector>
#include "constants.h"

using namespace std;

const double milliseconds = 1.0e-3;

class Game {

public:
	Game();

	void incTurn() { m_turn++; }
	void decTurn() { m_turn--; }
	int32_t turn() { return m_turn; }
	void incNodes() { m_nodes++; }
	int32_t nodes() { return m_nodes; }
	double elapsed();
	void incValue(const int &side, const int &fpet, const int &epet, const int &move, const double &value) {
		m_value[side][fpet][epet][move] += value;
	}
	double getValue(const int &side, const int &fpet, const int &epet, const int &move) {
		return m_value[side][fpet][epet][move];
	}

private:
	vector<vector<vector<vector<double> > > > m_value;

	struct _timeb m_start;
	int32_t m_turn;
	int32_t m_nodes;
};

#endif
