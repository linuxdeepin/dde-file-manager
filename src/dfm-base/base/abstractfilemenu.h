#ifndef ABSTRACTFILEMENU_H
#define ABSTRACTFILEMENU_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>

QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE
class AbstractFileMenuPrivate;

namespace FileMenuTypes
{
    extern const QString CreateNewDir;
    extern const QString CreateNewDoc;
    extern const QString ViewModeSwitch;
    extern const QString ViewSortSwitch;
    extern const QString OpenAsAdmin;
    extern const QString OpenInTerminal;
    extern const QString SelectAll;
    extern const QString Property;
    extern const QString Cut;
    extern const QString Copy;
    extern const QString Paste;
    extern const QString Rename;
} //namesapce FileMenuTypes

class AbstractFileMenu : public QObject
{
    Q_OBJECT
public:
    enum MenuMode{
        Empty,
        Normal,
    };

    explicit AbstractFileMenu(QObject *parent = nullptr);
    virtual ~AbstractFileMenu();
    virtual QMenu *build(MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &foucsUrl,
                         const QList<QUrl> &selected = {});
};

DFMBASE_END_NAMESPACE

#endif // ABSTRACTFILEMENU_H
