/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
    void unregisterExtensionView(int index);
    QMap<int, QWidget *> createExtensionView(const QUrl &url, const QVariantHash &option = QVariantHash());
    bool registerCustomView(CustomViewExtensionView view, const QString &scheme);
    void unregisterCustomView(const QString &scheme);
    QWidget *createCustomView(const QUrl &url);

    bool registerBasicViewExtension(BasicViewFieldFunc func, const QString &scheme);
    void unregisterBasicViewExtension(const QString &scheme);
    QMap<BasicExpandType, BasicExpandMap> createBasicViewExtensionField(const QUrl &url);

    bool addBasicFiledFiltes(const QString &scheme, PropertyFilterType filters);
    void removeBasicFiledFilters(const QString &scheme);
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
