// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMAdditionalMenuPrivate;
class DFMAdditionalMenu : public QObject
{
    Q_OBJECT
public:
    explicit DFMAdditionalMenu(QObject *parent = nullptr);
    ~DFMAdditionalMenu();

    // files.size() == 0 is emptyArea
    QList<QAction *> actions(const QStringList &files, const QString &currentDir = QString(), bool onDesktop = false);
protected:
    void appendParentMineType(const QStringList &parentmimeTypes,  QStringList& mimeTypes);

    void loadDesktopFile();
    
private:

    QScopedPointer<DFMAdditionalMenuPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMAdditionalMenu)

    Q_DISABLE_COPY(DFMAdditionalMenu)
};

DFM_END_NAMESPACE
