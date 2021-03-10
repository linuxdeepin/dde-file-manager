/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef DFMVIEWMANAGER_H
#define DFMVIEWMANAGER_H

#include "dfmglobal.h"

#include <QObject>

#include <functional>

DFM_BEGIN_NAMESPACE

class DFMBaseView;
class DFMViewManagerPrivate;
class DFMViewManager : public QObject
{
    Q_OBJECT

    typedef QPair<QString, QString> KeyType;
    typedef QPair<QString, std::function<DFMBaseView*()>> ViewCreatorType;

public:
    static DFMViewManager *instance();

    template <class T>
    void dRegisterUrlView(const QString &scheme, const QString &host)
    {
        if (isRegisted<T>(scheme, host))
            return;

        insertToCreatorHash(KeyType(scheme, host), ViewCreatorType(typeid(T).name(), [=] () {
            return (DFMBaseView*)new T();
        }));
    }
    bool isRegisted(const QString &scheme, const QString &host, const std::type_info &info) const;
    template <class T>
    bool isRegisted(const QString &scheme, const QString &host) const
    {
        return isRegisted(scheme, host, typeid(T));
    }

    void clearUrlView(const QString &scheme, const QString &host);

    DFMBaseView *createViewByUrl(const DUrl &fileUrl) const;
    QString suitedViewTypeNameByUrl(const DUrl &fileUrl) const;
    bool isSuited(const DUrl &fileUrl, const DFMBaseView *view) const;
    template <class T>
    inline bool isSuited(const DUrl &fileUrl) const
    { return suitedViewTypeNameByUrl(fileUrl) == typeid(T).name();}

private:
    explicit DFMViewManager(QObject *parent = 0);
    ~DFMViewManager();

    void insertToCreatorHash(const KeyType &type, const ViewCreatorType &creator);

    QScopedPointer<DFMViewManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMViewManager)
};

DFM_END_NAMESPACE

#endif // DFMVIEWMANAGER_H
