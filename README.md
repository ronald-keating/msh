# msh Program V1.1

Created by Ronald Keating

3-29-2019

## What is msh

It is a My Shell program that executes system commands, such as ps or ls.

Multiple commands are allowed with semicolon.
Piping works with only two arguments. For example:

	ls | more
	ls -lash | more
	ls | wc
	ls -l | less

Added Commands for v1.1:

	color [0, 30-37]    - Changes color of msh (no brackets)
		0: resets colors
		30: BLACK
		31: RED
		32: GREEN
		33: YELLOW
		34: BLUE
		35: MAGENTA
		36: CYAN
		37: WHITE

	history				- Prints last 20 commands
	!!					- Executes last command
	!#					- Executes command of some number, if number exists
	mshrc				- Executes commands in mshrc file one by one
					  There is a pause between each command
	cd 					- Change directory
	export PATH=$PATH:/[path extension]		- Expand path (no brackets)
	alias string="command"					- Set an alias
	unalias string							-remove alias


## Setup

Compile the shell:
	
	gcc msh.c -o msh

Run the executive file with

	./msh

## Running the Program
Once the program is running, you will be prompted with ">: "
This means that the program is ready to take in input
Enter a command and press enter to execute the command

Type    exit<enter>     to exit msh

Note: The program ignores any extra white spaces and/or single/double quotes
