#include <stdio.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <tuple>
#include <array>
#include "constants.h"
#include "petinfo.h"
#include "battle.h"
#include "turn.h"
#include "solver.h"
#include "matrix.h"
#include "io.h"

int32_t randInt(const int32_t maxInt);
int32_t simpleValue(Battle &mainBattle);
double SMAB(Battle &mainBattle, vector<vector<int> > &move, vector<vector<double> > &x, const int &depth);
double getPESMAB(Battle &mainBattle, const PE_SIDE firstSide, vector<vector<int> > &move, const vector<int> &s, const int &depth);

void play(Battle &battle, const int &depth, char *filename);
//void executeML(Battle &mainBattle, const vector<int> &move);
bool executeProp(Battle &mainBattle, const vector<int> &move);

int regression(Petinfo &pets);

#define TEAM 1
#define SEED 1

int main(int argc, char *argv[]) {

   Io args(argc, argv);
   if (args.needHelp()) {
      args.printHelp();
      return -1;
   }

   Petinfo pets(args.infoFile(), args.dump()); // load pets
   pets.checkAbilities();
#ifdef SEED
   int seed = static_cast<int>(time(0));
   srand(seed);
#endif
   srand(0);
   // set up team
   PE_TEAM_SELECTION teamA, teamB;

   exit(1);
   if (true) {
      return regression(pets);
   }

   for (int pet = PE_PET1; pet < PE_TEAM_SIZE; pet++) {
      for (teamA.pet[pet].petNumber = randInt(pets.numberPets()); !pets.numberBreeds(teamA.pet[pet].petNumber);) teamA.pet[pet].petNumber = randInt(pets.numberPets());
      //teamA.pet[0].petNumber = MAX_PETS - 1;
      teamA.pet[pet].breedNumber = randInt(pets.numberBreeds(teamA.pet[pet].petNumber));
      for (teamB.pet[pet].petNumber = randInt(pets.numberPets()); !pets.numberBreeds(teamB.pet[pet].petNumber);) teamB.pet[pet].petNumber = randInt(pets.numberPets());
      teamB.pet[pet].breedNumber = randInt(pets.numberBreeds(teamB.pet[pet].petNumber));
      for (int move = PE_SLOT1; move < PE_ABILITIES; move++) {
         teamA.pet[pet].abilitySelection[move] = randInt(PE_SELECTIONS);
         teamB.pet[pet].abilitySelection[move] = randInt(PE_SELECTIONS);
      }
   }

#ifdef TEAM 
   Battle mainBattle(pets, teamA, teamB);
#else
   Battle mainBattle(pets, teamB, teamA);
#endif
   play(mainBattle, PE_INFINITE, "C:\\code\\bp\\x64\\Release\\doodle.txt");

   mainBattle.dumpSituation();
   while (mainBattle.simpleEval() > 0 && mainBattle.simpleEval() < maxEval) {
      vector<vector<int> > move;
      vector<vector<double> > x;
      for (int depth = 1; depth < 6; depth++) {
         move.clear();
         x.clear();
         double value = SMAB(mainBattle, move, x, depth);
         printf("  depth %d nodes %8d value %4.0f time %9.3f\n", depth, mainBattle.nodes(), value, mainBattle.time());
         if (value == 0 || value == maxEval) break;
      }
      int bestRow = 0, bestCol = 0;
	  for (uint32_t row = 1; row < x[PE_SIDEA].size(); row++) {
         if (x[PE_SIDEA][row] > x[PE_SIDEA][bestRow]) bestRow = row;
      }
	  for (uint32_t col = 1; col < x[PE_SIDEB].size(); col++) {
         if (x[PE_SIDEB][col] > x[PE_SIDEB][bestCol]) bestCol = col;
      }

      vector<int> theMove(PE_SIDES, 0);
      theMove[0] = move[PE_SIDEA][bestRow];
      theMove[1] = move[PE_SIDEB][bestCol];
      //executeML(mainBattle, theMove);
      bool gameOver = executeProp(mainBattle, theMove);
      printf("turn %d best = %f %f and %d %d eval = %4d\n", mainBattle.turn(), x[PE_SIDEA][bestRow], x[PE_SIDEB][bestCol], move[PE_SIDEA][bestRow], move[PE_SIDEB][bestCol], mainBattle.simpleEval());
      //break;
      //if (mainBattle.turn() == 4) break;
      if (gameOver) break;
   }
   PE_SIDE winner;

   mainBattle.gameOver(winner);
   printf("Winner = %d\n", winner);

   return 0;
}

