#ifndef DMIMEDATABASE_H
#define DMIMEDATABASE_H

#include <QMimeDatabase>
#include <QFileInfo>

class DMimeDatabase : public QMimeDatabase
{
public:
    DMimeDatabase();
    static char* XDG_RUNTIME_DIR;
    static bool isGvfsFile(const QString& fileName);
    QMimeType mimeTypeForFile(const QString &fileName, MatchMode mode = MatchDefault) const;
    QMimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode = MatchDefault) const;
};

#endif // DMIMEDATABASE_H
