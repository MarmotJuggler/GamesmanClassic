/*****************************************************************************
 **
 ** NAME:        mrcheckers.c
 **
 ** DESCRIPTION: Rubik's Checkers
 **
 ** AUTHOR:      Johnny Tran
 **              Steve Wu
 **
 ** DATE:        02/13/2006
 **
 ** UPDATE HIST:
 **
 ** 2/13/2006    Started. Much of the code is shamelessly borrowed from
 **              m1210.c, which is used as a template. Done: InitializeGame,
 **              PrintPosition
 **
 *****************************************************************************/

/*****************************************************************************
 **
 ** Everything below here must be in every game file
 **
 *****************************************************************************/
#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions   = 0;      /* Calculated in InitializeGame */
POSITION kBadPosition         = -1;

POSITION gInitialPosition     = 0;      /* Calculated in InitializeGame */

POSITION gMinimalPosition     = 0;       /* TODO: ? */

STRING   kAuthorName          = "Johnny Tran and Steve Wu";
STRING   kGameName            = "Rubik's Checkers";
BOOLEAN  kPartizan            = TRUE;
BOOLEAN  kDebugMenu           = FALSE;
BOOLEAN  kGameSpecificMenu    = FALSE;
BOOLEAN  kTieIsPossible       = FALSE;  /* TODO: Can both players be locked? */
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;
void*    gGameSpecificTclInit = NULL;

STRING   kHelpGraphicInterface = "";    /* TODO */

STRING   kHelpTextInterface    =
"Play like checkers, except only kings can capture, men can only move backwards\n and cannot capture, and jumping a piece demotes a king to a man and captures a man.";

STRING   kHelpOnYourTurn =
"Select a piece and its destination(s). (i.e. a3-c5-e3 to double-capture; a1+\nto promote)";

STRING   kHelpStandardObjective =
"Eliminate all your opponent's pieces or block them from moving.";

STRING   kHelpReverseObjective =
"Eliminate all your pieces or block them from moving.";

STRING   kHelpTieOccursWhen = ""; /* empty since kTieIsPossible == FALSE */

STRING   kHelpExample =           /* TODO */
"Help Example";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

// External Functions
extern GENERIC_PTR      SafeMalloc();
extern void             SafeFree();
extern POSITION         generic_hash_init(int boardsize, int *pieces_array, int (*fn)(int *));
extern POSITION         generic_hash(char* board, int player);
extern char             *generic_unhash(POSITION hash_number, char *empty_board);
extern int              whoseMove (POSITION hashed);

// How a piece can move
#define FORWARD               0x1
#define BACKWARD              0x2
#define CAPTURE               0x4

// Player representation
#define P1                    1
#define P2                    2

// Piece representation
#define EMPTY                 ' '
#define P1KING                'G'
#define P1MAN                 'g'
#define P2KING                'O'
#define P2MAN                 'o'
#define P1NAME                "Green"
#define P2NAME                "Orange"

// How large the board is
unsigned int rows           = 6;  // Rubik's: 6, Checkers: 8
unsigned int cols           = 2;  // Rubik's: 4, Checkers: 4
unsigned int boardSize;     /* initialized in InitializeGame */

// How many rows closest to a player start out with pieces (< ROWS/2)
unsigned int startRows      = 2;

// Rubik's Checkers specific options
// You can change to regular checkers rules by changing these options
BOOLEAN demote              = TRUE;      // Capturing demotes a piece
unsigned int kingMobility   = (FORWARD | BACKWARD | CAPTURE);
unsigned int manMobility    = BACKWARD;  // Man can only move backwards
BOOLEAN forceCapture        = FALSE;     // No forced captures
BOOLEAN startPromoted       = TRUE;      // Starting pieces are already kings
unsigned int promoteRow     = BACKWARD;  // Promote on your row

/*
typedef enum square {
    empty, orangeKing, orangeMan, greenKing, greenMan
} squareState;
*/

void InitializeGame()
{
    int maxPieces = startRows * cols;
    boardSize = rows * cols;
    int pieces[] = { P1KING, 0, maxPieces,
                     P1MAN,  0, maxPieces,
                     P2KING, 0, maxPieces,
                     P2MAN,  0, maxPieces,
                     EMPTY,  boardSize-(maxPieces*2), boardSize-1,
                     -1 };
    char* initialPosition = (char*)SafeMalloc(boardSize * sizeof(char));
    int i;
    
    generic_hash_init(boardSize, pieces, NULL);

    // Create initial position
    for (i = 0; i < maxPieces; i++) {
        initialPosition[i] = (startPromoted ? P1KING : P1MAN);
        initialPosition[boardSize-1 - i] = (startPromoted ? P2KING : P2MAN);
    }
    
    for (i = maxPieces; i < (boardSize - maxPieces); i++) {
        initialPosition[i] = EMPTY;
    }
    
    gInitialPosition = generic_hash(initialPosition, P1);
    SafeFree(initialPosition);
}

