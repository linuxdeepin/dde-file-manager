// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASGRIDVIEW_H
#define CANVASGRIDVIEW_H

#include <QAbstractItemView>
#include <QScopedPointer>
#include <dfilemenumanager.h>
#include <QTime>

#define DesktopCanvasPath           "/com/deepin/dde/desktop/canvas"
#define DesktopCanvasInterface      "com.deepin.dde.desktop.Canvas"

enum DMD_TYPES : unsigned int;
class DUrl;
class DesktopItemDelegate;
class DFileSystemModel;
class DFileSelectionModel;
class CanvasViewPrivate;
class CanvasGridView: public QAbstractItemView
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DesktopCanvasInterface)

    Q_PROPERTY(double dodgeDuration READ dodgeDuration WRITE setDodgeDuration NOTIFY dodgeDurationChanged)
public:
    static QMap<DMD_TYPES, bool> virtualEntryExpandState;

    explicit CanvasGridView(const QString &screen, QWidget *parent = Q_NULLPTR);
    ~CanvasGridView() override;


    enum ContextMenuAction {
        DisplaySettings = MenuAction::UserMenuAction + 1,
        CornerSettings,
        WallpaperSettings,

        FileManagerProperty,

        AutoMerge,
        AutoSort,

        IconSize,
        IconSize0 = IconSize,
        IconSize1 = IconSize + 1,
        IconSize2 = IconSize + 2,
        IconSize3 = IconSize + 3,
        IconSize4 = IconSize + 4,
    };
    Q_ENUM(ContextMenuAction)

    // inherint prue virtual function of QAbstractItemView
    virtual QRect visualRect(const QModelIndex &index) const override;
    virtual QModelIndex indexAt(const QPoint &point) const override;
    virtual void scrollTo(const QModelIndex &index,
                          ScrollHint hint = EnsureVisible) override;
    virtual QModelIndex moveCursor(CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers) override;
    virtual int horizontalOffset() const override;
    virtual int verticalOffset() const override;
    virtual bool isIndexHidden(const QModelIndex &index) const override;
    virtual void setSelection(const QRect &rect,
                              QItemSelectionModel::SelectionFlags command) override;
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const override;

    // event override
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    //void resizeEvent(QResizeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool event(QEvent *event) override;

    virtual void rowsInserted(const QModelIndex &parent, int first, int last) override;
    virtual void keyboardSearch(const QString &search) override;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

#if QT_CONFIG(draganddrop)
    virtual void startDrag(Qt::DropActions supportedActions) override;
#endif
    QPixmap renderToPixmap(const QModelIndexList &indexes) const;
    QString canvansScreenName() const;
    QRect rectForIndex(const QModelIndex &index) const;
    DUrl currentUrl() const;
    bool setCurrentUrl(const DUrl &url);
    void initRootUrl();
    bool setRootUrl(const DUrl &url);
    const DUrlList autoMergeSelectedUrls() const;
    const DUrlList selectedUrls() const;
    void viewSelectedUrls(DUrlList &validSel, QModelIndexList &validIndexes);
    void setScreenNum(int num);
    void setScreenName(const QString &name);
    inline QString screenName() const {return m_screenName;}
    bool isSelected(const QModelIndex &index) const;
    void select(const QList<DUrl> &list);
    int selectedIndexCount() const;

    DFileSystemModel *model() const;
    DFileSelectionModel *selectionModel() const;
    DesktopItemDelegate *itemDelegate() const;
    void setItemDelegate(DesktopItemDelegate *delegate);

    double dodgeDuration() const;

    void openUrl(const DUrl &url);
    void openUrls(const QList<DUrl> &urls);

    QMargins cellMargins() const;
    QSize cellSize() const;

    WId winId() const;
    void setAutoMerge(bool enabled = false);
    void toggleEntryExpandedState(const DUrl &url);
    void updateEntryExpandedState(const DUrl &url);
    void setGeometry(const QRect &rect);
    void delayModelRefresh(int ms = 50);
    DUrl currentCursorFile() const;
    inline int screenNum() const {return m_screenNum;}
    void syncIconLevel(int level);
    void updateHiddenItems();
    void updateExpandItemGeometry();
    void updateCanvas();
signals:
    void sortRoleChanged(int role, Qt::SortOrder order);
    void autoAlignToggled();
    void autoMergeToggled();
    void changeIconLevel(int iconLevel);
    void dodgeDurationChanged(double dodgeDuration);

signals:
    void itemDeleted(const DUrl &url);
    void itemCreated(const DUrl &url);

public slots:
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;
    void setDodgeDuration(double dodgeDuration);
    virtual void selectAll() override;
protected slots:
    void onRefreshFinished();
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
// Debug interface
public Q_SLOTS:
    Q_SCRIPTABLE void EnableUIDebug(bool enable);
    Q_SCRIPTABLE QString Size();
    Q_SCRIPTABLE QString Dump();
    Q_SCRIPTABLE QString DumpPos(qint32 x, qint32 y);
    Q_SCRIPTABLE void Refresh(bool silent); // 刷新桌面图标
protected:
    void delayAutoMerge(int ms = 50);
    void delayArrage(int ms = 50);
    void delayCustom(int ms = 50);
private:
    Q_DISABLE_COPY(CanvasGridView)

    friend class DEventFilter;

    void initUI();
    void initConnection();

    void setIconByLevel(int level);
    void increaseIcon();
    void decreaseIcon();

    inline QPoint gridAt(const QPoint &pos) const;
    inline QRect gridRectAt(const QPoint &pos) const;
    inline QList<QRect> itemPaintGeomertys(const QModelIndex &index) const;
    inline QRect itemIconGeomerty(const QModelIndex &index) const;

    inline QModelIndex firstIndex();
    inline QModelIndex lastIndex();

    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command,
                      bool byIconRect);

    void handleContextMenuAction(int action);

    void showEmptyAreaMenu(const Qt::ItemFlags &indexFlags);
    void showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags);
    bool isIndexEmpty();
    QModelIndex moveCursorGrid(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

    void setGeometry(int, int, int, int) = delete;
    bool fetchDragEventUrlsFromSharedMemory();
    bool prohibitPaths();

    /**
     * @brief setTargetUrlToApp 为其他应用设置目标Url
     * 仅当被拖拽行为是DFileDrag时会被调用
     * @param data mimeData
     * @param url 目标Url
     */
    void setTargetUrlToApp(const QMimeData *data, const DUrl &url);

    QScopedPointer<CanvasViewPrivate> d;
    double m_dragMoveTime;
    QList<QUrl> m_urlsForDragEvent;

    QString m_screenName;
    int     m_screenNum{1};

    QTimer *m_refreshTimer = nullptr;
    QTime m_rt; //刷新计时，可以删除

    DUrl m_currentTargetUrl; //缓存当前目标Url
    int m_paintingLog = 3;

    QHash<QString,  ContextMenuAction> m_canvasOwnActions;
};


#endif // CANVASGRIDVIEW_H
