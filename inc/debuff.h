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

#ifndef DEBUFF_H
#define DEBUFF_H

#include <stdint.h>
#include "constants.h"
#include "state.h"

using namespace std;

typedef enum PE_DB_TYPE {
   PE_DB_OVERTIME,
   PE_DB_DAMAGED,
   PE_DB_MOBILITY,
   PE_DB_IMMUNITY,
   PE_DB_TYPES
};

typedef enum PE_DB_SUBTYPE {
   PE_DB_OVERTIME_DAMAGE_REGULAR_START,
   PE_DB_OVERTIME_WILDWINDS,
   PE_DB_OVERTIME_CORROSION,
   PE_DB_OVERTIME_SPORE_BURST,
   PE_DB_OVERTIME_ACID_TOUCH,
   PE_DB_OVERTIME_DEATH_AND_DECAY,
   PE_DB_OVERTIME_TOXIC_SMOKE,
   PE_DB_OVERTIME_HAUNT, // lots more properties, but is fine here
   PE_DB_OVERTIME_POISONED_START,
   PE_DB_OVERTIME_POISONED,
   PE_DB_OVERTIME_STING,  // counts as poisoned
   PE_DB_OVERTIME_ACIDIC_GOO, // counts as poisoned
   PE_DB_OVERTIME_CREEPING_OOZE, // counts as poisoned
   PE_DB_OVERTIME_CONFUSING_STING, // counts as poisoned, not completely implimented -- worst move ever
   PE_DB_OVERTIME_POISONED_END,
   PE_DB_OVERTIME_SWARM_OF_FLIES,
   PE_DB_OVERTIME_BLEED,
   PE_DB_OVERTIME_BURNING_START,
   PE_DB_OVERTIME_FLAMETHROWER, // counts as burning
   PE_DB_OVERTIME_FLAMEBREATH, // counts as burning
   PE_DB_OVERTIME_IMMOLATE, // counts as burning
   PE_DB_OVERTIME_FEL_IMMOLATE, // counts as burning
   PE_DB_OVERTIME_FLAME_JET,
   PE_DB_OVERTIME_BURNING_END,
   PE_DB_OVERTIME_DAMAGE_REGULAR_END,
   PE_DB_OVERTIME_DAMAGE_NOTDOT_START,
   PE_DB_OVERTIME_ROCK_BARRAGE,
   PE_DB_OVERTIME_BANANA_BARRAGE,
   PE_DB_OVERTIME_NUT_BARRAGE,
   PE_DB_OVERTIME_EGG_BARRAGE,
   PE_DB_OVERTIME_DAMAGE_NOTDOT_END,
   PE_DB_OVERTIME_DAMAGE_CONDITIONAL_START, // damage
   PE_DB_OVERTIME_CREEPING_FUNGUS,
   PE_DB_OVERTIME_DAMAGE_CONDITIONAL_END, // damage
   PE_DB_OVERTIME_DAMAGE_PROGRESSIVE_START,
   PE_DB_OVERTIME_AGONY,
   PE_DB_OVERTIME_DAMAGE_PROGRESSIVE_END,
   PE_DB_OVERTIME_DAMAGE_PERCENT_START,
   PE_DB_OVERTIME_GLOWING_TOXIN,
   PE_DB_OVERTIME_CORPSE_EXPLOSION,
   PE_DB_OVERTIME_DAMAGE_PERCENT_END,
   PE_DB_OVERTIME_DAMAGE_HEAL_START, // heals too (default is heals the damage)
   PE_DB_OVERTIME_DAMAGE_HEAL_FIXED_START, // heals fixed amount
   PE_DB_OVERTIME_SIPHON_LIFE,
   PE_DB_OVERTIME_DAMAGE_HEAL_FIXED_END, // heals fixed amount
   PE_DB_OVERTIME_LEECH_SEED,
   PE_DB_OVERTIME_DAMAGE_HEAL_END,
   PE_DB_OVERTIME_DAMAGE_NONACTIVE_START, // does damage to non-active pets
   PE_DB_OVERTIME_PHEROMONES,
   PE_DB_OVERTIME_DAMAGE_NONACTIVE_END, // does damage to non-active pets
   PE_DB_OVERTIME_ACCUMULATING_START,
   PE_DB_OVERTIME_FROSTBITE,
   PE_DB_OVERTIME_ACCUMULATING_END,
   PE_DB_OVERTIME_DAMAGE_END, // damage every round
   PE_DB_OVERTIME_RENEWING_MISTS,
   PE_DB_OVERTIME_SOOTHING_MISTS,
   PE_DB_OVERTIME_TRANQUILITY,
   PE_DB_OVERTIME_NATURES_WARD,
   PE_DB_OVERTIME_PROGRESSIVE_HEAL_START,
   PE_DB_OVERTIME_HIBERNATE,
   PE_DB_OVERTIME_PROGRESSIVE_HEAL_END,
   PE_DB_OVERTIME_WEATHER_HEAL_START,
   PE_DB_OVERTIME_PHOTOSYNTHESIS,
   PE_DB_OVERTIME_WEATHER_HEAL_END,
   PE_DB_OVERTIME_HEAL_END, // heal every round
   PE_DB_OVERTIME_OBJECTS_START,
   PE_DB_OVERTIME_CYCLONE,
   PE_DB_OVERTIME_TURRET1,
   PE_DB_OVERTIME_TURRET2,
   PE_DB_OVERTIME_MINEFIELD, // is also in the mobility catagory
   PE_DB_OVERTIME_BOOBY_TRAPPED_PRESENTS, // is also in the mobility catagory
   PE_DB_OVERTIME_XE321_BOOMBOT,
   PE_DB_OVERTIME_ROCKET,
   PE_DB_OVERTIME_MAGMA_TRAP,
   PE_DB_OVERTIME_SNAP_TRAP,
   PE_DB_OVERTIME_VOLCANO,
   PE_DB_OVERTIME_OBJECTS_END,
   PE_DB_OVERTIME_MISC_START,
   PE_DB_OVERTIME_IMMOLATION,
   PE_DB_OVERTIME_DARK_REBIRTH,
   PE_DB_OVERTIME_ROT,
   PE_DB_OVERTIME_BLISTERING_COLD,
   PE_DB_OVERTIME_MISC_END,
   PE_DB_OVERTIME_BEGIN_TIMERS,
   PE_DB_OVERTIME_START_BOMBS,
   PE_DB_OVERTIME_CURSE_OF_DOOM,
   PE_DB_OVERTIME_BOMB,
   PE_DB_OVERTIME_START_STICKYBOMBS,
   PE_DB_OVERTIME_STICKY_GRENADE,
   PE_DB_OVERTIME_EXPLOSIVE_BREW,
   PE_DB_OVERTIME_END_STICKYBOMBS,
   PE_DB_OVERTIME_START_STUNBOMBS,
   PE_DB_OVERTIME_ICETOMB,
   PE_DB_OVERTIME_ELEMENTIUMBOLT,
   PE_DB_OVERTIME_GEYSER,
   PE_DB_OVERTIME_STUNSEED,
   PE_DB_OVERTIME_END_STUNBOMBS,
   PE_DB_OVERTIME_START_ROOTBOMBS,
   PE_DB_OVERTIME_WHIRLPOOL,
   PE_DB_OVERTIME_ENTANGLINGROOTS,
   PE_DB_OVERTIME_END_ROOTBOMBS,
   PE_DB_OVERTIME_END_BOMBS,
   PE_DB_OVERTIME_SPORE_SHROOMS,
   PE_DB_OVERTIME_WISH,
   PE_DB_OVERTIME_FADING,
   PE_DB_OVERTIME_CHEW,
   PE_DB_OVERTIME_APOCALYPSE,
   PE_DB_OVERTIME_HEALTHY,
   PE_DB_OVERTIME_END_TIMERS, // timers must be last
   PE_DB_DAMAGED_SHATTERED_DEFENSES,
   PE_DB_DAMAGED_WEAKENED_DEFENSES,
   PE_DB_DAMAGED_RECKLESS_STRIKE,
   PE_DB_DAMAGED_ACIDIC_GOO,
   PE_DB_DAMAGED_CROUCH,
   PE_DB_DAMAGED_STAGGERED_STEPS,
   PE_DB_DAMAGED_EXTRA_PLATING,
   PE_DB_DAMAGED_TOUGH_CUDDLY,
   PE_DB_DAMAGED_RABIES_TAKEN,
   PE_DB_DAMAGED_COIN_TOSS_TAKEN,
   PE_DB_DAMAGED_CELESTIAL_BLESSING,
   PE_DB_DAMAGED_OVERLOAD,
   PE_DB_DAMAGED_UNHOLY_ASCENSION,
   PE_DB_DAMAGED_MOD_TAKEN_FACTOR_HIT_START, // has a hit timer
   PE_DB_DAMAGED_MOD_TAKEN_FACTOR_END,
   PE_DB_DAMAGED_MOD_GIVEN_FACTOR_START,
   PE_DB_DAMAGED_PUMPED,
   PE_DB_DAMAGED_WINDUP,
   PE_DB_DAMAGED_WEAKNESS_DAMAGE,
   PE_DB_DAMAGED_RABIES_GIVEN,
   PE_DB_DAMAGED_COIN_TOSS_GIVEN,
   PE_DB_DAMAGED_ATTACK_BOOST,
   PE_DB_DAMAGED_AMPLIFY_MAGIC,
   PE_DB_DAMAGED_INNER_VISION,
   PE_DB_DAMAGED_ATTACK_REDUCTION_TIME,
   PE_DB_DAMAGED_RIGHTEOUS_INSPIRATION_DAMAGE,
   PE_DB_DAMAGED_HEARTBREAK,
   PE_DB_DAMAGED_DRAGONKIN,
   PE_DB_DAMAGED_MOD_GIVEN_FACTOR_HIT_START, // has a hit timer
   PE_DB_DAMAGED_FOCUS_CHI,
   PE_DB_DAMAGED_PROWL,
   PE_DB_DAMAGED_CRYSTAL_OVERLOAD,
   PE_DB_DAMAGED_DOMINANCE,
   PE_DB_DAMAGED_SUPERCHARGED,
   PE_DB_DAMAGED_ATTACK_REDUCTION_HIT,
   PE_DB_DAMAGED_MOD_GIVEN_FACTOR_END,
   PE_DB_DAMAGED_MOD_TAKEN_ADD_START, // sign indicates addition or subtraction to damage taken
   PE_DB_DAMAGED_SHIELD,
   PE_DB_DAMAGED_EMERALD_PRESENCE,
   PE_DB_DAMAGED_JADESKIN,
   PE_DB_DAMAGED_STONESKIN,
   PE_DB_DAMAGED_TRIHORN_SHIELD,
   PE_DB_DAMAGED_IRONBARK,
   PE_DB_DAMAGED_BARKSKIN,
   PE_DB_DAMAGED_WILDMAGIC,
   PE_DB_DAMAGED_CORROSION,
   PE_DB_DAMAGED_EXPOSED_WOUNDS,
   PE_DB_DAMAGED_BLACK_CLAW,
   PE_DB_DAMAGED_FORBODING_CURSE_ADDDAM,
   PE_DB_DAMAGED_MANGLE,
   PE_DB_DAMAGED_MOD_TAKEN_DAMAGEBACK_START, // does damage to the attacking pet based on a debuff the attacked pet has second value indicates family for attack back, third value is damage back
   PE_DB_DAMAGED_SPIKED,
   PE_DB_DAMAGED_SPIRIT_SPIKES,
   PE_DB_DAMAGED_LIGHTNINGSHIELD,
   PE_DB_DAMAGED_THORNS,
   PE_DB_DAMAGED_HEAT_UP,
   PE_DB_DAMAGED_SPINY_CARAPACE,
   PE_DB_DAMAGED_SPINY_CARAPACE2,
   PE_DB_DAMAGED_MOD_TAKEN_HFACTOR_START,  // damage is a % of attacking pet's max health
   PE_DB_DAMAGED_TOXIC_SKIN,
   PE_DB_DAMAGED_MOD_TAKEN_HFACTOR_END,    
   PE_DB_DAMAGED_MOD_TAKEN_DFACTOR_START,  // damage is a % of damge done
   PE_DB_DAMAGED_REFLECTIVE_SHIELD,
   PE_DB_DAMAGED_MOD_TAKEN_DFACTOR_END,
   PE_DB_DAMAGED_MOD_TAKEN_DAMAGEBACK_END, // 
   PE_DB_DAMAGED_MOD_TAKEN_ADD_END,
   PE_DB_DAMAGED_MOD_TAKEN_HEALING_BACK_START,
   PE_DB_DAMAGED_PLAGUED_BLOOD,
   PE_DB_DAMAGED_TOUCH_OF_THE_ANIMUS,
   PE_DB_DAMAGED_MOD_TAKEN_HEALING_BACK_END,
   PE_DB_DAMAGED_MOD_GIVEN_ADD_START, // does damage to the pet doing damage based on a debuff the pet doing damage has
   PE_DB_DAMAGED_MOD_GIVEN_DAMAGEBACK,
   PE_DB_DAMAGED_BRITTLE_WEBBING,
   PE_DB_DAMAGED_MOD_GIVEN_DAMAGEBACK_HIT, // limited number of hits
   PE_DB_DAMAGED_CRYSTAL_OVERLOAD_HIT, 
   PE_DB_DAMAGED_MOD_GIVEN_ADD_END,
   PE_DB_DAMAGED_MOD_HEALING_TAKEN_FACTOR_START,
   PE_DB_DAMAGED_HEALING_REDUCTION,
   PE_DB_DAMAGED_MOD_HEALING_TAKEN_FACTOR_END,
   PE_DB_DAMAGED_LOCKED_ON,
   PE_DB_DAMAGED_BARREL_READY,
   PE_DB_DAMAGED_GRAVITY,
   PE_DB_DAMAGED_UNDEAD_MARK,
   PE_DB_DAMAGED_MECHANICAL_RES,
   PE_DB_MOBILITY_STUNNED,
   PE_DB_MOBILITY_ASLEEP,
   PE_DB_MOBILITY_POLYMORPHED, // transformed, cant move
   PE_DB_MOBILITY_GOT_SWITCHED, // forced switched this round, so can't move
   PE_DB_MOBILITY_MOVE_CONTROL_END,
   PE_DB_MOBILITY_ROOTS_START,
   PE_DB_MOBILITY_ROOTED, // cant switch pets
   PE_DB_MOBILITY_SKUNKY_BREW, // cant switch pets
   PE_DB_MOBILITY_LEVITATED, // cant switch pets
   PE_DB_MOBILITY_STICKY_GOO, // cant switch pets
   PE_DB_MOBILITY_WEBBED, // cant switch pets
   PE_DB_MOBILITY_ROOTS_END,
   PE_DB_MOBILITY_STUNS_ROOTS_END, // critters immune above this point
   PE_DB_MOBILITY_RESILIENT, // cant be disabled, put stuns, etc above this point
   PE_DB_MOBILITY_SELF_ROOT_START,
   PE_DB_MOBILITY_PLANTED, // as in from "plant"
   PE_DB_MOBILITY_SELF_ROOT_END,
   PE_DB_MOBILITY_SPEED_START,
   PE_DB_MOBILITY_SPEED_REDUCTION, // lower speed 25% (this can stack -- hiss + slither)   what to do? 
   PE_DB_MOBILITY_FORBODING_CURSE_SPEED,
   PE_DB_MOBILITY_QUICKSAND, 
   PE_DB_MOBILITY_SPEED_BOOST, // higher speed 100%
   PE_DB_MOBILITY_CENTRIFUGALHOOKS, // higher speed 100%
   PE_DB_MOBILITY_ADRENALINE, // higher speed 75%
   PE_DB_MOBILITY_DAZZLING_DANCE, // higher speed 25%
   PE_DB_MOBILITY_MOON_DANCE, // higher speed 25%
   PE_DB_MOBILITY_WEAKNESS_SPEED,
   PE_DB_MOBILITY_PROWL,
   PE_DB_MOBILITY_RIGHTEOUS_INSPIRATION_SPEED,
   PE_DB_MOBILITY_SPEED_END,
   PE_DB_MOBILITY_ACCURACY_START,
   PE_DB_MOBILITY_ACCURACY,
   PE_DB_MOBILITY_NIMBUS,
   PE_DB_MOBILITY_FOCUSED,
   PE_DB_MOBILITY_SLIPPERY_ICE,
   PE_DB_MOBILITY_AUTUMN_BREEZE,
   PE_DB_MOBILITY_STENCH,
   PE_DB_MOBILITY_BLINDING_START,
   PE_DB_MOBILITY_BLINDING_POISON, // blinded
   PE_DB_MOBILITY_PARTIALLY_BLINDED,
   PE_DB_MOBILITY_BLINDING_END,
   PE_DB_MOBILITY_UNCANNYLUCK_HIT,
   PE_DB_MOBILITY_RAIN_DANCE_HIT,
   PE_DB_MOBILITY_STIMPACK_HIT,
   PE_DB_MOBILITY_INEBRIATED,
   PE_DB_MOBILITY_ACCURACY_END,
   PE_DB_MOBILITY_ACCURACY_AGAINST_START,
   PE_DB_MOBILITY_CUTE_FACE,
   PE_DB_MOBILITY_FROLICK,
   PE_DB_MOBILITY_ACCURACY_AGAINST_END,
   PE_DB_MOBILITY_CRIT_START,
   PE_DB_MOBILITY_CROAK,
   PE_DB_MOBILITY_HAWK_EYE,
   PE_DB_MOBILITY_BONKERS,
   PE_DB_MOBILITY_CAW,
   PE_DB_MOBILITY_ADRENAL_GLANDS,
   PE_DB_MOBILITY_UNCANNYLUCK_CRIT,
   PE_DB_MOBILITY_RAIN_DANCE_CRIT,
   PE_DB_MOBILITY_STIMPACK_CRIT,
   PE_DB_MOBILITY_CRIT_END,
   PE_DB_MOBILITY_SWITCH_START,  // activated by switching pets
   PE_DB_MOBILITY_SANCTIFIED_GROUND, // buff -- do first
   PE_DB_MOBILITY_BOOBY_TRAPPED_PRESENTS,
   PE_DB_MOBILITY_MINEFIELD,
   PE_DB_MOBILITY_SWITCH_END,
   PE_DB_MOBILITY_SOOTHED,
   PE_DB_MOBILITY_NEVERMORE,
   PE_DB_IMMUNE_INVISIBLE,
   PE_DB_IMMUNE_DODGE,
   PE_DB_IMMUNE_DEFLECTION,
   PE_DB_IMMUNE_FEIGN_DEATH,
   PE_DB_IMMUNE_PORTAL,
   PE_DB_IMMUNE_FADE,
   PE_DB_IMMUNE_ETHEREAL,
   PE_DB_IMMUNE_PHASE_END,
   PE_DB_IMMUNE_UNDERGROUND,
   PE_DB_IMMUNE_FLYING,
   PE_DB_IMMUNE_SUBMERGED,
   PE_DB_IMMUNE_LOCATION_END,
   PE_DB_IMMUNE_BLOCKS_START,
   PE_DB_IMMUNE_BLOCK,
   PE_DB_IMMUNE_BUBBLE,
   PE_DB_IMMUNE_DECOY,
   PE_DB_IMMUNE_SOULWARD,
   PE_DB_IMMUNE_SILK_COCOON,
   PE_DB_IMMUNE_BLOCKS_ALL_START,  // are these objectS?
   PE_DB_IMMUNE_ICE_BARRIER,
   PE_DB_IMMUNE_BEAVER_DAM,
   PE_DB_IMMUNE_PRISMATIC_BARRIER,
   PE_DB_IMMUNE_ILLUSIONARY_BARRIER,
   PE_DB_IMMUNE_BLOCKS_END,
   PE_DB_IMMUNE_MISC_START,
   PE_DB_IMMUNE_BREAK_UNDERGROUND,
   PE_DB_IMMUNE_DARK_REBIRTH,
   PE_DB_IMMUNE_SURVIVAL,
   PE_DB_IMMUNE_UNDEAD
};

