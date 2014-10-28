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

#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <vector>
#include "constants.h"
#include "debuff.h"
#include "turn.h"
#include "tvector.h"

using namespace std;

class State {

public:

   State() {}

   State(const PE_SIDE &sides, const PE_PET_NUMBER &teamSize);

   // access fucntions
   PE_ERROR setStats(const vector<PE_SV> stats, const PE_SIDE side, const PE_PET_NUMBER petNumber);
   PE_ERROR setFamily(const PE_FAMILY family, const PE_SIDE side, const PE_PET_NUMBER petNumber);
   PE_ERROR setGenus(const PE_GENUS genus, const PE_SIDE side, const PE_PET_NUMBER petNumber) { m_genus[side][petNumber] = genus; return PE_OK; }
   PE_SV speed(const PE_SIDE side);
   PE_SV allPower(const PE_SIDE side, const PE_PET_NUMBER petNumber);
   PE_SV health(const PE_SIDE side);
   PE_SV allHealth(const PE_SIDE side, const PE_PET_NUMBER petNumber);
   PE_SV maxHealth(const PE_SIDE side);
   PE_SV maxAllHealth(const PE_SIDE side, const PE_PET_NUMBER petNumber);

   bool gameOver() { return m_gameOver; }
   PE_SIDE winner() { return m_winner; }

   PE_ERROR setHealthActive(const PE_SIDE side, const PE_SV value) { m_health[side][m_activePet[side]] = value;  return PE_OK; }

   bool activeDamaged(const PE_SIDE side) { return m_damaged[side][m_activePet[side]]; }
   bool activeDidDamage(const PE_SIDE side) { return m_didDamage[side][m_activePet[side]]; }

   bool isSpeedWinner(const PE_SIDE side);
   bool isSpeedLoser(const PE_SIDE side);
   PE_SIDE speedWinner() { return m_speedWinner; }
   PE_ERROR setSpeedWinner(const PE_SIDE side) { m_speedWinner = side; return PE_OK; }
   PE_SIDE switchPass() { return m_switchPass; }
   PE_ERROR setswitchPass() { m_switchPass = true; return PE_OK; }
   
   double modAccuracy(const PE_SIDE side, const double current, const PE_PET_NUMBER pet, const PE_PET_NUMBER epet);
   double modAccuracyActive(const PE_SIDE side, const double current);
   double modCrit(const PE_SIDE side, const double current, const PE_PET_NUMBER pet);
   double modCritActive(const PE_SIDE side, const double current);

   PE_PET_NUMBER activePet(const PE_SIDE side);
   PE_PET_NUMBER lastActivePet(const PE_SIDE side);   

   void modMoveBranching(const PE_SIDE side, Turn &turn);
   PE_MOVETYPE executeModMove(const PE_SIDE side, const PE_MOVETYPE type);

   PE_SV lastHit(const PE_SIDE side);

   PE_DURATION lastUsed(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_ABILITY slot) { return m_lastUsed[side][pet][slot]; }
   PE_ERROR used(const PE_SIDE side, const PE_ABILITY slot) { m_lastUsed[side][m_activePet[side]][slot] = PE_RESET_COOLDOWN; return PE_OK; }
   PE_ERROR setMulti(const PE_SIDE side, const PE_DURATION multi) { m_multiRound[side] = multi; return PE_OK; }
   PE_DURATION multi(const PE_SIDE side) { return m_multiRound[side]; }

   bool isBuffed(const PE_SIDE side, const PE_DB_TYPE type, const PE_DB_SUBTYPE subType);
   int getDuration(const PE_SIDE side, const PE_DB_TYPE type, const PE_DB_SUBTYPE subType);

   bool isBleeding(const PE_SIDE side);
   bool isBlinded(const PE_SIDE side);
   bool isBurning(const PE_SIDE side);
   bool isStunned(const PE_SIDE side);
   bool isChilled(const PE_SIDE side);
   bool isImmobile(const PE_SIDE side);
   bool isPoisoned(const PE_SIDE side);
   bool isRooted(const PE_SIDE side);
   bool isWebbed(const PE_SIDE side);

   PE_ERROR pushMoveStack(const PE_SIDE side, const PE_ABILITY ability) { m_moveStack[side].push_back(ability); return PE_OK; }
   PE_ABILITY backMoveStack(const PE_SIDE side) { return m_moveStack[side].back(); }
   int consecutiveCurrent(const PE_SIDE side);

   PE_ERROR activate(const PE_SIDE side, const PE_PET_NUMBER pet);

   PE_ERROR setFamily(const PE_SIDE side, const PE_FAMILY family);
   PE_FAMILY petFamily(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_FP purpose);

   PE_SV checkHitActive(const PE_SIDE side, const PE_FAMILY family, const PE_SV rawHit, const PE_PET_NUMBER source, const bool isCrit, const bool isPrimary, const bool isDot,const bool overRide);
   PE_SV doHit(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_FAMILY family, const PE_SV rawHit, const PE_PET_NUMBER source, const bool isCrit, const bool isPrimary, const bool isDot, const bool overRide);
   PE_SV doHitActive(const PE_SIDE side, const PE_FAMILY family, const PE_SV rawHit, const PE_PET_NUMBER source, const bool isCrit, const bool isPrimary, const bool isDot, const bool overRide);
   PE_SV doHeal(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_SV rawHeal, const bool isPrimary);
   PE_SV doHealActive(const PE_SIDE side, const PE_SV rawHeal, const bool isPrimary);

