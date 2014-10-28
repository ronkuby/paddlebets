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

#include "constants.h"
#include "move.h"
#include "turn.h"
#include "state.h"
#include "debuff.h"

PE_SV speedSimple(const PE_SIDE &side, State &state) {
   PE_SV speed = state.speed(side);

   return speed;
}

PE_SV speedFast(const PE_SIDE &side, State &state) {
   PE_SV speed = static_cast<int32_t>(state.speed(side)*(1.0 + PE_FAST_SPEED));

   return speed;
}

PE_SV speedFastest(const PE_SIDE &side, State &state) {

   return PE_INFINITE;
}

PE_SV speedFastBlizzard(const PE_SIDE &side, State &state) {
   PE_SV speed;

   if (state.weather() == PE_WEATHER_BLIZZARD) {
      speed = static_cast<int32_t>(state.speed(side)*(1.0 + PE_FAST_SPEED));
   }
   else speed = static_cast<int32_t>(state.speed(side));

   return speed;
}

// begin attack branches that add entries

PE_ERROR branchesNull(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;

   currentTurn.push(PE_MT_MISS, multiplier*maxProb);

   return err;
}

PE_ERROR branchesSingleAttack(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double currentCrit = state.modCritActive(move.side(), move.critProb());
   
   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HIT, multiplier*(currentProb - currentCrit));
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit);
      }
      else {
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb);
      }
      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}

PE_ERROR branchesHighLowAttack(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HITHIGH, multiplier*(currentProb - currentCrit)/2);
         currentTurn.push(PE_MT_HITLOW, multiplier*(currentProb - currentCrit)/2);
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit);
      }
      else {
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb);
      }
      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}

PE_ERROR branchesSingleDebuff(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   
   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      currentTurn.push(PE_MT_HIT, multiplier*currentProb);
      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}

// heal or self buff.  can't miss
PE_ERROR branchesSingleBuff(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;

   currentTurn.push(PE_MT_NO_ATTACK, multiplier*maxProb);

   return err;
}

PE_ERROR branchesSingleBuffDump(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;

   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (!state.isBuffed(move.side(), move.debuffType()[0], move.debuff()[0].subType())) {
      currentTurn.push(PE_MT_NO_ATTACK, multiplier*maxProb);
   }
   else {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesSingleDebuffDump(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;

   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (state.isBuffed(move.side()^1, move.debuffType()[0], move.debuff()[0].subType())) {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesSingleBuff(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesGravity(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;

   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (!state.isBuffed(move.side()^1, move.debuffType()[0], move.debuff()[0].subType())) {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesSingleDebuff(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesSingleAttackAddedEffect(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double effectProb = move.effectProb();
   double currentCrit = state.modCritActive(move.side(), move.critProb());
   
   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HIT_EFFECT, multiplier*(currentProb - currentCrit)*effectProb);
         currentTurn.push(PE_MT_HIT, multiplier*(currentProb - currentCrit)*(maxProb - effectProb));
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentCrit*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit*(maxProb - effectProb));
      }
      else {
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentProb*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb*(maxProb - effectProb));
      }

      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}

PE_ERROR branchesHLAttackAddedEffect(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double effectProb = move.effectProb();
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HITLOW_EFFECT, multiplier*(currentProb - currentCrit)*effectProb/2);
         currentTurn.push(PE_MT_HITHIGH_EFFECT, multiplier*(currentProb - currentCrit)*effectProb/2);
         currentTurn.push(PE_MT_HITLOW, multiplier*(currentProb - currentCrit)*(maxProb - effectProb) / 2);
         currentTurn.push(PE_MT_HITHIGH, multiplier*(currentProb - currentCrit)*(maxProb - effectProb)/2);
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentCrit*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit*(maxProb - effectProb));
      }
      else {
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentProb*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb*(maxProb - effectProb));
      }

      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}


PE_ERROR branchesSingleAttackAddedEffectDark(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double effectProb = move.effectProb();
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   if (state.weather() == PE_WEATHER_DARKNESS) effectProb = maxProb;
   
   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HITHIGH_EFFECT, multiplier*(currentProb - currentCrit)*effectProb);
         currentTurn.push(PE_MT_HITHIGH, multiplier*(currentProb - currentCrit)*(maxProb - effectProb));
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentCrit*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit*(maxProb - effectProb));
      }
      else {
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentProb*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb*(maxProb - effectProb));
      }

      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}

PE_ERROR branchesSingleAttackAddedEffectBlind(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double effectProb = move.effectProb();
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   if (!state.isBlinded(move.side()^1)) effectProb = 0;

   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HIT_EFFECT, multiplier*(currentProb - currentCrit)*effectProb);
         currentTurn.push(PE_MT_HIT, multiplier*(currentProb - currentCrit)*(maxProb - effectProb));
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentCrit*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit*(maxProb - effectProb));
      }
      else {
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentProb*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb*(maxProb - effectProb));
      }

      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}


PE_ERROR branchesHighLowAttackAddedEffectChill(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double effectProb = move.effectProb();
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   if (state.isChilled(move.side()^1)) effectProb = maxProb;
   
   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which redeuces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HITLOW_EFFECT, multiplier*(currentProb - currentCrit)*effectProb / 2);
         currentTurn.push(PE_MT_HITHIGH_EFFECT, multiplier*(currentProb - currentCrit)*effectProb / 2);
         currentTurn.push(PE_MT_HITLOW, multiplier*(currentProb - currentCrit)*(maxProb - effectProb) / 2);
         currentTurn.push(PE_MT_HITHIGH, multiplier*(currentProb - currentCrit)*(maxProb - effectProb) / 2);
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentCrit*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit*(maxProb - effectProb));
      }
      else {
         currentTurn.push(PE_MT_CRIT_EFFECT, multiplier*currentProb*effectProb);
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb*(maxProb - effectProb));
      }

      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}

PE_ERROR branchesMultiAttackThreeFirst(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound >= PE_ZERO_ROUND) { // move is done
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      err =  branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesMultiAttackTwoLast(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      currentTurn.push(PE_MT_MISS, maxProb);
   }
   else {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;   
}

PE_ERROR branchesMultiAttackTwoHLLast(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      currentTurn.push(PE_MT_MISS, maxProb);
   }
   else {
      err = branchesHighLowAttack(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesBleeding(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = move.baseProb();
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   if (state.isBleeding(move.side()^1)) {
      currentProb = maxProb;
   }
   currentProb = state.modAccuracyActive(move.side(), currentProb);
   
   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which reduces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HIT, multiplier*(currentProb - currentCrit));
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit);
      }
      else {
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb);
      }
      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}

PE_ERROR branchesBlinded(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = move.baseProb();
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   if (state.isBlinded(move.side()^1)) {
      currentProb = maxProb;
   }
   currentProb = state.modAccuracyActive(move.side(), currentProb);
   
   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which reduces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HIT, multiplier*(currentProb - currentCrit));
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit);
      }
      else {
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb);
      }
      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}

PE_ERROR branchesMoonlight(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = move.baseProb();
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   if (state.weather() == PE_WEATHER_MOONLIGHT) {
      currentProb = maxProb;
   }
   currentProb = state.modAccuracyActive(move.side(), currentProb);

   // currentProb gets modified
   // also check whether the damage is attenuated to zero, or there is an immune effect which reduces branching (all results are same)

   if (currentProb <= 0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HIT, multiplier*(currentProb - currentCrit));
         currentTurn.push(PE_MT_CRIT, multiplier*currentCrit);
      }
      else {
         currentTurn.push(PE_MT_CRIT, multiplier*currentProb);
      }
      if (currentProb < maxProb) {
         currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
      }
   }

   return err;
}

PE_ERROR branchesTwoAttacksAsOne(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   // currentProb gets modified

   if (currentProb <= 0.0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_CRITCRIT, multiplier*currentCrit); // i think the whole thing has to crit
         currentTurn.push(PE_MT_HITHIT, multiplier*(currentProb - currentCrit)*currentProb);
         if (currentProb < maxProb) {
            currentTurn.push(PE_MT_HIT, multiplier*(currentProb - currentCrit)*(maxProb - currentProb)); // this is the same as a single attack
            currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
         }
      }
      else {
         currentTurn.push(PE_MT_CRITCRIT, multiplier*currentProb); // i think the whole thing has to crit
         if (currentProb < maxProb) {
            currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
         }
      }
   }

   return err;
}

PE_ERROR branchesHLTwoAttacksAsOne(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   // currentProb gets modified

   if (currentProb <= 0.0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_CRITCRIT, multiplier*currentCrit); // i think the whole thing has to crit
         currentTurn.push(PE_MT_HITHIGHHIT, multiplier*(currentProb - currentCrit)*currentProb / 2);
         currentTurn.push(PE_MT_HITLOWHIT, multiplier*(currentProb - currentCrit)*currentProb / 2);
         if (currentProb < maxProb) {
            currentTurn.push(PE_MT_HITHIGH, multiplier*(currentProb - currentCrit)*(maxProb - currentProb) / 2); // this is the same as a single attack
            currentTurn.push(PE_MT_HITLOW, multiplier*(currentProb - currentCrit)*(maxProb - currentProb) / 2); // this is the same as a single attack
            currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
         }
      }
      else {
         currentTurn.push(PE_MT_CRITCRIT, multiplier*currentProb); // i think the whole thing has to crit
         if (currentProb < maxProb) {
            currentTurn.push(PE_MT_MISS, multiplier*(maxProb - currentProb));
         }
      }
   }

   return err;
}

PE_ERROR branchesTwoAttacksSecondNoCrit(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   // currentProb gets modified
   double mhitProb = (currentProb - currentCrit);
   double missProb = (maxProb - currentProb);

   if (currentProb <= 0.0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_CRITHIT, multiplier*currentCrit*currentProb);
         currentTurn.push(PE_MT_HITHIT, multiplier*mhitProb*currentProb);
         if (missProb > 0.0) {
            currentTurn.push(PE_MT_CRIT, multiplier*currentCrit*missProb);
            currentTurn.push(PE_MT_HIT, multiplier*mhitProb*missProb); // hit then miss = hit, right?
            currentTurn.push(PE_MT_MISSHIT, multiplier*missProb*currentProb); // hit then miss = hit, wrong!
            currentTurn.push(PE_MT_MISS, multiplier*missProb*missProb);
         }
      }
      else {
         currentTurn.push(PE_MT_CRITHIT, multiplier*currentProb*currentProb);
         if (missProb > 0.0) {
            currentTurn.push(PE_MT_CRIT, multiplier*currentProb*missProb);
            currentTurn.push(PE_MT_MISSHIT, multiplier*missProb*currentProb); // hit then miss = hit, wrong!
            currentTurn.push(PE_MT_MISS, multiplier*missProb*missProb);
         }
      }
   }

   return err;
}

PE_ERROR branchesTwoAttacks(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   // currentProb gets modified
   double mhitProb = (currentProb - currentCrit);
   double missProb = (maxProb - currentProb);

   if (currentProb <= 0.0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_CRITCRIT, multiplier*currentCrit*currentCrit);
         currentTurn.push(PE_MT_CRITHIT, multiplier*currentCrit*mhitProb);
         currentTurn.push(PE_MT_HITCRIT, multiplier*currentCrit*mhitProb);
         currentTurn.push(PE_MT_HITHIT, multiplier*mhitProb*mhitProb);
         if (missProb > 0.0) {
            currentTurn.push(PE_MT_HIT, 2*multiplier*mhitProb*missProb); // hit then miss = hit, right?
            currentTurn.push(PE_MT_CRIT, 2*multiplier*currentCrit*missProb); // hit then miss = hit, right?
            currentTurn.push(PE_MT_MISS, multiplier*missProb*missProb);
         }
      }
      else {
         currentTurn.push(PE_MT_CRITCRIT, multiplier*currentProb*currentProb);
         if (missProb > 0.0) {
            currentTurn.push(PE_MT_CRIT, 2 * multiplier*currentProb*missProb); // hit then miss = hit, right?
            currentTurn.push(PE_MT_MISS, multiplier*missProb*missProb);
         }
      }
   }
   
   return err;
}