double vab(Battle &mainBattle, const bool &switchOnly, vector<vector<int> > &move, vector<int> &ab, int depth) {
   double v;
   vector<PE_SV> speed;

   if (switchOnly) {
      Battle localBattle(mainBattle);
      localBattle.doSimpleMove(PE_SIDEA, move[0][ab[0]]);
      localBattle.doSimpleMove(PE_SIDEB, move[1][ab[1]]);
      vector<vector<int> > nextMove;
      vector<vector<double> > nextX;
      v = SMAB(localBattle, nextMove, nextX, depth - 1);
   }
   else {
      mainBattle.getSpeeds(move[0][ab[0]], move[1][ab[1]], speed);
            
      // check whether we have to split the moves
      if (speed[PE_SIDEA] == speed[PE_SIDEB] && speed[PE_SIDEA] != PE_INFINITE && speed[PE_SIDEA] != 0) {
         v = (getPESMAB(mainBattle, PE_SIDEA, move, ab, depth - 1) + getPESMAB(mainBattle, PE_SIDEB, move, ab, depth - 1))/2;
      }
      else { // clear first move
         if (speed[PE_SIDEA] > speed[PE_SIDEB]) {
            v = getPESMAB(mainBattle, PE_SIDEA, move, ab, depth - 1);
         }
         else {
            v = getPESMAB(mainBattle, PE_SIDEB, move, ab, depth - 1);
         }
      }
   }

   return v;
}

vector<vector<int> > pruneMoves(Battle &mainBattle, const vector<vector<int> > &move, const int &depth) {
   vector<vector<int> > newMove;
   vector<vector<int> > adup;
   vector<vector<int> > bdup;
   vector<double> mao, mad;
   vector<double> mbo, mbd;
   vector<int> s(PE_SIDES, 0);
   int bestA, bestB, best;

   newMove.resize(PE_SIDES);
   adup.resize(PE_SIDES);
   bdup.resize(PE_SIDES);

   // begin sideA offensive move adds
   bdup[PE_SIDEB].push_back(PE_PASS);
   bdup[PE_SIDEA] = move[PE_SIDEA];
   s[PE_SIDEB] = 0;
   for (uint32_t i = 0; i < move[PE_SIDEA].size(); i++) {
      if (move[PE_SIDEA][i] == PE_PASS) {
         mao.push_back(0);
      }
      else {
         s[PE_SIDEA] = i;
         mao.push_back(vab(mainBattle, false, bdup, s, depth - nullDepth));
      }
   }
   bestA = 0;
   for (uint32_t i = 1; i < move[PE_SIDEA].size(); i++) {
      if (mao[i] > mao[bestA]) {
         bestA = i;
      }
   }

   // begin side b offensive move adds
   adup[PE_SIDEA].push_back(PE_PASS);
   adup[PE_SIDEB] = move[PE_SIDEB];
   s[PE_SIDEA] = 0;
   for (uint32_t i = 0; i < move[PE_SIDEB].size(); i++) {
      if (move[PE_SIDEB][i] == PE_PASS) {
         mbo.push_back(maxEval);
      }
      else {
         s[PE_SIDEB] = i;
         mbo.push_back(vab(mainBattle, false, adup, s, depth - nullDepth));
      }
   }
   bestB = 0;
   for (uint32_t i = 1; i < move[PE_SIDEB].size(); i++) {
      if (mbo[i] < mbo[bestB]) {
         bestB = i;
      }
   }

   // begin side a defensive moves
   bdup[PE_SIDEB][0] = move[PE_SIDEB][bestB];
   s[PE_SIDEB] = 0;
   for (uint32_t i = 0; i < move[PE_SIDEA].size(); i++) {
      if (move[PE_SIDEA][i] == PE_PASS) {
         mad.push_back(0);
      }
      else {
         s[PE_SIDEA] = i;
         mad.push_back(vab(mainBattle, false, bdup, s, depth - nullDepth));
      }
   }
   best = 0;
   for (uint32_t i = 1; i < move[PE_SIDEA].size(); i++) {
      if (mad[i] > mad[best]) {
         best = i;
      }
   }
   newMove[PE_SIDEA].push_back(move[PE_SIDEA][best]);
   for (uint32_t i = 0; i < move[PE_SIDEA].size(); i++) {
      if (mad[i] + nullValue > mad[best] && i != best) {
         newMove[PE_SIDEA].push_back(move[PE_SIDEA][i]);
      }
   }

   // begin side b defensive move adds
   adup[PE_SIDEA][0] = move[PE_SIDEA][bestA];
   s[PE_SIDEA] = 0;
   for (uint32_t i = 0; i < move[PE_SIDEB].size(); i++) {
      if (move[PE_SIDEB][i] == PE_PASS) {
         mbd.push_back(maxEval);
      }
      else {
         s[PE_SIDEB] = i;
         mbd.push_back(vab(mainBattle, false, adup, s, depth - nullDepth));
      }
   }
   best = 0;
   for (uint32_t i = 1; i < move[PE_SIDEB].size(); i++) {
      if (mbd[i] < mbd[best]) {
         best = i;
      }
   }
   newMove[PE_SIDEB].push_back(move[PE_SIDEB][best]);
   for (uint32_t i = 0; i < move[PE_SIDEB].size(); i++) {
      if (mbd[i] - nullValue < mbd[best] && i != best) {
         newMove[PE_SIDEB].push_back(move[PE_SIDEB][i]);
      }
   }

   return newMove;
}

