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

#ifndef SIDEBARIMAGEVIEWMODEL_H
#define SIDEBARIMAGEVIEWMODEL_H

#include "preview_plugin_global.h"

#include <QAbstractListModel>

PREVIEW_BEGIN_NAMESPACE
typedef enum E_SideBar {
    SIDE_THUMBNIL = 0
} E_SideBar;

typedef enum ImageinfoType_e {
    IMAGE_PIXMAP = Qt::UserRole,
    IMAGE_ROTATE = Qt::UserRole + 2,
    IMAGE_PAGE_SIZE = Qt::UserRole + 6,
} ImageinfoType_e;

typedef struct ImagePageInfo_t
{
    int pageIndex;

    QString strcontents;

    explicit ImagePageInfo_t(int index = -1);

    bool operator==(const ImagePageInfo_t &other) const;

    bool operator<(const ImagePageInfo_t &other) const;

    bool operator>(const ImagePageInfo_t &other) const;
} ImagePageInfo_t;
PREVIEW_END_NAMESPACE

Q_DECLARE_METATYPE(PREVIEW_NAMESPACE::ImagePageInfo_t);

PREVIEW_BEGIN_NAMESPACE
class DocSheet;
class SideBarImageViewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SideBarImageViewModel(DocSheet *sheet, QObject *parent = nullptr);

public:
    /**
     * @brief resetData
     * 清空数据
     */
    void resetData();

    /**
     * @brief initModelLst
     * 初始化MODEL数据
     * @param pagelst
     * @param sort
     */
    void initModelLst(const QList<ImagePageInfo_t> &pagelst, bool sort = false);

    /**
     * @brief changeModelData
     * 重置model数据
     * @param pagelst
     */
    void changeModelData(const QList<ImagePageInfo_t> &pagelst);

    /**
     * @brief getModelIndexForPageIndex
     * 根据页码获取对应MODEL INDEX
     * @param pageIndex
     * @return
     */
    QList<QModelIndex> getModelIndexForPageIndex(int pageIndex);

    /**
     * @brief getPageIndexForModelIndex
     * 根据节点索引值,获取对应页码
     * @param row
     * @return
     */
    int getPageIndexForModelIndex(int row);

    /**
     * @brief handleThumbnail
     * 处理缩略图
     * @param index
     * @param pixmap
     */
    void handleRenderThumbnail(int index, QPixmap pixmap);

public slots:
    /**
     * @brief onUpdateImage
     * 通知指定页码数据变更
     * @param index
     */
    void onUpdateImage(int index);

protected:
    /**
     * @brief columnCount
     * view列数
     * @return
     */
    int columnCount(const QModelIndex &) const override;

    /**
     * @brief rowCount
     * view行数
     * @param parent
     * @return
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief data
     * view显示数据
     * @param index
     * @param role
     * @return
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief setData
     * 设置model数据
     * @param index
     * @param data
     * @param role
     * @return
     */
    bool setData(const QModelIndex &index, const QVariant &data, int role) override;

private:
    QObject *parentObj { nullptr };
    DocSheet *docSheet { nullptr };
    QList<ImagePageInfo_t> pageList;
    static QMap<QObject *, QVector<QPixmap>> g_sheetPixmapMap;
};
PREVIEW_END_NAMESPACE
#endif   // SIDEBARIMAGEVIEWMODEL_H
