#include "dadvancedinfoplugin.h"
#include "dadvancedinfowidget.h"
DAdvancedInfoPlugin::DAdvancedInfoPlugin(QObject *parent)
    : PropertyDialogExpandInfoInterface(parent)
{

}

QWidget *DAdvancedInfoPlugin::expandWidget(const QString &file)
{
    return new DAdvancedInfoWidget(0, file);
}

QString DAdvancedInfoPlugin::expandWidgetTitle(const QString &file)
{
    Q_UNUSED(file);
    return "Advanced info";
}
