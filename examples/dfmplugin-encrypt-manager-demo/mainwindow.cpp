// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <DSpinner>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFile>
#include <QDir>
#include <QtConcurrent/QtConcurrent>

#include <dfm-framework/event/event.h>

#include <fstream>

Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(bool *)

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initUi();
    initConnect();
}

void MainWindow::initUi()
{
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLay = new QVBoxLayout;
    mainWidget->setLayout(mainLay);

    editInput = new QLineEdit(mainWidget);

    textBrowser = new QTextBrowser(mainWidget);
    textBrowser->setReadOnly(true);

    QGridLayout *btnLay = new QGridLayout;
    btnCheckTpm = new QPushButton(tr("Check TPM"), mainWidget);
    btnGetRandom = new QPushButton(tr("Get Random"), mainWidget);
    btnCheckAlgo = new QPushButton(tr("Check Algo"), mainWidget);
    btnEncrypt = new QPushButton(tr("Encrypt"), mainWidget);
    btnDecrypt = new QPushButton(tr("Decrypt"), mainWidget);
    btnEncryptTwo = new QPushButton(tr("EncryptInProcess"), mainWidget);
    btnDecryptTwo = new QPushButton(tr("DecryptInProcess"), mainWidget);
    btnEncryptThree = new QPushButton(tr("EncryptByCommand"), mainWidget);
    btnDecryptThree = new QPushButton(tr("DecryptByCommand"), mainWidget);
    btnLay->addWidget(btnCheckTpm, 0, 0);
    btnLay->addWidget(btnGetRandom, 0, 1);
    btnLay->addWidget(btnCheckAlgo, 0, 2);
    btnLay->addWidget(btnEncrypt, 1, 0);
    btnLay->addWidget(btnDecrypt, 1, 1);
    btnLay->addWidget(btnEncryptTwo, 2, 0);
    btnLay->addWidget(btnDecryptTwo, 2, 1);
    btnLay->addWidget(btnEncryptThree, 3, 0);
    btnLay->addWidget(btnDecryptThree, 3, 1);

    mainLay->addWidget(editInput);
    mainLay->addWidget(textBrowser);
    mainLay->addItem(btnLay);

    setCentralWidget(mainWidget);
    setMinimumSize(800, 500);
}

