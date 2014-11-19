#!/bin/bash

# Description: This script changes the style format of
#              all the source code of the project.

# Run only on the directory of the script
cd "$(dirname ${BASH_SOURCE[0]})"

# Style and format recursively
astyle --style=java -C -S -xG -Y -XW -w -f -F -p -xd -k3 -y -xj -c -K -L --suffix=none --recursive ../*.cpp ../*.cc ../*.h

# Notify the user that we are done
echo 
echo "Code styling complete!"
echo

# Let the user see the output
read -n1 -r -p "Press any key to continue..." key
clear
