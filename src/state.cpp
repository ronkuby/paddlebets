/***************************************************************************
*  Copyright(C) 2014 Jon Goldstein (ronkuby@brasscube.com)                 *
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
****************************************************************************/

#include <stdint.h>
#include <vector>
#include "constants.h"
#include "state.h"
#include "tvector.h"

State::State(const PE_SIDE &sides, const PE_PET_NUMBER &teamSize) {
   m_family.resize(sides);
   m_genus.resize(sides);
   m_health.resize(sides);
   m_maxHealth.resize(sides);
   m_speed.resize(sides);

   m_baseFamily.resize(sides);
   m_baseHealth.resize(sides);
   m_basePower.resize(sides);
   m_baseSpeed.resize(sides);

   m_activePet.resize(sides);
   m_lastActive.resize(sides);
   m_lastUsed.resize(sides);
   m_moveStack.resize(sides);

   m_hit.resize(sides);

   m_damaged.resize(sides);
   m_didDamage.resize(sides);
   m_consumed.resize(sides);

   m_multiRound.resize(sides);
   m_debuff.resize(PE_DB_TYPES);
   for (int debuff = 0; debuff < PE_DB_TYPES; debuff++) {
      m_debuff[debuff].resize(sides);
   }

   for (int side = 0; side < sides; side++) {
      m_family[side].resize(teamSize);
      m_genus[side].resize(teamSize);
      m_health[side].resize(teamSize);
      m_maxHealth[side].resize(teamSize);
      m_speed[side].resize(teamSize);
      m_baseFamily[side].resize(teamSize);
      m_baseHealth[side].resize(teamSize);
	  m_basePower[side].resize(teamSize);
	  m_baseSpeed[side].resize(teamSize);

      for (int debuff = 0; debuff < PE_DB_TYPES; debuff++) {
         m_debuff[debuff][side].resize(teamSize);
      }

      m_hit[side].resize(teamSize);

      m_activePet[side] = PE_NO_ACTIVE;
      m_lastUsed[side].resize(teamSize);
      m_damaged[side].resize(teamSize);
      m_didDamage[side].resize(teamSize);
      m_consumed[side].resize(teamSize);
      m_multiRound[side] = PE_NO_ROUNDS;
      for (int pet = 0; pet < teamSize; pet++) {
         m_lastUsed[side][pet].assign(PE_ABILITIES, PE_LONG_COOLDOWN);
         m_damaged[side][pet] = false;
         m_didDamage[side][pet] = false;
         m_consumed[side][pet] = false;
      }
   }   
   m_weather = PE_NO_WEATHER;
   m_speedWinner = PE_SIDES;
   m_switchPass = false;
   m_gameOver = false;
   
}

PE_ERROR State::setStats(const vector<PE_SV> stats, const PE_SIDE side, const PE_PET_NUMBER petNumber) {

   if (side >= PE_SIDES) {
      return PE_OUT_OF_RANGE;
   }
   if (stats.size() < PE_STATS) {
      return PE_OUT_OF_RANGE;
   }
   if (petNumber >= PE_TEAM_SIZE) {
      return PE_OUT_OF_RANGE;
   }

   m_baseHealth[side][petNumber] = stats[PE_STAT_HEALTH];
   m_basePower[side][petNumber] = stats[PE_STAT_POWER];
   m_baseSpeed[side][petNumber] = stats[PE_STAT_SPEED];
   m_health[side][petNumber] = stats[PE_STAT_HEALTH];
   m_maxHealth[side][petNumber] = stats[PE_STAT_HEALTH];
   m_speed[side][petNumber] = stats[PE_STAT_SPEED];

   return PE_OK;
}

PE_ERROR State::setFamily(const PE_FAMILY family, const PE_SIDE side, const PE_PET_NUMBER petNumber) {

   if (side >= PE_SIDES) {
      return PE_OUT_OF_RANGE;
   }
   if (family >= PE_FAMILIES) {
      return PE_OUT_OF_RANGE;
   }
   if (petNumber >= PE_TEAM_SIZE) {
      return PE_OUT_OF_RANGE;
   }

   m_baseFamily[side][petNumber] = family;
   m_family[side][petNumber] = family;

   return PE_OK;
}

PE_SV State::health(const PE_SIDE side) {
   // add modifications here

   return m_health[side][m_activePet[side]];
}

PE_SV State::allHealth(const PE_SIDE side, const PE_PET_NUMBER petNumber) {
   // add modifications here

   return m_health[side][petNumber];
}

PE_SV State::maxHealth(const PE_SIDE side) {
   // add modifications here

   return m_maxHealth[side][m_activePet[side]];
}

PE_SV State::maxAllHealth(const PE_SIDE side, const PE_PET_NUMBER petNumber) {
   // add modifications here

   return m_maxHealth[side][petNumber];
}

PE_SV State::allPower(const PE_SIDE side, const PE_PET_NUMBER petNumber) {
	// add modifications here

	return m_basePower[side][petNumber];
}

PE_ERROR State::activate(const PE_SIDE side, const int pet) {
   PE_PET_NUMBER from = m_lastActive[side];
   vector<PE_SV> debuffValues;

   if (m_health[side][pet] <= 0) {
      return PE_OK;
   }
   Tvector *dbl;

   // activate switch buffs/debuffs
   if (!m_debuff[PE_DB_MOBILITY][side][from].empty()) {
      dbl = &m_debuff[PE_DB_MOBILITY][side][from];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() == PE_DB_MOBILITY_SANCTIFIED_GROUND) {
            debuffValues.push_back(static_cast<int>(PE_CELESTIAL_BLESSING*maxEval));
            this->pushDebuff(side, pet, PE_DB_DAMAGED, Debuff(PE_DB_DAMAGED_CELESTIAL_BLESSING, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
            removeDebuff(side, from, static_cast<PE_DB_TYPE>(PE_DB_MOBILITY), db->subType(), track);
         }
      }
   }
   if (!m_debuff[PE_DB_OVERTIME][side][from].empty()) {
      dbl = &m_debuff[PE_DB_OVERTIME][side][from];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() == PE_DB_OVERTIME_FROSTBITE) {
            removeDebuff(side, from, static_cast<PE_DB_TYPE>(PE_DB_OVERTIME), db->subType(), track);
         }
      }
   }

   // copy sticky debuffs
   for (int debuff = 0; debuff < PE_DB_TYPES; debuff++) {
      if (!m_debuff[debuff][side][from].empty()) {
         dbl = &m_debuff[debuff][side][from];
         for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
            Debuff *db = dbl->on(track);
            if (db->stick()) {
               pushDebuff(side, pet, static_cast<PE_DB_TYPE>(debuff), Debuff(*db));
            }
         }
      }
   }
   // delete sticky debuffs
   for (int debuff = 0; debuff < PE_DB_TYPES; debuff++) {
      if (!m_debuff[debuff][side][from].empty()) {
         dbl = &m_debuff[debuff][side][from];
         for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
            Debuff *db = dbl->on(track);
            if (db->stick()) {
               removeDebuff(side, from, static_cast<PE_DB_TYPE>(debuff), db->subType(), track);
            }
         }
      }
   }

   // now rooted if there is a mudslide
   if (m_weather == PE_WEATHER_MUDSLIDE) {
      pushDebuff(side, pet, PE_DB_MOBILITY, Debuff(PE_DB_MOBILITY_ROOTED, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
   }

   m_activePet[side] = pet;
   m_multiRound[side] = PE_NO_ROUNDS;
   m_lastActive[side] = pet;

   // activate switch triggers/traps
   if (!m_debuff[PE_DB_MOBILITY][side][pet].empty()) {
      dbl = &m_debuff[PE_DB_MOBILITY][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() > PE_DB_MOBILITY_SWITCH_START && db->subType() < PE_DB_MOBILITY_SWITCH_END) {
            this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, false);
            removeDebuff(side, pet, static_cast<PE_DB_TYPE>(PE_DB_MOBILITY), db->subType(), track);
         }
      }
   }

   return PE_OK;
}

PE_PET_NUMBER State::activePet(const PE_SIDE side) {

   return m_activePet[side];
}

PE_PET_NUMBER State::lastActivePet(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) {
      return m_lastActive[side];
   }

   return m_activePet[side];
}

PE_ERROR State::setFamily(const PE_SIDE side, const PE_FAMILY family) {

   if (m_activePet[side] == PE_NO_ACTIVE) return PE_OK;
   m_family[side][m_activePet[side]] = family;

   return PE_OK;
}

PE_FAMILY State::petFamily(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_FP purpose) {
   PE_FAMILY family = m_family[side][pet];

   if (purpose == PE_FP_DAMAGE_TAKEN) {
      if (!m_debuff[PE_DB_DAMAGED][side][pet].empty()) {
         Tvector *dbl = &m_debuff[PE_DB_DAMAGED][side][pet];
         for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
            Debuff *db = dbl->on(track);
            if (db->subType() == PE_DB_DAMAGED_UNDEAD_MARK) {
               family = PE_FAMILY_UNDEAD;
               dbl->end(track);
               break;
            }
         }
      }

   }

   return family;
}

PE_SV State::lastHit(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) {
      return 0;
   }
   if (m_hit[side][m_activePet[side]].size() == 0) {
      return 0;
   }

   return m_hit[side][m_activePet[side]].back();
}

