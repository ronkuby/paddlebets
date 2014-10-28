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
#include <utility>
#include <vector>
#include <string>
#include "constants.h"
#include "battle.h"
#include "petinfo.h"
#include "move.h"
#include "state.h"
#include "turn.h"

Battle::Battle(Petinfo &petInfo, PE_TEAM_SELECTION &teamA, PE_TEAM_SELECTION &teamB, int verbosity) {
   PE_ERROR error;
   PE_MOVE moveA, moveB;
   PE_DURATION cooldownA, cooldownB;
   PE_AN expectedNumAVA, expectedNumAVB;
   vector<PE_AV> abilityValuesA, abilityValuesB;
   int abilityA, abilityB;
   PE_FAMILY abilityFamilyA, abilityFamilyB;
   State state(PE_SIDES, PE_TEAM_SIZE);

   m_verbosity = verbosity;
   m_move[PE_SIDEA].resize(PE_TEAM_SIZE);
   m_move[PE_SIDEB].resize(PE_TEAM_SIZE);
#if PE_VERBOSE > PE_VERBOSE_OFF
   m_petName.resize(PE_SIDES);
   m_abilityName.resize(PE_SIDES);
   m_petName[PE_SIDEA].resize(PE_TEAM_SIZE);
   m_petName[PE_SIDEB].resize(PE_TEAM_SIZE);
   m_abilityName[PE_SIDEA].resize(PE_TEAM_SIZE);
   m_abilityName[PE_SIDEB].resize(PE_TEAM_SIZE);
#endif
   for (int pet = PE_PET1; pet < PE_TEAM_SIZE; pet++) {
      state.setFamily(petInfo.getFamily(teamA.pet[pet].petNumber), PE_SIDEA, (PE_PET_NUMBER)pet);
      state.setFamily(petInfo.getFamily(teamB.pet[pet].petNumber), PE_SIDEB, (PE_PET_NUMBER)pet);
      state.setGenus(petInfo.getGenus(teamA.pet[pet].petNumber), PE_SIDEA, (PE_PET_NUMBER)pet);
      state.setGenus(petInfo.getGenus(teamB.pet[pet].petNumber), PE_SIDEB, (PE_PET_NUMBER)pet);
      state.setStats(petInfo.getStats(teamA.pet[pet].petNumber, teamA.pet[pet].breedNumber), PE_SIDEA, (PE_PET_NUMBER)pet);
      state.setStats(petInfo.getStats(teamB.pet[pet].petNumber, teamB.pet[pet].breedNumber), PE_SIDEB, (PE_PET_NUMBER)pet);
#if PE_VERBOSE > PE_VERBOSE_OFF
   m_petName[PE_SIDEA][pet] = petInfo.getPetName(teamA.pet[pet].petNumber);
   m_petName[PE_SIDEB][pet] = petInfo.getPetName(teamB.pet[pet].petNumber);
   m_abilityName[PE_SIDEA][pet].resize(PE_ABILITIES);
   m_abilityName[PE_SIDEB][pet].resize(PE_ABILITIES);
#endif
      for (int move = PE_SLOT1; move < PE_ABILITIES; move++) {
         abilityA = move*PE_SELECTIONS + teamA.pet[pet].abilitySelection[move];
         petInfo.getAbilityValues(teamA.pet[pet].petNumber, teamA.pet[pet].breedNumber, (PE_ABILITY)abilityA, abilityValuesA);
         string abilityStringA(petInfo.getAbilityName(teamA.pet[pet].petNumber, (PE_ABILITY)abilityA, abilityFamilyA));
#if PE_VERBOSE > PE_VERBOSE_REQ
		 printf("%d %d %d %s\n", pet, move, abilityFamilyA, abilityStringA.c_str());
#endif
		 error = petInfo.getMove(abilityStringA, state.allPower(PE_SIDEA, pet), abilityFamilyA, moveA, expectedNumAVA, cooldownA);
		 if (error == PE_NEED_INFO) {
			 abilityValuesA.push_back(expectedNumAVA);
			 expectedNumAVA = 1;
			 error = PE_OK;
		 }
         if (error != PE_OK) {
#if PE_VERBOSE > PE_VERBOSE_OFF
			 printf("Unknown ability %s for pet %s\n", abilityStringA.c_str(), m_petName[PE_SIDEA][pet].c_str());
#endif
            exit(9);
         }
         else if (expectedNumAVA != abilityValuesA.size()) {
#if PE_VERBOSE > PE_VERBOSE_OFF
            printf("Wrong number of abilitiy numbers on ability %s for pet %s\n", abilityStringA, m_petName[PE_SIDEA][pet].c_str());
#endif
            exit(9);
         }
         else {
            m_move[PE_SIDEA][pet].push_back(move_ptr(new Move(PE_SIDEA, abilityFamilyA, moveA, abilityValuesA, cooldownA, pet)));
#if PE_VERBOSE > PE_VERBOSE_REQ
            printf("  %d -- ", moveA);
            for (uint32_t avn = 0; avn < abilityValuesA.size(); avn++) {
               printf("%d ", abilityValuesA[avn]);
            }
            printf("\n");
#endif
         }
         abilityB = move*PE_SELECTIONS + teamB.pet[pet].abilitySelection[move];
         petInfo.getAbilityValues(teamB.pet[pet].petNumber, teamB.pet[pet].breedNumber, (PE_ABILITY)abilityB, abilityValuesB);
         string abilityStringB(petInfo.getAbilityName(teamB.pet[pet].petNumber, (PE_ABILITY)abilityB, abilityFamilyB));
#if PE_VERBOSE > PE_VERBOSE_REQ
		 printf("%d %d %d %s\n", pet, move, abilityFamilyB, abilityStringB.c_str());
#endif
		 error = petInfo.getMove(abilityStringB, state.allPower(PE_SIDEB, pet), abilityFamilyB, moveB, expectedNumAVB, cooldownB);
		 if (error == PE_NEED_INFO) {
			 abilityValuesB.push_back(expectedNumAVB);
			 expectedNumAVB = 1;
			 error = PE_OK;
		 }
		 if (error != PE_OK) {
#if PE_VERBOSE > PE_VERBOSE_OFF
            printf("Unknown ability %s for pet %s\n", abilityStringB, m_petName[PE_SIDEB][pet].c_str());
#endif
            exit(9);
         }
         else if (expectedNumAVB != abilityValuesB.size()) {
#if PE_VERBOSE > PE_VERBOSE_OFF
            printf("Wrong number of abilitiy numbers on ability %s for pet %s\n", abilityStringB, m_petName[PE_SIDEB][pet].c_str());
#endif
            exit(9);
         }
         else {
            m_move[PE_SIDEB][pet].push_back(move_ptr(new Move(PE_SIDEB, abilityFamilyB, moveB, abilityValuesB, cooldownB, pet)));
#if PE_VERBOSE > PE_VERBOSE_REQ
            printf("  %d -- ", moveB);
			for (uint32_t avn = 0; avn < abilityValuesB.size(); avn++) {
               printf("%d ", abilityValuesB[avn]);
            }
            printf("\n");
#endif
         }
#if PE_VERBOSE > PE_VERBOSE_OFF
   m_abilityName[PE_SIDEA][pet][move] = abilityStringA;
   m_abilityName[PE_SIDEB][pet][move] = abilityStringB;
#endif
      }
   }
   m_game = game_ptr(new Game());
   m_state = state;
#if PE_VERBOSE > PE_VERBOSE_OFF
   state.setVerbosity(m_verbosity);
#endif

   m_status = PE_OK;
}