vector<vector<int> > prunePassSwitch(Battle &mainBattle, const vector<vector<int> > &move) {
   vector<vector<int> > newMove;

   newMove.resize(PE_SIDES);
   for (uint32_t i = 0; i < move[PE_SIDEA].size(); i++) {
      if (move[PE_SIDEA][i] < PE_ABILITIES) {
         newMove[PE_SIDEA].push_back(move[PE_SIDEA][i]);
      }
   }
   for (uint32_t i = 0; i < move[PE_SIDEB].size(); i++) {
      if (move[PE_SIDEB][i] < PE_ABILITIES) {
         newMove[PE_SIDEB].push_back(move[PE_SIDEB][i]);
      }
   }

   return newMove;
}

vector<vector<int> > prunePass(Battle &mainBattle, const vector<vector<int> > &move) {
   vector<vector<int> > newMove;
   bool haveAb;

   newMove.resize(PE_SIDES);
   haveAb = false;
   for (uint32_t i = 0; i < move[PE_SIDEA].size(); i++) {
      if (move[PE_SIDEA][i] < PE_ABILITIES) {
         newMove[PE_SIDEA].push_back(move[PE_SIDEA][i]);
         haveAb = true;
      }
      else if (move[PE_SIDEA][i] < PE_PASS) {
         newMove[PE_SIDEA].push_back(move[PE_SIDEA][i]);
      }
      else if (!haveAb) {
         newMove[PE_SIDEA].push_back(move[PE_SIDEA][i]);
      }
   }
   haveAb = false;
   for (uint32_t i = 0; i < move[PE_SIDEB].size(); i++) {
      if (move[PE_SIDEB][i] < PE_ABILITIES) {
         haveAb = true;
         newMove[PE_SIDEB].push_back(move[PE_SIDEB][i]);
      }
      else if (move[PE_SIDEB][i] < PE_PASS) {
         newMove[PE_SIDEB].push_back(move[PE_SIDEB][i]);
      }
      else if (!haveAb) {
         newMove[PE_SIDEB].push_back(move[PE_SIDEB][i]);
      }
   }

   return newMove;
}

double SMAB(Battle &mainBattle, vector<vector<int> > &move, vector<vector<double> > &x, const int &depth) {
   bool switchOnly;
   vector<int> s(PE_SIDES, 0);
   double currentValue = static_cast<double>(simpleValue(mainBattle));
   PE_ERROR errVal;

   mainBattle.incTurn();
   if (depth <= 0) {
      mainBattle.decTurn();
      return currentValue;
   }

   mainBattle.getMoves(move, switchOnly);

   if (!switchOnly && move[PE_SIDEA].size() > sizeSkipPass && move[PE_SIDEB].size() > sizeSkipPass) {
      if (depth >= nullDepth) {
         move = pruneMoves(mainBattle, move, depth);
      }
      else if (depth == 1 && !switchOnly) {
         move = prunePassSwitch(mainBattle, move);
      }
   }
   else {
      move = prunePass(mainBattle, move);
   }

   Matrix V(move[0].size(), move[1].size());
   V.set(0);

   for (int32_t mn = 0; mn < V.size(0)*V.size(1); mn++) {

      s[PE_SIDEA] = mn%move[0].size();
      s[PE_SIDEB] = mn/move[0].size();

      V(s[PE_SIDEA], s[PE_SIDEB]) = vab(mainBattle, switchOnly, move, s, depth);
   }

   // return the nash equilibrium
   double value;
   int32_t row, col;

   V.stabalize();
   if (V.pure(row, col, MATRIX_BOTH) == PE_OK) {
      value = V(row, col);
      x.resize(PE_SIDES);
      x[PE_SIDEA].resize(move[PE_SIDEA].size(), 0.0);
      x[PE_SIDEB].resize(move[PE_SIDEB].size(), 0.0);
      x[PE_SIDEA][row] = 1.0;
      x[PE_SIDEB][col] = 1.0;
   }
   else {
      Solver solve(V, MATRIX_BOTH);
      value = solve.newDoubleSolve(errVal, x);
      if (errVal == PE_SOLVER_NOSOLUTION) {
         value = V.average();
         x.clear();
         x.resize(PE_SIDES);
		 for (uint32_t amove = 0; amove < move[PE_SIDEA].size(); amove++) x[PE_SIDEA].push_back(1.0 / move[PE_SIDEA].size());
		 for (uint32_t bmove = 0; bmove < move[PE_SIDEB].size(); bmove++) x[PE_SIDEB].push_back(1.0 / move[PE_SIDEB].size());
      }
   }
   /*if ((depth == 4 && poop1) || (depth == 3 && poop2) || poop3) {
      for (int amove = 0; amove < move[PE_SIDEA].size(); amove++) printf("0%d: %d %d %f\r\n", depth, amove, move[PE_SIDEA][amove], x[PE_SIDEA][amove]);
      for (int bmove = 0; bmove < move[PE_SIDEB].size(); bmove++) printf("1%d: %d %d %f\r\n", depth, bmove, move[PE_SIDEB][bmove], x[PE_SIDEB][bmove]);
      V.dump();
   }*/
   /*if (depth == 4) {
      for (int amove = 0; amove < move[PE_SIDEA].size(); amove++) printf("0%d: %d %d %f\r\n", depth, amove, move[PE_SIDEA][amove], x[PE_SIDEA][amove]);
      for (int bmove = 0; bmove < move[PE_SIDEB].size(); bmove++) printf("1%d: %d %d %f\r\n", depth, bmove, move[PE_SIDEB][bmove], x[PE_SIDEB][bmove]);
      V.dump();
   }*/

   mainBattle.decTurn();

   return value;
}

