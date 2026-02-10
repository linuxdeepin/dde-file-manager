// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROPERTYDIALOGMANAGER_H
#define PROPERTYDIALOGMANAGER_H

#include "dfmplugin_propertydialog_global.h"

#include <QObject>

namespace dfmplugin_propertydialog {

class PropertyDialogManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertyDialogManager)

public:
    static PropertyDialogManager &instance();

    bool registerExtensionView(CustomViewExtensionView viewCreator, const QString &name, int index = -1);
    QMap<int, QWidget *> createExtensionView(const QUrl &url, const QVariantHash &option = QVariantHash());
    bool registerCustomView(CustomViewExtensionView view, const QString &scheme);
    QWidget *createCustomView(const QUrl &url);

    bool registerBasicViewExtension(BasicViewFieldFunc func, const QString &scheme);
    QMap<BasicExpandType, BasicExpandMap> createBasicViewExtensionField(const QUrl &url);

    bool addBasicFiledFiltes(const QString &scheme, PropertyFilterType filters);
    PropertyFilterType basicFiledFiltes(const QUrl &url);

    void addComputerPropertyDialog();

    QVariantHash getCreatorOptionByName(const QString &name) const;

private:
    explicit PropertyDialogManager(QObject *parent = nullptr);

private:
    QMultiHash<int, QVariantHash> creatorOptions {};
    QHash<QString, CustomViewExtensionView> viewCreateFunctionHash {};
    QHash<QString, BasicViewFieldFunc> basicViewFieldFuncHash {};
    QList<QString> propertyPathList {};
    QHash<QString, PropertyFilterType> filePropertyFilterHash {};
    bool basicInfoExpandState { true };
};

}

#endif   // PROPERTYDIALOGMANAGER_H
