#pragma once

#include <gmock/gmock.h>
#include <QCoreApplication>

#include "dattacheddeviceinterface.h"

class MockDAttachedDeviceInterface:public DAttachedDeviceInterface{

public:

    MOCK_METHOD0(isValid, bool());
    MOCK_METHOD0(detachable, bool());
    MOCK_METHOD0(detach, void());
    MOCK_METHOD0(displayName, QString());
    MOCK_METHOD0(deviceUsageValid, bool());
    MOCK_METHOD0(deviceUsage, QPair<quint64, quint64>());
    MOCK_METHOD0(iconName, QString());
    MOCK_METHOD0(mountpointUrl, QUrl());
    MOCK_METHOD0(accessPointUrl, QUrl());
};
