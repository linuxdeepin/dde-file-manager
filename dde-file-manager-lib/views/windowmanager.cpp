#include "windowmanager.h"
#include "dfilemanagerwindow.h"
#include "dthumbnailprovider.h"
#include "dabstractfilewatcher.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dfmeventdispatcher.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfmevent.h"

#include "models/fmstate.h"

#include "fileoperations/filejob.h"
#include "controllers/fmstatemanager.h"

#include "dialogs/dialogmanager.h"

#include "qobjecthelper.h"
#include "dfmsetting.h"

#include "widgets/singleton.h"

#include "shutil/fileutils.h"

#include <QThread>
#include <QDebug>
#include <QApplication>
#include <QX11Info>
#include <QScreen>
#include <QWindow>
#include <QProcess>

DFM_USE_NAMESPACE

enum NetWmState {
    NetWmStateAbove = 0x1,
    NetWmStateBelow = 0x2,
    NetWmStateFullScreen = 0x4,
    NetWmStateMaximizedHorz = 0x8,
    NetWmStateMaximizedVert = 0x10,
    NetWmStateModal = 0x20,
    NetWmStateStaysOnTop = 0x40,
    NetWmStateDemandsAttention = 0x80
};

Q_DECLARE_FLAGS(NetWmStates, NetWmState)

QHash<const QWidget*, quint64> WindowManager::m_windows;
int WindowManager::m_count = 0;

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{
    m_fmStateManager = new FMStateManager(this);
    m_fmStateManager->loadCache();
    qApp->setApplicationDisplayName(tr("Deepin File Manager"));
    initConnect();
}

WindowManager::~WindowManager()
{

}

void WindowManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestOpenNewWindowByUrl, this, &WindowManager::showNewWindow);
    connect(fileSignalManager, &FileSignalManager::aboutToCloseLastActivedWindow, this, &WindowManager::onLastActivedWindowClosed);
    connect(fileSignalManager, &FileSignalManager::requestQuitApplication, this, &WindowManager::quit);

    connect(DThumbnailProvider::instance(), &DThumbnailProvider::createThumbnailFinished,
                     this, [] (const QString &filePath) {
        const DUrl &fileUrl = DUrl::fromLocalFile(filePath);

        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(fileUrl);

        if (!fileInfo)
            return;

        DAbstractFileWatcher::ghostSignal(fileInfo->parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, fileUrl);
    });
}

void WindowManager::loadWindowState(DFileManagerWindow *window)
{
    FMState* state = m_fmStateManager->fmState();

    int width = state->width();
    int height = state->height();
    int windowState = state->windowState();
    window->resize(width, height);
    window->setWindowState(static_cast<Qt::WindowState>(windowState));
}


void WindowManager::saveWindowState(DFileManagerWindow *window)
{
    m_fmStateManager->fmState()->setViewMode(window->getFileViewMode());

    /// The power by dxcb platform plugin
    NetWmStates states = (NetWmStates)window->window()->windowHandle()->property("_d_netWmStates").toInt();

    if ((states & (NetWmStateMaximizedHorz | NetWmStateMaximizedVert)) == 0) {
        m_fmStateManager->fmState()->setX(window->x());
        m_fmStateManager->fmState()->setY(window->y());
        m_fmStateManager->fmState()->setWidth(window->size().width());
        m_fmStateManager->fmState()->setHeight(window->size().height());
    }
    m_fmStateManager->fmState()->setWindowState(window->windowState());
    m_fmStateManager->saveCache();
}

DUrl WindowManager::getUrlByWindowId(quint64 windowId)
{
    if (getWindowById(windowId)){
         DFileManagerWindow* window = qobject_cast<DFileManagerWindow*>(getWindowById(windowId));
         return window->currentUrl();
    }
    return DUrl::fromLocalFile(QDir::homePath());
}

bool WindowManager::tabAddableByWinId(const quint64 &winId)
{
    DFileManagerWindow* window = qobject_cast<DFileManagerWindow*>(getWindowById(winId));
    if(window)
        return window->tabAddable();
    return false;

}

void WindowManager::showNewWindow(const DUrl &url, const bool& isNewWindow)
{
    if (!isNewWindow){
        for(int i=0; i< m_windows.count(); i++){
            QWidget* window = const_cast<QWidget *>(m_windows.keys().at(i));
            DUrl currentUrl = static_cast<DFileManagerWindow *>(window)->currentUrl();
            if (currentUrl == url){
                qDebug() << currentUrl << static_cast<DFileManagerWindow *>(window);
                qApp->setActiveWindow(static_cast<DFileManagerWindow *>(window));
                return;
            }
        }
    }

    QX11Info::setAppTime(QX11Info::appUserTime());
    DFileManagerWindow *window = new DFileManagerWindow(url.isEmpty() ? DUrl::fromUserInput(globalSetting->defaultWindowPath()) : url);
    loadWindowState(window);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    qDebug() << "new window" << window->winId() << url;

    connect(window, &DFileManagerWindow::aboutToClose,
            this, &WindowManager::onWindowClosed);

    m_windows.insert(window, window->winId());

    if (m_windows.count() == 1){
        QPoint pos = QCursor::pos();
        QRect currentScreenGeometry;

        for (QScreen *screen : qApp->screens()) {
            if (screen->geometry().contains(pos)) {
                currentScreenGeometry = screen->geometry();
            }
        }

        if (currentScreenGeometry.isEmpty()) {
            currentScreenGeometry = qApp->primaryScreen()->geometry();
        }

        window->moveCenter(currentScreenGeometry.center());
    }
//    window->setFileViewMode(m_fmStateManager->fmState()->viewMode());

    qApp->setActiveWindow(window);
}



