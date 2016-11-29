#ifndef MENUINTERFACE
#define MENUINTERFACE

#include <QList>
#include <QIcon>
#include <QAction>

class MenuInterface
{
public:
    virtual ~MenuInterface() {}

    virtual QList<QIcon> additionalIcons(const QString &file)
    {
        Q_UNUSED(file)
        QList<QIcon> icons;
        return icons;
    }

    virtual QList<QAction*> additionalMenu(const QStringList &files){
        Q_UNUSED(files)
        QList<QAction*> actions;
        return actions;
    }

    virtual QList<QAction*> additionalEmptyMenu(){
        QList<QAction*> actions;
        return actions;
    }
};



#define MenuInterface_iid "com.deepin.dde-file-manager.MenuInterface"
Q_DECLARE_INTERFACE(MenuInterface, MenuInterface_iid)

#endif // MENUINTERFACE

