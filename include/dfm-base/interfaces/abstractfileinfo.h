// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTFILEINFO_H
#define ABSTRACTFILEINFO_H

#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_base_global.h>

#include <dfm-io/dfileinfo.h>

#include <QSharedData>
#include <QFile>
#include <QMimeType>
#include <QVariant>
#include <QMetaType>

class QDir;
class QDateTime;

namespace dfmbase {
class AbstractFileInfo;
}

typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> AbstractFileInfoPointer;

namespace dfmbase {
class AbstractFileInfo : public QSharedData
{
public:
    explicit AbstractFileInfo() = delete;
    explicit AbstractFileInfo(const QUrl &url);
    virtual ~AbstractFileInfo();

    virtual QUrl fileUrl() const;
    virtual bool exists() const;
    virtual void refresh();
    virtual QString filePath() const;
    virtual QString absoluteFilePath() const;
    virtual QString fileName() const;
    virtual QString baseName() const;
    virtual QString completeBaseName() const;
    virtual QString suffix() const;
    virtual QString completeSuffix() const;

    virtual QString path() const;
    virtual QString absolutePath() const;

    virtual bool isReadable() const;
    virtual bool isWritable() const;
    virtual bool isExecutable() const;
    virtual bool isHidden() const;
    virtual bool isNativePath() const;

    virtual bool isFile() const;
    virtual bool isDir() const;
    virtual bool isSymLink() const;
    virtual bool isRoot() const;
    virtual bool isBundle() const;
    virtual QString symLinkTarget() const;

    virtual QString owner() const;
    virtual uint ownerId() const;
    virtual QString group() const;
    virtual uint groupId() const;

    virtual bool permission(QFile::Permissions permissions) const;
    virtual QFile::Permissions permissions() const;
    virtual int countChildFile() const;
    virtual qint64 size() const;

    // createdTime
    virtual quint32 birthTime() const;
    virtual quint32 metadataChangeTime() const;
    virtual quint32 lastModified() const;
    virtual quint32 lastRead() const;

protected:
    QUrl url;
};

}
Q_DECLARE_METATYPE(AbstractFileInfoPointer);
Q_DECLARE_METATYPE(QList<AbstractFileInfoPointer>);

#endif   // ABSTRACTFILEINFO_H
