// $Id: mcambio.c,v 1.4 2006-02-26 20:36:53 simontaotw Exp $

/*
 * The above lines will include the name and log of the last person
 * to commit this file to CVS
 */

/************************************************************************
**
** NAME:        mcambio.c
**
** DESCRIPTION: Cambio
**
** AUTHOR:      Albert Chae and Simon Tao
**
** DATE:        Begin: 2/20/2006 End: 
**
** UPDATE HIST: 2/20/2006 - Updated game-specific constants.
**              2/21/2006 - Updated defines and structs, global variables, and InitializeGame(). Corrected CVS log.
**              2/26/2006 - Updated PrintPosition() (Modified PrintPosition() from mtopitop.c).
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>


/*************************************************************************
**
** Game-specific constants
**
**************************************************************************/

STRING   kGameName            = "Cambio"; /* The name of your game */
STRING   kAuthorName          = "Albert Chae and Simon Tao"; /* Your name(s) */
STRING   kDBName              = "cambio"; /* The name to store the database under */

BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */
BOOLEAN  kGameSpecificMenu    = TRUE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */
BOOLEAN  kTieIsPossible       = FALSE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/

BOOLEAN  kDebugMenu           = TRUE ; /* TRUE only when debugging. FALSE when on release. */
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */

POSITION gNumberOfPositions   =  900000000000; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/
/* 3^25 = 8.47E11 ~ 9E11 (this needs refining) */
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */
POSITION kBadPosition         = -1; /* A position that will never be used */

void*	 gGameSpecificTclInit = NULL;

/* 
 * Help strings that are pretty self-explanatory
 * Strings than span more than one line should have backslashes (\) at the end of the line.
 */

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
"BOARD SETUP:\n
1. The players each select a symbol to be their own, and decide\n
   who will begin the game.\n
2. Fill the four corners with neutral symbols.\n
3. Player A creates a starting position for Player B by placing\n
   5 cubes into the tray, with player B's symbol.\n  
   These cubes may be placed in any of the positions that a cube\n   may occupy when the tray is full.\n
4. Player B now places 4 of the first player A's symbol in any\n
   free positions.\n
5. Fill in the rest of the positions with neutral symbols.\n\n
PLAY:\n
Each player in turn places a spare cube with his/her own symbol, at\n
the beginning of any row or column; frees the end cube by lifting\n
it from the board; then pushes all the pieces in that line along\n
one place. A player MAY NOT push cubes with your opponents symbol\n
showing OFF the board.\n"; 

STRING   kHelpOnYourTurn =
"You may only push off neutral cubes, or cubes of your own symbol.\n
As the game progresses, you should have more and more of your own\n
cubes in play.";

STRING   kHelpStandardObjective =
"To make a line composed of all 5 of your own symbol - \n
horizontally, vertically or diagonally.\n";

STRING   kHelpReverseObjective =
"";

STRING   kHelpTieOccursWhen =
"A tie never occurs.";

STRING   kHelpExample =
"";


/*************************************************************************
**
** #defines and structs
**
**************************************************************************/

#define ROWCOUNT 5;
#define COLCOUNT 5;

#define BLANK ' ';
#define NEUTRAL '-';
#define A_PIECE 'X';
#define B_PIECE 'O';
#define UNKNOWN '?';

#define A_TURN 1;
#define B_TURN 2;

typedef enum possibleBoardPieces {
  Blank = 0, Neutral, A, B
} BoardPiece;

typedef enum playerTurn {
  A = 1, B
} PlayerTurn;

/* used in PrintPosition() */
#define BOLD_UL_CORNER 201;
#define BOLD_UR_CORNER 187;
#define BOLD_LL_CORNER 200;
#define BOLD_LR_CORNER 188;
#define BOLD_HOR 205;
#define BOLD_VERT 186;
#define BOLD_HOR_DOWN 209;
#define BOLD_HOR_UP 207;
#define BOLD_VERT_LEFT 182;
#define BOLD_VERT_RIGHT 199;
#define HOR_LINE 196;
#define VERT_LINE 179;
#define CROSS_LINE 197;

/*************************************************************************
**
** Global Variables
**
*************************************************************************/

