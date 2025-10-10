// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTBYBUTTON_P_H
#define SORTBYBUTTON_P_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <DToolButton>

namespace dfmplugin_titlebar {
class SortByButton;
class SortByButtonPrivate : public QObject
{
    Q_OBJECT
    friend class SortByButton;
    SortByButton *const q;

public:
    explicit SortByButtonPrivate(SortByButton *parent);
    virtual ~SortByButtonPrivate();
    void setItemSortRoles();
    void setItemGroupRoles();
    void sort();
    QString findObjNameByGroupStrategy(const QString &strategy);
    QString findStrategyByObjName(const QString &objName);

private slots:
    void menuTriggered(QAction *action);
    void groupMenuTriggered(QAction *action);

private:
    void setupMenu();
    void initializeUi();
    void initConnect();

    bool hoverFlag { false };
    QMenu *menu { nullptr };
    QMenu *groupMenu { nullptr };
    bool iconClicked { false };   // Add iconClicked state
    QHash<QString, QString> objNamesToGroupStrategies;
};
}   // namespace dfmplugin_titlebar

#endif   // SORTROLEBUTTON_P_H
