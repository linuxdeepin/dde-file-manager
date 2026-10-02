// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROFDRENDERER_H
#define ROFDRENDERER_H

#include <dfm-base/dfm_base_global.h>

#include <QImage>
#include <QScopedPointer>
#include <QString>

DFMBASE_BEGIN_NAMESPACE

/**
 * @brief OFD (GB/T 33190) page renderer based on librofd_ffi + cairo.
 *
 * Each instance owns its own document and renderer handles, so separate
 * instances are safe to use from different threads (e.g. thumbnail workers
 * and the preview render thread). Not copyable.
 */
class RofdRenderer
{
public:
    RofdRenderer();
    ~RofdRenderer();

    RofdRenderer(const RofdRenderer &) = delete;
    RofdRenderer &operator=(const RofdRenderer &) = delete;

    bool isValid() const;

    /**
     * @brief Open an OFD document. Any previously opened document is closed.
     * @return false if the file cannot be parsed as OFD.
     */
    bool openDocument(const QString &filePath);

    /**
     * @brief Total page count of the opened document (0 if not open).
     */
    int pageCount() const;

    /**
     * @brief Render one page, fitting the full page into @p width pixels
     * (height follows the page aspect ratio). Returns a null image on failure.
     */
    QImage renderPage(int pageIndex, int width);

private:
    void close();

    class Private;
    QScopedPointer<Private> d;
};

DFMBASE_END_NAMESPACE

#endif   // ROFDRENDERER_H
