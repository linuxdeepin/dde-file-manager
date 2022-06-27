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
#ifndef ABSTRACTSCENECREATOR_H
#define ABSTRACTSCENECREATOR_H
#include "dfm-base/dfm_base_global.h"

#include <QObject>

namespace dfmbase {

class AbstractMenuScene;
class AbstractSceneCreator : public QObject
{
    Q_OBJECT
public:
    AbstractSceneCreator();
    virtual ~AbstractSceneCreator();
    virtual AbstractMenuScene *create() = 0;
    virtual bool addChild(const QString &scene);
    virtual void removeChild(const QString &scene);
    inline QStringList getChildren() const
    {
        return children;
    }

protected:
    QStringList children;
};

}

#endif   // ABSTRACTSCENECREATOR_H
