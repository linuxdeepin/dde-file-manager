#ifndef DFMRECENTUTIL_H
#define DFMRECENTUTIL_H

#include "bookmarknode.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/urlroute.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>

const char RECENT_SCHEME[] = "recent";

class RecentUtil
{
    const QString m_recentFilePath;
    QXmlStreamReader m_recentReader;
    QList<BookmarkNode> m_nodes;

public:

    RecentUtil();

    QList<BookmarkNode> loadRecentFile();

    static RecentUtil &instance();
};

#endif // DFMRECENTUTIL_H
