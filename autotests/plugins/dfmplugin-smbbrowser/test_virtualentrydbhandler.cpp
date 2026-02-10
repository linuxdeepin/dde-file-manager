// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/datahelper/virtualentrydata.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <QVariantMap>
#include <QString>
#include <QUrl>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

DPSMBBROWSER_USE_NAMESPACE

class VirtualEntryDbHandler_Stub : public QObject
{
public:
    explicit VirtualEntryDbHandler_Stub(QObject *parent = nullptr)
        : QObject(parent)
    {
        __DBG_STUB_INVOKE__
    }
};

using namespace dfmplugin_smbbrowser;

class UT_VirtualEntryDBHandler : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        VirtualEntryDbHandler::instance()->handler = new dfmbase::SqliteHandle("test");
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

    static void SetUpTestCase()
    {
        DFMBASE_USE_NAMESPACE

        // stub for Constructor
        glob_stub().set(stub_ext::StubExt::get_ctor_addr<VirtualEntryDbHandler>(),
                        stub_ext::StubExt::get_ctor_addr<VirtualEntryDbHandler_Stub>());

        // stub for SqliteHandle::remove
        typedef bool (SqliteHandle::*Remove1)(const VirtualEntryData &);
        auto remove1 = static_cast<Remove1>(&SqliteHandle::remove<VirtualEntryData>);
        glob_stub().set_lamda(remove1, [] { __DBG_STUB_INVOKE__ return true; });

        typedef bool (SqliteHandle::*Remove2)(const Expression::Expr &);
        auto remove2 = static_cast<Remove2>(&SqliteHandle::remove<VirtualEntryData>);
        glob_stub().set_lamda(remove2, [] { __DBG_STUB_INVOKE__ return true; });

        glob_stub().set_lamda(&SqliteHandle::insert<VirtualEntryData>, [] { __DBG_STUB_INVOKE__ return 0; });
        glob_stub().set_lamda(&SqliteHandle::query<VirtualEntryData>, [] { __DBG_STUB_INVOKE__ return SqliteQueryable<VirtualEntryData>("test", "test"); });
        glob_stub().set_lamda(&SqliteHandle::dropTable<VirtualEntryData>, [] { __DBG_STUB_INVOKE__ return true; });

        typedef bool (SqliteHandle::*CreateTable)(const SqliteConstraint &, const SqliteConstraint &);
        auto createTable = static_cast<CreateTable>(&SqliteHandle::createTable<VirtualEntryData>);
        glob_stub().set_lamda(createTable, [] { __DBG_STUB_INVOKE__ return true; });
    }

    static void TearDownTestCase()
    {
        glob_stub().clear();
    }

private:
    stub_ext::StubExt stub;

    static stub_ext::StubExt &glob_stub()
    {
        static stub_ext::StubExt s;
        return s;
    }
};

TEST_F(UT_VirtualEntryDBHandler, Instance)
{
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance());
}

TEST_F(UT_VirtualEntryDBHandler, ClearData)
{
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance()->clearData());
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance()->clearData("smb://1.2.3.4/hello"));
}

TEST_F(UT_VirtualEntryDBHandler, RemoveData)
{
    stub.set_lamda(&VirtualEntryDbHandler::allSmbIDs,
                   [](void *, QStringList *, QStringList *seperated) {
                       __DBG_STUB_INVOKE__
                       *seperated = QStringList { "smb://1.2.3.4/hello" };
                       return QStringList {};
                   });
    stub.set_lamda(&protocol_display_utilities::getSmbHostPath, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4"; });
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance()->removeData("smb://1.2.3.4/hello"));
}

TEST_F(UT_VirtualEntryDBHandler, SaveAggregatedAndSeperated)
{
    stub.set_lamda(&protocol_display_utilities::getSmbHostPath, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4"; });
    stub.set_lamda(&VirtualEntryDbHandler::saveData, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance()->saveAggregatedAndSperated("smb://1.2.3.4", "1.2.3.4"));
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance()->saveAggregatedAndSperated("", ""));
}

TEST_F(UT_VirtualEntryDBHandler, SaveData)
{
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance()->saveData(VirtualEntryData()));
}

TEST_F(UT_VirtualEntryDBHandler, HasOfflineEntry)
{
    QStringList offlineEntry {};
    stub.set_lamda(&VirtualEntryDbHandler::allSmbIDs, [&] { __DBG_STUB_INVOKE__ return offlineEntry; });
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance()->hasOfflineEntry("test"));
    EXPECT_FALSE(VirtualEntryDbHandler::instance()->hasOfflineEntry("test"));

    offlineEntry << "smb://1.2.3.4/hello";
    EXPECT_TRUE(VirtualEntryDbHandler::instance()->hasOfflineEntry("smb://1.2.3.4/hello"));
}

TEST_F(UT_VirtualEntryDBHandler, AllSmbIDs)
{
    stub.set_lamda(&VirtualEntryDbHandler::virtualEntries,
                   [] { __DBG_STUB_INVOKE__ return QList<QSharedPointer<VirtualEntryData>>(); });
}

TEST_F(UT_VirtualEntryDBHandler, GetDisplayNameOf)
{
    QUrl u;
    u.setScheme("entry");
    u.setPath("smb://1.2.3.4.ventry");
    EXPECT_EQ("1.2.3.4", VirtualEntryDbHandler::instance()->getDisplayNameOf(u));
    u = "entry://smb://1.2.3.4/hello.ventry";
    EXPECT_EQ("", VirtualEntryDbHandler::instance()->getDisplayNameOf(u));
}

TEST_F(UT_VirtualEntryDBHandler, VirtualEntries)
{
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance()->virtualEntries());
    EXPECT_TRUE(VirtualEntryDbHandler::instance()->virtualEntries().count() == 0);
}

TEST_F(UT_VirtualEntryDBHandler, CheckDBExists)
{
    delete VirtualEntryDbHandler::instance()->handler;
    stub.set_lamda(&dfmbase::SqliteConnectionPool::openConnection, [] { __DBG_STUB_INVOKE__ return QSqlDatabase(); });
    bool dbValid = false;
    stub.set_lamda(&QSqlDatabase::isValid, [&] { __DBG_STUB_INVOKE__ return dbValid; });
    EXPECT_FALSE(VirtualEntryDbHandler::instance()->checkDbExists());

    dbValid = true;
    stub.set_lamda(&QSqlDatabase::close, [] { __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(VirtualEntryDbHandler::instance()->checkDbExists());
}

TEST_F(UT_VirtualEntryDBHandler, CreateTable)
{
    EXPECT_NO_FATAL_FAILURE(VirtualEntryDbHandler::instance()->createTable());
    EXPECT_TRUE(VirtualEntryDbHandler::instance()->createTable());
}

// #include "ut_virtualentrydbhandler.moc"
