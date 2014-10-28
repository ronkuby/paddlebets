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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>
#include <string>

const double verySmall = 1.0e-10;

typedef enum PE_ERROR {
   PE_OK = 0,
   PE_NEED_INFO,
   PE_OUT_OF_RANGE,
   PE_UNKNOWN_MOVE = 100,
   PE_SOLVER_NOSOLUTION = 200,
   PE_SOLVER_PRUNE,
   PE_PETDEAD = 300,
   PE_ERRORS
};

typedef int32_t PE_SIDE;
const PE_SIDE PE_SIDEA = 0;
const PE_SIDE PE_SIDEB = 1;
const int32_t PE_SIDES = 2;

typedef int32_t PE_PET_NUMBER;
const PE_PET_NUMBER PE_NO_ACTIVE = -1;
const PE_PET_NUMBER PE_PET1 = 0;
const PE_PET_NUMBER PE_PET2 = 1;
const PE_PET_NUMBER PE_PET3 = 2;
const PE_PET_NUMBER PE_TEAM_SIZE = 3;

typedef enum PE_STAT {
   PE_STAT_HEALTH,
   PE_STAT_POWER,
   PE_STAT_SPEED,
   PE_STATS
};

typedef enum PE_ABILITY {
   PE_SLOT1,
   PE_SLOT2,
   PE_SLOT3,
   PE_ABILITIES,
   PE_SWITCH1,
   PE_SWITCH2,
   PE_SWITCH3,
   PE_PASS
};

typedef enum PE_SELECTION {
   PE_SELECTION1,
   PE_SELECTION2,
   PE_SELECTIONS
};

typedef enum PE_FAMILY {
   PE_FAMILY_AQUATIC,    // done 5.3
   PE_FAMILY_BEAST,      // done
   PE_FAMILY_CRITTER,    // done 5.4
   PE_FAMILY_DRAGONKIN,  //
   PE_FAMILY_ELEMENTAL,  //
   PE_FAMILY_FLYING,     //
   PE_FAMILY_HUMANOID,   // done
   PE_FAMILY_MAGIC,      // done
   PE_FAMILY_MECHANICAL, // done
   PE_FAMILY_UNDEAD,     // done
   PE_FAMILY_NOFAMILY,
   PE_FAMILIES
};

typedef enum PE_GENUS {
   PE_GENUS_NOGENUS,
   PE_GENUS_INSECT,
   PE_GENOME,
};

typedef enum PE_WEATHER {
   PE_NO_WEATHER,
   PE_WEATHER_ARCANE,    // done
   PE_WEATHER_BLIZZARD,  // done
   PE_WEATHER_DARKNESS,  // done
   PE_WEATHER_LIGHTNING_STORM, //done?
   PE_WEATHER_MOONLIGHT, // done
   PE_WEATHER_MUDSLIDE,  // done
   PE_WEATHER_RAIN,      // done
   PE_WEATHER_SANDSTORM, // done
   PE_WEATHER_SCORCHED_EARTH, //done
   PE_WEATHER_SUNLIGHT,  // done
   PE_WEATHER_WINDY // does this exist?
};

typedef enum PE_FP {
   PE_FP_GENERAL,
   PE_FP_DAMAGE_TAKEN,
   PE_FPS
};

typedef enum PE_IMMUNE_BREAK { // whether a debuff can break an immunity
   PE_IB_NONE,      // goes through no immune
   PE_IB_LOCATION,  // goes through submerged/flying/etc
   PE_IB_ALL,       // goes through all
};

typedef uint32_t PE_PN; // pet number (within total list of pets)
typedef uint32_t PE_AN; // ability number (within total list of abilities)
typedef uint32_t PE_BN; // breed number (per pet)
typedef uint32_t PE_ANP; // ability number (per pet slot) 
typedef int32_t PE_AV; // ability value
typedef int32_t PE_SV; // stat value

// sure about these numbers
const int PE_MAX_LEVEL = 25;
const double PE_CRIT_MOD = 0.5;

const double PE_MECH_SURVIVE = 0.2;
const double PE_AQUA_DOT = -0.25;
const double PE_BEAST_THRESHOLD = 0.5;
const double PE_BEAST_DAMAGE = 0.25;
const double PE_DRAGONKIN_DAMAGE = 0.5; // normal buff (can be dispelled)
const double PE_DRAGONKIN_THRESHOLD = 0.5;
const double PE_FLYING_THRESHOLD = 0.5;
const double PE_FLYING_SPEED = 0.25;
const double PE_HUMANOID_REGEN = 0.04;
const double PE_MAGIC_MAXHIT = 0.35;

