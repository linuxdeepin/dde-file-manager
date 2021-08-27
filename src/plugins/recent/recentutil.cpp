#include "recentutil.h"

Q_GLOBAL_STATIC(RecentUtil, _recentUntil)

RecentUtil::RecentUtil()
    : m_recentFilePath(DFMStandardPaths::location(DFMStandardPaths::HomePath)
                       + "/.local/share/recently-used.xbel")
{

}

QList<BookmarkNode> RecentUtil::loadRecentFile() {

    QList<BookmarkNode> nodes;
    QFile file(m_recentFilePath);

    if (!file.open(QIODevice::ReadOnly)) {
        qInfo() << "Failed open recent file" << m_recentFilePath;
    } else {

        BookmarkNode node;
        m_recentReader.setDevice(&file);
        while (!m_recentReader.atEnd()) {

            if (!m_recentReader.readNextStartElement() ||
                    m_recentReader.name() != "bookmark") {
                continue;
            }

            node.href = m_recentReader.attributes().value("href").toUtf8();
            node.visited = m_recentReader.attributes().value("visited").toUtf8();
            node.modified = m_recentReader.attributes().value("modified").toUtf8();
            node.added = m_recentReader.attributes().value("added").toUtf8();

            m_recentReader.readNext();
            if (m_recentReader.name() == "metadata")
                node.info.meta_owner = m_recentReader.attributes().value("owner").toUtf8();

            if (m_recentReader.name() == "mime:mime-type")
                node.info.meta_mimetype = m_recentReader.attributes().value("type").toUtf8();

            if (m_recentReader.name() == "bookmark:application") {
                continue;
            }
            node.info.mete_application_name = m_recentReader.attributes().value("name").toUtf8();
            node.info.mete_application_modified = m_recentReader.attributes().value("modified").toUtf8();
            node.info.mete_application_count = m_recentReader.attributes().value("count").toUtf8();
            node.info.mete_application_exec = m_recentReader.attributes().value("exec").toUtf8();
        }

        if (!node.isEmpty()) nodes << node;

        file.close();
    }
    return nodes;
}

RecentUtil &RecentUtil::instance()
{
    return *_recentUntil;
}