void FreeGame()
{
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu() { }

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
** 
************************************************************************/

void GameSpecificMenu() {
    // TODO: Allow changing of the board size and rules
}

/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions(theOptions)
int theOptions[];
{
  /* No need to have anything here, we have no extra options */
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
** 
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
************************************************************************/

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
    // TODO
    return thePosition;
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
** 
** INPUTS:      POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
    // TODO
    return gInitialPosition;
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
** 
** INPUTS:      MOVE   *computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove(computersMove,computersName)
     MOVE computersMove;
     STRING computersName;
{
    // TODO
}

/************************************************************************
**
** NAME:        CountPieces
**
** DESCRIPTION: Count how many pieces each player has.
** 
** INPUTS:      char board[]: the board to inspect
**              unsigned int *p1Pieces: holds number of pieces Player 1 has
**              unsigned int *p2Pieces: holds number of pieces Player 2 has
**
** OUTPUTS:     total number of pieces
**
************************************************************************/

unsigned int CountPieces(char board[],
                         unsigned int *p1Pieces, unsigned int *p2Pieces) {
    unsigned int i;
    
    *p1Pieces = *p2Pieces = 0;
    
    // Check if one player has no more pieces
    for (i = 0; i < boardSize; i++) {
        switch (board[i]) {
        case P1KING:
        case P1MAN:
            (*p1Pieces)++;
            break;
        case P2KING:
        case P2MAN:
            (*p2Pieces)++;
        }
    }
    
    return *p1Pieces + *p2Pieces;
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has one = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
**              We assume Player 1 is always the human
**              No more pieces: primitive lose
**              All Locked pieces: primitive lose
**              Else: undecided
**
************************************************************************/

VALUE Primitive(position) 
     POSITION position;
{
    char board[boardSize];
    unsigned int p1Pieces, p2Pieces;
    
    // Check for no more pieces
    CountPieces(board, &p1Pieces, &p2Pieces);
    if (&p1Pieces == 0) return lose;  // Player 1 has no more pieces
    
    // TODO: Check for all pieces being locked (unable to move)
    
    return win; //undecided;
}

/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
************************************************************************/

void PrintPosition(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{
    char board[boardSize];
    int player;
    unsigned int i, j, k = 0;
    unsigned int p1Pieces, p2Pieces;
    
    generic_unhash(position, board);  // Obtain board state
    player = whoseMove(position);
    
    printf("RUBIK'S CHECKERS\n\n  ");
    
    // Print column letters **POSSIBLE OVERFLOW**
    for (i = 0; i < (cols*2); i++) {
        printf("%c", 'a' + i);
    }
    
    printf("\n --");
    for (i = 0; i < (cols*2); i++) printf("-");
    printf("\n");
    
    CountPieces(board, &p1Pieces, &p2Pieces);

    for (i = rows; i > 0; i--) {
        printf("%d|", i);  // Row number
        if ((i % 2) != 0) printf("%c", EMPTY);  // Shift alternating rows
        
        for (j = 0; j < cols; j++) {
            // Print square
            printf("%c", board[k++]);
            
            // Print empty squares in between
            if ((j != (cols-1)) || (i % 2) == 0) printf("%c", EMPTY);
        }
        printf("|");
        if (i == rows) {  // Print player 2's number of pieces
            printf("    %-8s: %d pieces", P2NAME, p2Pieces);
            if (player == P2) printf(" (%s's turn)", P2NAME);
        } else if (i == 1) {  // Print player 1's number of pieces
            printf("    %-8s: %d pieces", P1NAME, p1Pieces);
            if (player == P1) printf(" (%s's turn)", P1NAME);
        }
        printf("\n");
    }
    
    printf(" --");
    for (i = 0; i < (cols*2); i++) printf("-");
    printf("\n");
}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item  
**              in the linked list of moves that can be generated.
**
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

MOVELIST *GenerateMoves(position)
     POSITION position;
{
    MOVELIST *head = NULL;
    MOVELIST *CreateMovelistNode();
  
    // TODO
    return(head);
}

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
** 
** INPUTS:      POSITION *thePosition : The position the user is at. 
**              MOVE *theMove         : The move to fill with user's move. 
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
     POSITION thePosition;
     MOVE *theMove;
     STRING playerName;
{
  USERINPUT ret, HandleDefaultTextInput();
  BOOLEAN ValidMove();
  char input[2];
  
  // TODO

  input[0] = '3';

  do {
    printf("%8s's move [(u)ndo/1/2] : ", playerName);

    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if(ret != Continue)
      return(ret);
  }
  while (TRUE);
  return(Continue); /* this is never reached, but lint is now happy */
}

/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
     STRING input;
{
    // TODO
    return(TRUE);
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(input)
     STRING input;
{
    // TODO
    return(0);
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
** 
** INPUTS:      MOVE *theMove         : The move to print. 
**
************************************************************************/

void PrintMove(theMove)
     MOVE theMove;
{
    // TODO
}


STRING kDBName = "Rubik's Checkers" ;

int NumberOfOptions()
{
    // TODO
    return 0;
}

int getOption()
{
    // TODO
    return 0;
}

void setOption(int option)
{
    // TODO
}