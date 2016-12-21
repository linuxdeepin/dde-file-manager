#ifndef VIEWPLUGIN_H
#define VIEWPLUGIN_H

#include  "../plugininterfaces/view/viewinterface.h"


class ViewPlugin : public QObject, public ViewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ViewInterface_iid FILE "pluginView.json")
    Q_INTERFACES(ViewInterface)
public:
    ViewPlugin(QObject *parent = 0);

    QString bookMarkText();
    QIcon bookMarkNormalIcon();
    QIcon bookMarkHoverIcon();
    QIcon bookMarkPressedIcon();
    QIcon bookMarkCheckedIcon();
    QString crumbText();
    QIcon crumbNormalIcon();
    QIcon crumbHoverIcon();
    QIcon crumbPressedIcon();
    QIcon crumbCheckedIcon();
    bool isAddSeparator();
    QString scheme();
    QWidget* createView();
};

#endif // VIEWPLUGIN_H
