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

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include "constants.h"
#include "petinfo.h"
#include "move.h"

const int PE_NUM_BAD_PETS = 1;
char *PE_BAD_PETS[PE_NUM_BAD_PETS] = {"Tiny Red Dragon"};

Petinfo::Petinfo(const char *filename, const bool dump) {
   FILE *fid, *fout = NULL;
   char buffer[256];
   int32_t length, dummy;
   PE_FAMILY family;
   vector<string> badPets(PE_BAD_PETS, PE_BAD_PETS + PE_NUM_BAD_PETS);

   fopen_s(&fid, filename, "rb");
   if (fid == (FILE *)NULL) {
      printf("error reading data file\n");
      exit(-1);
   }
   if (dump) {
      fopen_s(&fout, "dump.txt", "w");
      if (fout == (FILE *)NULL) {
         printf("error opening dump file\n");
         exit(-1);
      }
      printf("Dumping pet data to dump.txt\n");
   }
   fread_s(&m_pets, sizeof(int32_t), sizeof(int32_t), 1, fid);
   fread_s(&m_abilities, sizeof(int32_t), sizeof(int32_t), 1, fid);
   m_abilityFamily.resize(m_abilities);
   fread_s(&m_abilityFamily[0], m_abilities*sizeof(int32_t), sizeof(int32_t), m_abilities, fid);
   for (int i = 0; i < m_abilities; i++) {
      fread_s(&length, sizeof(int32_t), sizeof(int32_t), 1, fid);
      fread_s(buffer, length*sizeof(char), sizeof(char), length, fid);
      buffer[length] = '\0';
      m_abilityName.push_back(buffer);
   }
   m_petAbility.resize(m_pets);
   m_petStats.resize(m_pets);
   m_abilityValues.resize(m_pets);

   PE_PN pet = 0;
   for (PE_PN i = 0; i < m_pets; i++) {
      fread_s(&length, sizeof(int32_t), sizeof(int32_t), 1, fid);
      fread_s(buffer, length*sizeof(char), sizeof(char), length, fid);
      buffer[length] = '\0';

      bool badPet = false;
      for (PE_PN bpet = 0; bpet < badPets.size(); bpet++) {
         if (buffer == badPets[bpet]) badPet = true;
      }
      if (!badPet) {
         m_petName.push_back(buffer);
      }

	  fread_s(&family, sizeof(PE_FAMILY), sizeof(PE_FAMILY), 1, fid);
      if (!badPet) {
         m_petFamily.push_back(family);
      }

	   fread_s(&length, sizeof(int32_t), sizeof(int32_t), 1, fid);
	   fread_s(buffer, length*sizeof(char), sizeof(char), length, fid);
      buffer[length] = '\0';
      if (!badPet) {
         m_petGenus.push_back(buffer);
      }

      m_petAbility[pet].resize(PE_ABILITIES*PE_SELECTIONS);
      fread_s(&m_petAbility[pet][0], PE_ABILITIES*PE_SELECTIONS*sizeof(int32_t), sizeof(int32_t), PE_ABILITIES*PE_SELECTIONS, fid);
      fread_s(&dummy, sizeof(int32_t), sizeof(int32_t), 1, fid);
      m_petBreeds.push_back(dummy);
      m_petStats[pet].resize(m_petBreeds[pet]);
      m_abilityValues[pet].resize(m_petBreeds[pet]);

      for (PE_BN n = 0; n < m_petBreeds[pet]; n++) {
         m_petStats[pet][n].resize(PE_STATS);
         fread_s(&m_petStats[pet][n][0], PE_STATS*sizeof(int32_t), sizeof(int32_t), PE_STATS, fid);
         m_abilityValues[pet][n].resize(PE_ABILITIES*PE_SELECTIONS);
         for (int a = 0; a < PE_ABILITIES*PE_SELECTIONS; a++) {
            fread_s(&dummy, sizeof(int32_t), sizeof(int32_t), 1, fid);
            if (dummy > 0) {
               m_abilityValues[pet][n][a].resize(dummy);
               fread_s(&m_abilityValues[pet][n][a][0], dummy*sizeof(int32_t), sizeof(int32_t), dummy, fid);
            }
         }
      }

      if (!badPet) {
         pet++;
      }
      else {
         m_petBreeds.pop_back();
      }
   }
   m_pets = pet;
   m_petAbility.resize(m_pets);
   m_petStats.resize(m_pets);
   m_abilityValues.resize(m_pets);

   if (dump) {
      for (PE_PN i = 0; i < m_pets; i++) {
         fprintf(fout, "%3d: %s\n", i + 1, m_petName[i].c_str());
         for (PE_BN j = 0; j < m_petBreeds[i]; j++) {
            fprintf(fout, "   %d: %d %d %d\n", j + 1, m_petStats[i][j][PE_STAT_HEALTH], m_petStats[i][j][PE_STAT_POWER], m_petStats[i][j][PE_STAT_SPEED]);
         }
         for (int a = PE_SLOT1; a < PE_ABILITIES; a++) {
            fprintf(fout, "     %s -- %s\n", m_abilityName[m_petAbility[i][2 * a]].c_str(), m_abilityName[m_petAbility[i][2 * a + 1]].c_str());
         }
      }
      fclose(fout);
   }
   fclose(fid);

#ifdef MAX_PETS
   if (MAX_PETS < m_pets) m_pets = MAX_PETS;
#endif

}

PE_GENUS Petinfo::getGenus(const PE_PN &pet) {

   if (m_petGenus[pet] == "Insect Companions") {
      return PE_GENUS_INSECT;
   }
   else {
      return PE_GENUS_NOGENUS;
   }
}

string Petinfo::getAbilityName(const PE_PN pet, const PE_ABILITY ability, PE_FAMILY &family) {
   
   if (pet >= m_pets || ability >= PE_ABILITIES*PE_SELECTIONS) {
      abort();
   }
   PE_AN abilityNumber = m_petAbility[pet][ability];
   family = m_abilityFamily[abilityNumber];
   
   return m_abilityName[abilityNumber];
}

PE_ERROR Petinfo::getAbilityValues(const PE_PN &pet, const PE_BN &breed, const PE_ABILITY ability, vector<PE_AV> &abilityValues) {
   // put error checking here later
   abilityValues = m_abilityValues[pet][breed][ability];

   return PE_OK;
}

