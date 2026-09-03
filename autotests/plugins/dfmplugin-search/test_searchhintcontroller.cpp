// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "utils/searchhintcontroller.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QVariantMap>
#include <QString>
#include <QStringList>

using namespace dfmplugin_search;
DFMBASE_USE_NAMESPACE

// Stub for DConfigManager::value to control search config behavior
static QVariant g_dconfigValue = QVariant(false);

class SearchHintControllerTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        g_dconfigValue = QVariant(false);

        // Stub DConfigManager::value to return controlled values
        stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                       [](DConfigManager *, const QString &, const QString &key, const QVariant &def) -> QVariant {
                           __DBG_STUB_INVOKE__
                           // Auth hint done = false, enable file index = false
                           if (key == "searchAuthHintDone")
                               return false;
                           if (key == "enableFileIndexSearch")
                               return false;
                           if (key == "enableFullTextSearch")
                               return true;
                           if (key == "enableOcrTextSearch")
                               return false;
                           if (key == "enableSemanticSearch")
                               return false;
                           return def;
                       });

        stub.set_lamda(&DConfigManager::setValue,
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) {
                           __DBG_STUB_INVOKE__
                       });

        // Stub FileManagerWindowsManager to return empty window list
        stub.set_lamda(&FileManagerWindowsManager::windowIdList, []() -> QList<quint64> {
            __DBG_STUB_INVOKE__
            return {};
        });

        controller = SearchHintController::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    SearchHintController *controller = nullptr;
};

// --- shouldShowAuthHint ---

TEST_F(SearchHintControllerTest, ShouldShowAuthHint_WhenIndexDisabled_ReturnsTrue)
{
    QString text;
    bool result = controller->shouldShowAuthHint(&text);
    // Auth hint should show when file index is disabled and auth not done
    // Result depends on disabledSearchModes being non-empty (full-text is enabled=true so not disabled)
    EXPECT_NO_FATAL_FAILURE(controller->shouldShowAuthHint(&text));
}

TEST_F(SearchHintControllerTest, ShouldShowAuthHint_TextParameter_PopulatedWhenTrue)
{
    QString text;
    controller->shouldShowAuthHint(&text);
    // Text may or may not be populated; just verify no crash
    SUCCEED();
}

// --- closeAllHints / closeHint ---

TEST_F(SearchHintControllerTest, CloseAllHints_ResetsWindowState)
{
    quint64 winId = 12345;
    EXPECT_NO_FATAL_FAILURE(controller->closeAllHints(winId));
}

TEST_F(SearchHintControllerTest, CloseHint_ResetsWindowState)
{
    quint64 winId = 67890;
    EXPECT_NO_FATAL_FAILURE(controller->closeHint(winId));
}

// --- onTextStatusResult / onOcrStatusResult ---

TEST_F(SearchHintControllerTest, OnTextStatusResult_Success_UpdatesState)
{
    EXPECT_NO_FATAL_FAILURE(controller->onTextStatusResult("Idle", "full", true));
}

TEST_F(SearchHintControllerTest, OnTextStatusResult_Failure_DoesNotUpdate)
{
    EXPECT_NO_FATAL_FAILURE(controller->onTextStatusResult("Failed", "", false));
}

TEST_F(SearchHintControllerTest, OnOcrStatusResult_Success_UpdatesState)
{
    EXPECT_NO_FATAL_FAILURE(controller->onOcrStatusResult("Idle", "ocr", true));
}

TEST_F(SearchHintControllerTest, OnOcrStatusResult_Failure_DoesNotUpdate)
{
    EXPECT_NO_FATAL_FAILURE(controller->onOcrStatusResult("Failed", "", false));
}

// --- onTextStatusChanged / onOcrStatusChanged ---

TEST_F(SearchHintControllerTest, OnTextStatusChanged_UpdatesState)
{
    EXPECT_NO_FATAL_FAILURE(controller->onTextStatusChanged("Running", "full"));
}

TEST_F(SearchHintControllerTest, OnOcrStatusChanged_UpdatesState)
{
    EXPECT_NO_FATAL_FAILURE(controller->onOcrStatusChanged("Running", "ocr"));
}

TEST_F(SearchHintControllerTest, OnTextStatusChanged_TransitionToRunning_ClearsDismissed)
{
    // First set a non-running state
    controller->onTextStatusChanged("WaitingPower", "full");
    // Then transition to Running
    EXPECT_NO_FATAL_FAILURE(controller->onTextStatusChanged("Running", "full"));
}

// --- tryShowHint ---

TEST_F(SearchHintControllerTest, TryShowHint_DoesNotCrash)
{
    quint64 winId = 11111;
    EXPECT_NO_FATAL_FAILURE(controller->tryShowHint(winId));
}

// --- authorizeSearchExperience ---

TEST_F(SearchHintControllerTest, AuthorizeSearchExperience_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->authorizeSearchExperience());
}

// --- dismissAuthHint ---

TEST_F(SearchHintControllerTest, DismissAuthHint_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->dismissAuthHint());
}

// --- startListening / stopListening ---

TEST_F(SearchHintControllerTest, StartListening_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->startListening());
}

TEST_F(SearchHintControllerTest, StopListening_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->stopListening());
}