PE_SV State::speed(const PE_SIDE side) {
   PE_SV baseSpeed = m_speed[side][m_activePet[side]];
   double modSpeed = 0;
   Tvector *dbl;

   if (!m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].empty()) {
      dbl = &m_debuff[PE_DB_MOBILITY][side][m_activePet[side]];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() > PE_DB_MOBILITY_SPEED_START && db->subType() < PE_DB_MOBILITY_SPEED_END) {
            modSpeed += static_cast<double>(db->value(0)) / maxEval;
         }
      }
   }
   if (petFamily(side, m_activePet[side], PE_FP_GENERAL) == PE_FAMILY_FLYING &&
      m_health[side][m_activePet[side]] > PE_FLYING_THRESHOLD*m_maxHealth[side][m_activePet[side]]) {
      modSpeed += PE_FLYING_SPEED;
   }
   if (!m_debuff[PE_DB_IMMUNITY][side][m_activePet[side]].empty()) {
      dbl = &m_debuff[PE_DB_IMMUNITY][side][m_activePet[side]];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() == PE_DB_IMMUNE_SILK_COCOON) {
            modSpeed = -1.0;
         }
      }
   }

   return static_cast<PE_SV>((1.0 + modSpeed)*baseSpeed);
}

bool State::isSpeedWinner(const PE_SIDE side) {
   if (m_speedWinner >= PE_SIDES || side == m_speedWinner) return true;
   else return false;
}

bool State::isSpeedLoser(const PE_SIDE side) {
   if (m_speedWinner != PE_SIDES && side != m_speedWinner) return true;
   else return false;
}

double State::modAccuracy(const PE_SIDE side, const double current, const PE_PET_NUMBER pet, const PE_PET_NUMBER epet) {
   double accuracy = current;

   // check buffs/debuffs on attacking side
   if (m_weather == PE_WEATHER_SANDSTORM && petFamily(side, pet, PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
      accuracy -= PE_SANDSTORM_REDUCTION;
   }
   if (m_weather == PE_WEATHER_DARKNESS && petFamily(side, pet, PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
      accuracy -= PE_DARKNESS_REDUCTION;
   }
   Tvector *dbl;
   if (!m_debuff[PE_DB_MOBILITY][side][pet].empty()) {
      dbl = &m_debuff[PE_DB_MOBILITY][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() > PE_DB_MOBILITY_ACCURACY_START && db->subType() < PE_DB_MOBILITY_ACCURACY_END) {
            accuracy += static_cast<double>(db->value(0)) / maxEval;
         }
      }
   }
   if (!m_debuff[PE_DB_MOBILITY][side ^ 1][epet].empty()) {
      dbl = &m_debuff[PE_DB_MOBILITY][side ^ 1][epet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() > PE_DB_MOBILITY_ACCURACY_AGAINST_START &&
            db->subType() < PE_DB_MOBILITY_ACCURACY_AGAINST_END) {
            accuracy += static_cast<double>(db->value(0)) / maxEval;
         }
      }
   }

   if (accuracy > maxProb) accuracy = maxProb;
   if (accuracy < 0.0) accuracy = 0.0;

   return accuracy;
}

double State::modAccuracyActive(const PE_SIDE side, const double current) {

   if (m_activePet[side] == PE_NO_ACTIVE || m_activePet[side^1] == PE_NO_ACTIVE) return 0.0;
   return modAccuracy(side, current, m_activePet[side], m_activePet[side^1]);
}

double State::modCrit(const PE_SIDE side, const double current, const PE_PET_NUMBER pet) {
   double crit = current;

   // check buffs/debuffs on attacking side
   if (!m_debuff[PE_DB_MOBILITY][side][pet].empty()) {
      Tvector *dbl = &m_debuff[PE_DB_MOBILITY][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() > PE_DB_MOBILITY_CRIT_START && db->subType() < PE_DB_MOBILITY_CRIT_END) {
            crit += static_cast<double>(db->value(0)) / maxEval;
         }
      }
   }
   if (crit < 0.0) crit = 0.0;

   return crit;
}

double State::modCritActive(const PE_SIDE side, const double current) {

   if (m_activePet[side] == PE_NO_ACTIVE) return 0.0;
   return modCrit(side, current, m_activePet[side]);
}

PE_SV State::modHit(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_SV hit) {
   // check the reduction to damage on the active pet due to weather, mitigation, etc..
   PE_SV modHit = hit;

   if (m_weather == PE_WEATHER_SANDSTORM) {
      modHit -= m_weatherValue;
   }
   if (!m_debuff[PE_DB_DAMAGED][side][pet].empty()) {
      Tvector *dbl = &m_debuff[PE_DB_DAMAGED][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() > PE_DB_DAMAGED_MOD_TAKEN_ADD_START && db->subType() < PE_DB_DAMAGED_MOD_TAKEN_ADD_END) {
            modHit += db->value(0);
         }
      }
   }

   return modHit;
}

void State::modMoveBranching(const PE_SIDE side, Turn &turn) {

   if (m_activePet[side] == PE_NO_ACTIVE) return;
   if (!turn.branches()) return;

   if (!m_debuff[PE_DB_OVERTIME][side][m_activePet[side]].empty()) {
      Tvector *dbl = &m_debuff[PE_DB_OVERTIME][side][m_activePet[side]];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() == PE_DB_OVERTIME_MAGMA_TRAP || db->subType() == PE_DB_OVERTIME_SNAP_TRAP) {
            if (turn.type(0) != PE_MT_NO_ATTACK) {
               int preBranches = turn.branches();
               for (int branch = 0; branch < preBranches; branch++) {
                  turn.push(static_cast<PE_MOVETYPE>(PE_MT_MAGMA_TRAP | turn.type(branch)), turn.prob(branch)*PE_MAGMA_TRAP);
                  turn.probMod(branch, maxProb - PE_MAGMA_TRAP);
               }
            }
         }
      }
   }

}

PE_MOVETYPE State::executeModMove(const PE_SIDE side, const PE_MOVETYPE type) {
   vector<PE_SV> debuffValues;

   if (isImmobile(side)) {
      return type;
   }
   else {
      if (type&PE_MT_MAGMA_TRAP) {
         PE_SV damage;
         if (!m_debuff[PE_DB_OVERTIME][side][m_activePet[side]].empty()) {
            Tvector *dbl = &m_debuff[PE_DB_OVERTIME][side][m_activePet[side]];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->subType() == PE_DB_OVERTIME_MAGMA_TRAP) {
                  damage = db->value(0);
                  this->removeDebuffActive(side, PE_DB_OVERTIME, PE_DB_OVERTIME_MAGMA_TRAP, track);
                  dbl->end(track);
                  break;
               }
               else if (db->subType() == PE_DB_OVERTIME_SNAP_TRAP) {
                  damage = db->value(0);
                  this->removeDebuffActive(side, PE_DB_OVERTIME, PE_DB_OVERTIME_SNAP_TRAP, track);
                  dbl->end(track);
                  break;
               }
            }
         }
         this->doHitActive(side, PE_FAMILY_ELEMENTAL, damage, PE_NO_ACTIVE, false, true, false, false);
         this->pushDebuffActive(side, PE_DB_MOBILITY, Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, m_activePet[side], PE_IB_NONE, debuffValues));
      }
      if (isImmobile(side)) return type;
      else return static_cast<PE_MOVETYPE>(type&PE_MT_MOD_MOVES);
   }
}

double State::factorTake(const PE_SIDE side, const PE_PET_NUMBER pet, const bool removeHits) {
   // check the increase/reduction percentage to damage on the active pet
   // check the increase/reduction percentage to damage on the active pet
   double hitFactor = 1.0;

   if (!m_debuff[PE_DB_DAMAGED][side][pet].empty()) {
      Tvector *dbl = &m_debuff[PE_DB_DAMAGED][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() < PE_DB_DAMAGED_MOD_TAKEN_FACTOR_END) {
            hitFactor *= (1.0 + static_cast<double>(db->value(0)) / maxEval);
            if (db->subType() == PE_DB_DAMAGED_OVERLOAD && removeHits) {
               vector<int> debuffValues;
               debuffValues.push_back(static_cast<int>(PE_HEARTBREAK*maxEval));
               this->pushDebuff(side, pet, PE_DB_DAMAGED, Debuff(PE_DB_DAMAGED_HEARTBREAK, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
            }
            if (db->subType() > PE_DB_DAMAGED_MOD_TAKEN_FACTOR_HIT_START && removeHits) {
               if (!db->dec(1)) { // remove single attack effects -- note this applies to dots
                  this->removeDebuff(side, pet, PE_DB_DAMAGED, db->subType(), track);
               }
            }
         }
      }
   }

   return hitFactor;
}

double State::factorGive(const PE_SIDE side, const PE_PET_NUMBER pet, const bool removeHits) {
   // check the increase/reduction percentage to damage on the active pet
   // check the increase/reduction percentage to damage on the active pet
   double hitFactor = 1.0;

   if (!m_debuff[PE_DB_DAMAGED][side][pet].empty()) {
      Tvector *dbl = &m_debuff[PE_DB_DAMAGED][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() > PE_DB_DAMAGED_MOD_GIVEN_FACTOR_START && db->subType() < PE_DB_DAMAGED_MOD_GIVEN_FACTOR_END) {
            hitFactor *= (1.0 + static_cast<double>(db->value(0)) / maxEval);
            if (db->subType() > PE_DB_DAMAGED_MOD_GIVEN_FACTOR_HIT_START && removeHits) {
               if (!db->dec(1)) { // remove single attack effects -- note this applies to dots
                  if (db->subType() == PE_DB_DAMAGED_PROWL) {
                     this->removeDebuff(side, pet, PE_DB_MOBILITY, PE_DB_MOBILITY_PROWL, PE_NO_TRACK);
                  }
                  this->removeDebuff(side, pet, PE_DB_DAMAGED, db->subType(), track);
               }
            }
         }
      }
   }

   return hitFactor;
}

