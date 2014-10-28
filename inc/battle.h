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

#ifndef BATTLE_H
#define BATTLE_H

#include <stdint.h>
#include <memory>
#include "constants.h"
#include "petinfo.h"
#include "move.h"
#include "state.h"
#include "turn.h"
#include "game.h"

using namespace std;

typedef shared_ptr<Move> move_ptr;
typedef shared_ptr<Game> game_ptr;

class Battle {

public:
   Battle(Petinfo &petInfo, PE_TEAM_SELECTION &teamA, PE_TEAM_SELECTION &teamB, int verbosity);
   //Battle::Battle(Petinfo &petInfo, PE_TEAM_SELECTION &teamA, PE_TEAM_SELECTION &teamB, game_ptr &game);
   // a constructor with two pets
   // a constructor with two pets and an evaluator?

   PE_ERROR getMoves(vector<vector<int> > &move, bool &switchOnly);
   PE_ERROR getSpeeds(const int &move1, const int &move2, vector<PE_SV> &speed);
   PE_ERROR getBranching(const PE_SIDE &side, const int &move, Turn &turn);
   PE_ERROR doMove(const PE_SIDE &side, const int &ability, const PE_MOVETYPE &type);
   PE_ERROR doSimpleMove(const PE_SIDE &side, const int &ability);
   PE_ERROR getBranchingEndturn(Turn &turn);
   PE_ERROR finishTurn(Turn &turn, const uint32_t outcome);

   int32_t simpleEval();
   bool gameOver(PE_SIDE &winner);

   PE_ERROR getStatus() { return m_status; }
   int32_t nodes() { return m_game->nodes(); }
   void incTurn() { m_game->incNodes(); m_game->incTurn(); }
   void decTurn() { m_game->decTurn(); }
   int32_t turn() { return m_game->turn(); }
   PE_ERROR incMoveValue(const PE_SIDE &side, const int &move, const double &value);
   double getMoveValue(const PE_SIDE &side, const int &move);
   game_ptr game() { return m_game; }

   void reportTime();
   double time();
   void dumpSituation();
#if PE_VERBOSE > PE_VERBOSE_OFF
   void setVerbosity(int verbosity) { m_verbosity = verbosity; m_state.setVerbosity(verbosity); }
#endif

private:
   PE_ERROR m_status;

#if PE_VERBOSE > PE_VERBOSE_OFF
   int m_verbosity;
   vector<vector<string> > m_petName;
   vector<vector<vector< string> > > m_abilityName;
#endif

   State m_state;

   vector<vector<move_ptr> > m_move[PE_SIDES];
   game_ptr m_game;
};

#endif
