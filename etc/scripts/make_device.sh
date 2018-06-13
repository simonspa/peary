#!/bin/bash

echo -e "\nPreparing code basis for a new Caribou device:\n"

# Ask for device name:
read -p "Name of the device? " DEVNAME
DEVNAMELONG="$DEVNAME"Device

# Ask for the interface type to connect to
echo ""
read -p "Interface type? " INTERFACE

# Check that interface type exists
OBJDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd -P )"
OBJDIR=$OBJDIR/../../peary/interfaces
if [ ! -e ${OBJDIR}/${INTERFACE}.hpp ]
then
  echo -e "\nInterface type \"${INTERFACE}\" does not exist. \nPlease check the supported interface types in ${OBJDIR}\n"
  exit
fi

echo "Creating directory and files..."

echo
# Try to find the devices directory:
BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd -P )"
DIRECTORIES[0]="${BASEDIR}/../../devices"
DIRECTORIES[1]="../devices"
DIRECTORIES[2]="devices"

DEVDIR=""
for DIR in "${DIRECTORIES[@]}"; do
    if [ -d "$DIR" ]; then
	DEVDIR="$DIR"
	break
    fi
done

# Create directory
mkdir "$DEVDIR/$DEVNAME"

# Copy over CMake file and sources from Example:
sed -e "s/ExampleDevice/$DEVNAMELONG/g" $DEVDIR/Example/CMakeLists.txt > $DEVDIR/$DEVNAME/CMakeLists.txt

# Copy over the README, setting current git username/email as author
# If this fails, use system username and hostname
MYNAME=$(git config user.name)
MYMAIL=$(git config user.email)
if [ -z "$MYNAME" ]; then
    MYNAME=$(whoami)
fi
if [ -z "$MYMAIL" ]; then
    MYMAIL=$(hostname)
fi
sed -e "s/ExampleDevice/$DEVNAMELONG/g" \
    -e "s/\*NAME\*/$MYNAME/g" \
    -e "s/\*EMAIL\*/$MYMAIL/g" \
    -e "s/Functional/Immature/g" \
    "$DEVDIR/Example/README.md" > "$DEVDIR/$DEVNAME/README.md"

# Copy over source code skeleton:
sed -e "s/ExampleDevice/$DEVNAMELONG/g" "$DEVDIR/Example/ExampleDevice.hpp" > "$DEVDIR/$DEVNAME/${DEVNAME}Device.hpp"
sed -e "s/ExampleDevice/$DEVNAMELONG/g" "$DEVDIR/Example/ExampleDevice.cpp" > "$DEVDIR/$DEVNAME/${DEVNAME}Device.cpp"

# Options for sed vary slightly between mac and linux
opt=-i
platform=`uname`
if [ "$platform" == "Darwin" ]; then opt="-i \"\""; fi

# Replace the corresponding interface type in the header file
command="sed ${opt} \
-e 's/PixelHit/${INTERFACE}/g' \
$DEVDIR/$DEVNAME/${DEVNAME}Device.hpp"
eval $command

# Print a summary of the module created:
FINALPATH=`realpath $DEVDIR/$DEVNAME`
echo "Name:   $DEVNAME"
echo "Author: $MYNAME ($MYMAIL)"
echo "Path:   $FINALPATH"
echo "This device connects to interface \"$INTERFACE\""
echo
echo "Re-run CMake in order to build your new device."
