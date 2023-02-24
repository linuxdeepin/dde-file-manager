// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "emblemhelper.h"

#include "dfm-base/utils/decorator/decoratorfileinfo.h"
#include "dfm-base/utils/decorator/decoratorfile.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/event/event.h>
#include <dfm-io/core/dfileinfo.h>

#include <QDebug>
#include <QStandardPaths>

USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DPEMBLEM_USE_NAMESPACE

void EmblemWorker::onProduce(const QUrl &url)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());

    const auto &emblems { fetchEmblems(url) };

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

void EmblemWorker::onClear()
{
    cache.clear();
}

QList<QIcon> EmblemWorker::fetchEmblems(const QUrl &url) const
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (!info)
        return {};

    QList<QIcon> emblemList;

    emblemList = getSystemEmblems(info);

    if (FileUtils::isGvfsFile(url))
        return emblemList;

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

QList<QIcon> EmblemWorker::getSystemEmblems(const AbstractFileInfoPointer &info) const
{
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

QMap<int, QIcon> EmblemWorker::getGioEmblems(const AbstractFileInfoPointer &info) const
{
    QMap<int, QIcon> emblemsMap;

    // use AbstractFileInfo to access emblems, avoid query again
    AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(info->urlOf(UrlInfoType::kUrl), false);
    if (!fileInfo)
        return {};
    const QStringList &emblemData = fileInfo->customAttribute("metadata::emblems", DFileInfo::DFileAttributeType::kTypeStringV).toStringList();

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

QIcon EmblemWorker::standardEmblem(const SystemEmblemType type) const
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

bool EmblemWorker::parseEmblemString(QIcon *emblem, QString &pos, const QString &emblemStr) const
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

        DecoratorFile dfile(imgPath);
        if (dfile.exists()) {
            if (dfile.size() > 102400)   // size small than 100kb
                return false;

            auto info = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(imgPath));
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

bool EmblemWorker::iconNamesEqual(const QList<QIcon> &first, const QList<QIcon> &second)
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

void EmblemWorker::setEmblemIntoIcons(const QString &pos, const QIcon &emblem, QMap<int, QIcon> *iconMap) const
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

QList<QIcon> EmblemHelper::emblemIcons(const QUrl &url) const
{
    if (hasEmblem(url))
        return productQueue.value(url);

    return {};
}

void EmblemHelper::pending(const QUrl &url)
{
    emit requestProduce(url);
}

void EmblemHelper::onEmblemChanged(const QUrl &url, const Product &product)
{
    productQueue[url] = product;
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
    connect(this, &EmblemHelper::requestProduce, worker, &EmblemWorker::onProduce, Qt::QueuedConnection);
    connect(this, &EmblemHelper::requestClear, worker, &EmblemWorker::onClear, Qt::QueuedConnection);
    connect(worker, &EmblemWorker::emblemChanged, this, &EmblemHelper::onEmblemChanged, Qt::QueuedConnection);

    workerThread.start();
}