   bool pushDebuff(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_DB_TYPE type, Debuff &debuff);
   bool pushDebuffActive(const PE_SIDE side, const PE_DB_TYPE type, Debuff &debuff);
   PE_ERROR removeDebuff(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_DB_TYPE type, const PE_DB_SUBTYPE subType, const int track);
   PE_ERROR removeDebuffActive(const PE_SIDE side, const PE_DB_TYPE type, const PE_DB_SUBTYPE subType, const int track);
   PE_ERROR removeDebuffs(const PE_SIDE side, const PE_PET_NUMBER pet);
   PE_ERROR removeDebuffsActive(const PE_SIDE side);
   PE_ERROR removeObjects();

   PE_ERROR setWeather(const PE_WEATHER weather, const PE_AV value);
   PE_WEATHER weather() { return m_weather; }

   PE_ERROR endBranching(Turn &turn);
   PE_ERROR wrapUp(Turn &turn, const uint32_t outcome);

   PE_SV directReduceHealth(const PE_SIDE side, const PE_SV hit);
   PE_SV directIncreaseHealth(const PE_SIDE side, const PE_SV hit);

   PE_ERROR clearDebuffsActive(const PE_SIDE side);

   PE_ERROR checkNoTracks();

   bool isConsumed(const PE_SIDE side, const PE_PET_NUMBER pet) { return m_consumed[side][pet]; }
   PE_ERROR consume(const PE_SIDE side, const PE_PET_NUMBER pet) { m_consumed[side][pet] = true; return PE_OK; }

   PE_ERROR die(const PE_SIDE side, const PE_PET_NUMBER pet);
   PE_ERROR couldDie(const PE_SIDE side, const PE_PET_NUMBER pet);

#if PE_VERBOSE > PE_VERBOSE_OFF
   void setVerbosity(int verbosity) { m_verbosity = verbosity; }
#endif

private:
   PE_SV modHit(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_SV hit);
   double factorTake(const PE_SIDE side, const PE_PET_NUMBER pet, const bool removeHits);
   double factorGive(const PE_SIDE side, const PE_PET_NUMBER pet, const bool removeHits);
   double factorHeal(const PE_SIDE side, const PE_PET_NUMBER pet);
   bool immuneHit(const PE_SIDE side, const PE_PET_NUMBER pet, const bool initial, const PE_PET_NUMBER source, const PE_IMMUNE_BREAK breaks, const bool removeHits);

   PE_SV damageBackTaken(const PE_SIDE side, const PE_PET_NUMBER pet, PE_SV strike); // debuff on side taking damage
   PE_SV healingBackTaken(const PE_SIDE side, const PE_PET_NUMBER pet);// debuff on side taking damage
   PE_SV damageBackGiven(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_PET_NUMBER source);

   PE_SV reduceHealth(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_SV hit);
   PE_SV increaseHealth(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_SV heal);

   PE_ERROR clearExpired(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_DB_TYPE type);

   PE_ERROR incRound(); // inc all probably part of a larger end turn() function

   vector<vector<PE_FAMILY> > m_baseFamily;
   vector<vector<PE_SV> > m_baseHealth;
   vector<vector<PE_SV> > m_basePower;
   vector<vector<PE_SV> > m_baseSpeed;

   vector<vector<PE_SV> > m_maxHealth;
   vector<vector<PE_FAMILY> > m_family;
   vector<vector<PE_GENUS> > m_genus;
   vector<vector<PE_SV> > m_health;
   vector<vector<PE_SV> > m_speed;

   bool m_gameOver;
   PE_SIDE m_winner;

   vector<PE_PET_NUMBER> m_lastActive;
   vector<PE_PET_NUMBER> m_activePet;
   vector<vector<vector<PE_DURATION> > > m_lastUsed; // rounds since ability last used
   vector<PE_DURATION> m_multiRound;

   vector<vector<PE_ABILITY> > m_moveStack;

   vector<vector<vector<Tvector> > > m_debuff;

   PE_AV m_weatherValue;
   PE_DURATION m_weatherCooldown;
   PE_WEATHER m_weather;

   vector<vector<vector<PE_SV> > > m_hit;   // damage stack for each pet (taken)

   vector<vector<bool> > m_damaged;   // whether a pet has been damaged so far in a round
   vector<vector<bool> > m_didDamage; // whether a pet inflicted damage so far in a round
   vector<vector<bool> > m_consumed;  // whether a dead pet has been eaten

   PE_SIDE m_speedWinner; // 0-1: this side won, 2: not set, 3: one side switch/passed
   bool m_switchPass; // whether either sideswitched or passed

#if PE_VERBOSE > PE_VERBOSE_OFF
   int m_verbosity;
#endif
};
   
#endif
   
