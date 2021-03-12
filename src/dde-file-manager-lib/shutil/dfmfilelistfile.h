/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#pragma once

#include <QIODevice>
#include <QObject>

class DFMFileListFilePrivate;
class DFMFileListFile : public QObject
{
    Q_OBJECT
public:
    enum Status {
        NoError = 0, //!< No error occurred.
        NotExisted,  //!< Not existed when try to read/parse the file.
        AccessError, //!< An access error occurred (e.g. trying to write to a read-only file).
        FormatError  //!< A format error occurred (e.g. loading a malformed desktop entry file).
    };
    Q_ENUM(Status)

public:
    // argument is a path, without filename.
    explicit DFMFileListFile(const QString &dirPath, QObject *parent = nullptr);
    ~DFMFileListFile();

    QString filePath() const;
    QString dirPath() const;

    bool save() const;

    bool contains(const QString &fileName) const;
    void insert(const QString &fileName);
    bool remove(const QString &fileName);
    QSet<QString> getHiddenFiles();

    static bool supportHideByFile(const QString &fileFullPath);
    static bool canHideByFile(const QString &fileFullPath);

public slots:
    bool reload();

private:
    QScopedPointer<DFMFileListFilePrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMFileListFile)
};
