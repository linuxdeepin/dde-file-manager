// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Include gio before any Qt header: gio headers contain struct members named
// "signals"/"slots", which must not be seen through Qt's keyword macros.
#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>

#include "folderpresentation.h"
#include "networkguard.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDrag>
#include <QFileInfo>
#include <QLocale>
#include <QMimeData>
#include <QProcess>
#include <QTranslator>
#include <QUrl>
#include <QQuickItem>
#include <QQuickWindow>
#include <qqml.h>

#include <DUtil>

namespace dfm {

namespace {
// The shell installs translators for applet plugins only; folder content
// providers are loaded through QPluginLoader without that hook. Install the
// catalog compiled into this plugin before the first QML view is created so
// qsTr() inside FolderView.qml resolves.
void installCatalog()
{
    static QTranslator *translator = nullptr;
    if (translator || !QCoreApplication::instance())
        return;
    translator = new QTranslator(QCoreApplication::instance());
    if (translator->load(QLocale::system(), QStringLiteral("dfm-folder-content"),
                         QStringLiteral("_"), QStringLiteral(":/dfm-folder-content/translations"))) {
        QCoreApplication::installTranslator(translator);
    } else {
        translator->deleteLater();
        translator = nullptr;
    }
}

// Launch a desktop entry through AM1, the same call the file manager's
// AppLaunchUtils makes (org.desktopspec.ApplicationManager1.Launch). Kept
// local because the standalone test build cannot link dfm-base.
bool launchDesktopByAm1(const QString &desktopPath)
{
    const QString appId = DUtil::getAppIdFromAbsolutePath(desktopPath);
    if (appId.isEmpty())
        return false;
    QDBusInterface appIface(QStringLiteral("org.desktopspec.ApplicationManager1"),
                            QStringLiteral("/org/desktopspec/ApplicationManager1/")
                                    + DUtil::escapeToObjectPath(appId),
                            QStringLiteral("org.desktopspec.ApplicationManager1.Application"),
                            QDBusConnection::sessionBus());
    if (!appIface.isValid())
        return false;
    // A stuck ApplicationManager1 must not freeze the dock's GUI thread for
    // the 25 s default DBus timeout.
    appIface.setTimeout(5000);
    const QVariantMap options { { QStringLiteral("_launch_type"), QStringLiteral("dde-file-manager") } };
    const QDBusMessage reply = appIface.callWithArgumentList(QDBus::Block, QStringLiteral("Launch"),
                                                            { QString(), QStringList(), options });
    if (reply.type() == QDBusMessage::ErrorMessage)
        qWarning() << "AM1 Launch failed for" << desktopPath << ":" << reply.errorMessage();
    return reply.type() != QDBusMessage::ErrorMessage;
}

// GIO fallback, mirroring the file manager's second launch strategy. It also
// covers desktop files unknown to AM1, e.g. custom shortcuts inside the pin.
bool launchDesktopByGio(const QString &desktopPath)
{
    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopPath.toLocal8Bit().constData());
    if (!appInfo)
        return false;
    const gboolean ok = g_app_info_launch(reinterpret_cast<GAppInfo *>(appInfo), nullptr, nullptr, nullptr);
    g_object_unref(appInfo);
    return ok;
}
}

FolderPresentation::FolderPresentation(const QString &url, QObject *parent)
    : dock::FolderPresentation(parent), m_pinnedPath(QUrl(url).toLocalFile())
{
    connect(&m_preview, &dock::DirectoryModel::countChanged, this, &FolderPresentation::presentationChanged);
    connect(&m_preview, &QAbstractItemModel::dataChanged, this, &FolderPresentation::presentationChanged);
    connect(&m_browser, &dock::DirectoryModel::pathChanged, this, &FolderPresentation::navigationChanged);
    connect(&m_browser, &dock::DirectoryModel::navigationChanged, this, &FolderPresentation::navigationChanged);
    // A renamed pinned directory keeps the same inode; follow it so the dock
    // entry does not degrade into a dead path.
    connect(&m_preview, &dock::DirectoryModel::pathMoved, this, &FolderPresentation::adoptPath);
    // A deleted pinned directory cannot ever show contents again; let the dock
    // drop the pin instead of keeping an empty folder around.
    connect(&m_preview, &dock::DirectoryModel::pathLost, this, &FolderPresentation::contentLost);
    m_preview.setPath(m_pinnedPath);
    m_browser.setPath(m_pinnedPath);
    // setPath already flags an unreachable network mount; skip the history
    // push too, since stat() below would hang on the dead mount.
    if (!dock::NetworkGuard::instance().isBusy(m_pinnedPath) && QFileInfo(m_pinnedPath).isDir())
        m_browser.navigateTo(m_pinnedPath);
}

void FolderPresentation::adoptPath(const QString &oldPath, const QString &newPath)
{
    Q_UNUSED(oldPath)
    if (newPath.isEmpty() || newPath == m_pinnedPath) return;
    m_pinnedPath = newPath;
    // Move the browser without pushing history: the folder itself did not
    // change, only its name, so back/forward entries stay meaningful.
    if (m_browser.path() != newPath) m_browser.setPath(newPath);
    Q_EMIT navigationChanged();
    Q_EMIT presentationChanged();
    Q_EMIT identityChanged(QUrl::fromLocalFile(newPath).toString(QUrl::FullyEncoded));
}

QString FolderPresentation::title() const
{
    const auto name = QFileInfo(m_pinnedPath).fileName();
    return name.isEmpty() ? m_pinnedPath : name;
}

