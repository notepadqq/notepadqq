#!/bin/sh

# This script helps automate generation of the resources.qrc file
# Available arguments:
#    --quiet  Skips asking to view and update the file, updating resources.qrc
#             without confirmation

PROJECT_DIR="$(dirname $( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd ))"
CURRENT_DIR="$(pwd)"

# Environment variables you can use to improve your viewing experience
DIFFTOOL="$DIFFTOOL"
EDITOR="$EDITOR"

# Put in the order of preferred, first to last
DIFF_BINS="vimdiff diff"
if [ -z "$DIFFTOOL" ]; then
    for d in $DIFF_BINS; do
        which $d > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            DIFFTOOL="$(which $d 2> /dev/null)"
            break
        fi
    done
fi

if [ -z "$EDITOR" ]; then
    EDITOR="$(which less 2> /dev/null)"
fi


rcc_add_icons()
{
    # Sort our icons by folder so its easier to read
    ICON_FILES=("$(find src/ui/icons -name index.theme)")
    ICON_FILES+=($(find src/ui/icons -type f -not -name index.theme | sort -t/ -nk5))
    echo "    <qresource prefix=\"icons/notepadqq\">"
    for f in ${ICON_FILES[@]}; do
        f="$(echo $f | cut -d/ -f3-)"
        falias="$(echo $f | cut -d/ -f3-)"
        echo "        <file alias=\"$falias\">$f</file>"
    done
    echo "    </qresource>"
    cd "$CURRENT_DIR"
}

rcc_add_translations()
{
    TRANSLATION_FILES=($(find src/translations -type f -name '*.qm' | sort))
    echo "    <qresource prefix=\"/translations\">"
    for f in ${TRANSLATION_FILES[@]}; do
        f="$(echo $f | cut -d/ -f2-)"
        falias="$(echo $f | cut -d/ -f2-)"
        echo "        <file alias=\"$falias\">../$f</file>"
    done
    echo "    </qresource>"
}

rcc_replace_file()
{
    echo -n "Replacing old resources.qrc..."
    mv "${TEMP_FILE}" "src/ui/resources.qrc"
    echo "Done."
}

cd "$PROJECT_DIR"

TEMP_FILE="$(mktemp -t resources.qrc.XXXXXXXX)"
echo -n "Generating new resources.qrc file..."

echo "<RCC>" > $TEMP_FILE
rcc_add_icons >> $TEMP_FILE
rcc_add_translations >> $TEMP_FILE
echo "</RCC>" >> $TEMP_FILE

echo "Done."
if [ "$1" != "--quiet" ]; then
    while true; do
        read -p "Would you like to view the updated resources.qrc? [Y/n] " yn
        case $yn in
            [Nn]* ) break;;
            [Yy]*|"" ) less "$TEMP_FILE"; break;;
            * ) echo "Valid options are y/n, or press enter for default.";;
        esac
    done

    while true; do
        read -p "Would you like to update resources.qrc now? [Y/n] " yn
        case $yn in
            [Nn]* ) break;;
            [Yy]*|"" ) rcc_replace_file; break;;
            * ) echo "Valid options are y/n, or press enter for default.";;
        esac
    done
else
    rcc_replace_file
fi

cd "$CURRENT_DIR"
