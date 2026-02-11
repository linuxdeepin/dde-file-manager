// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONVIEW_P_H
#define COLLECTIONVIEW_P_H

#include "view/collectionview.h"
#include "view/collectionviewmenu.h"
#include "mode/collectiondataprovider.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/mimedata/dfmmimedata.h>

#include <QAtomicInteger>
#include <QTimer>
#include <QUrl>
#include <QPointer>

#include <QGraphicsEffect>

namespace ddplugin_organizer {

class GraphicsEffect : public QGraphicsEffect
{
    Q_OBJECT
public:
    explicit GraphicsEffect(CollectionView *parent);

protected:
    void draw(QPainter *painter) override;
    void sourceChanged(ChangeFlags flags) override;
    QRectF boundingRectFor(const QRectF &sourceRect) const override;

private:
    CollectionView *view;
};

class CollectionViewPrivate : public QObject
{
    Q_OBJECT
public:
    enum class ClickedAction : uint8_t {
        kClicked = 0,
        kDoubleClicked
    };

    explicit CollectionViewPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionView *qq, QObject *parent = nullptr);
    ~CollectionViewPrivate();

    void initUI();
    void initConnect();
    void updateViewSizeData(const QSize &viewSize, const QMargins &viewMargins, const QSize &itemSize);
    void updateVerticalBarRange();
    QList<QRect> itemPaintGeomertys(const QModelIndex &index) const;
    QRect itemRect(const QModelIndex &index) const;
    QRect visualRect(const QPoint &pos) const;

    int verticalScrollToValue(const QModelIndex &index, const QRect &rect, QAbstractItemView::ScrollHint hint) const;
    QItemSelection selection(const QRect &rect) const;
    void selectItems(const QList<QUrl> &fileUrl) const;
    void selectRect(const QRect &rect) const;
    void toggleSelect() const;

    QPoint pointToPos(const QPoint &point) const;
    QPoint posToPoint(const QPoint &pos) const;
    int posToNode(const QPoint &pos) const;
    QPoint nodeToPos(const int node) const;

    void checkTouchDarg(QMouseEvent *event);
    bool isDelayDrag() const;
    QPixmap polymerizePixmap(QModelIndexList indexs) const;

    bool checkClientMimeData(QDragEnterEvent *event) const;
    bool checkXdndDirectSave(QDragEnterEvent *event) const;
    bool checkProhibitPaths(QDragEnterEvent *event) const;
    void preproccessDropEvent(QDropEvent *event, const QUrl &targetUrl) const;
    void handleMoveMimeData(QDropEvent *event, const QUrl &url);
    bool drop(QDropEvent *event);

    void helpAction();
    void openFiles();
    void moveToTrash();
    void showMenu();
    void deleteFiles();
    void clearClipBoard();
    void selectCollection();
    void copyFiles();
    void copyFilePath();
    void cutFiles();
    void pasteFiles();
    void undoFiles();
    void previewFiles();
    void showFilesProperty();
    void continuousSelection(const QPersistentModelIndex &newCurrent);
    QModelIndex findIndex(const QString &key, bool matchStart, const QModelIndex &current, bool reverseOrder, bool excludeCurrent) const;

    void updateDFMMimeData(QDropEvent *event);
    void redoFiles();
    bool checkTargetEnable(QDropEvent *event, const QUrl &targetUrl);
    void openIndexByClicked(const ClickedAction action, const QModelIndex &index);
    void openIndex(const QModelIndex &index);

private:
    void updateRowCount(const int &viewHeight, const int &itemHeight);
    void updateColumnCount(const int &viewWidth, const int &itemWidth);
    void updateCellMargins(const QSize &itemSize, const QSize &cellSize);
    void updateViewMargins(const QSize &viewSize, const QMargins &oldMargins);

    void drawDragText(QPainter *painter, const QString &str, const QRect &rect) const;
    void drawEllipseBackground(QPainter *painter, const QRect &rect) const;
    void updateTarget(const QMimeData *data, const QUrl &url);

    bool dropFilter(QDropEvent *event);
    bool dropClientDownload(QDropEvent *event) const;
    bool dropDirectSaveMode(QDropEvent *event) const;
    bool dropBetweenCollection(QDropEvent *event) const;
    bool dropFromCanvas(QDropEvent *event) const;
    bool dropMimeData(QDropEvent *event) const;
    bool dropFiles(QDropEvent *event) const;

private slots:
    void onItemsChanged(const QString &key);

public:
    CollectionView *q = nullptr;
    CollectionItemDelegate *delegate = nullptr;
    CanvasModelShell *canvasModelShell = nullptr;
    CanvasViewShell *canvasViewShell = nullptr;
    CanvasGridShell *canvasGridShell = nullptr;
    CanvasManagerShell *canvasManagerShell = nullptr;
    bool fileShiftable = false;
    CollectionViewMenu *menuProxy = nullptr;

    QString id;
    QPointer<CollectionDataProvider> provider = nullptr;

    int space = 0;
    QMargins viewMargins;
    QMargins cellMargins = QMargins(2, 2, 2, 2);
    int rowCount = 1;
    int columnCount = 1;
    int cellWidth = 1;
    int cellHeight = 1;

    QAtomicInteger<bool> canUpdateVerticalBarRange = true;
    QAtomicInteger<bool> needUpdateVerticalBarRange = false;
    bool showGrid = false;

    QTimer touchDragTimer;
    QUrl dropTargetUrl;

    QPersistentModelIndex pressedIndex;
    QPersistentModelIndex currentSelectionStartIndex;
    Qt::KeyboardModifiers pressedModifiers;
    QPoint pressedPosition;
    QRect elasticBand;
    bool pressedAlreadySelected = false;
    bool ignoreMouseEvent = false;

    Qt::SortOrder sortOrder = Qt::DescendingOrder;
    int sortRole = DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole;

    QString searchKeys;
    QTimer *searchTimer = nullptr;

    bool flicker = false;
    bool freeze { false };

    DFMBASE_NAMESPACE::DFMMimeData dfmmimeData;
};

}

#endif   // COLLECTIONVIEW_P_H
