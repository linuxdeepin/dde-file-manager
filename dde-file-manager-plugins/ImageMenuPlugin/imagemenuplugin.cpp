#include "imagemenuplugin.h"
#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QDebug>


ImageMenuPlugin::ImageMenuPlugin(QObject *parent) :
    QObject(parent)
{

}

QList<QIcon> ImageMenuPlugin::additionalIcons(const QString &file)
{
    QList<QIcon> icons;
    QIcon icon(":/images/phone.svg");
//    icons << icon;
    return icons;
}

QList<QAction *> ImageMenuPlugin::additionalMenu(const QStringList &files, const QString& currentDir)
{
    QList<QAction *> actions;
    QAction* formatAction = new QAction(tr("Format convert"), this);
    QAction* testAction = new QAction(tr("Format test"), this);
    QAction* sublAction = new QAction(tr("SubMenu"), this);

    actions << formatAction << testAction << sublAction;


    QMenu* subsubmenu = new QMenu;
    QList<QAction *> subsublActions;
    for (int i=0; i <10;  i++) {
        QAction* action = new QAction(QString::number(i), this);
        connect(action, &QAction::triggered, [](){
            qDebug() << "=======12345=======";
        });
        subsublActions.append(action);
    }
    subsubmenu->addActions(subsublActions);


    QMenu* menu = new QMenu;
    QList<QAction *> sublActions;
    for (int i=0; i <10;  i++) {
        QAction* action = new QAction(QString::number(i), this);
        connect(action, &QAction::triggered, [=](){
            qDebug() << "======dsd========";
        });
        sublActions.append(action);

        if (i % 2 == 0){
            action->setMenu(subsubmenu);
        }
    }
    menu->addActions(sublActions);
    sublAction->setMenu(menu);

    connect(formatAction, &QAction::triggered, this, &ImageMenuPlugin::handleFormat);
    connect(testAction, &QAction::triggered, this, &ImageMenuPlugin::handleTest);

    return actions;
}

QList<QAction *> ImageMenuPlugin::additionalEmptyMenu(const QString &currentDir)
{
    QList<QAction *> actions;
    QAction* formatAction = new QAction(tr("dssdds"), this);
    QAction* testAction = new QAction(tr("sdsdds"), this);
    QAction* sublAction = new QAction(tr("sdds"), this);

    actions << formatAction << testAction << sublAction;

    return actions;
}

void ImageMenuPlugin::handleFormat()
{
    qDebug() << "1111111111111111";
}

void ImageMenuPlugin::handleTest()
{
    qDebug() << "2222222222222222";
}