bool State::immuneHit(const PE_SIDE side, const PE_PET_NUMBER pet, const bool initial, const PE_PET_NUMBER source, const PE_IMMUNE_BREAK breaks, const bool removeHits) {
   // for blocks we can decrement the value and remove debuff as necessary

   if (breaks == PE_IB_ALL) return false;

   Tvector *dbl;
   if (!m_debuff[PE_DB_IMMUNITY][side][pet].empty()) {
      dbl = &m_debuff[PE_DB_IMMUNITY][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (initial) {
            if (db->subType() < PE_DB_IMMUNE_PHASE_END) {
               dbl->end(track);
               return true;
            }
            else if (db->subType() < PE_DB_IMMUNE_LOCATION_END) {
               if (breaks == PE_IB_LOCATION) continue;
               if (db->subType() == PE_DB_IMMUNE_UNDERGROUND || db->subType() == PE_DB_IMMUNE_SUBMERGED) {
                  if (source != PE_NO_ACTIVE) {
                     bool ugroundHit = false;
                     Tvector *dbl2 = &m_debuff[PE_DB_IMMUNITY][side ^ 1][source];
                     for (int track2 = dbl2->initTrack(); dbl2->notEnd(track); dbl2->next(track)) {
                        Debuff *db2 = dbl2->on(track);
                        if (db2->subType() == PE_DB_IMMUNE_BREAK_UNDERGROUND) ugroundHit = true;
                     }
                     if (!ugroundHit) {
                        dbl->end(track);
                        return true;
                     }
                  }
                  else {
                     dbl->end(track);
                     return true;
                  }
               }
               else {
                  dbl->end(track);
                  return true;
               }
            }
            else if (db->subType() < PE_DB_IMMUNE_BLOCKS_END && removeHits) {
               if (!db->dec(0)) {
                  this->removeDebuff(side, pet, PE_DB_IMMUNITY, db->subType(), track);
               }
               dbl->end(track);
               return true;
            }
         }
      }
   }

   // look for blocks on attacking side
   if (!m_debuff[PE_DB_IMMUNITY][side ^ 1][m_lastActive[side ^ 1]].empty()) {
      dbl = &m_debuff[PE_DB_IMMUNITY][side ^ 1][m_lastActive[side ^ 1]];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (initial) {
            if (db->subType() > PE_DB_IMMUNE_BLOCKS_ALL_START && db->subType() < PE_DB_IMMUNE_BLOCKS_END && removeHits) {
               if (!db->dec(0)) {
                  this->removeDebuff(side ^ 1, m_lastActive[side ^ 1], PE_DB_IMMUNITY, db->subType(), track);
               }
               dbl->end(track);
               return true;
            }
         }
      }
   }

   return false;
}

PE_SV State::damageBackTaken(const PE_SIDE side, const PE_PET_NUMBER pet, PE_SV strike) {
   PE_SV damageBack = 0;

   if (!m_debuff[PE_DB_DAMAGED][side][pet].empty()) {
      Tvector *dbl = &m_debuff[PE_DB_DAMAGED][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         PE_DB_SUBTYPE subType = db->subType();
         if (m_activePet[side] == PE_NO_ACTIVE) {
            dbl->end(track);
            break;
         }
         if (db->subType() > PE_DB_DAMAGED_MOD_TAKEN_DAMAGEBACK_START &&
            db->subType() < PE_DB_DAMAGED_MOD_TAKEN_DAMAGEBACK_END) {
            PE_SV hit = db->value(2); // normally a fixed value
            if (db->subType() > PE_DB_DAMAGED_MOD_TAKEN_HFACTOR_START && PE_DB_DAMAGED_MOD_TAKEN_HFACTOR_END) {
               hit = static_cast<PE_SV>(db->value(2)*m_maxHealth[side ^ 1][m_activePet[side]] / maxEval);
            }
            else if (db->subType() > PE_DB_DAMAGED_MOD_TAKEN_DFACTOR_START && PE_DB_DAMAGED_MOD_TAKEN_DFACTOR_END) {
               hit = static_cast<PE_SV>(db->value(2)*strike / maxEval);
            }
            damageBack += doHit(side ^ 1, m_activePet[side ^ 1], static_cast<PE_FAMILY>(db->value(1)), hit, PE_NO_ACTIVE, false, false, false, false);
         }
      }
   }

   return damageBack;
}

PE_SV State::healingBackTaken(const PE_SIDE side, const PE_PET_NUMBER pet) {
   PE_SV healingBack = 0;

   if (!m_debuff[PE_DB_DAMAGED][side][pet].empty()) {
      Tvector *dbl = &m_debuff[PE_DB_DAMAGED][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         PE_DB_SUBTYPE subType = db->subType();
         if (m_activePet[side] == PE_NO_ACTIVE) {
            dbl->end(track);
            break;
         }
         if (db->subType() > PE_DB_DAMAGED_MOD_TAKEN_HEALING_BACK_START &&
            db->subType() < PE_DB_DAMAGED_MOD_TAKEN_HEALING_BACK_END) {
            healingBack += this->doHeal(side ^ 1, m_activePet[side ^ 1], db->value(0), false);
         }
      }
   }

   return healingBack;
}

PE_SV State::damageBackGiven(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_PET_NUMBER source) {
   PE_SV damageBack = 0;

   if (!m_debuff[PE_DB_DAMAGED][side][source].empty()) {
      Tvector *dbl = &m_debuff[PE_DB_DAMAGED][side][source];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         PE_DB_SUBTYPE subType = db->subType();
         if (m_activePet[side] == PE_NO_ACTIVE) {
            dbl->end(track);
            break;
         }
         if (db->subType() > PE_DB_DAMAGED_MOD_GIVEN_ADD_START &&
            db->subType() < PE_DB_DAMAGED_MOD_GIVEN_ADD_END) {
            damageBack += doHit(side, source, static_cast<PE_FAMILY>(db->value(1)), db->value(2), PE_NO_ACTIVE, false, false, false, false);
            if (db->subType() > PE_DB_DAMAGED_MOD_GIVEN_DAMAGEBACK_HIT) {
               if (!db->dec(3)) {
                  this->removeDebuff(side, source, PE_DB_DAMAGED, db->subType(), track);
               }
            }
         }
      }
   }

   return damageBack;
}

PE_SV State::checkHitActive(const PE_SIDE side, const PE_FAMILY family, const PE_SV rawHit, const PE_PET_NUMBER source, const bool isCrit, const bool isPrimary, const bool isDot, const bool overRide) {
   PE_PET_NUMBER pet = m_activePet[side];
   if (pet == PE_NO_ACTIVE) return 0;
   if (m_health[side][pet] <= 0) return 0;
   PE_PET_NUMBER enemy = m_activePet[side^1];
   bool liveEnemy = enemy != PE_NO_ACTIVE;

   // could add crit possibility to dots and procced effects here

   PE_SV modHita = modHit(side, pet, rawHit);
   if (modHita <= 0) {
      return 0;
   }

   if (!overRide) {
      if (immuneHit(side, pet, isPrimary && !isDot, source, PE_IB_NONE, false)) {
         return 0;
      }
   }

   double factor = 1.0;
   factor *= factorTake(side, pet, false);
   if (liveEnemy && source == enemy) {
      factor *= factorGive(side^1, enemy, false);
   }
   if (isCrit) {
      factor *= (1.0 + PE_CRIT_MOD);
   }
   // standard racial factor
   factor *= (1.0 + familyMod[family][petFamily(side, pet, PE_FP_DAMAGE_TAKEN)]);
   // weather/special racial effects
   if (petFamily(side, pet, PE_FP_GENERAL) == PE_FAMILY_AQUATIC && isDot) factor *= (1.0 + PE_AQUA_DOT);
   if (m_weather == PE_WEATHER_MOONLIGHT && family == PE_FAMILY_MAGIC) factor *= (1.0 + PE_MOONLIGHT_MAGIC);
   if (m_weather == PE_WEATHER_LIGHTNING_STORM && family == PE_FAMILY_MECHANICAL) factor *= (1.0 + PE_LIGHTNING_MECHANICAL);
   if (m_weather == PE_WEATHER_RAIN && family == PE_FAMILY_AQUATIC) factor *= (1.0 + PE_RAIN_AQUA);
   if (!isDot && liveEnemy) { // otherwise activepet might be invalid
      if (petFamily(side^1, enemy, PE_FP_GENERAL) == PE_FAMILY_BEAST && m_health[side ^ 1][enemy] < PE_BEAST_THRESHOLD*m_maxHealth[side ^ 1][enemy]) {
         factor *= (1.0 + PE_BEAST_DAMAGE);
      }
   }

   modHita = static_cast<PE_SV>(modHita*factor);

   return modHita;
}

