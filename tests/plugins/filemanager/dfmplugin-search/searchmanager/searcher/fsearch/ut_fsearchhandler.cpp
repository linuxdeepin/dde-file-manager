// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager/searcher/fsearch/fsearchhandler.h"

#include "stubext.h"

#include <dfm-base/base/device/deviceutils.h>

#include <gtest/gtest.h>

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(FSearchHandlerTest, ut_checkPathSearchable)
{
    stub_ext::StubExt st;
    st.set_lamda(&DeviceUtils::fstabBindInfo, [] {
        __DBG_STUB_INVOKE__
        QMap<QString, QString> info { { "/data/home", "/home" } };
        return info;
    });

    EXPECT_TRUE(FSearchHandler::checkPathSearchable("/data/home"));
}

TEST(FSearchHandlerTest, ut_init)
{
    FSearchHandler handler;
    handler.init();

    EXPECT_TRUE(handler.app);
    EXPECT_TRUE(handler.app->db);
    EXPECT_TRUE(handler.app->pool);
    EXPECT_TRUE(handler.app->search);
}

TEST(FSearchHandlerTest, ut_reset)
{
    FSearchHandler handler;
    handler.init();
    handler.isStop = true;

    handler.reset();
    EXPECT_FALSE(handler.isStop);
}

TEST(FSearchHandlerTest, ut_loadDatabase)
{
    stub_ext::StubExt st;
    st.set_lamda(load_database, [] { __DBG_STUB_INVOKE__ return true; });

    FSearchHandler handler;
    handler.init();

    EXPECT_TRUE(handler.loadDatabase("/", ""));
}

TEST(FSearchHandlerTest, ut_updateDatabase)
{
    stub_ext::StubExt st;
    st.set_lamda(load_database, [] { __DBG_STUB_INVOKE__ return true; });

    FSearchHandler handler;
    handler.init();
    char path[6] = { "/home" };
    handler.app->config->locations = g_list_append(handler.app->config->locations, path);

    EXPECT_TRUE(handler.updateDatabase());
}

TEST(FSearchHandlerTest, ut_saveDatabase)
{
    stub_ext::StubExt st;
    st.set_lamda(&db_save_locations, [] { __DBG_STUB_INVOKE__ return true; });

    FSearchHandler handler;
    handler.init();

    EXPECT_TRUE(handler.saveDatabase("/home"));
}

TEST(FSearchHandlerTest, ut_search)
{
    FSearchHandler handler;
    handler.init();

    stub_ext::StubExt st;
    st.set_lamda(db_perform_search, [&] { __DBG_STUB_INVOKE__ handler.syncMutex.unlock(); });

    auto callback = [](const QString &, bool) {};
    EXPECT_TRUE(handler.search("test", callback));
}

TEST(FSearchHandlerTest, ut_stop)
{
    FSearchHandler handler;
    handler.stop();

    EXPECT_TRUE(handler.isStop);
}

TEST(FSearchHandlerTest, ut_setMaxResults)
{
    FSearchHandler handler;
    handler.setMaxResults(100);

    EXPECT_EQ(handler.maxResults, 100);
}

TEST(FSearchHandlerTest, ut_setFlags_1)
{
    FSearchHandler handler;
    handler.init();

    handler.setFlags(FSearchHandler::FSearchFlag::FSEARCH_FLAG_ALL);

    EXPECT_TRUE(handler.app->db->db_config->filter_hidden_file);
    EXPECT_TRUE(handler.app->db->db_config->enable_py);
    EXPECT_TRUE(handler.app->config->enable_regex);
}

TEST(FSearchHandlerTest, ut_setFlags_2)
{
    FSearchHandler handler;
    handler.init();

    handler.setFlags(FSearchHandler::FSearchFlag::FSEARCH_FLAG_NONE);

    EXPECT_FALSE(handler.app->db->db_config->filter_hidden_file);
    EXPECT_FALSE(handler.app->db->db_config->enable_py);
    EXPECT_FALSE(handler.app->config->enable_regex);
}

TEST(FSearchHandlerTest, ut_dbTimeStamp)
{
    FSearchHandler handler;
    EXPECT_EQ(0, handler.dbTimeStamp());

    handler.init();
    handler.app->db->timestamp = 100;

    EXPECT_EQ(100, handler.dbTimeStamp());
}

TEST(FSearchHandlerTest, ut_releaseApp)
{
    FSearchHandler handler;
    handler.init();
    handler.releaseApp();

    EXPECT_FALSE(handler.app);
}

TEST(FSearchHandlerTest, ut_reveiceResultsCallback)
{
    FSearchHandler handler;
    handler.init();

    BTreeNode *node = btree_node_new("home", "", "", 0, 0, 0, false);
    DatabaseSearchEntry *entry = (DatabaseSearchEntry *)calloc(1, sizeof(DatabaseSearchEntry));
    entry->node = node;
    entry->pos = 0;

    DatabaseSearchResult data;
    data.num_files = 1;
    data.num_folders = 0;
    data.cb_data = nullptr;
    data.results = g_ptr_array_sized_new(1);
    g_ptr_array_add(data.results, entry);

    bool finished = false;
    auto callback = [&](const QString &, bool finish) {
        __DBG_STUB_INVOKE__
        finished = finish;
    };

    handler.callbackFunc = callback;
    handler.reveiceResultsCallback(&data, &handler);

    free(entry);
    btree_node_free(node);

    EXPECT_TRUE(finished);
}
