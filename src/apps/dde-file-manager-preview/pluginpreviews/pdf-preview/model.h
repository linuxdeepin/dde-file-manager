// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
        kNoError = 0,
        kNeedPassword,   //需要密码
        kWrongPassword,   //密码错误
        kFileError,   //打开失败
        kFileDamaged,   //打开成功 文件损坏
        kConvertFailed   //转换失败
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
