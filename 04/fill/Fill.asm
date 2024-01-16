// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.

// black = -1
// while (true)
// {
// 	if (keyboard)
// 		screen = black
// 	else
// 		screen = 0
// }

(WAIT)
	@KBD
	D = M
	@WHITE
	D; JEQ
	@BLACK
	0; JMP
(WHITE)
	@SCREEN
	D = A
	@8192
	D = D + A
	@curReg
	M = D
(SETWHITE)
	@curReg
	M = M - 1
	D = M
	A = M
	M = 0
	@SCREEN
	D = D - A

	@WAIT
	D; JEQ
	@SETWHITE
	0; JMP
(BLACK)
	@SCREEN
	D = A
	@8192
	D = D + A
	@curReg
	M = D
(SETBLACK)
	@curReg
	M = M - 1
	D = M
	A = M
	M = -1
	@SCREEN
	D = D - A

	@WAIT
	D; JEQ
	@SETBLACK
	0; JMP
