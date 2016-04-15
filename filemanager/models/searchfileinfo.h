#ifndef SEARCHFILEINFO_H
#define SEARCHFILEINFO_H

#include "fileinfo.h"

class SearchFileInfo : public FileInfo
{
public:
    SearchFileInfo();
    SearchFileInfo(const DUrl &url);

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;

    QIcon fileIcon() const Q_DECL_OVERRIDE;

    DUrl parentUrl() const Q_DECL_OVERRIDE;

private:
    DUrl m_parentUrl;
};

#endif // SEARCHFILEINFO_H
