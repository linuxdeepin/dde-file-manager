#ifndef DLEFTSIDEBAR_H
#define DLEFTSIDEBAR_H

#include <QFrame>

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
    void initHomeBar();
    void initCommonFolderBar();
    void initDiskBar();
    void initNetWorkBar();
    void initConnect();
    QString getStandardPathbyId(int id);

signals:

public slots:
    void handleLocationChanged(int id);

private:
    QFrame* m_homeBar = NULL;
    DCheckableButton* m_homeButton = NULL;
    DCheckableButton* m_historyButton = NULL;

    QFrame* m_commonFolderBar = NULL;
    DCheckableButton* m_desktopButton = NULL;
    DCheckableButton* m_videoButton = NULL;
    DCheckableButton* m_musicButton = NULL;
    DCheckableButton* m_pictureButton = NULL;
    DCheckableButton* m_docmentButton = NULL;
    DCheckableButton* m_downloadButton = NULL;

    QFrame* m_diskBar = NULL;
    DCheckableButton* m_computerButton = NULL;
    DCheckableButton* m_trashButton = NULL;
    QFrame* m_networkBar = NULL;

    QButtonGroup* m_buttonGroup;

};

#endif // DLEFTSIDEBAR_H
