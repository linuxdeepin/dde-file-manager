// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FAKEPREVIEWPLUGIN_H
#define FAKEPREVIEWPLUGIN_H

#include <dfm-base/interfaces/abstractbasepreview.h>
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

#include <QPointer>
#include <QUrl>
#include <QWidget>

namespace covplugin {

// a preview widget with observable behaviour, used by the loader/factory tests
class CovPreview : public dfmbase::AbstractBasePreview
{
    Q_OBJECT
    // observable from the test binary without a cross-DSO qobject_cast
    // (the plugin DSO has its own staticMetaObject instance)
    Q_PROPERTY(QUrl lastUrl READ fileUrl)
    Q_PROPERTY(int playCount READ playCountValue)
public:
    explicit CovPreview(QObject *parent = nullptr)
        : dfmbase::AbstractBasePreview(parent) {}
    ~CovPreview() override
    {
        // FilePreviewDialog reparents the content widget into its layout and
        // destroys it together with the dialog; the QPointer is null by then,
        // so the dangling deleteLater() (heap use-after-free) is avoided.
        if (content)
            content->deleteLater();
    }

    bool setFileUrl(const QUrl &url) override
    {
        lastUrl = url;
        playCountWhenSet = playCount;
        emit titleChanged();
        return true;
    }
    QUrl fileUrl() const override { return lastUrl; }
    QWidget *contentWidget() const override
    {
        if (!content) {
            content = new QWidget;
            content->setFixedSize(300, 200);
        }
        return content;
    }
    QString title() const override { return QString("cov: ") + lastUrl.fileName(); }
    void play() override { ++playCount; }
    void stop() override { ++stopCount; }

    int playCountValue() const { return playCount; }   // Qt property reader

    int playCount = 0;
    int stopCount = 0;
    int playCountWhenSet = -1;
    QUrl lastUrl;

private:
    mutable QPointer<QWidget> content;
};

// the plugin factory exposed through Q_PLUGIN_METADATA
class CovPreviewPlugin : public dfmbase::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.deepin.filemanager.FilePreviewFactoryInterface_iid" FILE "test_fakepreviewplugin.json")
public:
    explicit CovPreviewPlugin(QObject *parent = nullptr)
        : dfmbase::AbstractFilePreviewPlugin(parent) {}
    dfmbase::AbstractBasePreview *create(const QString &key) override
    {
        Q_UNUSED(key)
        return new CovPreview();
    }
};

}   // namespace covplugin

#endif   // FAKEPREVIEWPLUGIN_H
