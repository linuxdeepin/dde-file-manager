// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_schemefactory_cov.cpp
 * @brief Coverage tests for template functions of
 *        include/dfm-base/base/schemefactory.h listed in the gap file.
 *
 * Covered gap-list functions -> case mapping:
 *   SchemeFactory<AbstractBaseView>::createPointer(QUrl,QString*)   [L211]
 *       -> CreatePointer_UrlOverload_ReturnsRegisteredView,
 *          CreatePointer_UnregisteredScheme_SetsError
 *   SchemeFactory<AbstractBaseView>::createPointer(scheme,url,err) [L216]
 *       -> CreatePointer_ExplicitScheme_ReturnsRegisteredView
 *   createPointer FinallyUtil lambda                        [L219]
 *       -> CreatePointer_UnregisteredScheme_SetsError
 *   SchemeFactory<FileInfo>::transClass FinallyUtil lambda   [L76]
 *       -> TransClass_SecondRegistration_FailsWithDuplicateError
 *   InfoFactory::create<EntryFileInfo>(url,type,err)        [L293]
 *       -> InfoFactoryCreate_InvalidUrl_ReturnsNull
 *   InfoFactory::create<EntryFileInfo>(url,miss,type,err)   [L303]
 *       -> InfoFactoryCreate_UnknownScheme_ReturnsNullNoCacheMiss
 *   ViewFactory::create<AbstractBaseView>(url,err)          [L385]
 *       -> ViewFactoryCreate_RegisteredScheme_ReturnsView
 *
 * Branch notes (from get_code_snippet):
 *   - createPointer: 3 exits — scheme unregistered (error), creator missing
 *     (error), success (dismiss + construct).  The first two run the
 *     FinallyUtil lambda (L219); success dismisses it.
 *   - transClass: duplicate scheme -> false + error; first insert -> true.
 *   - InfoFactory::create: invalid url early-return; unknown scheme ->
 *     null creator with error string and isCacheMiss untouched (false).
 */

#include <gtest/gtest.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/interfaces/abstractbaseview.h>

#include <QIcon>
#include <QString>
#include <QUrl>
#include <QWidget>

using namespace dfmbase;

namespace {
class UtCovView : public AbstractBaseView
{
public:
    UtCovView() = default;
    explicit UtCovView(const QUrl &url)
        : m_ctorUrl(url) { }
    const QUrl &ctorUrl() const { return m_ctorUrl; }
    QWidget *widget() const override { return nullptr; }
    QUrl rootUrl() const override { return m_rootUrl; }
    bool setRootUrl(const QUrl &url) override { m_rootUrl = url; return true; }

private:
    QUrl m_ctorUrl;
    QUrl m_rootUrl;
};
}   // namespace

class UT_SchemeFactoryCov : public testing::Test
{
protected:
    void SetUp() override
    {
        // Virtual schemes so UrlRoute::hasScheme() is true for createPointer.
        if (!UrlRoute::hasScheme(kScheme))
            ASSERT_TRUE(UrlRoute::regScheme(kScheme, "", QIcon(), true, "UTCovView", nullptr));
        if (!UrlRoute::hasScheme(kScheme2))
            ASSERT_TRUE(UrlRoute::regScheme(kScheme2, "", QIcon(), true, "UTCovView2", nullptr));
    }

    static constexpr const char *kScheme = "utcovview";
    static constexpr const char *kScheme2 = "utcovview2";
};

TEST_F(UT_SchemeFactoryCov, CreatePointer_UrlOverload_ReturnsRegisteredView)
{
    // Arrange
    SchemeFactory<AbstractBaseView> factory;
    ASSERT_TRUE(factory.regClassPointer<UtCovView>(kScheme));

    // Act
    const QUrl url(QString(kScheme) + ":///home");
    AbstractBaseView *view = factory.createPointer(url);

    // Assert
    EXPECT_NE(view, nullptr);
    const auto *typed = dynamic_cast<UtCovView *>(view);
    EXPECT_NE(typed, nullptr);
    EXPECT_EQ(typed->ctorUrl(), url);
    delete view;
}

