/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef NORMALIZEDMODE_H
#define NORMALIZEDMODE_H

#include "canvasorganizer.h"

DDP_ORGANIZER_BEGIN_NAMESPACE
class NormalizedModePrivate;
class NormalizedMode : public CanvasOrganizer
{
    Q_OBJECT
    friend class NormalizedModePrivate;
public:
   explicit NormalizedMode(QObject *parent = nullptr);
   int mode() const override;
   bool initialize(FileProxyModel *) override;
   void reset() override;
public slots:
   void rebuild();
   void onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl);
   void onFileInserted(const QModelIndex &parent, int first, int last);
   void onFileAboutToBeRemoved(const QModelIndex &parent, int first, int last);
   void onFileDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles );
protected slots:
    bool filterDataRested(QList<QUrl> *urls) override;
    bool filterDataInserted(const QUrl &url) override;
    bool filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl) override;
protected:
    bool setClassifier(Classifier id);
private:
    NormalizedModePrivate *d;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // NORMALIZEDMODE_H
