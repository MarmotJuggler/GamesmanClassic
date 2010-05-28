####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

#############################################################################
# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.
############################################################################
proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "L Game"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 19388
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kToWinStandard  "Force your opponent into a position from which he cannot move."
    set kToWinMisere  "Get yourself in a position from which you cannot move."

    global rectSize
    set rectSize 100

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	#SetToWinString "To Win: (fill in)"
	SetToWinString [concat "To Win: " [C_GetStandardObjString]]
    } else {
	#SetToWinString "To Win: (fill in)"
	SetToWinString [concat "To Win: " [C_GetReverseObjString]]
    }
    SetToMoveString "To Move: Each player takes turns moving his or her L piece into a different position. At the end of each turn, players place one of the neutral circle pieces onto any open square. Click on the small L that represents the location you want to move your L piece. Then click on the square where you want to place the black or white neutral circle."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Michael Savitzky, Alex Kozlowski"
    set kTclAuthors "Zach Bush"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
}


#############################################################################
# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
#############################################################################
proc GS_NameOfPieces {} {

   return [list blue red]

}


#############################################################################
# GS_ColorOfPlayers should return a list of two strings, 
# each representing the color of a player.
# If a specific color appears uniquely on one player's pieces,
# it might be a good choice for that player's color.
# In impartial games, both players may share the same color.
# If the game is tic tac toe, this might be the line 
# return [list blue red]
# If the game is nim, this might be the line
# return [list green green]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he clicks "New Game"
# The left player's color should be the first item in the list.
# The right player's color should be second.
#############################################################################
proc GS_ColorOfPlayers {} {

	return [list blue red]
    
}


#############################################################################
# GS_SetupRulesFrame sets up the rules frame;
# Adds widgets to the rules frame that will allow the user to 
# select the variant of this game to play. The options 
# selected by the user should be stored in a set of global
# variables.
# This procedure must initialize the global variables to some
# valid game variant.
# The rules frame must include a standard/misere setting.
# Args: rulesFrame (Frame) - The rules frame to which widgets
# should be added
# Modifies: the rules frame and its global variables
# Returns: nothing
#############################################################################
proc GS_SetupRulesFrame { rulesFrame } {

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame"]

    global kLabelFont
    set ruleNum 0
    foreach rule $ruleset {
	frame $rulesFrame.rule$ruleNum -borderwidth 2 -relief raised
	pack $rulesFrame.rule$ruleNum  -fill both -expand 1
	message $rulesFrame.rule$ruleNum.label -text [lindex $rule 0] -font $kLabelFont
	pack $rulesFrame.rule$ruleNum.label -side left
	set rulePartNum 0
	foreach rulePart [lrange $rule 1 end] {
	    radiobutton $rulesFrame.rule$ruleNum.p$rulePartNum -text $rulePart -variable [lindex $ruleSettingGlobalNames $ruleNum] -value $rulePartNum -highlightthickness 0 -font $kLabelFont
	    pack $rulesFrame.rule$ruleNum.p$rulePartNum -side left -expand 1 -fill both
	    incr rulePartNum
	}
	incr ruleNum
    } 
}


#############################################################################
# GS_GetOption gets the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code
#############################################################################
proc GS_GetOption { } {
    # TODO: Needs to change with more variants
    global gMisereGame
    set option 1
    set option [expr $option + (1-$gMisereGame)]
    return $option
}


#############################################################################
# GS_SetOption modifies the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing
#############################################################################
proc GS_SetOption { option } {
    # TODO: Needs to change with more variants
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
}


