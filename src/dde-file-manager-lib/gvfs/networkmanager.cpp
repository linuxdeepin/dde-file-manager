/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "networkmanager.h"
#include "dfmeventdispatcher.h"
#include "dfileservices.h"

#include "app/filesignalmanager.h"
#include "app/define.h"

#include "singleton.h"
#include "deviceinfo/udisklistener.h"

#include "views/windowmanager.h"

#include "gvfsmountmanager.h"

#include <QProcess>
#include <QRegularExpression>
#include <QTimer>

DFM_USE_NAMESPACE

enum EventLoopCode {
    FetchFinished = 0,
    MountFinished = 1,
    FetchFailed = -1
};

NetworkNode::NetworkNode()
{

}

NetworkNode::~NetworkNode()
{

}

QString NetworkNode::url() const
{
    return m_url;
}

void NetworkNode::setUrl(const QString &url)
{
    m_url = url;
}

QString NetworkNode::displayName() const
{
    return m_displayName;
}

void NetworkNode::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}
QString NetworkNode::iconType() const
{
    return m_iconType;
}

void NetworkNode::setIconType(const QString &iconType)
{
    m_iconType = iconType;
}

QDebug operator<<(QDebug dbg, const NetworkNode &node)
{
    dbg.nospace() << "NetworkNode{"
                  << "url: " << node.url() << ", "
                  << "displayName: " << node.displayName() << ", "
                  << "iconType: " << node.iconType() << ", "
                  << "}";
    return dbg;
}


QStringList NetworkManager::SupportScheme = {
    "network",
    "smb",
    "ftp",
    "sftp"
};
QMap<DUrl, NetworkNodeList> NetworkManager::NetworkNodes = {};
GCancellable *NetworkManager::m_networks_fetching_cancellable = NULL;
QPointer<QEventLoop> NetworkManager::eventLoop;

NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{
    qDebug() << "Create NetworkManager";
    qRegisterMetaType<NetworkNodeList>(QT_STRINGIFY(NetworkNodeList));
    initData();
    initConnect();
}

NetworkManager::~NetworkManager()
{
    initData();
    initConnect();
}

void NetworkManager::initData()
{

}

void NetworkManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestFetchNetworks, this, &NetworkManager::fetchNetworks);
}

bool NetworkManager::fetch_networks(gchar *url, DFMEvent *e)
{
    QPointer<QEventLoop> oldEventLoop = eventLoop;
    QEventLoop event_loop;

    eventLoop = &event_loop;

    GFile *network_file;
    network_file = g_file_new_for_uri(url);

    if (m_networks_fetching_cancellable) {
        g_cancellable_cancel(m_networks_fetching_cancellable);
        g_clear_object(&m_networks_fetching_cancellable);
    }
    m_networks_fetching_cancellable = g_cancellable_new();

    int ret = 0;

    do {
        g_file_enumerate_children_async(network_file,
                                        "standard::type,standard::target-uri,standard::name,standard::display-name,standard::icon,mountable::can-mount",
                                        G_FILE_QUERY_INFO_NONE,
                                        G_PRIORITY_DEFAULT,
                                        m_networks_fetching_cancellable,
                                        network_enumeration_finished,
                                        e);
        ret = eventLoop->exec();
    } while (ret == EventLoopCode::MountFinished); // 需要重新执行 g_file_enumerate_children_async

    g_clear_object(&network_file);

    if (oldEventLoop) {
        oldEventLoop->exit(ret);
    }

    return ret == EventLoopCode::FetchFinished;
}

