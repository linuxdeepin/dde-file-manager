#ifndef MIMETYPEDISPLAYMANAGER_H
#define MIMETYPEDISPLAYMANAGER_H

#include <QObject>
#include <QMap>

#include "dabstractfileinfo.h"

class MimeTypeDisplayManager : public QObject
{
    Q_OBJECT

public:

    typedef DAbstractFileInfo::FileType FileType;

    explicit MimeTypeDisplayManager(QObject *parent = 0);
    ~MimeTypeDisplayManager();

    void initData();
    void initConnect();

    QString displayName(const QString& mimeType);
    FileType displayNameToEnum(const QString& mimeType);
    QString defaultIcon(const QString& mimeType);

    QMap<FileType, QString> displayNames();
    static QStringList readlines(const QString& path);
    static void loadSupportMimeTypes();
    static QStringList supportArchiveMimetypes();
    static QStringList supportVideoMimeTypes();

private:
    QMap<FileType, QString> m_displayNames;
    QMap<FileType, QString> m_defaultIconNames;
    static QStringList ArchiveMimeTypes;
    static QStringList AvfsBlackList;
    static QStringList TextMimeTypes;
    static QStringList VideoMimeTypes;
    static QStringList AudioMimeTypes;
    static QStringList ImageMimeTypes;
    static QStringList ExecutableMimeTypes;

    bool m_supportLoaded = false;
};

#endif // MIMETYPEDISPLAYMANAGER_H
