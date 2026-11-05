// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDER_CONTENT_PRESENTATION_TEST_H
#define FOLDER_CONTENT_PRESENTATION_TEST_H

#include "folderpresentation.h"

#include <gtest/gtest.h>
#include <QIcon>
#include <QUrl>
#include <memory>

// Shared fixture: one provider instance, presentations created from it the
// same way the dock would (provider->create), parented to the caller.
class PresentationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // The dtk DCI icon provider warns about missing icons when no theme
        // is set; the dock icon theme has the generic icons the QML uses.
        QIcon::setThemeName(QStringLiteral("bloom"));
        if (!m_provider)
            m_provider = std::make_unique<dfm::FolderProvider>();
    }

    dfm::FolderProvider *provider() { return m_provider.get(); }

    dock::FolderPresentation *create(const QString &localPath)
    {
        return m_provider->create(QUrl::fromLocalFile(localPath).toString(QUrl::FullyEncoded), nullptr);
    }

private:
    std::unique_ptr<dfm::FolderProvider> m_provider;
};

#endif  // FOLDER_CONTENT_PRESENTATION_TEST_H
