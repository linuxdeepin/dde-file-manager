#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#include "controllers/interface/tagmanagerdaemon_interface.h"

DFM_USE_NAMESPACE

static constexpr const  char *const service{ "com.deepin.filemanager.daemon" };
static constexpr const char *const path{ "/com/deepin/filemanager/daemon/TagManagerDaemon" };


namespace  {

class TestTagManagerDaemonInterface : public testing::Test
{
public:
    std::unique_ptr<TagManagerDaemonInterface> m_daemonInterface{ nullptr };

    virtual void SetUp() override
    {
        m_daemonInterface = std::unique_ptr<TagManagerDaemonInterface> { new TagManagerDaemonInterface {
                service,
                path,
                QDBusConnection::systemBus()
            }
        };

        // blumia: since it's associated with context menu so we need to set a shorter timeout time.
        m_daemonInterface->setTimeout(3000);
    }

    virtual void TearDown() override
    {
        m_daemonInterface->destroyed();
    }
};

}

TEST_F(TestTagManagerDaemonInterface, tst_test)
{

}