int boardSize = 25;
int aCount = 0;
int bCount = 0;

Player gWhosTurn;
int gameType;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/

/* External */
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();
extern POSITION         generic_hash_init(int boardsize, int pieces_array[], int (*vcfg_function_ptr)(int* cfg));
extern POSITION         generic_hash(char *board, int player);
extern char            *generic_unhash(POSITION hash_number, char *empty_board);
extern int              whoseMove (POSITION hashed);
/* Internal */
void                    InitializeGame();
MOVELIST               *GenerateMoves(POSITION position);
POSITION                DoMove (POSITION position, MOVE move);
VALUE                   Primitive (POSITION position);
void                    PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn);
void                    PrintComputersMove(MOVE computersMove, STRING computersName);
void                    PrintMove(MOVE move);
USERINPUT               GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName);
BOOLEAN                 ValidTextInput(STRING input);
MOVE                    ConvertTextInputToMove(STRING input);
void                    GameSpecificMenu();
void                    SetTclCGameSpecificOptions(int options[]);
POSITION                GetInitialPosition();
int                     NumberOfOptions();
int                     getOption();
void                    setOption(int option);
void                    DebugMenu();
/* Game-specific */
char                    BoardPieceToChar(BoardPiece piece);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
** 
************************************************************************/

void InitializeGame ()
{
  int i;
  int piecesArray[] = { Blank, Neutral, A, B };

  BoardPiece boardArray[boardSize];

  gNumberOfPositions = generic_hash_init(boardSize, piecesArray, NULL);
  gWhosTurn = A_TURN;
    
  /* setting up the four corners; to neutrals */
  boardArray[0] = boardArray[COLCOUNT-1] = boardArray[boardSize-(COLCOUNT-1)]  = boardArray[boardSize-1] = Neutral;

  /* setting up the rest of the board; to blanks */
  for (i = 0; i < boardSize; i++) {
    boardArray[i] = Blank;
  }
    
  gInitialPosition = generic_hash(boardArray, gWhosTurn);
}


/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Creates a linked list of every move that can be reached
**              from this position. Returns a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : Current position for move
**                                  generation.
**
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of
**                                  the linked list of generated moves
**
** CALLS:       MOVELIST *CreateMovelistNode();
**
************************************************************************/

