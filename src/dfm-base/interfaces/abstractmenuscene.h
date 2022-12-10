/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ABSTRACTMENUSCENE_H
#define ABSTRACTMENUSCENE_H
#include "dfm-base/dfm_base_global.h"

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
