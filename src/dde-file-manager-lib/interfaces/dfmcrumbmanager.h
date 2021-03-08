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

#ifndef DFMCRUMBMANAGER_H
#define DFMCRUMBMANAGER_H

#include "dfmglobal.h"

#include <QObject>

DFM_BEGIN_NAMESPACE

class DFMCrumbBar;
class DFMCrumbInterface;
class DFMCrumbManagerPrivate;
class DFMCrumbManager : public QObject
{
    Q_OBJECT

    typedef QString KeyType;
    typedef QPair<QString, std::function<DFMCrumbInterface*()>> CrumbCreaterType;

public:
    static DFMCrumbManager *instance();

    template <class T>
    void dRegisterCrumbCreator(const QString &scheme)
    {
        if (isRegisted<T>(scheme))
            return;

        insertToCreatorHash(KeyType(scheme), CrumbCreaterType(typeid(T).name(), [ = ]() {
            return static_cast<DFMCrumbInterface *>(new T());
        }));
    }

    bool isRegisted(const QString &scheme, const std::type_info &info) const;

    template <class T>
    bool isRegisted(const QString &scheme) const
    {
        return isRegisted(scheme, typeid(T));
    }

    DFMCrumbInterface *createControllerByUrl(const DUrl &fileUrl, DFMCrumbBar *crumbBar) const;

private:
    explicit DFMCrumbManager(QObject *parent = nullptr);
    ~DFMCrumbManager();

    void insertToCreatorHash(const KeyType &type, const CrumbCreaterType &creator);

    QScopedPointer<DFMCrumbManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMCrumbManager)

};

DFM_END_NAMESPACE

#endif // DFMCRUMBMANAGER_H
