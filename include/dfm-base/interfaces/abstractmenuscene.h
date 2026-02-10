// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTMENUSCENE_H
#define ABSTRACTMENUSCENE_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
QT_END_NAMESPACE

Q_DECLARE_METATYPE(const char *)

namespace dfmbase {

class AbstractMenuScenePrivate;
class AbstractMenuScene : public QObject
{
    Q_OBJECT
public:
    explicit AbstractMenuScene(QObject *parent = nullptr);
    virtual ~AbstractMenuScene();
    virtual QString name() const = 0;
    virtual bool initialize(const QVariantHash &params);
    virtual bool create(QMenu *parent);
    virtual void updateState(QMenu *parent);
    virtual bool triggered(QAction *action);
    virtual bool actionFilter(AbstractMenuScene *caller, QAction *action);
    virtual AbstractMenuScene *scene(QAction *action) const;
    virtual bool addSubscene(AbstractMenuScene *scene);
    virtual void removeSubscene(AbstractMenuScene *scene);
    inline QList<AbstractMenuScene *> subscene() const
    {
        return subScene;
    }

protected:
    virtual void setSubscene(const QList<AbstractMenuScene *> &scenes);

protected:
    QList<AbstractMenuScene *> subScene;
};

//! this function must not be called in `actionFilter`
inline bool filterActionBySubscene(AbstractMenuScene *self, QAction *action)
{
    if (!self || !action)
        return false;

    for (AbstractMenuScene *child : self->subscene())
        if (child->actionFilter(self, action))
            return true;

    return false;
}

}

#endif   // ABSTRACTMENUSCENE_H