PE_ERROR branchesThreeAttacks(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   // currentProb gets modified
   double mhitProb = (currentProb - currentCrit);
   double missProb = (maxProb - currentProb);

   if (currentProb <= 0.0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_CRITCRITCRIT, multiplier*currentCrit*currentCrit*currentCrit);
         currentTurn.push(PE_MT_CRITCRITHIT, multiplier*currentCrit*currentCrit*mhitProb);
         currentTurn.push(PE_MT_CRITHITCRIT, multiplier*currentCrit*mhitProb*currentCrit);
         currentTurn.push(PE_MT_HITCRITCRIT, multiplier*mhitProb*currentCrit*currentCrit);
         currentTurn.push(PE_MT_CRITHITHIT, multiplier*currentCrit*mhitProb*mhitProb);
         currentTurn.push(PE_MT_HITCRITHIT, multiplier*mhitProb*currentCrit*mhitProb);
         currentTurn.push(PE_MT_HITHITCRIT, multiplier*mhitProb*mhitProb*currentCrit);
         currentTurn.push(PE_MT_HITHITHIT, multiplier*mhitProb*mhitProb*mhitProb);
         if (missProb > 0.0) {
            double addlM = multiplier*3*missProb;
            currentTurn.push(PE_MT_CRITCRIT, addlM*currentCrit*currentCrit);
            currentTurn.push(PE_MT_CRITHIT, addlM*currentCrit*mhitProb);
            currentTurn.push(PE_MT_HITCRIT, addlM*currentCrit*mhitProb);
            currentTurn.push(PE_MT_HITHIT, addlM*mhitProb*mhitProb);
            addlM = multiplier*3*missProb*missProb;
            currentTurn.push(PE_MT_CRIT, addlM*currentCrit);
            currentTurn.push(PE_MT_HIT, addlM*mhitProb);
            addlM = multiplier*missProb*missProb*missProb;
            currentTurn.push(PE_MT_MISS, addlM);
         }
      }
      else {
         currentTurn.push(PE_MT_CRITCRITCRIT, multiplier*currentProb*currentProb*currentProb);
         if (missProb > 0.0) {
            currentTurn.push(PE_MT_CRITCRIT, multiplier * 3 * missProb*currentProb*currentProb);
            currentTurn.push(PE_MT_CRIT, multiplier * 3 * missProb*missProb*currentProb);
            currentTurn.push(PE_MT_MISS, multiplier*missProb*missProb*missProb);
         }
      }
   }

   return err;
}

PE_ERROR branchesFourAttacks(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   // currentProb gets modified
   double mhitProb = (currentProb - currentCrit);
   double missProb = (maxProb - currentProb);

   if (currentProb <= 0.0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         currentTurn.push(PE_MT_HITHITHITHIT, multiplier*mhitProb*mhitProb*mhitProb*mhitProb);
         currentTurn.push(PE_MT_HITHITHITCRIT, multiplier*mhitProb*mhitProb*mhitProb*currentCrit);
         currentTurn.push(PE_MT_HITHITCRITHIT, multiplier*mhitProb*mhitProb*currentCrit*mhitProb);
         currentTurn.push(PE_MT_HITCRITHITHIT, multiplier*mhitProb*currentCrit*mhitProb*mhitProb);
         currentTurn.push(PE_MT_CRITHITHITHIT, multiplier*currentCrit*mhitProb*mhitProb*mhitProb);
         currentTurn.push(PE_MT_HITHITCRITCRIT, multiplier*mhitProb*mhitProb*currentCrit*currentCrit);
         currentTurn.push(PE_MT_HITCRITHITCRIT, multiplier*mhitProb*currentCrit*mhitProb*currentCrit);
         currentTurn.push(PE_MT_HITCRITCRITHIT, multiplier*mhitProb*currentCrit*currentCrit*mhitProb);
         currentTurn.push(PE_MT_CRITHITHITCRIT, multiplier*currentCrit*mhitProb*mhitProb*currentCrit);
         currentTurn.push(PE_MT_CRITHITCRITHIT, multiplier*currentCrit*mhitProb*currentCrit*mhitProb);
         currentTurn.push(PE_MT_CRITCRITHITHIT, multiplier*currentCrit*currentCrit*mhitProb*mhitProb);
         currentTurn.push(PE_MT_CRITCRITCRITHIT, multiplier*currentCrit*currentCrit*currentCrit*mhitProb);
         currentTurn.push(PE_MT_CRITCRITHITCRIT, multiplier*currentCrit*currentCrit*mhitProb*currentCrit);
         currentTurn.push(PE_MT_CRITHITCRITCRIT, multiplier*currentCrit*mhitProb*currentCrit*currentCrit);
         currentTurn.push(PE_MT_HITCRITCRITCRIT, multiplier*mhitProb*currentCrit*currentCrit*currentCrit);
         currentTurn.push(PE_MT_CRITCRITCRITCRIT, multiplier*currentCrit*currentCrit*currentCrit*currentCrit);

         if (missProb > 0.0) {
            //printf("mm %f\n", multiplier*(1 - (1 - missProb)*(1 - missProb)*(1 - missProb)*(1 - missProb)));
            double addlM = multiplier*4*missProb;
            currentTurn.push(PE_MT_CRITCRITCRIT, addlM*currentCrit*currentCrit*currentCrit);
            currentTurn.push(PE_MT_CRITCRITHIT, addlM*currentCrit*currentCrit*mhitProb);
            currentTurn.push(PE_MT_CRITHITCRIT, addlM*currentCrit*mhitProb*currentCrit);
            currentTurn.push(PE_MT_HITCRITCRIT, addlM*mhitProb*currentCrit*currentCrit);
            currentTurn.push(PE_MT_CRITHITHIT, addlM*currentCrit*mhitProb*mhitProb);
            currentTurn.push(PE_MT_HITCRITHIT, addlM*mhitProb*currentCrit*mhitProb);
            currentTurn.push(PE_MT_HITHITCRIT, addlM*mhitProb*mhitProb*currentCrit);
            currentTurn.push(PE_MT_HITHITHIT, addlM*mhitProb*mhitProb*mhitProb);
            addlM = multiplier*6*missProb*missProb;
            currentTurn.push(PE_MT_CRITCRIT, addlM*currentCrit*currentCrit);
            currentTurn.push(PE_MT_CRITHIT, addlM*currentCrit*mhitProb);
            currentTurn.push(PE_MT_HITCRIT, addlM*currentCrit*mhitProb);
            currentTurn.push(PE_MT_HITHIT, addlM*mhitProb*mhitProb);
            addlM = multiplier*4*missProb*missProb*missProb;
            currentTurn.push(PE_MT_CRIT, addlM*currentCrit);
            currentTurn.push(PE_MT_HIT, addlM*mhitProb);
            addlM = multiplier*missProb*missProb*missProb*missProb;
            currentTurn.push(PE_MT_MISS, addlM);
         }
      }
      else {
         currentTurn.push(PE_MT_CRITCRITCRITCRIT, multiplier*currentProb*currentProb*currentProb*currentProb);
         if (missProb > 0.0) {
            currentTurn.push(PE_MT_CRITCRITCRIT, multiplier * 4 * missProb*currentProb*currentProb*currentProb);
            currentTurn.push(PE_MT_CRITCRIT, multiplier * 6 * missProb*missProb*currentProb*currentProb);
            currentTurn.push(PE_MT_CRIT, multiplier * 4 * missProb*missProb*missProb*currentProb);
            currentTurn.push(PE_MT_MISS, multiplier*missProb*missProb*missProb*missProb);
         }
      }
   }

   return err;
}

// not currently used.  if implimented, make sure currentCrit >= currentProb case is accounted for
PE_ERROR branchesFourAttacksEffect(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   const int32_t normalHits = 4;
   const int32_t effect = one<<(2*normalHits);
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());
   double currentCrit = state.modCritActive(move.side(), move.critProb());

   // currentProb gets modified
   double mhitProb = (currentProb - currentCrit);
   double missProb = (maxProb - currentProb);

   if (currentProb <= 0.0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      if (currentProb > currentCrit) {
         for (int32_t outcome = 0; outcome < (one<<(2*normalHits)); outcome++) {
            double prob = multiplier;
            for (uint32_t hitCheck = 0; hitCheck < normalHits; hitCheck++) {
               if (outcome&(one<<(2*hitCheck))) prob *= mhitProb;
               else if (outcome&(one<<(2*hitCheck + 1))) prob *= currentCrit;
               else prob*= missProb;
            }
            if (prob > 0) {
               currentTurn.push(static_cast<PE_MOVETYPE>(outcome), prob*(1 - move.effectProb()));
               currentTurn.push(static_cast<PE_MOVETYPE>(outcome|effect), prob*move.effectProb());
            }
         }
      }
   }

   return err;
}

// approximation
PE_ERROR branchesFourAttacksEffectFiftyApprox(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   const int32_t normalHits = 4;
   const int32_t effect = one<<(2*normalHits);
   const double normalize = multiplier/0.875;
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());

   // currentProb gets modified
   double missProb = (maxProb - currentProb);

   // we will do
   // two hits + effect             
   // three hits + effect
   // one hit + effect
   // two hits + no effect
   // three hits + no effect
   // one hit + no effect

   if (currentProb <= 0.0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      currentTurn.push(static_cast<PE_MOVETYPE>(0x05|effect), normalize*move.effectProb()*6*currentProb*currentProb*missProb*missProb);
      currentTurn.push(static_cast<PE_MOVETYPE>(0x15|effect), normalize*move.effectProb()*4*currentProb*currentProb*currentProb*missProb);
      currentTurn.push(static_cast<PE_MOVETYPE>(0x01|effect), normalize*move.effectProb()*4*currentProb*missProb*missProb*missProb); 
      //currentTurn.push(static_cast<PE_MOVETYPE>(0x55|effect), move.effectProb()*currentProb*currentProb*currentProb*currentProb);
      //currentTurn.push(static_cast<PE_MOVETYPE>(0x00|effect), move.effectProb()*missProb*missProb*missProb*missProb); 
      currentTurn.push(static_cast<PE_MOVETYPE>(0x05), (1.0 - move.effectProb())*normalize*6*currentProb*currentProb*missProb*missProb);
      currentTurn.push(static_cast<PE_MOVETYPE>(0x15), (1.0 - move.effectProb())*normalize*4*currentProb*currentProb*currentProb*missProb);
      currentTurn.push(static_cast<PE_MOVETYPE>(0x01), (1.0 - move.effectProb())*normalize*4*currentProb*missProb*missProb*missProb); 
      //currentTurn.push(static_cast<PE_MOVETYPE>(0x55), (1.0 - move.effectProb())*currentProb*currentProb*currentProb*currentProb);
      //currentTurn.push(static_cast<PE_MOVETYPE>(0x00), (1.0 - move.effectProb())*missProb*missProb*missProb*missProb); 
   }

   return err;
}

// approximation
/*PE_ERROR branchesFourAttacksFiftyApprox(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   const int32_t normalHits = 4;
   const double normalize = multiplier/0.875;
   PE_ERROR err = PE_OK;
   double currentProb = state.modAccuracyActive(move.side(), move.baseProb());

   // currentProb gets modified
   double missProb = (maxProb - currentProb);

   // we will do
   // two hits + effect             
   // three hits + effect
   // one hit + effect
   // two hits + no effect
   // three hits + no effect
   // one hit + no effect

   if (currentProb <= 0.0) {
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      currentTurn.push(static_cast<PE_MOVETYPE>(0x05|effect), normalize*move.effectProb()*6*currentProb*currentProb*missProb*missProb);
      currentTurn.push(static_cast<PE_MOVETYPE>(0x15|effect), normalize*move.effectProb()*4*currentProb*currentProb*currentProb*missProb);
      currentTurn.push(static_cast<PE_MOVETYPE>(0x01|effect), normalize*move.effectProb()*4*currentProb*missProb*missProb*missProb); 
      //currentTurn.push(static_cast<PE_MOVETYPE>(0x55|effect), move.effectProb()*currentProb*currentProb*currentProb*currentProb);
      //currentTurn.push(static_cast<PE_MOVETYPE>(0x00|effect), move.effectProb()*missProb*missProb*missProb*missProb); 
      currentTurn.push(static_cast<PE_MOVETYPE>(0x05), (1.0 - move.effectProb())*normalize*6*currentProb*currentProb*missProb*missProb);
      currentTurn.push(static_cast<PE_MOVETYPE>(0x15), (1.0 - move.effectProb())*normalize*4*currentProb*currentProb*currentProb*missProb);
      currentTurn.push(static_cast<PE_MOVETYPE>(0x01), (1.0 - move.effectProb())*normalize*4*currentProb*missProb*missProb*missProb); 
      //currentTurn.push(static_cast<PE_MOVETYPE>(0x55), (1.0 - move.effectProb())*currentProb*currentProb*currentProb*currentProb);
      //currentTurn.push(static_cast<PE_MOVETYPE>(0x00), (1.0 - move.effectProb())*missProb*missProb*missProb*missProb); 
   }

   return err;
}*/

// begin attack branches that redirect based on chance

PE_ERROR branchesOneTwoAttacks(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if ((err = branchesSingleAttack(move, multiplier*fiftyProb, state, currentTurn)) != PE_OK) {
      return err;
   }
   if ((err = branchesTwoAttacks(move, multiplier*fiftyProb, state, currentTurn)) != PE_OK) {
      return err;
   }

   return err;
}

PE_ERROR branchesTwoThreeAttacks(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;
   
   if ((err = branchesTwoAttacks(move, multiplier*fiftyProb, state, currentTurn)) != PE_OK) {
      return err;
   }
   if ((err = branchesThreeAttacks(move, multiplier*fiftyProb, state, currentTurn)) != PE_OK) {
      return err;
   }

   return err;
}

// begin attack branches that redirect based on conditions

