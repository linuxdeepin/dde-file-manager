#ifndef DFMRECENTUTIL_H
#define DFMRECENTUTIL_H

#include "dfmbookmarknode.h"

#include "base/dfmstandardpaths.h"
#include "base/dfmurlroute.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>

const char RECENT_SCHEME[] = "recent";

class DFMRecentUtil
{
    const QString m_recentFilePath;
    QXmlStreamReader m_recentReader;
    QList<BookmarkNode> m_nodes;

public:

    DFMRecentUtil();

    QList<BookmarkNode> loadRecentFile();

    static DFMRecentUtil &instance();
};

#endif // DFMRECENTUTIL_H
