#include "dfmbookmarknode.h"

BookmarkNode::Info::Info()
{

}

BookmarkNode::Info::Info(const BookmarkNode::Info &info)
{
    * this = info;
}

bool BookmarkNode::Info::operator ==(const BookmarkNode::Info &info) const
{
    return     meta_owner == info.meta_owner
            && meta_mimetype == info.meta_mimetype
            && mete_application_name == info.mete_application_name
            && mete_application_modified == info.mete_application_modified
            && mete_application_count == info.mete_application_count
            && mete_application_exec == info.mete_application_exec;
}

BookmarkNode::Info &BookmarkNode::Info::operator =(BookmarkNode::Info &&info)
{
    meta_owner = info.meta_owner;
    meta_mimetype = info.meta_mimetype;
    mete_application_name = info.mete_application_name;
    mete_application_modified = info.mete_application_modified;
    mete_application_count = info.mete_application_count;
    mete_application_exec = info.mete_application_exec;
    return *this;
}

bool BookmarkNode::Info::isEmpty()
{
    return     meta_owner.isEmpty()
            && meta_mimetype.isEmpty()
            && mete_application_name.isEmpty()
            && mete_application_modified.isEmpty()
            && mete_application_count.isEmpty()
            && mete_application_exec.isEmpty();
}

BookmarkNode::Info &BookmarkNode::Info::operator =(const BookmarkNode::Info &info)
{
    meta_owner = info.meta_owner;
    meta_mimetype = info.meta_mimetype;
    mete_application_name = info.mete_application_name;
    mete_application_modified = info.mete_application_modified;
    mete_application_count = info.mete_application_count;
    mete_application_exec = info.mete_application_exec;
    return *this;
}

BookmarkNode::BookmarkNode(const BookmarkNode &node)
{
    *this = node;
}

bool BookmarkNode::operator ==(const BookmarkNode &node) const
{
    return     info == node.info
            && visited == node.visited
            && href == node.href
            && modified == node.modified
            && added == node.added;
}

BookmarkNode &BookmarkNode::operator =(BookmarkNode &&node)
{
    info = node.info;
    visited = node.visited;
    href = node.href;
    modified = node.modified;
    added = node.added;
    return *this;
}

bool BookmarkNode::isEmpty()
{
    return     info.isEmpty()
            && visited.isEmpty()
            && href.isEmpty()
            && modified.isEmpty()
            && added.isEmpty();
}

BookmarkNode &BookmarkNode::operator =(const BookmarkNode &node)
{
    info = node.info;
    visited = node.visited;
    href = node.href;
    modified = node.modified;
    added = node.added;
    return *this;
}

QDebug &operator<<(QDebug &out, const BookmarkNode::Info &info)
{
    out << QString(" BookmarkNode::Info(0x%0){ ").arg(qint64(&info),0,16);
    out << "info.meta_owner: " << info.meta_owner << "; "
        << "info.meta_mimetype: " << info.meta_mimetype << "; "
        << "info.mete_application_name: " << info.mete_application_name << "; "
        << "info.mete_application_modified: " << info.mete_application_modified << "; "
        << "info.mete_application_count: " << info.mete_application_count << "; "
        << "info.mete_application_exec: " << info.mete_application_exec;
    out << "}";
    return out;
}

QDebug &operator<<(QDebug &out, const BookmarkNode &node)
{
    out << QString("BookmarkNode(0x%0){ ").arg(qint64(&node),0,16);
    out << "node.visited: " << node.visited << "; "
        << "node.href: " << node.href << "; "
        << "node.modified: " << node.modified << "; "
        << "node.added: " << node.added << "; "
        << "node.info: " << node.info;
    out << "}";
    return out;
}