PE_ERROR branchesTwoIfAttacked(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (!state.activeDamaged(move.side())) {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesTwoIfFaster(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.isSpeedWinner(move.side())) {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesTwoIfSlower(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.isSpeedLoser(move.side())) {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesTwoIfBleeding(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.isBleeding(move.side()^1)) {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesTwoIfStunned(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.isStunned(move.side() ^ 1)) {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesTwoIfBurning(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.isBurning(move.side()^1)) {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesTwoIfChilled(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.isChilled(move.side()^1)) {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesTwoIfSun(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.weather() == PE_WEATHER_SUNLIGHT) {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }

   return err;
}


PE_ERROR branchesTwoFourIfWebbed(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.isWebbed(move.side()^1)) {
      err = branchesFourAttacks(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesTwoAttacks(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesTwoIfLowerHealth(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.health(move.side()) >= state.health(move.side()^1)) {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesTwoIfArcane(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.weather() != PE_WEATHER_ARCANE) {
      err = branchesSingleAttack(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesHLTwoIfArcane(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err;

   if (state.weather() != PE_WEATHER_ARCANE) {
      err = branchesHighLowAttack(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesHLTwoAttacksAsOne(move, multiplier, state, currentTurn);
   }

   return err;
}


PE_ERROR branchesThreeIfFaster(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;

   if (state.isSpeedWinner(move.side())) {
      err = branchesTwoThreeAttacks(move, multiplier, state, currentTurn);
   }
   else {
      err = branchesOneTwoAttacks(move, multiplier, state, currentTurn);
   }

   return err;
}

PE_ERROR branchesOnetoThreeAttacks(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;

   if ((err = branchesSingleAttack(move, multiplier*thirdProb, state, currentTurn)) != PE_OK) {
      return err;
   }
   if ((err = branchesTwoAttacks(move, multiplier*thirdProb, state, currentTurn)) != PE_OK) {
      return err;
   }
   if ((err = branchesThreeAttacks(move, multiplier*thirdProb, state, currentTurn)) != PE_OK) {
      return err;
   }

   return err;
}

PE_ERROR branchesMultiAttackThreeFirstSun(Move &move, const double &multiplier, State &state, Turn &currentTurn) {
   PE_ERROR err = PE_OK;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound >= PE_ZERO_ROUND) { // move is done
      currentTurn.push(PE_MT_MISS, multiplier*maxProb);
   }
   else {
      err = branchesTwoIfSun(move, multiplier, state, currentTurn);
   }

   return err;
}

// begin moves

PE_SV moveNull(Move &move, const PE_MOVETYPE &type, State &state) {

   return 0;
}

PE_SV moveSingleAttack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   return damageDone;
}

PE_SV moveHighLowAttack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else if (type == PE_MT_HITLOW) {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0] - move.abilityValues()[1], move.pet(), false, true, false, false);
   }
   else if (type == PE_MT_HITHIGH) {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0] + move.abilityValues()[1], move.pet(), false, true, false, false);
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0] + move.abilityValues()[1], move.pet(), true, true, false, false);
   }

   return damageDone;
}

PE_SV moveSingleHeal(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;

   healingDone = state.doHealActive(move.side(), move.abilityValues()[0], true);

   return healingDone;
}

PE_SV moveHealEqualize(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV exchange = (state.health(move.side()) - state.health(move.side()^1))/2;

   if (exchange > 0) {
      state.directReduceHealth(move.side(), exchange);
      state.directIncreaseHealth(move.side()^1, exchange);
   }
   else if (exchange < 0) {
      state.directReduceHealth(move.side()^1, -exchange);
      state.directIncreaseHealth(move.side(), -exchange);
   }

   return exchange;
}

PE_SV moveSingleHealBuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;

   healingDone = state.doHealActive(move.side(), move.abilityValues()[0], true);
   state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0])); // does the health add

   return healingDone;
}


PE_SV moveHealFraction(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0, heal;

   heal = state.maxHealth(move.side())*move.abilityValues()[0]/maxEval;
   healingDone = state.doHealActive(move.side(), heal, true);

   return healingDone;
}

PE_SV moveHealHalfHit(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;

   healingDone = state.doHealActive(move.side(), move.abilityValues()[0], true);
   healingDone += state.doHealActive(move.side(), state.lastHit(move.side())/2, true);

   return healingDone;
}

PE_SV moveHealAll(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      healingDone += state.doHeal(move.side(), pet, move.abilityValues()[0], true);
   }

   return healingDone;
}

PE_SV moveHealOthers(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      if (pet != state.activePet(move.side())) {
         healingDone += state.doHeal(move.side(), pet, move.abilityValues()[0], true);
      }
   }

   return healingDone;
}

PE_SV moveHealAllAquatic(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      if (state.petFamily(move.side(), pet, PE_FP_GENERAL) == PE_FAMILY_AQUATIC) {
         healingDone += state.doHeal(move.side(), pet, move.abilityValues()[0], true);
      }
   }

   return healingDone;
}

PE_SV moveHealSelfAllMechanical(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;

   healingDone += state.doHeal(move.side(), state.activePet(move.side()), move.abilityValues()[0], true);
   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      if (state.petFamily(move.side(), pet, PE_FP_GENERAL) == PE_FAMILY_AQUATIC &&
         pet != state.activePet(move.side())) {
         healingDone += state.doHeal(move.side(), pet, move.abilityValues()[0]/2, true);
      }
   }

   return healingDone;
}

PE_SV moveHealAllApplyDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      healingDone += state.doHeal(move.side(), pet, move.abilityValues()[0], true);
   }
   if (type == PE_MT_MISS) {
   }
   else {
      state.pushDebuffActive(move.side() ^ 1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return healingDone;
}


PE_SV moveSingleDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return damageDone;
}

PE_SV moveTwoDebuffs(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      if (state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]))) {
         state.pushDebuffActive(move.side()^1, move.debuffType()[1], Debuff(move.debuff()[1]));
      }
   }

   return damageDone;
}

PE_SV moveTeamDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
         state.pushDebuff(move.side()^1, pet, move.debuffType()[0], Debuff(move.debuff()[0]));
      }
   }

   return damageDone;
}

PE_SV moveSingleBuff(Move &move, const PE_MOVETYPE &type, State &state) {

   state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));

   return 0;
}

PE_SV moveSingleBuffOverDump(Move &move, const PE_MOVETYPE &type, State &state) {

   if (!state.isBuffed(move.side(), move.debuffType()[0], move.debuff()[0].subType())) {
      state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
   }
   else {
      state.pushDebuffActive(move.side(), move.debuffType()[1], Debuff(move.debuff()[1]));
   }

   return 0;
}

// pushes a buff/debuf to both sides without being blocked
PE_SV moveSingleBuffBoth(Move &move, const PE_MOVETYPE &type, State &state) {

   state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
   state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));

   return 0;
}

PE_SV moveSingleBuffTransform(Move &move, const PE_MOVETYPE &type, State &state) {

   state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
   state.setFamily(move.side(), move.family());

   return 0;
}

PE_SV moveTwoBuffs(Move &move, const PE_MOVETYPE &type, State &state) {

   state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
   state.pushDebuffActive(move.side(), move.debuffType()[1], Debuff(move.debuff()[1]));

   return 0;
}

PE_SV moveHealTwoBuffs(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;

   healingDone = state.doHealActive(move.side(), move.abilityValues()[0], true);
   state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
   state.pushDebuffActive(move.side(), move.debuffType()[1], Debuff(move.debuff()[1]));

   return healingDone;
}

PE_SV moveBuffDebuffClear(Move &move, const PE_MOVETYPE &type, State &state) {
	PE_SV damageDone = 0;

   state.removeDebuffsActive(move.side());

	return 0;
}

PE_SV moveTeamBuffDebuffClear(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      state.removeDebuffs(move.side(), pet);
   }

   return 0;
}

PE_SV moveSingleBuffTeam(Move &move, const PE_MOVETYPE &type, State &state) {

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      state.pushDebuff(move.side(), pet, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return 0;
}

PE_SV moveTeamAttackSame(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      damageDone += state.doHit(move.side() ^ 1, pet, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   return 0;
}

PE_SV moveTeamAttackSameHealthCost(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      damageDone += state.doHit(move.side() ^ 1, pet, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   }
   PE_SV healthHit = static_cast<PE_SV>((static_cast<double>(state.maxHealth(move.side()))*move.abilityValues()[1]) / maxEval);

   state.directReduceHealth(move.side(), healthHit);

   return 0;
}

// this would be a paint to really do because we need to know how many are alive to do the hit/crit prob
PE_SV moveTeamAttackSplit(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   int petsLeft = 0;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      if (state.allHealth(move.side() ^ 1, pet) > 0) {
         petsLeft++;
      }
   }

   PE_SV hit = move.abilityValues()[0] / petsLeft;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      damageDone += state.doHit(move.side() ^ 1, pet, move.family(), hit, move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   return 0;
}

PE_SV moveMultiTeamAttackSameExtraRain(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_SV hit = move.abilityValues()[0];
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
   }
   if (state.weather() == PE_WEATHER_RAIN) {
      hit = static_cast<PE_SV>(hit*PE_DREADFUL_BREATH);
   }
   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      damageDone += state.doHit(move.side() ^ 1, pet, move.family(), hit, move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   return 0;
}

PE_SV moveTeamAttack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   int activePet = state.activePet(move.side()^1);
   damageDone += state.doHit(move.side() ^ 1, activePet, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      if (pet != activePet) {
         damageDone += state.doHit(move.side() ^ 1, pet, move.family(), move.abilityValues()[1], move.pet(), type == PE_MT_CRIT, true, false, false);
      }
   }

   return 0;
}

PE_SV moveTeamAttackHalf(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_SV hit = move.abilityValues()[0];

   int activePet = state.activePet(move.side()^1);
   damageDone += state.doHit(move.side() ^ 1, activePet, move.family(), hit, move.pet(), type == PE_MT_CRIT, true, false, false);
   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      if (pet != activePet) {
         damageDone += state.doHit(move.side() ^ 1, pet, move.family(), hit / 2, move.pet(), type == PE_MT_CRIT, true, false, false);
      }
   }

   return 0;
}

PE_SV moveHighLowAttackObjectClear(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveHighLowAttack(move, type, state);
   state.removeObjects();

   return 0;
}

PE_SV moveTeamAttackObjectClear(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      damageDone += state.doHit(move.side() ^ 1, pet, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   }
   state.removeObjects();

   return 0;
}

PE_SV moveSingleAttackBack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (damageDone) {
         state.doHitActive(move.side(), move.family(), move.abilityValues()[1], PE_NO_ACTIVE, false, true, false, false);
      }
   }

   return damageDone;
}

PE_SV moveSingleAttackApplyDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (damageDone > 0) {
         state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
      }
   }

   return damageDone;
}

PE_SV moveHighLowAttackApplyDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = moveHighLowAttack(move, type, state);
      if (damageDone > 0) {
         state.pushDebuffActive(move.side() ^ 1, move.debuffType()[0], Debuff(move.debuff()[0]));
      }
   }

   return damageDone;
}

PE_SV moveSingleHealthAttackDie(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), static_cast<PE_SV>((move.abilityValues()[0] * state.maxHealth(move.side())) / maxEval), move.pet(), type == PE_MT_CRIT, true, false, false);
   }
   state.die(move.side(), move.pet()); // is this dangerous?

   return damageDone;
}

PE_SV moveTeamAttackDie(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
         damageDone += state.doHit(move.side() ^ 1, pet, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      }
   }
   state.die(move.side(), move.pet()); // is this dangerous?

   return damageDone;
}

PE_SV moveTeamDebuffDie(Move &move, const PE_MOVETYPE &type, State &state) {

   if (type == PE_MT_MISS) {
   }
   else {
      for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
         state.pushDebuff(move.side() ^ 1, pet, move.debuffType()[0], Debuff(move.debuff()[0]));
      }
   }
   state.die(move.side(), move.pet()); // is this dangerous?

   return PE_OK;
}

PE_SV moveHaunt(Move &move, const PE_MOVETYPE &type, State &state) {
   Debuff deb = Debuff(move.debuff()[0]);

   if (type == PE_MT_MISS) {
   }
   else {
      (*deb.values()).push_back(state.health(move.side()));
      if (state.pushDebuffActive(move.side() ^ 1, move.debuffType()[0], deb)) {
         state.die(move.side(), move.pet());
      }
   }

   return PE_OK;
}

PE_SV moveSingleHealthAttackApplyDebuffDie(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0, hit = static_cast<PE_SV>((move.abilityValues()[0] * state.maxHealth(move.side())) / maxEval);

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), hit, move.pet(), type == PE_MT_CRIT, true, false, false);
      if (damageDone > 0) {
         state.pushDebuffActive(move.side() ^ 1, move.debuffType()[0], Debuff(move.debuff()[0]));
      }
   }
   state.die(move.side(), move.pet()); // is this dangerous?

   return damageDone;
}

PE_SV moveSingleAttackApplyTwoDebuffs(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (damageDone > 0) {
         state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
         state.pushDebuffActive(move.side()^1, move.debuffType()[1], Debuff(move.debuff()[1]));
      }
   }

   return damageDone;
}

