#!/bin/sh

data="ZmlsZS1tYW5hZ2VyLnNo"
for i in "$@"
do
        arg_base64=`echo -n $i|base64 -w 0`
        data=$data" "$arg_base64
done

echo -n $data|socat - $XDG_RUNTIME_DIR/dde-file-manager

if [ $? != 0 ]; then
        dde-file-manager "$@"
fi
