#ifndef DLEFTSIDEBAR_H
#define DLEFTSIDEBAR_H

#include <QFrame>
#include <QListWidget>
#include <QStackedWidget>
#include <QScrollBar>
#include <QPushButton>
#include <QGraphicsView>
#include <QLabel>

#include "durl.h"

#define LEFTSIDEBAR_MIN 48
#define LEFTSIDEBAR_MAX 200
#define LEFTSIDEBAR_NORMAL 160
#define LEFTSIDEBAR_THRES 70

class BookMark;
class DCheckableButton;
class QButtonGroup;
class BMListWidget;
class DBookmarkItem;
class DBookmarkScene;
class DBookmarkItemGroup;
class DFMEvent;
class DFileView;
class DToolBar;


class DLeftSideBar : public QFrame
{
    Q_OBJECT
public:
    explicit DLeftSideBar(QWidget *parent = 0);
    ~DLeftSideBar();
    void initData();
    void initUI();
    void initConnect();
    void initNav();
    QGraphicsView *view() const;
    DBookmarkScene* scene();

    DToolBar *toolbar() const;
    void setToolbar(DToolBar *toolbar);

    void setDisableUrlSchemes(const QList<QString> &schemes);

protected:
    void resizeEvent(QResizeEvent *e);

signals:
    void moveSplitter(int pos, int index);
public slots:
    void handleLocationChanged(const DFMEvent &e);
    void navSwitched();
    void toTightNav();
    void toNormalNav();
    void doDragEnter();
    void doDragLeave();
    void handdleRequestDiskInfosFinihsed();
    void handleUserShareCountChanged(const int &count);
    void centerOnMyShareItem(const QString& path);
    QPoint getMyShareItemCenterPos();
    void playtShareAddedAnimation();
    void updateVerticalScrollBar();

protected:
    void paintEvent(QPaintEvent *event);

private:
    void loadBookmark();
    void addNetworkBookmarkItem();
    void addUserShareBookmarkItem();
    void loadDevices();
    void loadPluginBookmarks();
    bool m_isTight = false;
    bool m_entered = false;
    QFrame* m_homeBar = NULL;
    QPushButton* m_tightNavFileButton = NULL;
    DCheckableButton* m_homeButton = NULL;
    DCheckableButton* m_recentButton = NULL;

    QFrame* m_commonFolderBar = NULL;
    DCheckableButton* m_desktopButton = NULL;
    DCheckableButton* m_videoButton = NULL;
    DCheckableButton* m_musicButton = NULL;
    DCheckableButton* m_pictureButton = NULL;
    DCheckableButton* m_docmentButton = NULL;
    DCheckableButton* m_downloadButton = NULL;
    DCheckableButton* m_trashButton = NULL;

    QFrame* m_diskBar = NULL;
    DCheckableButton* m_computerButton = NULL;
    DCheckableButton * m_favoriteButton = NULL;
    DCheckableButton * m_myMobileButton = NULL;
    QFrame* m_networkBar = NULL;
    QGraphicsView * m_view = NULL;

    QButtonGroup* m_buttonGroup;
    QButtonGroup* m_tightNavButtonGroup;
    BMListWidget * m_listWidget = NULL;
    BMListWidget * m_listWidgetTight = NULL;

    QStringList m_nameList;

    QFrame * m_tightNav = NULL;
    QFrame * m_nav = NULL;
    QStackedWidget * m_stackedWidget = NULL;
    bool m_navState = false;

    DBookmarkScene * m_scene;
    DBookmarkItemGroup * m_itemGroup;
    DToolBar *m_toolbar = NULL;
    QScrollBar* m_verticalScrollBar = NULL;
};

#endif // DLEFTSIDEBAR_H
