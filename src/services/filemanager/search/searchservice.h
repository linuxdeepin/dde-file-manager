/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef SEARCHSERVICE_H
#define SEARCHSERVICE_H

#include <dfm-framework/service/pluginservicecontext.h>

class SearchServicePrivate;
class SearchService final : public dpf::PluginService, dpf::AutoServiceRegister<SearchService>
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.SearchService";
    }

    QStringList fulltextSearch(quint64 winId, const QString &path, const QString &keyword);
    bool createFullTextIndex(const QString &path);
    void updateFullTextIndex(quint64 winId, const QString &path);
    bool hasUpdated(quint64 winId);

    QStringList fileSearch(quint64 winId, const QString &path, const QString &keyword, bool useRegExp = true);
    QStringList fileSearch(quint64 winId, qint32 maxCount, qint64 maxTime, quint32 &startOffset, quint32 &endOffset,
                           const QString &path, const QString &keyword, bool useRegExp = true);

    void stop(quint64 winId);

private:
    explicit SearchService(QObject *parent = nullptr);
    ~SearchService();

    QScopedPointer<SearchServicePrivate> d;
};

#endif   // SEARCHSERVICE_H
