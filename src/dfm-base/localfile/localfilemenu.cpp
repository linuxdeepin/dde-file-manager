#include "localfilemenu.h"
#include "dfm-base/base/urlroute.h"

#include <QUrl>
#include <QDir>

DFMBASE_BEGIN_NAMESPACE

LocalFileMenu::LocalFileMenu(QObject *parent)
    :AbstractFileMenu(parent)
{

}

QMenu *LocalFileMenu::build(AbstractFileMenu::MenuMode mode,
                            const QUrl &rootUrl, const QUrl &foucsUrl,
                            const QList<QUrl> &selected)
{
    QString path = UrlRoute::urlToPath(rootUrl);
    if (!QDir(path).exists())
        return nullptr;
    return AbstractFileMenu::build(mode, rootUrl, foucsUrl, selected);
}

DFMBASE_END_NAMESPACE
