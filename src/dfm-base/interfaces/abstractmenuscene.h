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

DFMBASE_BEGIN_NAMESPACE

class AbstractMenuScenePrivate;
class AbstractMenuScene : public QObject
{
public:
    explicit AbstractMenuScene(QObject *parent = nullptr);
    virtual ~AbstractMenuScene();
    virtual QString name() const = 0;
    virtual bool initialize(const QVariantHash &params);
    virtual bool create(QMenu *parent);
    virtual void updateState(QMenu *parent);
    virtual bool triggered(QAction *action);
    virtual AbstractMenuScene *scene(QAction *action) const;
    virtual bool addSubscene(AbstractMenuScene *scene);
    virtual void removeSubscene(AbstractMenuScene *scene);
    inline QList<AbstractMenuScene *> subscene() const
    {
        return subScene;
    }
protected:
    virtual void setSubscene(const QList<dfmbase::AbstractMenuScene *> &scenes);
protected:
    QList<AbstractMenuScene *> subScene;
};

DFMBASE_END_NAMESPACE

#endif   // ABSTRACTMENUSCENE_H