// should not modify state
PE_ERROR Battle::getMoves(vector<vector<int> > &move, bool &switchOnly) {
   bool forceSwitch[PE_SIDES] = {false, false};

   move.resize(PE_SIDES);
   for (PE_SIDE side = PE_SIDEA; side < PE_SIDES; side++) {
      move[side].clear();
      PE_PET_NUMBER activePet = m_state.activePet(side);
      if (activePet == PE_NO_ACTIVE) {
         forceSwitch[side] = true;
         for (int pet = PE_PET1; pet < PE_TEAM_SIZE; pet++) {
            if (m_state.allHealth(side, (PE_PET_NUMBER)pet) > 0) {
               move[side].push_back(PE_SWITCH1 + pet - PE_PET1);
            }
         }
      }
      else {
         if (m_state.multi(side) < PE_ZERO_ROUND) {
            if (!m_state.isImmobile(side)) {
               for (int slot = PE_SLOT1; slot < PE_ABILITIES; slot++) {
                  if (m_state.lastUsed(side, activePet, (PE_ABILITY)slot) > m_move[side][activePet][slot]->cooldown()) {
                     move[side].push_back(slot);
                  }
               }
            }
            // check no switching
            if (!m_state.isRooted(side)) {
               for (int pet = PE_PET1; pet < PE_TEAM_SIZE; pet++) {
                  if (m_state.allHealth(side, (PE_PET_NUMBER)pet) > 0 && m_state.activePet(side) != pet) {
                     move[side].push_back(PE_SWITCH1 + pet - PE_PET1);
                  }
               }
            }
            move[side].push_back(PE_PASS);
         }
         else {
            move[side].push_back(m_state.backMoveStack(side));
         }
      }  
   }
   if (!forceSwitch[PE_SIDEA] && forceSwitch[PE_SIDEB]) {
      move[PE_SIDEA].clear();
      move[PE_SIDEA].push_back(PE_PASS);
   }
   if (!forceSwitch[PE_SIDEB] && forceSwitch[PE_SIDEA]) {
      move[PE_SIDEB].clear();
      move[PE_SIDEB].push_back(PE_PASS);
   }
   switchOnly = forceSwitch[PE_SIDEA]||forceSwitch[PE_SIDEB];

   return PE_OK;
}

