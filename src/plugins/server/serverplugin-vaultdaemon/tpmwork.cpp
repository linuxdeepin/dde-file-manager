// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tpmwork.h"
#include "vaultconfigoperator.h"

#include <QDebug>

#include <fstream>

inline constexpr char kPcr[] { "7" };

using namespace serverplugin_vaultdaemon;

TpmWork::TpmWork(QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<TpmDecryptState>("TpmDecryptState");
}

TpmWork::~TpmWork()
{
    quit();
    wait();
}

void TpmWork::run()
{
    QString passwd;

    if (system("tpm2_getrandom 6")) {
        fmCritical() << "Tpm is not available!";
        Q_EMIT workFinished(TpmDecryptState::kNotAvailable, passwd);
        return;
    }

    VaultConfigOperator config;
    const QString hashAlgo = config.get(kConfigNodeNameOfTPM, kConfigKeyPrimaryHashAlgo).toString();
    const QString keyAlgo = config.get(kConfigNodeNameOfTPM, kConfigKeyPrimaryKeyAlgo).toString();
    if (hashAlgo.isEmpty() || keyAlgo.isEmpty()) {
        fmCritical() << "Get tpm algo failed!";
        Q_EMIT workFinished(TpmDecryptState::kDecryptFailed, passwd);
        return;
    }
    std::string pcr = kPcr;
    std::string pcr_bank = hashAlgo.toStdString();
    const std::string primary_key_alg = keyAlgo.toStdString();
    const std::string primary_hash_alg = hashAlgo.toStdString();

    const std::string basePath = kVaultConfigPath.toStdString();

    const std::string cipherPath = basePath + "/cipher.out";
    const std::string ivPath = basePath + "/iv.bin";
    const std::string pubKeyPath = basePath + "/key.pub";
    const std::string priKeyPath = basePath + "/key.priv";

    const std::string sessionPath = basePath + "/session.dat";
    const std::string primaryCtxPath = basePath + "/primary.ctx";
    const std::string keyNamePath = basePath + "/key.name";
    const std::string keyCtxPath = basePath + "/key.ctx";
    const std::string clearPath = basePath + "/clear.out";

    if (std::system(("tpm2_startauthsession --policy-session -S " + sessionPath + " -g " + primary_hash_alg + " -G " + primary_key_alg).c_str())) {
        fmCritical() << "Command tpm2_startauthsession run failed!";
        Q_EMIT workFinished(TpmDecryptState::kDecryptFailed, passwd);
        return;
    }
    if (std::system(("tpm2_policypcr -S " + sessionPath + " -l " + pcr_bank + ":" + pcr).c_str())) {
        fmCritical() << "Command tpm2_policypcr run failed!";
        Q_EMIT workFinished(TpmDecryptState::kDecryptFailed, passwd);
        return;
    }
    if (std::system(("tpm2_createprimary -C o -g " + primary_hash_alg + " -G " + primary_key_alg + " -c " + primaryCtxPath).c_str())) {
        fmCritical() << "Command tpm2_createprimary run faild!";
        Q_EMIT workFinished(TpmDecryptState::kDecryptFailed, passwd);
        return;
    }
    if (std::system(("tpm2_load -C " + primaryCtxPath + " -u " + pubKeyPath + " -r " + priKeyPath + " -n " + keyNamePath +  " -c " + keyCtxPath).c_str())) {
        fmCritical() << "Command tpm2_load run failed!";
        Q_EMIT workFinished(TpmDecryptState::kDecryptFailed, passwd);
        return;
    }
    if (std::system(("tpm2_encryptdecrypt -Q --iv " + ivPath + " -c " + keyCtxPath + " -o " + clearPath + " " + cipherPath + " -p session:" + sessionPath).c_str())) {
        fmCritical() << "Command tpm2_encryptdecrypt run failed!";
        Q_EMIT workFinished(TpmDecryptState::kDecryptFailed, passwd);
        return;
    }
    if (std::system(("tpm2_flushcontext " + sessionPath).c_str())) {
        fmCritical() << "Command tpm2_flushcontext run failed!";
        Q_EMIT workFinished(TpmDecryptState::kDecryptFailed, passwd);
        return;
    }

    std::ifstream plain_ifs(clearPath, std::ios_base::in);
    if (!plain_ifs.is_open()) {
        fmCritical() << "Tpm file not open!";
        Q_EMIT workFinished(TpmDecryptState::kDecryptFailed, passwd);
        return;
    }
    std::string plain;
    plain_ifs >> plain;
    plain_ifs.close();

    passwd = QString::fromStdString(plain);

    // clean files
    std::system(("rm " + clearPath + " " + keyCtxPath + " " + keyNamePath + " " + primaryCtxPath + " " + sessionPath).c_str());

    Q_EMIT workFinished(TpmDecryptState::kDecryptSuccess, passwd);
}
