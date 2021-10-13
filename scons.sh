DIR="$( cd "$( dirname "$(readlink -f "$0")" )" > /dev/null && pwd )"
SHPATH="$(ls -t "$HOME/.vscode/extensions/entan-gl.cc65-vice-"*"/build.sh" | head -1)"
if test -e "$DIR/../../../build.sh"; then
    sh "$DIR/../../../build.sh" scons "$@"
elif test -e "$SHPATH"; then
    sh "$SHPATH" scons "$@"
else
    scons "$@"
fi
