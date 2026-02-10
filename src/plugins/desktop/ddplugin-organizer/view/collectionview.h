// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONVIEW_H
#define COLLECTIONVIEW_H

#include "ddplugin_organizer_global.h"

#include <QAbstractItemView>
#include <QSharedPointer>

namespace ddplugin_organizer {

class CollectionModel;
class CollectionItemDelegate;
class CollectionDataProvider;
class CanvasModelShell;
class CanvasViewShell;
class CanvasGridShell;
class CanvasManagerShell;
class CollectionViewPrivate;
class CollectionViewBroker;
class CollectionView : public QAbstractItemView
{
    Q_OBJECT
    friend class CollectionViewPrivate;
    friend class CollectionViewBroker;

public:
    explicit CollectionView(const QString &uuid, CollectionDataProvider *dataProvider, QWidget *parent = nullptr);
    ~CollectionView() override;
    QString id() const;
    void setCanvasModelShell(CanvasModelShell *sh);
    void setCanvasViewShell(CanvasViewShell *sh);
    void setCanvasGridShell(CanvasGridShell *sh);
    void setCanvasManagerShell(CanvasManagerShell *sh);
    void setFileShiftable(const bool enable);
    bool fileShiftable() const;
    QMargins cellMargins() const;
    CollectionModel *model() const;
    CollectionItemDelegate *itemDelegate() const;
    CollectionDataProvider *dataProvider() const;
    WId winId() const;
    void updateRegionView();
    void refresh(bool silence);
    void setFreeze(bool freeze);

    void openEditor(const QUrl &url);
    void selectUrl(const QUrl &url, const QItemSelectionModel::SelectionFlag &flags);
    void selectUrls(const QList<QUrl> &urls);

    void setModel(QAbstractItemModel *model) override;
    void setSelectionModel(QItemSelectionModel *sel) override;
    void reset() override;
    void selectAll() override;
    QRect visualRect(const QModelIndex &index) const override;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
    QModelIndex indexAt(const QPoint &point) const override;
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;
    void keyboardSearch(const QString &search) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void sort(int role);
    void toggleSelect();
    using QAbstractItemView::selectedIndexes;

protected Q_SLOTS:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

protected:
    QModelIndex moveCursor(CursorAction cursorAction,
                           Qt::KeyboardModifiers modifiers) override;

    int horizontalOffset() const override;
    int verticalOffset() const override;

    bool isIndexHidden(const QModelIndex &index) const override;

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
    QRegion visualRegionForSelection(const QItemSelection &selection) const override;
    bool lessThan(const QUrl &left, const QUrl &right) const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void changeEvent(QEvent *event) override;

    void scrollContentsBy(int dx, int dy) override;

private:
    QSharedPointer<CollectionViewPrivate> d = nullptr;
};

}

#endif   // COLLECTIONVIEW_H
