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

#ifndef TVECTOR_H
#define TVECTOR_H

#include <vector>
#include <assert.h>
#include "debuff.h"

using namespace std;

const int PE_NO_TRACK = -1;

class Tvector {

public:
   void pushBuff(const Debuff &debuff) { m_debuff.push_back(debuff); }
   int initTrack() {
      m_track.push_back(0);
      m_viable.push_back(true);
      return static_cast<int>(m_track.size() - 1);
   }
   bool notEnd(const uint32_t track) {
#if PE_VERBOSE > PE_VERBOSE_OFF
      assert(track < m_track.size() && track >= 0);
#endif
      if (m_track[track] >= m_debuff.size()) { // could lose more than 1 track
#if PE_VERBOSE > PE_VERBOSE_OFF
         assert(track == m_track.size() - 1);
#endif
         m_track.pop_back();
         m_viable.pop_back();
         return false;
      }
      else {
         return true;
      }
   }
   void end(const uint32_t track) {
#if PE_VERBOSE > PE_VERBOSE_OFF
      assert(track < m_track.size() && track >= 0);
      assert(track == m_track.size() - 1);
#endif
      m_track.pop_back();
      m_viable.pop_back();
   }
   void next(const uint32_t track) {
#if PE_VERBOSE > PE_VERBOSE_OFF
      assert(track < m_track.size() && track >= 0);
#endif
      m_track[track]++;
      m_viable[track] = true;
   }
   Debuff *on(const uint32_t track) {
#if PE_VERBOSE > PE_VERBOSE_OFF
      assert(track < m_track.size() && track >= 0);
      assert(m_track[track] < m_debuff.size());
#endif
      return &m_debuff[m_track[track]];
   }
   int onNum(const uint32_t track) {
#if PE_VERBOSE > PE_VERBOSE_OFF
      assert(track < m_track.size() && track >= 0);
      assert(m_track[track] < m_debuff.size());
#endif
      return m_track[track];
   }
   vector<Debuff> *list() { return &m_debuff; } // only for removedebuff and the booleans!
   void erase(const uint32_t theTrack) {
      uint32_t number = m_track[theTrack];
#if PE_VERBOSE > PE_VERBOSE_OFF
      assert(number >= 0);
      assert(number < m_debuff.size());
#endif
      for (uint32_t track = 0; track < m_track.size(); track++) {
// #if PE_VERBOSE > PE_VERBOSE_OFF    USE VIABLE INSTEAD.  we can remove a track that we are on now
//         if (track != theTrack) {
//            assert(m_track[track] != number);
//         }
// #endif
         if (m_track[track] <= number) {
            m_track[track]--;
         }
         if (m_track[track] == number) {
            m_viable[track] = false;
         }
      }
      m_debuff.erase(m_debuff.begin() + number);
   }
   bool viable(const uint32_t track) { return m_viable[track]; }

   Debuff *operator[](const uint32_t buffNumber) {
      return &m_debuff[buffNumber];
   }

#if PE_VERBOSE > PE_VERBOSE_OFF
   int tracks() { return static_cast<int>(m_track.size()); }
   void checkNoTracks() {
      assert(m_track.size() == 0);
   }
#endif

private:
   vector<Debuff>    m_debuff;
   vector<uint32_t>  m_track;
   vector<bool>      m_viable; // false when a track is on a debuff which has been deleted
};

#endif