#############################################################################
# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"
#############################################################################
proc GS_Initialize { c } {

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    # $c configure -width 500 -height 500
    global kLabelFont gMisereGame

    global gFrameWidth gFrameHeight

    set edgeBuffer 0
    set rectSize [expr [expr $gFrameHeight - $edgeBuffer * 2] / 4]
    set size [expr (4 * $rectSize) + $edgeBuffer]

    $c create rectangle $edgeBuffer $edgeBuffer $size $size \
      -fill grey \
      -tags back

    for {set j 0} {$j < 5} {incr j} {
       set x [expr ($j * $rectSize) + $edgeBuffer]
       $c create line $x $edgeBuffer $x $size -width 1 -tags back
       $c create line $edgeBuffer $x $size $x -width 1 -tags back
    }

    ##Make the small L-pieces
    makeL $c 1 0 1 num1 sq9 sq5 sq1 sq2
    makeL $c 1 3 1 num2 sq2 sq3 sq6 sq10
    makeL $c 1 5 1 num3 sq11 sq3 sq7 sq4
    makeL $c 1 0 3 num4 sq13 sq9 sq5 sq6
    makeL $c 1 3 3 num5 sq14 sq10 sq7 sq6
    makeL $c 1 5 3 num6 sq15 sq11 sq7 sq8
    
    makeL $c 2 2 1 num7 sq10 sq2 sq6 sq1
    makeL $c 2 4 1 num8 sq11 sq3 sq7 sq2
    makeL $c 2 7 1 num9 sq12 sq8 sq3 sq4
    makeL $c 2 2 3 num10 sq14 sq10 sq6 sq5
    makeL $c 2 4 3 num11 sq15 sq11 sq7 sq6
    makeL $c 2 7 3 num12 sq16 sq12 sq8 sq7
    
    makeL $c 3 6 0 num13 sq3 sq8 sq4 sq2
    makeL $c 3 6 3 num14 sq6 sq7 sq8 sq12
    makeL $c 3 6 5 num15 sq10 sq11 sq12 sq16
    makeL $c 3 4 0 num16 sq3 sq7 sq1 sq2
    makeL $c 3 5 2 num17 sq5 sq6 sq7 sq11
    makeL $c 3 4 5 num18 sq9 sq10 sq11 sq15
    
    makeL $c 4 6 2 num19 sq6 sq7 sq8 sq4
    makeL $c 4 6 4 num20 sq11 sq12 sq10 sq8
    makeL $c 4 6 7 num21 sq14 sq15 sq16 sq12
    makeL $c 4 4 2 num22 sq6 sq5 sq3 sq7
    makeL $c 4 5 5 num23 sq9 sq10 sq11 sq7
    makeL $c 4 4 7 num24 sq13 sq14 sq15 sq11
    
    makeL $c 5 7 6 num25 sq8 sq12 sq16 sq15
    makeL $c 5 4 6 num26 sq7 sq11 sq15 sq14
    makeL $c 5 2 6 num27 sq6 sq10 sq14 sq13
    makeL $c 5 7 4 num28 sq4 sq8 sq11 sq12
    makeL $c 5 4 4 num29 sq3 sq7 sq11 sq10
    makeL $c 5 2 4 num30 sq6 sq2 sq10 sq9
    
    makeL $c 6 5 6 num31 sq7 sq11 sq15 sq16
    makeL $c 6 3 6 num32 sq6 sq10 sq14 sq15
    makeL $c 6 0 6 num33 sq9 sq5 sq13 sq14
    makeL $c 6 5 4 num34 sq3 sq7 sq11 sq12
    makeL $c 6 3 4 num35 sq2 sq6 sq10 sq11
    makeL $c 6 0 4 num36 sq9 sq5 sq1 sq10
    
    makeL $c 7 1 7 num37 sq9 sq15 sq14 sq13
    makeL $c 7 1 4 num38 sq9 sq5 sq11 sq10
    makeL $c 7 1 2 num39 sq7 sq5 sq1 sq6
    makeL $c 7 3 7 num40 sq16 sq15 sq14 sq10
    makeL $c 7 2 5 num41 sq12 sq11 sq10 sq6
    makeL $c 7 3 2 num42 sq8 sq2 sq7 sq6
    
    makeL $c 8 1 5 num43 sq9 sq11 sq10 sq13
    makeL $c 8 1 3 num44 sq9 sq5 sq7 sq6
    makeL $c 8 1 0 num45 sq3 sq5 sq1 sq2
    makeL $c 8 3 5 num46 sq12 sq11 sq10 sq14
    makeL $c 8 2 2 num47 sq8 sq7 sq6 sq10
    makeL $c 8 3 0 num48 sq4 sq3 sq6 sq2

    ##Make the blue, red, and grey tiles
    for {set j 0} {$j < 16} {incr j} {
       set x [expr ($j % 4) * $rectSize]
       set y [expr ($j / 4) * $rectSize]

       $c create rectangle $x $y [expr $x + $rectSize] [expr $y + $rectSize] \
         -fill blue \
         -tags "blue-$j" 
       $c create rectangle $x $y [expr $x + $rectSize] [expr $y + $rectSize] \
         -fill red \
         -tags "red-$j" 
       $c create rectangle $x $y [expr $x + $rectSize] [expr $y + $rectSize] \
         -stipple gray25 \
         -fill blue \
         -outline blue \
         -width 3 \
         -tags "shade-$j" 
    }
    $c raise back
} 

