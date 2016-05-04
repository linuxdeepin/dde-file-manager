#ifndef DLEFTSIDEBAR_H
#define DLEFTSIDEBAR_H

#include <QFrame>
#include <QListWidget>
#include <QStackedWidget>
#include <QScrollBar>
#include <QPushButton>
#include <QGraphicsView>

#include "durl.h"

#define LEFTSIDEBAR_MIN 60
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
class FMEvent;


class DLeftSideBar : public QFrame
{
    Q_OBJECT
public:
    explicit DLeftSideBar(QWidget *parent = 0);
    ~DLeftSideBar();
    void initData();
    void initUI();
    void initConnect();
    void initTightNav();
    void initNav();
    DUrl getStandardPathByKey(QString key);
protected:
    void resizeEvent(QResizeEvent *e);

signals:
    void moveSplitter(int pos, int index);
public slots:
    void handleLocationChanged(const FMEvent &e);
    void toTightNav();
    void toNormalNav();
    void doDragEnter();
    void doDragLeave();
private:
    void loadBookmark();
    void loadDevices();
    bool m_isTight = false;
    QFrame* m_homeBar = NULL;
    QPushButton* m_fileButton = NULL;
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

    QButtonGroup* m_buttonGroup;
    QButtonGroup* m_tightNavButtonGroup;
    BMListWidget * m_listWidget = NULL;
    BMListWidget * m_listWidgetTight = NULL;
    QMap<QString, QString> m_icons;
    QMap<QString, QString> m_checkedIcons;
    QMap<QString, QString> m_bigIcons;
    QMap<QString, QString> m_checkedBigIcons;
    QStringList m_nameList;
    QStringList m_systemPathKeys;
    QFrame * m_tightNav = NULL;
    QFrame * m_nav = NULL;
    QStackedWidget * m_stackedWidget = NULL;
    bool m_navState = false;

    DBookmarkScene * m_scene;
    DBookmarkScene * m_tightScene;
    DBookmarkItemGroup * m_itemGroup;
    DBookmarkItemGroup * m_itemGroupTight;

    QMap<QString, QString> m_systemBookMarks;
};

#endif // DLEFTSIDEBAR_H
