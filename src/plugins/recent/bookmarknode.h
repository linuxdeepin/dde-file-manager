#ifndef DFMBOOKMARKNODE_H
#define DFMBOOKMARKNODE_H

#include <QString>
#include <QDebug>

//<bookmark visited="2021-07-20T14:01:48Z"
//    href="file:///home/funning/Desktop/%E6%88%AA%E5%9B%BE%E5%BD%95%E5%B1%8F_%E9%80%89%E6%8B%A9%E5%8C%BA%E5%9F%9F_20210720220107.gif"
//    modified="2021-07-20T14:01:48Z"
//    added="2021-07-20T14:01:47Z">
//<info>
//  <metadata owner="http://freedesktop.org">
//      <mime:mime-type type="image/gif"/>
//      <bookmark:applications>
//          <bookmark:application name="" modified="2021-07-20T14:01:48Z" count="2" exec="deepin-image-viewer --new-window %F"/>
//      </bookmark:applications>
//  </metadata>
//</info>
//</bookmark>

//关联结构体
class BookmarkNode :public QSharedData
{
    friend class DFMRecentUtil;
    friend QDebug& operator<<(QDebug &out, const BookmarkNode& node);

public:
    class Info
    {
        friend class RecentUtil;
        friend class BookmarkNode;
        friend QDebug& operator<<(QDebug &out, const Info& info);

        QString meta_owner;
        QString meta_mimetype;
        QString mete_application_name;
        QString mete_application_modified;
        QString mete_application_count;
        QString mete_application_exec;

    public:
        explicit Info();
        explicit Info(const Info &info);
        bool operator == (const Info &info) const;
        Info &operator = (const Info &info);
        Info &operator = (Info &&info);
        bool isEmpty();
    };

    BookmarkNode::Info info;
    QString visited;
    QString href;
    QString modified;
    QString added;

    BookmarkNode(){}
    BookmarkNode(const BookmarkNode &node);
    bool operator == (const BookmarkNode &node) const;
    BookmarkNode &operator = (const BookmarkNode &node);
    BookmarkNode &operator = (BookmarkNode &&node);
    bool isEmpty();
};

QDebug& operator<<(QDebug &out, const BookmarkNode::Info& info);
QDebug& operator<<(QDebug &out, const BookmarkNode& node);

Q_DECLARE_METATYPE(BookmarkNode)
Q_DECLARE_METATYPE(BookmarkNode::Info)

#endif // DFMBOOKMARKNODE_H
