/************************************************************************
**
** NAME:	solveloopyup.c
**
** DESCRIPTION:	Bottom-up, Go-again, Loopy solver.
**
** AUTHOR:	Bryon Ross
**		GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2004-04-29
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include "loopyupsolver.h"


/*
 * Loopy Up Solver
 * Author: Bryon Ross
 * Last Major Update: 2004-04-29
 * 
 * Guarantees: Generates valid game tree
 *             Generates valid remoteness (ties considered better than draws)
 *             All positions are decided. The value of an invalid position
 *              is meaningless
 *             Works with Go Again, but very slowly
 *
 * Requires:   GAMESMAN standard functions
 *             POSITIONLIST* GenerateParents(POSITION) is defined and
 *              returns a list of positions that are parents of the
 *              given position, excluding those beyond the scope of the 
 *              currently implemented game. If a position can be reached
 *              via more than one move, it must be included that number of
 *              times in the list.
 *
 *             For all non-primitive positions, the number of moves
 *             generated by GenerateMoves must equal the number of 
 *             times that position is generated as a parent by 
 *             GenerateParents. Thus, unreachable positions should not 
 *             generate any parents!
 */

#define BadChildrenCount -1
#define loopyup_debug FALSE

/* local only to this solver */
POSITION* loopyup_childrenCount;
BOOLEAN loopyup_goAgain;

/* local function prototypes */
void loopyup_DeterminePrimitives();
void loopyup_CountChildren(POSITION);
void loopyup_DetermineValueFromPrimitives();
void loopyup_DetermineLocalValueGoAgain(POSITION, VALUE, REMOTENESS, BOOLEAN);
void loopyup_DetermineLocalValueNoGoAgain(POSITION, VALUE, REMOTENESS, BOOLEAN);
void loopyup_StoreValueAndPropagate(POSITION, VALUE, REMOTENESS);
void loopyup_UpdateRemotenessAndPropagate(POSITION, VALUE, REMOTENESS);
void loopyup_CleanUpDatabase();

/* function pointer for GoAgain v. NoGoAgain */
void (*loopyup_DetermineLocalValue) (POSITION, VALUE, REMOTENESS, BOOLEAN);


VALUE loopyup_DetermineValue(POSITION pos) {
  
  loopyup_goAgain = (gGoAgain != DefaultGoAgain);

  if (loopyup_goAgain) {
    loopyup_DetermineLocalValue = loopyup_DetermineLocalValueGoAgain;
  }
  else {
    loopyup_childrenCount = (POSITION*) SafeMalloc(gNumberOfPositions * sizeof(POSITION));
    loopyup_DetermineLocalValue = loopyup_DetermineLocalValueNoGoAgain;
  } 
  
  loopyup_DeterminePrimitives();
  
  loopyup_DetermineValueFromPrimitives();
  
  if (!loopyup_goAgain)
    SafeFree(loopyup_childrenCount);
  
  loopyup_CleanUpDatabase();
  
  return GetValueOfPosition(pos);
}

void loopyup_DeterminePrimitives() {
  POSITION pos;
  VALUE primitiveValue;

  if (loopyup_debug)
    printf("\nPhase 1 of 2: Determine primitives\n");

  for (pos=0; pos<gNumberOfPositions; pos++) {
    primitiveValue = Primitive(pos);
    if (primitiveValue != undecided) {
      StoreValueOfPosition(pos, primitiveValue);
      MarkAsVisited(pos);
      if (!loopyup_goAgain)
	loopyup_childrenCount[pos] = 0;
    }
    else {
      UnMarkAsVisited(pos);
      if (!loopyup_goAgain)
	loopyup_childrenCount[pos] = BadChildrenCount;
    }
    if (loopyup_debug)
      if(pos%50000==0) printf("%f%% complete\n", 100.0*pos/gNumberOfPositions);
  }
  if (loopyup_debug)
    printf("Phase 1 Complete\n");
}

void loopyup_CountChildren(POSITION pos) {
  MOVELIST *head, *moveList;
  POSITION moveCount;

  head = moveList = GenerateMoves(pos);
  moveCount = 0;
  while(moveList != NULL) {
    moveCount++;
    moveList = moveList->next;
  }
  loopyup_childrenCount[pos] = moveCount;
  FreeMoveList(head);
}

