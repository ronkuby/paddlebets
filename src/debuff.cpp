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
#include "constants.h"
#include "state.h"
#include "debuff.h"

Debuff::Debuff(const PE_DB_SUBTYPE subType, const PE_DURATION duration, const bool stickActive, const PE_SIDE sourceSide, const PE_PET_NUMBER sourcePet, const PE_IMMUNE_BREAK breaks, const vector<int> &values) {
	m_subType = subType;
	m_duration = duration;
	m_stickActive = stickActive;
	m_sourceSide = sourceSide;
	m_sourcePet = sourcePet;
	m_values = values;
   m_breaks = breaks;
	m_index = 0;
}
