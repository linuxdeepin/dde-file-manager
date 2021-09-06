/*
 * Copyright (C) 2021 ~ 2016 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
*/

#ifndef DFILEDIALOGPRIVATE_P_H
#define DFILEDIALOGPRIVATE_P_H

#include <QFileDialog>
#include <QModelIndexList>
#include <durl.h>

class QEventLoop;
class DFileView;
class FileDialogStatusBar;

class DFileDialogPrivate
{
public:
    int result = 0;
    bool hideOnAccept = true;
    bool allowMixedSelection = false;

    QFileDialog::FileMode fileMode = QFileDialog::AnyFile;
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    QFileDialog::Options options;
    QEventLoop *eventLoop = Q_NULLPTR;
    QStringList nameFilters;

    DFileView *view = Q_NULLPTR;
    int currentNameFilterIndex = -1;
    QDir::Filters filters = nullptr;
    QString currentInputName;
    mutable QModelIndexList orderedSelectedList;
    FileDialogStatusBar *statusBar;

public:
    QList<DUrl> orderedSelectedUrls() const;
    bool checkFileSuffix(const QString &fileName, const int &filterIndex, QString &suffix) const;
};

#endif // DFILEDIALOGPRIVATE_P_H