proc makeL {c num x y t t1 t2 t3 t4} {
    global gFrameHeight
    set edgeBuffer 0
    set rectSize [expr [expr $gFrameHeight - $edgeBuffer * 2] / 4]
    set factor [expr $rectSize / 2]
    
    if {$num == 1} {
	$c create polygon [expr ($x + .25) * $factor] [expr ( $y + .125) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .375) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .375) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .125) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .25) * $factor] [expr ( $y + .125 ) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .375) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25 
    }
    if {$num == 2} {
	$c create polygon [expr ( $x + .75) * $factor] [expr ( $y + .125) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .375) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .375) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .125) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .5) * $factor] [expr ( $y + .125 ) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .375) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    
    if {$num == 3} {
	$c create polygon [expr (($x + .125) * $factor)] [expr ( $y + .25) * $factor] \
		[expr ( $x + .125) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .625) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .625) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .25) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .625) * $factor] [expr ( $y + .25 ) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .5) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    
    if {$num == 4} {
	$c create polygon [expr ( $x + .125) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .125) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .625) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .625) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .75) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .625) * $factor] [expr ( $y + .5 ) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .75) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    
    if {$num == 5} {
	$c create polygon [expr ( $x + .75) * $factor] [expr ( $y + .125) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .625) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .625) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .125) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .5) * $factor] [expr ( $y + .625 ) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .875) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    if {$num == 6} {
	$c create polygon [expr ( $x + .25) * $factor] [expr ( $y + .125) * $factor] \
		[expr ( $x + .25) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .875) * $factor] \
		[expr ( $x + .75) * $factor] [expr ( $y + .625) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .625) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .125) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .25) * $factor] [expr ( $y + .875 ) * $factor] \
		[expr ( $x + .5) * $factor] [expr ( $y + .625) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25  
    }
    
    if {$num == 7} {
	$c create polygon [expr ( $x + .125) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .125) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .25) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .125) * $factor] [expr ( $y + .75 ) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .5) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
    
    if {$num == 8} {
	$c create polygon [expr ( $x + .125) * $factor] [expr ( $y + .75) * $factor] \
		[expr ( $x + .125) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .25) * $factor] \
		[expr ( $x + .875) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .5) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .75) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill cyan -outline black -width 1
	
	$c create rectangle [expr ( $x + .125) * $factor] [expr ( $y + .25 ) * $factor] \
		[expr ( $x + .375) * $factor] [expr ( $y + .5) * $factor] \
		-tags "$t $t1 $t2 $t3 $t4 small" -fill blue -stipple gray25
    }
}

#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    $c delete all
}


#############################################################################
# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.
#
# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.
#############################################################################
proc GS_DrawPosition { c position } {

    $c raise base
    
    set l1 [unhashL1 $position]
    set l2 [unhashS1 $position]
    set s1 [unhashL2 $position]
    set s2 [unhashS2 $position]
    set turn [unhashTurn $position]

    DrawL l1 blue
    DrawL l2 red
    DrawS s1
    DrawS s2

}

proc unhashL1 { position } {
   return [expr ((($position >> 1) / 7 / 8 / 24) % 48) + 1]
}

proc unhashL2 { position } {
   return [expr ((($position >> 1) / 7 / 8) % 24) + 1];
}

proc unhashS1 { position } {
   return [expr ((($position >> 1) / 7) % 8) + 1];
}

proc unhashS2 { position } {
   return [expr (($position >> 1) % 7) + 1];
}

proc unhashTurn { position } {
   return [expr ($position & 1) + 1]
}

proc DrawL { pos color } {
}


#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
    # TODO: The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position
}


#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
    # Optional Procedure
    return ""    
}


#############################################################################
# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"
#
# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.
#############################################################################
proc GS_HandleMove { c oldPosition theMove newPosition } {

	### TODO: Fill this in
    
}


#############################################################################
# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.
#
# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of lists.  Each list contains a move and its value.
# These moves are represented as numbers (same as in C)
# The value will be either "Win" "Lose" or "Tie"
# Example:  moveList: { 73 Win } { 158 Lose } { 22 Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {

    ### TODO: Fill this in
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    ### TODO: Fill this in

}


#############################################################################
# GS_HandleUndo handles undoing a move (possibly with animation)
# Here's the undo logic
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A
#
# By default this function just calls GS_DrawPosition, but you certainly don't 
# need to keep that.
#############################################################################
proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {

    ### TODO if needed
    GS_DrawPosition $c $positionAfterUndo
}


#############################################################################
# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
#############################################################################
proc GS_GetGameSpecificOptions { } {
}


#############################################################################
# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {

	### TODO if needed
	
}


#############################################################################
# GS_UndoGameOver is called when the player hits undo after the game is finished.
# This is provided so that you may undo the drawing you did in GS_GameOver if you 
# drew something.
# For instance, if you drew a line crossing out the winning row in tic tac toe, 
# this is where you sould delete the line.
#
# note: GS_HandleUndo is called regardless of whether the move undoes the end of the 
# game, so IF you choose to do nothing in GS_GameOver, you needn't do anything here either.
#############################################################################
proc GS_UndoGameOver { c position } {

	### TODO if needed

}