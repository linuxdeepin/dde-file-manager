#include "searchfileinfo.h"

#include "../views/dfileview.h"

#include <QIcon>

SearchFileInfo::SearchFileInfo()
    : FileInfo()
{

}

SearchFileInfo::SearchFileInfo(const DUrl &url)
    : FileInfo(url)
{
    const QString &fragment = url.fragment();

    if(!fragment.isEmpty() && fragment != "stop") {
        m_parentUrl = url;
        m_parentUrl.setFragment(QString());
        data->fileInfo.setFile(url.fragment());
        data->url = DUrl::fromLocalFile(data->fileInfo.absoluteFilePath());
    } else {
        data->fileInfo = QFileInfo();
    }
}

bool SearchFileInfo::isCanRename() const
{
    if(m_parentUrl.isEmpty())
        return false;

    return FileInfo::isCanRename();
}

bool SearchFileInfo::isReadable() const
{
    if(m_parentUrl.isEmpty())
        return true;

    return FileInfo::isReadable();
}

bool SearchFileInfo::isWritable() const
{
    if(m_parentUrl.isEmpty())
        return false;

    return FileInfo::isWritable();
}

bool SearchFileInfo::isDir() const
{
    if(m_parentUrl.isEmpty())
        return true;

    return FileInfo::isDir();
}

QIcon SearchFileInfo::fileIcon() const
{
    if(m_parentUrl.isEmpty())
        return QIcon();

    return FileInfo::fileIcon();
}

DUrl SearchFileInfo::parentUrl() const
{
    return m_parentUrl;
}

quint8 SearchFileInfo::supportViewMode() const
{
    if(data->url.isSearchFile())
        return DFileView::ListMode;

    return FileInfo::supportViewMode();
}

int SearchFileInfo::getIndexByFileInfo(getFileInfoFun fun, const AbstractFileInfoPointer &info,
                                       quint8 columnType, Qt::SortOrder order) const
{
    Q_UNUSED(columnType)
    Q_UNUSED(order)

    /// if is file then return -1(insert last)

    if(info->isFile())
        return -1;

    int index = 0;

    forever {
        const AbstractFileInfoPointer tmp_info = fun(index);

        if(!tmp_info)
            break;

        if(tmp_info->isFile()) {
            break;
        }

        ++index;
    }

    return index;
}

quint8 SearchFileInfo::userColumnCount() const
{
    return 1;
}

QVariant SearchFileInfo::userColumnDisplayName(quint8 userColumnType) const
{
    Q_UNUSED(userColumnType)

    return QObject::tr("absolute path");
}

QVariant SearchFileInfo::userColumnData(quint8 userColumnType) const
{
    Q_UNUSED(userColumnType)

    return absoluteFilePath();
}
