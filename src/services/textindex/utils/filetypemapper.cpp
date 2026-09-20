// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetypemapper.h"

#include <DConfig>

#include <QFileInfo>

#include <memory>

DCORE_USE_NAMESPACE
SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {

// org.deepin.anything dconfig 的后缀键 → anything file_type 字段值。
// 类型字符串必须与 deepin-anything-daemon 的 file_type_mapping_ 完全一致
// （dfm-search 按这些值做类型过滤）。
struct SuffixKeyMapping
{
    const char *dconfigKey;
    const char *fileType;
};

constexpr SuffixKeyMapping kSuffixKeyMappings[] = {
    { "app_file_suffix", "app" },
    { "archive_file_suffix", "archive" },
    { "audio_file_suffix", "audio" },
    { "doc_file_suffix", "doc" },
    { "pic_file_suffix", "pic" },
    { "video_file_suffix", "video" },
};

}   // namespace

FileTypeMapper &FileTypeMapper::instance()
{
    static FileTypeMapper instance;
    return instance;
}

FileTypeMapper::FileTypeMapper()
{
    loadMappings();
}

void FileTypeMapper::loadMappings()
{
    // 局部同步使用，统一用 RAII 清理；不用 deleteLater()——本函数可能在
    // 静态单例构造期间执行，此时无事件循环，deferred delete 不会执行导致泄漏。
    const std::unique_ptr<DConfig> dconfig(DConfig::create("org.deepin.anything", "org.deepin.anything"));
    if (!dconfig || !dconfig->isValid()) {
        fmWarning() << "FileTypeMapper: Failed to load org.deepin.anything dconfig";
        return;
    }

    for (const auto &mapping : kSuffixKeyMappings) {
        const QVariant value = dconfig->value(QString::fromLatin1(mapping.dconfigKey));
        if (!value.isValid())
            continue;

        // anything 的后缀配置为分号分隔字符串（如 "7z;ace;ar;..."），兼容 stringlist 类型
        QStringList extensions;
        if (value.type() == QVariant::StringList || value.type() == QVariant::List) {
            extensions = value.toStringList();
        } else {
            extensions = value.toString().split(';', Qt::SkipEmptyParts);
        }

        const QString type = QString::fromLatin1(mapping.fileType);
        for (const QString &ext : std::as_const(extensions)) {
            QString cleaned = ext.trimmed();
            if (cleaned.startsWith('.'))
                cleaned = cleaned.mid(1);
            if (!cleaned.isEmpty())
                m_extToType.insert(cleaned.toLower(), type);
        }
    }

    fmInfo() << "FileTypeMapper: Loaded" << m_extToType.size() << "extension mappings";
}

QString FileTypeMapper::fileTypeForExtension(const QString &ext) const
{
    QString cleaned = ext;
    if (cleaned.startsWith('.'))
        cleaned = cleaned.mid(1);
    cleaned = cleaned.toLower();

    auto it = m_extToType.constFind(cleaned);
    if (it != m_extToType.constEnd())
        return it.value();
    return QStringLiteral("other");
}

QString FileTypeMapper::fileTypeForPath(const QString &filePath) const
{
    const QFileInfo fileInfo(filePath);
    if (fileInfo.isDir())
        return QStringLiteral("dir");

    const QString ext = fileInfo.suffix().toLower();
    if (ext.isEmpty())
        return QStringLiteral("other");

    return fileTypeForExtension(ext);
}

SERVICETEXTINDEX_END_NAMESPACE
