// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenamedocumentbuilder.h"

#include "utils/filetypemapper.h"
#include "utils/indexutility.h"
#include "utils/pinyinprocessor.h"

#include <dfm-search/field_names.h>

#include <lucene++/NumericField.h>

#include <QDateTime>
#include <QFileInfo>

#include <glib.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE
using namespace Lucene;
DFM_SEARCH_USE_NS
using namespace DFMSEARCH::LuceneFieldNames;

namespace {

// 生成祖先路径列表（与 deepin-anything add_ancestor_paths 一致）：
// 从文件路径向上逐级截断到父目录，直到根目录 "/"，依次加入列表（含 "/"）。
// 例如 "/home/user/a/b.txt" → ["/home/user/a", "/home/user", "/home", "/"]
QStringList buildAncestorPaths(const QString &filePath)
{
    QStringList ancestorPaths;
    if (filePath.isEmpty() || !filePath.startsWith('/'))
        return ancestorPaths;

    QString currentPath = filePath;
    int lastSlash = 1;
    while (lastSlash != 0) {
        lastSlash = currentPath.lastIndexOf('/');
        currentPath.resize(lastSlash == 0 ? 1 : lastSlash);
        ancestorPaths.append(currentPath);
    }
    return ancestorPaths;
}

}   // namespace

DocumentPtr FileNameDocumentBuilder::build(const QString &filePath, const QString &text,
                                           const BuilderOptions &options) const
{
    Q_UNUSED(text)
    Q_UNUSED(options)

    DocumentPtr doc = newLucene<Document>();

    const QFileInfo fileInfo(filePath);
    const QString fileName = fileInfo.fileName();

    // file_name — 小写后 NGram 分词（与 anything 的 toLower(toUnicode(file_name)) 一致）
    doc->add(newLucene<Field>(FileName::kFileName, fileName.toLower().toStdWString(),
                              Field::STORE_YES, Field::INDEX_ANALYZED));

    // file_name_lower — 小写、存储、不分词（通配符搜索）
    doc->add(newLucene<Field>(FileName::kFileNameLower, fileName.toLower().toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // full_path — 不分词、存储（主键 Term，删除/更新定位）
    doc->add(newLucene<Field>(FileName::kFullPath, filePath.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // file_type — 由 FileTypeMapper 依据 anything dconfig 后缀映射得到
    // （app/archive/audio/doc/pic/video/dir/other，与 anything 一致）
    const QString fileType = FileTypeMapper::instance().fileTypeForPath(filePath);
    doc->add(newLucene<Field>(FileName::kFileType, fileType.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // file_ext — 小写、不含点（与 anything tolower(extension().substr(1)) 一致）
    const QString fileExt = fileInfo.suffix().toLower();
    if (!fileExt.isEmpty()) {
        doc->add(newLucene<Field>(FileName::kFileExt, fileExt.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    }

    // birth_time / modify_time — Unix 秒
    const qint64 birthTimeSecs = fileInfo.birthTime().toSecsSinceEpoch();
    NumericFieldPtr birthTimeField = newLucene<NumericField>(FileName::kBirthTime, Field::STORE_YES, true);
    birthTimeField->setLongValue(birthTimeSecs);
    doc->add(birthTimeField);

    const qint64 modifyTimeSecs = fileInfo.lastModified().toSecsSinceEpoch();
    NumericFieldPtr modifyTimeField = newLucene<NumericField>(FileName::kModifyTime, Field::STORE_YES, true);
    modifyTimeField->setLongValue(modifyTimeSecs);
    doc->add(modifyTimeField);

    // file_size — 字节
    const qint64 fileSize = fileInfo.size();
    NumericFieldPtr fileSizeField = newLucene<NumericField>(FileName::kFileSize, Field::STORE_YES, true);
    fileSizeField->setLongValue(fileSize);
    doc->add(fileSizeField);

    // file_size_str — 与 anything 同样使用 glib g_format_size 格式化（SI，1000 进制）
    gchar *formatted = g_format_size(static_cast<guint64>(fileSize));
    doc->add(newLucene<Field>(FileName::kFileSizeStr, QString::fromUtf8(formatted).toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    g_free(formatted);

    // pinyin / pinyin_acronym — 拼音全拼与首字母（已小写化）
    QString pinyinFull;
    QString pinyinAcronym;
    PinyinProcessor::instance().convertToPinyin(fileName, pinyinFull, pinyinAcronym);
    if (!pinyinFull.isEmpty()) {
        doc->add(newLucene<Field>(FileName::kPinyin, pinyinFull.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_ANALYZED));
    }
    if (!pinyinAcronym.isEmpty()) {
        doc->add(newLucene<Field>(FileName::kPinyinAcronym, pinyinAcronym.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_ANALYZED));
    }

    // is_hidden — 路径中含隐藏段（/.xxx）则标记 Y
    const QString hiddenTag = DFMSEARCH::Global::isHiddenPathOrInHiddenDir(fileInfo.absoluteFilePath())
            ? QStringLiteral("Y")
            : QStringLiteral("N");
    doc->add(newLucene<Field>(FileName::kIsHidden, hiddenTag.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // ancestor_paths — 多值 Term，不存储（从父目录到 "/"）
    const QStringList ancestorPaths = buildAncestorPaths(filePath);
    for (const QString &ancestorPath : ancestorPaths) {
        doc->add(newLucene<Field>(FileName::kAncestorPaths, ancestorPath.toStdWString(),
                                  Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    }

    return doc;
}

std::list<PathDerivedFieldSpec> FileNameDocumentBuilder::pathDerivedFields() const
{
    return {
        { FileName::kIsHidden, [](const QString &newPath) -> FieldPtr {
             const QString hiddenTag = DFMSEARCH::Global::isHiddenPathOrInHiddenDir(QFileInfo(newPath).absoluteFilePath())
                     ? QStringLiteral("Y")
                     : QStringLiteral("N");
             return newLucene<Field>(FileName::kIsHidden, hiddenTag.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
         } },
    };
}

SERVICETEXTINDEX_END_NAMESPACE
