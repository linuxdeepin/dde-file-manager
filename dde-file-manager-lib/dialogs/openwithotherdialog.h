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

    void initDataAsync();
    void initUI();
    void initConnect();

signals:
    void requestMatchApps(const QString& keyword, const DesktopAppList& otherDesktopAppList);
    void requestAppendPageItems();

public slots:
    void updateAppList(DesktopAppList desktopAppList);
    void handleButtonClicked(int index, QString text);
    void onSearchTextChanged();
    void appendPageItems();
    void onScrollBarValueChanged(const int& value);
    void showEmptyPage();
    void hideEmptyPage();

private:
    DUrl m_url;
    AppMatchWorker* m_appMatchWorker;
    QListWidget* m_appListWidget = NULL;
    QButtonGroup* m_OpenWithButtonGroup = NULL;
    DSearchEdit* m_searchEdit;
    QTimer* m_searchTimer;
    QString m_searchKeyword;
    DesktopAppList m_otherAppList;
    DesktopAppQueue m_appQueue;
    QScrollBar* m_verticalScrollBar;
    QFrame* m_emptyPage;
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

#endif // OPENWITHOTHERDIALOG_H
