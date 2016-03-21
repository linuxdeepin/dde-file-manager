#ifndef DLEFTSIDEBAR_H
#define DLEFTSIDEBAR_H

#include <QFrame>
#include <QListWidget>
#include <QStackedWidget>
#include <QScrollBar>

class DCheckableButton;
class QButtonGroup;

class DLeftSideBar : public QFrame
{
    Q_OBJECT
public:
    explicit DLeftSideBar(QWidget *parent = 0);
    ~DLeftSideBar();
    void initData();
    void initUI();
//    void initHomeBar();
//    void initCommonFolderBar();
//    void initDiskBar();
//    void initNetWorkBar();
    void initConnect();
    void initTightNav();
    void initNav();
    QString getStandardPathbyId(int id);

signals:

public slots:
    void handleLocationChanged(int id);
    void toTightNav();
    void toNormalNav();
private:
    QFrame* m_homeBar = NULL;
    DCheckableButton* m_fileButton = NULL;
    DCheckableButton* m_tightNavFileButton = NULL;
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
    QListWidget * m_listWidget = NULL;
    QStringList m_iconlist;
    QStringList m_nameList;
    QFrame * m_tightNav = NULL;
    QFrame * m_nav = NULL;
    QStackedWidget * m_stackedWidget = NULL;
    bool m_navState = false;
};

#endif // DLEFTSIDEBAR_H
