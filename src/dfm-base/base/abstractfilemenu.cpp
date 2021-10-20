#include "abstractfilemenu.h"

#include <QAction>
#include <QMenu>

DFMBASE_BEGIN_NAMESPACE

namespace FileMenuTypes
{
    const QString CreateNewDir = {QObject::tr("Create New Dir")};
    const QString CreateNewDoc = {QObject::tr("Create New Doc")};
    const QString ViewModeSwitch = {QObject::tr("Switch View Mode")};
    const QString ViewSortSwitch = {QObject::tr("Switch View Sort")};
    const QString OpenAsAdmin = {QObject::tr("Open As Admin")};
    const QString OpenInTerminal {QObject::tr("Open In Terminal")};
    const QString SelectAll {QObject::tr("Select All")};
    const QString Property {QObject::tr("Property")};
    const QString Cut {QObject::tr("Cut")};
    const QString Copy {QObject::tr("Copy")};
    const QString Paste {QObject::tr("Paste")};
    const QString Rename = {QObject::tr("Rename")};
} //namespace FileMenuTypes

AbstractFileMenu::AbstractFileMenu(QObject *parent)
    : QObject (parent)
{

}

AbstractFileMenu::~AbstractFileMenu()
{

}

QMenu *AbstractFileMenu::build(AbstractFileMenu::MenuMode mode,
                               const QUrl &rootUrl,
                               const QUrl &foucsUrl,
                               const QList<QUrl> &selected)
{
    Q_UNUSED(rootUrl)
    Q_UNUSED(foucsUrl)
    Q_UNUSED(selected)

    if (AbstractFileMenu::MenuMode::Empty == mode) {
        QMenu *menu = new QMenu(nullptr);
        menu->addAction(FileMenuTypes::CreateNewDir);
        menu->addAction(FileMenuTypes::CreateNewDoc);
        menu->addAction(FileMenuTypes::ViewModeSwitch);
        menu->addAction(FileMenuTypes::ViewSortSwitch);
        menu->addAction(FileMenuTypes::OpenAsAdmin);
        menu->addAction(FileMenuTypes::OpenInTerminal);
        menu->addAction(FileMenuTypes::SelectAll);
        menu->addAction(FileMenuTypes::Property);
        return menu;
    }

    if (AbstractFileMenu::MenuMode::Normal == mode) {
        QMenu *menu = new QMenu(nullptr);
        menu->addAction(FileMenuTypes::CreateNewDir);
        menu->addAction(FileMenuTypes::CreateNewDoc);
        menu->addAction(FileMenuTypes::ViewModeSwitch);
        menu->addAction(FileMenuTypes::ViewSortSwitch);
        menu->addAction(FileMenuTypes::OpenAsAdmin);
        menu->addAction(FileMenuTypes::OpenInTerminal);
        menu->addAction(FileMenuTypes::SelectAll);
        menu->addAction(FileMenuTypes::Property);
        return menu;
    }

    return nullptr;
}

DFMBASE_END_NAMESPACE
