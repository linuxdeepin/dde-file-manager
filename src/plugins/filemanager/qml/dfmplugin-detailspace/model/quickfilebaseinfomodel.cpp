// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quickfilebaseinfomodel.h"
#include "utils/detailmanager.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace dfmplugin_detailspace {

// TODO: remove it 后续 BasicFieldExpandEnum 和 DetailFilterType 并为单个枚举
static DetailFilterType enumConvert(BasicFieldExpandEnum value)
{
    switch (value) {
    case BasicFieldExpandEnum::kFileName:
        return DetailFilterType::kFileNameField;
    case BasicFieldExpandEnum::kFileSize:
        return DetailFilterType::kFileSizeField;
    case BasicFieldExpandEnum::kFileViewSize:
        return DetailFilterType::kFileViewSizeField;
    case BasicFieldExpandEnum::kFileDuration:
        return DetailFilterType::kFileDurationField;
    case BasicFieldExpandEnum::kFileType:
        return DetailFilterType::kFileTypeField;
    case BasicFieldExpandEnum::kFileInterviewTime:
        return DetailFilterType::kFileInterviewTimeField;
    case BasicFieldExpandEnum::kFileChangeTIme:
        return DetailFilterType::kFileChangeTimeField;
    default:
        return DetailFilterType::kNotFilter;
    }
}

/*!
 * \class QuickFileBaseInfoModel
 * \brief 基础文件信息模型
 * \details 根据传入的文件信息获取基础文件名/大小/修改时间等信息，对音视频文件提供文件大小和时长信息
 */
QuickFileBaseInfoModel::QuickFileBaseInfoModel(QObject *parent)
    : QAbstractListModel { parent }
{
    connect(this, &QuickFileBaseInfoModel::sigExtenInfo,
            this, &QuickFileBaseInfoModel::receiveExtenInfo, Qt::QueuedConnection);
}

QUrl QuickFileBaseInfoModel::url() const
{
    return modelUrl;
}

void QuickFileBaseInfoModel::setUrl(const QUrl &url)
{
    if (url != modelUrl) {
        modelUrl = url;
        resetFieldValue(modelUrl);

        Q_EMIT urlChanged(url);
    }
}

int QuickFileBaseInfoModel::rowCount(const QModelIndex &parent) const
{
    return keyValueList.size();
}

QVariant QuickFileBaseInfoModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= keyValueList.size()) {
        return {};
    }

    const KeyValue &keyValue = keyValueList.at(index.row());
    switch (role) {
    case kKeyRole:
        return keyValue.key;
    case kValueRole:
        return keyValue.value;
    }

    return {};
}

QHash<int, QByteArray> QuickFileBaseInfoModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[kKeyRole] = "key";
    roles[kValueRole] = "value";
    return roles;
}

void QuickFileBaseInfoModel::imageExtenInfo(const QUrl &url, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if ((url == modelUrl) && !properties.isEmpty()) {
        QString viewSize = parseViewSize(properties);
        Q_EMIT sigExtenInfo(DetailFilterType::kFileViewSizeField, viewSize);
    }
}

void QuickFileBaseInfoModel::videoExtenInfo(const QUrl &url, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if ((url == modelUrl) && !properties.isEmpty()) {
        QString viewSize = parseViewSize(properties);
        Q_EMIT sigExtenInfo(DetailFilterType::kFileViewSizeField, viewSize);
        QString durationStr = parseDuration(properties);
        Q_EMIT sigExtenInfo(DetailFilterType::kFileDurationField, durationStr);
    }
}

void QuickFileBaseInfoModel::audioExtenInfo(const QUrl &url, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties)
{
    if ((url == modelUrl) && !properties.isEmpty()) {
        QString durationStr = parseViewSize(properties);
        Q_EMIT sigExtenInfo(DetailFilterType::kFileDurationField, durationStr);
    }
}

void QuickFileBaseInfoModel::receiveExtenInfo(DetailFilterType field, const QString &value)
{
    appendKeyValue(field, value);
}

void QuickFileBaseInfoModel::clearKeyValue()
{
    markedType = DetailFilterType::kNotFilter;
    keyValueList.clear();
}

