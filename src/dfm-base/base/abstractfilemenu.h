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

namespace FileMenuTypes {
extern const QString CREATE_NEW_DIR;
extern const QString CREATE_NEW_DOC;
extern const QString VIEW_MODE_SWITCH;
extern const QString VIEW_SORT_SWITCH;
extern const QString OPEN_AS_ADMIN;
extern const QString OPEN_IN_TERMINAL;
extern const QString SELECT_ALL;
extern const QString PROPERTY;
extern const QString CUT;
extern const QString COPY;
extern const QString PASTE;
extern const QString RENAME;
} // namesapce FileMenuTypes

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