PE_SV moveTeamAttackApplyDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      for (PE_PET_NUMBER pet = 0; pet< PE_TEAM_SIZE; pet++) {
         damageDone += state.doHit(move.side() ^ 1, pet, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
         if (damageDone > 0) {
            state.pushDebuff(move.side()^1, pet, move.debuffType()[0], Debuff(move.debuff()[0]));
         }
      }
   }

   return damageDone;
}

PE_SV moveSingleAttackApplyBuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   }
   state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));

   return damageDone;
}

PE_SV moveHighLowAttackApplyBuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = moveHighLowAttack(move, type, state);
   }
   state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));

   return damageDone;
}


PE_SV moveSingleAttackApplyBuffDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
	PE_SV damageDone = 0;

	if (type == PE_MT_MISS) {
	}
	else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
		if (damageDone > 0) {
			state.pushDebuffActive(move.side() ^ 1, move.debuffType()[1], Debuff(move.debuff()[1]));
		}
	}
	state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));

	return damageDone;
}

PE_SV moveSingleBuffDump(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (!state.isBuffed(move.side(), move.debuffType()[0], move.debuff()[0].subType())) {
      state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
   }
   else {
      if (type == PE_MT_MISS) {
      }
      else {
         damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      }
      state.removeDebuffActive(move.side(), move.debuffType()[0], move.debuff()[0].subType(), PE_NO_TRACK);
   }

   return damageDone;
}

PE_SV moveSingleDebuffDump(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (!state.isBuffed(move.side()^1, move.debuffType()[0], move.debuff()[0].subType())) {
      state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }
   else {
      if (type == PE_MT_MISS) {
      }
      else {
         damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      }
      state.removeDebuffActive(move.side()^1, move.debuffType()[0], move.debuff()[0].subType(), PE_NO_TRACK);
   }

   return damageDone;
}

PE_SV moveLastHit(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_SV lastHit = state.lastHit(move.side());

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), lastHit, move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   return damageDone;
}


PE_SV moveSingleAttackSwitchNext(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (damageDone) {
         PE_PET_NUMBER current = state.lastActivePet(move.side()^1);
         PE_PET_NUMBER first, second;
         if (current == PE_PET1) {
            first = PE_PET2;
            second = PE_PET3;
         }
         else if (current = PE_PET2) {
            first = PE_PET3;
            second = PE_PET1;
         }
         else if (current = PE_PET3) {
            first = PE_PET1;
            second = PE_PET2;
         }
         if (state.allHealth(move.side()^1, first) > 0) {
            state.activate(move.side()^1, first);
            state.pushDebuffActive(move.side()^1, move.debuffType()[0], move.debuff()[0]);
         }
         else if (state.allHealth(move.side()^1, second) > 0) {
            state.activate(move.side()^1, second);
            state.pushDebuffActive(move.side()^1, move.debuffType()[0], move.debuff()[0]);
         }
      }
   }

   return damageDone;
}

PE_SV moveSingleAttackSwitchHighest(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   if (damageDone) {
      int hpet = -1;
      for (int opet = 0; opet < PE_TEAM_SIZE; opet++) {
         if (opet != state.activePet(move.side()^1)) {
            if (hpet < 0) {
               if (state.allHealth(move.side()^1, opet) > 0) {
                  hpet = opet;
               }
            }
            else if (state.allHealth(move.side()^1, opet) > state.allHealth(move.side()^1, hpet)) {
               hpet = opet;
            }
         }
      }
      if (hpet >= 0) {
         state.activate(move.side()^1, hpet);
         state.pushDebuffActive(move.side() ^ 1, move.debuffType()[0], move.debuff()[0]);
      }
   }

   return damageDone;
}

PE_SV moveSingleAttackSwitchLowest(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   if (damageDone) {
      int hpet = -1;
      for (int opet = 0; opet < PE_TEAM_SIZE; opet++) {
         if (opet != state.activePet(move.side() ^ 1)) {
            if (hpet < 0) {
               if (state.allHealth(move.side() ^ 1, opet) > 0) {
                  hpet = opet;
               }
            }
            else if (state.allHealth(move.side() ^ 1, opet) < state.allHealth(move.side() ^ 1, hpet)) {
               hpet = opet;
            }
         }
      }
      if (hpet >= 0) {
         state.activate(move.side() ^ 1, hpet);
         state.pushDebuffActive(move.side() ^ 1, move.debuffType()[0], move.debuff()[0]);
      }
   }

   return damageDone;
}


PE_SV moveRighteousInspiration(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   int hpet = -1;

   for (int opet = 0; opet < PE_TEAM_SIZE; opet++) {
      if (opet != state.activePet(move.side())) {
         if (hpet < 0) {
            if (state.allHealth(move.side(), opet) > 0) {
               hpet = opet;
            }
         }
         else if (state.allHealth(move.side(), opet) > state.allHealth(move.side(), hpet)) {
            hpet = opet;
         }
      }
   }
   if (hpet >= 0) {
      state.activate(move.side(), hpet);
   }
   state.pushDebuffActive(move.side(), move.debuffType()[0], move.debuff()[0]);
   state.pushDebuffActive(move.side(), move.debuffType()[1], move.debuff()[1]);

   return PE_OK;
}

// this is gravity and stupid
PE_SV moveGravity(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (!state.isBuffed(move.side()^1, move.debuffType()[0], move.debuff()[0].subType())) {
      damageDone = moveSingleAttackApplyDebuff(move, type, state);
   }
   else {
      if (type == PE_MT_MISS) {
      }
      else {
         state.pushDebuffActive(move.side()^1, move.debuffType()[1], move.debuff()[1]);
      }
   }

   return damageDone;
}

PE_SV moveHealAttack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (damageDone > 0) {
         state.doHealActive(move.side(), static_cast<PE_SV>(damageDone*move.effectProb()), true);
      }
   }

   return damageDone;
}

PE_SV moveHealAttackDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      state.doHealActive(move.side(), static_cast<PE_SV>(move.abilityValues()[1]), true);
      if (damageDone > 0) {
         state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
      }
   }

   return damageDone;
}

PE_SV moveNoBelow1Attack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.checkHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (damageDone >= state.health(move.side()^1)) {
         state.directIncreaseHealth(move.side()^1, 1 + damageDone - state.health(move.side()^1));
      }
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   return damageDone;
}

PE_SV moveHealDoulbeWebAttack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (damageDone > 0) {
         PE_SV toHeal = move.abilityValues()[1];
         state.doHealActive(move.side(), toHeal, true);
         if (state.isWebbed(move.side()^1)) {
            state.doHealActive(move.side(), toHeal, true);
         }
      }
   }

   return damageDone;
}

PE_SV moveHealKillAttack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      PE_SV preHealth = state.health(move.side()^1);
      damageDone = moveSingleAttack(move, type, state);
      if (damageDone >= preHealth && state.activePet(move.side()^1) == PE_NO_ACTIVE) { // does it count if there is a procced effect that kills?
         state.doHealActive(move.side(), move.abilityValues()[1], true);
      }
   }

   return damageDone;
}

PE_SV moveTwoAttacksAsOne(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type <= PE_MT_ONE_ATTACK) {
      damageDone = moveSingleAttack(move, type, state);
   }
   else {// just set the trigger to false!
      if (type == PE_MT_HITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[1], move.pet(), false, false, false, false);
      }
      else if (type == PE_MT_CRITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[1], move.pet(), true, false, false, false);
      }
   }

   return damageDone;
}

PE_SV moveSecondIfFaster(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      PE_SV hit;

      if (state.isSpeedWinner(move.side())) hit = move.abilityValues()[1];
      else hit = move.abilityValues()[0];
      damageDone += state.doHitActive(move.side() ^ 1, move.family(), hit, move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   return damageDone;
}


PE_SV moveHLTwoAttacksAsOne(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type <= PE_MT_ONE_ATTACK) {
      damageDone = moveHighLowAttack(move, type, state);
   }
   else {// just set the trigger to false!
      if (type != PE_MT_CRITCRIT) {
         if (type == PE_MT_HITLOWHIT) {
            damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0] - move.abilityValues()[1], move.pet(), false, true, false, false);
         }
         else if (type == PE_MT_HITHIGHHIT) {
            damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0] + move.abilityValues()[1], move.pet(), false, true, false, false);
         }
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[2], move.pet(), false, false, false, false);
      }
      else { // both crit
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0] + move.abilityValues()[1], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[2], move.pet(), true, false, false, false);
      }
   }

   return damageDone;
}

PE_SV moveTwoAttackFraction(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_SV secondHit = state.maxHealth(move.side()^1)*move.abilityValues()[1]/maxEval;

   if (type <= PE_MT_ONE_ATTACK) {
      damageDone = moveSingleAttack(move, type, state);
   }
   else {// just set the trigger to false!
      if (type == PE_MT_HITHIT) {
         damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.directReduceHealth(move.side()^1, secondHit);
      }
      else if (type == PE_MT_CRITHIT) {
         damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.directReduceHealth(move.side()^1, secondHit);
      }
      else if (type == PE_MT_MISSHIT) {
         damageDone = state.directReduceHealth(move.side()^1, secondHit);
      }
   }

   return damageDone;
}

PE_SV moveDrainBlood(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type ==PE_MT_MISS) {
   }
   else {
      damageDone += state.directReduceHealth(move.side() ^ 1, static_cast<PE_SV>((move.abilityValues()[0] * state.health(move.side() ^ 1)) / maxEval));
   }
   state.doHealActive(move.side(), static_cast<PE_SV>((move.abilityValues()[1] * damageDone)) / maxEval, true);

   return damageDone;
}

PE_SV moveTwoAttacksSame(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type <= PE_MT_ONE_ATTACK) {
      damageDone = moveSingleAttack(move, type, state);
   }
   else {
      if (type == PE_MT_HITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_CRITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_HITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_CRITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
   }

   return damageDone;
}

PE_SV moveThreeAttacksSame(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type <= PE_MT_ONE_ATTACK) {
      damageDone = moveSingleAttack(move, type, state);
   }
   else if (type <= PE_MT_TWO_ATTACKS) {
      damageDone = moveTwoAttacksSame(move, type, state);
   }
   else {
      if (type == PE_MT_HITHITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_HITHITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_HITCRITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_CRITHITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_CRITCRITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_CRITHITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_HITCRITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_CRITCRITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
   }

   return damageDone;
}

PE_SV moveThreeAttacksSameDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   moveThreeAttacksSame(move, type, state);
   state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));

   return damageDone;
}


PE_SV moveFourAttacksSame(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type <= PE_MT_ONE_ATTACK) {
      damageDone = moveSingleAttack(move, type, state);
   }
   else if (type <= PE_MT_TWO_ATTACKS) {
      damageDone = moveTwoAttacksSame(move, type, state);
   }
   else if (type <= PE_MT_THREE_ATTACKS) {
      damageDone = moveThreeAttacksSame(move, type, state);
   }
   else {
      if (type == PE_MT_HITHITHITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_HITHITHITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_HITHITCRITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_HITCRITHITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_CRITHITHITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_HITHITCRITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_HITCRITHITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_HITCRITCRITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_CRITHITHITCRIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_CRITHITCRITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_CRITCRITHITHIT) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_CRITCRITCRITHIT) {
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_CRITCRITHITCRIT) {
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_CRITHITCRITCRIT) {
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_HITCRITCRITCRIT) {
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
      else if (type == PE_MT_CRITCRITCRITCRIT) {
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
   }

   return damageDone;
}

PE_SV moveFourAttacksSameDebuff(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   moveFourAttacksSame(move, type, state);
   state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));

   return damageDone;
}

PE_SV moveFourAttacksEffect(Move &move, const PE_MOVETYPE &type, State &state) {
   const int32_t normalAttacks = 4;
   PE_SV damageDone = 0;

   for (int32_t attack = 0; attack < normalAttacks; attack++) {
      if (type&(one<<(2*attack))) {
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), false, true, false, false);
      }
      else if (type&(one<<(2*attack + 1))) {
         damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), true, true, false, false);
      }
   }
   if (type&(one<<(2*normalAttacks))) {
      state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return damageDone;
}


PE_SV moveDoubleIfLowHealth(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      bool lowHealth = state.health(move.side()^1) < state.maxHealth(move.side()^1)/4;
      if (lowHealth) damageDone += state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   return damageDone;
}

PE_SV moveDoubleIfPoisoned(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (state.isPoisoned(move.side() ^ 1)) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      }
   }

   return damageDone;
}

PE_SV moveDoubleIfBlinded(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (state.isBlinded(move.side() ^ 1)) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      }
   }

   return damageDone;
}

PE_SV moveDoubleIfStunned(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      if (state.isStunned(move.side() ^ 1)) {
         damageDone += state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT, true, false, false);
      }
   }

   return damageDone;
}

