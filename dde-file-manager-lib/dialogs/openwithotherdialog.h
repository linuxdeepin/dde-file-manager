#ifndef OPENWITHOTHERDIALOG_H
#define OPENWITHOTHERDIALOG_H

#include <ddialog.h>
#include <QListWidget>
#include <QQueue>
#include "durl.h"
#include "dabstractfileinfo.h"
#include "dsearchedit.h"
#include "../shutil/mimesappsmanager.h"

class QScrollBar;
class DSearchLineEdit;

typedef QList<DesktopFile> DesktopAppList;
Q_DECLARE_METATYPE(DesktopAppList)

typedef QQueue<DesktopFile> DesktopAppQueue;

class AppMatchWorker;

DWIDGET_USE_NAMESPACE

class OpenWithOtherDialog : public DDialog
{
    Q_OBJECT
public:
    explicit OpenWithOtherDialog(const DUrl& url, QWidget *parent = 0);
    ~OpenWithOtherDialog();

    void initData();
    void initDataAsync();
    void initUI();
    void initConnect();

signals:
    void requestMatchApps(const QString& keyword, const DesktopAppList& otherDesktopAppList);
    void requestAppendPageItems();

public slots:
    void updateAppList(DesktopAppList desktopAppList);
    void onSearchTextChanged();
    void appendPageItems();
    void onScrollBarValueChanged(const int& value);
    void showEmptyPage();
    void hideEmptyPage();
    void selectUp();
    void selectDown();
    void selectByItem(QListWidgetItem* item);
    bool confirmSelection();
    void onItemEntered(QListWidgetItem* item);

    //overrided function for done action
    void done(int r) Q_DECL_OVERRIDE;

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    DUrl m_url;
    AppMatchWorker* m_appMatchWorker = NULL;
    QListWidget* m_appListWidget = NULL;
    QButtonGroup* m_OpenWithButtonGroup = NULL;
    DSearchLineEdit* m_searchEdit = NULL;
    QTimer* m_searchTimer = NULL;
    QScrollBar* m_verticalScrollBar = NULL;
    QWidget* m_emptyPage = NULL;
    QWidget* m_mainFrame = NULL;
    QString m_searchKeyword;
    DesktopAppList m_otherAppList;
    DesktopAppQueue m_appQueue;
    bool m_isPageEmpty = false;
    void searchApps();
};


class AppMatchWorker: public QObject{
    Q_OBJECT
public:
    explicit AppMatchWorker(QObject* parent = 0);
    ~AppMatchWorker();
    enum WorkerState{
        Ready,
        Running,
        Finished
    };

    WorkerState workerState() const;
    void setWorkerState(const WorkerState &workerState);

signals:
    void finishedMatchTask(const DesktopAppList& desktopAppList);

public slots:
    void matchApps(QString keyword, DesktopAppList otherDesktopAppList);

private:
    bool matchByFullName(const QString& pattern, const QString& source);
    bool matchByCharactorQueue(const QString& pattern, const QString& source);
    WorkerState m_workerState;
};

class DSearchLineEdit: public DSearchEdit{
    Q_OBJECT
public:
    explicit DSearchLineEdit(QWidget* parent = 0);

signals:
    void keyUpPressed();
    void keyDownPressed();

protected:
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

};

#endif // OPENWITHOTHERDIALOG_H
