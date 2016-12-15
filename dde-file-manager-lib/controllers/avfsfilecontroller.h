#ifndef AVFSFILECONTROLLER_H
#define AVFSFILECONTROLLER_H

#include <QObject>

#include "dabstractfilecontroller.h"

class AVFSFileController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit AVFSFileController(QObject* parent = 0);

    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

    const DDirIteratorPointer createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags, bool &accepted) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const DUrl &fileUrl, QObject *parent, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFileLocation(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool openFileByApp(const DUrl &fileUrl, const QString& app, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;

    bool openInTerminal(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

    static DUrl realUrl(const DUrl& url);
    static QString findArchFileRootPath(const DUrl& url);
private:
};

#endif // AVFSFILECONTROLLER_H
