#ifndef DMIMEDATABASE_H
#define DMIMEDATABASE_H

#include <QMimeDatabase>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DMimeDatabase : public QMimeDatabase
{
    Q_DISABLE_COPY(DMimeDatabase)

public:
    DMimeDatabase();

    QMimeType mimeTypeForFile(const QString &fileName, MatchMode mode = MatchDefault) const;
    QMimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode = MatchDefault) const;
    QMimeType mimeTypeForUrl(const QUrl &url) const;
};

DFM_END_NAMESPACE

#endif // DMIMEDATABASE_H