// should not modify state
PE_ERROR Battle::getSpeeds(const int &move1, const int &move2, vector<PE_SV> &speed) {

   if (speed.size() == 0) speed.resize(PE_SIDES);
   if (move1 == PE_PASS) speed[PE_SIDEA] = 0;
   else if (move1 > PE_ABILITIES) { // goes first, but loses speed war
      speed[PE_SIDEA] = PE_INFINITE;
   }
   else {
      speed[PE_SIDEA] = m_move[PE_SIDEA][m_state.activePet(PE_SIDEA)][move1]->speed(m_state);
   }
   if (move2 == PE_PASS) speed[PE_SIDEB] = 0;
   else if (move2 > PE_ABILITIES) {
      speed[PE_SIDEB] = PE_INFINITE; // goes first, but loses speed war
   }
   else {
      speed[PE_SIDEB] = m_move[PE_SIDEB][m_state.activePet(PE_SIDEB)][move2]->speed(m_state);
   }
#if PE_VERBOSE > PE_VERBOSE_REQ
   if (m_verbosity > PE_VERBOSE_REQ) {
      if (speed[PE_SIDEA] > speed[PE_SIDEB]) {
         printf("%s(%d) goes first (%d vs %d)\r\n", m_petName[PE_SIDEA][m_state.activePet(PE_SIDEA)].c_str(), PE_SIDEA + 1, speed[PE_SIDEA], speed[PE_SIDEB]);
      }
      else if (speed[PE_SIDEA] < speed[PE_SIDEB]) {
         printf("%s(%d) goes first (%d vs %d)\r\n", m_petName[PE_SIDEB][m_state.activePet(PE_SIDEB)].c_str(), PE_SIDEB + 1, speed[PE_SIDEB], speed[PE_SIDEA]);
      }
   }
#endif
#if PE_VERBOSE > PE_VERBOSE_OFF
   if (m_verbosity > PE_VERBOSE_OFF) {
      m_state.checkNoTracks();
   }
#endif

   return PE_OK;
}

// not ok to modify state
PE_ERROR Battle::getBranching(const PE_SIDE &side, const int &move, Turn &turn) {

   turn.clear();
   if (move < PE_ABILITIES) {
      if (m_state.activePet(side) == PE_NO_ACTIVE) { // the pets died
         turn.push(PE_MT_PASS_SWITCH, maxProb);
      }
      else if (m_state.isImmobile(side)) { // pet got stunned
         turn.push(PE_MT_PASS_SWITCH, maxProb);
      }
      else {
         m_move[side][m_state.activePet(side)][move]->branches(m_state, turn);
         m_state.modMoveBranching(side, turn);
      }
   }
   else {
      turn.push(PE_MT_PASS_SWITCH, maxProb);
   }

   return PE_OK;
}

// not ok to modify state
// accounts for states with highly randomized game changing events
PE_ERROR Battle::getBranchingEndturn(Turn &turn) {

   turn.clear();
   m_state.endBranching(turn);

   return PE_OK;
}