void QuickFileBaseInfoModel::resetFieldValue(const QUrl &url)
{
    reseting = true;
    beginResetModel();

    clearKeyValue();

    markFilter(url);
    // 优先执行拓展，以执行替换操作
    basicExpand(url);
    basicFill(url);

    sortKeyValue();

    endResetModel();
    reseting = false;
}

/*!
 * \brief 获取路径 \a url 当前过滤的字段类型，通过标记 Flag 已写入进行过滤
 */
void QuickFileBaseInfoModel::markFilter(const QUrl &url)
{
    QUrl filterUrl = url;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ filterUrl }, &urls);

    if (ok && !urls.isEmpty())
        filterUrl = urls.first();

    DetailFilterType fieldFilters = DetailManager::instance().basicFiledFiltes(filterUrl);

    // 设置过滤标识 & 已被写入的数据(拓展模块替换) 不需要二次写入
    markedType &= fieldFilters;
}

/*!
 * \brief 填充 \a url 拓展信息，插入的数据不会标记写入标识，相同字段后续也可插入；替换的数据后续不会重复更新
 */
void QuickFileBaseInfoModel::basicExpand(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpandMap> fieldCondition = DetailManager::instance().createBasicViewExtensionField(url);

    QMapIterator exIter(fieldCondition);
    while (exIter.hasNext()) {
        QMultiMapIterator valueItr(exIter.value());

        while (valueItr.hasNext()) {
            switch (exIter.key()) {
            case BasicExpandType::kFieldInsert:
                appendKeyValue(enumConvert(valueItr.key()), valueItr.value().second, valueItr.value().first, true);
                break;
            case BasicExpandType::kFieldReplace:
                appendKeyValue(enumConvert(valueItr.key()), valueItr.value().second, valueItr.value().first);
                break;
            }

            valueItr.next();
        }

        exIter.next();
    }
}

/*!
 * \brief 填充 \a url 基础信息，名称/大小/修改时间，音视频文件会请求拓展插件提取文件大小和长度。
 */
void QuickFileBaseInfoModel::basicFill(const QUrl &url)
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (info.isNull())
        return;

    if (!(markedType & DetailFilterType::kFileNameField)) {
        appendKeyValue(DetailFilterType::kFileNameField, info->displayOf(DisPlayInfoType::kFileDisplayName));
    }

    if (!(markedType & DetailFilterType::kFileSizeField)) {
        if (!info->isAttributes(OptInfoType::kIsDir)) {
            appendKeyValue(DetailFilterType::kFileSizeField, FileUtils::formatSize(info->size()));
        }
    }

    QUrl localUrl = url;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
    if (ok && !urls.isEmpty())
        localUrl = urls.first();
    FileInfoPointer localinfo = InfoFactory::create<FileInfo>(localUrl);
    if (!localinfo) {
        return;
    }

    if (!(markedType & kFileTypeField)) {
        const QString &mimeName { localinfo->nameOf(NameInfoType::kMimeTypeName) };
        const FileInfo::FileType type = MimeTypeDisplayManager::instance()->displayNameToEnum(mimeName);
        appendKeyValue(DetailFilterType::kFileTypeField, localinfo->displayOf(DisPlayInfoType::kMimeTypeDisplayName));
        QList<DFileInfo::AttributeExtendID> extenList;

        if (type == FileInfo::FileType::kVideos) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth << DFileInfo::AttributeExtendID::kExtendMediaHeight << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished, this, &QuickFileBaseInfoModel::videoExtenInfo, Qt::UniqueConnection);
            const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = localinfo->mediaInfoAttributes(DFileInfo::MediaType::kVideo, extenList);
            if (!mediaAttributes.isEmpty())
                videoExtenInfo(url, mediaAttributes);

        } else if (type == FileInfo::FileType::kImages) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaWidth << DFileInfo::AttributeExtendID::kExtendMediaHeight;
            connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished, this, &QuickFileBaseInfoModel::imageExtenInfo, Qt::UniqueConnection);
            const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = localinfo->mediaInfoAttributes(DFileInfo::MediaType::kImage, extenList);
            if (!mediaAttributes.isEmpty())
                imageExtenInfo(url, mediaAttributes);

        } else if (type == FileInfo::FileType::kAudios) {
            extenList << DFileInfo::AttributeExtendID::kExtendMediaDuration;
            connect(&FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished, this, &QuickFileBaseInfoModel::audioExtenInfo, Qt::UniqueConnection);
            const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = localinfo->mediaInfoAttributes(DFileInfo::MediaType::kAudio, extenList);
            if (!mediaAttributes.isEmpty())
                audioExtenInfo(url, mediaAttributes);
        }
    }

    if (!(markedType & kFileInterviewTimeField)) {
        auto lastRead = localinfo->timeOf(TimeInfoType::kLastRead).value<QDateTime>();
        if (lastRead.isValid()) {
            appendKeyValue(DetailFilterType::kFileInterviewTimeField, lastRead.toString(FileUtils::dateTimeFormat()));
        }
    }

    if (!(markedType & kFileChangeTimeField)) {
        auto lastModified = localinfo->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
        if (lastModified.isValid()) {
            appendKeyValue(DetailFilterType::kFileChangeTimeField, lastModified.toString(FileUtils::dateTimeFormat()));
        }
    }
}

