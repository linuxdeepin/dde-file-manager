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

#ifndef MODEL_H
#define MODEL_H
#include "preview_plugin_global.h"

#include "global.h"

#include <QObject>
#include <QRect>

namespace plugin_filepreview {
class DPdfAnnot;
class FormField : public QObject
{
    Q_OBJECT
public:
    FormField()
        : QObject() {}
    virtual ~FormField() {}
    virtual QRectF boundary() const = 0;
    virtual QString name() const = 0;
    virtual QWidget *createWidget() = 0;
signals:
    void wasModified();
};

class Page : public QObject
{
    Q_OBJECT
public:
    Page()
        : QObject()
    {
    }
    virtual ~Page() {}

    virtual QSizeF sizeF() const = 0;
    virtual QImage render(int width, int height, const QRect &slice = QRect()) const = 0;
};

class Document : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError = 0,
        NeedPassword,   //需要密码
        WrongPassword,   //密码错误
        FileError,   //打开失败
        FileDamaged,   //打开成功 文件损坏
        ConvertFailed   //转换失败
    };

    Document()
        : QObject() {}
    virtual ~Document() {}
    virtual int pageCount() const = 0;
    virtual Page *page(int index) const = 0;
};

class DocumentFactory
{
public:
    static Document *getDocument(const int &fileType, const QString &filePath, const QString &password, Document::Error &error);
};
}
#endif   // MODEL_H
