// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    //NOTE [XIAO] 从PLGUIN中加载View
    void initViewFromPlugin();
    void insertToCreatorHash(const KeyType &type, const ViewCreatorType &creator);

    QScopedPointer<DFMViewManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMViewManager)
};

DFM_END_NAMESPACE

#endif // DFMVIEWMANAGER_H
