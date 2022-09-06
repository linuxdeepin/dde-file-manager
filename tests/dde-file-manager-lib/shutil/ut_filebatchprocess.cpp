// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shutil/filebatchprocess.h"
#include "shutil/fileutils.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
    class TestFileBatchProcess : public testing::Test {
    public:
        void SetUp() override
        {

        }
        void TearDown() override
        {
        }

    public:
        // return file path
        QString createOneFile(const QString& filename)
        {
            QString folderPath = getTestFolder();

            QDir dir;
            if (!dir.exists(folderPath))
            {
                dir.mkpath(folderPath);
            }

            QString filePath = QString("%1/%2").arg(folderPath).arg(filename);

            QFile file(filePath);
            QString content= "test for shutil/filebatchprocess.h";
            //判断文件是否存在
            if(!file.exists())
            {
                file.open(QIODevice::ReadWrite | QIODevice::Text);
                QByteArray str = content.toUtf8();
                file.write(str);
            }

            file.close();
            return filePath;
        }

        // return file folder
        QString getTestFolder()
        {
            return QString("%1/test_shutil").arg(QDir::currentPath());
        }
    };
}

TEST_F(TestFileBatchProcess, can_replase_exited_files)
{
    QString findStr{ "src" };
    QString replaceStr{ "new" };

    QList<DUrl> originUrls;
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src1.txt")));
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src2.txt")));

    QSharedMap<DUrl, DUrl> result = FileBatchProcess::instance()->replaceText(originUrls,QPair<QString, QString>{ findStr, replaceStr });

    EXPECT_FALSE(result->isEmpty());

    QMap<DUrl, DUrl>::const_iterator beg = result->constBegin();
    QMap<DUrl, DUrl>::const_iterator end = result->constEnd();
    for (;beg != end; ++beg) {
        EXPECT_TRUE(beg.key().fileName().contains("src"));
        EXPECT_TRUE(beg.value().fileName().contains("new"));
    }
}

TEST_F(TestFileBatchProcess, cannot_replase_emtpty_files)
{
    QString findStr{ "src" };
    QString replaceStr{ "new" };

    QList<DUrl> originUrls;

    QSharedMap<DUrl, DUrl> result = FileBatchProcess::instance()->replaceText(originUrls,QPair<QString, QString>{ findStr, replaceStr });

    EXPECT_TRUE(result == nullptr);
}

TEST_F(TestFileBatchProcess, can_add_files_name_at_after)
{
    QString addStr{ "testCase" };
    DFileService::AddTextFlags replaceStr{ DFileService::AddTextFlags::After };

    QList<DUrl> originUrls;
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src1.txt")));
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src2.txt")));

    QSharedMap<DUrl, DUrl> result = FileBatchProcess::instance()->addText(originUrls,QPair<QString, DFileService::AddTextFlags>{ addStr, replaceStr });

    EXPECT_FALSE(result->isEmpty());

    QMap<DUrl, DUrl>::const_iterator beg = result->constBegin();
    QMap<DUrl, DUrl>::const_iterator end = result->constEnd();
    for (;beg != end; ++beg) {
        EXPECT_TRUE(beg.key().fileName().contains("src"));
        EXPECT_TRUE(beg.value().fileName().contains(addStr));
    }
}

TEST_F(TestFileBatchProcess, can_add_files_name_at_before)
{
    QString addStr{ "testCase" };
    DFileService::AddTextFlags replaceStr{ DFileService::AddTextFlags::Before };

    QList<DUrl> originUrls;
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src1.txt")));
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src2.txt")));

    QSharedMap<DUrl, DUrl> result = FileBatchProcess::instance()->addText(originUrls,QPair<QString, DFileService::AddTextFlags>{ addStr, replaceStr });

    EXPECT_FALSE(result->isEmpty());

    QMap<DUrl, DUrl>::const_iterator beg = result->constBegin();
    QMap<DUrl, DUrl>::const_iterator end = result->constEnd();
    for (;beg != end; ++beg) {
        EXPECT_TRUE(beg.key().fileName().contains("src"));
        EXPECT_TRUE(beg.value().fileName().contains(addStr));
    }
}

TEST_F(TestFileBatchProcess, can_replase_exited_files_with_custom)
{
    QString findStr{ "new" };
    QString replaceStr{ "new" };

    QList<DUrl> originUrls;
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src1.txt")));
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src2.txt")));

    QSharedMap<DUrl, DUrl> result = FileBatchProcess::instance()->customText(originUrls,QPair<QString, QString>{ findStr, replaceStr });

    EXPECT_FALSE(result->isEmpty());

    QMap<DUrl, DUrl>::const_iterator beg = result->constBegin();
    QMap<DUrl, DUrl>::const_iterator end = result->constEnd();
    for (;beg != end; ++beg) {
        EXPECT_TRUE(beg.key().fileName().contains("src"));
        EXPECT_TRUE(beg.value().fileName().contains("new"));
    }
}

TEST_F(TestFileBatchProcess, batch_replase_exited_files)
{
    FileUtils::removeRecurse(getTestFolder(), "");

    QString findStr{ "src" };
    QString replaceStr{ "new" };

    QList<DUrl> originUrls;
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src1.txt")));
    originUrls.push_back(DUrl::fromLocalFile(createOneFile("src2.txt")));

    QSharedMap<DUrl, DUrl> result = FileBatchProcess::instance()->replaceText(originUrls,QPair<QString, QString>{ findStr, replaceStr });

    QMap<DUrl, DUrl> bacthResult = FileBatchProcess::instance()->batchProcessFile(result);

    EXPECT_FALSE(result->isEmpty());

    QMap<DUrl, DUrl>::const_iterator beg = result->constBegin();
    QMap<DUrl, DUrl>::const_iterator end = result->constEnd();
    for (;beg != end; ++beg) {
        EXPECT_TRUE(beg.key().fileName().contains("src"));
        EXPECT_TRUE(beg.value().fileName().contains("new"));
    }
}
