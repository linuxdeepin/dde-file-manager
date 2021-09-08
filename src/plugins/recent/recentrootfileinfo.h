#ifndef RECENTROOTFILEINFO_H
#define RECENTROOTFILEINFO_H

#include "dfm-base/base/abstractfileinfo.h"

class RecentRootFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
public:
    explicit RecentRootFileInfo(const QUrl &url)
        : DFMBASE_NAMESPACE::AbstractFileInfo(url)
    {
        Q_UNUSED(url);
    }
};

#endif // RECENTROOTFILEINFO_H
