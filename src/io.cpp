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

#include <stdio.h>
#include <string.h>
#include <string>
#include "io.h"
#include "petinfo.h"

Io::Io(int argc, char *argv[]) {

   m_needHelp = false;
   m_dump = false;
   m_teamsLoaded = false;
   m_depth = PE_DEFAULT_DEPTH;
   m_games = PE_DEFAULT_GAMES;
   if (argc < 3) {
      m_needHelp = true;
   }
   else if (argc == 3) {
      m_infoFile.assign(argv[1]);
      if (!strcmp(argv[2], "dump")) {
         m_dump = true;
      }
      else {
         m_needHelp = true;
      }
   }
   else if (argc == 4) {
      m_infoFile.assign(argv[1]);
      if (!strcmp(argv[2], "hh") || !strcmp(argv[2], "HH")) {
         m_playMode = PE_PM_TWOHUMANS;
      }
      else if (!strcmp(argv[2], "hc") || !strcmp(argv[2], "HC")) {
         m_playMode = PE_PM_HUMANCOMP;
      }
      else if (!strcmp(argv[2], "cc") || !strcmp(argv[2], "CC")) {
         m_playMode = PE_PM_TWOCOMPS;
      }
      else {
         m_needHelp = true;
      }
      if (this->loadTeams(argv[3])) {
         m_teamsLoaded = true;
      }
   }
   else if (argc == 5) {
      m_infoFile.assign(argv[1]);
      if (!strcmp(argv[2], "hh") || !strcmp(argv[2], "HH")) {
         m_playMode = PE_PM_TWOHUMANS;
      }
      else if (!strcmp(argv[2], "hc") || !strcmp(argv[2], "HC")) {
         m_playMode = PE_PM_HUMANCOMP;
      }
      else if (!strcmp(argv[2], "cc") || !strcmp(argv[2], "CC")) {
         m_playMode = PE_PM_TWOCOMPS;
      }
      else {
         m_needHelp = true;
      }
      if (this->loadTeams(argv[3])) {
         m_teamsLoaded = true;
      }
      int depth;
      depth = *argv[4] - '0';
      if (depth < 1 || depth > 9) {
         m_needHelp = true;
      }
      else m_depth = depth + 1;
      if (m_playMode != PE_PM_TWOCOMPS && m_playMode != PE_PM_HUMANCOMP) m_needHelp = true;
   }
   else if (argc == 6) {
      m_infoFile.assign(argv[1]);
      if (!strcmp(argv[2], "hh") || !strcmp(argv[2], "HH")) {
         m_playMode = PE_PM_TWOHUMANS;
      }
      else if (!strcmp(argv[2], "hc") || !strcmp(argv[2], "HC")) {
         m_playMode = PE_PM_HUMANCOMP;
      }
      else if (!strcmp(argv[2], "cc") || !strcmp(argv[2], "CC")) {
         m_playMode = PE_PM_TWOCOMPS;
      }
      else {
         m_needHelp = true;
      }
      if (this->loadTeams(argv[3])) {
         m_teamsLoaded = true;
      }
      int depth;
      depth = *argv[4] - '0';
      if (depth < 1 || depth > 9) {
         m_needHelp = true;
      }
      else m_depth = depth + 1;
      int games = strtol(argv[5], (char **)NULL, 0);
      if (games < 1 || games > PE_MAX_GAMES) m_needHelp = true;
      else m_games = games;
      if (m_playMode != PE_PM_TWOCOMPS) m_needHelp = true;
   }
}

bool Io::copyTeams(Petinfo &pets, PE_TEAM_SELECTION &teamA, PE_TEAM_SELECTION &teamB) {

   if (!m_teamsLoaded) return false;
   for (int pet = 0; pet < PE_TEAM_SIZE; pet++) {
      if (m_teamA.pet[pet].petNumber < 0 || m_teamA.pet[pet].petNumber >= pets.numberPets()) return false;
      teamA.pet[pet].petNumber = m_teamA.pet[pet].petNumber;
      if (m_teamA.pet[pet].breedNumber < 0 || m_teamA.pet[pet].breedNumber >= pets.numberBreeds(m_teamA.pet[pet].petNumber)) return false;
      teamA.pet[pet].breedNumber = m_teamA.pet[pet].breedNumber;
      if (m_teamB.pet[pet].petNumber < 0 || m_teamB.pet[pet].petNumber >= pets.numberPets()) return false;
      teamB.pet[pet].petNumber = m_teamB.pet[pet].petNumber;
      if (m_teamB.pet[pet].breedNumber < 0 || m_teamB.pet[pet].breedNumber >= pets.numberBreeds(m_teamB.pet[pet].petNumber)) return false;
      teamB.pet[pet].breedNumber = m_teamB.pet[pet].breedNumber;
      for (int ability = 0; ability < PE_ABILITIES; ability++) {
         teamA.pet[pet].abilitySelection[ability] = m_teamA.pet[pet].abilitySelection[ability];
         teamB.pet[pet].abilitySelection[ability] = m_teamB.pet[pet].abilitySelection[ability];
      }
   }

   return true;
}


bool Io::loadTeams(const char *filename) {
   errno_t err;
   FILE *fid;
   char line[256], *finger, *next;
   long int dummy, ability;
   int onPet = 0;

   err = fopen_s(&fid, filename, "r");
   if (err != NULL) {
      printf("can not open team file\n");
      return false;
   }
   while (!feof(fid)) {
      finger = fgets(line, 255, fid);
      if (!finger) continue;
      finger = strchr(line, '#');
      if (finger) *finger = '\0';
      next = line;
      for (finger = next; (*finger == ' ' || *finger == '\t') && *finger != '\0'; finger++) {}
      if (*finger == '\0') continue;
      if (!(dummy = strtol(finger, &next, 0))) continue;

      if (onPet >= PE_TEAM_SIZE) m_teamA.pet[onPet].petNumber = dummy - 1;
      else m_teamB.pet[onPet - PE_TEAM_SIZE].petNumber = dummy - 1;

      for (finger = next; (*finger == ' ' || *finger == '\t') && *finger != '\0'; finger++) {}
      if (*finger == '\0') continue;
      if (!(dummy = strtol(finger, &next, 0))) continue;

      if (onPet >= PE_TEAM_SIZE) m_teamA.pet[onPet].breedNumber = dummy - 1;
      else m_teamB.pet[onPet - PE_TEAM_SIZE].breedNumber = dummy - 1;

      for (finger = next; (*finger == ' ' || *finger == '\t') && *finger != '\0'; finger++) {}
      for (ability = 0; ability < PE_ABILITIES; ability++) {
         if (*finger == '\0') break;
         if (*finger < '1' || *finger > '2') break;
         if (onPet >= PE_TEAM_SIZE) m_teamA.pet[onPet].abilitySelection[ability] = *finger - '1';
         else m_teamB.pet[onPet - PE_TEAM_SIZE].abilitySelection[ability] = *finger - '1';
         finger++;
      }
      if (ability == PE_ABILITIES) onPet++;
   }
   fclose(fid);

   if (onPet == PE_SIDES*PE_TEAM_SIZE) return true;
   else return false;
}

void Io::printHelp() {

   printf("usage: paddlebets <data file> <hh/hc/cc> <team file> [<depth (1-9)>] [<# of games>]\n");
}
