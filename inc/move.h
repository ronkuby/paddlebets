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

#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>
#include <vector>
#include "constants.h"
#include "petinfo.h"
#include "turn.h"
#include "state.h"
#include "debuff.h"
#include "tvector.h"

using namespace std;

class Move {

public:
   Move(const PE_SIDE side, const PE_FAMILY family, const PE_MOVE moveType, const vector<PE_AV> &abilityValues, const PE_DURATION cooldown, const PE_PET_NUMBER pet);

   // processing functions
   PE_SV speed(State &state) { return m_speed(m_side, state); }
   PE_ERROR branches(State &state, Turn &currentTurn) { return m_branches(*this, maxProb, state, currentTurn); }
   PE_SV execute(const PE_MOVETYPE &type, State &state) { return m_move(*this, type, state); }

   // access functions
   PE_SIDE side() { return m_side; }
   PE_PET_NUMBER pet() { return m_pet; }
   PE_FAMILY family() { return m_family; }
   vector<PE_AV> abilityValues() { return m_abilityValues; }
   double baseProb() { return m_baseProb; }
   double critProb() { return m_critProb; }
   double effectProb() { return m_effectProb; }
   PE_DURATION cooldown() { return m_cooldown; }
   vector<PE_DB_TYPE> debuffType() { return m_debuffType; }
   vector<Debuff> debuff() { return m_debuff; }

private:
   // these attributes do not change after initialization, with a few exceptions
   PE_SIDE m_side;
   PE_PET_NUMBER m_pet;
   PE_FAMILY m_family;
   vector<PE_AV> m_abilityValues;
   double m_baseProb; // base proability of hitting
   double m_critProb; // base proability of critting
   double m_effectProb; // base probability of proccing additional effect
   PE_DURATION m_cooldown;
   vector<PE_DB_TYPE> m_debuffType;
   vector<Debuff> m_debuff;
   
   //int m_speed;
   // the effective speed for the move
   PE_SV (*m_speed)(const PE_SIDE &side, State &state);
   // the number of branches for the current move.
   // information such as weather, enemy buffs, user debuffs, and whether this is the first move should all be in state and
   // accounted for by this calculation
   PE_ERROR (*m_branches)(Move &move, const double &multiplier, State &state, Turn &currentTurn);
   // execute move #branch.  return the adjusted state and the probability of the brach occurring
   PE_SV (*m_move)(Move &move, const PE_MOVETYPE &type, State &state);
};

#endif
