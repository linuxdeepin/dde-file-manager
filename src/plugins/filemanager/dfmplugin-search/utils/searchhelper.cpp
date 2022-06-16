/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "searchhelper.h"
#include "topwidget/advancesearchbar.h"

#include "services/filemanager/workspace/workspace_defines.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/dpf.h>

#include <QUrlQuery>

DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

static inline QString parseDecodedComponent(const QString &data)
{
    return QString(data).replace(QLatin1Char('%'), QStringLiteral("%25"));
}

SearchHelper *SearchHelper::instance()
{
    static SearchHelper ins;
    return &ins;
}

QUrl SearchHelper::rootUrl()
{
    return fromSearchFile("/");
}

bool SearchHelper::isRootUrl(const QUrl &url)
{
    return url.path() == rootUrl().path();
}

bool SearchHelper::isSearchFile(const QUrl &url)
{
    return url.scheme() == scheme();
}

QUrl SearchHelper::searchTargetUrl(const QUrl &searchUrl)
{
    QUrlQuery query(searchUrl.query());
    return QUrl(query.queryItemValue("url", QUrl::FullyDecoded));
}

QString SearchHelper::searchKeyword(const QUrl &searchUrl)
{
    QUrlQuery query(searchUrl.query());
    return query.queryItemValue("keyword", QUrl::FullyDecoded);
}

QString SearchHelper::searchTaskId(const QUrl &searchUrl)
{
    QUrlQuery query(searchUrl.query());
    return query.queryItemValue("taskId", QUrl::FullyDecoded);
}

QUrl SearchHelper::setSearchKeyword(const QUrl &searchUrl, const QString &keyword)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("keyword");
    query.addQueryItem("keyword", parseDecodedComponent(keyword));
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::setSearchTargetUrl(const QUrl &searchUrl, const QUrl &targetUrl)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("url");
    query.addQueryItem("url", parseDecodedComponent(targetUrl.toString()));
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::setSearchTaskId(const QUrl &searchUrl, const QString &taskId)
{
    QUrl url(searchUrl);
    QUrlQuery query(url.query());
    query.removeQueryItem("taskId");
    query.addQueryItem("taskId", taskId);
    url.setQuery(query);

    return url;
}

QUrl SearchHelper::fromSearchFile(const QString &filePath)
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath(filePath);

    return url;
}

QUrl SearchHelper::fromSearchFile(const QUrl &targetUrl, const QString &keyword, const QString &taskId)
{
    QUrl url = rootUrl();
    QUrlQuery query;

    query.addQueryItem("url", parseDecodedComponent(targetUrl.toString()));
    query.addQueryItem("keyword", parseDecodedComponent(keyword));
    query.addQueryItem("taskId", taskId);
    url.setQuery(query);

    return url;
}

bool SearchHelper::showTopWidget(QWidget *w, const QUrl &url)
{
    auto topWidget = qobject_cast<AdvanceSearchBar *>(w);
    if (!topWidget)
        return false;

    bool visible = w->isVisible();
    if (visible)
        topWidget->refreshOptions(url);

    return visible;
}

bool SearchHelper::customColumnRole(const QUrl &rootUrl, QList<ItemRoles> *roleList)
{
    if (rootUrl.scheme() != scheme())
        return false;

    const QUrl &targetUrl = searchTargetUrl(rootUrl);
    if (!dpfHookSequence->run("dfmplugin_workspace", "hook_FetchCustomColumnRoles", targetUrl, roleList)) {
        roleList->append(kItemNameRole);
        roleList->append(kItemFilePathRole);
        roleList->append(kItemFileLastModifiedRole);
        roleList->append(kItemFileSizeRole);
        roleList->append(kItemFileMimeTypeRole);
    }

    return true;
}

bool SearchHelper::customRoleDisplayName(const QUrl &rootUrl, const ItemRoles role, QString *displayName)
{
    if (rootUrl.scheme() != scheme())
        return false;

    const QUrl &targetUrl = searchTargetUrl(rootUrl);
    if (dpfHookSequence->run("dfmplugin_workspace", "hook_FetchCustomRoleDisplayName", targetUrl, role, displayName))
        return true;

    if (role == kItemFilePathRole) {
        displayName->append(tr("Path"));
        return true;
    }

    return false;
}

bool SearchHelper::customRoleData(const QUrl &rootUrl, const QUrl &url, const ItemRoles role, QVariant *data)
{
    if (rootUrl.scheme() != scheme())
        return false;

    const QUrl &targetUrl = searchTargetUrl(rootUrl);
    if (dpfHookSequence->run("dfmplugin_workspace", "hook_FetchCustomRoleData", targetUrl, url, role, data))
        return true;

    if (role == kItemFilePathRole) {
        auto info = InfoFactory::create<AbstractFileInfo>(url);
        if (info) {
            data->setValue(info->fileDisplayPath());
            return true;
        }
    }

    return false;
}

bool SearchHelper::blockPaste(quint64 winId, const QUrl &to)
{
    Q_UNUSED(winId)

    if (to.scheme() == SearchHelper::scheme()) {
        qDebug() << "The search root directory does not support paste!";
        return true;
    }
    return false;
}

SearchHelper::SearchHelper(QObject *parent)
    : QObject(parent)
{
}

SearchHelper::~SearchHelper()
{
}

DPSEARCH_END_NAMESPACE