const double PE_RAIN_AQUA = 0.25;
const double PE_SANDSTORM_REDUCTION = 0.10;
const double PE_DARKNESS_REDUCTION = 0.10;
const double PE_DARKNESS_HEALING = 0.50;
const double PE_SUNLIGHT_HEALING = 1.25;
const double PE_SUNLIGHT_HEALTH = 1.50;
const double PE_MOONLIGHT_HEALING = 1.25;
const double PE_MOONLIGHT_MAGIC = 0.1;
const double PE_LIGHTNING_MECHANICAL = 0.25;

const double PE_PROWL_SPEED = -0.30;
const double PE_FORBODING_CURSE = -0.25;
const double PE_CROAK = -1.00;
const double PE_RIGHTEOUS_INSPIRATION_SPEED = 1.00;
const double PE_BONESTORM = 0.1;
// damage factor mods
const double PE_CUTE_FACE = -0.25;
const double PE_FROLICK = -0.5;
const double PE_WEAKENED_DEFENSES = 0.25; // take 25% more damage
const double PE_RABIES_TAKEN = 0.5;
const double PE_COIN_TOSS_TAKEN = 0.5;
const double PE_RABIES_GIVEN = 0.25;
const double PE_COIN_TOSS_GIVEN = 0.25;
const double PE_SHATTERED = 1.0;
const double PE_ACIDIC_GOO = 0.25;
const double PE_CROUCHED = -0.5;
const double PE_STAGGERED_STEPS = -0.5;
const double PE_EXTRA_PLATING = -0.5;
const double PE_TOUGH_CUDDLY = -0.5;
const double PE_CELESTIAL_BLESSING = -0.5;
const double PE_PUMPED = 0.1;
const double PE_WINDUP = 0.1;
const double PE_ATTACK_BOOST = 0.25;
const double PE_RECKLESS = 0.25;
const double PE_AMPLIFY_BOOST = 0.50;
const double PE_INNER_VISION_BOOST = 1.00;
const double PE_FOCUS_CHI_BOOST = 1.00;
const double PE_DOMINANCE_BOOST = 1.00;
const double PE_SUPERCHARGE = 1.25;
const double PE_RIGHTEOUS_INSPIRATION_DAMAGE = 1.00;
const double PE_WEAKNESS = -0.5;
const double PE_OVERLOAD = 0.5;
const double PE_UNHOLY_ASCENSION = 0.25;
const double PE_HEARTBREAK = 1.0;
const double PE_ATTACK_REDUCTION_HIT = -0.5;
const double PE_ATTACK_REDUCTION_HIT2 = -0.25; // like bark
const double PE_PROWL_ATTACK = 1.5;
const double PE_CRYSTALOVERLOAD_ATTACK = 1.0;
//end damage factors
//healing mods
const double PE_HEALING_REDUCTION = -0.5;
//hit mod
const double PE_AUTUMN_BREEZE = -0.25;
const double PE_INEBRIATED = -0.25;
const double PE_SKUNKY_BREW = -0.25;
const double PE_ACCURACY = 0.25;
const double PE_NIMBUS = 0.2;
const double PE_INEBRIATE = -0.25;
const double PE_UNCANNY_LUCK_HIT = 0.5;
const double PE_UNCANNY_LUCK_CRIT = 0.25;
const double PE_RAIN_DANCE_HIT = 0.5;
const double PE_RAIN_DANCE_CRIT = 0.25;
const double PE_HAWK_EYE = 0.25;
const double PE_BONKERS = 1.0;
const double PE_CAW = 0.5;
const double PE_ADRENAL_GLANDS = 0.5;
const double PE_BLINDING_POISON = -1.00;
const double PE_PARTIALLY_BLINDED = -0.50;
const double PE_REBIRTH_DAMAGE = 0.2;
const double PE_GLOWING_TOXIN = 0.06;
const double PE_CORPSE_EXPLOSION = 0.05;
const double PE_CORPSE_EXPLOSION_BOOM = 0.15;
const double PE_EXPLODE = 0.40;
const int32_t PE_NEVERMORE = -4; // holds it for five rounds
const double PE_CREEPING_FUNGUS = 1.5;
const double PE_DREADFUL_BREATH = 1.34;
const double PE_TOXIC_SKIN = 0.05;
const double PE_REFLECTIVE_SHIELD = 0.5;
const double PE_DRAIN_BLOOD1 = 0.1;
const double PE_DRAIN_BLOOD2 = 3.0;