MOVELIST *GenerateMoves (POSITION position)
{
    MOVELIST *moves = NULL;
    
    /* Use CreateMovelistNode(move, next) to 'cons' together a linked list */
    
    return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Applies the move to the position.
** 
** INPUTS:      POSITION position : The old position
**              MOVE     move     : The move to apply to the position
**
** OUTPUTS:     (POSITION)        : The position that results from move
**
** CALLS:       Some Board Hash Function
**              Some Board Unhash Function
**
*************************************************************************/

POSITION DoMove (POSITION position, MOVE move)
{
    return 0;
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Returns the value of a position if it fulfills certain
**              'primitive' constraints.
**
**              Example: Tic-tac-toe - Last piece already placed
**
**              Case                                  Return Value
**              *********************************************************
**              Current player sees three in a row    lose
**              Entire board filled                   tie
**              All other cases                       undecided
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE)           : one of
**                                  (win, lose, tie, undecided)
**
** CALLS:       None              
**
************************************************************************/

VALUE Primitive (POSITION position)
{
    return undecided;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()      : Returns the prediction of the game
**
************************************************************************/

void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn)
{
  int i;
  char *theBoard = generic_unhash(position,theBoard);
  
  /***********************LINE 1**************************/
  printf("       %c%c%c%c%c%c%c%c%c%c%c\n", BOLD_UL_CORNER, 
	                                    BOLD_HOR, BOLD_HOR_DOWN, 
                                            BOLD_HOR, BOLD_HOR_DOWN, 
                                            BOLD_HOR, BOLD_HOR_DOWN, 
                                            BOLD_HOR, BOLD_HOR_DOWN, 
                                            BOLD_HOR, 
                                            BOLD_UR_CORNER);

  /***********************LINE 2**************************/
  printf("       %c", BOLD_VERT);
  for (i = 0; i < COLCOUNT; i++) {
    printf("%c%c", BoardPieceToChar(arrayHashedBoard[i]), (i == (COLCOUNT-1)) ? VERT_LINE : BOLD_VERT);
  }
  printf("          (  0  1  2  3  4 )\n");

  /***********************LINE 3**************************/
  printf("       %c%c%c%c%c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE,
                                            HOR_LINE, 
                                            BOLD_VERT_LEFT);

  /***********************LINE 4**************************/
  printf("       %c", BOLD_VERT);
  for (; i < COLCOUNT*2; i++) {
    printf("%c%c", BoardPieceToChar(arrayHashedBoard[i]), (i == (COLCOUNT*2-1)) ? VERT_LINE : BOLD_VERT);
  }
  printf("          (  5  6  7  8  9 )\n");

  /***********************LINE 5**************************/
  printf("       %c%c%c%c%c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE,
                                            HOR_LINE, 
                                            BOLD_VERT_LEFT);

  /***********************LINE 6**************************/
  printf("       %c", BOLD_VERT);
  for (; i < COLCOUNT*3; i++) {
    printf("%c%c", BoardPieceToChar(arrayHashedBoard[i]), (i == (COLCOUNT*3-1)) ? VERT_LINE : BOLD_VERT);
  }
  printf("  LEGEND: ( 10 11 12 13 14 )\n");

  /***********************LINE 7**************************/
  printf("       %c%c%c%c%c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE,
                                            HOR_LINE, 
                                            BOLD_VERT_LEFT);

  /***********************LINE 8**************************/
  printf("       %c", BOLD_VERT);
  for (; i < COLCOUNT*4; i++) {
    printf("%c%c", BoardPieceToChar(arrayHashedBoard[i]), (i == (COLCOUNT*4-1)) ? VERT_LINE : BOLD_VERT);
  }
  printf("          ( 15 16 17 18 19 )\n");

  /***********************LINE 9**************************/
  printf("       %c%c%c%c%c%c%c%c%c%c%c\n", BOLD_VERT_RIGHT, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE, 
                                            HOR_LINE, CROSS_LINE,
                                            HOR_LINE, 
                                            BOLD_VERT_LEFT);

  /***********************LINE 10**************************/
  printf("       %c", BOLD_VERT);
  for (; i < COLCOUNT*5; i++) {
    printf("%c%c", BoardPieceToChar(arrayHashedBoard[i]), (i == (COLCOUNT*5-1)) ? VERT_LINE : BOLD_VERT);
  }
  printf("          ( 20 21 22 23 24 )\n");

  /***********************LINE 11**************************/
  printf("       %c%c%c%c%c%c%c%c%c%c%c\n", BOLD_LL_CORNER, 
	                                    BOLD_HOR, BOLD_HOR_UP, 
                                            BOLD_HOR, BOLD_HOR_UP, 
                                            BOLD_HOR, BOLD_HOR_UP, 
                                            BOLD_HOR, BOLD_HOR_UP, 
                                            BOLD_HOR, 
                                            BOLD_LR_CORNER);

  /***********************LINE 12**************************/
  printf("                      A's Symbol = X\n");
  printf("                      B's Symbol = O\n");
  printf("                         Neutral = -\n");
  printf("\n%s\n\n", GetPrediction(position, playersName, usersTurn));

  SafeFree(theBoard);
}


/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName)
{
    
}


/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Prints the move in a nice format.
** 
** INPUTS:      MOVE move         : The move to print. 
**
************************************************************************/

void PrintMove (MOVE move)
{
    
}


/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here. 
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move. 
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName)
{
    USERINPUT input;
    USERINPUT HandleDefaultTextInput();
    
    for (;;) {
        /***********************************************************
         * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
         ***********************************************************/
	printf("%8s's move [(undo)/(MOVE FORMAT)] : ", playersName);
	
	input = HandleDefaultTextInput(position, move, playersName);
	
	if (input != Continue)
		return input;
    }

    /* NOTREACHED */
    return Continue;
}


