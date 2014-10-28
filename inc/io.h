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

#ifndef IO_H
#define IO_H

#include <string>
#include "constants.h"
#include "petinfo.h"

typedef enum PE_PLAYMODE {
   PE_PM_TWOHUMANS,
   PE_PM_HUMANCOMP,
   PE_PM_TWOCOMPS,
   PE_PLAYMODES
};

#define PE_DEFAULT_DEPTH 4
#define PE_DEFAULT_GAMES 100
#define PE_MAX_GAMES 16777215

using namespace std;

class Io {

public:

   Io(int argc, char *argv[]);
   void printHelp();

   bool needHelp() { return m_needHelp; }
   bool dump() { return m_dump; }
   const char *infoFile() { return m_infoFile.c_str(); }
   bool copyTeams(Petinfo &pets, PE_TEAM_SELECTION &teamA, PE_TEAM_SELECTION &teamB);
   PE_PLAYMODE playMode() { return m_playMode; }
   int depth() { return m_depth; }
   int games() { return m_games; }

private:

   bool loadTeams(const char *filename);

   bool m_needHelp;
   bool m_dump;
   string m_infoFile;
   PE_TEAM_SELECTION m_teamA, m_teamB;
   bool m_teamsLoaded;
   PE_PLAYMODE m_playMode;
   int m_depth, m_games;
};

#endif
