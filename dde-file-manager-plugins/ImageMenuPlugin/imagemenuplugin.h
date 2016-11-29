#ifndef IMAGEMENUPLUGIN_H
#define IMAGEMENUPLUGIN_H

#include <QObject>
#include "../plugininterfaces/menu/menuinterface.h"


class ImageMenuPlugin : public QObject, public MenuInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID MenuInterface_iid FILE "ImageMenuPlugin.json")
    Q_INTERFACES(MenuInterface)

public:
    ImageMenuPlugin(QObject *parent = 0);

    QList<QIcon> additionalIcons(const QString &file);
    QList<QAction *> additionalMenu(const QStringList &files);
    QList<QAction*> additionalEmptyMenu();

public:
    void handleFormat();
    void handleTest();
};

#endif // IMAGEMENUPLUGIN_H
