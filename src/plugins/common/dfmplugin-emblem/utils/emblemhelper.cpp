// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "emblemhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/protocolutils.h>

#include <dfm-framework/event/event.h>
#include <dfm-io/dfileinfo.h>

#include <QDebug>
#include <QStandardPaths>

USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DPEMBLEM_USE_NAMESPACE

void GioEmblemWorker::onProduce(const FileInfoPointer &info)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());

    if (!info) {
        return;  // 添加空指针检查
    }

    const auto &emblems { fetchEmblems(info) };

    const QUrl &url = info->urlOf(UrlInfoType::kUrl);
    if (cache.contains(url)) {
        const auto &old { cache.value(url) };
        if (!iconNamesEqual(old, emblems)) {
            cache[url] = emblems;
            emit emblemChanged(url, emblems);
        }
    } else {   // save to cache
        cache.insert(url, emblems);
        emit emblemChanged(url, emblems);
    }
}

void GioEmblemWorker::onClear()
{
    cache.clear();
}

QList<QIcon> GioEmblemWorker::fetchEmblems(const FileInfoPointer &info) const
{
    if (!info)
        return {};

    QList<QIcon> emblemList;

    // add gio emblem icons
    const auto &gioEmblemsMap = getGioEmblems(info);
    QMap<int, QIcon>::const_iterator iter = gioEmblemsMap.begin();
    while (iter != gioEmblemsMap.end()) {
        if (iter.key() == emblemList.count()) {
            emblemList.append(iter.value());
            ++iter;
            continue;
        }

        if (iter.key() > emblemList.count()) {
            emblemList.append(QIcon());
            continue;
        }

        if (emblemList.at(iter.key()).isNull())
            emblemList.replace(iter.key(), iter.value());

        ++iter;
    }

    return emblemList;
}

QMap<int, QIcon> GioEmblemWorker::getGioEmblems(const FileInfoPointer &info) const
{
    QMap<int, QIcon> emblemsMap;

    if (!info)
        return {};
    const QStringList &emblemData = info->customAttribute("metadata::emblems", DFileInfo::DFileAttributeType::kTypeStringV).toStringList();

    if (emblemData.isEmpty())
        return emblemsMap;

    const QString &emblemsStr = emblemData.first();

    if (!emblemsStr.isEmpty()) {
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
        const QStringList &emblemsStrList = emblemsStr.split("|", QString::SkipEmptyParts);
#else
        const QStringList &emblemsStrList = emblemsStr.split("|", Qt::SkipEmptyParts);
#endif
        for (int i = 0; i < emblemsStrList.length(); i++) {
            QString pos;
            QIcon emblem;
            if (parseEmblemString(&emblem, pos, emblemsStrList.at(i)))
                setEmblemIntoIcons(pos, emblem, &emblemsMap);
        }

        return emblemsMap;
    }

    return emblemsMap;
}

bool GioEmblemWorker::parseEmblemString(QIcon *emblem, QString &pos, const QString &emblemStr) const
{
    // default position
    pos = "rd";

    if (!emblemStr.isEmpty()) {
        QIcon emblemIcon;
        QString imgPath;

        if (emblemStr.contains(";")) {
            QStringList emStrList = emblemStr.split(";");
            imgPath = emStrList.at(0);
            pos = emStrList.at(1);
        } else {
            imgPath = emblemStr;
        }

        // to standard path
        if (imgPath.startsWith("~/"))
            imgPath.replace(0, 1, QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

        DFMIO::DFile dfile(imgPath);
        if (dfile.exists()) {
            if (dfile.size() > 102400)   // size small than 100kb
                return false;

            auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(imgPath));
            const QString &suffix = info->nameOf(NameInfoType::kCompleteSuffix);
            // check support type
            if (suffix != "svg" && suffix != "png" && suffix != "gif" && suffix != "bmp" && suffix != "jpg")
                return false;

            emblemIcon = QIcon(imgPath);
            if (!emblemIcon.isNull()) {
                *emblem = emblemIcon;
                return true;
            }
        }
    }

    return false;
}

bool GioEmblemWorker::iconNamesEqual(const QList<QIcon> &first, const QList<QIcon> &second)
{
    if (first.isEmpty() && second.isEmpty())
        return true;

    if (first.size() != second.size())
        return false;

    QVector<QString> firstNames { first.size(), QString() };
    QVector<QString> secondNames { second.size(), QString() };

    std::transform(first.begin(), first.end(), firstNames.begin(), [](const QIcon &icon) {
        return icon.name();
    });

    std::transform(second.begin(), second.end(), secondNames.begin(), [](const QIcon &icon) {
        return icon.name();
    });

    return firstNames == secondNames;
}

void GioEmblemWorker::setEmblemIntoIcons(const QString &pos, const QIcon &emblem, QMap<int, QIcon> *iconMap) const
{
    int emblemIndex = 0;   // default position rd = 0, rightdown

    // left down
    if (pos == "ld")
        emblemIndex = 1;
    // left up
    else if (pos == "lu")
        emblemIndex = 2;
    // right up
    else if (pos == "ru")
        emblemIndex = 3;

    (*iconMap)[emblemIndex] = emblem;
}

EmblemHelper::EmblemHelper(QObject *parent)
    : QObject(parent)
{
    initialize();
}

EmblemHelper::~EmblemHelper()
{
    workerThread.quit();
    workerThread.wait();
}

