#ifndef DFMCTXMENUPLUGIN_H
#define DFMCTXMENUPLUGIN_H

#include "dde-file-manager-plugins/menuinterface.h"

class DFMCtxMenuPlugin : public MenuInterface
{
    Q_OBJECT
    Q_INTERFACES(MenuInterface)
    Q_PLUGIN_METADATA(IID MenuInterface_iid)
public:
    DFMCtxMenuPlugin();

    QList<QAction*> additionalMenu(const QStringList &files, const QString& currentDir) override;
};

#endif // DFMCTXMENUPLUGIN_H
