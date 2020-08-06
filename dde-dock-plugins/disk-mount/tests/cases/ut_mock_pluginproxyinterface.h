#pragma once

#include <gmock/gmock.h>

#include "pluginproxyinterface.h"

class MockPluginProxyInterface:public PluginProxyInterface{

public:
    MOCK_METHOD2(itemAdded, void(PluginsItemInterface * const, const QString &));
    MOCK_METHOD2(itemUpdate, void(PluginsItemInterface * const, const QString &));
    MOCK_METHOD2(itemRemoved, void(PluginsItemInterface * const, const QString &));
    MOCK_METHOD3(requestWindowAutoHide, void(PluginsItemInterface * const, const QString &, const bool));
    MOCK_METHOD2(requestRefreshWindowVisible, void(PluginsItemInterface * const, const QString &));
    MOCK_METHOD3(requestSetAppletVisible, void(PluginsItemInterface * const, const QString &, const bool));
    MOCK_METHOD3(saveValue, void(PluginsItemInterface * const, const QString &, const QVariant &));
    MOCK_METHOD3(getValue, const QVariant(PluginsItemInterface * const, const QString &, const QVariant&));
    MOCK_METHOD2(removeValue, void(PluginsItemInterface *const, const QStringList &));
};
