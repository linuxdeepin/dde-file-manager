/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef FILECALCULATIONUTILS_H
#define FILECALCULATIONUTILS_H

#include "dfmplugin_filepreview_global.h"

#include <QObject>
#include <QList>

DPFILEPREVIEW_BEGIN_NAMESPACE
class CalculationUtils : public QObject
{
    Q_OBJECT
public:
    struct FileStatisticInfo
    {
        qint64 totalSize { 0 };
        qint32 fileCount { 0 };
    };

    explicit CalculationUtils(QObject *parent = nullptr);

    virtual ~CalculationUtils() override;

    void startThread(const QList<QUrl> &files);

signals:
    void sigFileChange(qint32 count);

    void sigTotalChange(qint64 totalSize);

    void sigStartRun(const QList<QUrl> &files);

public slots:
    void statisticsFileInfo(const QList<QUrl> &files);

private:
    quint16 getMemoryPageSize();
    void statisticFilesSize(const QUrl &url, FileStatisticInfo *&sizeInfo);

private:
    QThread *thread { nullptr };
    FileStatisticInfo *info { nullptr };
};
DPFILEPREVIEW_END_NAMESPACE
#endif   // FILECALCULATIONUTILS_H