PE_SV moveIncreasingAttack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_SV adjustedHit = move.abilityValues()[0];

   int cons = state.consecutiveCurrent(move.side());
   if (cons > 1) {
      adjustedHit += (cons - 1)*move.abilityValues()[1];
   }
   if (adjustedHit > move.abilityValues()[2]) adjustedHit = move.abilityValues()[2];
   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side()^1, move.family(), adjustedHit, move.pet(), type == PE_MT_CRIT, true, false, false);
   }

   return damageDone;
}

PE_SV movePushBuffMulti(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
      state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return PE_OK;
}

PE_SV moveHealMultiLast(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
   }
   else if (onRound == PE_ZERO_ROUND) {
      healingDone = moveSingleHeal(move, type, state);
   }

   return healingDone;
}


PE_SV moveConsumeAlly(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone = 0, heal = state.maxHealth(move.side())/2;

   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      if (pet != state.activePet(move.side()) && state.allHealth(move.side(), pet) <= 0 && !state.isConsumed(move.side(), pet)) {
         healingDone = state.directIncreaseHealth(move.side(), heal);
         state.consume(move.side(), pet);
         break;
      }
   }

   return healingDone;
}


// this move apparently triggers things like minefield in game.  this is stupid and im not going to do it that way
PE_SV moveSearMagic(Move &move, const PE_MOVETYPE &type, State &state) {

   state.clearDebuffsActive(move.side());

   return 0;
}

PE_SV moveImmuneEmerge(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_ONE_ROUND);
      state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
   }
   else {
      // submerged and burrows pets can hit other submerged and burrowed pets
      damageDone += moveHighLowAttack(move, type, state);
      state.removeDebuffActive(move.side(), move.debuffType()[0], move.debuff()[0].subType(), PE_NO_TRACK);
   }

   return damageDone;
}

PE_SV moveImmune2Emerge(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
      state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
      state.removeDebuffActive(move.side(), move.debuffType()[0], PE_DB_IMMUNE_BREAK_UNDERGROUND, PE_NO_TRACK);
   }
   // submerged and burrows pets can hit other submerged and burrowed pets
   damageDone += moveTwoAttacksSame(move, type, state);
   if (onRound == PE_ZERO_ROUND) {
      state.removeDebuffActive(move.side(), move.debuffType()[0], move.debuff()[0].subType(), PE_NO_TRACK);
   }

   return damageDone;
}

PE_SV moveHealKillAttackTwoRound(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_ONE_ROUND);
   }
   damageDone = moveHealKillAttack(move, type, state);

   return damageDone;
}

PE_SV moveAttackAddedEffect(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT || type == PE_MT_CRIT_EFFECT, true, false, false);
   }
   if (damageDone > 0 && type > PE_MT_NORMAL_ATTACKS) {
      state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return damageDone;
}

PE_SV moveAttackHLAddedEffect(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      if (type == PE_MT_HITLOW || type == PE_MT_HITLOW_EFFECT) {
         damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0] - move.abilityValues()[1], move.pet(), false, true, false, false);
      }
      else if (type == PE_MT_HITHIGH || type == PE_MT_HITHIGH_EFFECT) {
         damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0] + move.abilityValues()[1], move.pet(), false, true, false, false);
      }
      else {
         damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0] + move.abilityValues()[1], move.pet(), true, true, false, false);
      }
   }
   if (damageDone > 0 && type > PE_MT_NORMAL_ATTACKS) {
      state.pushDebuffActive(move.side() ^ 1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return damageDone;
}

PE_SV moveIncreasingAttackAddedEffect(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_SV adjustedHit = move.abilityValues()[0];

   int cons = state.consecutiveCurrent(move.side());
   if (cons > 1) {
      adjustedHit += (cons - 1)*move.abilityValues()[1];
   }
   if (adjustedHit > move.abilityValues()[2]) adjustedHit = move.abilityValues()[2];
   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side()^1, move.family(), adjustedHit, move.pet(), type == PE_MT_CRIT || type == PE_MT_CRIT_EFFECT, true, false, false);
   }
   if (damageDone > 0 && type > PE_MT_NORMAL_ATTACKS) {
      state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return damageDone;
}

PE_SV moveAttackAddedEffectIfFaster(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
   }
   else {
      damageDone = state.doHitActive(move.side()^1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT || type == PE_MT_CRIT_EFFECT, true, false, false);
      if (damageDone > 0 && state.isSpeedWinner(move.side()) && type > PE_MT_NORMAL_ATTACKS) {
         state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
      }
   }

   return damageDone;
}

PE_SV moveAttackHealIfMiss(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   if (type == PE_MT_MISS) {
      state.doHealActive(move.side(), move.abilityValues()[1], true);
   }
   else {
      damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT || type == PE_MT_CRIT_EFFECT, true, false, false);
   }

   return damageDone;
}

PE_SV moveAttackAddedSelfDebuffIfMiss(Move &move, const PE_MOVETYPE &type, State &state) {
	PE_SV damageDone = 0;

	if (type == PE_MT_MISS) {
		state.pushDebuffActive(move.side(), move.debuffType()[0], Debuff(move.debuff()[0]));
	}
	else {
		damageDone = state.doHitActive(move.side() ^ 1, move.family(), move.abilityValues()[0], move.pet(), type == PE_MT_CRIT || type == PE_MT_CRIT_EFFECT, true, false, false);
	}

	return damageDone;
}

PE_SV moveMultiTwoAttacks(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_ONE_ROUND);
   }
   damageDone = moveSingleAttack(move, type, state);

   return damageDone;
}

PE_SV moveMultiThreeAttacks(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
   }
   damageDone = moveSingleAttack(move, type, state);

   return damageDone;
}

PE_SV moveMultiThreeQuake(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_DURATION onRound = state.multi(move.side());
   int activePet = state.activePet(move.side() ^ 1);
   static int32_t round;

   if (onRound < PE_ZERO_ROUND) {
      vector<int> debuffValues;

      state.setMulti(move.side(), PE_TWO_ROUNDS);
      state.pushDebuffActive(move.side(), PE_DB_IMMUNITY, Debuff(PE_DB_IMMUNE_BREAK_UNDERGROUND, PE_TWO_ROUNDS, false, move.side(), move.pet(), PE_IB_ALL, debuffValues));
      round = 0;
   }
   damageDone += state.doHit(move.side() ^ 1, activePet, move.family(), move.abilityValues()[round], move.pet(), type == PE_MT_CRIT, true, false, false);
   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      if (pet != activePet) {
         damageDone += state.doHit(move.side() ^ 1, pet, move.family(), move.abilityValues()[round] / 2, move.pet(), type == PE_MT_CRIT, true, false, false);
      }
   }
   round++;

   return damageDone;
}

PE_SV moveMultiThreeTantrum(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      vector<int> debuffValues;

      state.setMulti(move.side(), PE_TWO_ROUNDS);
      state.pushDebuffActive(move.side(), PE_DB_IMMUNITY, Debuff(PE_DB_IMMUNE_BREAK_UNDERGROUND, PE_TWO_ROUNDS, false, move.side(), move.pet(), PE_IB_ALL, debuffValues));
   }
   damageDone = moveTeamAttackHalf(move, type, state);

   return damageDone;
}

PE_SV moveMultiTwoDownBack(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_ONE_ROUND);
   }
   else {
      damageDone = moveSingleAttack(move, type, state);
   }

   return damageDone;
}

PE_SV moveMultiThreeUpFront(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone =0;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
      damageDone = moveSingleAttack(move, type, state);
   }

   return damageDone;
}

PE_SV moveMultiThreeUpFrontConditional(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
      damageDone = moveTwoAttacksAsOne(move, type, state);
   }

   return damageDone;
}

PE_SV moveMultiThreeAttacksEffect(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
   }
   damageDone = moveThreeAttacksSame(move, type, state);
   if (damageDone > 0) {
      state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return damageDone;
}

PE_SV moveTwoRoundThreeAttacksEffect(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_ONE_ROUND);
   }
   damageDone = moveThreeAttacksSame(move, type, state);
   if (damageDone > 0) {
      state.pushDebuffActive(move.side()^1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return damageDone;
}

PE_SV moveOneRoundThreeAttacksEffect(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;

   damageDone = moveThreeAttacksSame(move, type, state);
   if (damageDone > 0) {
      state.pushDebuffActive(move.side() ^ 1, move.debuffType()[0], Debuff(move.debuff()[0]));
   }

   return damageDone;
}


PE_SV moveMultiThreeAttacksArcane(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
   }
   damageDone = moveTwoAttacksAsOne(move, type, state);

   return damageDone;
}

PE_SV moveMultiHLThreeAttacksArcane(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;
   PE_DURATION onRound = state.multi(move.side());

   if (onRound < PE_ZERO_ROUND) {
      state.setMulti(move.side(), PE_TWO_ROUNDS);
   }
   damageDone = moveHLTwoAttacksAsOne(move, type, state);

   return damageDone;
}


PE_SV movePlant(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone;

   if (state.isBuffed(move.side(), move.debuffType()[0], move.debuff()[0].subType())) {
      int rounds = PE_LONG_COOLDOWN - state.getDuration(move.side(), move.debuffType()[0], move.debuff()[0].subType());
      healingDone = state.doHealActive(move.side(), rounds*move.abilityValues()[0], true);
      state.removeDebuffActive(move.side(), move.debuffType()[0], move.debuff()[0].subType(), PE_NO_TRACK);
   }
   else {
      state.pushDebuffActive(move.side(), move.debuffType()[0], move.debuff()[0]);
   }

   return healingDone;
}

// weather moves

PE_SV moveArcaneStorm(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveTeamAttackSame(move, type, state);

   state.setWeather(PE_WEATHER_ARCANE, 0); 

   return damageDone;
}

PE_SV moveCallBlizzard(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveSingleAttack(move, type, state);

   state.setWeather(PE_WEATHER_BLIZZARD, 0); 

   return damageDone;
}

PE_SV moveCallDarkness(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveSingleAttack(move, type, state);

   state.setWeather(PE_WEATHER_DARKNESS, 0); 

   return damageDone;
}

PE_SV moveCallLightning(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveSingleAttack(move, type, state);

   state.setWeather(PE_WEATHER_LIGHTNING_STORM, move.abilityValues()[1]); 

   return damageDone;
}

PE_SV moveMoonfire(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveSingleAttack(move, type, state);

   state.setWeather(PE_WEATHER_MOONLIGHT, 0); 

   return damageDone;
}

PE_SV moveStarfall(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone, healingDone = 0;

   damageDone = moveTeamAttackSame(move, type, state);
   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      healingDone += state.doHeal(move.side(), pet, move.abilityValues()[1], true);
   }

   return damageDone;
}

PE_SV moveMudslide(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveSingleAttack(move, type, state);

   state.setWeather(PE_WEATHER_MUDSLIDE, 0); 

   return damageDone;
}

PE_SV moveCleansingRain(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV healingDone;

   healingDone = moveHealAll(move, type, state);
   state.setWeather(PE_WEATHER_RAIN, 0); 

   return healingDone;
}

PE_SV moveAcidRain(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone;

   damageDone = moveTeamAttackSame(move, type, state);
   state.setWeather(PE_WEATHER_RAIN, 0);

   return damageDone;
}

PE_SV moveSandstorm(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveSingleAttack(move, type, state);

   state.setWeather(PE_WEATHER_SANDSTORM, move.abilityValues()[1]); 

   return damageDone;
}

PE_SV moveScorchedEarth(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveSingleAttack(move, type, state);

   state.setWeather(PE_WEATHER_SCORCHED_EARTH, move.abilityValues()[1]); 

   return damageDone;
}

PE_SV moveIlluminate(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveSingleAttack(move, type, state);

   state.setWeather(PE_WEATHER_SUNLIGHT, 0);

   return damageDone;
}

PE_SV moveSunlight(Move &move, const PE_MOVETYPE &type, State &state) {
   PE_SV damageDone = 0;

   damageDone = moveTeamAttackSame(move, type, state);

   state.setWeather(PE_WEATHER_SUNLIGHT, 0); 

   return damageDone;
}

