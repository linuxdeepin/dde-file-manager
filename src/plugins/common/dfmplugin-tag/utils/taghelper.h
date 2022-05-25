/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef TAGHELPER_H
#define TAGHELPER_H

#include "dfmplugin_tag_global.h"

#include "services/filemanager/sidebar/sidebar_defines.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include <QColor>

class QPainter;

DPTAG_BEGIN_NAMESPACE

inline constexpr int kTagDiameter { 10 };

struct TagColorDefine
{
    QString colorName;
    QString iconName;
    QString displayName;
    QColor color;

public:
    TagColorDefine(const QString &colorName,
                   const QString &iconName,
                   const QString &dispaly,
                   const QColor &color);
};

class TagHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TagHelper)
public:
    static TagHelper *instance();

    QList<QColor> defualtColors() const;

    QColor qureyColorByColorName(const QString &name) const;
    QColor qureyColorByDisplayName(const QString &name) const;
    QString qureyColorNameByColor(const QColor &color) const;
    QString qureyIconNameByColorName(const QString &colorName) const;
    QString qureyIconNameByColor(const QColor &color) const;
    QString qureyDisplayNameByColor(const QColor &color) const;
    QString qureyColorNameByDisplayName(const QString &name) const;

    QString getTagNameFromUrl(const QUrl &url) const;
    QUrl makeTagUrlByTagName(const QString &tag) const;

    QString getColorNameByTag(const QString &tagName) const;
    bool isDefualtTag(const QString &tagName) const;

    void paintTags(QPainter *painter, QRectF &rect, const QList<QColor> &colors) const;

    DSB_FM_NAMESPACE::SideBar::ItemInfo createSidebarItemInfo(const QString &tag);

    void showTagEdit(const QRectF &parentRect, const QRectF &iconRect, const QList<QUrl> &fileList);

    static QUrl redirectTagUrl(const QUrl &url);

    // services instance
    static DSB_FM_NAMESPACE::WindowsService *winServIns();
    static DSB_FM_NAMESPACE::TitleBarService *titleServIns();
    static DSB_FM_NAMESPACE::SideBarService *sideBarServIns();
    static DSB_FM_NAMESPACE::WorkspaceService *workspaceServIns();
    static DSC_NAMESPACE::FileOperationsService *fileOperationsServIns();
    static DPF_NAMESPACE::EventSequenceManager *eventSequence();

private:
    explicit TagHelper(QObject *parent = nullptr);
    void initTagColorDefines();
    TagColorDefine randomTagDefine() const;

    QList<TagColorDefine> colorDefines;
};

DPTAG_END_NAMESPACE

#endif   // TAGHELPER_H
