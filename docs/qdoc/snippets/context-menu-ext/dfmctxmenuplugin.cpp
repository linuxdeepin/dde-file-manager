#include "dfmctxmenuplugin.h"

DFMCtxMenuPlugin::DFMCtxMenuPlugin()
{
}

QList<QAction *> DFMCtxMenuPlugin::additionalMenu(const QStringList &files, const QString &currentDir)
{
    QList<QAction *> actionPtrList;
    actionPtrList.append(new QAction("Sample Text"));
    return actionPtrList;
}