class Debuff {

public:
   Debuff(const PE_DB_SUBTYPE subType, const PE_DURATION duration, const bool stickActive, const PE_SIDE sourceSide, const PE_PET_NUMBER sourcePet, const PE_IMMUNE_BREAK breaks, const vector<int> &values);
   PE_DB_SUBTYPE subType() { return m_subType; }
   PE_DURATION duration() { return m_duration; }
   PE_ERROR tick() { m_duration--; return PE_OK; }
   PE_ERROR refresh(const PE_DURATION &duration) { if (m_duration < duration) m_duration = duration; return PE_OK; }
   bool stick() { return m_stickActive; }
   PE_PET_NUMBER sourcePet() {return m_sourcePet; }
   PE_SIDE sourceSide() {return m_sourceSide; }
   vector<int> *values() { return &m_values; }
   int value(const int index) { return m_values[index]; } // function should know how many values there are
   int pop() { return m_values[m_index++]; }
   int dec(const int index) { return --(m_values[index]); }
   PE_ERROR add(const int index, const int value) {m_values[index] += value; return PE_OK; }
   PE_IMMUNE_BREAK immuneBreak() { return m_breaks; }

private:
   PE_DB_SUBTYPE m_subType;
   PE_DURATION m_duration;
   bool m_stickActive;
   PE_SIDE m_sourceSide;
   PE_PET_NUMBER m_sourcePet;
   PE_IMMUNE_BREAK m_breaks;
   int m_index;
   vector<int> m_values;
};

#endif