void NetworkManager::network_enumeration_finished(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GFileEnumerator *enumerator = nullptr;
    GError *error = nullptr;

    enumerator = g_file_enumerate_children_finish(G_FILE(source_object), res, &error);

    qDebug() << "network_enumeration_finished";

    if (error) {
        DFMUrlBaseEvent *event = static_cast<DFMUrlBaseEvent *>(user_data);
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
                !g_error_matches(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED)) {
            qWarning("Failed to fetch network locations: %s", error->message);
            if (event->fileUrl() == DUrl::fromNetworkFile("/")) {
                NetworkManager::restartGVFSD();
            }
        }
        qDebug() << error->message;
        MountStatus status = gvfsMountManager->mount_sync(*event);
        g_clear_error(&error);

        if (eventLoop) {
            // 挂载完成时, 返回 1, 在fetch_networks中再次调用g_file_enumerate_children_async获取列表
            if (status == MOUNT_SUCCESS || status == MOUNT_PASSWORD_WRONG) {
                eventLoop->exit(EventLoopCode::MountFinished);
            } else {
                eventLoop->exit(EventLoopCode::FetchFailed);
            }
        }
    } else {
        if (!enumerator) {
            if (eventLoop) {
                eventLoop->exit(EventLoopCode::FetchFailed);
            }

            return;
        }

        g_file_enumerator_next_files_async(enumerator,
                                           G_MAXINT32,
                                           G_PRIORITY_DEFAULT,
                                           m_networks_fetching_cancellable,
                                           network_enumeration_next_files_finished,
                                           user_data);
    }
}

void NetworkManager::network_enumeration_next_files_finished(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GList *detected_networks;
    GError *error;

    error = NULL;

    detected_networks = g_file_enumerator_next_files_finish(G_FILE_ENUMERATOR(source_object),
                                                            res, &error);

    if (error) {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            qWarning("Failed to fetch network locations: %s", error->message);
            DFMEvent *event = static_cast<DFMEvent *>(user_data);
            if (event->fileUrl() == DUrl::fromNetworkFile("/")) {
                NetworkManager::restartGVFSD();
            }
        }
        g_clear_error(&error);
    } else {
        populate_networks(G_FILE_ENUMERATOR(source_object), detected_networks, user_data);

        g_list_free_full(detected_networks, g_object_unref);
    }

    if (eventLoop) {
        eventLoop->exit(error ? EventLoopCode::FetchFailed : EventLoopCode::FetchFinished);
    }
}

