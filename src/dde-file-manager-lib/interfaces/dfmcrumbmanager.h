// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    //NOTE [XIAO] 从PLUGIN中加载面包屑插件
    void initCrumbControllerFromPlugin();

    Q_DECLARE_PRIVATE(DFMCrumbManager)

};

DFM_END_NAMESPACE

#endif // DFMCRUMBMANAGER_H
