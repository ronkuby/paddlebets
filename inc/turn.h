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

#ifndef TURN_H
#define TURN_H

#include <stdint.h>
#include <vector>
#include "constants.h"
#include "petinfo.h"

using namespace std;

const uint32_t one = 1;

typedef enum PE_BRANCHTYPE {
   PE_TURN_MOVE,
   PE_TURN_END
};

typedef enum PE_MOVETYPE {
   PE_MT_PASS_SWITCH, // zero attack moves
   PE_MT_NO_ATTACK, // buffs & heals
   PE_MT_MISS = 100, // single attack moves
   PE_MT_HIT,
   PE_MT_HITHIGH,
   PE_MT_HITLOW,
   PE_MT_CRIT,
   PE_MT_ONE_ATTACK,
   PE_MT_HITHIT = 200,
   PE_MT_HITCRIT,
   PE_MT_CRITHIT,
   PE_MT_CRITCRIT,
   PE_MT_MISSHIT,
   PE_MT_TWO_ATTACKS,
   PE_MT_HITHIGHHIT,
   PE_MT_HITLOWHIT,
   PE_MT_HITHITHIT = 300,
   PE_MT_HITHITCRIT,
   PE_MT_HITCRITHIT,
   PE_MT_CRITHITHIT,
   PE_MT_HITCRITCRIT,
   PE_MT_CRITHITCRIT,
   PE_MT_CRITCRITHIT,
   PE_MT_CRITCRITCRIT,
   PE_MT_THREE_ATTACKS,
   PE_MT_HITHITHITHIT = 400,
   PE_MT_HITHITHITCRIT,
   PE_MT_HITHITCRITHIT,
   PE_MT_HITCRITHITHIT,
   PE_MT_CRITHITHITHIT,
   PE_MT_HITHITCRITCRIT,
   PE_MT_HITCRITHITCRIT,
   PE_MT_HITCRITCRITHIT,
   PE_MT_CRITHITHITCRIT,
   PE_MT_CRITHITCRITHIT,
   PE_MT_CRITCRITHITHIT,
   PE_MT_CRITCRITCRITHIT,
   PE_MT_CRITCRITHITCRIT,
   PE_MT_CRITHITCRITCRIT,
   PE_MT_HITCRITCRITCRIT,
   PE_MT_CRITCRITCRITCRIT,
   PE_MT_FOUR_ATTACKS,
   PE_MT_HITHITHITHITHIT = 500,
   PE_MT_CRITHITHITHITHIT,
   PE_MT_CRITCRITHITHITHIT,
   PE_MT_CRITCRITCRITHITHIT,
   PE_MT_CRITCRITCRITCRITHIT,
   PE_MT_CRITCRITCRITCRITCRIT,
   PE_MT_FIVE_ATTACKS,
   PE_MT_NORMAL_ATTACKS,
   PE_MT_HIT_EFFECT = 1024,
   PE_MT_HITLOW_EFFECT,
   PE_MT_HITHIGH_EFFECT,
   PE_MT_CRIT_EFFECT,
   PE_MT_MOD_MOVES = 2047,
   PE_MT_MAGMA_TRAP = 2048
};

class Turn {

public:
   void push(const PE_MOVETYPE type, const double prob) {
      m_type.push_back(type);
      m_prob.push_back(prob);
   }
   int branches() { return static_cast<int>(m_type.size()); }
   PE_MOVETYPE type(const int number) { return m_type[number]; }
   bool doAction(const int type, const int action) { return ((one<<action)&(m_type[type])) > 0; }
   vector<double> *probs() { return &m_prob; }
   double prob(const int number) { return m_prob[number]; }
   void probMod(const int number, const double factor) { m_prob[number] *= factor; }
   void clear() {
      m_type.clear();
      m_prob.clear();
   }

private:
   // these attributes are set when the user is about to move
   vector<PE_MOVETYPE> m_type;
   vector<double> m_prob;
   PE_BRANCHTYPE m_branchType;
};

#endif
