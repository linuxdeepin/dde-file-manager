// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/utils/rofd/rofdrenderer.h>

#include <rofd.h>

#include <cairo.h>

#include <QDebug>

#include <cmath>

DFMBASE_USE_NAMESPACE

static constexpr qreal kMillimetresPerInch = 25.4;

class RofdRenderer::Private
{
public:
    rofd_document_t *document = nullptr;
    rofd_renderer_t *renderer = nullptr;
};

RofdRenderer::RofdRenderer()
    : d(new Private)
{
}

RofdRenderer::~RofdRenderer()
{
    close();
}

void RofdRenderer::close()
{
    if (d->renderer) {
        rofd_renderer_free(d->renderer);
        d->renderer = nullptr;
    }
    if (d->document) {
        rofd_document_free(d->document);
        d->document = nullptr;
    }
}

bool RofdRenderer::isValid() const
{
    return d->document != nullptr;
}

bool RofdRenderer::openDocument(const QString &filePath)
{
    close();

    rofd_error_t *error = nullptr;
    rofd_load_options_t loadOptions;
    rofd_load_options_init(&loadOptions, sizeof(loadOptions));
    if (ROFD_STATUS_OK != rofd_document_open(filePath.toUtf8().constData(), &loadOptions,
                                             &d->document, &error)) {
        qCWarning(logDFMBase) << "rofd: cannot open document:" << filePath
                              << "-" << (error ? rofd_error_get_message(error) : "unknown error");
        rofd_error_free(error);
        return false;
    }

    rofd_renderer_options_t rendererOptions;
    rofd_renderer_options_init(&rendererOptions, sizeof(rendererOptions));
    if (ROFD_STATUS_OK != rofd_renderer_new(&rendererOptions, &d->renderer, &error)) {
        qCWarning(logDFMBase) << "rofd: cannot create renderer:" << filePath
                              << "-" << (error ? rofd_error_get_message(error) : "unknown error");
        rofd_error_free(error);
        close();
        return false;
    }
    return true;
}

int RofdRenderer::pageCount() const
{
    size_t count = 0;
    rofd_error_t *error = nullptr;
    if (!d->document
        || ROFD_STATUS_OK != rofd_document_get_page_count(d->document, &count, &error)) {
        rofd_error_free(error);
        return 0;
    }
    return static_cast<int>(count);
}

QImage RofdRenderer::renderPage(int pageIndex, int width)
{
    if (!d->document || !d->renderer || pageIndex < 0 || width <= 0) {
        qCWarning(logDFMBase) << "rofd: invalid render request, page:" << pageIndex << "width:" << width;
        return QImage();
    }

    rofd_error_t *error = nullptr;
    rofd_page_t *page = nullptr;
    if (ROFD_STATUS_OK != rofd_document_get_page(d->document, static_cast<size_t>(pageIndex), &page, &error)
        || !page) {
        qCWarning(logDFMBase) << "rofd: cannot get page" << pageIndex << ":"
                              << (error ? rofd_error_get_message(error) : "unknown error");
        rofd_error_free(error);
        return QImage();
    }

    rofd_rect_t pageRect = { 0.0, 0.0, 0.0, 0.0 };
    if (ROFD_STATUS_OK != rofd_page_get_size_mm(page, &pageRect, nullptr) || pageRect.width_mm <= 0.0) {
        qCWarning(logDFMBase) << "rofd: cannot query page size, page:" << pageIndex;
        rofd_page_free(page);
        return QImage();
    }

    // rofd renders with a single millimetre-to-pixel scale, width is the
    // target full-page width in pixels.
    rofd_render_options_t options;
    rofd_render_options_init(&options, sizeof(options));
    options.dpi = (static_cast<double>(width) / pageRect.width_mm) * kMillimetresPerInch;
    options.scale = 1.0;

    int32_t pixelWidth = 0;
    int32_t pixelHeight = 0;
    if (ROFD_STATUS_OK != rofd_renderer_get_pixel_size(d->renderer, page, &options,
                                                       &pixelWidth, &pixelHeight, &error)
        || pixelWidth <= 0 || pixelHeight <= 0) {
        qCWarning(logDFMBase) << "rofd: cannot compute pixel size, page:" << pageIndex << ":"
                              << (error ? rofd_error_get_message(error) : "unknown error");
        rofd_error_free(error);
        rofd_page_free(page);
        return QImage();
    }

    const int argbStride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, pixelWidth);
    const int maskStride = cairo_format_stride_for_width(CAIRO_FORMAT_A8, pixelWidth);
    if (pixelWidth > 32767 || pixelHeight > 32767 || argbStride < 0 || maskStride < 0
        || (static_cast<quint64>(argbStride) * 2 + static_cast<quint64>(maskStride))
                * static_cast<quint64>(pixelHeight)
            > options.max_raster_bytes) {
        qCWarning(logDFMBase) << "rofd: render target exceeds raster limits:" << pixelWidth << "x" << pixelHeight;
        rofd_page_free(page);
        return QImage();
    }

    QImage image(QSize(pixelWidth, pixelHeight), QImage::Format_ARGB32_Premultiplied);
    if (image.isNull()) {
        qCWarning(logDFMBase) << "rofd: cannot allocate render image:" << pixelWidth << "x" << pixelHeight;
        rofd_page_free(page);
        return QImage();
    }
    image.fill(Qt::white);

    cairo_surface_t *surface = cairo_image_surface_create_for_data(image.bits(),
                                                                   CAIRO_FORMAT_ARGB32,
                                                                   image.width(),
                                                                   image.height(),
                                                                   image.bytesPerLine());
    cairo_t *cr = CAIRO_STATUS_SUCCESS == cairo_surface_status(surface) ? cairo_create(surface) : nullptr;
    if (!cr || CAIRO_STATUS_SUCCESS != cairo_status(cr)) {
        qCWarning(logDFMBase) << "rofd: cannot create cairo surface/context for rendering";
        if (cr)
            cairo_destroy(cr);
        cairo_surface_destroy(surface);
        rofd_page_free(page);
        return QImage();
    }

    rofd_render_report_t *report = nullptr;
    rofd_status_t status = rofd_renderer_render_page_cairo(d->renderer, page, cr, &options,
                                                           &report, &error);
    cairo_surface_flush(surface);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    if (report) {
        rofd_render_report_free(report);
    }
    rofd_page_free(page);

    if (status != ROFD_STATUS_OK) {
        qCWarning(logDFMBase) << "rofd: page render failed, status:" << status << ":"
                              << (error ? rofd_error_get_message(error) : "unknown error");
        rofd_error_free(error);
        return QImage();
    }
    rofd_error_free(error);
    return image;
}