double getPESMAB(Battle &mainBattle, const PE_SIDE firstSide, vector<vector<int> > &move, const vector<int> &s, const int &depth) {
   PE_ERROR err;
   PE_SIDE secondSide = firstSide^1;
   vector<Turn> turn;
   turn.resize(PE_SIDES + 1);
   double pCheck = 0.0, runningProb = 0.0;
   double v;
   PE_SIDE winner;

   err = mainBattle.getBranching(firstSide, move[firstSide][s[firstSide]], turn[firstSide]);
   for (int32_t brancha = 0; brancha < turn[firstSide].branches(); brancha++) {
      if (turn[firstSide].prob(brancha) < ignoreBranch) continue;
      Battle halfBattle(mainBattle);

      halfBattle.doMove(firstSide, move[firstSide][s[firstSide]], turn[firstSide].type(brancha));
      if (halfBattle.gameOver(winner)) {
         runningProb += turn[firstSide].prob(brancha)*static_cast<double>(maxEval)*(winner^1);
         pCheck += turn[firstSide].prob(brancha);
         continue;
      }
      err = halfBattle.getBranching(secondSide, move[secondSide][s[secondSide]], turn[secondSide]);
	  for (int32_t branchb = 0; branchb < turn[secondSide].branches(); branchb++) {
         if (turn[secondSide].prob(branchb) < ignoreBranch) continue;
         double jointProb = turn[firstSide].prob(brancha)*turn[secondSide].prob(branchb);
         if (jointProb < ignoreJoint) continue;
         Battle fullBattle(halfBattle);

         fullBattle.doMove(secondSide, move[secondSide][s[secondSide]], turn[secondSide].type(branchb));
         if (fullBattle.gameOver(winner)) {
            v = static_cast<double>(maxEval)*(winner^1);
            runningProb += jointProb*v;
            pCheck += jointProb;
         }
         else {
            err = fullBattle.getBranchingEndturn(turn[PE_SIDES]);
            if (turn[PE_SIDES].branches() > 1) {
               for (int32_t branchc = 0; branchc < turn[PE_SIDES].branches(); branchc++) {
                  double hugProb = jointProb*turn[PE_SIDES].prob(branchc);
                  Battle endBattle(fullBattle);

                  endBattle.finishTurn(turn[PE_SIDES], turn[PE_SIDES].type(branchc));
                  if (endBattle.gameOver(winner)) {
                     v = static_cast<double>(maxEval)*(winner^1);
                  }
                  else {
                     vector<vector<int> > nextMove;
                     vector<vector<double> > nextX;
                     if (hugProb >= primaryJoint) {
                        v = SMAB(endBattle, nextMove, nextX, depth);
                     }
                     else {
                        v = SMAB(endBattle, nextMove, nextX, depth - 1);
                     }
                  }
                  runningProb += hugProb*v;
                  pCheck += hugProb;
               }
            }
            else {
               fullBattle.finishTurn(turn[PE_SIDES], 0);
               if (fullBattle.gameOver(winner)) {
                  v = static_cast<double>(maxEval)*(winner^1);
               }
               else {
                  vector<vector<int> > nextMove;
                  vector<vector<double> > nextX;
                  if (jointProb >= primaryJoint) {
                     v = SMAB(fullBattle, nextMove, nextX, depth);
                  }
                  else {
                     v = SMAB(fullBattle, nextMove, nextX, depth - 1);
                  }
               }
               runningProb += jointProb*v;
               pCheck += jointProb;
            }
         }
      }
   }

   if (pCheck > 0.0) {
      return runningProb/pCheck;
   }
   else {
      return static_cast<double>(simpleValue(mainBattle));
   }

}