void QuickFileBaseInfoModel::sortKeyValue()
{
    std::sort(keyValueList.begin(), keyValueList.end(), [](const KeyValue &first, const KeyValue &second) {
        return first.type < second.type;
    });
}

QString QuickFileBaseInfoModel::parseViewSize(const QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> &properties) const
{
    int width = properties[DFileInfo::AttributeExtendID::kExtendMediaWidth].toInt();
    int height = properties[DFileInfo::AttributeExtendID::kExtendMediaHeight].toInt();
    const QString &viewSize = QString::number(width) + "x" + QString::number(height);

    return viewSize;
}

QString QuickFileBaseInfoModel::parseDuration(const QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> &properties) const
{
    QVariant duration = properties[DFileInfo::AttributeExtendID::kExtendMediaDuration];
    int s = duration.toInt();
    QTime t(0, 0, 0);
    t = t.addMSecs(s);
    const QString &durationStr = t.toString("hh:mm:ss");

    return durationStr;
}

QString QuickFileBaseInfoModel::displayFieldName(DetailFilterType field)
{
    switch (field) {
    case DetailFilterType::kFileNameField:
        return tr("Name");
    case DetailFilterType::kFileSizeField:
        return tr("Size");
    case DetailFilterType::kFileViewSizeField:
        return tr("Dimension");
    case DetailFilterType::kFileDurationField:
        return tr("Duration");
    case DetailFilterType::kFileTypeField:
        return tr("Type");
    case DetailFilterType::kFileInterviewTimeField:
        return tr("Accessed");
    case DetailFilterType::kFileChangeTimeField:
        return tr("Modified");
    default:
        return tr("Unknown");
    }
}

/*!
 * \brief 追加文件信息键值对，\a field 是当前字段标识，存储时通过 displayFieldName() 转换为键名，\a value 为展示的值信息，
 *  \a specialKey 用于拓展信息显示特殊的字段名称，\a insert 用于标记字段数据是被插入的，不会记录对应字段已被写入。
 *  更新数据后，对应字段(非插入)会标记已写入，后续不会重复更新。
 */
void QuickFileBaseInfoModel::appendKeyValue(DetailFilterType field, const QString &value, const QString &specialKey, bool insert)
{
    // 除拓展数据，已有的数据不允许二次插入
    if (markedType.testFlags(field) && !insert) {
        return;
    }

    // 非插入数据会标记字段已被填充
    if (!insert) {
        markedType |= field;
    }

    // 拓展的数据使用特殊键值
    QString key = specialKey.isEmpty() ? displayFieldName(field) : specialKey;

    if (!reseting) {
        // 非复位模型状态，按顺序插入
        auto findItr = std::lower_bound(keyValueList.begin(), keyValueList.end(), field, [](const KeyValue &val, DetailFilterType cmpType) {
            return val.type < cmpType;
        });
        int index = std::distance(keyValueList.begin(), findItr);

        beginInsertRows(QModelIndex(), index, index);
        keyValueList.insert(index, { field, key, value });
        endInsertRows();

    } else {
        keyValueList.append({ field, key, value });
    }
}

}
