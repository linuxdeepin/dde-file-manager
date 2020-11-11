#include <dgiofile.h>
#include <dgiofileinfo.h>

#include "dfileiconprovider.h"
#include "dfileinfo.h"
#include "dgvfsfileinfo.h"
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using DFM_NAMESPACE::DFileIconProvider;
namespace  {
    class DFileIconProviderTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            provider = new DFileIconProvider;
        }

        virtual void TearDown() override
        {
            delete provider;
            provider = nullptr;
        }

        DFileIconProvider *provider;
    };
}

TEST_F(DFileIconProviderTest, global_instance)
{
    auto instance = DFileIconProvider::globalProvider();
    EXPECT_TRUE(instance != nullptr);
}

TEST_F(DFileIconProviderTest, qicon)
{
    QFileInfo qfileinfo;
    qfileinfo.setFile("/usr/");
    QIcon qicon = provider->icon(qfileinfo);
    EXPECT_TRUE(!qicon.isNull());
    qfileinfo.setFile("/invliad");
    qicon = provider->icon(qfileinfo);
    EXPECT_TRUE(qicon.isNull());
}

TEST_F(DFileIconProviderTest, dicon)
{
    const DFileInfo dfileinfo("/home/");
    QIcon dicon = provider->icon(dfileinfo);
    EXPECT_TRUE(!dicon.isNull());
    const DFileInfo dfileinfo2("/invlide");
    dicon = provider->icon(dfileinfo2);
    EXPECT_TRUE(!dicon.isNull());
}

TEST_F(DFileIconProviderTest, gvfsicon)
{
    const DGvfsFileInfo info("ftp://ftp.freebsd.org");
    QIcon ftpIcon = provider->icon(info);
    EXPECT_TRUE(!ftpIcon.isNull());
    const DGvfsFileInfo info2("/invalid");
    QIcon icon = provider->icon(info2);
    EXPECT_TRUE(!icon.isNull());
}
