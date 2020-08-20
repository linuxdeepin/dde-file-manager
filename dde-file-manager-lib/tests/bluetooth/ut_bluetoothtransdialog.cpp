#include <gtest/gtest.h>
#include "bluetooth/bluetoothtransdialog.h"

namespace {
class TestBluetoothTransDialog : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestBluetoothTransDialog";
        dlg = new BluetoothTransDialog(QStringList() << "/usr/bin/dde-desktop", BluetoothTransDialog::DirectlySend,
                                       "12-23-34-45-56");
    }

    void TearDown() override
    {
        std::cout << "end TestBluetoothTransDialog";
        delete dlg;
        dlg = nullptr;
    }

public:
    BluetoothTransDialog *dlg = nullptr;
};
} // namespace

TEST_F(TestBluetoothTransDialog, ShowDialog)
{
    dlg->show();
    dlg->hide();
}