/*void executeML(Battle &mainBattle, const vector<int> &move) {
   bool switchOnly;
   vector<PE_SV> speed(PE_SIDES, 0);
   vector<vector<int> > movesAgain;
   int firstSide, secondSide;
   int mlBranch;
   vector<Turn> turn;
   turn.resize(PE_SIDES);

   mainBattle.incTurn();
   mainBattle.getMoves(movesAgain, switchOnly);
   if (switchOnly) {
      mainBattle.doSimpleMove(PE_SIDEA, move[PE_SIDEA]);
      mainBattle.doSimpleMove(PE_SIDEB, move[PE_SIDEB]);
   }
   else {
      mainBattle.getSpeeds(move[PE_SIDEA], move[PE_SIDEB], speed);
      if (speed[PE_SIDEA] >= speed[PE_SIDEB]) {
         firstSide = PE_SIDEA;
         secondSide = PE_SIDEB;
      }
      else {
         firstSide = PE_SIDEB;
         secondSide = PE_SIDEA;
      }
      mainBattle.getBranching(firstSide, move[firstSide], turn[firstSide]);
      mlBranch = 0;
      for (int branch = 1; branch < turn[firstSide].branches(); branch++) {
         if (turn[firstSide].prob(branch) > turn[firstSide].prob(mlBranch)) mlBranch = branch;
      }
      mainBattle.doMove(firstSide, move[firstSide], turn[firstSide].type(mlBranch));
      mainBattle.getBranching(secondSide, move[secondSide], turn[secondSide]);
      mlBranch = 0;
      for (int branch = 1; branch < turn[secondSide].branches(); branch++) {
         if (turn[secondSide].prob(branch) > turn[secondSide].prob(mlBranch)) mlBranch = branch;
      }
      mainBattle.doMove(secondSide, move[secondSide], turn[secondSide].type(mlBranch));
      mainBattle.finishTurn();
   }

}*/

bool executeProp(Battle &mainBattle, const vector<int> &move) {
   bool switchOnly;
   vector<PE_SV> speed(PE_SIDES, 0);
   vector<vector<int> > movesAgain;
   int32_t firstSide, secondSide, branch;
   vector<Turn> turn;
   turn.resize(PE_SIDES + 1);
   double roll, accum;
   PE_SIDE winner;

   mainBattle.incTurn();
   mainBattle.getMoves(movesAgain, switchOnly);
   if (switchOnly) {
      mainBattle.doSimpleMove(PE_SIDEA, move[PE_SIDEA]);
      mainBattle.doSimpleMove(PE_SIDEB, move[PE_SIDEB]);
   }
   else {
      mainBattle.getSpeeds(move[PE_SIDEA], move[PE_SIDEB], speed);
      if (speed[PE_SIDEA] >= speed[PE_SIDEB]) {
         firstSide = PE_SIDEA;
         secondSide = PE_SIDEB;
      }
      else {
         firstSide = PE_SIDEB;
         secondSide = PE_SIDEA;
      }
      mainBattle.getBranching(firstSide, move[firstSide], turn[firstSide]);
      roll = static_cast<double>(randInt(10000))/10000.0;
      accum = 0.0;
      for (branch = 0; branch < turn[firstSide].branches(); branch++) {
         accum += turn[firstSide].prob(branch);
         if (accum > roll) break;
      }
      mainBattle.doMove(firstSide, move[firstSide], turn[firstSide].type(branch));
      if (mainBattle.gameOver(winner)) return true;

      mainBattle.getBranching(secondSide, move[secondSide], turn[secondSide]);
      roll = static_cast<double>(randInt(10000))/10000.0;
      accum = 0.0;
      for (branch = 0; branch < turn[secondSide].branches(); branch++) {
         accum += turn[secondSide].prob(branch);
         if (accum > roll) break;
      }
      mainBattle.doMove(secondSide, move[secondSide], turn[secondSide].type(branch));
      if (mainBattle.gameOver(winner)) return true;
      mainBattle.getBranchingEndturn(turn[PE_SIDES]);
      roll = static_cast<double>(randInt(10000))/10000.0;
      accum = 0.0;
      for (branch = 0; branch < turn[PE_SIDES].branches(); branch++) {
         accum += turn[PE_SIDES].prob(branch);
         if (accum > roll) break;
      }
      mainBattle.finishTurn(turn[PE_SIDES], turn[PE_SIDES].type(branch));
      if (mainBattle.gameOver(winner)) return true;
   }

   return false;
}


