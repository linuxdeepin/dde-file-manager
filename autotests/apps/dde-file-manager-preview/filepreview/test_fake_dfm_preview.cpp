// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A stand-in for the real libdfm-preview.so: exports the two entry points the
// preview application resolves via QLibrary and records the calls so tests
// can observe them.
//
// The test binary links this library directly AND PreviewLibrary::load()
// dlopen()s a copied .so (different inode) from its sandbox directory, so the
// process ends up with two instances of these globals. To stay observable
// across both instances every call is additionally appended to a plain text
// file whose path is taken from the COV_FAKE_LOG environment variable.

#include <QList>
#include <QUrl>

#include <cstdio>
#include <cstdlib>

static int g_initCount = 0;
static int g_showCount = 0;
static quint64 g_lastWinId = 0;

static void covLog(const char *tag, quint64 winId)
{
    const char *path = ::getenv("COV_FAKE_LOG");
    if (!path || !*path)
        return;
    FILE *f = ::fopen(path, "a");
    if (!f)
        return;
    ::fprintf(f, "%s %llu\n", tag, static_cast<unsigned long long>(winId));
    ::fclose(f);
}

extern "C" {

__attribute__((visibility("default"))) int initFilePreview()
{
    ++g_initCount;
    covLog("init", 0);
    return 0;
}

__attribute__((visibility("default"))) int showFilePreviewDialog(quint64 windowId,
                                                                 const QList<QUrl> &selecturls,
                                                                 const QList<QUrl> dirUrl)
{
    Q_UNUSED(selecturls)
    Q_UNUSED(dirUrl)
    ++g_showCount;
    g_lastWinId = windowId;
    covLog("show", windowId);
    return 0;
}

__attribute__((visibility("default"))) int covFakeInitCount() { return g_initCount; }
__attribute__((visibility("default"))) int covFakeShowCount() { return g_showCount; }
__attribute__((visibility("default"))) quint64 covFakeLastWinId() { return g_lastWinId; }

}   // extern "C"
