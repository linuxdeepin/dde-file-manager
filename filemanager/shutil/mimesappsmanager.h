#ifndef MIMESAPPSMANAGER_H
#define MIMESAPPSMANAGER_H


#include <QSet>
#include <QMimeType>
#include <QMap>
#include <QFileInfo>

class MimesAppsManager
{
public:

    static QString getDefaultAppByFileName(const QString& fileName);
    static QString getDefaultAppByMimeType(const QMimeType& mimeType);
    static QString getDefaultAppByMimeType(const QString& mimeType);

    static QStringList getDesktopFiles();
    static QMap<QString, QStringList> getMimeTypeApps();
    static bool lessByDateTime(const QFileInfo& f1,  const QFileInfo& f2);

    static void test();

private:
    MimesAppsManager();
    ~MimesAppsManager();
};

#endif // MIMESAPPSMANAGER_H