quint64 WindowManager::getWindowId(const QWidget *window)
{
    int winId = m_windows.value(window->topLevelWidget(), 0);

    if (winId != 0)
        return winId;

    while (window) {
        if (window->inherits("DFileManagerWindow")) {
            return window->winId();
        }

        window = window->parentWidget();
    }

    return 0;
}

QWidget *WindowManager::getWindowById(quint64 winId)
{
    const QWidget *widget = m_windows.key(winId);

    if (widget)
        return const_cast<QWidget*>(widget);

    for (QWidget *widget : qApp->topLevelWidgets()) {
        if (widget->winId() == winId)
            return widget;
    }

    return Q_NULLPTR;
}

void WindowManager::onWindowClosed()
{
    if (m_windows.count() == 1){
        DFileManagerWindow* window = static_cast<DFileManagerWindow*>(sender());
        saveWindowState(window);
        dialogManager->closeAllPropertyDialog();
    }
    m_windows.remove(static_cast<const QWidget*>(sender()));
}

void WindowManager::onLastActivedWindowClosed(quint64 winId)
{
    QList<quint64> winIds = m_windows.values();
    foreach (quint64 id, winIds) {
        if (id != winId){
            getWindowById(id)->close();
        }
    }
    getWindowById(winId)->close();
    qApp->quit();
}

void WindowManager::quit()
{
    if (m_windows.count() == 0){
        if (dialogManager->isTaskDialogEmpty()){
            qApp->quit();
        }
    }
}

static bool isAvfsMounted()
{
    QProcess p;
    QString cmd = "/bin/bash";
    QStringList args;
    args << "-c" << "ps -ax -o 'cmd'|grep '.avfs$'";
    p.start(cmd, args);
    p.waitForFinished();
    QString avfsBase = qgetenv("AVFSBASE");
    QString avfsdDir;
    if(avfsBase.isEmpty()){
        QString home = qgetenv("HOME");
        avfsdDir = home + "/.avfs";
    } else{
        avfsdDir = avfsBase + "/.avfs";
    }

    while (!p.atEnd()) {
        QString result = p.readLine().trimmed();
        if(!result.isEmpty()){
            QStringList datas = result.split(" ");

            if(datas.count() == 2){
                //compare current user's avfs path
                if(datas.last() != avfsdDir)
                    continue;

                if(datas.first() == "avfsd" && QFile::exists(datas.last()))
                    return true;
            }
        }
    }

    return false;
}

bool WindowManager::fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData)
{
    switch (event->type()) {
    case DFMEvent::OpenNewWindow: {
        const QSharedPointer<DFMOpenNewWindowEvent> &e = event.dynamicCast<DFMOpenNewWindowEvent>();

        for (const DUrl &url : e->urlList()) {
            showNewWindow(url, e->force());
        }

        break;
    }
    case DFMEvent::ChangeCurrentUrl: {
        const QSharedPointer<DFMChangeCurrentUrlEvent> &e = event.dynamicCast<DFMChangeCurrentUrlEvent>();

        if (DFileManagerWindow *window = const_cast<DFileManagerWindow*>(qobject_cast<const DFileManagerWindow*>(e->window()))) {
            window->preHandleCd(e->fileUrl(), e->sender());
        }

        break;
    }
    case DFMEvent::OpenUrl: {
        const QSharedPointer<DFMOpenUrlEvent> &e = event.dynamicCast<DFMOpenUrlEvent>();

        //sort urls by files and dirs
        DUrlList dirList;

        foreach (DUrl url, e->urlList()) {
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(url);

            if (globalSetting->isCompressFilePreview()
                    && isAvfsMounted()
                    && FileUtils::isArchive(url.toLocalFile())
                    && fileInfo->mimeType().name() != "application/vnd.debian.binary-package") {
                const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(DUrl::fromAVFSFile(url.path()));

                if (info->exists()) {
                    url.setScheme(AVFS_SCHEME);
                    dirList << url;
                    continue;
                }
            }

            if (fileInfo) {
                if (fileInfo->isDir())
                    dirList << url;
                else
                    DFileService::instance()->openFile(url, event->sender());
            }

            //computer url is virtual dir
            if (url == DUrl::fromComputerFile("/"))
                dirList << url;
        }

        if (dirList.isEmpty())
            break;

        QVariant result;

        if (e->dirOpenMode() == DFMOpenUrlEvent::OpenInCurrentWindow) {
            result = DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(dirList.first(), getWindowById(event->eventId()), event->sender());
        } else {
            result = DFMEventDispatcher::instance()->processEvent<DFMOpenNewWindowEvent>(dirList, e->dirOpenMode() == DFMOpenUrlEvent::ForceOpenNewWindow, event->sender());
        }

        if (resultData)
            *resultData = result;

        break;
    }
    default:
        return false;
    }

    return true;
}