PE_SV State::doHit(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_FAMILY family, const PE_SV rawHit, const PE_PET_NUMBER source, const bool isCrit, const bool isPrimary, const bool isDot, const bool overRide) {
   PE_PET_NUMBER enemy = m_activePet[side^1];
   bool liveEnemy = enemy != PE_NO_ACTIVE;

   if (m_health[side][pet] <= 0) return 0; // already dead

   // could add crit possibility to dots and procced effects here

   PE_SV modHita = modHit(side, pet, rawHit);
   if (modHita <= 0) {
      return 0;
   }
   if (!overRide) {
      if (immuneHit(side, pet, isPrimary && !isDot, source, PE_IB_NONE, true)) {
         return 0;
      }
   }

   double factor = 1.0;
   factor *= factorTake(side, pet, true);
   if (liveEnemy && source == enemy) {
      factor *= factorGive(side^1, enemy, true);
   }
   if (isCrit) {
      factor *= (1.0 + PE_CRIT_MOD);
   }
   // standard racial factor
   factor *= (1.0 + familyMod[family][petFamily(side, pet, PE_FP_DAMAGE_TAKEN)]);

   // weather/special racial effects
   if (petFamily(side, pet, PE_FP_GENERAL) == PE_FAMILY_AQUATIC && isDot) factor *= (1.0 + PE_AQUA_DOT);
   if (m_weather == PE_WEATHER_MOONLIGHT && family == PE_FAMILY_MAGIC) factor *= (1.0 + PE_MOONLIGHT_MAGIC);
   if (m_weather == PE_WEATHER_LIGHTNING_STORM && family == PE_FAMILY_MECHANICAL) factor *= (1.0 + PE_LIGHTNING_MECHANICAL);
   if (m_weather == PE_WEATHER_RAIN && family == PE_FAMILY_AQUATIC) factor *= (1.0 + PE_RAIN_AQUA);
   if (!isDot && liveEnemy) { // otherwise activepet might be invalid
      if (petFamily(side^1, enemy, PE_FP_GENERAL) == PE_FAMILY_BEAST && m_health[side ^ 1][enemy] < PE_BEAST_THRESHOLD*m_maxHealth[side ^ 1][enemy]) {
         factor *= (1.0 + PE_BEAST_DAMAGE);
      }
   }

   modHita = static_cast<PE_SV>(modHita*factor);

   PE_SV preHit = m_health[side][pet];
   PE_SV strike = reduceHealth(side, pet, modHita);

   if (source != PE_NO_ACTIVE) {
      m_didDamage[side^1][source] = true;
      // dragonkin racial doesnt work unless the dragon did the damage

      if (petFamily(side^1, source, PE_FP_GENERAL) == PE_FAMILY_DRAGONKIN) {
         if (m_health[side][pet] < PE_DRAGONKIN_THRESHOLD*m_maxHealth[side][pet] && 
            preHit >= PE_DRAGONKIN_THRESHOLD*m_maxHealth[side][pet]) {
            if (!isBuffed(side^1, PE_DB_DAMAGED, PE_DB_DAMAGED_DRAGONKIN)) {
               vector<PE_SV> debuffValues;

               debuffValues.push_back(static_cast<PE_SV>(PE_DRAGONKIN_DAMAGE*maxEval));
               pushDebuff(side^1, source, PE_DB_DAMAGED, Debuff(PE_DB_DAMAGED_DRAGONKIN, PE_ONE_ROUND, false, side^1, source, PE_IB_NONE, debuffValues));
            }
         }
      }
   }

   // procced effects
   if (isPrimary) { // otherwise activepet might be invalid

      PE_SV damageBack = 0; // both are damage back to giving pet
      if (!isDot && source != PE_NO_ACTIVE) {
         //  proc debuff on pet giving damage:
         damageBackGiven(side^1, pet, source);
         // proc buff on pet taking damage:
         damageBackTaken(side, pet, strike);
         healingBackTaken(side, pet);
      }

      PE_SV damageForward = 0; // procced effects on pet already receiving damage
      if (m_weather == PE_WEATHER_LIGHTNING_STORM && petFamily(side, pet, PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
         if (m_health[side][pet] > 0) {
            damageForward += doHit(side, pet, PE_FAMILY_MECHANICAL, m_weatherValue, PE_NO_ACTIVE, false, false, isDot, false);
         }
      }
   }

   return modHita;
}

PE_SV State::doHitActive(const PE_SIDE side, const PE_FAMILY family, const PE_SV rawHit, const PE_PET_NUMBER source, const bool isCrit, const bool isPrimary, const bool isDot, const bool overRide) {
   PE_PET_NUMBER pet = m_activePet[side];

   if (m_activePet[side] == PE_NO_ACTIVE) return 0;
   return doHit(side, m_activePet[side], family, rawHit, source, isCrit, isPrimary, isDot, overRide);
}

double State::factorHeal(const PE_SIDE side, const PE_PET_NUMBER pet) {
   // check the increase/reduction percentage to damage on the active pet
   double healFactor = 1.0;

   if (m_weather == PE_WEATHER_DARKNESS) {
      if (petFamily(side, pet, PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
         healFactor *= PE_DARKNESS_HEALING;
      }
   }
   else if (m_weather == PE_WEATHER_SUNLIGHT) {
      if (petFamily(side, pet, PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
         healFactor *= PE_SUNLIGHT_HEALING;
      }
   }
   else if (m_weather == PE_WEATHER_MOONLIGHT) {
      if (petFamily(side, pet, PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
         healFactor *= PE_MOONLIGHT_HEALING;
      }
   }
   if (!m_debuff[PE_DB_DAMAGED][side][pet].empty()) {
      Tvector *dbl = &m_debuff[PE_DB_DAMAGED][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() > PE_DB_DAMAGED_MOD_HEALING_TAKEN_FACTOR_START &&
            db->subType() < PE_DB_DAMAGED_MOD_HEALING_TAKEN_FACTOR_END) {
            healFactor *= static_cast<double>(db->value(0)) / maxEval;
         }
      }
   }

   return healFactor;
}

PE_SV State::doHeal(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_SV rawHeal, const bool isPrimary) {
   PE_SV modHeala = rawHeal;

   if (m_health[side][pet] <= 0) return 0;
   double factor = 1.0;
   factor *= factorHeal(side, pet);
   modHeala = static_cast<PE_SV>(modHeala*factor);
   if (modHeala > 0) {
      increaseHealth(side, pet, modHeala);
      return modHeala;
   }
   else {
      return 0;
   }
}

PE_SV State::doHealActive(const PE_SIDE side, const PE_SV rawHeal, const bool isPrimary) {

   if (m_activePet[side] == PE_NO_ACTIVE) return 0;
   return doHeal(side, m_activePet[side], rawHeal, isPrimary);
}


PE_ERROR State::die(const PE_SIDE side, const PE_PET_NUMBER pet) {

   m_health[side][pet] = 0;
   if (pet == m_activePet[side]) {
      m_activePet[side] = PE_NO_ACTIVE;
   }
   if (!m_gameOver) {
      int apet;
      for (apet = 0; apet < PE_TEAM_SIZE; apet++) {
         if (m_health[side][apet] > 0) break;
      }
      if (apet == PE_TEAM_SIZE) {
         m_gameOver = true;
         m_winner = side^1;
      }
   }

   return PE_OK;
}

PE_ERROR State::couldDie(const PE_SIDE side, const PE_PET_NUMBER pet) {
   bool survives = false;
   vector<PE_SV> debuffValues;
   Tvector *dbl;

   if (!m_debuff[PE_DB_IMMUNITY][side][pet].empty()) {
      dbl = &m_debuff[PE_DB_IMMUNITY][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() == PE_DB_IMMUNE_SURVIVAL || db->subType() == PE_DB_IMMUNE_UNDEAD) {
            m_health[side][pet] = 1;
            survives = true;
         }
         else if (db->subType() == PE_DB_IMMUNE_DARK_REBIRTH) {
            m_health[side][pet] = m_maxHealth[side][pet];
            this->pushDebuff(side, pet, PE_DB_OVERTIME, Debuff(PE_DB_OVERTIME_DARK_REBIRTH, PE_FIVE_ROUNDS, false, side, PE_NO_ACTIVE, PE_IB_NONE, debuffValues));
            survives = true;
         }
      }
   }
   if (!m_debuff[PE_DB_OVERTIME][side][pet].empty()) {
      dbl = &m_debuff[PE_DB_OVERTIME][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() == PE_DB_OVERTIME_DARK_REBIRTH) {
            this->removeDebuff(side, pet, PE_DB_IMMUNITY, PE_DB_IMMUNE_DARK_REBIRTH, track);
         }
      }
   }

   if (!survives) {
      if (petFamily(side, pet, PE_FP_GENERAL) == PE_FAMILY_UNDEAD) {
         m_debuff[PE_DB_IMMUNITY][side][pet].pushBuff(Debuff(PE_DB_IMMUNE_UNDEAD, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
         m_health[side][pet] = 1;
      }
      else if (petFamily(side, pet, PE_FP_GENERAL) == PE_FAMILY_MECHANICAL) {
         bool alreadyRessed = false;
         
         if (!m_debuff[PE_DB_DAMAGED][side][pet].empty()) {
            dbl = &m_debuff[PE_DB_DAMAGED][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->subType() == PE_DB_DAMAGED_MECHANICAL_RES) {
                  alreadyRessed = true;
               }
            }
         }
         if (alreadyRessed) {
            die(side, pet);
         }
         else {
            m_debuff[PE_DB_DAMAGED][side][pet].pushBuff(Debuff(PE_DB_DAMAGED_MECHANICAL_RES, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
            m_health[side][pet] = static_cast<PE_SV>(m_maxHealth[side][pet]*PE_MECH_SURVIVE);
         }
      }
      else {
         die(side, pet);
      }
   }

   return PE_OK;
}

PE_SV State::reduceHealth(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_SV hit) {
   PE_SV modHit = hit;

   if (petFamily(side, pet, PE_FP_GENERAL) == PE_FAMILY_MAGIC) {
      if (modHit > static_cast<PE_SV>(PE_MAGIC_MAXHIT*m_maxHealth[side][pet])) {
         modHit = static_cast<PE_SV>(PE_MAGIC_MAXHIT*m_maxHealth[side][pet]);
      }
   }

   m_hit[side][pet].push_back(modHit);

#if PE_VERBOSE > PE_VERBOSE_REQ
   if (m_verbosity > PE_VERBOSE_REQ) {
      printf("  reducing   side %d pet %d by %4d health\n", side, pet, modHit);
   }
#endif

   // check whether pet is immune to go below a certain level (survival, undead), or procs the mech racial
   if (hit >= m_health[side][pet]) {
      couldDie(side, pet);
   }
   else {
      m_health[side][pet] -= modHit;
      m_damaged[side][pet] = true;
   }

   return hit;
}

PE_SV State::directReduceHealth(const PE_SIDE side, const PE_SV hit) {

   if (m_activePet[side] == PE_NO_ACTIVE) return 0;
   return reduceHealth(side, m_activePet[side], hit);
}

PE_SV State::increaseHealth(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_SV heal) {
   PE_SV healingDone = heal;

#if PE_VERBOSE > PE_VERBOSE_REQ
   if (m_verbosity > PE_VERBOSE_REQ) {
      printf("  increasing side %d pet %d by %4d health\n", side, pet, heal);
   }
#endif

   if (healingDone > m_maxHealth[side][pet] - m_health[side][pet]) {
      healingDone = m_maxHealth[side][pet] - m_health[side][pet];
   }
   m_health[side][pet] += healingDone;

   return healingDone;
}

PE_SV State::directIncreaseHealth(const PE_SIDE side, const PE_SV hit) {

   if (m_activePet[side] == PE_NO_ACTIVE) return 0;
   return increaseHealth(side, m_activePet[side], hit);
}

// this makes a copy, so it will not change the original
bool State::pushDebuff(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_DB_TYPE type, Debuff &debuff) {
   bool alreadyDebuffed = false;
   int debuffIndex = -1;

   if (m_health[side][pet] <= 0) {
      return false;
   }
   if (debuff.sourceSide() != side) { // attack debuff could be blocked
      if (immuneHit(side, pet, true, debuff.sourcePet(), debuff.immuneBreak(), true)) {
         return false;
      }
   }
   if (type == PE_DB_MOBILITY) { // resilience and critter check
      if (debuff.subType() < PE_DB_MOBILITY_MOVE_CONTROL_END) {
         if (petFamily(side, pet, PE_FP_GENERAL) == PE_FAMILY_CRITTER) { // critter
            return false;
         }
         if (!m_debuff[PE_DB_MOBILITY][side][pet].empty()) {
            Tvector *dbl = &m_debuff[PE_DB_MOBILITY][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->subType() == PE_DB_MOBILITY_RESILIENT) {
                  dbl->end(track);
                  return false;
               }
            }
         }
      }
      if (debuff.subType() < PE_DB_MOBILITY_STUNS_ROOTS_END) {
         if (petFamily(side, pet, PE_FP_GENERAL) == PE_FAMILY_CRITTER) { // critter
            return false;
         }
         if (m_weather == PE_WEATHER_ARCANE) { // arcane winds
            return false;
         }
      }
      if (debuff.subType() < PE_DB_MOBILITY_MOVE_CONTROL_END) { // resilience
         if (!m_debuff[PE_DB_MOBILITY][side][pet].empty()) {
            Tvector *dbl = &m_debuff[PE_DB_MOBILITY][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->subType() == PE_DB_MOBILITY_RESILIENT) {
                  dbl->end(track);
                  return false;
               }
            }
         }
      }
   }
   if (!m_debuff[type][side][pet].empty()) {
      Tvector *dbl = &m_debuff[type][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() == debuff.subType()) {
            alreadyDebuffed = true;
            debuffIndex = dbl->onNum(track);
         }
      }
   }
   if (m_weather == PE_WEATHER_RAIN) { // cleansing rain
      if ((type == PE_DB_OVERTIME && debuff.subType() < PE_DB_OVERTIME_DAMAGE_END) || debuff.subType() == PE_DB_DAMAGED_ACIDIC_GOO ||
         type == PE_DB_DAMAGED_CORROSION) {
         debuff.tick();
      }
   }
   if (!alreadyDebuffed) {
#if PE_VERBOSE > PE_VERBOSE_REQ
      if (m_verbosity > PE_VERBOSE_REQ) {
         printf("  buffing    side %d pet %d type %d subtype %2d tracks %d\n", side, pet, type, debuff.subType(), m_debuff[type][side][pet].tracks());
      }
#endif
      m_debuff[type][side][pet].pushBuff(debuff);
      /*if (type == PE_DB_MOBILITY && debuff.subType() < PE_DB_MOBILITY_MOVE_CONTROL_END) {
         m_multiRound[side] = PE_NO_ROUNDS; // interrupt multi-round move
      }*/ // this no longer seems to be the case
      if (debuff.subType() == PE_DB_OVERTIME_HEALTHY) {
         m_maxHealth[side][pet] += 5*PE_MAX_LEVEL;
      }
      if (type == PE_DB_IMMUNITY) {
         if (debuff.subType() == PE_DB_IMMUNE_UNDERGROUND || debuff.subType() == PE_DB_IMMUNE_SUBMERGED) {
            vector<int> debuffValues;
            this->pushDebuff(side, pet, PE_DB_IMMUNITY, Debuff(PE_DB_IMMUNE_BREAK_UNDERGROUND, debuff.duration(), false, side, pet, PE_IB_NONE, debuffValues));
         }
      }
   }
   else {
      m_debuff[type][side][pet][debuffIndex]->refresh(debuff.duration());
      if (debuff.subType() == PE_DB_MOBILITY_SPEED_BOOST) {
         m_debuff[type][side][pet][debuffIndex]->add(0, debuff.value(0));
      }
      else if (debuff.subType() == PE_DB_OVERTIME_FROSTBITE) {
         //if (m_debuff[type][side][pet][debuffIndex]->value(2) < 4) {
            m_debuff[type][side][pet][debuffIndex]->add(2, 1);
         //}
      }
#if PE_VERBOSE > PE_VERBOSE_REQ
      if (m_verbosity > PE_VERBOSE_REQ) {
         printf("  update     side %d pet %d type %d subtype %2d tracks %d\n", side, pet, type, debuff.subType(), m_debuff[type][side][pet].tracks());
      }
#endif
      // also copy the values?
   }

   return true;
}

// this makes a copy, so it will not change the original
bool State::pushDebuffActive(const PE_SIDE side, const PE_DB_TYPE type, Debuff &debuff) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;
   return pushDebuff(side, m_activePet[side], type, debuff);
}

