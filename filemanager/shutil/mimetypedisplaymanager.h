#ifndef MIMETYPEDISPLAYMANAGER_H
#define MIMETYPEDISPLAYMANAGER_H

#include <QObject>
#include <QMap>

class MimeTypeDisplayManager : public QObject
{
    Q_OBJECT
public:
    explicit MimeTypeDisplayManager(QObject *parent = 0);
    ~MimeTypeDisplayManager();

    void initData();
    void initConnect();

    QString displayName(const QString& mimeType);

    static QStringList readlines(const QString& path);
    static void loadSupportMimeTypes();

signals:

public slots:

private:



    QMap<QString, QString> m_displayNames;
    static QStringList ArchiveMimeTypes;
    static QStringList TextMimeTypes;
    static QStringList VideoMimeTypes;
    static QStringList AudioMimeTypes;
    static QStringList ImageMimeTypes;
    static QStringList ExecutableMimeTypes;

    bool m_supportLoaded = false;
};

#endif // MIMETYPEDISPLAYMANAGER_H
