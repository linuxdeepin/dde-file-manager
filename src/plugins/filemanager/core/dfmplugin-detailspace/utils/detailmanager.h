/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
