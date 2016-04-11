#ifndef TRASHFILEINFO_H
#define TRASHFILEINFO_H

#include "abstractfileinfo.h"

class TrashFileInfo : public AbstractFileInfo
{
public:
    TrashFileInfo();
    TrashFileInfo(const QUrl &url);
    TrashFileInfo(const QString &url);

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;

    QString displayName() const;

    void setUrl(const QString &fileUrl) Q_DECL_OVERRIDE;
    void setUrl(const QUrl &fileUrl) Q_DECL_OVERRIDE;

    QIcon fileIcon() const Q_DECL_OVERRIDE;

    QString parentUrl() const Q_DECL_OVERRIDE;

private:
    QString desktopIconName;
    QString m_displayName;
    QString originalPath;
    QString deletionDate;

    void updateInfo();
};

#endif // TRASHFILEINFO_H