int32_t simpleValue(Battle &mainBattle) {

   return mainBattle.simpleEval();
}

void psl(vector<int> &move, const PE_ABILITY &comp, char *st) {
	uint32_t m;

   for (m = 0; m < move.size(); m++) {
      if (move[m] == comp) {
         printf("%s", st);
         break;
      }
   }
   if (m == move.size()) printf("  ");
}

void printMoves(vector<vector<int> > &move) {

   printf("    ");
   psl(move[PE_SIDEA], PE_SLOT1, "A1");
   printf("    ");
   psl(move[PE_SIDEA], PE_SLOT2, "A2");
   printf("    ");
   psl(move[PE_SIDEA], PE_SLOT3, "A3");
   printf("    ");
   psl(move[PE_SIDEA], PE_SWITCH1, "S4");
   printf("    ");
   psl(move[PE_SIDEA], PE_SWITCH2, "S5");
   printf("    ");
   psl(move[PE_SIDEA], PE_SWITCH3, "S6");
   printf("    ");
   psl(move[PE_SIDEA], PE_PASS, "P7");
   printf("          ");
   psl(move[PE_SIDEB], PE_SLOT1, "A1");
   printf("    ");
   psl(move[PE_SIDEB], PE_SLOT2, "A2");
   printf("    ");
   psl(move[PE_SIDEB], PE_SLOT3, "A3");
   printf("    ");
   psl(move[PE_SIDEB], PE_SWITCH1, "S4");
   printf("    ");
   psl(move[PE_SIDEB], PE_SWITCH2, "S5");
   printf("    ");
   psl(move[PE_SIDEB], PE_SWITCH3, "S6");
   printf("    ");
   psl(move[PE_SIDEB], PE_PASS, "P7");
   printf("\r\n");

}

void psp(const int &s) {
   if (s == PE_SLOT1)   printf("    A1                                          ");
   if (s == PE_SLOT2)   printf("          A2                                    ");
   if (s == PE_SLOT3)   printf("                A3                              ");
   if (s == PE_SWITCH1) printf("                      S4                        ");
   if (s == PE_SWITCH2) printf("                            S5                  ");
   if (s == PE_SWITCH3) printf("                                  S6            ");
   if (s == PE_PASS)    printf("                                        P7      ");
}

char nextChar(FILE **ins) {
   char next;

   if (*ins == (FILE *)NULL) {
      next = _getch();
   }
   else {
      fread_s(&next, 1, 1, 1, *ins);
   }
   if (next == 'c') {
      if (*ins != (FILE *)NULL) {
         fclose(*ins);
      }
      exit(9);
   }
   else if (next == 'h' && *ins != (FILE *)NULL) {
      fclose(*ins);
      *ins = (FILE *)NULL;
      return nextChar(ins);
   }

   return next;
}

int *getMoves(vector<vector<int> > &move, FILE **ins) {
   static int s[2];
   char input[2];
   uint32_t k;

   for (uint32_t j = 0; j < PE_SIDES; j++) {
      if (move[j].size() == 1) {
         s[j] = move[j][0];
         psp(s[j]);
         continue;
      }
      while (true) {
         input[j] = nextChar(ins);
         s[j] = input[j] - '1';
         if (s[j] >= PE_ABILITIES) s[j]++;
         for (k = 0; k < move[j].size(); k++) {
            if (s[j] == move[j][k]) break;
         }
         if (k < move[j].size()) break;
      }
      psp(s[j]);
   }
   printf("\r\n");

   return s;
}

int getBranch(vector<Turn> &turn, int side, FILE **ins) {
   int32_t branch;

   printf("Side %d  branch   prob   index\r\n", side);
   for (int32_t i = 0; i < turn[side].branches(); i++) {
      printf("        %3d    %5.2f    %d\r\n", i + 1, 100.0*turn[side].prob(i), turn[side].type(i));
   }
   if (turn[side].branches() > 1) {
      while (true) {
         branch = static_cast<int>(nextChar(ins) - '1');
         if (branch >= 0 && branch < turn[side].branches()) break;
      }
   }
   else branch = 0;
   printf("selected %d\r\n", branch + 1);

   return branch; 
}