PE_ERROR Petinfo::checkAbilities() {
	const PE_SV dummyPower = 100;
   PE_ERROR error;
   PE_MOVE move;
   PE_DURATION cooldown;
   PE_AN expectedNumAV;
   PE_FAMILY abilityFamily;
   vector<PE_AV> abilityValues;
   vector<bool> loaded(m_abilities, false);
   int count = 0;
   
   for (uint32_t pet = 0; pet < m_pets; pet++) {
	   for (uint32_t breed = 0; breed < m_petBreeds[pet]; breed++) {
         for (int ab = 0; ab < PE_SELECTIONS*PE_ABILITIES; ab++) {
            getAbilityValues(pet, breed, (PE_ABILITY)ab, abilityValues);
            string abilityString(getAbilityName(pet, (PE_ABILITY)ab, abilityFamily));

            error = this->getMove(abilityString, dummyPower, abilityFamily, move, expectedNumAV, cooldown);
			   if (error == PE_NEED_INFO) {
				   abilityValues.push_back(expectedNumAV);
				   expectedNumAV = 1;
				   error = PE_OK;
			   }
			   if (error != PE_OK) {
#if PE_VERBOSE > PE_VERBOSE_OFF
				   for (uint32_t ab = 0; ab < loaded.size(); ab++) {
                  if (loaded[ab]) count++;
               }
               printf("Unknown ability %s family %d for pet %s (%d).  Count = %d/%d.\n", abilityString.c_str(), abilityFamily, m_petName[pet].c_str(), pet, count, loaded.size());
			   for (uint32_t i = 0; i < abilityValues.size(); i++) {
                  printf("  %d %d\n", i, abilityValues[i]);
               }
#endif
               exit(9);
            }
            else if (expectedNumAV != abilityValues.size()) {
#if PE_VERBOSE > PE_VERBOSE_OFF
				printf("Wrong number of ability numbers on ability %s for pet %s (%d) %d %d\n", abilityString.c_str(), m_petName[pet].c_str(), pet, expectedNumAV, abilityValues.size());
			   for (uint32_t i = 0; i < abilityValues.size(); i++) {
                  printf("  %d %d\n", i, abilityValues[i]);
               }
#endif
               exit(9);
            }
            loaded[m_petAbility[pet][ab]] = true;
         }
      }
   }
   for (uint32_t ab = 0; ab < loaded.size(); ab++) {
      if (loaded[ab]) count++;
   }
#if PE_VERBOSE > PE_VERBOSE_OFF
   printf("Successfully loaded %d/%d abilities.\r\n", count, m_abilities);
#endif

   return PE_OK;
}
   
