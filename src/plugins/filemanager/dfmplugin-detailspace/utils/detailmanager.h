// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILMANAGER_H
#define DETAILMANAGER_H

#include "dfmplugin_detailspace_global.h"

#include <QObject>

namespace dfmplugin_detailspace {

class DetailManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DetailManager)

public:
    static DetailManager &instance();

    bool registerExtensionView(CustomViewExtensionView view, int index = -1);
    QMap<int, QWidget *> createExtensionView(const QUrl &url);

    bool registerBasicViewExtension(const QString &scheme, BasicViewFieldFunc func);
    bool registerBasicViewExtensionRoot(const QString &scheme, BasicViewFieldFunc func);
    QMap<BasicExpandType, BasicExpandMap> createBasicViewExtensionField(const QUrl &url);

    bool addBasicFiledFiltes(const QString &scheme, DetailFilterType filters);
    bool addRootBasicFiledFiltes(const QString &scheme, DetailFilterType filters);
    DetailFilterType basicFiledFiltes(const QUrl &url);

private:
    explicit DetailManager(QObject *parent = nullptr);

private:
    QMultiHash<int, CustomViewExtensionView> constructList {};
    QHash<QString, DetailFilterType> detailFilterHashNormal {};
    QHash<QString, DetailFilterType> detailFilterHashRoot {};
    QHash<QString, BasicViewFieldFunc> basicViewFieldFuncHashNormal {};
    QHash<QString, BasicViewFieldFunc> basicViewFieldFuncHashRoot {};
};

}   // namespace dfmplugin_detailspace

#endif   // DETAILMANAGER_H