void play(Battle &battle, const int &depth, char *filename) {
   bool switchOnly;
   vector<vector<int> > move;
   vector<PE_SV> speed(PE_SIDES, 0);
   vector<vector<int32_t> > value;
   int *s;
   PE_SIDE firstSide;
   uint32_t branch[PE_SIDES + 1];
   FILE *inStream;

   if (filename != NULL) {
      fopen_s(&inStream, filename, "rb");
   }
   else {
      inStream = (FILE *)NULL;
   }
   for (int32_t i = 0; i < depth; i++) {
      // get moves from user
      battle.dumpSituation();
      move.clear();
      battle.incTurn();
      battle.getMoves(move, switchOnly);
      printMoves(move);
      s = getMoves(move, &inStream);
      if (switchOnly) {
         battle.doSimpleMove(PE_SIDEA, s[PE_SIDEA]);
         battle.doSimpleMove(PE_SIDEB, s[PE_SIDEB]);
      }
      else {
         battle.getSpeeds(s[PE_SIDEA], s[PE_SIDEB], speed);
            
         // check whether we have to split the moves
         if (speed[PE_SIDEA] == speed[PE_SIDEB] && speed[PE_SIDEA] != PE_INFINITE && speed[PE_SIDEA] != 0) {
            printf("SPLIT MOVE -- side A moving first\r\n");
         }
         if (speed[PE_SIDEA] >= speed[PE_SIDEB]) {
            firstSide = PE_SIDEA;
         }
         else {
            firstSide = PE_SIDEB;
         }

         PE_SIDE secondSide = firstSide^1;
         vector<Turn> turn;
         turn.resize(PE_SIDES + 1);

         battle.getBranching(firstSide, s[firstSide], turn[firstSide]);
         branch[firstSide] = getBranch(turn, firstSide, &inStream);
         battle.doMove(firstSide, s[firstSide], turn[firstSide].type(branch[firstSide]));
         battle.getBranching(secondSide, s[secondSide], turn[secondSide]);
         branch[secondSide] = getBranch(turn, secondSide, &inStream);

         battle.doMove(secondSide, s[secondSide], turn[secondSide].type(branch[secondSide]));
         battle.getBranchingEndturn(turn[PE_SIDES]);
         branch[PE_SIDES] = getBranch(turn, PE_SIDES, &inStream);
         battle.finishTurn(turn[PE_SIDES], turn[PE_SIDES].type(branch[PE_SIDES]));
      }
      printf("CURRENT VALUE: %d\r\n", battle.simpleEval());
   }
}

int32_t randInt(const int32_t maxInt) {
   int32_t preval, value;
   //printf("%d\n", static_cast<int32_t>(rand()));
   //printf("%d\n", static_cast<int32_t>(rand())*maxInt);
   //printf("%d\n", (static_cast<int32_t>(rand())*maxInt)/static_cast<int32_t>(RAND_MAX + 1));
   preval = static_cast<int32_t>(rand())*maxInt;
   value = preval/static_cast<int32_t>(RAND_MAX + 1);
   return value;
}

