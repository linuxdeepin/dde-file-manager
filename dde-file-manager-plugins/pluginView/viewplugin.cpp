#include "viewplugin.h"
#include <QLabel>

ViewPlugin::ViewPlugin(QObject *parent) :
    QObject(parent)
{
    Q_INIT_RESOURCE(pluginview);
}

QString ViewPlugin::bookMarkText()
{
    return "view";
}

QIcon ViewPlugin::bookMarkNormalIcon()
{
    return QIcon(":/images/release.png");
}

QIcon ViewPlugin::bookMarkHoverIcon()
{
    return QIcon(":/images/hover.png");
}

QIcon ViewPlugin::bookMarkPressedIcon()
{
    return QIcon(":/images/pressed.svg");
}

QIcon ViewPlugin::bookMarkCheckedIcon()
{
    return QIcon(":/images/checked.svg");
}

QString ViewPlugin::crumbText()
{
    return "view";
}

QIcon ViewPlugin::crumbNormalIcon()
{
    return QIcon(":/images/release.png");
}

QIcon ViewPlugin::crumbHoverIcon()
{
    return QIcon(":/images/hover.png");
}

QIcon ViewPlugin::crumbPressedIcon()
{
    return QIcon(":/images/hover.png");
}

QIcon ViewPlugin::crumbCheckedIcon()
{
    return QIcon(":/images/checked.svg");
}

bool ViewPlugin::isAddSeparator()
{
    return true;
}

QString ViewPlugin::scheme()
{
    return "view";
}

QWidget *ViewPlugin::createView()
{
    QLabel* label = new QLabel;
    label->setText("view");
    label->setAlignment(Qt::AlignCenter);
//    label->setStyleSheet("background-color: green; margins:0");
    return label;
}
