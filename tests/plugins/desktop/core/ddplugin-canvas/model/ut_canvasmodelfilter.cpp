// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/canvasmodelfilter.h"
#include "model/canvasproxymodel_p.h"
#include "utils/fileutil.h"

#include "stubext.h"

#include <QStandardPaths>
#include <QGSettings>

#include <gtest/gtest.h>

DDP_CANVAS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(CanvasModelFilter, base)
{
    CanvasProxyModel model;
    CanvasModelFilter filter(&model);
    EXPECT_EQ(filter.model, &model);
    EXPECT_FALSE(filter.insertFilter({}));
    QList<QUrl> list;
    EXPECT_FALSE(filter.resetFilter(list));
    EXPECT_FALSE(filter.updateFilter({}));
    EXPECT_FALSE(filter.removeFilter({}));
    EXPECT_FALSE(filter.renameFilter({}, {}));
}

namespace  {
class TestFileInfo : public FileInfo
{
public:
    using FileInfo::FileInfo;
    void refresh() override {re = true;}
    bool isAttributes(const FileIsType type) const {
        if (type == OptInfoType::kIsHidden)
            return hidden;
        return FileInfo::isAttributes(type);
    }
public:
    bool re = false;
    bool hidden = true;
};
}

TEST(HiddenFileFilter, insertFilter)
{
    CanvasProxyModel model;
    HiddenFileFilter filter(&model);
    FileInfoPointer file(new TestFileInfo(QUrl("file:///var/log")));
    TestFileInfo *ptr = dynamic_cast<TestFileInfo *>(file.data());

    stub_ext::StubExt stub;
    bool cf = false;
    stub.set_lamda(&DesktopFileCreator::createFileInfo, [&file, &cf](){
        cf = true;
        return file;
    });

    ptr->hidden = false;
    model.d->filters |= QDir::Hidden;

    EXPECT_FALSE(filter.insertFilter({}));
    EXPECT_FALSE(cf);
    EXPECT_FALSE(ptr->re);

    ptr->re = false;
    ptr->hidden = true;
    model.d->filters &= ~QDir::Hidden;

    EXPECT_TRUE(filter.insertFilter({}));
    EXPECT_TRUE(cf);
    EXPECT_TRUE(ptr->re);

    ptr->re = false;
    ptr->hidden = false;
    model.d->filters &= ~QDir::Hidden;
    EXPECT_FALSE(filter.insertFilter({}));
    EXPECT_TRUE(cf);
    EXPECT_TRUE(ptr->re);
}

TEST(HiddenFileFilter, resetFilter)
{
    CanvasProxyModel model;
    HiddenFileFilter filter(&model);
    FileInfoPointer file(new TestFileInfo(QUrl("file:///var/log")));
    TestFileInfo *ptr = dynamic_cast<TestFileInfo *>(file.data());

    stub_ext::StubExt stub;
    bool cf = false;
    stub.set_lamda(&DesktopFileCreator::createFileInfo, [&file, &cf](){
        cf = true;
        return file;
    });

    ptr->hidden = false;
    model.d->filters |= QDir::Hidden;
    QList<QUrl> urls = {QUrl::fromLocalFile("/var/log")};

    {
        auto in = urls;
        EXPECT_FALSE(filter.resetFilter(in));
        EXPECT_FALSE(cf);
        EXPECT_EQ(in.size(), 1);
    }

    {
        auto in = urls;
        ptr->re = false;
        ptr->hidden = true;
        model.d->filters &= ~QDir::Hidden;

        EXPECT_FALSE(filter.resetFilter(in));
        EXPECT_TRUE(cf);
        EXPECT_EQ(in.size(), 0);
    }

    {
        auto in = urls;
        ptr->re = false;
        ptr->hidden = false;
        model.d->filters &= ~QDir::Hidden;
        EXPECT_FALSE(filter.resetFilter(in));
        EXPECT_TRUE(cf);
        EXPECT_EQ(in.size(), 1);
    }
}

TEST(HiddenFileFilter, updateFilter)
{
    CanvasProxyModel model;
    HiddenFileFilter filter(&model);
    model.d->filters &= ~QDir::Hidden;

    stub_ext::StubExt stub;
    bool re = false;
    stub.set_lamda(&CanvasProxyModel::refresh, [&re](){
        re = true;
    });

    {
        EXPECT_FALSE(filter.updateFilter(QUrl::fromLocalFile("/var")));
        EXPECT_FALSE(re);
    }

    {
        re = false;
        EXPECT_FALSE(filter.updateFilter(QUrl::fromLocalFile("/var/.hidden")));
        EXPECT_TRUE(re);
    }

    {
        re = false;
        model.d->filters |= QDir::Hidden;
        EXPECT_FALSE(filter.updateFilter(QUrl::fromLocalFile("/var/.hidden")));
        EXPECT_FALSE(re);
    }
}