QVariantList FolderPresentation::previewIcons() const
{
    QVariantList icons;
    for (int row = 0; row < qMin(4, m_preview.rowCount()); ++row)
        icons.append(QVariantMap{{QStringLiteral("iconName"), m_preview.get(row).value("iconName")}});
    return icons;
}

QUrl FolderPresentation::contentUrl() const { return QUrl(QStringLiteral("qrc:/dfm-folder-content/FolderView.qml")); }
QUrl FolderPresentation::iconUrl() const { return QUrl(QStringLiteral("qrc:/dfm-folder-content/FolderIcon.qml")); }
void FolderPresentation::navigateTo(const QString &path) { m_browser.navigateTo(path); }
void FolderPresentation::goBack() { m_browser.goBack(); }
void FolderPresentation::goForward() { m_browser.goForward(); }
void FolderPresentation::refreshDirectory() { m_browser.refresh(); }
// True when @p path is an entry actually shown by this view.
bool FolderPresentation::containsPath(const QString &path) const
{
    for (int row = 0; row < m_browser.rowCount(); ++row) {
        if (m_browser.get(row).value("filePath").toString() == path)
            return true;
    }
    return false;
}

void FolderPresentation::openFile(const QString &path)
{
    // An unreachable mount would hang the exists() probe below; the popup
    // already shows an "unreachable" hint, so just ignore the click.
    if (dock::NetworkGuard::instance().isBusy(path))
        return;
    // Only operate on an entry actually shown by this view.
    if (!containsPath(path))
        return;
    if (!QFileInfo::exists(path)) return;
    // Same handling as the file manager (LocalFileHandler::doOpenFiles):
    // a desktop entry starts the application it points to instead of
    // opening the file itself; non-desktop files keep QDesktopServices.
    if (QFileInfo(path).suffix() == QLatin1String("desktop")) {
        if (launchDesktopByAm1(path) || launchDesktopByGio(path))
            return;
        qWarning() << "Failed to launch desktop entry:" << path;
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void FolderPresentation::startFileDrag(const QString &path, QQuickItem *item)
{
    if (path.isEmpty() || dock::NetworkGuard::instance().isBusy(path) || !QFileInfo::exists(path))
        return;
    // Only operate on an entry actually shown by this view, like openFile().
    if (!containsPath(path))
        return;
    // A real system drag (QDrag) is the only reliable way to hand a file to
    // external targets such as the desktop or another file manager window:
    // the uri-list is offered through the Wayland/X11 drag-and-drop protocol
    // and the drop site performs the copy.
    QDrag *drag = new QDrag(item);
    QMimeData *mime = new QMimeData;
    mime->setUrls({ QUrl::fromLocalFile(path) });
    drag->setMimeData(mime);
    if (item && item->window()) {
        // Show the tile itself under the cursor while dragging: grab the
        // window (device pixels) and crop the item's scene rectangle.
        QQuickWindow *window = item->window();
        const qreal dpr = window->devicePixelRatio();
        const QImage full = window->grabWindow();
        QRectF scene(item->mapToScene(QPointF(0, 0)), QSizeF(item->width(), item->height()));
        QRect device(qRound(scene.left() * dpr), qRound(scene.top() * dpr),
                     qRound(scene.width() * dpr), qRound(scene.height() * dpr));
        device = device.intersected(full.rect());
        if (!device.isEmpty()) {
            QPixmap pixmap = QPixmap::fromImage(full.copy(device));
            pixmap.setDevicePixelRatio(dpr);
            drag->setPixmap(pixmap);
            drag->setHotSpot(QPoint(qRound(pixmap.width() / dpr / 2),
                                    qRound(pixmap.height() / dpr / 2)));
        }
    }
    drag->exec(Qt::CopyAction);
    drag->deleteLater();
}

void FolderPresentation::openInFileManager()
{
    // Reveal the currently browsed directory, not the pinned one, so a page
    // that navigated into a subdirectory opens where the user is looking.
    const QString path = m_browser.path();
    if (path.isEmpty())
        return;
    // While the mount is unreachable the stat() below would hang; hand the
    // path to the file manager anyway and let it show its own "cannot access"
    // dialog, so the user still gets feedback.
    if (!dock::NetworkGuard::instance().isBusy(path) && !QFileInfo(path).isDir())
        return;
    // Same invocation the FileManager1 daemon uses for ShowFolders.
    const QStringList args { QStringLiteral("--raw"),
                             QUrl::fromLocalFile(path).toString() };
    if (!QProcess::startDetached(QStringLiteral("file-manager.sh"), args))
        QProcess::startDetached(QStringLiteral("dde-file-manager"), args);
}

QString FolderProvider::normalizeId(const QString &id) const
{
    const QString local = QUrl(id).toLocalFile();
    // Refuse to pin a directory on an unreachable network mount: every later
    // presentation action would have to be guarded against the dead path.
    if (dock::NetworkGuard::instance().isBusy(local))
        return {};
    const QFileInfo info(local);
    if (!info.isDir() || !info.isReadable()) return {};
    return QUrl::fromLocalFile(info.canonicalFilePath()).toString(QUrl::FullyEncoded);
}

dock::FolderPresentation *FolderProvider::create(const QString &id, QObject *parent)
{
    static const int type = qmlRegisterAnonymousType<dock::DirectoryModel>("org.deepin.dfm.folder", 1);
    Q_UNUSED(type)
    return new FolderPresentation(id, parent);
}

FolderProvider::FolderProvider()
{
    installCatalog();
}

}