PE_ERROR State::removeDebuff(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_DB_TYPE type, const PE_DB_SUBTYPE subType, const int track) {
   bool removed = false;

   Tvector *dbl = &m_debuff[type][side][pet];
   for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
      Debuff *db = dbl->on(track);
      if (db->subType() == subType) {
#if PE_VERBOSE > PE_VERBOSE_REQ
         if (m_verbosity > PE_VERBOSE_REQ) {
            printf("  removing   side %d pet %d type %d subtype %2d\n", side, pet, type, subType);
         }
#endif
		   if (subType == PE_DB_OVERTIME_APOCALYPSE) {
			   if (!((m_genus[side][pet] == PE_GENUS_INSECT) || this->isBuffed(side, PE_DB_IMMUNITY, PE_DB_IMMUNE_SURVIVAL))) {
				   this->reduceHealth(side, pet, m_health[side][pet]);
			   }
		   }
         else if (subType == PE_DB_OVERTIME_HAUNT) {
            m_health[side ^ 1][db->sourcePet()] = db->value(2);
         }
         if (dbl->viable(track)) {
            dbl->erase(track);
         }
         removed = true;
         dbl->end(track);
         break;
      }
   }
   if (removed) {
      if (type == PE_DB_IMMUNITY) {
         if (subType == PE_DB_IMMUNE_UNDERGROUND || subType == PE_DB_IMMUNE_SUBMERGED) {
            this->removeDebuff(side, pet, PE_DB_IMMUNITY, PE_DB_IMMUNE_BREAK_UNDERGROUND, PE_NO_TRACK);
         }
      }
      if (type == PE_DB_OVERTIME) {
         if (subType == PE_DB_OVERTIME_MINEFIELD) {
            this->removeDebuff(side, pet, PE_DB_MOBILITY, PE_DB_MOBILITY_MINEFIELD, PE_NO_TRACK);
         }
         else if (subType == PE_DB_OVERTIME_BOOBY_TRAPPED_PRESENTS) {
            this->removeDebuff(side, pet, PE_DB_MOBILITY, PE_DB_MOBILITY_BOOBY_TRAPPED_PRESENTS, PE_NO_TRACK);
         }
      }
      if (type == PE_DB_MOBILITY) {
         if (subType == PE_DB_MOBILITY_MINEFIELD) {
            this->removeDebuff(side, pet, PE_DB_OVERTIME, PE_DB_OVERTIME_MINEFIELD, PE_NO_TRACK);
         }
         else if (subType == PE_DB_MOBILITY_BOOBY_TRAPPED_PRESENTS) {
            this->removeDebuff(side, pet, PE_DB_OVERTIME, PE_DB_OVERTIME_BOOBY_TRAPPED_PRESENTS, PE_NO_TRACK);
         }
      }
   }

   return PE_OK;
}

PE_ERROR State::removeDebuffActive(const PE_SIDE side, const PE_DB_TYPE type, const PE_DB_SUBTYPE subType, const int track) {

   if (m_activePet[side] == PE_NO_ACTIVE) return PE_OK;
   return removeDebuff(side, m_activePet[side], type, subType, track);
}


PE_ERROR State::removeDebuffs(const PE_SIDE side, const PE_PET_NUMBER pet) {
   Tvector *dbl;
   Debuff *db;
   int track;

   //if (m_health[side][pet] <= 0) return PE_OK;
   if (!m_debuff[PE_DB_OVERTIME][side][pet].empty()) {
      dbl = &m_debuff[PE_DB_OVERTIME][side][pet];
      for (track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         db = dbl->on(track);
         if (db->subType() < PE_DB_OVERTIME_OBJECTS_START || db->subType() > PE_DB_OVERTIME_OBJECTS_END) {
            this->removeDebuff(side, pet, PE_DB_OVERTIME, db->subType(), track);
         }
      }
   }
   if (!m_debuff[PE_DB_DAMAGED][side][pet].empty()) {
      dbl = &m_debuff[PE_DB_DAMAGED][side][pet];
      for (track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         db = dbl->on(track);
         if (db->subType() != PE_DB_DAMAGED_MECHANICAL_RES) {
            this->removeDebuff(side, pet, PE_DB_DAMAGED, db->subType(), track);
         }
      }
   }
   if (!m_debuff[PE_DB_MOBILITY][side][pet].empty()) {
      dbl = &m_debuff[PE_DB_MOBILITY][side][pet];
      for (track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         db = dbl->on(track);
         if (db->subType() < PE_DB_MOBILITY_SWITCH_START || db->subType() > PE_DB_MOBILITY_SWITCH_END) {
            this->removeDebuff(side, pet, PE_DB_MOBILITY, db->subType(), track);
         }
      }
   }
   if (!m_debuff[PE_DB_IMMUNITY][side][pet].empty()) {
      dbl = &m_debuff[PE_DB_IMMUNITY][side][pet];
      for (track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         db = dbl->on(track);
         if (db->subType() != PE_DB_IMMUNE_UNDEAD) {
            this->removeDebuff(side, pet, PE_DB_IMMUNITY, db->subType(), track);
         }
      }
   }

   return PE_OK;
}