PE_ERROR Petinfo::getMove(const string &moveName, const PE_SV &power, const PE_FAMILY &family, PE_MOVE &move, PE_AN &numAbilityValues, PE_DURATION &cooldown) {

   move = PE_INVALID_MOVE;
   numAbilityValues = PE_INVALID_ABILITYNUMBER;
   cooldown = PE_INVALID_COOLDOWN;
   switch (family) {
   case PE_FAMILY_AQUATIC:
      if (!moveName.compare("Acid Rain")) {
         move = PE_MOVE_ACIDRAIN;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Acid Touch")) {
	 	   move = PE_MOVE_DOT4ACIDDAMAGE100;
  		   numAbilityValues = 2;
		   cooldown = PE_NO_COOLDOWN;
	   }
	   else if (!moveName.compare("Blood in the Water")) {
         move = PE_MOVE_BLEED100DAMAGE050;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Brew Bolt")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Cleansing Rain")) {
         move = PE_MOVE_CLEANSINGRAIN;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Creeping Ooze")) {
		 move = PE_MOVE_DOT3OOZEDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Croak")) {
         move = PE_MOVE_DECCRIT4ROUND;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Dive")) {
         move = PE_MOVE_SUBMERGEDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Frog Kiss")) {
         move = PE_MOVE_TRANSFORM125INCREASING100;
         numAbilityValues = 3;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Grasp")) {
         move = PE_MOVE_ROOTED2DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Healing Stream")) {
         move = PE_MOVE_HEALALLAQUATIC;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Healing Wave")) {
         move = PE_MOVE_SIMPLEHEAL;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Pump")) {
         move = PE_MOVE_PUMPNDUMP100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Rain Dance")) {
         move = PE_MOVE_RAINDANCE;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Renewing Mists")) {
         move = PE_MOVE_HOT3ROUNDRM;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Spiny Carapace")) {
         move = PE_MOVE_SPINYCARAPACE;
         numAbilityValues = 1;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Steam Vent")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Surge")) {
         move = PE_MOVE_FASTDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Swallow You Whole")) {
         move = PE_MOVE_DOUBLEBELOW25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Tail Slap")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Tidal Wave")) {
         move = PE_MOVE_TEAMATTACKOBJECTCLEAR100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Toxic Skin")) {
         move = PE_MOVE_TOXICSKIN;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Water Jet")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Whirlpool")) {
         move = PE_MOVE_WHIRLPOOL;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   case PE_FAMILY_BEAST:
      if (!moveName.compare("Banana Barrage")) {
         move = PE_MOVE_DOT2STICKDAMAGE050;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Bark")) {
         move = PE_MOVE_2DECDMG1ATTACK100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Barrel Toss")) {
         move = PE_MOVE_BUFFNDUMP100;
         numAbilityValues = 1;
         cooldown = PE_ZERO_ROUND;
      }
      else if (!moveName.compare("Bash")) {
         move = PE_MOVE_STUN;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Bite")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Black Claw")) {
         move = PE_MOVE_ADDDAMCLAW;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Blinding Poison")) {
         move = PE_MOVE_BLINDINGPOISON;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Bloodfang")) {
         move = PE_MOVE_HEALKILLDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Brittle Webbing")) {
         move = PE_MOVE_BRITTLEDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Burrow")) {
         move = PE_MOVE_GOLOWDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Call the Pack")) {
         move = PE_MOVE_1ROUNDINCDMG1ROUND100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Claw")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Crystal Prison")) {
         move = PE_MOVE_STUN;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Devour")) {
         move = PE_MOVE_HEALKILL2ROUNDDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Digest Brains")) {
         move = PE_MOVE_SIMPLEHEAL;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Dominance")) {
         move = PE_MOVE_DOMINANCE;
         numAbilityValues = 0;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Exposed Wounds")) {
         move = PE_MOVE_ADDDAMEXPOSED;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Feed")) {
         move = PE_MOVE_HEAL100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Feign Death")) {
         move = PE_MOVE_FEIGN_DEATH;
         numAbilityValues = 0;
         cooldown = PE_EIGHT_ROUNDS;
      }
      else if (!moveName.compare("Gnaw")) {
         move = PE_MOVE_EXTRAFASTDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Going Bonkers!")) {
         move = PE_MOVE_BONKERS;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Haywire")) {
         move = PE_MOVE_2ROUNDATTACK100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Headbutt")) {
         move = PE_MOVE_STUNNED25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Hibernate")) {
         move = PE_MOVE_HIBERNATE;
         numAbilityValues = 3;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Hiss")) {
         move = PE_MOVE_DECSPD425DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Horn Attack")) {
         move = PE_MOVE_STUNFASTER050DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Horn Gore")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Howl")) {
         move = PE_MOVE_INCDMG2ROUND100;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Huge Fang")) {
         move = PE_MOVE_INCREASINGDAMAGE100;
         numAbilityValues = 3;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Huge, Sharp Teeth!")) {
         move = PE_MOVE_DOTBLEED4DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Hunting Party")) {
         move = PE_MOVE_2ROUNDINCDMG1ROUND100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Impale")) {
         move = PE_MOVE_DOUBLEBELOW25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Leap")) {
         move = PE_MOVE_INCSPD100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Mangle")) {
         move = PE_MOVE_ADDDAMMANGLE;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Maul")) {
         move = PE_MOVE_EXTRABLEEDDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Niuzao\\'s Charge")) {
         move = PE_MOVE_BUILDUP1DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Pheromones")) {
         move = PE_MOVE_PHEROMONES;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Plot Twist")) {
         move = PE_MOVE_THREEATTACKSDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Poison Fang")) {
         move = PE_MOVE_DOTEL5DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Poison Spit")) {
         move = PE_MOVE_DOTEL3DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Pounce")) {
         move = PE_MOVE_EXTRAFASTDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Primal Cry")) {
         move = PE_MOVE_TEAMDECSPD425DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Prowl")) {
         move = PE_MOVE_PROWL;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Puncture Wound")) {
         move = PE_MOVE_DOUBLEPOISONEDDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Rake")) {
         move = PE_MOVE_DECDMG1ATTACK100;
         numAbilityValues = 1;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Rampage")) {
         move = PE_MOVE_3ROUNDATTACK100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Ravage")) {
         move = PE_MOVE_HEALKILL2ROUNDDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Rend")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Rip")) {
         move = PE_MOVE_DOTBLEED5DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Roar")) {
         move = PE_MOVE_INCDMG325DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Rush")) {
         move = PE_MOVE_INCSPD100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Screech")) {
         move = PE_MOVE_DECSPD425DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Shell Shield")) {
         move = PE_MOVE_SHIELD5ROUND;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Skunky Brew")) {
         move = PE_MOVE_SKUNKYBREW;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Slither")) {
         move = PE_MOVE_DECSPD225DAMAGEHL100;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Smash")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Snap")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Spiderling Swarm")) {
         move = PE_MOVE_DOUBLEWEBSPLIT2DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Spirit Claws")) {
         move = PE_MOVE_MOONLIGHT100DAMAGE080;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Sticky Web")) {
         move = PE_MOVE_WEBBED2DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Sting")) {
         move = PE_MOVE_STING6ROUND;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Strike")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("The Good Stuff")) {
         move = PE_MOVE_HEALOTHERS;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Thrash")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Tough n\\' Cuddly")) {
         move = PE_MOVE_DECDMG3ROUND050;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Trample")) {
         move = PE_MOVE_FRACTION010DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Trihorn Charge")) {
         move = PE_MOVE_FASTDAMAGE085;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Trihorn Shield")) {
         move = PE_MOVE_TRIHORNSHIELD3ROUND;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Triple Snap")) {
         move = PE_MOVE_ONETOTHREEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Trumpet Strike")) {
         move = PE_MOVE_INCDMG325DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Vengeance")) {
         move = PE_MOVE_LASTHITDAMAGE100;
         numAbilityValues = 0;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Vicious Fang")) {
         move = PE_MOVE_INCREASINGDAMAGE100;
         numAbilityValues = 3;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Woodchipper")) {
         move = PE_MOVE_DOTBLEED4DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   case PE_FAMILY_CRITTER:
      if (!moveName.compare("Acidic Goo")) {
         move = PE_MOVE_ACIDIC_GOO;
         numAbilityValues = 1;
         cooldown = PE_ZERO_ROUND;
      }
      else if (!moveName.compare("Apocalypse")) {
         move = PE_MOVE_APOCALYPSE;
         numAbilityValues = 0;
         cooldown = PE_TWENTY_ROUNDS;
      }
      else if (!moveName.compare("Adrenaline Rush")) {
         move = PE_MOVE_INCSPD375DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Beaver Dam")) {
         move = PE_MOVE_BEAVER_DAM;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Bleat")) {
         move = PE_MOVE_HEALALL;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Buried Treasure")) {
         move = PE_MOVE_HEALFRACTION025;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Chew")) {
         move = PE_MOVE_CHEW;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Chomp")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Comeback")) {
         move = PE_MOVE_EXTRALOWHDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Crouch")) {
         move = PE_MOVE_DECDMG2ROUND050;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Dazzling Dance")) {
         move = PE_MOVE_INCTEAMSPD9ROUND025;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Egg Barrage")) {
         move = PE_MOVE_DOT1STICKDAMAGE050;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Flank")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Flurry")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Food Coma")) {
         move = PE_MOVE_FOOD_COMA;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Gobble Strike")) {
         move = PE_MOVE_INCSPD450DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Hoof")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
	  else if (!moveName.compare("Lucky Dance")) {
		  move = PE_MOVE_UNCANNYLUCK;
		  numAbilityValues = 0;
		  cooldown = PE_NO_COOLDOWN;
	  }
	  else if (!moveName.compare("Mudslide")) {
         move = PE_MOVE_MUDSLIDE;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Nature\\'s Touch")) {
         move = PE_MOVE_SIMPLEHEAL;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Nut Barrage")) {
         move = PE_MOVE_DOT4STICKDAMAGE080;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Perk Up")) {
         move = PE_MOVE_HEALINCMAXHEALTH9ROUNDS;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Powerball")) {
         move = PE_MOVE_INCSPD020INCREASINGDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Quick Attack")) {
         move = PE_MOVE_FASTDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Scratch")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Skitter")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Soothe")) {
         move = PE_MOVE_SOOTHE;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Stampede")) { // question: is the damage distributed over 3 attacks
         move = PE_MOVE_3ROUNDINCDMG2ROUND100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Stench")) {
         move = PE_MOVE_DECACC425;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Survival")) {
         move = PE_MOVE_ENDURE1;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Swarm")) { // question: is the damage distributed over 3 attacks
         move = PE_MOVE_3ROUNDINCDMG2ROUND100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Swarm of Flies")) {
         move = PE_MOVE_DMGACTIVE5ROUND100_SWARM;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Tongue Lash")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Tranquility")) {
         move = PE_MOVE_HOT2ROUNDTQ;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Uncanny Luck")) {
         move = PE_MOVE_UNCANNYLUCK;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Vicious Streak")) {
         move = PE_MOVE_INCSPD2100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   case PE_FAMILY_DRAGONKIN:
      if (!moveName.compare("Ancient Blessing")) {
         move = PE_MOVE_HEALINCMAXHEALTH9ROUNDS;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Breath")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Cataclysm")) {
         move = PE_MOVE_SIMPLEDAMAGE050;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Deep Breath")) {
         move = PE_MOVE_BUILDUP1DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Elementium Bolt")) {
         move = PE_MOVE_ELEMENTIUMBOLT;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Emerald Dream")) {
         move = PE_MOVE_HIBERNATE;
         numAbilityValues = 3;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Emerald Presence")) {
         move = PE_MOVE_EPRESENCE5ROUND;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Flame Breath")) {
         move = PE_MOVE_DOTBURN4DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Frost Breath")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Jade Breath")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Healing Flame")) {
         move = PE_MOVE_HEALHALFHIT;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Instability")) {
         move = PE_MOVE_SIMPLEDAMAGE050;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Proto-Strike")) {
         move = PE_MOVE_GOHIGHDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Roll")) {
         move = PE_MOVE_INCDMG325DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Scorched Earth")) {
         move = PE_MOVE_SCORCHEDEARTH;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Shadowflame")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Shriek")) {
         move = PE_MOVE_DECDMG325DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Sleeping Gas")) {
         move = PE_MOVE_SLEEPING125INCREASING100;
         numAbilityValues = 3;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Spiked Skin")) {
         move = PE_MOVE_SPIKED5ROUND;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Starfall")) {
         move = PE_MOVE_STARFALL;
         numAbilityValues = 2;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Tail Sweep")) {
         move = PE_MOVE_EXTRASLOWDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   case PE_FAMILY_ELEMENTAL:
      if (!moveName.compare("Autumn Breeze")) {
         move = PE_MOVE_AUTUMNBREEZE;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Avalanche")) {
         move = PE_MOVE_TEAMATTACKSPLIT100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Barkskin")) {
         move = PE_MOVE_BARKSKIN;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Belly Slide")) {
         move = PE_MOVE_FIRSTBLIZZARDDAMAGE050;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Blistering Cold")) {
         move = PE_MOVE_BLISTERINGCOLD;
         numAbilityValues = 0;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Breath of Sorrow")) {
         move = PE_MOVE_DECHEAL250DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Burn")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Call Blizzard")) {
         move = PE_MOVE_CALLBLIZZARD;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Cauterize")) {
         move = PE_MOVE_HEALHALFHIT;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Conflagrate")) {
         move = PE_MOVE_EXTRABURNDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Crystal Overload")) {
         move = PE_MOVE_CRYSTAL_OVERLOAD;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Dark Rebirth")) {
         move = PE_MOVE_REBIRTH1;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Darkflame")) {
         move = PE_MOVE_DECHEAL250DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Deep Freeze")) {
         move = PE_MOVE_STUNNED25CHILLHLDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Early Advantage")) {
         move = PE_MOVE_EXTRALOWHDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Entangling Roots")) {
         move = PE_MOVE_ENTANGLINGROOTS;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Explosive Brew")) {
         move = PE_MOVE_EXPLOSIVEBREW;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Fel Immolate")) {
         move = PE_MOVE_ENTANGLINGROOTS;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Fire Quills")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Flamethrower")) {
         move = PE_MOVE_DOTBURN2DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Flame Jet")) {
         move = PE_MOVE_FLAMEJET;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Frost Nova")) {
         move = PE_MOVE_ROOTED2DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Frost Shock")) {
         move = PE_MOVE_DECSPD225DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Frost Spit")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Geyser")) {
         move = PE_MOVE_GEYSER;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Heat Up")) {
         move = PE_MOVE_HEATUP;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Howling Blast")) {
         move = PE_MOVE_EXTRACHILLEDDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Ice Barrier")) {
         move = PE_MOVE_ICE_BARRIER;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Ice Lance")) {
         move = PE_MOVE_EXTRACHILLEDDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Ice Tomb")) {
         move = PE_MOVE_ICETOMB;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Immolate")) {
         move = PE_MOVE_DOTBURNI4DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Immolation")) {
         move = PE_MOVE_IMMOLATION;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Inebriate")) {
         move = PE_MOVE_INEBRIATE;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Inspiring Song")) {
         move = PE_MOVE_HEALALL;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Ironbark")) {
         move = PE_MOVE_IRONBARK;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Jolt")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Lash")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Leech Seed")) {
         move = PE_MOVE_HEAL1ROUNDDAMAGE100;
         numAbilityValues = 3;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Lightning Shield")) {
         move = PE_MOVE_LIGHTNINGSHIELD2ROUND;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Magma Trap")) {
         move = PE_MOVE_MAGMATRAP;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Magma Wave")) {
         move = PE_MOVE_TEAMATTACKOBJECTCLEAR100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Nature\\'s Ward")) {
         move = PE_MOVE_HOT5ROUNDELEMENTAL;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Paralyzing Shock")) {
         move = PE_MOVE_ROOTED3DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Photosynthesis")) {
         move = PE_MOVE_HOT5ROUNDPT;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Plant")) {
         move = PE_MOVE_PLANT;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Poison Lash")) {
         move = PE_MOVE_DOTEL5DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Poisoned Branch")) {
         move = PE_MOVE_DOTEL4DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Quicksand")) {
         move = PE_MOVE_QUICKSAND;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Railgun")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Rock Barrage")) {
         move = PE_MOVE_DOT3STICKDAMAGE050;
         numAbilityValues = 2;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Rupture")) {
         move = PE_MOVE_STUNNED25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Sand Bolt")) {
         move = PE_MOVE_DECSPD125DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Seethe")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Snowball")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Solar Beam")) {
         move = PE_MOVE_EXTRASUNRECHARGE2DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Sons of the Flame")) {
         move = PE_MOVE_SUBMERGE2SUNS;
         numAbilityValues = 1;
         cooldown = PE_EIGHT_ROUNDS;
      }
      else if (!moveName.compare("Sons of the Root")) {
         move = PE_MOVE_SUBMERGE2SUNS;
         numAbilityValues = 1;
         cooldown = PE_EIGHT_ROUNDS;
      }
      else if (!moveName.compare("Spark")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Spore Shrooms")) {
         move = PE_MOVE_SPORE_SHROOMS;
         numAbilityValues = 1;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Sticky Goo")) {
         move = PE_MOVE_ROOTED5DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Stone Rush")) {
         move = PE_MOVE_SIMPLEDAMAGEBACK100;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Stone Shot")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Stun Seed")) {
         move = PE_MOVE_STUNSEED;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Sulfuras Smash")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Sunlight")) {
         move = PE_MOVE_SUNLIGHT;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
	  else if (!moveName.compare("Super Sticky Goo")) {
		  move = PE_MOVE_ROOTED2DAMAGE100;
		  numAbilityValues = 1;
		  cooldown = PE_NO_COOLDOWN;
	  }
	  else if (!moveName.compare("Thorns")) {
         move = PE_MOVE_THORNS;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Thunderbolt")) {
         move = PE_MOVE_TEAMATTACKSPLIT100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Slippery Ice")) {
         move = PE_MOVE_DECACC420DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Soothing Mists")) {
         move = PE_MOVE_HOT5ROUNDSM;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Volcano")) {
         move = PE_MOVE_VOLCANO;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   case PE_FAMILY_FLYING:
      if (!moveName.compare("Accuracy")) {
         move = PE_MOVE_INCHIT425;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Alpha Strike")) {
         move = PE_MOVE_EXTRAFASTDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Anzu\\'s Blessing")) {
         move = PE_MOVE_ANZU;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Caw")) {
         move = PE_MOVE_INCCRIT250;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Cocoon Strike")) {
         move = PE_MOVE_COCOONSTRIKE;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Confusing Sting")) {
         move = PE_MOVE_CONFUSING_STING;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Cyclone")) {
         move = PE_MOVE_CYCLONE;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Flock")) {
         move = PE_MOVE_3ROUNDINCDMG2ROUND100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Flyby")) {
         move = PE_MOVE_INCTARDMG325DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Glowing Toxin")) {
         move = PE_MOVE_GLOWINGTOXIN;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Hawk Eye")) {
         move = PE_MOVE_INCCRIT425;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Lift-Off")) {
         move = PE_MOVE_GOHIGHDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Moth Balls")) {
         move = PE_MOVE_MOTHBALLS;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Moth Dust")) {
         move = PE_MOVE_SLEEPED25HIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Murder")) {
         move = PE_MOVE_3ROUNDINCDMG2ROUND100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Nevermore")) {
         move = PE_MOVE_NEVERMORE;
         numAbilityValues = 0;
         cooldown = PE_TEN_ROUNDS;
      }
      else if (!moveName.compare("Nocturnal Strike")) {
         move = PE_MOVE_BLIND100DAMAGE050;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Reckless Strike")) {
         move = PE_MOVE_INCDMGTAK25HLDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Peck")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Predatory Strike")) {
         move = PE_MOVE_DOUBLEBELOW25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Quills")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Sandstorm")) {
         move = PE_MOVE_SANDSTORM;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Slicing Wind")) {
         move = PE_MOVE_ONETOTHREEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Squawk")) {
         move = PE_MOVE_DECDMG325DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Wild Winds")) {
         move = PE_MOVE_DOT2WINDDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   case PE_FAMILY_HUMANOID:
      if (!moveName.compare("Aged Yolk")) {
         move = PE_MOVE_CLEARUSERDEBUFFS;
         numAbilityValues = 0;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Amber Prison")) {
         move = PE_MOVE_STUN;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Backflip")) {
         move = PE_MOVE_STUNDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Blackout Kick")) {
         move = PE_MOVE_STUN;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Blitz")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Body Slam")) {
         move = PE_MOVE_SIMPLEDAMAGEBACK100;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Bow Shot")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Broom")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Bubble")) {
         move = PE_MOVE_BUBBLE;
         numAbilityValues = 0;
         cooldown = PE_EIGHT_ROUNDS;
      }
      else if (!moveName.compare("Call Darkness")) {
         move = PE_MOVE_CALLDARKNESS;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Charge")) {
         move = PE_MOVE_FASTDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Clobber")) {
         move = PE_MOVE_STUN;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Club")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Counterstrike")) {
         move = PE_MOVE_EXTRAATTDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Crush")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Cute Face")) {
         move = PE_MOVE_CUTE_FACE;
         numAbilityValues = 0;
         cooldown = PE_ZERO_ROUND;
      }
      else if (!moveName.compare("Deflection")) {
         move = PE_MOVE_DEFLECTION;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Dodge")) {
         move = PE_MOVE_DODGE;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Dreadful Breath")) {
         move = PE_MOVE_EXTRARAIN3ROUNDTEAMATTACK100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Eggnog")) {
         move = PE_MOVE_SEARMAGIC;
         numAbilityValues = 0;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Falling Murloc")) {
         move = PE_MOVE_STUNSELFDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Focus")) {
         move = PE_MOVE_INCHIT5100;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Focus Chi")) {
         move = PE_MOVE_INCDMG1HIT1ROUND;
         numAbilityValues = 0;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Frenzyheart Brew")) {
         move = PE_MOVE_INCDMG325DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Frolick")) {
         move = PE_MOVE_FROLICK;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Fury of 1,000 Fists")) {
         move = PE_MOVE_STUNNEDBLINDDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Gauss Rifle")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Haymaker")) {
		  move = PE_MOVE_STUNMISSDAMAGE050;
		  numAbilityValues = 1;
		  cooldown = PE_THREE_ROUNDS;
	  }
	  else if (!moveName.compare("Heroic Leap")) {
		  move = PE_MOVE_GOHIGHDAMAGE100;
		  numAbilityValues = 2;
		  cooldown = PE_FOUR_ROUNDS;
	  }
     else if (!moveName.compare("High Fiber")) {
        move = PE_MOVE_CLEARTEAMDEBUFFS;
        numAbilityValues = 0;
        cooldown = PE_FOUR_ROUNDS;
     }
     else if (!moveName.compare("Holy Charge")) {
        move = PE_MOVE_BUILDUP1DAMAGE100;
        numAbilityValues = 1;
        cooldown = PE_THREE_ROUNDS;
     }
     else if (!moveName.compare("Holy Justice")) {
        move = PE_MOVE_STUN;
        numAbilityValues = 0;
        cooldown = PE_FIVE_ROUNDS;
     }
     else if (!moveName.compare("Holy Sword")) {
        move = PE_MOVE_SIMPLEHIGHLOW100;
        numAbilityValues = 2;
        cooldown = PE_NO_COOLDOWN;
     }
     else if (!moveName.compare("Jab")) {
        move = PE_MOVE_SIMPLEHIGHLOW100;
        numAbilityValues = 2;
        cooldown = PE_NO_COOLDOWN;
     }
     else if (!moveName.compare("Kick")) {
        move = PE_MOVE_STUNDAMAGE100;
        numAbilityValues = 1;
        cooldown = PE_THREE_ROUNDS;
     }
     else if (!moveName.compare("Love Potion")) {
        move = PE_MOVE_HEALFRACTION025FIRST;
        numAbilityValues = 0;
        cooldown = PE_THREE_ROUNDS;
     }
     else if (!moveName.compare("Lovestruck")) {
        move = PE_MOVE_STUN;
        numAbilityValues = 0;
        cooldown = PE_FIVE_ROUNDS;
     }
     else if (!moveName.compare("Macabre Maraca")) {
        move = PE_MOVE_SIMPLEDAMAGE100;
        numAbilityValues = 1;
        cooldown = PE_NO_COOLDOWN;
     }
     else if (!moveName.compare("Omnislash")) {
        move = PE_MOVE_TEAMATTACKSAME100;
        numAbilityValues = 1;
        cooldown = PE_NO_COOLDOWN;
     }
     else if (!moveName.compare("Perfumed Arrow")) {
        move = PE_MOVE_STUNNED25DAMAGE100;
        numAbilityValues = 1;
        cooldown = PE_TWO_ROUNDS;
     }
     else if (!moveName.compare("Punch")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
     else if (!moveName.compare("Punish")) {
        move = PE_MOVE_SIMPLEHIGHLOW100;
        numAbilityValues = 2;
        cooldown = PE_NO_COOLDOWN;
     }
     else if (!moveName.compare("Rapid Fire")) {
        move = PE_MOVE_TEAMATTACKSAME100;
        numAbilityValues = 1;
        cooldown = PE_NO_COOLDOWN;
     }
     else if (!moveName.compare("Reflective Shield")) {
        move = PE_MOVE_REFLECTIVESHIELD;
        numAbilityValues = 0;
        cooldown = PE_THREE_ROUNDS;
     }
     else if (!moveName.compare("Restoration")) {
        move = PE_MOVE_HEALFRACTION025FIRST;
        numAbilityValues = 0;
        cooldown = PE_THREE_ROUNDS;
     }
     else if (!moveName.compare("Shield Block")) {
         move = PE_MOVE_SHIELDBLOCK;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
     else if (!moveName.compare("Shieldstorm")) {
        move = PE_MOVE_SHIELDSTORM;
        numAbilityValues = 0;
        cooldown = PE_EIGHT_ROUNDS;
     }
     else if (!moveName.compare("Shot Through The Heart")) {
        move = PE_MOVE_BUILDUP1DAMAGE100;
        numAbilityValues = 1;
        cooldown = PE_THREE_ROUNDS;
     }
     else if (!moveName.compare("Soul Ward")) {
        move = PE_MOVE_SOULWARD;
        numAbilityValues = 0;
        cooldown = PE_THREE_ROUNDS;
     }
     else if (!moveName.compare("Staggered Steps")) {
        move = PE_MOVE_2DECDMG2ROUND050;
        numAbilityValues = 0;
        cooldown = PE_THREE_ROUNDS;
     }
     else if (!moveName.compare("Surge of Light")) {
         move = PE_MOVE_STUNNED25DARKDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Takedown")) {
         move = PE_MOVE_DOUBLESTUNNEDDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Tornado Punch")) {
         move = PE_MOVE_STUNNED25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Weakening Blow")) {
         move = PE_MOVE_NOBELOW1DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Whirlwind")) {
         move = PE_MOVE_TEAMATTACKSAME100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   case PE_FAMILY_MAGIC:
      if (!moveName.compare("Adrenal Glands")) {
         move = PE_MOVE_ADRENALGLANDS;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
         return PE_NEED_INFO;
      }
      else if (!moveName.compare("Amplify Magic")) {
         move = PE_MOVE_INCDMG2ROUND050;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Arcane Blast")) {
         move = PE_MOVE_INCREASINGDAMAGE100;
         numAbilityValues = 3;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Arcane Explosion")) {
         move = PE_MOVE_TEAMATTACK100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Arcane Slash")) {
         move = PE_MOVE_DIFFERENTFASTDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Arcane Storm")) {
         move = PE_MOVE_ARCANESTORM;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Baneling Burst")) {
         move = PE_MOVE_EXPLODE;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Beam")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
	   else if (!moveName.compare("Blast of Hatred")) {
	 	   move = PE_MOVE_EXTRAATTDAMAGE100;
 		   numAbilityValues = 2;
		   cooldown = PE_NO_COOLDOWN;
	   }
      else if (!moveName.compare("Blessed Hammer")) {
         move = PE_MOVE_THREEATTACKSDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Blingtron Gift Package")) {
         move = PE_MOVE_HEALMISSDAMAGE050;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Blinkstrike")) {
         move = PE_MOVE_FASTDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Celestial Blessing")) {
         move = PE_MOVE_CELESTIALBLESSING;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Centrifugal Hooks")) {
         move = PE_MOVE_CENTRIFUGALHOOKS;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Clean-Up")) {
         move = PE_MOVE_HIGHLOWATTACKOBJECTCLEAR100;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Coin Toss")) {
         move = PE_MOVE_COINTOSS;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Competitive Spirit")) {
         move = PE_MOVE_INCDMG325DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Corrosion")) {
         move = PE_MOVE_CORROSION;
         numAbilityValues = 3;
         cooldown = PE_NO_COOLDOWN;
	  }
     else if (!moveName.compare("Counterspell")) {
        move = PE_MOVE_STUNDAMAGE100;
        numAbilityValues = 1;
        cooldown = PE_THREE_ROUNDS;
     }
     else if (!moveName.compare("Darkmoon Curse")) {
        move = PE_MOVE_DECDMG225INCDMG225DAMAGE100;
        numAbilityValues = 1;
        cooldown = PE_FOUR_ROUNDS;
     }
     else if (!moveName.compare("Drain Power")) {
		  move = PE_MOVE_DECDMG225INCDMG225DAMAGE100;
		  numAbilityValues = 1;
		  cooldown = PE_FOUR_ROUNDS;
	  }
      else if (!moveName.compare("Emerald Bite")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Ethereal")) {
         move = PE_MOVE_FADE;
         numAbilityValues = 0;
         cooldown = PE_EIGHT_ROUNDS;
      }
      else if (!moveName.compare("Expunge")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Evanescence")) {
         move = PE_MOVE_DODGE;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Evolution")) {
         move = PE_MOVE_INCDMG325DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Eyeblast")) {
         move = PE_MOVE_DECSPD425DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Fade")) {
         move = PE_MOVE_FADE;
         numAbilityValues = 0;
         cooldown = PE_EIGHT_ROUNDS;
      }
      else if (!moveName.compare("Feedback")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Flash")) {
         move = PE_MOVE_FLASH;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Focused Beams")) {
         move = PE_MOVE_INCREASINGDAMAGE100;
         numAbilityValues = 3;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Forboding Curse")) {
         move = PE_MOVE_FORBODINGCURSE;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Gift of Winter\\'s Veil")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Gravity")) {
         move = PE_MOVE_GRAVITY;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Illuminate")) {
         move = PE_MOVE_ILLUMINATE;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Illusionary Barrier")) {
         move = PE_MOVE_ILLUSIONARYBARRIER;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Inner Vision")) {
         move = PE_MOVE_INNERVISION;
         numAbilityValues = 0;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Interrupting Gaze")) {
         move = PE_MOVE_STUNDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Invisibility")) {
         move = PE_MOVE_FADING;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Jade Claw")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Jadefire Lightning")) {
         move = PE_MOVE_TEAMATTACKSAME100;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Jadeskin")) {
         move = PE_MOVE_JADESKIN5ROUND;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Laser")) {
         move = PE_MOVE_SIMPLEDAMAGE200;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Lens Flare")) {
         move = PE_MOVE_BLINDED2100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Life Exchange")) {
         move = PE_MOVE_HEALEQUALIZE;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Light")) {
         move = PE_MOVE_DOUBLEBLINDEDDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Magic Hat")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Magic Sword")) {
         move = PE_MOVE_050CRITDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Mana Surge")) {
         move = PE_MOVE_EXTRAARCANE3ROUNDDAMAGE100;
         numAbilityValues = 3;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Metabolic Boost")) {
         move = PE_MOVE_INCSPD100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Meteor Strike")) {
         move = PE_MOVE_GOHIGHDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Moon Dance")) {
         move = PE_MOVE_2INCTEAMSPD9ROUND025;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Moon Fang")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Moon Tears")) {
         move = PE_MOVE_STARFALL;
         numAbilityValues = 2;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Moonfire")) {
         move = PE_MOVE_MOONFIRE;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Nether Blast")) {
         move = PE_MOVE_INCREASINGDAMAGE100;
         numAbilityValues = 3;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Nether Gate")) {
         move = PE_MOVE_SWITCHNEXTDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Nimbus")) {
         move = PE_MOVE_NIMBUS;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Onyx Bite")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Ooze Touch")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Phase Shift")) {
         move = PE_MOVE_DODGE;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Phaser")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Portal")) {
         move = PE_MOVE_PORTAL;
         numAbilityValues = 0;
         cooldown = PE_EIGHT_ROUNDS;
      }
      else if (!moveName.compare("Prismatic Barrier")) {
         move = PE_MOVE_PRISMATICBARRIER;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Psychic Blast")) {
         move = PE_MOVE_EXTRAARCANEDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Righteous Inspiration")) {
         move = PE_MOVE_RIGHTEOUSINSPIRATION;
         numAbilityValues = 0;
         cooldown = PE_EIGHT_ROUNDS;
      }
      else if (!moveName.compare("Sear Magic")) {
         move = PE_MOVE_SEARMAGIC;
         numAbilityValues = 0;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Shadow Talon")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Soulrush")) {
         move = PE_MOVE_STUNNED25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Spectral Spine")) {
         move = PE_MOVE_BLINDED50DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Spectral Strike")) {
         move = PE_MOVE_BLIND100DAMAGE050;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Spirit Spikes")) {
         move = PE_MOVE_SPIRITSPIKES;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Spiritfire Bolt")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Stimpack")) {
         move = PE_MOVE_STIMPACK;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Stoneskin")) {
         move = PE_MOVE_STONESKIN5ROUND;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Surge of Power")) {
         move = PE_MOVE_RECHARGE2DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Sweep")) {
         move = PE_MOVE_SWITCHHIGHESTDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Weakness")) {
         move = PE_MOVE_DECSPD150DECDMG150DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Wild Magic")) {
         move = PE_MOVE_ADDDAMWILD;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Wish")) {
         move = PE_MOVE_HEAL1ROUND;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Zergling Rush")) {
         move = PE_MOVE_3ROUNDINCDMG2ROUND100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   case PE_FAMILY_MECHANICAL:
      if (!moveName.compare("Armageddon")) {
         move = PE_MOVE_ARMAGEDDON;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Booby-Trapped Presents")) {
        move = PE_MOVE_BOOBYTRAPPEDPRESENTS;
        numAbilityValues = 1;
        cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Batter")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Bombing Run")) {
         move = PE_MOVE_BOMBINGRUN;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Build Turret")) {
         move = PE_MOVE_BUILDTURRET;
         numAbilityValues = 1;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Call Lightning")) {
         move = PE_MOVE_CALLLIGHTNING;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Chop")) {
         move = PE_MOVE_DOTBLEED5DAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Decoy")) {
         move = PE_MOVE_DECOY;
         numAbilityValues = 0;
         cooldown = PE_EIGHT_ROUNDS;
      }
      else if (!moveName.compare("Demolish")) {
  		   move = PE_MOVE_SIMPLEDAMAGE050;
 		   numAbilityValues = 1;
		   cooldown = PE_NO_COOLDOWN;
	   }
      else if (!moveName.compare("Explode")) {
         move = PE_MOVE_EXPLODE;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Extra Plating")) {
         move = PE_MOVE_EXTRAPLATING;
         numAbilityValues = 0;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Flux")) {
         move = PE_MOVE_TEAMATTACK100;
         numAbilityValues = 2;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Frying Pan")) {
         move = PE_MOVE_STUNNED25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Greench\\'s Gift")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Heartbroken")) {
         move = PE_MOVE_HEARTBROKEN;
         numAbilityValues = 0;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Inflation")) {
         move = PE_MOVE_3ROUNDINCDMG2ROUND100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Interrupting Jolt")) {
         move = PE_MOVE_STUNDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Ion Cannon")) {
         move = PE_MOVE_RECHARGE2DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Launch")) {
         move = PE_MOVE_GOHIGHDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Launch Rocket")) {
         move = PE_MOVE_LAUCHROCKET;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Lock-On")) {
         move = PE_MOVE_DEBUFFNDUMP100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Make it Rain")) {
         move = PE_MOVE_DOT4STICKDAMAGE080;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Metal Fist")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Minefield")) {
         move = PE_MOVE_MINEFIELD;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Missile")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Overtune")) {
         move = PE_MOVE_INCSPD2100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Quake")) {
         move = PE_MOVE_QUAKE;
         numAbilityValues = 1;
         cooldown = PE_TWO_ROUNDS;
      }
      else if (!moveName.compare("Reboot")) {
         move = PE_MOVE_REPAIR;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Rebuild")) {
         move = PE_MOVE_HEALSELFALLMECHANICAL;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Repair")) {
         move = PE_MOVE_REPAIR;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Screeching Gears")) {
         move = PE_MOVE_STUNNED25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Shock and Awe")) {
         move = PE_MOVE_STUNNED25DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Siphon Anima")) {
         move = PE_MOVE_HEAL100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("SMCKTHAT.EXE")) {
         move = PE_MOVE_EXTRAFASTDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Snap Trap")) {
         move = PE_MOVE_SNAPTRAP;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Sticky Grenade")) {
         move = PE_MOVE_STICKYGRENADE;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Supercharge")) {
         move = PE_MOVE_SUPERCHARGE;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Touch of the Animus")) {
         move = PE_MOVE_TOUCHOFTHEANIMUS;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Toxic Smoke")) {
         move = PE_MOVE_TOXICSMOKE;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Tympanic Tantrum")) {
         move = PE_MOVE_TYMPANICTANTRUM;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Wind-Up")) {
         move = PE_MOVE_WINDUPNDUMP100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("XE-321 Boombot")) {
         move = PE_MOVE_XE321BOOMBOT;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("U-238 Rounds")) {
         move = PE_MOVE_EXTRAFASTONETWO100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Zap")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   case PE_FAMILY_UNDEAD:
      if (!moveName.compare("Absorb")) {
         move = PE_MOVE_HEAL100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Agony")) {
		   move = PE_MOVE_AGONY;
		   numAbilityValues = static_cast<PE_AN>(5 + 0.25*power);
		   cooldown = PE_NO_COOLDOWN;
		   return PE_NEED_INFO;
	   }
      else if (!moveName.compare("Bone Barrage")) {
         move = PE_MOVE_DOT4STICKDAMAGE080;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Bone Bite")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Bone Prison")) {
         move = PE_MOVE_ROOTED2DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("BONESTORM")) {
         move = PE_MOVE_BONESTORM;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Consume")) {
         move = PE_MOVE_HEAL100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Consume Corpse")) {
         move = PE_MOVE_CONSUME_CORPSE;
         numAbilityValues = 0;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Corpse Explosion")) {
         move = PE_MOVE_CORPSEEXPLOSION;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Creeping Fungus")) {
         move = PE_MOVE_CREEPING_FUNGUS;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Creepy Chomp")) {
         move = PE_MOVE_SIMPLEDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Curse of Doom")) {
         move = PE_MOVE_CURSEOFDOOM;
         numAbilityValues = 1;
         cooldown = PE_FIVE_ROUNDS;
      }
      else if (!moveName.compare("Dark Simulacrum")) {
         move = PE_MOVE_NULL;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Dead Man\\'s Party")) {
         move = PE_MOVE_3ROUNDINCDMG2ROUND100DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Death and Decay")) {
         move = PE_MOVE_DOT9DEATH;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Death Coil")) {
         move = PE_MOVE_HEAL050DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Death Grip")) {
         move = PE_MOVE_SWITCHLOWESTDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Diseased Bite")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Drain Blood")) {
         move = PE_MOVE_DRAINBLOOD;
         numAbilityValues = 0;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Ghostly Bite")) {
         move = PE_MOVE_STUNSELFDAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Haunt")) {
         move = PE_MOVE_HAUNT;
         numAbilityValues = 1;
         cooldown = PE_FOUR_ROUNDS;
      }
      else if (!moveName.compare("Infected Claw")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Leech Life")) {
         move = PE_MOVE_HEALDOUBLEWEBDAMAGE100;
         numAbilityValues = 2;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Plagued Blood")) {
         move = PE_MOVE_PLAGUEDBLOOD;
         numAbilityValues = 1;
         cooldown = PE_ONE_ROUND;
      }
      else if (!moveName.compare("Rabid Strike")) {
         move = PE_MOVE_INCDMG325INCDMG350DAMAGE100;
         numAbilityValues = 1;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Rot")) {
         move = PE_MOVE_ROT;
         numAbilityValues = 1;
         cooldown = PE_THREE_ROUNDS;
      }
      else if (!moveName.compare("Shadow Shock")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Shadow Slash")) {
         move = PE_MOVE_SIMPLEHIGHLOW100;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Siphon Life")) {
         move = PE_MOVE_SIPHONLIFE;
         numAbilityValues = 2;
         cooldown = PE_NO_COOLDOWN;
      }
      else if (!moveName.compare("Unholy Ascension")) {
         move = PE_MOVE_UNHOLYASCENSION;
         numAbilityValues = 0;
         cooldown = PE_NO_COOLDOWN;
      }
      else {
         return PE_UNKNOWN_MOVE;
      }
      break;
   default:
      return PE_OUT_OF_RANGE;
   }

   if (move == PE_INVALID_MOVE) {
      return PE_UNKNOWN_MOVE;
   }
   if (numAbilityValues == PE_INVALID_ABILITYNUMBER) {
      return PE_UNKNOWN_MOVE;
   }
   if (cooldown == PE_INVALID_COOLDOWN) {
      return PE_UNKNOWN_MOVE;
   }

   return PE_OK;
}



