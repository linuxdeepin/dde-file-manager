#ifndef DADVANCEDINFOPLUGIN_H
#define DADVANCEDINFOPLUGIN_H

#include <../dde-file-manager-plugins/plugininterfaces/menu/menuinterface.h>
#include <QObject>

class QWidget;

class DAdvancedInfoPlugin : public PropertyDialogExpandInfoInterface
{
    Q_OBJECT
    Q_INTERFACES(PropertyDialogExpandInfoInterface)
    Q_PLUGIN_METADATA(IID PropertyDialogExpandInfoInterface_iid FILE "dde-advanced-property-plugin.json")

public:
    explicit DAdvancedInfoPlugin(QObject *parent = 0);

    QWidget* expandWidget(const QString& file);

    QString expandWidgetTitle(const QString& file);
};

#endif // DADVANCEDINFOPLUGIN_H
