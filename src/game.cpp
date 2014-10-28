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

#include <stdint.h>
#include <vector>
#include <sys/timeb.h>
#include "constants.h"
#include "game.h"

Game::Game() {

	_ftime64_s(&m_start);
	m_turn = -1;
	m_nodes = 0;

	m_value.resize(PE_SIDES);
	for (uint32_t side = 0; side < m_value.size(); side++) {
		m_value[side].resize(PE_TEAM_SIZE);
		for (uint32_t fpet = 0; fpet < m_value[side].size(); fpet++) {
			m_value[side][fpet].resize(PE_TEAM_SIZE);
			for (uint32_t epet = 0; epet < m_value[side][fpet].size(); epet++) {
				m_value[side][fpet][epet].resize(PE_PASS + 1, 0.0);
			}
		}
	}

}

double Game::elapsed() {
	double diff = 0.0;
	struct _timeb now;

	_ftime64_s(&now);
	diff = static_cast<double>(now.time - m_start.time);
	diff += milliseconds*static_cast<double>(now.millitm - m_start.millitm);

	return diff;
}
