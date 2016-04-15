#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "abstractfilecontroller.h"

class SearchController : AbstractFileController
{
public:
    explicit SearchController(QObject *parent = 0);

    const QList<AbstractFileInfo*> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;
    AbstractFileInfo *createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
};

#endif // SEARCHCONTROLLER_H
