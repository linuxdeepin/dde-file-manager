#!/bin/sh

data="ZmlsZS1tYW5hZ2VyLnNo"
for i in "$@"
do
        arg_base64=`echo -n $i|base64 -w 0`
        data=$data" "$arg_base64
done

target=$XDG_RUNTIME_DIR/dde-file-manager

if [ ! -f $target ];then
        target=/tmp/dde-file-manager
        if [ ! -f $target ];then
                dde-file-manager "$@"
                exit "$?"
        fi
fi

echo -n $data|socat - $tagrt

if [ $? != 0 ]; then
        dde-file-manager "$@"
fi