void loopyup_DetermineValueFromPrimitives() {
  POSITION pos;
  POSITION parent;
  POSITIONLIST *phead, *parents;
  VALUE primitiveValue;
  
  if (loopyup_debug)
    printf("Phase 2 of 2: Determine values from primitives\n");

  for (pos=0; pos<gNumberOfPositions; pos++) {
    /* if primitive */
    if (Visited(pos)) {
      primitiveValue = GetValueOfPosition(pos);
      phead = parents = GenerateParents(pos);
      while (parents != NULL) {
	parent = parents->position;
	assert(parent <= gNumberOfPositions);
	loopyup_DetermineLocalValue(parent, primitiveValue, 0, FALSE);
	parents = parents->next;
      }
      FreePositionList(phead);
    }
    if (loopyup_debug)
      if(pos%10000==0) printf("%f%% complete\n", 100.0*pos/gNumberOfPositions);
  }

  if (loopyup_debug)
    printf("Phase 2 Complete\n");
}

void loopyup_DetermineLocalValueNoGoAgain(POSITION pos, VALUE callersValue, REMOTENESS callersRemoteness, BOOLEAN updateRemotenessOnly) {
  REMOTENESS winRemoteness;
  MOVELIST *mhead, *moveList;
  POSITION child;
  VALUE myValue;
  REMOTENESS childRemoteness;
  REMOTENESS myRemoteness;

  if (pos >= gNumberOfPositions) {
    printf("Illegal position from GenerateParents: " POSITION_FORMAT "\n", pos);
    printf("gNumberOfPositions: " POSITION_FORMAT "\n", gNumberOfPositions);
    assert(pos < gNumberOfPositions);
  }

  myValue = GetValueOfPosition(pos);
  myRemoteness = Remoteness(pos);

  /* ignore primitives */
  if (Visited(pos)) {
    return;
  }

  /* instant win */
  if (callersValue == lose) {
    if (myValue == win) {
      if (myRemoteness > callersRemoteness) {
	/* don't have to check for max remoteness b/c 
	 * callersRemoteness < myRemoteness <= REMOTENESS_MAX-1 */
	myRemoteness = callersRemoteness+1;
      }
      loopyup_UpdateRemotenessAndPropagate(pos, win, myRemoteness);
    }
    else {
      if (callersRemoteness < REMOTENESS_MAX-1)
	myRemoteness = callersRemoteness + 1;
      else
	myRemoteness = callersRemoteness;
      
      loopyup_StoreValueAndPropagate(pos, win, myRemoteness);
    }
    return;
  }

  if (callersValue == tie && callersRemoteness<REMOTENESS_MAX && myValue != win) {
    if (myValue == tie) {
      if (myRemoteness > callersRemoteness) {
	/* don't have to check for max remoteness b/c 
	 * callersRemoteness < myRemoteness <= REMOTENESS_MAX-1 */
	myRemoteness = callersRemoteness+1;
      }
      loopyup_UpdateRemotenessAndPropagate(pos, tie, myRemoteness);
    }
    else {
      if (callersRemoteness < REMOTENESS_MAX-1)
	myRemoteness = callersRemoteness + 1;
      else
	myRemoteness = callersRemoteness;
      
      loopyup_StoreValueAndPropagate(pos, tie, myRemoteness);
    }
    return;
  }

  /* only loses and draws from now on */
  /* already know value or draw, so return */
  if (myValue != undecided || myValue == tie) {
    return;
  }

  if (loopyup_childrenCount[pos] == BadChildrenCount) {
    loopyup_CountChildren(pos);
  }

  if (!updateRemotenessOnly)
    loopyup_childrenCount[pos]--;

  /* only winning children left */
  if (loopyup_childrenCount[pos]==0) {
    winRemoteness = -1;

    mhead = moveList = GenerateMoves(pos);

    while(moveList != NULL) {
      child = DoMove(pos, moveList->move);
      childRemoteness = Remoteness(child);

      if (childRemoteness>winRemoteness) {
	winRemoteness = childRemoteness;
      }
            
      moveList = moveList->next;
    }
    FreeMoveList(mhead);

    myValue = lose;
    if (myRemoteness < REMOTENESS_MAX-1)
      myRemoteness = winRemoteness + 1;
    else 
      myRemoteness = winRemoteness;

    loopyup_StoreValueAndPropagate(pos, myValue, myRemoteness);
  }
}