Move::Move(const PE_SIDE side, const PE_FAMILY family, const PE_MOVE moveType, const vector<PE_AV> &abilityValues, const PE_DURATION cooldown, const PE_PET_NUMBER pet) {

   m_side = side;
   m_pet = pet;
   m_family = family;
   m_abilityValues = abilityValues;
   m_cooldown = cooldown;
   vector<PE_SV> debuffValues;
   m_baseProb = maxProb;
   m_critProb = baseCritProb;
   m_effectProb = maxProb;
   int32_t average, difference, power;
   if (m_abilityValues.size() >= 2) {
      average = (m_abilityValues[1] + m_abilityValues[0]) >> 1;
      difference = (3 * (m_abilityValues[1] - m_abilityValues[0])) >> 3;
   }

   switch (moveType) {
   case PE_MOVE_NULL:
      m_speed = &speedSimple;
      m_branches = &branchesNull;
      m_move = &moveNull;
      break;
   case PE_MOVE_SIMPLEDAMAGE200:
      m_baseProb = 2*maxProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttack;
      break;
   case PE_MOVE_SIMPLEDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttack;
      break;
   case PE_MOVE_SIMPLEHIGHLOW100:
      m_speed = &speedSimple;
      m_branches = &branchesHighLowAttack;
      m_move = &moveHighLowAttack;
      m_abilityValues[0] = average;
      m_abilityValues[1] = difference;
      break;
   case PE_MOVE_SIMPLEDAMAGE050:
      m_baseProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttack;
      break;
   case PE_MOVE_NOBELOW1DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveNoBelow1Attack;
      break;
   case PE_MOVE_050CRITDAMAGE100:
      m_critProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveNoBelow1Attack;
      break;
   case PE_MOVE_TEAMATTACK100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack; // not really -- fix this eventually
      m_move = &moveTeamAttack;
      break;
   case PE_MOVE_TEAMATTACKSAME100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack; // not really -- fix this eventually
      m_move = &moveTeamAttackSame;
      break;
   case PE_MOVE_BONESTORM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack; // not really -- fix this eventually
      m_move = &moveTeamAttackSameHealthCost;
      m_abilityValues.push_back(static_cast<int>(PE_BONESTORM*maxEval));
      break;
   case PE_MOVE_TEAMATTACKSPLIT100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack; // not really -- fix this eventually
      m_move = &moveTeamAttackSplit;
      break;
   case PE_MOVE_HIGHLOWATTACKOBJECTCLEAR100:
      m_speed = &speedSimple;
      m_branches = &branchesHighLowAttack;
      m_move = &moveHighLowAttackObjectClear;
      break;
   case PE_MOVE_TEAMATTACKOBJECTCLEAR100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack; // not really -- fix this eventually
      m_move = &moveTeamAttackObjectClear;
      break;
   case PE_MOVE_SIMPLEDAMAGEBACK100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackBack;
      break;
   case PE_MOVE_EXTRAATTDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesTwoIfAttacked;
      m_move = &moveTwoAttacksAsOne;
      break;
   case PE_MOVE_EXTRAFASTDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesTwoIfFaster;
      m_move = &moveTwoAttacksAsOne;
      break;
   case PE_MOVE_DIFFERENTFASTDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSecondIfFaster;
      break;
   case PE_MOVE_EXTRASLOWDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesTwoIfSlower;
      m_move = &moveTwoAttacksAsOne;
      break;
   case PE_MOVE_DOUBLEBELOW25DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveDoubleIfLowHealth;
      break;
   case PE_MOVE_DOUBLEPOISONEDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveDoubleIfPoisoned;
      break;
   case PE_MOVE_DOUBLEBLINDEDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveDoubleIfBlinded;
      break;
   case PE_MOVE_DOUBLESTUNNEDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveDoubleIfStunned;
      break;
   case PE_MOVE_FRACTION010DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesTwoAttacksSecondNoCrit;
      m_abilityValues.push_back(maxEval/10);
      m_move = &moveTwoAttackFraction;
      break;
   case PE_MOVE_DRAINBLOOD:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveDrainBlood;
      m_abilityValues.push_back(static_cast<int>(maxEval / PE_DRAIN_BLOOD1));
      m_abilityValues.push_back(static_cast<int>(PE_DRAIN_BLOOD2*maxEval));
      break;
   case PE_MOVE_EXTRACHILLEDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesTwoIfChilled;
      m_move = &moveTwoAttacksAsOne;
      break;
   case PE_MOVE_EXTRABLEEDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesTwoIfBleeding;
      m_move = &moveTwoAttacksAsOne;
      break;
   case PE_MOVE_EXTRABURNDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesTwoIfBurning;
      m_move = &moveTwoAttacksAsOne;
      break;
   case PE_MOVE_LASTHITDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveLastHit;
      break;
   case PE_MOVE_BLEED100DAMAGE050:
      m_baseProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesBleeding;
      m_move = &moveSingleAttack;
      break;
   case PE_MOVE_BLIND100DAMAGE050:
      m_baseProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesBlinded;
      m_move = &moveSingleAttack;
      break;
   case PE_MOVE_MOONLIGHT100DAMAGE080:
      m_baseProb = maxProb - 4*fiveProb;
      m_speed = &speedSimple;
      m_branches = &branchesMoonlight;
      m_move = &moveSingleAttack;
      break;
   case PE_MOVE_EXTRALOWHDAMAGE100: // to do properly, check for lower health after first part of hit
      m_speed = &speedSimple;
      m_branches = &branchesTwoIfLowerHealth;
      m_move = &moveTwoAttacksAsOne;
      break;
   case PE_MOVE_EXTRAARCANEDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesTwoIfArcane;
      m_move = &moveTwoAttacksAsOne;
      break;
   case PE_MOVE_EXTRAFASTONETWO100:
      m_speed = &speedSimple;
      m_branches = &branchesThreeIfFaster;
      m_move = &moveThreeAttacksSame;
      break;
   case PE_MOVE_ONETOTHREEDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesOnetoThreeAttacks;
      m_move = &moveThreeAttacksSame;
      break;
   case PE_MOVE_INCREASINGDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveIncreasingAttack;
      break;
   case PE_MOVE_HEAL100DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveHealAttack;
      break;
   case PE_MOVE_HEAL050DAMAGE100:
      m_effectProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveHealAttack;
      break;
   case PE_MOVE_HEALDOUBLEWEBDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveHealDoulbeWebAttack;
      break;
   case PE_MOVE_DOUBLEWEBSPLIT2DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesTwoFourIfWebbed;
      m_abilityValues[0] /= 2;
      m_move = &moveFourAttacksSame;
      break;
   case PE_MOVE_HEALKILLDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveHealKillAttack;
      break;
   case PE_MOVE_FASTDAMAGE100:
      m_speed = &speedFast;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttack;
      break;
   case PE_MOVE_FASTDAMAGE085:
      m_baseProb = maxProb - 3*fiveProb;
      m_speed = &speedFast;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttack;
      break;
   case PE_MOVE_FIRSTBLIZZARDDAMAGE050:
      m_baseProb = fiftyProb;
      m_speed = &speedFastBlizzard;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttack;
      break;
   case PE_MOVE_SWITCHNEXTDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackSwitchNext;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_GOT_SWITCHED, PE_ZERO_ROUND, false, side, pet, PE_IB_ALL, debuffValues));
      break;
   case PE_MOVE_SWITCHHIGHESTDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackSwitchHighest;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_GOT_SWITCHED, PE_ZERO_ROUND, false, side, pet, PE_IB_ALL, debuffValues));
      break;
   case PE_MOVE_SWITCHLOWESTDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackSwitchLowest;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_GOT_SWITCHED, PE_ZERO_ROUND, false, side, pet, PE_IB_ALL, debuffValues));
      break;
   case PE_MOVE_HIBERNATE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &movePushBuffMulti;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_abilityValues[0]);
      debuffValues.push_back(m_abilityValues[1]);
      debuffValues.push_back(m_abilityValues[2]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_HIBERNATE, PE_TWO_ROUNDS, false, side, pet, PE_IB_ALL, debuffValues));
      break;
   case PE_MOVE_REPAIR:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveHealMultiLast;
      break;
   case PE_MOVE_CONSUME_CORPSE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveConsumeAlly;
      break;
   case PE_MOVE_GOLOWDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesMultiAttackTwoHLLast;
      m_move = &moveImmuneEmerge;
      m_abilityValues[0] = average;
      m_abilityValues[1] = difference;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_UNDERGROUND, PE_ONE_ROUND, false, side, pet, PE_IB_ALL, debuffValues));
      break;
   case PE_MOVE_SUBMERGEDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesMultiAttackTwoHLLast;
      m_move = &moveImmuneEmerge;
      m_abilityValues[0] = average;
      m_abilityValues[1] = difference;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_SUBMERGED, PE_ONE_ROUND, false, side, pet, PE_IB_ALL, debuffValues));
      break;
   case PE_MOVE_GOHIGHDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesMultiAttackTwoHLLast;
      m_move = &moveImmuneEmerge;
      m_abilityValues[0] = average;
      m_abilityValues[1] = difference;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_FLYING, PE_ONE_ROUND, false, side, pet, PE_IB_ALL, debuffValues));
      break;
   case PE_MOVE_SUBMERGE2SUNS:
      m_speed = &speedSimple;
      m_branches = &branchesTwoAttacks;
      m_move = &moveImmune2Emerge;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_SUBMERGED, PE_TWO_ROUNDS, false, side, pet, PE_IB_ALL, debuffValues));
      break;
   case PE_MOVE_HEALKILL2ROUNDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveHealKillAttackTwoRound;
      break;
   case PE_MOVE_DECSPD425DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(-maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_REDUCTION, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DECSPD225DAMAGEHL100:
      m_speed = &speedSimple;
      m_branches = &branchesHighLowAttack;
      m_move = &moveHighLowAttackApplyDebuff;
      m_abilityValues[0] = average;
      m_abilityValues[1] = difference;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(-maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_REDUCTION, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DECSPD225DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(-maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_REDUCTION, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DECSPD125DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(-maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_REDUCTION, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveSingleAttackApplyDebuff;
      break;
   case PE_MOVE_QUICKSAND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(-maxEval/2);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_QUICKSAND, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveSingleAttackApplyDebuff;
      break;
   case PE_MOVE_MOTHBALLS:
      m_baseProb = fiftyProb;
      m_effectProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesFourAttacksEffectFiftyApprox;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(-maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_REDUCTION, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveFourAttacksEffect;
      break;
   case PE_MOVE_TEAMDECSPD425DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(-maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_REDUCTION, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveTeamAttackApplyDebuff;
      break;
   case PE_MOVE_DECACC420DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(-maxEval/5);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SLIPPERY_ICE, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveSingleAttackApplyDebuff;
      break;
   case PE_MOVE_DECACC425:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(-maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STENCH, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveSingleDebuff;
      break;
   case PE_MOVE_INEBRIATE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_INEBRIATED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_INEBRIATED, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveSingleDebuff;
      break;
   case PE_MOVE_BLINDINGPOISON:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_BLINDING_POISON*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_BLINDING_POISON, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_FLASH:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_PARTIALLY_BLINDED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_PARTIALLY_BLINDED, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DECDMG325DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_ATTACK_REDUCTION_TIME, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveSingleAttackApplyDebuff;
      break;
   case PE_MOVE_INCTARDMG325DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_WEAKENED_DEFENSES*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_WEAKENED_DEFENSES, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveSingleAttackApplyDebuff;
      break;
   case PE_MOVE_UNHOLYASCENSION:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveTeamDebuffDie;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_UNHOLY_ASCENSION*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_UNHOLY_ASCENSION, PE_NINE_ROUNDS, false, side, pet, PE_IB_ALL, debuffValues));
      break;
   case PE_MOVE_DECHEAL250DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_HEALING_REDUCTION*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_HEALING_REDUCTION, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_move = &moveSingleAttackApplyDebuff;
      break;
   case PE_MOVE_DECDMG1ATTACK100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_ATTACK_REDUCTION_HIT*maxEval));
      debuffValues.push_back(1); // one hit reduce
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_ATTACK_REDUCTION_HIT, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_2DECDMG1ATTACK100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_ATTACK_REDUCTION_HIT2*maxEval));
      debuffValues.push_back(1); // one hit reduce
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_ATTACK_REDUCTION_HIT, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STING6ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_STING, PE_SIX_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CONFUSING_STING:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_CONFUSING_STING, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ACIDIC_GOO:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveTwoDebuffs;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_ACIDIC_GOO, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_ACIDIC_GOO*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_ACIDIC_GOO, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CORROSION:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyTwoDebuffs;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_CORROSION, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(m_abilityValues[2]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_CORROSION, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CREEPING_FUNGUS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_CREEPING_FUNGUS, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SPORE_SHROOMS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_SPORE_SHROOMS, PE_TWO_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_PROWL:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveTwoBuffs;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_PROWL_ATTACK*maxEval));
      debuffValues.push_back(1); // one attack
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_PROWL, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_PROWL_SPEED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_PROWL, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_RIGHTEOUSINSPIRATION:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveRighteousInspiration;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_RIGHTEOUS_INSPIRATION_DAMAGE*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_PROWL, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_RIGHTEOUS_INSPIRATION_SPEED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_PROWL, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CRYSTAL_OVERLOAD:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveTwoBuffs;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_CRYSTALOVERLOAD_ATTACK*maxEval));
      debuffValues.push_back(1); // one attack
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_CRYSTAL_OVERLOAD, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(0); // no protection
      debuffValues.push_back(family); // family
      debuffValues.push_back(m_abilityValues[0]); // damageback
      debuffValues.push_back(1); // 1 hit aaaag!
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_CRYSTAL_OVERLOAD_HIT, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOMINANCE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_DOMINANCE_BOOST*maxEval));
      debuffValues.push_back(1); // one attack
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_DOMINANCE, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SUPERCHARGE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_SUPERCHARGE*maxEval));
      debuffValues.push_back(1); // one attack
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SUPERCHARGED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HEAL1ROUNDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveHealAttackDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_family);
      debuffValues.push_back(m_abilityValues[2]);
      debuffValues.push_back(m_abilityValues[2]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_LEECH_SEED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_PHEROMONES:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveHealAttackDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_PHEROMONES, PE_THREE_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SIPHONLIFE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_family);
      debuffValues.push_back(m_abilityValues[0]);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_SIPHON_LIFE, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOTEL5DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_ELEMENTAL);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_POISONED, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOTEL4DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_ELEMENTAL);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_POISONED, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOTEL3DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_ELEMENTAL);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_POISONED, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOT2WINDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_WILDWINDS, PE_TWO_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOT3OOZEDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
 	   m_debuff.push_back(Debuff(PE_DB_OVERTIME_CREEPING_OOZE, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOT4ACIDDAMAGE100:
	   m_speed = &speedSimple;
	   m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
	   debuffValues.push_back(family);
	   debuffValues.push_back(m_abilityValues[1]);
	   m_debuff.push_back(Debuff(PE_DB_OVERTIME_ACID_TOUCH, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
	   break;
   case PE_MOVE_TOXICSMOKE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_TOXIC_SMOKE, PE_TWO_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HAUNT:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveHaunt;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_HAUNT, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOT9DEATH:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_DEATH_AND_DECAY, PE_NINE_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_AGONY:
	   m_speed = &speedSimple;
	   m_branches = &branchesSingleBuff;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
	   debuffValues.push_back(family);
	   debuffValues.push_back(m_abilityValues[0]);
	   debuffValues.push_back(m_abilityValues[0]);
	   m_debuff.push_back(Debuff(PE_DB_OVERTIME_AGONY, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
	   break;
   case PE_MOVE_DOT4STICKDAMAGE080:
      m_baseProb = maxProb - 4*fiveProb;
      m_speed = &speedSimple;
      m_branches = &branchesThreeAttacks;
      m_move = &moveThreeAttacksSameDebuff;
      m_abilityValues[0] = abilityValues[0] / 3;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_NUT_BARRAGE, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOT3STICKDAMAGE050:
      m_baseProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesFourAttacks;
      m_move = &moveFourAttacksSameDebuff;
      m_abilityValues[0] = abilityValues[0] / 4;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_ROCK_BARRAGE, PE_THREE_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOT2STICKDAMAGE050:
      m_baseProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesFourAttacks;
      m_move = &moveFourAttacksSameDebuff;
      m_abilityValues[0] = abilityValues[0] / 4;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_BANANA_BARRAGE, PE_TWO_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOT1STICKDAMAGE050:
      m_baseProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesFourAttacks;
      m_move = &moveFourAttacksSameDebuff;
      m_abilityValues[0] = abilityValues[0] / 4;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_BANANA_BARRAGE, PE_ONE_ROUND, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOTBLEED5DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_BEAST);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_BLEED, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOTBLEED4DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_BEAST);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_BLEED, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOTBURN2DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_ELEMENTAL);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_FLAMETHROWER, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOTBURN4DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_DRAGONKIN);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_FLAMEBREATH, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOTBURNI4DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_ELEMENTAL);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_IMMOLATE, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DOTBURNI24DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_ELEMENTAL);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_FEL_IMMOLATE, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_IMMOLATION:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(PE_FAMILY_ELEMENTAL);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_IMMOLATION, PE_NINE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_GLOWINGTOXIN:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(static_cast<int>(PE_GLOWING_TOXIN*maxEval));
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_GLOWING_TOXIN, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CORPSEEXPLOSION:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleHealthAttackApplyDebuffDie;
      m_abilityValues.push_back(static_cast<int>(PE_CORPSE_EXPLOSION_BOOM*maxEval));
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(static_cast<int>(PE_CORPSE_EXPLOSION*maxEval));
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_CORPSE_EXPLOSION, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_EXPLODE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleHealthAttackDie;
      m_abilityValues.push_back(static_cast<int>(PE_EXPLODE*maxEval));
      break;
   case PE_MOVE_ARMAGEDDON:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveTeamAttackDie;
      break;
   case PE_MOVE_ROOTED2DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_ROOTED, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_WEBBED2DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_WEBBED, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ROOTED3DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_ROOTED, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SKUNKYBREW:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SKUNKY_BREW, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ROOTED5DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STICKY_GOO, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BRITTLEDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(0);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_BRITTLE_WEBBING, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCSPD100DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(maxEval);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_BOOST, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCSPD2100DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(maxEval);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_BOOST, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CENTRIFUGALHOOKS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(maxEval);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_CENTRIFUGALHOOKS, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCSPD375DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(3*maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_ADRENALINE, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCSPD450DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(maxEval/2);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_BOOST, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCSPD020INCREASINGDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(maxEval/5);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SPEED_BOOST, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_TRANSFORM125INCREASING100:
      m_effectProb = 5*fiveProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttackAddedEffect;
      m_move = &moveIncreasingAttackAddedEffect;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_POLYMORPHED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SLEEPING125INCREASING100:
      m_effectProb = 5*fiveProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttackAddedEffect;
      m_move = &moveIncreasingAttackAddedEffect;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_ASLEEP, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STUNFASTER050DAMAGE100:
      m_effectProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttackAddedEffect;
      m_move = &moveAttackAddedEffectIfFaster;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_ZERO_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STUNMISSDAMAGE050:
	   m_baseProb = fiftyProb;
	   m_speed = &speedSimple;
	   m_branches = &branchesSingleAttack;
	   m_move = &moveAttackAddedSelfDebuffIfMiss;
	   m_debuffType.push_back(PE_DB_MOBILITY);
	   m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
	   break;
   case PE_MOVE_HEALMISSDAMAGE050:
      m_baseProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveAttackHealIfMiss;
      break;
   case PE_MOVE_STUNSELFDAMAGE100:
      m_baseProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STUNNED25DARKDAMAGE100:
      m_effectProb = 5*fiveProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttackAddedEffectDark;
      m_move = &moveAttackAddedEffect;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STUNNEDBLINDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttackAddedEffectBlind;
      m_move = &moveAttackAddedEffect;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STUNNED25CHILLHLDAMAGE100:
      m_effectProb = 5*fiveProb;
      m_speed = &speedSimple;
      m_branches = &branchesHighLowAttackAddedEffectChill;
      m_move = &moveAttackHLAddedEffect;
      m_abilityValues[0] = average;
      m_abilityValues[1] = difference;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STUNDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttackAddedEffect;
      m_move = &moveAttackAddedEffect;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_ZERO_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_FLAMEJET:
      m_effectProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttackAddedEffect;
      m_move = &moveAttackAddedEffect;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_FLAME_JET, PE_THREE_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SLEEPED25HIGHLOW100:
      m_effectProb = 5*fiveProb;
      m_speed = &speedSimple;
      m_branches = &branchesHLAttackAddedEffect;
      m_move = &moveAttackHLAddedEffect;
      m_abilityValues[0] = average;
      m_abilityValues[1] = difference;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_ASLEEP, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STUNNED25DAMAGE100:
      m_effectProb = 5 * fiveProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttackAddedEffect;
      m_move = &moveAttackAddedEffect;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BLINDED50DAMAGE100:
      m_effectProb = fiftyProb;
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttackAddedEffect;
      m_move = &moveAttackAddedEffect;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_PARTIALLY_BLINDED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_PARTIALLY_BLINDED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BLINDED2100DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_PARTIALLY_BLINDED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_PARTIALLY_BLINDED, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_THREEATTACKSDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesThreeAttacks;
      m_move = &moveThreeAttacksSame;
      m_abilityValues[0] /= 3;
      break;
   case PE_MOVE_3ROUNDINCDMG2ROUND100DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesThreeAttacks;
      m_move = &moveMultiThreeAttacksEffect;
      m_abilityValues[0] /= 3;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_SHATTERED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SHATTERED_DEFENSES, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_2ROUNDINCDMG1ROUND100DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesThreeAttacks;
      m_move = &moveTwoRoundThreeAttacksEffect;
      m_abilityValues[0] /= 3;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_SHATTERED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SHATTERED_DEFENSES, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_1ROUNDINCDMG1ROUND100DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesThreeAttacks;
      m_move = &moveOneRoundThreeAttacksEffect;
      m_abilityValues[0] /= 3;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_SHATTERED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SHATTERED_DEFENSES, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_2ROUNDATTACK100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveMultiTwoAttacks;
      break;
   case PE_MOVE_3ROUNDATTACK100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveMultiThreeAttacks;
      break;
   case PE_MOVE_EXTRARAIN3ROUNDTEAMATTACK100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveMultiTeamAttackSameExtraRain;
      break;
   case PE_MOVE_QUAKE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveMultiThreeQuake;
      power = static_cast<int32_t>(static_cast<double>(m_abilityValues[0] - 12) / 0.6);
      m_abilityValues.push_back(m_abilityValues[0] + 2 + power / 10);
      m_abilityValues.push_back(m_abilityValues[0] + 4 + power / 5);
      break;
   case PE_MOVE_TYMPANICTANTRUM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveMultiThreeTantrum;
      break;
   case PE_MOVE_RECHARGE2DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesMultiAttackThreeFirst;
      m_move = &moveMultiThreeUpFront;
      break;
   case PE_MOVE_BUILDUP1DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesMultiAttackTwoLast;
      m_move = &moveMultiTwoDownBack;
      break;
   case PE_MOVE_EXTRASUNRECHARGE2DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesMultiAttackThreeFirstSun;
      m_move = &moveMultiThreeUpFrontConditional;
      break;
   case PE_MOVE_EXTRAARCANE3ROUNDDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesHLTwoIfArcane;
      m_abilityValues[0] = average;
      m_abilityValues[1] = difference;
      m_move = &moveMultiHLThreeAttacksArcane;
      break;
   case PE_MOVE_SIMPLEHEAL:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleHeal;
      break;
   case PE_MOVE_HEALEQUALIZE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveHealEqualize;
      break;
   case PE_MOVE_HEALFRACTION025:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_abilityValues.push_back(maxEval/4);
      m_move = &moveHealFraction;
      break;
   case PE_MOVE_HEALFRACTION025FIRST:
      m_speed = &speedFast;
      m_branches = &branchesSingleBuff;
      m_abilityValues.push_back(maxEval / 4);
      m_move = &moveHealFraction;
      break;
   case PE_MOVE_HEALHALFHIT:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveHealHalfHit;
      break;
   case PE_MOVE_HEALALL:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveHealAll;
      break;
   case PE_MOVE_HEALOTHERS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveHealOthers;
      break;
   case PE_MOVE_HEALALLAQUATIC:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveHealAllAquatic;
      break;
   case PE_MOVE_HEALSELFALLMECHANICAL:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveHealSelfAllMechanical;
      break;
   case PE_MOVE_AUTUMNBREEZE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveHealAllApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_AUTUMN_BREEZE*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_AUTUMN_BREEZE, PE_TWO_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CLEARUSERDEBUFFS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveBuffDebuffClear;
      break;
   case PE_MOVE_CLEARTEAMDEBUFFS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveTeamBuffDebuffClear;
      break;
   case PE_MOVE_DECCRIT4ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_CROAK*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_CROAK, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCCRIT425:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_HAWK_EYE*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_HAWK_EYE, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BONKERS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_BONKERS*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_BONKERS, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCCRIT250:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_CAW*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_CAW, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ADRENALGLANDS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_ADRENAL_GLANDS*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_ADRENAL_GLANDS, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCHIT425:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_ACCURACY*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_ACCURACY, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_NIMBUS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_NIMBUS*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_NIMBUS, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ANZU:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveTwoBuffs;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_ACCURACY*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_ACCURACY, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.clear();
      debuffValues.push_back(static_cast<int>(PE_ATTACK_BOOST*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_ATTACK_BOOST, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCHIT5100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_FOCUSED, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_UNCANNYLUCK:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveTwoBuffs;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_UNCANNY_LUCK_CRIT*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_UNCANNYLUCK_CRIT, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_UNCANNY_LUCK_HIT*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_UNCANNYLUCK_HIT, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_RAINDANCE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveHealTwoBuffs;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_RAIN_DANCE_CRIT*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_UNCANNYLUCK_CRIT, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_RAIN_DANCE_HIT*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_UNCANNYLUCK_HIT, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STIMPACK:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveTwoBuffs;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(maxEval/2));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STIMPACK_CRIT, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(-maxEval/4));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STIMPACK_HIT, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CELESTIALBLESSING:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SANCTIFIED_GROUND, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DMGACTIVE5ROUND100_SWARM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_SWARM_OF_FLIES, PE_FIVE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_WHIRLPOOL:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_WHIRLPOOL, PE_TWO_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_EXPLOSIVEBREW:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_EXPLOSIVE_BREW, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STICKYGRENADE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_STICKY_GRENADE, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ENTANGLINGROOTS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_ENTANGLINGROOTS, PE_ONE_ROUND, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_MAGMATRAP:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_MAGMA_TRAP, PE_NINE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_SNAPTRAP:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_SNAP_TRAP, PE_NINE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_GEYSER:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_GEYSER, PE_THREE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_ICETOMB:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_ICETOMB, PE_THREE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_ELEMENTIUMBOLT:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_ELEMENTIUMBOLT, PE_THREE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_CURSEOFDOOM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_CURSE_OF_DOOM, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BOMBINGRUN:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_BOMB, PE_THREE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_STUNSEED:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_ELEMENTIUMBOLT, PE_THREE_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CHEW:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_CHEW, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_PLANT:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &movePlant;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_PLANTED, PE_LONG_COOLDOWN, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CYCLONE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveTeamDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_CYCLONE, PE_FIVE_ROUNDS, false, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_BUILDTURRET:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleBuffOverDump;
      m_debuffType.push_back(PE_DB_OVERTIME);
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]/3);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_TURRET1, PE_FOUR_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_TURRET2, PE_FOUR_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_XE321BOOMBOT:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_XE321_BOOMBOT, PE_THREE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_VOLCANO:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_VOLCANO, PE_THREE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_BOOBYTRAPPEDPRESENTS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveTwoDebuffs;
      m_debuffType.push_back(PE_DB_OVERTIME);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_BOOBY_TRAPPED_PRESENTS, PE_NINE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_BOOBY_TRAPPED_PRESENTS, PE_NINE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_MINEFIELD:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveTwoDebuffs;
      m_debuffType.push_back(PE_DB_OVERTIME);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_MINEFIELD, PE_NINE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_MINEFIELD, PE_NINE_ROUNDS, true, side, pet, PE_IB_LOCATION, debuffValues));
      break;
   case PE_MOVE_HEAL1ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_WISH, PE_ONE_ROUND, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HEALINCMAXHEALTH9ROUNDS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleHealBuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_HEALTHY, PE_NINE_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_FADING:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_FADING, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DODGE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_DODGE, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DEFLECTION:
      m_speed = &speedFast;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_DEFLECTION, PE_ZERO_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DECDMG2ROUND050:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_CROUCHED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_CROUCH, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_2DECDMG2ROUND050:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_STAGGERED_STEPS*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_STAGGERED_STEPS, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_EXTRAPLATING:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_EXTRA_PLATING*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_EXTRA_PLATING, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DECDMG3ROUND050:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_TOUGH_CUDDLY*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_TOUGH_CUDDLY, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCDMG2ROUND100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_SHATTERED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SHATTERED_DEFENSES, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCDMG325DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_ATTACK_BOOST*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_ATTACK_BOOST, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCDMGTAK25HLDAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesHighLowAttack;
      m_move = &moveHighLowAttackApplyBuff;
      m_abilityValues[0] = average;
      m_abilityValues[1] = difference;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_RECKLESS*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_RECKLESS_STRIKE, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCDMG2ROUND050:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_AMPLIFY_BOOST*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_AMPLIFY_MAGIC, PE_TWO_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INNERVISION:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_AMPLIFY_BOOST*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_INNER_VISION, PE_ONE_ROUND, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCDMG1HIT1ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_FOCUS_CHI_BOOST*maxEval));
      debuffValues.push_back(1); // one attack
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_FOCUS_CHI, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCDMG325INCDMG350DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyTwoDebuffs;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_RABIES_GIVEN*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_RABIES_GIVEN, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_RABIES_TAKEN*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_RABIES_TAKEN, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_COINTOSS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyTwoDebuffs;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_COIN_TOSS_GIVEN*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_COIN_TOSS_GIVEN, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_COIN_TOSS_TAKEN*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_COIN_TOSS_TAKEN, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DECDMG225INCDMG225DAMAGE100:
	   m_speed = &speedSimple;
	   m_branches = &branchesSingleAttack;
	   m_move = &moveSingleAttackApplyBuffDebuff;
	   m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_ATTACK_BOOST*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_ATTACK_BOOST, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
	   debuffValues.clear();
	   m_debuffType.push_back(PE_DB_DAMAGED);
	   debuffValues.push_back(static_cast<int>(-maxEval / 4));
	   m_debuff.push_back(Debuff(PE_DB_DAMAGED_ATTACK_REDUCTION_TIME, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
	   break;
   case PE_MOVE_DECSPD150DECDMG150DAMAGE100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyTwoDebuffs;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_WEAKNESS*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_WEAKNESS_DAMAGE, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      debuffValues.clear();
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(-maxEval/2));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_WEAKNESS_SPEED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HEARTBROKEN:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_OVERLOAD*maxEval));
      debuffValues.push_back(1); // one attack
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_OVERLOAD, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BUFFNDUMP100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuffDump;
      m_move = &moveSingleBuffDump;
      m_debuffType.push_back(PE_DB_DAMAGED);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_BARREL_READY, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_PUMPNDUMP100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuffDump;
      m_move = &moveSingleBuffDump;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_PUMPED*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_PUMPED, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_WINDUPNDUMP100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuffDump;
      m_move = &moveSingleBuffDump;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(static_cast<int>(PE_WINDUP*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_WINDUP, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_LAUCHROCKET:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuffDump;
      m_move = &moveSingleBuffDump;
      m_debuffType.push_back(PE_DB_OVERTIME);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_ROCKET, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DEBUFFNDUMP100:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuffDump;
      m_move = &moveSingleDebuffDump;
      m_debuffType.push_back(PE_DB_DAMAGED);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_LOCKED_ON, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_GRAVITY:
      m_speed = &speedSimple;
      m_branches = &branchesGravity;
      m_move = &moveGravity;
      m_debuffType.push_back(PE_DB_DAMAGED);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_GRAVITY, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_LEVITATED, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SOOTHE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_SOOTHED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_NEVERMORE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_NEVERMORE, PE_LONG_COOLDOWN, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_CUTE_FACE:
      
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_CUTE_FACE*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_CUTE_FACE, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_FROLICK:
      
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_FROLICK*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_FROLICK, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SEARMAGIC:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSearMagic;
      break;
   case PE_MOVE_STUN:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_FOOD_COMA:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_MOBILITY);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_STUNNED, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_FEIGN_DEATH:
      m_speed = &speedFastest;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_FEIGN_DEATH, PE_ZERO_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_PORTAL:
      m_speed = &speedFastest;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_PORTAL, PE_ZERO_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_FADE:
      m_speed = &speedFastest;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_FADE, PE_ZERO_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ETHEREAL:
      m_speed = &speedFastest;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_ETHEREAL, PE_ZERO_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ENDURE1:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_SURVIVAL, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_REBIRTH1:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_DARK_REBIRTH, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BUBBLE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(2); // two blocks
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_BUBBLE, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_DECOY:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(2); // two blocks
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_DECOY, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SOULWARD:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(1); // two blocks
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_SOULWARD, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SHIELDBLOCK:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(1); // one blocks
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_BLOCK, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_COCOONSTRIKE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(1); // one blocks
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_SILK_COCOON, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_APOCALYPSE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuffBoth;
      m_debuffType.push_back(PE_DB_OVERTIME);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_APOCALYPSE, PE_FIFTEEN_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ICE_BARRIER:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(2); // two blocks
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_ICE_BARRIER, PE_LONG_COOLDOWN, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BEAVER_DAM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(2); // two blocks
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_BEAVER_DAM, PE_LONG_COOLDOWN, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_PRISMATICBARRIER:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(2); // two blocks
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_PRISMATIC_BARRIER, PE_LONG_COOLDOWN, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ILLUSIONARYBARRIER:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(2); // two blocks
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_ILLUSIONARY_BARRIER, PE_LONG_COOLDOWN, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HOT3ROUNDRM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_RENEWING_MISTS, PE_THREE_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HOT5ROUNDSM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_SOOTHING_MISTS, PE_FIVE_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HOT5ROUNDPT:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_PHOTOSYNTHESIS, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HOT5ROUNDELEMENTAL:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuffTransform;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_NATURES_WARD, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HOT2ROUNDTQ:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_TRANQUILITY, PE_TWO_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ROT:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyTwoDebuffs;
      m_debuffType.push_back(PE_DB_OVERTIME);
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_ROT, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_debuffType.push_back(PE_DB_DAMAGED);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_UNDEAD_MARK, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BLISTERINGCOLD:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_OVERTIME);
      debuffValues.push_back(family);
      debuffValues.push_back(PE_FROSTBITE);
      debuffValues.push_back(1); // first one
      m_debuff.push_back(Debuff(PE_DB_OVERTIME_BLISTERING_COLD, PE_FOUR_ROUNDS, true, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SHIELD5ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SHIELD, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_EPRESENCE5ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_EMERALD_PRESENCE, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_JADESKIN5ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_JADESKIN, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_BARKSKIN:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_BARKSKIN, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_STONESKIN5ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_STONESKIN, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_TRIHORNSHIELD3ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_TRIHORN_SHIELD, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_IRONBARK:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_IRONBARK, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ADDDAMWILD: // note this is not affected by family
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_WILDMAGIC, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ADDDAMEXPOSED: // note this is not affected by family
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_EXPOSED_WOUNDS, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ADDDAMMANGLE: // note this is not affected by family
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_MANGLE, PE_TEN_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ADDDAMCLAW: // note this is not affected by family
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_BLACK_CLAW, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_FORBODINGCURSE: // note this is not affected by family
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveTwoDebuffs;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_FORBODING_CURSE_ADDDAM, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(static_cast<int>(PE_FORBODING_CURSE*maxEval));
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_FORBODING_CURSE_SPEED, PE_FOUR_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SPIKED5ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-m_abilityValues[0]);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SPIKED, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SPIRITSPIKES:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(-m_abilityValues[0]);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[1]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SPIRIT_SPIKES, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_LIGHTNINGSHIELD2ROUND:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(0);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_LIGHTNINGSHIELD, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_THORNS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(0);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_THORNS, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_HEATUP:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(0);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_HEAT_UP, PE_THREE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SPINYCARAPACE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuffOverDump;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(0);
      debuffValues.push_back(family);
      debuffValues.push_back(m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SPINY_CARAPACE, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.clear();
      debuffValues.push_back(0);
      debuffValues.push_back(family);
      debuffValues.push_back(2*m_abilityValues[0]);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_SPINY_CARAPACE2, PE_ONE_ROUND, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_TOXICSKIN:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(0);
      debuffValues.push_back(family);
      debuffValues.push_back(static_cast<int>(PE_TOXIC_SKIN*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_TOXIC_SKIN, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_REFLECTIVESHIELD:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveSingleBuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(0);
      debuffValues.push_back(PE_FAMILY_NOFAMILY);
      debuffValues.push_back(static_cast<int>(PE_REFLECTIVE_SHIELD*maxEval));
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_REFLECTIVE_SHIELD, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_PLAGUEDBLOOD:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(PE_PLAGUED_BLOOD);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_PLAGUED_BLOOD, PE_FIVE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_TOUCHOFTHEANIMUS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSingleAttackApplyDebuff;
      m_debuffType.push_back(PE_DB_DAMAGED);
      debuffValues.push_back(PE_TOUCH_OF_THE_ANIMUS);
      m_debuff.push_back(Debuff(PE_DB_DAMAGED_TOUCH_OF_THE_ANIMUS, PE_TWO_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_INCTEAMSPD9ROUND025:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleBuffTeam;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(maxEval/4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_DAZZLING_DANCE, PE_NINE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_2INCTEAMSPD9ROUND025:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleBuffTeam;
      m_debuffType.push_back(PE_DB_MOBILITY);
      debuffValues.push_back(maxEval / 4);
      m_debuff.push_back(Debuff(PE_DB_MOBILITY_MOON_DANCE, PE_NINE_ROUNDS, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_SHIELDSTORM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleDebuff;
      m_move = &moveSingleBuffTeam;
      m_debuffType.push_back(PE_DB_IMMUNITY);
      debuffValues.push_back(1); // one block
      m_debuff.push_back(Debuff(PE_DB_IMMUNE_BLOCK, PE_LONG_COOLDOWN, false, side, pet, PE_IB_NONE, debuffValues));
      break;
   case PE_MOVE_ARCANESTORM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveArcaneStorm;
      break;
   case PE_MOVE_CALLBLIZZARD:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveCallBlizzard;
      break;
   case PE_MOVE_CALLDARKNESS:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveCallDarkness;
      break;
   case PE_MOVE_CALLLIGHTNING:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveCallLightning;
      break;
   case PE_MOVE_MOONFIRE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveMoonfire;
      break;
   case PE_MOVE_STARFALL:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveStarfall;
      break;
   case PE_MOVE_MUDSLIDE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveMudslide;
      break;
   case PE_MOVE_CLEANSINGRAIN:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveCleansingRain;
      break;
   case PE_MOVE_ACIDRAIN:
      m_speed = &speedSimple;
      m_branches = &branchesSingleBuff;
      m_move = &moveAcidRain;
      break;
   case PE_MOVE_SANDSTORM:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSandstorm;
      break;
   case PE_MOVE_SCORCHEDEARTH:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveScorchedEarth;
      break;
   case PE_MOVE_ILLUMINATE:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSunlight;
      break;
   case PE_MOVE_SUNLIGHT:
      m_speed = &speedSimple;
      m_branches = &branchesSingleAttack;
      m_move = &moveSunlight;
      break;
   default:
      break;
   }

}