// ok to modify state (duh)
PE_ERROR Battle::doMove(const PE_SIDE &side, const int &move, const PE_MOVETYPE &type) {

   m_state.pushMoveStack(side, static_cast<PE_ABILITY>(move));
   if (type != PE_MT_PASS_SWITCH) {
      if (move < PE_ABILITIES) {
         if (m_state.speedWinner() >= PE_SIDES) {
            m_state.setSpeedWinner(side);
         }
#if PE_VERBOSE > PE_VERBOSE_REQ
         if (m_verbosity > PE_VERBOSE_REQ) {
            printf("Side %d %s uses %s.\r\n", side + 1, m_petName[side][m_state.activePet(side)].c_str(), m_abilityName[side][m_state.activePet(side)][move].c_str());
         }
#endif
         if (type > PE_MT_MOD_MOVES) {
            PE_MOVETYPE modType = m_state.executeModMove(side, type);
         }
         if (type < PE_MT_MOD_MOVES) {
            m_state.used(side, static_cast<PE_ABILITY>(move));
            m_move[side][m_state.activePet(side)][move]->execute(type, m_state);
         }
      }
      else {
         printf("big time\r\n");
         exit(9);
      }
   }
   else {
#if PE_VERBOSE > PE_VERBOSE_REQ
      if (m_verbosity > PE_VERBOSE_REQ) {
         if (move == PE_PASS) {
            printf("Side %d passes.\r\n", side + 1);
         }
         else if (move > PE_ABILITIES) {
            printf("Side %d switches to pet %d.\r\n", side + 1, move - PE_ABILITIES);
         }
      }
#endif
      m_state.setSpeedWinner(PE_SIDES + 1);
      if (move > PE_ABILITIES && move != PE_PASS) {
         m_state.activate(side, move - PE_SWITCH1);
      }
   }
#if PE_VERBOSE > PE_VERBOSE_OFF
   m_state.checkNoTracks();
#endif
#if PE_VERBOSE > PE_VERBOSE_REQ
   if (m_verbosity > PE_VERBOSE_REQ) {
      printf("Side %d move finish.\r\n", side + 1);
   }
#endif

   return PE_OK;
}

PE_ERROR Battle::doSimpleMove(const PE_SIDE &side, const int &ability) {

   if (ability < PE_ABILITIES) {
      return PE_OUT_OF_RANGE;
   }
   else {
#if PE_VERBOSE > PE_VERBOSE_REQ
      if (m_verbosity > PE_VERBOSE_REQ) {
         if (ability == PE_PASS) {
            printf("Side %d passes.\r\n", side + 1);
         }
         else if (ability > PE_ABILITIES) {
            printf("Side %d switches to pet %d.\r\n", side + 1, ability - PE_ABILITIES);
         }
      }
#endif

      m_state.setSpeedWinner(PE_SIDES + 1);
      if (ability != PE_PASS) {
         m_state.pushMoveStack(side, static_cast<PE_ABILITY>(ability));
         m_state.activate(side, ability - PE_SWITCH1);
      }
   }
#if PE_VERBOSE > PE_VERBOSE_OFF
   if (m_verbosity > PE_VERBOSE_OFF) {
      m_state.checkNoTracks();
   }
#endif
#if PE_VERBOSE > PE_VERBOSE_REQ
   if (m_verbosity > PE_VERBOSE_REQ) {
      printf("Side %d move finish.\r\n", side + 1);
   }
#endif

   return PE_OK;
}

PE_ERROR Battle::finishTurn(Turn &turn, const uint32_t outcome) {
   PE_ERROR err = m_state.wrapUp(turn, outcome);

#if PE_VERBOSE > PE_VERBOSE_OFF
   m_state.checkNoTracks();
#endif

   return err;
}

int32_t Battle::simpleEval() {
   int32_t sideAHealth;
   int32_t sideBHealth;

   sideAHealth = m_state.allHealth(PE_SIDEA, PE_PET1) + m_state.allHealth(PE_SIDEA, PE_PET2) + m_state.allHealth(PE_SIDEA, PE_PET3);
   sideBHealth = m_state.allHealth(PE_SIDEB, PE_PET1) + m_state.allHealth(PE_SIDEB, PE_PET2) + m_state.allHealth(PE_SIDEB, PE_PET3);
   int32_t totalHealth = sideAHealth + sideBHealth;

   if (totalHealth > 0) {
      return (maxEval*sideAHealth)/totalHealth;
   }
   else {
      return maxEval/2;
   }
}

bool Battle::gameOver(PE_SIDE &winner) {

   if (m_state.gameOver()) {
      winner = m_state.winner();
      return true;
   }
   else {
      return false;
   }

}

