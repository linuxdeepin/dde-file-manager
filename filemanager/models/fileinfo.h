#ifndef FILEINFO_H
#define FILEINFO_H

#include <QObject>
#include <QFileInfo>
#include <QUrl>
#include <QMimeType>

#include "utils/debugobejct.h"

#include "abstractfileinfo.h"

class FileInfo : public AbstractFileInfo
{
public:
    FileInfo();
    FileInfo(const QString & file);
    FileInfo(const QFileInfo &fileInfo);

    virtual void setFile(const QString &file);

    static bool exists(const QString &file);
    static QMimeType mimeType(const QString &file);

    bool isCanRename() const Q_DECL_OVERRIDE;

    QString mimeTypeName() const Q_DECL_OVERRIDE;

    QIcon fileIcon() const Q_DECL_OVERRIDE;

private:
    using AbstractFileInfo::setUrl;
};

Q_DECLARE_METATYPE(FileInfo)

#endif // FILEINFO_H
