#!/bin/sh

#file-manager.sh
data="ZmlsZS1tYW5hZ2VyLnNo"

#dde-file-manager
executable="ZGRlLWZpbGUtbWFuYWdlcg=="

#-n
newWindow="LW4="

for i in "$@"
do
        arg_base64=`echo -n $i|base64 -w 0`
        data=$data" "$arg_base64
done

target=$XDG_RUNTIME_DIR/dde-file-manager

if [ ! -S $target ];then
        target=/tmp/dde-file-manager
        if [ ! -S $target ];then
                dde-file-manager "$@"
                exit "$?"
        fi
fi

echo $executable $newWindow | socat - $target

if [ $? != 0 ]; then
        dde-file-manager "$@"
fi