void NetworkManager::populate_networks(GFileEnumerator *enumerator, GList *detected_networks, gpointer user_data)
{
    GList *l;
    GFile *file;
    GFile *activatable_file;
    gchar *uri;
    GFileType type;
//    gchar *name;

    NetworkNodeList nodeList;

    for (l = detected_networks; l != NULL; l = l->next) {
        GFileInfo *fileInfo = static_cast<GFileInfo *>(l->data);
        file = g_file_enumerator_get_child(enumerator, fileInfo);
        type = g_file_info_get_file_type(fileInfo);
        if (type == G_FILE_TYPE_SHORTCUT || type == G_FILE_TYPE_MOUNTABLE)
            uri = g_file_info_get_attribute_as_string(fileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
        else
            uri = g_file_get_uri(file);

        activatable_file = g_file_new_for_uri(uri);
//        name = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_STANDARD_NAME);
        gchar *display_name = g_file_info_get_attribute_as_string(fileInfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
        GIcon *icon = g_file_info_get_icon(fileInfo);
        gchar *iconPath = g_icon_to_string(icon);

        // URL Cleanup:
        // blumia: sometimes it will happend in weird Mac mini device with a url format like: `smb://[10.0.61.210]:445/`
        //         Wikipedia said (haven't take a look at releated RFCs) brackets should only be used with IPv6 addresses.
        //         Not sure it's a bug of gvfs or Apple, so do a workaround cleanup here.
        QString uriStr = QString(uri);
        QRegularExpression re(R"reg((\[)(?:\d+\.){3}\d+(\]))reg");
        if (re.match(uriStr).hasMatch()) {
            uriStr.remove('[');
            uriStr.remove(']');
        }

        NetworkNode node;
        node.setUrl(uriStr);
        node.setDisplayName(QString(display_name));
        node.setIconType(QString(iconPath));

        qDebug() << node;

        nodeList.append(node);

        g_free(uri);
        g_free(display_name);
        g_free(iconPath);
        g_clear_object(&file);
        g_clear_object(&activatable_file);
    }

    DFMUrlBaseEvent *event = static_cast<DFMUrlBaseEvent *>(user_data);
    DUrl neturl = event->fileUrl();
    static QRegularExpression regExp("^(smb|smb-share)://((?!/).*)/(?<name>((?!/).*))",
                                     QRegularExpression::DotMatchesEverythingOption
                                     | QRegularExpression::DontCaptureOption
                                     | QRegularExpression::OptimizeOnFirstUsageOption);
    std::string stdStr = neturl.toString().toStdString();
    QString urlString = QUrl::fromPercentEncoding(stdStr.data());
    const QRegularExpressionMatch &match = regExp.match(urlString, 0, QRegularExpression::NormalMatch,
                                                        QRegularExpression::DontCheckSubjectStringMatchOption);
    if (match.hasMatch()) {
        QString filename = match.captured("name");
        if (!filename.isEmpty())
            neturl = DUrl(urlString.replace(filename,filename.toLower()));
        qInfo() << "current net url = " << neturl.toString();
    }
    NetworkNodes.remove(neturl);
    NetworkNodes.insert(neturl, nodeList);
    qDebug() << "request NetworkNodeList successfully";
}

void NetworkManager::restartGVFSD()
{
    QProcess p;
    p.start("killall", {"gvfsd"});
    bool ret = p.waitForFinished();
    if (ret) {
        bool result = QProcess::startDetached("/usr/lib/gvfs/gvfsd");
        qDebug() << "restart gvfsd" << result;
    } else {
        qDebug() << "killall gvfsd failed";
    }
}

void NetworkManager::fetchNetworks(const DFMUrlBaseEvent &event)
{
    qDebug() << event;
    DFMEvent *e = new DFMEvent(event);
    // fix bug 100864 使用smb://ip或者域名/共享名/path，gio使用这个路径作为挂载，就会直接通过网络访问smb://ip或者域名/共享名/path，
    // 但是共享下的path被删除了，所以gio返回错误，不让挂载。其真实要挂载的是smb://ip或者域名/共享名，这里修改流程挂载smb://ip或者域名/共享名
    // 再跳转到这个目录下smb://ip或者域名/共享名/path
    DUrl fileUrl = event.fileUrl();
    static const QRegExp rxPath(R"((^/[^/]+))");
    if (rxPath.indexIn(fileUrl.path()) != -1) {
        fileUrl.setPath(rxPath.cap(1));
    }
    QString path = fileUrl.toString();
    QString fullPath = event.fileUrl().toString();

    UDiskDeviceInfoPointer p1 = deviceListener->getDeviceByMountPoint(fullPath);
    UDiskDeviceInfoPointer p2 = deviceListener->getDeviceByMountPointFilePath(fullPath);

    qDebug() << path << fullPath << p1 << p2;

    if (p1) {
        e->setData(p1->getMountPointUrl());
        if (DUrl(path) != p1->getMountPointUrl()) {
            DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, e->fileUrl(), WindowManager::getWindowById(e->windowId()));
        } else {
            qWarning() << p1->getMountPointUrl() << "can't get data";
        }
    } else {
        std::string stdPath = path.toStdString();
        gchar *url = const_cast<gchar *>(stdPath.c_str());

        if (fetch_networks(url, e)) {
            QWidget *main_window = WindowManager::getWindowById(e->windowId());

            // call later
            QTimer::singleShot(0, this, [fullPath, main_window] {
                const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(nullptr, DUrl(fullPath));

                if (info && info->canRedirectionFileUrl()) {
                    DUrl redirectUrl = info->redirectedFileUrl();
                    redirectUrl.setScheme(redirectUrl.scheme()+ NETWORK_REDIRECT_SCHEME_EX);
                    redirectUrl.setQuery(fullPath);
                    DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(nullptr, redirectUrl, main_window);
                 }
            });
        }
    }
    delete e;
}

void NetworkManager::cancelFeatchNetworks()
{
    if (eventLoop)
        eventLoop->exit(EventLoopCode::FetchFailed);
}
