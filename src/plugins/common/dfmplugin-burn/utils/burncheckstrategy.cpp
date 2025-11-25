// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "burncheckstrategy.h"
#include "burnhelper.h"

#include <QDebug>

namespace dfmplugin_burn {

static constexpr int kMaxCommonDirDeepLength { 8 };
static constexpr int kMaxCommonFileNameBytes { 255 };
static constexpr int kMaxCommontFilePathBytes { 1024 };

static constexpr int kMaxISO9660FileNameSize { 32 };

static constexpr int kMaxJolietFileNameSize { 103 };
static constexpr int kMaxJolietFilePathSize { 800 };

BurnCheckStrategy::BurnCheckStrategy(const QString &path, QObject *parent)
    : QObject(parent), currentStagePath(path)
{
}

bool BurnCheckStrategy::check()
{
    Q_ASSERT(!currentStagePath.isEmpty());

    QFileInfo info { currentStagePath };
    if (!info.isDir())
        return true;

    const QFileInfoList &rootFileInfoGroup { BurnHelper::localFileInfoList(currentStagePath) };
    for (const QFileInfo &info : rootFileInfoGroup) {
        if (!validFile(info))
            return false;
        if (info.isDir()) {
            const auto &filters { QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks };
            const QFileInfoList &subFileInfoGroup { BurnHelper::localFileInfoListRecursive(info.absoluteFilePath(), filters) };
            if (!std::all_of(subFileInfoGroup.cbegin(), subFileInfoGroup.cend(), [this](const QFileInfo &info) {
                    return validFile(info);
                })) {
                return false;
            }
        }
    }

    return true;
}

QString BurnCheckStrategy::lastError() const
{
    return errorMsg;
}

QString BurnCheckStrategy::lastInvalidName() const
{
    return autoFeed(invalidName);
}

bool BurnCheckStrategy::validFile(const QFileInfo &info)
{
    if (!info.exists())
        return true;

    QString absoluteFilePathWithStagePath { info.absoluteFilePath() };
    const QString &fileName { info.fileName() };
    const QString &absoluteFilePath { QDir::separator() + absoluteFilePathWithStagePath.remove(currentStagePath) };
    invalidName = fileName;

    if (!validFileNameCharacters(fileName)) {
        errorMsg = "Invalid FileNameCharacters Length: " + invalidName;
        return false;
    }

    if (!validFilePathCharacters(absoluteFilePath)) {
        errorMsg = "Invalid FilePathCharacters Length: " + invalidName;
        return false;
    }

    if (!validFileNameBytes(fileName)) {
        errorMsg = "Invalid FileNameBytes Length: " + invalidName;
        return false;
    }

    if (!validFilePathBytes(absoluteFilePath)) {
        errorMsg = "Invalid FilePathBytes Length: " + invalidName;
        return false;
    }

    if (!validFilePathDeepLength(absoluteFilePath)) {
        errorMsg = "Invalid FilePathDeepLength: " + invalidName;
        return false;
    }

    invalidName = "";
    return true;
}

QString BurnCheckStrategy::autoFeed(const QString &text) const
{
    QString name { text };
    int c = name.length();

    if (c > 50) {
        for (int i = 1; i <= c / 50; i++) {
            int n = i * 50;
            name.insert(n, "\n");
        }
    }

    return name;
}

bool BurnCheckStrategy::validCommonFileNameBytes(const QString &fileName)
{
    return fileName.toUtf8().size() < kMaxCommonFileNameBytes;
}

bool BurnCheckStrategy::validComontFilePathBytes(const QString &filePath)
{
    return filePath.toUtf8().size() < kMaxCommontFilePathBytes;
}

bool BurnCheckStrategy::validCommonFilePathDeepLength(const QString &filePath)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    return filePath.split(QDir::separator(), QString::SkipEmptyParts).size() <= kMaxCommonDirDeepLength;
#else
    return filePath.split(QDir::separator(), Qt::SkipEmptyParts).size() <= kMaxCommonDirDeepLength;
#endif
}

bool BurnCheckStrategy::validFileNameCharacters(const QString &fileName)
{
    Q_UNUSED(fileName)
    return true;
}

bool BurnCheckStrategy::validFilePathCharacters(const QString &filePath)
{
    Q_UNUSED(filePath)
    return true;
}

bool BurnCheckStrategy::validFileNameBytes(const QString &fileName)
{
    Q_UNUSED(fileName)
    return true;
}

bool BurnCheckStrategy::validFilePathBytes(const QString &filePath)
{
    Q_UNUSED(filePath)
    return true;
}

bool BurnCheckStrategy::validFilePathDeepLength(const QString &filePath)
{
    Q_UNUSED(filePath)
    return true;
}

/*!
 * \brief 最大文件名/目录名：32个字符，且最大目录深度为8
 * \param path
 * \param parent
 */
ISO9660CheckStrategy::ISO9660CheckStrategy(const QString &path, QObject *parent)
    : BurnCheckStrategy(path, parent)
{
}

bool ISO9660CheckStrategy::validFileNameCharacters(const QString &fileName)
{
    return fileName.size() < kMaxISO9660FileNameSize;
}

bool ISO9660CheckStrategy::validFilePathDeepLength(const QString &filePath)
{
    return validCommonFilePathDeepLength(filePath);
}

/*!
 * \brief 最大文件名/目录名：64个字符，最大路径（包含文件名）的长度是120个字符
 * \param path
 * \param parent
 */
JolietCheckStrategy::JolietCheckStrategy(const QString &path, QObject *parent)
    : BurnCheckStrategy(path, parent)
{
}

bool JolietCheckStrategy::validFileNameCharacters(const QString &fileName)
{
    return fileName.size() < kMaxJolietFileNameSize;
}

bool JolietCheckStrategy::validFilePathCharacters(const QString &filePath)
{
    return filePath.size() < kMaxJolietFilePathSize;
}

/*!
 * \brief 文件名255字节，最大目录深度（含文件名）为1023字节，且最大目录深度为8
 * \param path
 * \param parent
 */
RockRidgeCheckStrategy::RockRidgeCheckStrategy(const QString &path, QObject *parent)
    : BurnCheckStrategy(path, parent)
{
}

bool RockRidgeCheckStrategy::validFileNameBytes(const QString &fileName)
{
    return validCommonFileNameBytes(fileName);
}

bool RockRidgeCheckStrategy::validFilePathBytes(const QString &filePath)
{
    return validComontFilePathBytes(filePath);
}

bool RockRidgeCheckStrategy::validFilePathDeepLength(const QString &filePath)
{
    return validCommonFilePathDeepLength(filePath);
}

/*!
 * \brief 文件名255字节，最大目录（含文件名）为1023字节
 * \param path
 * \param parent
 */
UDFCheckStrategy::UDFCheckStrategy(const QString &path, QObject *parent)
    : BurnCheckStrategy(path, parent)
{
}

bool UDFCheckStrategy::validFileNameBytes(const QString &fileName)
{
    return validCommonFileNameBytes(fileName);
}

bool UDFCheckStrategy::validFilePathBytes(const QString &filePath)
{
    return validComontFilePathBytes(filePath);
}

}   // namespace dfmplugin_burn
