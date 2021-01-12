#pragma once
#include <dblockdevice.h>
#include <QCoreApplication>
#include <QFile>
#include "ut_mock_stub_common_define.h"

bool optical_stub();

bool ejectable_stub();

bool removable_stub();

QString getDummyMountPoint();

QString getAppRunPath();

QByteArrayList mountPoints_stub();

QByteArray device_stub();

bool canPowerOff_stub();

bool isValid_false_stub();

bool isValid_true_stub();

QString idLabel_empty_stub();

QString idLabel_dummy_stub();

qulonglong size_0_stub();

qulonglong size_1KB_stub();

qulonglong size_2KB_stub();

QDBusError lastError_stub();

QDBusError nolastError_stub();

bool exists_false_stub();

bool exists_true_stub();

uint getuid_stub();

uint groupId_stub();

bool permission_false_stub(QFile::Permissions permissions);

QString owner_stub();

QString get_empty_string_stub();