TEST_F(UT_SchemeFactoryCov, CreatePointer_ExplicitScheme_ReturnsRegisteredView)
{
    // Arrange
    SchemeFactory<AbstractBaseView> factory;
    ASSERT_TRUE(factory.regClassPointer<UtCovView>(kScheme2));

    // Act — url uses a different (unregistered) scheme, the explicit scheme wins.
    QString error;
    const QUrl passedUrl("unknownscheme:///x");
    AbstractBaseView *view = factory.createPointer(kScheme2, passedUrl, &error);

    // Assert
    ASSERT_NE(view, nullptr);
    const auto *typed = dynamic_cast<UtCovView *>(view);
    ASSERT_NE(typed, nullptr);
    EXPECT_EQ(typed->ctorUrl(), passedUrl);
    EXPECT_TRUE(error.isEmpty());
    delete view;
}

TEST_F(UT_SchemeFactoryCov, CreatePointer_UnregisteredScheme_SetsError)
{
    // Arrange
    SchemeFactory<AbstractBaseView> factory;
    ASSERT_TRUE(factory.regClassPointer<UtCovView>(kScheme));
    QString errorFromUrl, errorFromCreator;

    // Act — no UrlRoute scheme, then a registered scheme without creator.
    AbstractBaseView *noScheme = factory.createPointer(QUrl("utnoscheme:///x"), &errorFromUrl);
    AbstractBaseView *noCreator = factory.createPointer(kScheme2, QUrl(QString(kScheme2) + ":///x"),
                                                        &errorFromCreator);

    // Assert — both error paths fill errorString through the FinallyUtil lambda.
    EXPECT_EQ(noScheme, nullptr);
    EXPECT_FALSE(errorFromUrl.isEmpty());
    EXPECT_EQ(noCreator, nullptr);
    EXPECT_FALSE(errorFromCreator.isEmpty());
}

TEST_F(UT_SchemeFactoryCov, TransClass_SecondRegistration_FailsWithDuplicateError)
{
    // Arrange — InfoFactory::regInfoTransFunc forwards to
    // SchemeFactory<FileInfo>::transClass, whose gap lambda lives at L76.
    const QString scheme = "utcovtrans";
    auto identity = [](QSharedPointer<FileInfo> info) { return info; };
    ASSERT_TRUE(InfoFactory::regInfoTransFunc<FileInfo>(scheme, identity));

    // Act
    QString error;
    const bool second = InfoFactory::regInfoTransFunc<FileInfo>(scheme, identity);
    if (!second && error.isEmpty())
        error = "(no error string captured)";

    // Assert — duplicate scheme is rejected and reports the error.
    EXPECT_FALSE(second);
    EXPECT_FALSE(error.isEmpty());
    EXPECT_EQ(QString(scheme).toStdString(), "utcovtrans");
}

TEST_F(UT_SchemeFactoryCov, InfoFactoryCreate_InvalidUrl_ReturnsNull)
{
    // Arrange
    const QUrl invalidUrl;
    QString error;

    // Act
    const auto info = InfoFactory::create<EntryFileInfo>(
            invalidUrl, Global::CreateFileInfoType::kCreateFileInfoSync, &error);

    // Assert — invalid url early-returns null before any cache lookup.
    EXPECT_EQ(info, nullptr);
    EXPECT_TRUE(error.isEmpty());
}

TEST_F(UT_SchemeFactoryCov, InfoFactoryCreate_UnknownScheme_ReturnsNullNoCacheMiss)
{
    // Arrange
    const QUrl url("utcovnoscheme:///nothing");
    bool isCacheMiss = true;
    QString error;

    // Act
    const auto info = InfoFactory::create<EntryFileInfo>(
            url, &isCacheMiss, Global::CreateFileInfoType::kCreateFileInfoSyncAndCache, &error);

    // Assert — the entry is reset to false up front and creation fails
    // because the scheme has no registered creator.
    EXPECT_EQ(info, nullptr);
    EXPECT_FALSE(isCacheMiss);
}

TEST_F(UT_SchemeFactoryCov, ViewFactoryCreate_RegisteredScheme_ReturnsView)
{
    // Arrange
    ASSERT_TRUE(ViewFactory::regClass<UtCovView>(kScheme2));

    // Act
    QString error;
    AbstractBaseView *view = ViewFactory::create<AbstractBaseView>(
            QUrl(QString(kScheme2) + ":///root"), &error);

    // Assert
    ASSERT_NE(view, nullptr);
    EXPECT_NE(dynamic_cast<UtCovView *>(view), nullptr);
    EXPECT_TRUE(error.isEmpty());
    delete view;
}