void loopyup_DetermineLocalValueGoAgain(POSITION pos, VALUE callersValue, REMOTENESS callersRemoteness, BOOLEAN updateRemotenessOnly) {
  BOOLEAN foundTie, foundWin, foundLose, foundUndecided;
  REMOTENESS winRemoteness, tieRemoteness, loseRemoteness;
  MOVELIST *mhead, *moveList;
  POSITION child;
  VALUE oldValue, newValue, childValue;
  REMOTENESS childRemoteness;
  REMOTENESS oldRemoteness, newRemoteness;

  /* if primitive, skip */
  if (Visited(pos)) {
    return;
  }

  oldValue = GetValueOfPosition(pos);
  oldRemoteness = Remoteness(pos);

  foundTie = foundWin = foundLose = foundUndecided = FALSE;
  winRemoteness = 0;
  tieRemoteness = loseRemoteness = REMOTENESS_MAX;
  
  mhead = moveList = GenerateMoves(pos);
  
  while(moveList != NULL) {
    child = DoMove(pos, moveList->move);
    childValue = GetValueOfPosition(child);
    childRemoteness = Remoteness(child);
    
    if (gGoAgain(pos, moveList->move)) {
      switch (childValue) {
      case lose: childValue = win; break;
      case win: childValue = lose; break;
      default: /* do nothing */ break;
      }
    }
    
    if (childValue == lose) {
      foundLose = TRUE;
      if (childRemoteness<loseRemoteness) {
	loseRemoteness = childRemoteness;
      }
    }
    /* tie (not draw) */
    else if (childValue == tie && childRemoteness<REMOTENESS_MAX) {
      foundTie = TRUE;
      if (childRemoteness<tieRemoteness) {
	tieRemoteness = childRemoteness;
      }
    }
    else if (childValue == win) {
      foundWin = TRUE;
      if (childRemoteness>winRemoteness) {
	winRemoteness = childRemoteness;
      }
    }
    else {
      /* draw or undecided */
      foundUndecided = TRUE;
    }
    
    moveList = moveList->next;
  }
  FreeMoveList(mhead);
  
  if (foundLose) {
    newValue = win;
    newRemoteness = loseRemoteness;
  }
  else if (foundTie) {
    newValue = tie;
    newRemoteness = tieRemoteness;
  }
  else if (foundUndecided) {
    /* draw or undecided, no update */
    return;
  }
  else if (foundWin) {
    newValue = lose;
    newRemoteness = winRemoteness;
  }

  if (newRemoteness < REMOTENESS_MAX-1) {
    newRemoteness++;
  }

  if (newValue != oldValue) {
    loopyup_StoreValueAndPropagate(pos, newValue, newRemoteness);
  }
  else if (newRemoteness != oldRemoteness) {
    loopyup_UpdateRemotenessAndPropagate(pos, newValue, newRemoteness);
  }
}

void loopyup_StoreValueAndPropagate(POSITION pos, VALUE value, REMOTENESS remoteness) {
   POSITIONLIST *phead, *parents;
   POSITION parent;

   StoreValueOfPosition(pos, value);
   SetRemoteness(pos, remoteness);

   phead = parents = GenerateParents(pos);
    while(parents != NULL) {
      parent = parents->position;
      loopyup_DetermineLocalValue(parent, value, remoteness, FALSE);
      parents = parents->next;
    }
    FreePositionList(phead);
}

void loopyup_UpdateRemotenessAndPropagate(POSITION pos, VALUE value, REMOTENESS remoteness) {
   POSITIONLIST *phead, *parents;
   POSITION parent;

   SetRemoteness(pos, remoteness);

   phead = parents = GenerateParents(pos);
    while(parents != NULL) {
      parent = parents->position;
      loopyup_DetermineLocalValue(parent, value, remoteness, TRUE);
      parents = parents->next;
    }
    FreePositionList(phead);
}


void loopyup_CleanUpDatabase() {
  POSITION pos;
  
  for (pos=0; pos<gNumberOfPositions; pos++) {
    UnMarkAsVisited(pos);
    if (GetValueOfPosition(pos) == undecided) {
      StoreValueOfPosition(pos, tie);
      SetRemoteness(pos, REMOTENESS_MAX);
    }
  }
}