PE_ERROR Battle::incMoveValue(const PE_SIDE &side, const int &move, const double &value) {
   PE_PET_NUMBER fpet = m_state.lastActivePet(side), epet = m_state.lastActivePet(side^1);

   if (fpet != PE_NO_ACTIVE && epet != PE_NO_ACTIVE) {
      m_game->incValue(side, fpet, epet, move, value);
   }

   return PE_OK;
}

double Battle::getMoveValue(const PE_SIDE &side, const int &move) {
   PE_PET_NUMBER fpet = m_state.activePet(side), epet = m_state.activePet(side^1);

   if (fpet != PE_NO_ACTIVE && epet != PE_NO_ACTIVE) {
      m_game->getValue(side, fpet, epet, move);
   }

   return PE_OK;
}

#if PE_VERBOSE > PE_VERBOSE_OFF
string cutTo(const string &original, const uint32_t max) {
   string buffer;

   buffer = original;
   if (buffer.length() > max) buffer.erase(buffer.begin() + max, buffer.end());
   
   return buffer;
}
#endif


void Battle::dumpSituation() {

#if PE_VERBOSE > PE_VERBOSE_OFF
   const int maxPetName = 21;
   const int maxAbilityName = 18;

   printf("=================================================================================================\r\n");
   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      printf("|                            %s", cutTo(m_abilityName[PE_SIDEA][pet][0], maxAbilityName).c_str());
	  for (uint32_t i = 0; i < maxAbilityName - cutTo(m_abilityName[PE_SIDEA][pet][0], maxAbilityName).length(); i++) printf(" ");
      printf(" |                            %s", cutTo(m_abilityName[PE_SIDEB][pet][0], maxAbilityName).c_str());
	  for (uint32_t i = 0; i < maxAbilityName - cutTo(m_abilityName[PE_SIDEB][pet][0], maxAbilityName).length(); i++) printf(" ");
      printf(" |\r\n");
      printf("| %s", cutTo(m_petName[PE_SIDEA][pet], maxPetName).c_str());
	  for (uint32_t i = 0; i < maxPetName - cutTo(m_petName[PE_SIDEA][pet], maxPetName).length(); i++) printf(" ");
      printf(" %4d %s", m_state.allHealth(PE_SIDEA, pet), cutTo(m_abilityName[PE_SIDEA][pet][1], maxAbilityName).c_str());
	  for (uint32_t i = 0; i < maxAbilityName - cutTo(m_abilityName[PE_SIDEA][pet][1], maxAbilityName).length(); i++) printf(" ");
      printf(" | %s", cutTo(m_petName[PE_SIDEB][pet], maxPetName).c_str());
	  for (uint32_t i = 0; i < maxPetName - cutTo(m_petName[PE_SIDEB][pet], maxPetName).length(); i++) printf(" ");
      printf(" %4d %s", m_state.allHealth(PE_SIDEB, pet), cutTo(m_abilityName[PE_SIDEB][pet][1], maxAbilityName).c_str());
	  for (uint32_t i = 0; i < maxAbilityName - cutTo(m_abilityName[PE_SIDEB][pet][1], maxAbilityName).length(); i++) printf(" ");
      printf(" |\r\n");
      if (m_state.activePet(PE_SIDEA) == pet) {
         printf("|          *****             %s", cutTo(m_abilityName[PE_SIDEA][pet][2], maxAbilityName).c_str());
      }
      else {
         printf("|                            %s", cutTo(m_abilityName[PE_SIDEA][pet][2], maxAbilityName).c_str());
      }
	  for (uint32_t i = 0; i < maxAbilityName - cutTo(m_abilityName[PE_SIDEA][pet][2], maxAbilityName).length(); i++) printf(" ");
      if (m_state.activePet(PE_SIDEB) == pet) {
         printf(" |          *****             %s", cutTo(m_abilityName[PE_SIDEB][pet][2], maxAbilityName).c_str());
      }
      else {
         printf(" |                            %s", cutTo(m_abilityName[PE_SIDEB][pet][2], maxAbilityName).c_str());
      }
	  for (uint32_t i = 0; i < maxAbilityName - cutTo(m_abilityName[PE_SIDEB][pet][2], maxAbilityName).length(); i++) printf(" ");
      printf(" |\r\n");
      if (pet != PE_TEAM_SIZE - 1) {
         printf("|-----------------------------------------------|-----------------------------------------------|\r\n");
      }
   }
   printf("=================================================================================================\r\n");
#endif

}


void Battle::reportTime() {

   printf("elapsed time is %9.3fs\r\n", m_game->elapsed());
}

double Battle::time() {

   return m_game->elapsed();
}