PE_ERROR State::removeDebuffsActive(const PE_SIDE side) {

	if (m_activePet[side] == PE_NO_ACTIVE) return PE_OK;
	return removeDebuffs(side, m_activePet[side]);
}

PE_ERROR State::removeObjects() {

   for (PE_SIDE side = 0; side < PE_SIDES; side++) {
      for (PE_PET_NUMBER pet = 0; pet < PE_TEAM_SIZE; pet++) {

         if (!m_debuff[PE_DB_OVERTIME][side][pet].empty()) {
            Tvector *dbl = &m_debuff[PE_DB_OVERTIME][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->subType() > PE_DB_OVERTIME_OBJECTS_START &&
                  db->subType() < PE_DB_OVERTIME_OBJECTS_END) {
                  this->removeDebuff(side, pet, PE_DB_OVERTIME, db->subType(), track);
               }
            }
         }
      }
   }

   return PE_OK;
}

int State::getDuration(const PE_SIDE side, const PE_DB_TYPE type, const PE_DB_SUBTYPE subType) {

   if (m_activePet[side] == PE_NO_ACTIVE) return -1;

   vector<Debuff> *dbl = m_debuff[type][side][m_activePet[side]].list();
   for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
      if ((*dbl)[debuff].subType() == subType) {
         return (*dbl)[debuff].duration();
      }
   }

   return -1;
}

bool State::isBuffed(const PE_SIDE side, const PE_DB_TYPE type, const PE_DB_SUBTYPE subType) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;

   if (!m_debuff[type][side][m_activePet[side]].empty()) {
      vector<Debuff> *dbl = m_debuff[type][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() == subType) {
            return true;
         }
      }
   }

   return false;
}

bool State::isBlinded(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;
   if (m_weather == PE_WEATHER_DARKNESS) {
      if (petFamily(side, m_activePet[side], PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
         return true;
      }
   }
   if (!m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].empty()) {
      vector<Debuff> *dbl = m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() > PE_DB_MOBILITY_BLINDING_START && (*dbl)[debuff].subType() < PE_DB_MOBILITY_BLINDING_END) {
            return true;
         }
      }
   }
   /*for (int di = 0; di < m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].size(); di++) {
      if (m_debuff[PE_DB_MOBILITY][side][m_activePet[side]][di].subType() == PE_DB_MOBILITY_SLIPPERY_ICE) {
         return true;
      }
   }*/

   return false;
}

bool State::isBleeding(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;

   if (!m_debuff[PE_DB_OVERTIME][side][m_activePet[side]].empty()) {
      vector<Debuff> *dbl = m_debuff[PE_DB_OVERTIME][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() == PE_DB_OVERTIME_BLEED) {
            return true;
         }
      }
   }

   return false;
}

bool State::isStunned(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;

   if (!m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].empty()) {
      vector<Debuff> *dbl = m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() == PE_DB_MOBILITY_STUNNED) {
            return true;
         }
      }
   }

   return false;
}


bool State::isBurning(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;
   if (m_weather == PE_WEATHER_SCORCHED_EARTH) {
      if (petFamily(side, m_activePet[side], PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
         return true;
      }
   }
   if (!m_debuff[PE_DB_OVERTIME][side][m_activePet[side]].empty()) {
      vector<Debuff> *dbl = m_debuff[PE_DB_OVERTIME][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() > PE_DB_OVERTIME_BURNING_START &&
            (*dbl)[debuff].subType() < PE_DB_OVERTIME_BURNING_END) {
            return true;
         }
      }
   }

   return false;
}

bool State::isChilled(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;
   if (m_weather == PE_WEATHER_BLIZZARD) {
      if (petFamily(side, m_activePet[side], PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
         return true;
      }
   }
   if (!m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].empty()) {
      vector<Debuff> *dbl = m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() == PE_DB_MOBILITY_SLIPPERY_ICE) {
            return true;
         }
      }
   }

   return false;
}

bool State::isImmobile(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;
   if (!m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].empty()) {
      vector<Debuff> *dbl = m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() < PE_DB_MOBILITY_MOVE_CONTROL_END) {
            return true;
         }
      }
   }

   return false;
}

bool State::isPoisoned(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;
   vector<Debuff> *dbl;
   if (!m_debuff[PE_DB_OVERTIME][side][m_activePet[side]].empty()) {
      dbl = m_debuff[PE_DB_OVERTIME][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() > PE_DB_OVERTIME_POISONED_START &&
            (*dbl)[debuff].subType() < PE_DB_OVERTIME_POISONED_END) {
            return true;
         }
      }
   }
   if (!m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].empty()) {
      dbl = m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() == PE_DB_MOBILITY_BLINDING_POISON) {
            return true;
         }
      }
   }

   return false;
}

bool State::isRooted(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;
   if (!m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].empty()) {
      vector<Debuff> *dbl = m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if (((*dbl)[debuff].subType() > PE_DB_MOBILITY_ROOTS_START &&
            (*dbl)[debuff].subType() < PE_DB_MOBILITY_ROOTS_END) ||
            ((*dbl)[debuff].subType() > PE_DB_MOBILITY_SELF_ROOT_START &&
            (*dbl)[debuff].subType() < PE_DB_MOBILITY_SELF_ROOT_END)) {
            return true;
         }
      }
   }

   return false;
}

bool State::isWebbed(const PE_SIDE side) {

   if (m_activePet[side] == PE_NO_ACTIVE) return false;
   vector<Debuff> *dbl;
   if (!m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].empty()) {
      dbl = m_debuff[PE_DB_MOBILITY][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() == PE_DB_MOBILITY_WEBBED) {
            return true;
         }
      }
   }
   if (!m_debuff[PE_DB_DAMAGED][side][m_activePet[side]].empty()) {
      dbl = m_debuff[PE_DB_DAMAGED][side][m_activePet[side]].list();
      for (uint32_t debuff = 0; debuff < dbl->size(); debuff++) {
         if ((*dbl)[debuff].subType() == PE_DB_DAMAGED_BRITTLE_WEBBING) {
            return true;
         }
      }
   }

   return false;
}

PE_ERROR State::setWeather(const PE_WEATHER weather, const PE_AV value) {

   if (weather == PE_WEATHER_SUNLIGHT && m_weather != PE_WEATHER_SUNLIGHT) {
      for (PE_SIDE side = 0; side < PE_SIDES; side++) {
         for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
            m_maxHealth[side][pet] = static_cast<int>(m_maxHealth[side][pet]*PE_SUNLIGHT_HEALTH);
         }
      }
   }
   m_weather = weather;
   if (weather == PE_WEATHER_DARKNESS) {
      m_weatherCooldown = PE_FIVE_ROUNDS;
   }
   else {
      m_weatherCooldown = PE_NINE_ROUNDS;
   }
   m_weatherValue = value;

   return PE_OK;
}

int State::consecutiveCurrent(const PE_SIDE side) {
   PE_ABILITY ability;

   if (m_moveStack[side].size() <= 1) return static_cast<int>(m_moveStack[side].size());
   ability = m_moveStack[side].back();
   std::vector<PE_ABILITY>::reverse_iterator rit;
   int count = 0;
   for (rit = m_moveStack[side].rbegin(); rit != m_moveStack[side].rend(); rit++) {
      if (*rit != ability) break;
      count++;
   }

   return count;
}

PE_ERROR State::incRound() {

   for (int side = 0; side < PE_SIDES; side++) {
      m_multiRound[side]--;
      for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
         m_lastUsed[side][pet][PE_SLOT1]++;
         m_lastUsed[side][pet][PE_SLOT2]++;
         m_lastUsed[side][pet][PE_SLOT3]++;
      
      }
   }

   return PE_OK;
}

PE_ERROR State::clearDebuffsActive(const PE_SIDE side) {
   PE_PET_NUMBER pet = m_activePet[side];

   if (pet == PE_NO_ACTIVE) return PE_OK;
   for (PE_DB_TYPE type = PE_DB_OVERTIME; type < PE_DB_TYPES; type = static_cast<PE_DB_TYPE>(static_cast<int>(type) + 1)) {
      if (m_debuff[type][side][pet].empty()) continue;

      Tvector *dbl = &m_debuff[type][side][pet];
      for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
         Debuff *db = dbl->on(track);
         if (db->subType() != PE_DB_OVERTIME_APOCALYPSE && 
            db->subType() != PE_DB_MOBILITY_RESILIENT) {
#if PE_VERBOSE > PE_VERBOSE_REQ
   printf("  wiped                     type %d subtype %2d\n", type, db->subType());
#endif
            this->removeDebuff(side, pet, type, db->subType(), track);
         }
      }
   }

   return PE_OK;
}

