#!/bin/bash

set -e

# Compiler used in building
CC=gcc

# Compiler flags. Add or remove flags according to your platform
CCFLAGS="-Wall -Wextra -Wpedantic"

# Main source file
MAINSRCFILE="src/main.c"

# Output program file
OUTPROGFILE=miscounts-legacy

build() {
	$CC -o $OUTPROGFILE $MAINSRCFILE $CCFLAGS
}

install_program() {
	install -m775 $OUTPROGFILE $1$OUTPROGFILE
}

install_program_as_root() {
	sudo install -m775 $OUTPROGFILE $1$OUTPROGFILE
}

print_help_msg() {
	printf "./install.bash:\n\n"

	printf " -b | --build   => Builds the program\n"
	printf "  \`- -w | --wait | --wait-for-me <SEC (optional)>  => Let the script wait for user\n"
	printf "  \`- -n | --name | --name-of-program <NAME>        => Override the name of the built program\n\n"

	printf " -i | --install	=> Installs the program\n"
	printf "  \`- -l | --local  | --local-install		=> Install for the current user\n"
	printf "  \`- -s | --system | --system-wide-install	=> Install for everyone\n\n"

	printf " -h 	=> Shows this message\n\n\n"

	printf "Common examples:\n\n"
	printf "  ./install.bash -b; ./install.sh -l 	  => Quickest way of getting this program on your machine\n"
	printf "  ./install.bash --build -n miscount -w	  => Wait 3 seconds and build program with the output file named as \`miscount\`\n\n"
}

main() {
	if [[ $# < 1 ]]; then
		print_help_msg
		exit 64
	fi

	case $1 in
		"-b" | "--build")
			case $2 in
				"-w" | "--wait" | "--wait-for-me")
					SLEEPTIME=$3

					if [[ -z $SLEEPTIME ]]; then
						printf "Time not provided; waiting 1 second..."
						SLEEPTIME=1
					fi

					sleep $SLEEPTIME
				;;
				"-n" | "--name-of-program")
					if [[ -z $3 ]]; then
						printf "Please provide a name\n"
						exit 64
					fi

					OUTPROGFILE=$3
				;; 
			esac

			if [[ -f $OUTPROGFILE ]]; then
				printf "\`$OUTPROGFILE\` exists. Pass \`$0 --install\` to install the program\n"
				exit 64
			fi

			build
		;;
		"-i" | "--install")
			if [[ ! -f $OUTPROGFILE ]]; then
				printf "Regular file \`$OUTPROGFILE\` not found. Please pass \`$0 --build\` to create the file\n"
				exit 64
			fi

			case $2 in
				"-l" | "--local" | "--local-install")
					printf "Installing to ~/.local/bin\n"
					mkdir -p ~/.local/bin
					install_program ~/.local/bin/

					exit 0
				;;
				"-s" | "--system" | "--system-wide-install")
					printf "Installing to /usr/bin\n"

					printf "\nEnter your password to allow this:\n"
					install_program_as_root /usr/bin/

					exit 0
				;;
			esac

			install_program /usr/local/bin/
		;;
		"-h" | "--help")
			print_help_msg
		;;
	esac
}

main $@