int regression(Petinfo &pets) {
   PE_TEAM_SELECTION teamA, teamB;
   FILE *log;
   int seed;


   fopen_s(&log, "log.txt", "w");
   for (int iter = 0; iter < 10000; iter++) {
      for (int pet = PE_PET1; pet < PE_TEAM_SIZE; pet++) {
         for (teamA.pet[pet].petNumber = randInt(pets.numberPets()); !pets.numberBreeds(teamA.pet[pet].petNumber);) teamA.pet[pet].petNumber = randInt(pets.numberPets());
         teamA.pet[pet].breedNumber = randInt(pets.numberBreeds(teamA.pet[pet].petNumber));
         for (teamB.pet[pet].petNumber = randInt(pets.numberPets()); !pets.numberBreeds(teamB.pet[pet].petNumber);) teamB.pet[pet].petNumber = randInt(pets.numberPets());
         teamB.pet[pet].breedNumber = randInt(pets.numberBreeds(teamB.pet[pet].petNumber));
         for (int move = PE_SLOT1; move < PE_ABILITIES; move++) {
            teamA.pet[pet].abilitySelection[move] = randInt(PE_SELECTIONS);
            teamB.pet[pet].abilitySelection[move] = randInt(PE_SELECTIONS);
         }
      }
      /*teamA.pet[0].petNumber = 435, teamA.pet[1].petNumber = 480, teamA.pet[2].petNumber = 619;
      teamA.pet[0].breedNumber = 0, teamA.pet[1].breedNumber = 0, teamA.pet[2].breedNumber = 2;
      teamA.pet[0].abilitySelection[0] = 0, teamA.pet[0].abilitySelection[1] = 0, teamA.pet[0].abilitySelection[2] = 1;
      teamA.pet[1].abilitySelection[0] = 1, teamA.pet[1].abilitySelection[1] = 1, teamA.pet[1].abilitySelection[2] = 0;
      teamA.pet[2].abilitySelection[0] = 0, teamA.pet[2].abilitySelection[1] = 1, teamA.pet[2].abilitySelection[2] = 0;
      teamB.pet[0].petNumber = 27, teamB.pet[1].petNumber = 379, teamB.pet[2].petNumber = 364;
      teamB.pet[0].breedNumber = 1, teamB.pet[1].breedNumber = 0, teamB.pet[2].breedNumber = 0;
      teamB.pet[0].abilitySelection[0] = 1, teamB.pet[0].abilitySelection[1] = 1, teamB.pet[0].abilitySelection[2] = 0;
      teamB.pet[1].abilitySelection[0] = 0, teamB.pet[1].abilitySelection[1] = 1, teamB.pet[1].abilitySelection[2] = 1;
      teamB.pet[2].abilitySelection[0] = 0, teamB.pet[2].abilitySelection[1] = 0, teamB.pet[2].abilitySelection[2] = 1;*/

      seed = static_cast<int>(time(0));
      fprintf(log, "%d %d\n", iter, seed);
      fprintf(log, "teamA: %3d %3d %3d  %d %d %d    %d %d %d  %d %d %d  %d %d %d\n",
         teamA.pet[0].petNumber, teamA.pet[1].petNumber, teamA.pet[2].petNumber,
         teamA.pet[0].breedNumber, teamA.pet[1].breedNumber, teamA.pet[2].breedNumber,
         teamA.pet[0].abilitySelection[0], teamA.pet[0].abilitySelection[1], teamA.pet[0].abilitySelection[2],
         teamA.pet[1].abilitySelection[0], teamA.pet[1].abilitySelection[1], teamA.pet[1].abilitySelection[2],
         teamA.pet[2].abilitySelection[0], teamA.pet[2].abilitySelection[1], teamA.pet[2].abilitySelection[2]);
      fprintf(log, "teamB: %3d %3d %3d  %d %d %d    %d %d %d  %d %d %d  %d %d %d\n",
         teamB.pet[0].petNumber, teamB.pet[1].petNumber, teamB.pet[2].petNumber,
         teamB.pet[0].breedNumber, teamB.pet[1].breedNumber, teamB.pet[2].breedNumber,
         teamB.pet[0].abilitySelection[0], teamB.pet[0].abilitySelection[1], teamB.pet[0].abilitySelection[2],
         teamB.pet[1].abilitySelection[0], teamB.pet[1].abilitySelection[1], teamB.pet[1].abilitySelection[2],
         teamB.pet[2].abilitySelection[0], teamB.pet[2].abilitySelection[1], teamB.pet[2].abilitySelection[2]);
      fflush(log);

      srand(seed);
      //srand(1414261701);
      Battle mainBattle(pets, teamA, teamB);

      mainBattle.dumpSituation();
      while (mainBattle.simpleEval() > 0 && mainBattle.simpleEval() < maxEval) {
         vector<vector<int> > move;
         vector<vector<double> > x;
         for (int depth = 1; depth < 5; depth++) {
            move.clear();
            x.clear();
            double value = SMAB(mainBattle, move, x, depth);
            printf("  depth %d nodes %8d value %4.0f time %9.3f\n", depth, mainBattle.nodes(), value, mainBattle.time());
            if (value == 0 || value == maxEval) break;
         }
         int bestRow = 0, bestCol = 0;
         for (uint32_t row = 1; row < x[PE_SIDEA].size(); row++) {
            if (x[PE_SIDEA][row] > x[PE_SIDEA][bestRow]) bestRow = row;
         }
         for (uint32_t col = 1; col < x[PE_SIDEB].size(); col++) {
            if (x[PE_SIDEB][col] > x[PE_SIDEB][bestCol]) bestCol = col;
         }

         vector<int> theMove(PE_SIDES, 0);
         theMove[0] = move[PE_SIDEA][bestRow];
         theMove[1] = move[PE_SIDEB][bestCol];
         bool gameOver = executeProp(mainBattle, theMove);
         printf("turn %d best = %f %f and %d %d eval = %4d\n", mainBattle.turn(), x[PE_SIDEA][bestRow], x[PE_SIDEB][bestCol], move[PE_SIDEA][bestRow], move[PE_SIDEB][bestCol], mainBattle.simpleEval());
         //break;
         //if (mainBattle.turn() == 4) break;
         if (gameOver) break;
      }
      PE_SIDE winner;

      mainBattle.gameOver(winner);
      printf("Winner = %d\n", winner);
   }
   fclose(log);

   return 0;
}