// not 100% on these numbers
const double PE_FAST_SPEED = 5.0;
const double PE_MAGMA_TRAP = 0.1; // magma trap likelihood of activating
const int32_t PE_PLAGUED_BLOOD = 50;
const int32_t PE_FROSTBITE = 29;
const int32_t PE_TOUCH_OF_THE_ANIMUS = 166;

const PE_AN PE_INVALID_ABILITYNUMBER = 9999;

const PE_SV PE_INFINITE = 99999;

typedef int32_t PE_DURATION;
const PE_DURATION PE_INVALID_COOLDOWN = -1;
const PE_DURATION PE_NO_COOLDOWN = 0;
const PE_DURATION PE_RESET_COOLDOWN = 0;
const PE_DURATION PE_LONG_COOLDOWN = 999;

const PE_DURATION PE_NO_ROUNDS = -1;
const PE_DURATION PE_ZERO_ROUND = 0;
const PE_DURATION PE_ONE_ROUND = 1;
const PE_DURATION PE_TWO_ROUNDS = 2;
const PE_DURATION PE_THREE_ROUNDS = 3;
const PE_DURATION PE_FOUR_ROUNDS = 4;
const PE_DURATION PE_FIVE_ROUNDS = 5;
const PE_DURATION PE_SIX_ROUNDS = 6;
const PE_DURATION PE_EIGHT_ROUNDS = 8;
const PE_DURATION PE_NINE_ROUNDS = 9;
const PE_DURATION PE_TEN_ROUNDS = 10;
const PE_DURATION PE_FIFTEEN_ROUNDS = 15;
const PE_DURATION PE_TWENTY_ROUNDS = 20;

typedef struct PE_PET_SELECTION_STRUCT {
   PE_PN petNumber;
   PE_BN breedNumber;
   PE_ANP abilitySelection[PE_ABILITIES];
} PE_PET_SELECTION;

typedef struct PE_TEAM_SELECTION_STRUCT {
   PE_PET_SELECTION pet[PE_TEAM_SIZE];
} PE_TEAM_SELECTION;

const double P_P = +0.50;
const double P_M = -0.34;

