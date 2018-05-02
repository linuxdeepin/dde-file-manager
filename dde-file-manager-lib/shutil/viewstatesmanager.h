/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef VIEWSTATESMANAGER_H
#define VIEWSTATESMANAGER_H

#include <QObject>
#include "../views/dfileview.h"
#include "dfilesystemmodel.h"
#include <QJsonObject>
#include "durl.h"

struct ViewState{
    int iconSize = -1;
//    int sortRole = -1;
//    Qt::SortOrder sortOrder;
    DFileView::ViewMode viewMode;
    bool isValid() const;
};

Q_DECLARE_FLAGS(SortOrders, Qt::SortOrder)

Q_DECLARE_METATYPE(ViewState)

class ViewStatesManager : public QObject
{
    Q_OBJECT
    Q_ENUM(Qt::SortOrder)
public:
    explicit ViewStatesManager(QObject *parent = 0);
    void initData();

    void loadViewStates(const QJsonObject &viewStateObj);
    void loadDefaultViewStates(const QJsonObject& viewStateObj);

    void saveViewState(const DUrl& url, const ViewState& viewState);
    ViewState viewstate(const DUrl& url);

    static QString getViewStateFilePath();
    static ViewState objectToViewState(const QJsonObject& obj);
    static QJsonObject viewStateToObject(const ViewState& viewState);
    static QString getDefaultViewStateConfigFile();
    static bool isValidViewStateObj(const QJsonObject& obj);
    static bool isValidViewState(const ViewState& state);

signals:

public slots:

private:
    QMap<DUrl, ViewState> m_viewStatesMap;
    QMap<DUrl, ViewState> m_defaultViewStateMap;
    QJsonObject m_viewStatesJsonObject;
    QJsonObject m_defautlViewStateJsonObject;

};

#endif // VIEWSTATESMANAGER_H