void MainWindow::initConnect()
{
    connect(btnCheckTpm, &QPushButton::clicked, this, [this] {
        int result = dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_TPMIsAvailablePro").toInt();
        if (result == 0)
            textBrowser->append("TPM is available!");
        else
            textBrowser->append("TPM is not available!");
    });
    connect(btnGetRandom, &QPushButton::clicked, this, [this] {
        int size = editInput->text().toInt();
        QString out;
        int result = dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_GetRandomByTPMPro", size, &out).toInt();
        if (result == 0) {
            textBrowser->append(QString("Random is: %1").arg(out));
        } else {
            textBrowser->append("Get random failed!");
        }
    });
    connect(btnCheckAlgo, &QPushButton::clicked, this, [this] {
        const QString algoName = editInput->text();
        bool bSupport { false };
        int result = dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_IsTPMSupportAlgoPro", algoName, &bSupport).toInt();
        if (result == 0) {
            textBrowser->append(QString("The check result is %1!").arg(bSupport));
        } else {
            textBrowser->append("Check algo name failed!");
        }
    });
    connect(btnEncrypt, &QPushButton::clicked, this, [this] {
        const QString hashAlgo = "sha256";
        const QString keyAlgo = "aes";
        const QString keyPin = ""/*"12345678"*/;
        const QString password = "Qwer@1234";
        const QString dirPath = "/home/uos/gongheng/tmpTemp";

        QFutureWatcher<bool> watcher;
        QEventLoop loop;
        QFuture<bool> future = QtConcurrent::run([hashAlgo, keyAlgo, keyPin, password, dirPath] {
            return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_EncryptByTPM", hashAlgo, keyAlgo, keyPin, password, dirPath).toBool();
        });
        connect(&watcher, &QFutureWatcher<bool>::finished, this, [&watcher, &loop] {
            if (watcher.result()) {
                loop.exit(0);
            } else {
                loop.exit(-1);
            }
        });
        watcher.setFuture(future);

        DSpinner spinner(this);
        spinner.setFixedSize(50, 50);
        spinner.move((width() - spinner.width()) / 2, (height() - spinner.height()) / 2);
        spinner.start();
        spinner.show();

        int re = loop.exec();
        bool result = re == 0 ? true : false;

        if (result) {
            textBrowser->append("Encrypt success!");
            QFile file(dirPath + QDir::separator() + "tpm_encrypt.txt");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QByteArray ciphertext = file.readAll();
                QByteArray baseCiphertext = ciphertext.toBase64();
                textBrowser->append("Cipher text: " + QString(baseCiphertext));
                file.close();
            }
            file.setFileName(dirPath + QDir::separator() + "key.priv");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QByteArray priKey = file.readAll();
                QByteArray basePriKey = priKey.toBase64();
                textBrowser->append("Pri key: " + QString(basePriKey));
                file.close();
            }

            file.setFileName(dirPath + QDir::separator() + "key.pub");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QByteArray pubKey = file.readAll();
                QByteArray basePubKey = pubKey.toBase64();
                textBrowser->append("Pub key: " + QString(basePubKey));
                file.close();
            }

        } else {
            textBrowser->append("Encrypt failed!");
        }
    });
    connect(btnDecrypt, &QPushButton::clicked, this, [this] {
        const QString keyPin = "" /*"12345678"*/;
        const QString dirPath = "/home/uos/gongheng/tmpTemp";
        QString password;
        bool result = dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_DecryptByTPM", keyPin, dirPath, &password).toBool();
        if (result) {
            textBrowser->append(QString("Password is: %1 !").arg(password));
        } else {
            textBrowser->append("Decrypt failed!");
        }
    });
    connect(btnEncryptTwo, &QPushButton::clicked, this, [this]{
            const QString &dirPath = QDir::homePath() + "/.TPMKey";
            QDir dir(dirPath);
            if (!dir.exists())
                dir.mkpath(dirPath);
            QString pwd;
            int success = dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_GetRandomByTPM", 14, &pwd).toInt();
            if (success != 0) {
                textBrowser->append("Create password faild!");
                return;
            }
            // TPM
//            QVariantMap map {
//                { "PropertyKey_EncryptType", 1 },
//                { "PropertyKey_SessionHashAlgo", "sha256" },
//                { "PropertyKey_SessionKeyAlgo", "aes" },
//                { "PropertyKey_PrimaryHashAlgo", "sha256" },
//                { "PropertyKey_PrimaryKeyAlgo", "rsa" },
//                { "PropertyKey_MinorHashAlgo", "sha256" },
//                { "PropertyKey_MinorKeyAlgo", "aes" },
//                { "PropertyKey_DirPath", dirPath },
//                { "PropertyKey_Plain", pwd },
//                { "PropertyKey_Pcr", "7" },
//                { "PropertyKey_PcrBank", "sha256" }
//            };
//            QVariantMap map {
//                { "PropertyKey_EncryptType", 2 },
//                { "PropertyKey_SessionHashAlgo", "sha256" },
//                { "PropertyKey_SessionKeyAlgo", "aes" },
//                { "PropertyKey_PrimaryHashAlgo", "sha256" },
//                { "PropertyKey_PrimaryKeyAlgo", "rsa" },
//                { "PropertyKey_MinorHashAlgo", "sha256" },
//                { "PropertyKey_MinorKeyAlgo", "aes" },
//                { "PropertyKey_DirPath", dirPath },
//                { "PropertyKey_Plain", pwd },
//                { "PropertyKey_PinCode", "pin123456" }
//            };
            QVariantMap map {
                { "PropertyKey_EncryptType", 3 },
                { "PropertyKey_SessionHashAlgo", "sha256" },
                { "PropertyKey_SessionKeyAlgo", "aes" },
                { "PropertyKey_PrimaryHashAlgo", "sha256" },
                { "PropertyKey_PrimaryKeyAlgo", "rsa" },
                { "PropertyKey_MinorHashAlgo", "sha256" },
                { "PropertyKey_MinorKeyAlgo", "aes" },
                { "PropertyKey_DirPath", dirPath },
                { "PropertyKey_Plain", pwd },
                { "PropertyKey_Pcr", "7" },
                { "PropertyKey_PcrBank", "sha256" },
                { "PropertyKey_PinCode", "pin123456" }
            };
            // TCM
//            QVariantMap map {
//                { "PropertyKey_EncryptType", 1 },
//                { "PropertyKey_SessionHashAlgo", "sm3_256" },
//                { "PropertyKey_SessionKeyAlgo", "sm4" },
//                { "PropertyKey_PrimaryHashAlgo", "sm3_256" },
//                { "PropertyKey_PrimaryKeyAlgo", "sm4" },
//                { "PropertyKey_MinorHashAlgo", "sm3_256" },
//                { "PropertyKey_MinorKeyAlgo", "sm4" },
//                { "PropertyKey_DirPath", dirPath },
//                { "PropertyKey_Plain", pwd },
//                { "PropertyKey_Pcr", "7" },
//                { "PropertyKey_PcrBank", "sm3_256" }
//            };
//            QVariantMap map {
//                { "PropertyKey_EncryptType", 2 },
//                { "PropertyKey_SessionHashAlgo", "sm3_256" },
//                { "PropertyKey_SessionKeyAlgo", "sm4" },
//                { "PropertyKey_PrimaryHashAlgo", "sm3_256" },
//                { "PropertyKey_PrimaryKeyAlgo", "sm4" },
//                { "PropertyKey_MinorHashAlgo", "sm3_256" },
//                { "PropertyKey_MinorKeyAlgo", "sm4" },
//                { "PropertyKey_DirPath", dirPath },
//                { "PropertyKey_Plain", pwd },
//                { "PropertyKey_PinCode", "pin123456" }
//            };
//            QVariantMap map {
//                { "PropertyKey_EncryptType", 3 },
//                { "PropertyKey_SessionHashAlgo", "sm3_256" },
//                { "PropertyKey_SessionKeyAlgo", "sm4" },
//                { "PropertyKey_PrimaryHashAlgo", "sm3_256" },
//                { "PropertyKey_PrimaryKeyAlgo", "sm4" },
//                { "PropertyKey_MinorHashAlgo", "sm3_256" },
//                { "PropertyKey_MinorKeyAlgo", "sm4" },
//                { "PropertyKey_DirPath", dirPath },
//                { "PropertyKey_Plain", pwd },
//                { "PropertyKey_Pcr", "7" },
//                { "PropertyKey_PcrBank", "sha256" },
//                { "PropertyKey_PinCode", "pin123456" }
//            };
            QFutureWatcher<int> watcher;
            QEventLoop loop;
            QFuture<int> future = QtConcurrent::run([map]{
                return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_EncryptByTPMPro", map).toInt();
            });
            connect(&watcher, &QFutureWatcher<int>::finished, this, [&watcher, &loop]{
                if (watcher.result() == 0) {
                    loop.exit(0);
                } else {
                    loop.exit(-1);
                }
            });
            watcher.setFuture(future);

            DSpinner spinner(this);
            spinner.setFixedSize(50, 50);
            spinner.move((width() - spinner.width())/2, (height() - spinner.height())/2);
            spinner.start();
            spinner.show();

            int re = loop.exec();
            bool result = (re == 0 ? true : false);

            if (result) {
                textBrowser->append(QString("Encrypt success! password is: %1").arg(pwd));
            } else {
                textBrowser->append("Encrypt failed!");
            }
        });

    connect(btnDecryptTwo, &QPushButton::clicked, this, [this]{
        const QString &dirPath = QDir::homePath() + "/.TPMKey";
        QDir dir(dirPath);
        if (!dir.exists())
            dir.mkpath(dirPath);
        QString pwd;
        // TPM
        //            QVariantMap map {
        //                { "PropertyKey_EncryptType", 1 },
        //                { "PropertyKey_SessionHashAlgo", "sha256" },
        //                { "PropertyKey_SessionKeyAlgo", "aes" },
        //                { "PropertyKey_PrimaryHashAlgo", "sha256" },
        //                { "PropertyKey_PrimaryKeyAlgo", "rsa" },
        //                { "PropertyKey_DirPath", dirPath },
        //                { "PropertyKey_Pcr", "7" },
        //                { "PropertyKey_PcrBank", "sha256" }
        //            };
        //            QVariantMap map {
        //                { "PropertyKey_EncryptType", 2 },
        //                { "PropertyKey_SessionHashAlgo", "sha256" },
        //                { "PropertyKey_SessionKeyAlgo", "aes" },
        //                { "PropertyKey_PrimaryHashAlgo", "sha256" },
        //                { "PropertyKey_PrimaryKeyAlgo", "rsa" },
        //                { "PropertyKey_DirPath", dirPath },
        //                { "PropertyKey_PinCode", "pin123456" }
        //            };
        QVariantMap map {
            { "PropertyKey_EncryptType", 3 },
            { "PropertyKey_SessionHashAlgo", "sha256" },
            { "PropertyKey_SessionKeyAlgo", "aes" },
            { "PropertyKey_PrimaryHashAlgo", "sha256" },
            { "PropertyKey_PrimaryKeyAlgo", "rsa" },
            { "PropertyKey_DirPath", dirPath },
            { "PropertyKey_Pcr", "7" },
            { "PropertyKey_PcrBank", "sha256" },
            { "PropertyKey_PinCode", "pin123456" }
        };
        // TCM
        //            QVariantMap map {
        //                { "PropertyKey_EncryptType", 1 },
        //                { "PropertyKey_SessionHashAlgo", "sm3_256" },
        //                { "PropertyKey_SessionKeyAlgo", "sm4" },
        //                { "PropertyKey_PrimaryHashAlgo", "sm3_256" },
        //                { "PropertyKey_PrimaryKeyAlgo", "sm4" },
        //                { "PropertyKey_DirPath", dirPath },
        //                { "PropertyKey_Pcr", "7" },
        //                { "PropertyKey_PcrBank", "sm3_256" }
        //            };
        //            QVariantMap map {
        //                { "PropertyKey_EncryptType", 2 },
        //                { "PropertyKey_SessionHashAlgo", "sm3_256" },
        //                { "PropertyKey_SessionKeyAlgo", "sm4" },
        //                { "PropertyKey_PrimaryHashAlgo", "sm3_256" },
        //                { "PropertyKey_PrimaryKeyAlgo", "sm4" },
        //                { "PropertyKey_DirPath", dirPath },
        //                { "PropertyKey_PinCode", "pin123456" }
        //            };
        //            QVariantMap map {
        //                { "PropertyKey_EncryptType", 3 },
        //                { "PropertyKey_SessionHashAlgo", "sm3_256" },
        //                { "PropertyKey_SessionKeyAlgo", "sm4" },
        //                { "PropertyKey_PrimaryHashAlgo", "sm3_256" },
        //                { "PropertyKey_PrimaryKeyAlgo", "sm4" },
        //                { "PropertyKey_DirPath", dirPath },
        //                { "PropertyKey_Pcr", "7" },
        //                { "PropertyKey_PcrBank", "sm3_256" },
        //                { "PropertyKey_PinCode", "pin123456" }
        //            };
        QFutureWatcher<int> watcher;
        QEventLoop loop;
        QFuture<int> future = QtConcurrent::run([map, &pwd]{
            return dpfSlotChannel->push("dfmplugin_encrypt_manager", "slot_DecryptByTPMPro", map, &pwd).toInt();
        });
        connect(&watcher, &QFutureWatcher<int>::finished, this, [&watcher, &loop]{
            if (watcher.result() == 0) {
                loop.exit(0);
            } else {
                loop.exit(-1);
            }
        });
        watcher.setFuture(future);

        DSpinner spinner(this);
        spinner.setFixedSize(50, 50);
        spinner.move((width() - spinner.width())/2, (height() - spinner.height())/2);
        spinner.start();
        spinner.show();

        int re = loop.exec();
        bool result = (re == 0 ? true : false);

        if (result) {
            textBrowser->append(QString("Decrypt success! password is: %1").arg(pwd));
        } else {
            textBrowser->append("Decrypt failed!");
        }
    });

    connect(btnEncryptThree, &QPushButton::clicked, this, [this] {

        int nType = 2; // 0:kTpmAndPcr  1:kTpmAndPin  2:kTpmAndPinAndPcr
        const std::string plain  = "Qwer@1234";

        const std::string pinCode = "pin123456";

        const std::string pcr = "7";
        const std::string pcr_bank = "sha256";

        const std::string session_key_alg = "aes";
        const std::string session_hash_alg = "sha256";
        const std::string primary_key_alg = "rsa";
        const std::string primary_hash_alg = "sha256";
        const std::string minor_key_alg = "aes";
        const std::string minor_hash_alg = "sha256";

        const QString &dirPath = QDir::homePath() + "/.TPMKey";
        QDir dir(dirPath);
        if (!dir.exists())
            dir.mkpath(dirPath);
        const std::string basePath = dirPath.toStdString();

        const std::string plainPath = basePath + "/plain.dat";
        const std::string ivPath = basePath + "/iv.bin";

        const std::string sessionPath = basePath + "/session.dat";
        const std::string policyPath = basePath + "/policy.dat";
        const std::string primaryCtxPath = basePath + "/primary.ctx";
        const std::string pubKeyPath = basePath + "/key.pub";
        const std::string priKeyPath = basePath + "/key.priv";
        const std::string keyNamePath = basePath + "/key.name";
        const std::string keyCtxPath = basePath + "/key.ctx";
        const std::string cipherPath = basePath + "/cipher.out";

        const std::string pcrPath = basePath + "/pcr_val.bin";

        // generate plain & iv
        if (std::system(("echo " + plain + " > " + plainPath).c_str()))
            return false;
        if (std::system(("tpm2_getrandom -o " + ivPath + " 16").c_str()))
            return false;

        // set policy
        if (std::system(("tpm2_startauthsession -S " + sessionPath + " -g " + session_hash_alg + " -G " + session_key_alg).c_str()))
            return false;
        if (nType == 0) {
            if (std::system(("tpm2_pcrread " + pcr_bank + ":" + pcr + " -o " + pcrPath).c_str()))
                return false;
            if (std::system(("tpm2_policypcr -S " + sessionPath + " -l " + pcr_bank + ":" + pcr + " -L " + policyPath + " -f " + pcrPath).c_str()))
                return false;
        } else if (nType == 1) {
            if (std::system(("tpm2_policypassword -S " + sessionPath + " -L " + policyPath).c_str()))
                return false;
        } else if (nType == 2) {
            if (std::system(("tpm2_pcrread " + pcr_bank + ":" + pcr + " -o " + pcrPath).c_str()))
                return false;
            if (std::system(("tpm2_policypcr -S " + sessionPath + " -l " + pcr_bank + ":" + pcr + " -L " + policyPath + " -f " + pcrPath).c_str()))
                return false;
            if (std::system(("tpm2_policypassword -S " + sessionPath + " -L " + policyPath).c_str()))
                return false;
        } else {
            qCritical() << "Tpm type unkonw!";
            return false;
        }
        if (std::system(("tpm2_flushcontext " + sessionPath).c_str()))
            return false;

        // generate keys
        if (std::system(("tpm2_createprimary -C o -g " + primary_hash_alg + " -G " + primary_key_alg + " -c " + primaryCtxPath).c_str()))
            return false;
        if (nType == 0) {
            if (std::system(("tpm2_create -g " + minor_hash_alg + " -G " + minor_key_alg + " -u " + pubKeyPath + " -r " + priKeyPath + " -C " + primaryCtxPath + " -L " + policyPath).c_str()))
                return false;
        } else if (nType == 1) {
            if (std::system(("tpm2_create -g " + minor_hash_alg + " -G " + minor_key_alg + " -u " + pubKeyPath + " -r " + priKeyPath + " -C " + primaryCtxPath + " -L " + policyPath + " -p " + pinCode).c_str()))
                return false;
        } else if (nType == 2) {
            if (std::system(("tpm2_create -g " + minor_hash_alg + " -G " + minor_key_alg + " -u " + pubKeyPath + " -r " + priKeyPath + " -C " + primaryCtxPath + " -L " + policyPath + " -p " + pinCode).c_str()))
                return false;
        } else {
            qCritical() << "Tpm type unkonw!";
            return false;
        }

        if (std::system(("tpm2_load -C " + primaryCtxPath + " -u " + pubKeyPath + " -r " + priKeyPath + " -n " + keyNamePath + " -c " + keyCtxPath).c_str()))
            return false;

        // generate cipher
        if (std::system(("tpm2_startauthsession --policy-session -S " + sessionPath + " -g " + session_hash_alg + " -G " + session_key_alg).c_str()))
            return false;
        if (nType == 0) {
            if (std::system(("tpm2_pcrread " + pcr_bank + ":" + pcr + " -o " + pcrPath).c_str()))
                return false;
            if (std::system(("tpm2_policypcr -S " + sessionPath + " -l " + pcr_bank + ":" + pcr + " -f " + pcrPath).c_str()))
                return false;
            if (std::system(("tpm2_encryptdecrypt -Q --iv " + ivPath + " -c " + keyCtxPath + " -o " + cipherPath + " " + plainPath + " -p session:" + sessionPath).c_str()))
                return false;
        } else if (nType == 1) {
            if (std::system(("tpm2_policypassword -S " + sessionPath + " -L " + policyPath).c_str()))
                return false;
            if (std::system(("tpm2_encryptdecrypt -Q --iv " + ivPath + " -c " + keyCtxPath + " -o " + cipherPath + " " + plainPath + " -p session:" + sessionPath + "+" + pinCode).c_str()))
                return false;
        } else if (nType == 2) {
            if (std::system(("tpm2_pcrread " + pcr_bank + ":" + pcr + " -o " + pcrPath).c_str()))
                return false;
            if (std::system(("tpm2_policypcr -S " + sessionPath + " -l " + pcr_bank + ":" + pcr + " -f " + pcrPath).c_str()))
                return false;
            if (std::system(("tpm2_policypassword -S " + sessionPath + " -L " + policyPath).c_str()))
                return false;
            if (std::system(("tpm2_encryptdecrypt -Q --iv " + ivPath + " -c " + keyCtxPath + " -o " + cipherPath + " " + plainPath + " -p session:" + sessionPath + "+" + pinCode).c_str()))
                return false;
        } else {
            qCritical() << "Tpm type unkonw!";
            return false;
        }
        if (std::system(("tpm2_flushcontext " + sessionPath).c_str()))
            return false;

        // clean files
        if (nType == 0) {
            if (std::system(("rm " + keyCtxPath + " " + keyNamePath + " " + plainPath + " " + policyPath + " " + primaryCtxPath + " " + sessionPath + " " + pcrPath).c_str()))
                return false;
        } else if (nType == 1) {
            if (std::system(("rm " + keyCtxPath + " " + keyNamePath + " " + plainPath + " " + policyPath + " " + primaryCtxPath + " " + sessionPath).c_str()))
                return false;
        } else if (nType == 2) {
            if (std::system(("rm " + keyCtxPath + " " + keyNamePath + " " + plainPath + " " + policyPath + " " + primaryCtxPath + " " + sessionPath + " " + pcrPath).c_str()))
                return false;
        } else {
            qCritical() << "Tpm type unkonw!";
            return false;
        }

        textBrowser->append(QString("Encrypt success! password is: %1").arg(QString::fromStdString(plain)));
        return true;
    });

    connect(btnDecryptThree, &QPushButton::clicked, this, [this] {
        int nType = 2; // 0:kTpmAndPcr  1:kTpmAndPin  2:kTpmAndPcrAndPin
        const std::string pinCode = "pin123456";
        std::string pcr = "7";
        std::string pcr_bank = "sha256";

        const std::string session_key_alg = "aes";
        const std::string session_hash_alg = "sha256";
        const std::string primary_key_alg = "rsa";
        const std::string primary_hash_alg = "sha256";

        const QString &dirPath = QDir::homePath() + "/.TPMKey";
        QDir dir(dirPath);
        if (!dir.exists())
            dir.mkpath(dirPath);
        const std::string basePath = dirPath.toStdString();

        const std::string cipherPath = basePath + "/cipher.out";
        const std::string ivPath = basePath + "/iv.bin";
        const std::string pubKeyPath = basePath + "/key.pub";
        const std::string priKeyPath = basePath + "/key.priv";

        const std::string sessionPath = basePath + "/session.dat";
        const std::string policyPath = basePath + "/policy.dat";
        const std::string primaryCtxPath = basePath + "/primary.ctx";
        const std::string keyNamePath = basePath + "/key.name";
        const std::string keyCtxPath = basePath + "/key.ctx";
        const std::string clearPath = basePath + "/clear.out";

        if (std::system(("tpm2_startauthsession --policy-session -S " + sessionPath + " -g " + session_hash_alg + " -G " + session_key_alg).c_str()))
            return false;
        if (nType == 0) {
            if (std::system(("tpm2_policypcr -S " + sessionPath + " -l " + pcr_bank + ":" + pcr).c_str()))
                return false;
        } else if (nType == 1) {
            if (std::system(("tpm2_policypassword -S " + sessionPath + " -L " + policyPath).c_str()))
                return false;
        } else if (nType == 2) {
            if (std::system(("tpm2_policypcr -S " + sessionPath + " -l " + pcr_bank + ":" + pcr).c_str()))
                return false;
            if (std::system(("tpm2_policypassword -S " + sessionPath + " -L " + policyPath).c_str()))
                return false;
        }  else {
            qCritical() << "Tpm type unkonw!";
            return false;
        }
        if (std::system(("tpm2_createprimary -C o -g " + primary_hash_alg + " -G " + primary_key_alg + " -c " + primaryCtxPath).c_str()))
            return false;
        if (std::system(("tpm2_load -C " + primaryCtxPath + " -u " + pubKeyPath + " -r " + priKeyPath + " -n " + keyNamePath +  " -c " + keyCtxPath).c_str()))
            return false;
        if (nType == 0) {
            if (std::system(("tpm2_encryptdecrypt -Q --iv " + ivPath + " -c " + keyCtxPath + " -d -o " + clearPath + " " + cipherPath + " -p session:" + sessionPath).c_str()))
                return false;
        } else if (nType == 1) {
            if (std::system(("tpm2_encryptdecrypt -Q --iv " + ivPath + " -c " + keyCtxPath + " -d -o " + clearPath + " " + cipherPath + " -p session:" + sessionPath + "+" + pinCode).c_str()))
                return false;
        } else if (nType == 2) {
            if (std::system(("tpm2_encryptdecrypt -Q --iv " + ivPath + " -c " + keyCtxPath + " -d -o " + clearPath + " " + cipherPath + " -p session:" + sessionPath + "+" + pinCode).c_str()))
                return false;
        } else {
            qCritical() << "Tpm type unkonw!";
            return false;
        }
        if (std::system(("tpm2_flushcontext " + sessionPath).c_str()))
            return false;

        std::ifstream plain_ifs(clearPath, std::ios_base::in);
        if (!plain_ifs.is_open())
            return false;
        std::string plain;
        plain_ifs >> plain;
        plain_ifs.close();

        textBrowser->append(QString("Decrypt success! password is: %1").arg(QString::fromStdString(plain)));

        // clean files
        if (nType == 0) {
            std::system(("rm " + clearPath + " " + keyCtxPath + " " + keyNamePath + " " + primaryCtxPath + " " + sessionPath).c_str());
        } else if (nType == 1) {
            std::system(("rm " + clearPath + " " + keyCtxPath + " " + keyNamePath + " " + policyPath + " " + primaryCtxPath + " " + sessionPath).c_str());
        } else if (nType == 2) {
            std::system(("rm " + clearPath + " " + keyCtxPath + " " + keyNamePath + " " + policyPath + " " + primaryCtxPath + " " + sessionPath).c_str());
        } else {
            qCritical() << "Tpm type unkonw!";
            return false;
        }

        return true;
    });
}
