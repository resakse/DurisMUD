#!/bin/bash
# This script pulls the files from mud/src to mud/areas/de/src as appropriate.
# The goal of this script is to update the files that are used by both the DE and
#   the main mud code.  For example, if you add a race to the MUD, this script
#   should pull the new race into the DE code and recompile.

echo -e 'This is no longer needed as we are now using links instead of duplicate files.'
exit

echo Pulling files from main code...
echo

echo Testing differences in files...
echo

if( ! diff spells.h ../../../src/spells.h > /dev/null ); then
  tput setaf 6
  cp -v -f ../../../src/spells.h .
  tput sgr0
else
  echo -e '   spells.h is the same, not pulling.'
fi

if( ! diff skills.c ../../../src/skills.c > /dev/null ); then
  tput setaf 6
  cp -v -f ../../../src/skills.c .
  tput sgr0
else
  echo '   skills.c is the same, not pulling.'
fi

if( ! diff defines.h ../../../src/defines.h > /dev/null ); then
  tput setaf 6
  cp -v -f ../../../src/defines.h .
  tput sgr0
else
  echo '  defines.h is the same, not pulling.'
fi

if( ! diff skillrec.h ../../../src/skillrec.h > /dev/null ); then
  tput setaf 6
  cp -v -f ../../../src/skillrec.h .
  tput sgr0
else
  echo ' skillrec.h is the same, not pulling.'
fi

if( ! diff objmisc.h ../../../src/objmisc.h > /dev/null ); then
  tput setaf 6
  cp -v -f ../../../src/objmisc.h .
  tput sgr0
else
  echo '  objmisc.h is the same, not pulling.'
fi

if( ! diff -q common.c ../../../src/common.c > /dev/null ); then
  tput setaf 6
  cp -v -f ../../../src/common.c .
  tput sgr0
else
  echo '   common.c is the same, not pulling.'
fi

read -p "Press [Enter] key to start compiling..."

make clean
make
