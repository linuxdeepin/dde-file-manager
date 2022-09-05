// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
