#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "abstractfilecontroller.h"

#include <QSet>

class SearchController : AbstractFileController
{
    Q_OBJECT

public:
    explicit SearchController(QObject *parent = 0);

    const QList<AbstractFileInfoPointer> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;
    const AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

private:
    void searchStart(const DUrl &fileUrl, QDir::Filters filter);

    mutable QSet<DUrl> activeJob;
};

#endif // SEARCHCONTROLLER_H
