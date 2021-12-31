/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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

#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/interfaces/abstractbaseview.h"

#include <DListView>

DWIDGET_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

class FileViewItem;
class FileViewModel;
class FileViewPrivate;
class BaseItemDelegate;
class FileSortFilterProxyModel;
class FileView final : public DListView, public DFMBASE_NAMESPACE::AbstractBaseView
{
    Q_OBJECT
    friend class FileViewPrivate;
    QSharedPointer<FileViewPrivate> d;

public:
    enum class ClickedAction : uint8_t {
        kClicked = 0,
        kDoubleClicked
    };

    enum class ViewMode : uint8_t {
        IconMode = 0x01,
        ListMode = 0x02,
        ExtendMode = 0x04,
        AllViewMode = IconMode | ListMode | ExtendMode
    };

    explicit FileView(const QUrl &url, QWidget *parent = nullptr);

    QWidget *widget() const override;
    bool setRootUrl(const QUrl &url) override;
    QUrl rootUrl() const override;
    ViewState viewState() const override;
    QList<QAction *> toolBarActionList() const override;
    QList<QUrl> selectedUrlList() const override;
    void refresh() override;

    void setViewMode(ViewMode mode);
    void setDelegate(ViewMode mode, BaseItemDelegate *view);
    FileViewModel *model() const;
    void setModel(QAbstractItemModel *model) override;

    FileSortFilterProxyModel *proxyModel() const;
    int getColumnWidth(const int &column) const;
    int getHeaderViewWidth() const;
    int selectedIndexCount() const;

    inline void setViewModeToList()
    {
        setViewMode(ViewMode::ListMode);
    }
    inline void setViewModeToIcon()
    {
        setViewMode(ViewMode::IconMode);
    }

public slots:
    void onHeaderViewMouseReleased();
    void onHeaderSectionResized(int logicalIndex, int oldSize, int newSize);
    void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void onClicked(const QModelIndex &index);
    void onDoubleClicked(const QModelIndex &index);
    void onScalingValueChanged(const int value);
    void delayUpdateStatusBar();
    void viewModeChanged(quint64 windowId, int viewMode);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

Q_SIGNALS:
    void reqOpenNewWindow(const QList<QUrl> &urls);

private slots:
    void loadViewState(const QUrl &url);

private:
    void initializeModel();
    void initializeDelegate();
    void initializeStatusBar();
    void initializeConnect();

    void updateStatusBar();
    void setDefaultViewMode();
    void openIndexByClicked(const ClickedAction action, const QModelIndex &index);
    void openIndex(const QModelIndex &index);
    const FileViewItem *sourceItem(const QModelIndex &index) const;

    QVariant fileViewStateValue(const QUrl &url, const QString &key, const QVariant &defalutValue);
    void setFileViewStateValue(const QUrl &url, const QString &key, const QVariant &value);
};

DPWORKSPACE_END_NAMESPACE

#endif   // FILEVIEW_H