PE_ERROR State::clearExpired(const PE_SIDE side, const PE_PET_NUMBER pet, const PE_DB_TYPE type) {

   if (m_debuff[type][side][pet].empty()) return PE_OK;

   Tvector *dbl = &m_debuff[type][side][pet];
   for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
      Debuff *db = dbl->on(track);
      db->tick();
      if (db->duration() < PE_ZERO_ROUND) {
#if PE_VERBOSE > PE_VERBOSE_REQ
         if (m_verbosity > PE_VERBOSE_REQ) {
            printf("  expired                 type %d subtype %2d\n", type, db->subType());
         }
#endif
         this->removeDebuff(side, pet, type, db->subType(), track);
      }
   }

   return PE_OK;
}

PE_ERROR State::endBranching(Turn &turn) {
   const int maxEvents = 8; // 256 branching
   vector<double> actionProb;

   for (PE_SIDE side = 0; side < PE_SIDES; side++) {
      for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
         if (!m_debuff[PE_DB_OVERTIME][side][pet].empty()) {
            Tvector *dbl = &m_debuff[PE_DB_OVERTIME][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->subType() == PE_DB_OVERTIME_CYCLONE) {
                  actionProb.push_back(7 * fiveProb);
               }
               else if (db->subType() == PE_DB_OVERTIME_VOLCANO) {
                  if (db->duration() <= PE_ONE_ROUND) {
                     actionProb.push_back(5 * fiveProb);
                  }
               }
            }
         }
      }
   }
   // random pet switching after fade
   for (PE_SIDE side = 0; side < PE_SIDES; side++) {
      if (m_activePet[side] == PE_NO_ACTIVE) continue;
      if (m_health[side][m_activePet[side]] > 0) {
         int pet = m_activePet[side];
         if (!m_debuff[PE_DB_IMMUNITY][side][pet].empty()) {
            Tvector *dbl = &m_debuff[PE_DB_IMMUNITY][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->subType() == PE_DB_IMMUNE_FADE) {
                  int opets = 0;
                  for (int pet2 = 0; pet2 < PE_TEAM_SIZE; pet2++) {
                     if (pet2 != pet && m_health[side][pet2] > 0) {
                        opets++;
                     }
                  }
                  if (opets == 2) {
                     actionProb.push_back(fiftyProb); // fifty prob
                  }
               }
            }
         }
      }
   }

   if (actionProb.size() > maxEvents) {
      actionProb.erase(actionProb.begin(), actionProb.begin() + maxEvents);
   }
   for (uint32_t outcome = 0; outcome < (one<<actionProb.size()); outcome++) {
      double netProb = 1.0;
      for (uint32_t action = 0; action < actionProb.size(); action++) {
         if ((one<<action)&outcome) {
            netProb *= actionProb[action];
         }
         else {
            netProb *= (1 - actionProb[action]);
         }
      }
      turn.push(static_cast<PE_MOVETYPE>(outcome), netProb);
   }

   return PE_OK;
}


