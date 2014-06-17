#!/bin/bash

# This shell script runs the GLSL shader validator from the Khronos Group on all shader files.

# Execute this script in a directory containing shader source files. It validate all files
# with a .vs, .fs, or .gs extension in all subdirectories.

validator_exe="$HOME/progs/glslvalidator/build/install/bin/glslangValidator"

function validate_source() {

	f=$1
	if [[ "${f: -3}" == ".vs" ]]; then
		tmpfile="_validation_file.vert"
	elif [[ "${f: -3}" == ".fs" ]]; then
		tmpfile="_validation_file.frag"
	elif [[ "${f: -3}" == ".gs" ]]; then
		tmpfile="_validation_file.geom"
	else
		echo "Invalid file extension: ${f: -3}"
		exit
	fi

	if [[ "${f: -3}" != ".gs" || $2 == "150" ]]; then
		echo "#version $2" | cat - $f > ${tmpfile}
		log=`${validator_exe} ${tmpfile} | grep -v "Warning, version $2 is not yet complete" | grep -v "^$"`
		rm ${tmpfile}
		if [ -n "${log}" ]; then
			echo "Validation failed for GLSL version $2:"
			echo "${log}"
		fi
	fi
}

for f in `find . -type f -name "*.vs" -o -name "*.fs" -o -name "*.gs"`
do
	echo "--------- $f"

	validate_source "$f" "120"
	validate_source "$f" "130"
	validate_source "$f" "150"

done

