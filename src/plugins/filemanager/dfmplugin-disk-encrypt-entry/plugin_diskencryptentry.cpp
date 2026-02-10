// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin_diskencryptentry.h"
#include "menu/diskencryptmenuscene.h"
#include "events/eventshandler.h"
#include "utils/encryptutils.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QTranslator>
#include <QTimer>
#include <QMenu>

using namespace dfmplugin_diskenc;

DFM_LOG_REGISTER_CATEGORY(DISKENC_NAMESPACE)

static constexpr char kEncryptDConfig[] { "org.deepin.dde.file-manager.diskencrypt" };

bool hasComputerMenuRegisted()
{
    return dpfSlotChannel->push(kMenuPluginName, "slot_MenuScene_Contains", QString(kComputerMenuSceneName)).toBool();
}

void DiskEncryptEntry::initialize()
{
    // auto i18n = new QTranslator(this);
    // i18n->load(QLocale(), "disk-encrypt", "_", "/usr/share/dde-file-manager/translations");
    // QCoreApplication::installTranslator(i18n);

    auto mng = dfmbase::DConfigManager::instance();
    mng->addConfig(kEncryptDConfig);
    connect(mng, &dfmbase::DConfigManager::valueChanged,
            this, [this](const QString &config, const QString &key) {
                if (config == kEncryptDConfig && key == "enableEncrypt")
                    initEncryptEvents();
            });
}

bool DiskEncryptEntry::start()
{
    initEncryptEvents();
    return true;
}

void DiskEncryptEntry::initEncryptEvents()
{
    if (!config_utils::enableEncrypt()) {
        fmInfo() << "Disk encryption is disabled in configuration, skipping event initialization";
        return;
    }

    static bool inited = false;
    if (inited) {
        fmDebug() << "Encrypt events already initialized, skipping";
        return;
    }

    dpfSlotChannel->push(kMenuPluginName, "slot_MenuScene_RegisterScene",
                         DiskEncryptMenuCreator::name(), new DiskEncryptMenuCreator);

    if (hasComputerMenuRegisted()) {
        dpfSlotChannel->push("dfmplugin_menu", "slot_MenuScene_Bind",
                             DiskEncryptMenuCreator::name(), QString(kComputerMenuSceneName));
    } else {
        dpfSignalDispatcher->subscribe(kMenuPluginName, "signal_MenuScene_SceneAdded",
                                       this, &DiskEncryptEntry::onComputerMenuSceneAdded);
    }

    EventsHandler::instance()->bindDaemonSignals();
    EventsHandler::instance()->hookEvents();

    QString decJob = EventsHandler::instance()->unfinishedDecryptJob();
    if (!decJob.isEmpty() && !EventsHandler::instance()->isTaskWorking()) {
        QTimer::singleShot(1000, this, [=] {
            processUnfinshedDecrypt(decJob);
        });
    }

    inited = true;
}

void DiskEncryptEntry::onComputerMenuSceneAdded(const QString &scene)
{
    if (scene == "ComputerMenu") {
        dpfSlotChannel->push(kMenuPluginName, "slot_MenuScene_Bind",
                             DiskEncryptMenuCreator::name(), kComputerMenuSceneName);
        dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                         this, &DiskEncryptEntry::onComputerMenuSceneAdded);
    }
}

void DiskEncryptEntry::processUnfinshedDecrypt(const QString &device)
{
    QString dev(device);
    QString ignoreFile = "/tmp/dfm_ignore_decrypt_auto_reqeust_" + dev.replace("/", "_");
    if (QFile(ignoreFile).exists()) {
        fmInfo() << "Ignore file exists for device, skipping auto-decrypt:" << ignoreFile;
        return;
    }

    QMenu *menu = new QMenu();
    DiskEncryptMenuScene *scene = new DiskEncryptMenuScene();

    QUrl url;
    url.setScheme("entry");
    url.setPath(QString("%1.blockdev").arg(device.mid(5)));
    QVariant urls = QVariant::fromValue<QList<QUrl>>({ url });
    QVariantHash params;
    params.insert(dfmbase::MenuParamKey::kSelectFiles, urls);
    scene->initialize(params);
    scene->create(menu);
    scene->updateState(menu);

    auto actions = menu->actions();
    auto ret = std::find_if(actions.cbegin(), actions.cend(), [](QAction *act) {
        qWarning() << act->property(dfmbase::ActionPropertyKey::kActionID).toString();
        return act->property(dfmbase::ActionPropertyKey::kActionID).toString() == "de_1_decrypt";
    });
    if (ret == actions.cend()) {
        fmWarning() << "Decrypt action not found in menu for device:" << device;
        return;
    }

    scene->triggered(*ret);
    delete scene;
    delete menu;

    QFile f(ignoreFile);
    f.open(QIODevice::Truncate | QIODevice::WriteOnly);
    f.close();
}
