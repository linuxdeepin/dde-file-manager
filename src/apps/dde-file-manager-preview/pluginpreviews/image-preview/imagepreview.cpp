// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagepreview.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/fileutils.h>
#include "imageview.h"

#include <QImageReader>
#include <QProcess>
#include <QMimeType>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>
#include <QMimeData>

DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;

ImagePreview::ImagePreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    fmInfo() << "Image preview: ImagePreview instance created";
}

ImagePreview::~ImagePreview()
{
    fmInfo() << "Image preview: ImagePreview instance destroyed";

    if (imageView)
        imageView->deleteLater();

    if (messageStatusBar)
        messageStatusBar->deleteLater();
}

bool ImagePreview::canPreview(const QUrl &url, QByteArray *format) const
{
    fmDebug() << "Image preview: checking if can preview:" << url;

    QByteArray f = QImageReader::imageFormat(url.toLocalFile());

    if (f.isEmpty()) {
        fmDebug() << "Image preview: QImageReader could not detect format, trying MIME database";
        DFMBASE_NAMESPACE::DMimeDatabase mimeDatabase;

        const QMimeType &mt = mimeDatabase.mimeTypeForFile(url, QMimeDatabase::MatchContent);

        f = mt.preferredSuffix().toLatin1();

        if (f.isEmpty()) {
            fmWarning() << "Image preview: no format detected for file:" << url;
            if (format) {
                *format = f;
            }
            return false;
        }
    }

    if (format) {
        *format = f;
    }

    bool supported = QImageReader::supportedImageFormats().contains(f);
    fmDebug() << "Image preview: format" << f << "supported:" << supported << "for file:" << url;
    return supported;
}

void ImagePreview::initialize(QWidget *window, QWidget *statusBar)
{
    Q_UNUSED(window)

    fmDebug() << "Image preview: initializing with status bar";

    messageStatusBar = new QLabel(statusBar);
    messageStatusBar->setStyleSheet("QLabel{font-family: Helvetica;\
                                   font-size: 12px;\
                                   font-weight: 300;}");
    // Expanding 水平策略：让 messageStatusBar 在 QHBoxLayout 中抢占 previewTitle 与 openBtn
    // 之间的多余空间，配合 AlignCenter 使分辨率文本在该空间内居中——previewTitle 短时
    // 接近 statusBar 几何中心，previewTitle 长时多余空间被压缩，标签自然回缩避让。
    messageStatusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    messageStatusBar->setAlignment(Qt::AlignCenter);

    messageStatusBar->show();

    // 分辨率标签通过 statusBarWidget() 交给 FilePreviewDialog 的 QHBoxLayout 统一管理，
    // 不再用 DAnchors 绝对居中——避免与 previewTitle 在 statusBar 中心区域重叠。

    fmDebug() << "Image preview: status bar initialized";
}

bool ImagePreview::setFileUrl(const QUrl &url)
{
    fmInfo() << "Image preview: setting file URL:" << url;

    if (currentFileUrl == url) {
        fmDebug() << "Image preview: URL unchanged, skipping:" << url;
        return true;
    }

    QUrl tmpUrl = UrlRoute::fromLocalFile(url.path());
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (info.isNull()) {
        fmWarning() << "Image preview: failed to create FileInfo for:" << url;
        return false;
    }

    if (info->canAttributes(CanableInfoType::kCanRedirectionFileUrl)) {
        tmpUrl = info->urlOf(UrlInfoType::kRedirectedFileUrl);
        fmDebug() << "Image preview: using redirected URL:" << tmpUrl;
    }

    if (!tmpUrl.isLocalFile()) {
        fmWarning() << "Image preview: URL is not a local file:" << tmpUrl;
        return false;
    }

    QByteArray format;
    if (!canPreview(tmpUrl, &format)) {
        fmWarning() << "Image preview: cannot preview file:" << tmpUrl << "format:" << format;
        return false;
    }

    currentFileUrl = tmpUrl;

    if (!imageView) {
        fmDebug() << "Image preview: creating new ImageView for:" << tmpUrl.toLocalFile() << "format:" << format;
        imageView = new ImageView(tmpUrl.toLocalFile(), format);
    } else {
        fmDebug() << "Image preview: updating existing ImageView with:" << tmpUrl.toLocalFile() << "format:" << format;
        imageView->setFile(tmpUrl.toLocalFile(), format);
    }

    const QSize &image_size = imageView->sourceSize();
    fmDebug() << "Image preview: image size:" << image_size;

    messageStatusBar->setText(QString("%1x%2").arg(image_size.width()).arg(image_size.height()));
    messageStatusBar->adjustSize();

    imageTitle = QFileInfo(tmpUrl.toLocalFile()).fileName();

    fmInfo() << "Image preview: file URL set successfully:" << url << "title:" << imageTitle;
    Q_EMIT titleChanged();

    return true;
}

QUrl ImagePreview::fileUrl() const
{
    return currentFileUrl;
}

QWidget *ImagePreview::contentWidget() const
{
    return imageView;
}

QString ImagePreview::title() const
{
    return imageTitle;
}

QWidget *ImagePreview::statusBarWidget() const
{
    return messageStatusBar;
}

Qt::Alignment ImagePreview::statusBarWidgetAlignment() const
{
    // 返回 0（默认）让 widget 按 sizePolicy 参与 QHBoxLayout 的空间分配——
    // 配合 messageStatusBar 的 Expanding 策略使其占满 previewTitle 与 openBtn 之间的多余空间。
    // 文本居中由 QLabel::setAlignment(AlignCenter) 负责，不在此处设 alignment 以免抑制 Expanding。
    return Qt::Alignment();
}
