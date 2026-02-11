// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTSCENECREATOR_H
#define ABSTRACTSCENECREATOR_H
#include <dfm-base/dfm_base_global.h>

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