PE_ERROR State::wrapUp(Turn &turn, const uint32_t outcome) {
   vector<PE_SV> debuffValues;
   uint32_t action = 0;

   // cooldowns, multi-round turn
   incRound();

   // check dots and such
   for (int side = 0; side < PE_SIDES; side++)  {
      for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
         if (m_health[side][pet] == 0) continue;
         if (m_debuff[PE_DB_OVERTIME][side][pet].empty()) continue;
         Tvector *dbl = &m_debuff[PE_DB_OVERTIME][side][pet];
         for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
            Debuff *db = dbl->on(track);
            PE_DB_SUBTYPE subType = db->subType();
            if (subType < PE_DB_OVERTIME_DAMAGE_END) { // regular dots are assummed not to crit
               if (subType < PE_DB_OVERTIME_DAMAGE_REGULAR_END) {
                  this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), db->sourcePet(), false, true, true, false);
               }
               else if (subType < PE_DB_OVERTIME_DAMAGE_NOTDOT_END) {
                  this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), db->sourcePet(), false, true, false, false);
               }
               else if (subType < PE_DB_OVERTIME_DAMAGE_CONDITIONAL_END) {
                  if (subType == PE_DB_OVERTIME_CREEPING_FUNGUS) {
                     PE_SV hit = db->value(1);
                     if (m_weather == PE_WEATHER_MOONLIGHT && petFamily(side, pet, PE_FP_GENERAL) != PE_FAMILY_ELEMENTAL) {
                        hit = static_cast<PE_SV>(hit*PE_CREEPING_FUNGUS);
                     }
                     PE_SV damageDone = this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), db->sourcePet(), false, true, true, false);
                  }
               }
               else if (subType < PE_DB_OVERTIME_DAMAGE_PROGRESSIVE_END) {
                  this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), db->sourcePet(), false, true, true, false);
                  if (dbl->viable(track)) db->add(1, db->value(2));
               }
               else if (subType < PE_DB_OVERTIME_DAMAGE_PERCENT_END) {
                  this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), static_cast<PE_SV>(db->value(1)*m_maxHealth[side][pet] / maxEval), db->sourcePet(), false, true, true, false);
                  //reduceHealth(side, pet, db->value(0)*m_maxHealth[side][pet] / maxEval); // no, it's a hit
               }
               else if (subType < PE_DB_OVERTIME_DAMAGE_HEAL_END) {
                  PE_PET_NUMBER source = db->sourcePet();
                  PE_SV todo = db->value(2);
                  PE_SV damageDone = this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), db->sourcePet(), false, true, true, false);
                  if (subType < PE_DB_OVERTIME_DAMAGE_HEAL_FIXED_END) {
                     this->doHeal(side ^ 1, source, todo, true);
                  }
                  else if (damageDone > 0) {
                     this->doHeal(side ^ 1, source, damageDone, true);
                  }
               }
               else if (subType < PE_DB_OVERTIME_DAMAGE_NONACTIVE_END) {
                  for (int pet2 = 0; pet2 < PE_TEAM_SIZE; pet2++) {
                     if (m_health[side][pet] == 0 || pet2 == pet) continue;
                     this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), db->sourcePet(), false, true, true, false);
                  }
               }
               else if (subType < PE_DB_OVERTIME_ACCUMULATING_END) {
                  for (int iter = 0; iter < db->value(2); iter++) {
                     this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), db->sourcePet(), false, true, true, false);
                  }
               }
            }
            else if (subType < PE_DB_OVERTIME_HEAL_END) { // heal over time
               if (subType < PE_DB_OVERTIME_PROGRESSIVE_HEAL_START) {
                  this->doHeal(side, pet, db->value(0), true);
               }
               else if (subType < PE_DB_OVERTIME_WEATHER_HEAL_START) {
                  this->doHeal(side, pet, db->pop(), true);
               }
               else {
                  this->doHeal(side, pet, db->value(0), true);
                  if (m_weather == PE_WEATHER_SUNLIGHT) {
                     this->doHeal(side, pet, db->value(0), true);
                  }
               }
            }
            else if (subType < PE_DB_OVERTIME_OBJECTS_END) {
               if (subType == PE_DB_OVERTIME_CYCLONE) {
                  if (turn.doAction(outcome, action++)) {
                     this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, false);
                  }
               }
               else if (subType == PE_DB_OVERTIME_VOLCANO) {
                  if (db->duration() <= PE_ONE_ROUND) {
                     this->doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, false);
                     if (turn.doAction(outcome, action++)) {
                        this->pushDebuff(side, pet, PE_DB_MOBILITY, Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
                     }
                  }
               }
               else if (subType == PE_DB_OVERTIME_TURRET1 || subType == PE_DB_OVERTIME_TURRET2) {
                  this->doHit(side ^ 1, this->activePet(side ^ 1), static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, false);
                  if (dbl->viable(track)) this->doHit(side ^ 1, this->activePet(side ^ 1), static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, false);
                  if (dbl->viable(track)) this->doHit(side ^ 1, this->activePet(side ^ 1), static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, false);
               }
               else if (subType == PE_DB_OVERTIME_XE321_BOOMBOT) {
                  if (db->duration() == PE_ZERO_ROUND) {
                     this->doHit(side ^ 1, this->activePet(side ^ 1), static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, false);
                  }
               }

            }
            else if (subType < PE_DB_OVERTIME_MISC_END) {
               if (subType == PE_DB_OVERTIME_IMMOLATION) {
                  this->doHitActive(side ^ 1, static_cast<PE_FAMILY>(db->value(0)), db->value(1), pet, false, true, false, false);
               }
               else if (subType == PE_DB_OVERTIME_DARK_REBIRTH &&
                  db->duration() < PE_FIVE_ROUNDS) {
                  this->directReduceHealth(side, static_cast<PE_SV>(m_maxHealth[side][pet] * PE_REBIRTH_DAMAGE));
               }
               else if (subType == PE_DB_OVERTIME_ROT && db->duration() > PE_ZERO_ROUND) {
                  this->pushDebuff(side, pet, PE_DB_DAMAGED, Debuff(PE_DB_DAMAGED_UNDEAD_MARK, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
               }
            }
            else if (db->duration() == PE_ZERO_ROUND) { // timers
               if (subType > PE_DB_OVERTIME_START_BOMBS && subType < PE_DB_OVERTIME_END_BOMBS) {
                  PE_SV damaged = 0;
                  if (subType < PE_DB_OVERTIME_START_STICKYBOMBS || subType > PE_DB_OVERTIME_END_STICKYBOMBS) {
                     damaged += doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, false);
                  }
                  else {
                     damaged += doHit(side, pet, static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, true);
                  }
                  if (subType > PE_DB_OVERTIME_START_STUNBOMBS && subType < PE_DB_OVERTIME_END_STUNBOMBS) {
                     if (damaged) {
                        this->pushDebuff(side, pet, PE_DB_MOBILITY, Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
                     }
                  }
                  else if (subType > PE_DB_OVERTIME_START_ROOTBOMBS && subType < PE_DB_OVERTIME_END_ROOTBOMBS) {
                     if (damaged) {
                        this->pushDebuff(side, pet, PE_DB_MOBILITY, Debuff(PE_DB_MOBILITY_ROOTED, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
                     }
                  }
               }
               else if (subType == PE_DB_OVERTIME_SPORE_SHROOMS) {
                  this->pushDebuff(side, pet, PE_DB_OVERTIME, Debuff(PE_DB_OVERTIME_SPORE_BURST, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, *db->values()));
               }
               else if (subType == PE_DB_OVERTIME_CHEW) {
                  this->doHitActive(side ^ 1, static_cast<PE_FAMILY>(db->value(0)), db->value(1), PE_NO_ACTIVE, false, true, false, false);
               }
               else if (subType == PE_DB_OVERTIME_WISH) {
                  PE_SV healed = this->doHeal(side, pet, m_maxHealth[side][pet] / 2, true);
               }
               else if (subType == PE_DB_OVERTIME_FADING) {
                  // note this goes to the active pet
                  this->pushDebuffActive(side, PE_DB_IMMUNITY, Debuff(PE_DB_IMMUNE_INVISIBLE, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
               }
               else if (subType == PE_DB_OVERTIME_APOCALYPSE) {
                  if (!((m_genus[side][pet] == PE_GENUS_INSECT) || this->isBuffed(side, PE_DB_IMMUNITY, PE_DB_IMMUNE_SURVIVAL))) {
                     this->reduceHealth(side, pet, m_health[side][pet]);
                  }
               }
               else if (subType == PE_DB_OVERTIME_HEALTHY) {
                  m_maxHealth[side][pet] -= 5 * PE_MAX_LEVEL;
                  if (m_maxHealth[side][pet] < m_health[side][pet]) m_health[side][pet] = m_maxHealth[side][pet];
               }
            }
         }
         for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
            Debuff *db = dbl->on(track);
            if (db->subType() == PE_DB_OVERTIME_BLISTERING_COLD) {
               this->pushDebuffActive(side, PE_DB_OVERTIME, Debuff(PE_DB_OVERTIME_FROSTBITE, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, *db->values()));
            }
         }
      }
   }



   // scorched earth
   if (m_weather == PE_WEATHER_SCORCHED_EARTH) {
      for (int side = 0; side < PE_SIDES; side++)  {
         if (m_activePet[side] == PE_NO_ACTIVE) continue;
         if (m_health[side][m_activePet[side]] == 0) continue;
         if (petFamily(side, m_activePet[side], PE_FP_GENERAL) == PE_FAMILY_ELEMENTAL) continue;
         this->doHit(side, m_activePet[side], PE_FAMILY_DRAGONKIN, m_weatherValue, PE_NO_ACTIVE, false, true, true, false);
      }
   }

   // clear all expired dots
   for (int side = 0; side < PE_SIDES; side++) {
      for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
         if (m_health[side][pet] == 0) {
            this->removeDebuffs(side, pet); // clear all buffs
            continue;
         }
         Tvector *dbl;
         // check on dots which depend on lack of action (being damaged, etc) to work
         if (!m_debuff[PE_DB_MOBILITY][side][pet].empty()) {
            dbl = &m_debuff[PE_DB_MOBILITY][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (m_health[side][pet] > 0) {
                  if (db->duration() == PE_ZERO_ROUND) {
                     if (db->subType() == PE_DB_MOBILITY_SOOTHED && !m_damaged[side][pet] && !immuneHit(side, pet, true, PE_NO_ACTIVE, PE_IB_NONE, false)) {
                        this->pushDebuff(side, pet, PE_DB_MOBILITY, Debuff(PE_DB_MOBILITY_ASLEEP, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
                     }
                  }
                  if (dbl->viable(track)) {
                     if (db->subType() == PE_DB_MOBILITY_ASLEEP && m_damaged[side][pet]) {
                        this->removeDebuff(side, pet, PE_DB_MOBILITY, PE_DB_MOBILITY_ASLEEP, track);
                     }
                  }
               }
            }
         }

         // switchy pets
         if (!m_debuff[PE_DB_IMMUNITY][side][pet].empty()) {
            dbl = &m_debuff[PE_DB_IMMUNITY][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (m_health[side][pet] > 0) {
                  if (db->duration() == PE_ZERO_ROUND) {
                     // feign death
                     if (db->subType() == PE_DB_IMMUNE_FEIGN_DEATH || db->subType() == PE_DB_IMMUNE_PORTAL) {
                        int hp = -1;
                        for (PE_PET_NUMBER mp = 0; mp < PE_TEAM_SIZE; mp++) {
                           if (mp != pet) {
                              if (hp < 0) {
                                 if (m_health[side][mp] > 0) hp = mp;
                              }
                              else if (m_health[side][mp] > m_health[side][hp]) hp = mp;
                           }
                        }
                        if (hp >= 0) this->activate(side, hp);
                     }
                     // fade
                     if (db->subType() == PE_DB_IMMUNE_FADE) {
                        vector <int>ppet;
                        for (PE_PET_NUMBER pet2 = 0; pet2 < PE_TEAM_SIZE; pet2++) {
                           if (pet2 != pet && m_health[side][pet2] > 0) ppet.push_back(pet2);
                        }
                        if (ppet.size() == 2) {
                           if (turn.doAction(outcome, action++)) this->activate(side, ppet[0]);
                           else this->activate(side, ppet[1]);
                        }
                        else if (ppet.size() == 1) {
                           this->activate(side, ppet[0]);
                        }
                     }
                  }
               }
            }
         }

         // humanoid racial
         if (m_didDamage[side][pet] && petFamily(side, pet, PE_FP_GENERAL) == PE_FAMILY_HUMANOID) {
            m_health[side][pet] += static_cast<PE_SV>(PE_HUMANOID_REGEN*m_maxHealth[side][pet]);
            if (m_maxHealth[side][pet] < m_health[side][pet]) m_health[side][pet] = m_maxHealth[side][pet];
         }

         // detransform
         if (!m_debuff[PE_DB_OVERTIME][side][pet].empty()) {
            dbl = &m_debuff[PE_DB_OVERTIME][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->subType() == PE_DB_OVERTIME_NATURES_WARD &&
                  db->duration() == PE_ZERO_ROUND) {
                  m_family[side][pet] = m_baseFamily[side][pet];

               }
            }
         }

         // tricky: here we check whether a stun that will affect a pet next round exists
         // if so, push a resilience with one extra round.  as long as stunned + resilience = stunned,
         // this should work
         if (!m_debuff[PE_DB_MOBILITY][side][pet].empty()) {
            dbl = &m_debuff[PE_DB_MOBILITY][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->duration() >= PE_ONE_ROUND) {
                  if (db->subType() < PE_DB_MOBILITY_MOVE_CONTROL_END) {
                     this->pushDebuff(side, pet, PE_DB_MOBILITY, Debuff(PE_DB_MOBILITY_RESILIENT, PE_TWO_ROUNDS + db->duration(), false, side, pet, PE_IB_NONE, debuffValues));
                     m_multiRound[side] = PE_NO_ROUNDS; // interupt multi-round stuff
                  }
               }
               if (dbl->viable(track)) {
                  if (db->subType() == PE_DB_MOBILITY_NEVERMORE) {
                     PE_ABILITY move = this->backMoveStack(side);
                     if (move < PE_ABILITIES) {
                        m_lastUsed[side][pet][move] = PE_NEVERMORE;
                        this->removeDebuff(side, pet, PE_DB_MOBILITY, PE_DB_MOBILITY_NEVERMORE, track);
                     }
                  }
               }
            }
         }
         if (!m_debuff[PE_DB_IMMUNITY][side][pet].empty()) {
            dbl = &m_debuff[PE_DB_IMMUNITY][side][pet];
            for (int track = dbl->initTrack(); dbl->notEnd(track); dbl->next(track)) {
               Debuff *db = dbl->on(track);
               if (db->subType() == PE_DB_IMMUNE_UNDEAD &&
                  db->duration() == PE_ZERO_ROUND) {
                  die(side, pet);
               }
            }
         }

         for (PE_DB_TYPE type = PE_DB_OVERTIME; type < PE_DB_TYPES; type = static_cast<PE_DB_TYPE>(static_cast<int>(type) + 1)) {
            clearExpired(side, pet, type);
         }
      }
   }

   // tick weather
   if (m_weather != PE_NO_WEATHER) {
      m_weatherCooldown--;
      if (m_weatherCooldown < PE_ZERO_ROUND) {
         if (m_weather == PE_WEATHER_SUNLIGHT) {
            for (int side = 0; side < PE_SIDES; side++)  {
               for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
                  m_maxHealth[side][pet] = static_cast<int>(m_maxHealth[side][pet]/PE_SUNLIGHT_HEALTH);
                  if (m_health[side][pet] > m_maxHealth[side][pet]) m_health[side][pet] = m_maxHealth[side][pet];
               }
            }
         }
         m_weather = PE_NO_WEATHER;
      }
   }

   // clear damage flags
   for (int side = 0; side < PE_SIDES; side++)  {
      for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
         m_damaged[side][pet] = false;
         m_didDamage[side][pet] = false;
      }
   }
   m_speedWinner = PE_SIDES;
   m_switchPass = false;

#if PE_VERBOSE > PE_VERBOSE_REQ
   if (m_verbosity > PE_VERBOSE_REQ) {
      printf("End turn.  Weather is %d\r\n", m_weather);
   }
#endif

   return PE_OK;
}
#if PE_VERBOSE > PE_VERBOSE_OFF
   PE_ERROR State::checkNoTracks() {
      for (int side = 0; side < PE_SIDES; side++) {
         for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
             for (PE_DB_TYPE type = PE_DB_OVERTIME; type < PE_DB_TYPES; type = static_cast<PE_DB_TYPE>(static_cast<int>(type) + 1)) {
                m_debuff[type][side][pet].checkNoTracks();
             }
         }
      }

      return PE_OK;
   }
#endif