QList<QIcon> EmblemHelper::systemEmblems(const FileInfoPointer &info) const
{
    static bool hideSystemEmblems = DConfigManager::instance()->value(kConfigPath, kHideSystemEmblems, false).toBool();
    if (hideSystemEmblems)
        return {};

    if (!info)
        return {};

    // feat: story 1477
    // For desktop files hide all system emblem icons
    if (FileUtils::isDesktopFileInfo(info))
        return {};

    QList<QIcon> emblems;

    if (info->isAttributes(OptInfoType::kIsSymLink))
        emblems << QIcon::fromTheme("emblem-symbolic-link", standardEmblem(SystemEmblemType::kLink));

    if (!info->isAttributes(OptInfoType::kIsWritable))
        emblems << QIcon::fromTheme("emblem-readonly", standardEmblem(SystemEmblemType::kLock));

    if (!info->isAttributes(OptInfoType::kIsReadable))
        emblems << QIcon::fromTheme("emblem-unreadable", standardEmblem(SystemEmblemType::kUnreadable));

    bool shared = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_IsPathShared", info->pathOf(PathInfoType::kAbsoluteFilePath)).toBool();
    if (shared)
        emblems << QIcon::fromTheme("emblem-shared", standardEmblem(SystemEmblemType::kShare));

    return emblems;
}

QList<QRectF> EmblemHelper::emblemRects(const QRectF &paintArea) const
{
    QSizeF baseEmblemSize = paintArea.size() / 3;
    baseEmblemSize.setWidth(qBound(kMinEmblemSize, baseEmblemSize.width(), kMaxEmblemSize));
    baseEmblemSize.setHeight(qBound(kMinEmblemSize, baseEmblemSize.width(), kMaxEmblemSize));

    double offset = paintArea.width() / 8;
    const QSizeF &offsetSize = baseEmblemSize / 2;

    QList<QRectF> list;
    list.append(QRectF(QPointF(paintArea.right() - offset - offsetSize.width(),
                               paintArea.bottom() - offset - offsetSize.height()),
                       baseEmblemSize));
    list.append(QRectF(QPointF(paintArea.left() + offset - offsetSize.width(), list.first().top()), baseEmblemSize));
    list.append(QRectF(QPointF(list.at(1).left(), paintArea.top() + offset - offsetSize.height()), baseEmblemSize));
    list.append(QRectF(QPointF(list.first().left(), list.at(2).top()), baseEmblemSize));

    return list;
}

QList<QIcon> EmblemHelper::gioEmblemIcons(const QUrl &url) const
{
    if (hasEmblem(url))
        return productQueue.value(url);

    return {};
}

void EmblemHelper::pending(const FileInfoPointer &info)
{
    if (!info)
        return;
    
    emit requestProduce(info);
}

bool EmblemHelper::isExtEmblemProhibited(const FileInfoPointer &info, const QUrl &url)
{
    // SMB mounted by cifs (v6), so mountpoint is native path
    if (ProtocolUtils::isRemoteFile(url))
        return true;

    // In the block device, all file extension emblem icons are displayed by default,
    // When configuring emblem icons display, all file extension corners are displayed in the block device
    // When emblem icons hiding is configured, all file extension corners are hidden in the block device
    if ((info ? !info->extendAttributes(ExtInfoType::kFileLocalDevice).toBool() : !ProtocolUtils::isLocalFile(url))) {
        bool enable { DConfigManager::instance()->value("org.deepin.dde.file-manager.emblem", "blockExtEnable", true).toBool() };
        if (enable)
            return false;
        return true;
    }

    return false;
}

void EmblemHelper::onEmblemChanged(const QUrl &url, const Product &product)
{
    productQueue[url] = product;
    if (product.isEmpty())
        return;
    auto eventID { DPF_NAMESPACE::Event::instance()->eventType("ddplugin_canvas", "slot_FileInfoModel_UpdateFile") };
    if (eventID != DPF_NAMESPACE::EventTypeScope::kInValid)
        dpfSlotChannel->push("ddplugin_canvas", "slot_FileInfoModel_UpdateFile", url);
    else
        dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_FileUpdate", url);
}

bool EmblemHelper::onUrlChanged(quint64 windowId, const QUrl &url)
{
    Q_UNUSED(windowId);
    Q_UNUSED(url);

    clearEmblem();
    emit requestClear();

    return false;
}

void EmblemHelper::initialize()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    dpfSignalDispatcher->installEventFilter(GlobalEventType::kChangeCurrentUrl, this, &EmblemHelper::onUrlChanged);

    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &EmblemHelper::requestProduce, worker, &GioEmblemWorker::onProduce, Qt::QueuedConnection);
    connect(this, &EmblemHelper::requestClear, worker, &GioEmblemWorker::onClear, Qt::QueuedConnection);
    connect(worker, &GioEmblemWorker::emblemChanged, this, &EmblemHelper::onEmblemChanged, Qt::QueuedConnection);

    workerThread.start();
}

QIcon EmblemHelper::standardEmblem(const SystemEmblemType type) const
{
    static QIcon linkEmblem(QIcon::fromTheme("emblem-symbolic-link"));
    static QIcon lockEmblem(QIcon::fromTheme("emblem-locked"));
    static QIcon unreadableEmblem(QIcon::fromTheme("emblem-unreadable"));
    static QIcon shareEmblem(QIcon::fromTheme("emblem-shared"));

    switch (type) {
    case SystemEmblemType::kLink:
        return linkEmblem;
    case SystemEmblemType::kLock:
        return lockEmblem;
    case SystemEmblemType::kUnreadable:
        return unreadableEmblem;
    case SystemEmblemType::kShare:
        return shareEmblem;
    }

    return QIcon();
}
