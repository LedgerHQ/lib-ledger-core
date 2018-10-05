#!/bin/bash
echo $1
echo $2
if ! otool -l $1 | grep LC_RPATH; then
	if [ "$2" == "x86_64" ]; then
		install_name_tool -add_rpath "@executable_path/Frameworks/$2" $1;
	else
		install_name_tool -add_rpath "@executable_path/Frameworks" $1;
	fi
else
    echo 'Otool Operation Skipped';
fi;