// attacker defender
const double familyMod[PE_FAMILIES][PE_FAMILIES] = {{0.0, 0.0, 0.0, 0.0, P_P, 0.0, 0.0, P_M, 0.0, 0.0, 0.0},
                                                    {0.0, 0.0, P_P, 0.0, 0.0, P_M, 0.0, 0.0, 0.0, 0.0, 0.0},
                                                    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, P_M, 0.0, 0.0, P_P, 0.0},
                                                    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, P_P, 0.0, P_M, 0.0},
                                                    {0.0, 0.0, P_M, 0.0, 0.0, 0.0, 0.0, 0.0, P_P, 0.0, 0.0},
                                                    {P_P, 0.0, 0.0, P_M, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                                    {0.0, P_M, 0.0, P_P, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                                    {0.0, 0.0, 0.0, 0.0, 0.0, P_P, 0.0, 0.0, P_M, 0.0, 0.0},
                                                    {0.0, P_P, 0.0, 0.0, P_M, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                                    {P_M, 0.0, 0.0, 0.0, 0.0, 0.0, P_P, 0.0, 0.0, 0.0, 0.0},
                                                    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};

typedef enum PE_MOVE {
   PE_INVALID_MOVE,
   // basic attacks
   PE_MOVE_SIMPLEDAMAGE200,
   PE_MOVE_SIMPLEDAMAGE100,
   PE_MOVE_SIMPLEHIGHLOW100,
   PE_MOVE_SIMPLEDAMAGE050,
   PE_MOVE_ONETOTHREEDAMAGE100,
   PE_MOVE_NOBELOW1DAMAGE100,
   PE_MOVE_050CRITDAMAGE100, // 50% chance to crit
   // team attacks
   PE_MOVE_TEAMATTACK100,                  // one value specified for active, other for backline
   PE_MOVE_TEAMATTACKSAME100,              // one value for all
   PE_MOVE_TEAMATTACKSPLIT100,
   PE_MOVE_TEAMATTACKOBJECTCLEAR100,
   PE_MOVE_BONESTORM,
   // attacks with damage back
   PE_MOVE_SIMPLEDAMAGEBACK100,
   // attacks with conditional extra damage
   PE_MOVE_EXTRAATTDAMAGE100,    // extra damage if attacked
   PE_MOVE_EXTRAFASTDAMAGE100,    // extra damage if faster
   PE_MOVE_DIFFERENTFASTDAMAGE100,    // different damage if faster
   PE_MOVE_EXTRASLOWDAMAGE100,    // extra damage if slower
   PE_MOVE_DOUBLEBELOW25DAMAGE100,    // extra damage if opponent lower than 25% health
   PE_MOVE_DOUBLEPOISONEDDAMAGE100,           // double damage if poisoned
   PE_MOVE_DOUBLEBLINDEDDAMAGE100,           // double damage if blinded
   PE_MOVE_DOUBLESTUNNEDDAMAGE100,  // double damage if stunned
   PE_MOVE_EXTRACHILLEDDAMAGE100,    // extra damage if chilled
   PE_MOVE_EXTRABLEEDDAMAGE100,    // extra damage if bleeding
   PE_MOVE_EXTRABURNDAMAGE100,    // extra damage if bleeding
   PE_MOVE_EXTRALOWHDAMAGE100,   // extra damage if user has lower health
   PE_MOVE_EXTRAARCANEDAMAGE100, // extra damage if weather is arcane
   PE_MOVE_EXTRAFASTONETWO100,   // 1-2 hits, one more if user is first
   PE_MOVE_INCREASINGDAMAGE100,  // increasing damage if used consecutively
   PE_MOVE_FRACTION010DAMAGE100,    // hits with second hit reducing health
   PE_MOVE_DOUBLEWEBSPLIT2DAMAGE100,
   PE_MOVE_LASTHITDAMAGE100, // hits for last hit against active pet (misses if no last hit)
   PE_MOVE_FLASH,
   // attacks with conditional accuracy
   PE_MOVE_BLEED100DAMAGE050,    // extra hit if bleeding
   PE_MOVE_BLIND100DAMAGE050,    // extra hit if target blind
   PE_MOVE_MOONLIGHT100DAMAGE080,    // extra hit if moonlight
   // attacks with conditional speed
   PE_MOVE_FIRSTBLIZZARDDAMAGE050,    // first if blizzard
   // attacks with heal
   PE_MOVE_HEAL100DAMAGE100,     // heals 100% of the damage done
   PE_MOVE_HEAL050DAMAGE100,     // heals 50% of the damage done
   PE_MOVE_HEALKILLDAMAGE100,    // heals if attack kills the enemy
   PE_MOVE_HEALDOUBLEWEBDAMAGE100,     // heals 100% of the damage done
   PE_MOVE_HEAL1ROUNDDAMAGE100,  // heals seperate damage done + 1 round DOT/HOT
   PE_MOVE_PHEROMONES,
   PE_MOVE_DRAINBLOOD,
   PE_MOVE_HEALMISSDAMAGE050,         // heals if misses
   // attack that go first
   PE_MOVE_FASTDAMAGE100,        // goes fast (5x?)
   PE_MOVE_FASTDAMAGE085,        // goes fast (5x?)
   // multi-round attacks
   PE_MOVE_GOLOWDAMAGE100,       // goes underground, then attacks
   PE_MOVE_GOHIGHDAMAGE100,       // goes in the air, then attacks
   PE_MOVE_SUBMERGEDAMAGE100,       // goes underwater, then attacks
   PE_MOVE_SUBMERGE2SUNS,       // goes underwater, then attacks
   PE_MOVE_HEALKILL2ROUNDDAMAGE100,  // heals if kills
   PE_MOVE_EXTRAARCANE3ROUNDDAMAGE100, // mana surge
   PE_MOVE_2ROUNDATTACK100,      // haywire
   PE_MOVE_3ROUNDATTACK100,      // rampage
   PE_MOVE_QUAKE,
   PE_MOVE_TYMPANICTANTRUM,
   PE_MOVE_RECHARGE2DAMAGE100,
   PE_MOVE_BUILDUP1DAMAGE100,
   // multi-attack attacks
   PE_MOVE_THREEATTACKSDAMAGE100,
   // attacks with buff/debuff
   PE_MOVE_1ROUNDINCDMG1ROUND100DAMAGE100, // increases opponent's damage taken by 100% (if hit) over 1 round (split damage)
   PE_MOVE_DECSPD425DAMAGE100,   // reduces opponents speed by 25% for 4 rounds and attacks
   PE_MOVE_DECSPD225DAMAGEHL100,   // reduces opponents speed by 25% for 2 rounds and attacks (slither)
   PE_MOVE_DECSPD225DAMAGE100,   // reduces opponents speed by 25% for 2 rounds and attacks (frost shock)
   PE_MOVE_DECSPD125DAMAGE100,   // reduces opponents speed by 25% for 2 rounds and attacks
   PE_MOVE_QUICKSAND,            // reduces opponents speed by 50% for 2 rounds and attacks
   PE_MOVE_DECDMG325DAMAGE100,   // reduces opponents damage by 25% for 3 rounds and attacks
   PE_MOVE_DECACC420DAMAGE100,   // reduces opponents accuracy by 20% for 4 rounds and attacks
   PE_MOVE_INCSPD100DAMAGE100,   // increase user speed by 100% next round and attacks
   PE_MOVE_INCSPD2100DAMAGE100,   // increase user speed by 100% next two round and attacks
   PE_MOVE_CENTRIFUGALHOOKS,   // increase user speed by 100% next two round and attacks
   PE_MOVE_INCSPD375DAMAGE100,   // increase user speed by 75% for 3 rounds and attacks
   PE_MOVE_INCSPD450DAMAGE100,   // increase user speed by 75% for 3 rounds and attacks
   PE_MOVE_INCDMG325DAMAGE100,   // increase user damage by 25% for 3 rounds
   PE_MOVE_INCDMGTAK25HLDAMAGE100,   // increase user taken damage by 25% for 3 rounds (reckless strike)
   PE_MOVE_DECDMG225INCDMG225DAMAGE100,   // reduce target damage done, increase damage done
   PE_MOVE_INCDMG325INCDMG350DAMAGE100,   // increase target damage by 25% for 3 rounds in ... rabies
   PE_MOVE_COINTOSS,           // increase target damage by 25% for 3 rounds in ... rabies
   PE_MOVE_INCTARDMG325DAMAGE100,   // increase target damage taken by 25% for 3 rounds
   PE_MOVE_DECSPD150DECDMG150DAMAGE100, // weakness
   PE_MOVE_HEARTBROKEN, // 
   PE_MOVE_DECDMG1ATTACK100,     // decrease damage from next hit (rake)
   PE_MOVE_2DECDMG1ATTACK100,     // decrease damage from next hit (different reduction -- bark)
   PE_MOVE_IRONBARK,     // decrease damage for one round
   PE_MOVE_DECHEAL250DAMAGE100,  // receives lower healing for 2 rounds
   PE_MOVE_DOTBLEED5DAMAGE100,   // attacks, dots with bleed
   PE_MOVE_DOTBLEED4DAMAGE100,   // attacks, dots with bleed
   PE_MOVE_DOTBURN2DAMAGE100,   // attacks, dots with burn
   PE_MOVE_DOTBURN4DAMAGE100,   // attacks, dots with burn
   PE_MOVE_DOTBURNI4DAMAGE100,   // attacks, dots with burn
   PE_MOVE_DOTBURNI24DAMAGE100,   // attacks, dots with burn
   PE_MOVE_FLAMEJET,
   PE_MOVE_CORROSION,             // + dmg, dot (aquatics??)
   PE_MOVE_DOTEL3DAMAGE100,      // applies an elemental dot (3 rounds) and attacks
   PE_MOVE_DOTEL4DAMAGE100,      // applies an elemental dot (4 rounds) and attacks
   PE_MOVE_DOTEL5DAMAGE100,      // applies an elemental dot (5 rounds) and attacks
   PE_MOVE_DOT2WINDDAMAGE100,    // wild winds
   PE_MOVE_DOT3OOZEDAMAGE100,    // creeping ooze
   PE_MOVE_DOT4ACIDDAMAGE100,    // acid touch
   PE_MOVE_TOXICSMOKE,           // 
   PE_MOVE_DOT9DEATH,            // death and decay
   PE_MOVE_DOT4STICKDAMAGE080,   // nut barrage & money barrage & bone barrage
   PE_MOVE_DOT3STICKDAMAGE050,   // rock barrage
   PE_MOVE_DOT2STICKDAMAGE050,   // banana barrage
   PE_MOVE_DOT1STICKDAMAGE050,   // egg barrage
   PE_MOVE_BUFFNDUMP100,         // barrel toss
   PE_MOVE_PUMPNDUMP100,         // pump
   PE_MOVE_WINDUPNDUMP100,       // pump
   PE_MOVE_LAUCHROCKET,          // pump
   PE_MOVE_DEBUFFNDUMP100,       // lock-on
   PE_MOVE_GRAVITY,              // gravity
   PE_MOVE_ROT,                  // rot
   PE_MOVE_BLISTERINGCOLD,       // 
   PE_MOVE_INCSPD020INCREASINGDAMAGE100, // progressively add speed buff
   PE_MOVE_ROOTED2DAMAGE100, // rooted for 2
   PE_MOVE_WEBBED2DAMAGE100, // rooted for 2
   PE_MOVE_ROOTED3DAMAGE100, // rooted for 3
   PE_MOVE_SKUNKYBREW, // rooted for  5
   PE_MOVE_ROOTED5DAMAGE100, // rooted for 5 -- sticky goo only? (no)
   PE_MOVE_BRITTLEDAMAGE100,
   PE_MOVE_STUNDAMAGE100, // stun (only this round)
   PE_MOVE_BLINDED2100DAMAGE100, // 100% chance to partially blind
   PE_MOVE_BLINDED50DAMAGE100, // 50% chance to partially blind
   PE_MOVE_STUNNED25DAMAGE100, // 25% chance to stun
   PE_MOVE_SLEEPED25HIGHLOW100, // 25% chance to sleep
   PE_MOVE_STUNNED25DARKDAMAGE100, // 25% chance to stun, 100% if darkness
   PE_MOVE_STUNNED25CHILLHLDAMAGE100, // 25% chance to stun, 100% if chilled (deep freeze)
   PE_MOVE_STUNNEDBLINDDAMAGE100, // 100% chance to stun if blinded
   PE_MOVE_MOTHBALLS,
   PE_MOVE_SIPHONLIFE,              // dot with heal
   PE_MOVE_PLAGUEDBLOOD, 
   PE_MOVE_TOUCHOFTHEANIMUS,
   // team attacks with debuff
   PE_MOVE_TEAMDECSPD425DAMAGE100,   // reduces team opponents speed by 25% for 4 rounds and attacks
   // hybrid attacks
   PE_MOVE_TRANSFORM125INCREASING100, // 25% chance to tranform for 1 round, increasing damage
   PE_MOVE_SLEEPING125INCREASING100, // 25% chance to sleep for 1 round, increasing damage
   PE_MOVE_3ROUNDINCDMG2ROUND100DAMAGE100, // increases opponent's damage taken by 100% (if hit) over 3 rounds (split damage)
   PE_MOVE_2ROUNDINCDMG1ROUND100DAMAGE100, // increases opponent's damage taken by 100% (if hit) over 2 rounds (split damage)
   PE_MOVE_STUNFASTER050DAMAGE100,    // 50% chance to stun if faster
   PE_MOVE_STUNMISSDAMAGE050,         // stuns if misses
   PE_MOVE_STUNSELFDAMAGE100,         // stuns self
   PE_MOVE_EXTRASUNRECHARGE2DAMAGE100, // extra damage if sun
   PE_MOVE_EXTRARAIN3ROUNDTEAMATTACK100, // bunch of crap
   PE_MOVE_CLEARUSERDEBUFFS, // clears almost everything from active
   PE_MOVE_CLEARTEAMDEBUFFS, // clears almost everything from team
   PE_MOVE_CORPSEEXPLOSION,
   PE_MOVE_EXPLODE,
   PE_MOVE_ARMAGEDDON,
   PE_MOVE_RAINDANCE,           // heal with double buff
   PE_MOVE_AUTUMNBREEZE,   // team heal with team hit debuff
   PE_MOVE_RIGHTEOUSINSPIRATION,
   PE_MOVE_HIGHLOWATTACKOBJECTCLEAR100, // attack and clear objects
   PE_MOVE_HAUNT,
   // switching attacks
   PE_MOVE_SWITCHNEXTDAMAGE100,
   PE_MOVE_SWITCHHIGHESTDAMAGE100,
   PE_MOVE_SWITCHLOWESTDAMAGE100,
   // basic heals
   PE_MOVE_SIMPLEHEAL,
   PE_MOVE_HEALFRACTION025,           // heals 1/4 health
   PE_MOVE_HEALFRACTION025FIRST,      // heals 1/4 health and goes first
   PE_MOVE_HEALHALFHIT,               // heals fixed amount + 1/2 last hit
   PE_MOVE_HEAL1ROUND,
   PE_MOVE_HEALALL,
   PE_MOVE_HEALOTHERS,
   PE_MOVE_HEALEQUALIZE, // life exchange
   PE_MOVE_HEALALLAQUATIC,
   PE_MOVE_HEALSELFALLMECHANICAL,
   PE_MOVE_HEALINCMAXHEALTH9ROUNDS,
   PE_MOVE_HIBERNATE,
   PE_MOVE_REPAIR,
   PE_MOVE_CONSUME_CORPSE,
   // straight buff/debuff
   PE_MOVE_DMGACTIVE5ROUND100_SWARM, // does damage to the active pet for 5 rounds
   PE_MOVE_GLOWINGTOXIN,
   PE_MOVE_CONFUSING_STING,
   PE_MOVE_IMMOLATION,
   PE_MOVE_CREEPING_FUNGUS,
   PE_MOVE_SPORE_SHROOMS,
   PE_MOVE_AGONY,                 // "increasing damage" inconsistent against aquatics (bug)
   PE_MOVE_MAGMATRAP,             // hate this one
   PE_MOVE_SNAPTRAP,              // 
   PE_MOVE_ICETOMB,               // ice tomb
   PE_MOVE_ELEMENTIUMBOLT,        // elementium bolt
   PE_MOVE_WHIRLPOOL,             // whirlpool
   PE_MOVE_EXPLOSIVEBREW,         // 
   PE_MOVE_STICKYGRENADE,         // 
   PE_MOVE_GEYSER,                // geyser
   PE_MOVE_BOOBYTRAPPEDPRESENTS,  // switch
   PE_MOVE_MINEFIELD,             // switch
   PE_MOVE_STUNSEED,              //
   PE_MOVE_ENTANGLINGROOTS,       //
   PE_MOVE_CHEW,                  // chew
   PE_MOVE_CURSEOFDOOM,           // COD
   PE_MOVE_BOMBINGRUN,            // 
   PE_MOVE_UNCANNYLUCK,           // luck
   PE_MOVE_STIMPACK,              // stimpack
   PE_MOVE_DECDMG2ROUND050,       // reduces damage against user by 50% for 2 rounds
   PE_MOVE_2DECDMG2ROUND050,      // reduces damage against user by 50% for 2 rounds
   PE_MOVE_EXTRAPLATING,      // reduces damage against user by 50% for 2 rounds
   PE_MOVE_DECDMG3ROUND050,       // reduces damage against user by 50% for 3 rounds (tough and cudly)
   PE_MOVE_INCDMG2ROUND100,       // increases damage against opponent by 100% for 2 rounds
   PE_MOVE_INCDMG2ROUND050,       // increase user damage by 50% for 2 rounds
   PE_MOVE_INNERVISION,           // increase user damage by 100% for 1 round
   PE_MOVE_INCCRIT425,            // increases critical strike by 25% for 4 rounds
   PE_MOVE_BONKERS,               // increases critical strike by 100% for 2 rounds
   PE_MOVE_INCCRIT250,            // increases critical strike by 50% for 2 rounds
   PE_MOVE_ADRENALGLANDS,            // increases critical strike by 50% for 5 rounds
   PE_MOVE_NIMBUS,                // increases team hit by 20% for 9 rounds
   PE_MOVE_ANZU,                  // accuracy + attack boost
   PE_MOVE_INCHIT425,             // luck?
   PE_MOVE_INCHIT5100,            // luck?
   PE_MOVE_CELESTIALBLESSING,            // 
   PE_MOVE_ENDURE1,               // survives the next round (health no go below 1)
   PE_MOVE_REBIRTH1,              // survives the next round bad bad
   PE_MOVE_HOT5ROUNDELEMENTAL,    // Nature's Ward
   PE_MOVE_HOT5ROUNDPT,           // photosyn
   PE_MOVE_HOT5ROUNDSM,           // five round hot
   PE_MOVE_HOT3ROUNDRM,           // three round hot
   PE_MOVE_HOT2ROUNDTQ,           // tranquillity
   PE_MOVE_SHIELD5ROUND,          // five round shield
   PE_MOVE_EPRESENCE5ROUND,       // soooh
   PE_MOVE_JADESKIN5ROUND,       // soooh
   PE_MOVE_BARKSKIN,             //
   PE_MOVE_STONESKIN5ROUND,       // five round shield
   PE_MOVE_TRIHORNSHIELD3ROUND,   // three round shield
   PE_MOVE_SPIKED5ROUND,          // five round shield with damage back
   PE_MOVE_SPIRITSPIKES,          // 
   PE_MOVE_LIGHTNINGSHIELD2ROUND, // two round damage back (no shield)
   PE_MOVE_THORNS,                // five round damage back
   PE_MOVE_HEATUP,                // 3 round damage back
   PE_MOVE_SPINYCARAPACE,         //
   PE_MOVE_TOXICSKIN,             //
   PE_MOVE_REFLECTIVESHIELD,      //
   PE_MOVE_INCTEAMSPD9ROUND025,   // increase team speed by 25% for 9 rounds (dazzling dance)
   PE_MOVE_2INCTEAMSPD9ROUND025,  // increase team speed by 25% for 9 rounds (moon dance)
   PE_MOVE_SHIELDSTORM,           // gives each pet a block
   PE_MOVE_PLANT,                 // plant
   PE_MOVE_CYCLONE,               // release a cyclone
   PE_MOVE_BUILDTURRET,           // 
   PE_MOVE_XE321BOOMBOT,           // 
   PE_MOVE_VOLCANO,               // make a volcano
   PE_MOVE_UNHOLYASCENSION,
   PE_MOVE_ADDDAMWILD,            // adds damage against target
   PE_MOVE_ADDDAMEXPOSED,         // adds damage against target
   PE_MOVE_ADDDAMCLAW,            // adds damage against target
   PE_MOVE_FORBODINGCURSE,        // adds damage against target and a speed debuff
   PE_MOVE_ADDDAMMANGLE,          // adds damage against target
   PE_MOVE_DODGE,                 // dodge
   PE_MOVE_DEFLECTION,            // deflection
   PE_MOVE_FADING,                // go invisible in two turns
   PE_MOVE_FEIGN_DEATH,           // stuff
   PE_MOVE_PORTAL,               // stuff
   PE_MOVE_FADE,                  // more stuff
   PE_MOVE_ETHEREAL,              // more stuff n stuff
   PE_MOVE_SHIELDBLOCK,           // block 1 attack
   PE_MOVE_BUBBLE,                // block 2 attacks
   PE_MOVE_DECOY,                // block 2 attacks
   PE_MOVE_SOULWARD,               // block 1 attack
   PE_MOVE_COCOONSTRIKE,          // block 1 attacks, goes second next round
   PE_MOVE_ICE_BARRIER,           // block 2 attacks from anyone
   PE_MOVE_BEAVER_DAM,            // block 2 attacks from anyone
   PE_MOVE_PRISMATICBARRIER,            // block 2 attacks from anyone
   PE_MOVE_ILLUSIONARYBARRIER,            // block 2 attacks from anyone
   PE_MOVE_DECCRIT4ROUND,         // lower enemy crit
   PE_MOVE_DECACC425,
   PE_MOVE_INEBRIATE,
   PE_MOVE_BLINDINGPOISON,
   PE_MOVE_NEVERMORE,
   PE_MOVE_CUTE_FACE,             // 
   PE_MOVE_FROLICK,               // 
   PE_MOVE_SOOTHE,                // asleep in one round if not damaged
   PE_MOVE_STUN,                  // stun for a round
   PE_MOVE_FOOD_COMA,             //
   PE_MOVE_INCDMG1HIT1ROUND,      // double damage, 1 hit, 1 round, (focused chi)
   PE_MOVE_DOMINANCE,             // double damage, 1 hit, 2 rounds
   PE_MOVE_SUPERCHARGE,           // double damage, 1 hit, 1 round
   PE_MOVE_PROWL,                 // + 150% dmg on hit, -30% on speed
   PE_MOVE_CRYSTAL_OVERLOAD,      // annoying ability
   PE_MOVE_ACIDIC_GOO,            // + 25% dmg, dot (aquatics??)
   PE_MOVE_SEARMAGIC,
   PE_MOVE_APOCALYPSE,
   PE_MOVE_STING6ROUND,

   // weather
   PE_MOVE_ARCANESTORM,
   PE_MOVE_CALLBLIZZARD,
   PE_MOVE_CALLDARKNESS,
   PE_MOVE_CALLLIGHTNING,
   PE_MOVE_MOONFIRE,
   PE_MOVE_STARFALL,
   PE_MOVE_MUDSLIDE,
   PE_MOVE_ACIDRAIN,
   PE_MOVE_CLEANSINGRAIN,
   PE_MOVE_SANDSTORM,
   PE_MOVE_SCORCHEDEARTH,
   PE_MOVE_ILLUMINATE,
   PE_MOVE_SUNLIGHT,

   // null
   PE_MOVE_NULL,

   PE_MOVES
};

const uint32_t slotSelection = 2;
const int32_t maxEval = 10000;
const double maxProb = 1.0;
const double baseCritProb = 0.05;
const double fiveProb = 0.05;
const double fiftyProb = 0.5;
const double thirdProb = 0.333333333;

#define MAX_PETS 1000

#define PE_VERBOSE_OFF 0
#define PE_VERBOSE_REQ 1
#define PE_VERBOSE_ON 2

#define PE_VERBOSE PE_VERBOSE_ON

// Begin game changing constants
const int32_t perTurn = 350;
const double ignoreBranch = 0.09;
const double ignoreJoint = 0.04;
const double primaryJoint = 0.50;
//const double ignoreBranch = 0.05;
//const double ignoreJoint = 0.01;
//const double primaryJoint = 0.20;
const int maxSimplex = 16;
const double depthFactor = 1.0e6;
const int sizeSkipPass = 3;
const int nullDepth = 2;
const int32_t nullValue = 200;

#endif