/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER
**                                          COMMANDS IN YOUR GAME
**              ?, s, u, r, h, a, c, q
**                                          However, something like a3
**                                          is okay.
** 
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9
**                       Only integers between 1 to 9 are accepted
**                       regardless of board position.
**                       Moves will be checked by the core.
**
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput (STRING input)
{
    return FALSE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
** 
** INPUTS:      STRING input : The VALID string input from the user.
**
** OUTPUTS:     MOVE         : Move converted from user input.
**
************************************************************************/

MOVE ConvertTextInputToMove (STRING input)
{
    return 0;
}


/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Prints, receives, and sets game-specific parameters.
**
**              Examples
**              Board Size, Board Type
**
**              If kGameSpecificMenu == FALSE
**                   Gamesman will not enable GameSpecificMenu
**                   Gamesman will not call this function
** 
**              Resets gNumberOfPositions if necessary
**
************************************************************************/

void GameSpecificMenu ()
{
    
}


/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions (int options[])
{
    
}
  
  
/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Called when the user wishes to change the initial
**              position. Asks the user for an initial position.
**              Sets new user defined gInitialPosition and resets
**              gNumberOfPositions if necessary
** 
** OUTPUTS:     POSITION : New Initial Position
**
************************************************************************/

POSITION GetInitialPosition ()
{
    return 0;
}


/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of variants
**              your game supports.
**
** OUTPUTS:     int : Number of Game Variants
**
************************************************************************/

int NumberOfOptions ()
{
    return 0;
}


/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function that returns a number corresponding
**              to the current variant of the game.
**              Each set of variants needs to have a different number.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption ()
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
    return 0;
}


/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The corresponding unhash function for game variants.
**              Unhashes option and sets the necessary variants.
**
** INPUT:       int option : the number representation of the options.
**
************************************************************************/

void setOption (int option)
{
    /* If you have implemented symmetries you should
       include the boolean variable gSymmetries in your
       hash */
}


/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default
**              debug menu). Menu used to debug internal problems.
**
**              If kDebugMenu == FALSE
**                   Gamesman will not display a debug menu option
**                   Gamesman will not call this function
** 
************************************************************************/

void DebugMenu ()
{
    
}


/************************************************************************
**
** Everything specific to this module goes below these lines.
**
** Things you want down here:
** Move Hasher
** Move Unhasher
** Any other function you deem necessary to help the ones above.
** 
************************************************************************/

char BoardPieceToChar(BoardPiece piece) {
  switch(piece) {
    case Blank: return BLANK;
    case Neutral: return NEUTRAL;
    case A: return A_PIECE;
    case B: return B_PIECE;
  }

  return UNKNOWN;
}


// $Log: not supported by cvs2svn $
// Revision 1.3  2006/02/22 02:54:48  simontaotw
// Updated defines and structs, global variables, and InitializeGame(). Corrected CVS log.
//
// Revision 1.1  2006/02/21 03:17:00  simontaotw
// Updated game-specific constants
//
// Revision 1.7  2006/01/29 09:59:47  ddgarcia
// Removed "gDatabase" reference from comment in InitializeGame
//
// Revision 1.6  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.5  2005/10/06 03:06:11  hevanm
// Changed kDebugDetermineValue to be FALSE.
//
// Revision 1.4  2005/05/02 17:33:01  nizebulous
// mtemplate.c: Added a comment letting people know to include gSymmetries
// 	     in their getOption/setOption hash.
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for
//         gamesman.  mttc.c had a PLAYER type already, so I changed it.
// analysis.c: Changed initialization of option variable in analyze() to -1.
// db.c: Changed check in the getter functions (GetValueOfPosition and
//       getRemoteness) to check if gMenuMode is Evaluated.
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame
//             which is a generic version of the two that uses to PLAYER's.
// gameplay.h: Created the necessary structs and types to have PLAYER's, both
// 	    Human and Computer to be sent in to the PlayGame function.
// gamesman.h: Really don't think I changed anything....
// globals.h: Also don't think I changed anything....both these I just looked at
//            and possibly made some format changes.
// textui.c: Redid the portion of the menu that allows you to choose opponents
// 	  and then play a game.  Added computer vs. computer play.  Also,
//           changed the analysis part of the menu so that analysis should
// 	  work properly with symmetries (if it is in getOption/setOption hash).
//
// Revision 1.3  2005/03/10 02:06:47  ogren
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer
//