TEST(InnerDesktopAppFilter, construction_0)
{
    CanvasProxyModel model;
    stub_ext::StubExt stub;
    QString schema;
    stub.set_lamda(&QGSettings::isSchemaInstalled, [&schema](const QByteArray &schema_id){
        schema = schema_id;
        return false;
    });

    bool update = false;
    stub.set_lamda(&InnerDesktopAppFilter::update, [&update](){
        update = true;
    });

    InnerDesktopAppFilter filter(&model);
    EXPECT_FALSE(update);
    EXPECT_EQ(schema, QString("com.deepin.dde.filemanager.desktop"));

    EXPECT_EQ(filter.keys.value("desktopComputer")
                ,QUrl::fromLocalFile(QStandardPaths::standardLocations(
                                         QStandardPaths::DesktopLocation).first() + "/dde-computer.desktop"));
    EXPECT_EQ(filter.keys.value("desktopTrash")
                ,QUrl::fromLocalFile(QStandardPaths::standardLocations(
                                         QStandardPaths::DesktopLocation).first() + "/dde-trash.desktop"));
    EXPECT_EQ(filter.keys.value("desktopHomeDirectory")
                ,QUrl::fromLocalFile(QStandardPaths::standardLocations(
                                         QStandardPaths::DesktopLocation).first() + "/dde-home.desktop"));

    EXPECT_FALSE(filter.hidden.value("desktopComputer", true));
    EXPECT_FALSE(filter.hidden.value("desktopTrash", true));
    EXPECT_FALSE(filter.hidden.value("desktopHomeDirectory", true));
}

TEST(InnerDesktopAppFilter, construction_1)
{
    if (!QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.desktop"))
        return;

    CanvasProxyModel model;
    stub_ext::StubExt stub;
    stub.set_lamda(&QGSettings::isSchemaInstalled, [](){
        return true;
    });

    bool update = false;
    stub.set_lamda(&InnerDesktopAppFilter::update, [&update](){
        update = true;
    });

    InnerDesktopAppFilter filter(&model);
    EXPECT_TRUE(update);
}

TEST(InnerDesktopAppFilter, update)
{
    if (!QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.desktop"))
        return;

    CanvasProxyModel model;
    InnerDesktopAppFilter filter(&model);
    decltype(filter.hidden) show;
    show.insert("desktopComputer", false);
    show.insert("desktopTrash", false);
    show.insert("desktopHomeDirectory", false);

    stub_ext::StubExt stub;
    stub.set_lamda(&QGSettings::get, [&show](QGSettings *self, const QString &key){
        return show.value(key, true);
    });

    filter.update();

    EXPECT_TRUE(filter.hidden.value("desktopComputer", false));
    EXPECT_TRUE(filter.hidden.value("desktopTrash", false));
    EXPECT_TRUE(filter.hidden.value("desktopHomeDirectory", false));
    EXPECT_FALSE(filter.hidden.value("HomeDirectory", false));
}

TEST(InnerDesktopAppFilter, resetFilter)
{
    CanvasProxyModel model;
    InnerDesktopAppFilter filter(&model);

    filter.hidden["desktopComputer"] = true;
    filter.hidden["desktopTrash"] = true;
    filter.hidden["desktopHomeDirectory"] = true;

    QList<QUrl> list = {QUrl::fromLocalFile(QStandardPaths::standardLocations(
                             QStandardPaths::DesktopLocation).first() + "/dde-computer.desktop"),
                    QUrl::fromLocalFile(QStandardPaths::standardLocations(
                             QStandardPaths::DesktopLocation).first() + "/dde-trash.desktop"),
                    QUrl::fromLocalFile(QStandardPaths::standardLocations(
                             QStandardPaths::DesktopLocation).first() + "/dde-home.desktop")};
    EXPECT_FALSE(filter.resetFilter(list));
    EXPECT_EQ(list.size(), 0);
}

TEST(InnerDesktopAppFilter, changed)
{
    if (!QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.desktop"))
        return;

    CanvasProxyModel model;
    InnerDesktopAppFilter filter(&model);
    decltype(filter.hidden) show;
    show.insert("desktopComputer", false);
    show.insert("desktopTrash", false);
    show.insert("desktopHomeDirectory", false);

    stub_ext::StubExt stub;
    stub.set_lamda(&QGSettings::get, [&show](QGSettings *self, const QString &key){
        return show.value(key, false);
    });

    filter.changed("desktopComputer");

    EXPECT_TRUE(filter.hidden.value("desktopComputer", true));
    EXPECT_FALSE(filter.hidden.value("desktopTrash", true));
    EXPECT_FALSE(filter.hidden.value("desktopHomeDirectory", true));

    filter.changed("desktopTrash");

    EXPECT_TRUE(filter.hidden.value("desktopComputer", true));
    EXPECT_TRUE(filter.hidden.value("desktopTrash", true));
    EXPECT_FALSE(filter.hidden.value("desktopHomeDirectory", true));


    filter.changed("desktopHomeDirectory");

    EXPECT_TRUE(filter.hidden.value("desktopComputer", true));
    EXPECT_TRUE(filter.hidden.value("desktopTrash", true));
    EXPECT_TRUE(filter.hidden.value("desktopHomeDirectory", true));
}
