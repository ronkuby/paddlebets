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

#ifndef PETINFO_H
#define PETINFO_H

#include <stdint.h>
#include <vector>
#include <string>
#include "constants.h"

using namespace std;

class Petinfo {

public:
   Petinfo(const char *filename, const bool dump);

   PE_FAMILY getFamily(const PE_PN &pet) { return m_petFamily[pet]; }
   PE_GENUS getGenus(const PE_PN &pet);
   vector<PE_SV> getStats(const PE_PN &pet, const PE_BN &breed) { return m_petStats[pet][breed]; }

   string getAbilityName(const PE_PN pet, const PE_ABILITY ability, PE_FAMILY &family);
   string getPetName(const PE_PN &pet) { return m_petName[pet]; };

   // function that takes a string (ability) and power and returns several numbers.
   // the first is an enumeratored move type.  the calling function then looks up the move type and
   // fills in the internal move function pointer appropriately
   // others are the family type for the damage, required number of move numbers, duration, others
   // note the other information is stored as additional internal variables and only used by
   // the move function as needed
   PE_ERROR getMove(const string &moveName, const PE_SV &power, const PE_FAMILY &family, PE_MOVE &move, PE_AN &numAbilityValues, PE_DURATION &cooldown);
   PE_ERROR getAbilityValues(const PE_PN &pet, const PE_BN &breed, const PE_ABILITY ability, vector<PE_AV> &abilityValues);
   PE_ERROR checkAbilities();

   PE_PN numberPets() { return m_pets; }
   PE_BN numberBreeds(const PE_PN &pet) { return m_petBreeds[pet]; }

private:
   // pet stuff
   PE_PN m_pets;
   vector<PE_FAMILY> m_petFamily;
   vector<string> m_petGenus;
   vector<string> m_petName;
   vector<PE_BN> m_petBreeds;
   vector<vector<PE_AN> > m_petAbility;
   vector<vector<vector<PE_SV> > > m_petStats;
   vector<vector<vector<vector<PE_AV> > > > m_abilityValues;

   // ability stuff
   int32_t m_abilities;
   vector<PE_FAMILY> m_abilityFamily;
   vector<string> m_abilityName;

};

#endif
