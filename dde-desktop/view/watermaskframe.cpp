/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "watermaskframe.h"
#include <QFile>
#include <QDebug>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QUrl>

#include "util/xcb/xcb.h"

WaterMaskFrame::WaterMaskFrame(const QString &fileName, QWidget *parent) :
    QFrame(parent),
    m_configFile(fileName)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    bool isConfigFileExist = checkConfigFile(m_configFile);
    if (isConfigFileExist) {
        loadConfig(m_configFile);
    } else {
        hide();
    }
}

WaterMaskFrame::~WaterMaskFrame()
{

}

bool WaterMaskFrame::checkConfigFile(const QString &fileName)
{
    if (QFile(fileName).exists()) {
        return true;
    } else {
        return false;
    }
}

void WaterMaskFrame::loadConfig(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "WaterMask config file doesn't exist!";
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error == QJsonParseError::NoError) {
        m_configs = QJsonObject::fromVariantMap(doc.toVariant().toMap());
        initUI();
    } else {
        qDebug() << error.errorString();
    }

}

void WaterMaskFrame::initUI()
{
    if (m_configs.contains("isMaskAlwaysOn")) {
        m_isMaskAlwaysOn =  m_configs.value("isMaskAlwaysOn").toBool();
    }

    QString maskLogoUri;
    if (m_configs.contains("maskLogoUri")) {
        maskLogoUri = m_configs.value("maskLogoUri").toString();
    } else {
        maskLogoUri = "";
    }

    QString maskLogoLayoutAlign;
    if (m_configs.contains("maskLogoLayoutAlign")) {
        maskLogoLayoutAlign = m_configs.value("maskLogoLayoutAlign").toString();
    } else {
        maskLogoLayoutAlign = "center";
    }

    int maskLogoWidth;
    if (m_configs.contains("maskLogoWidth")) {
        maskLogoWidth = m_configs.value("maskLogoWidth").toInt();
    } else {
        maskLogoWidth = 239;
    }

    int maskLogoHeight;
    if (m_configs.contains("maskLogoHeight")) {
        maskLogoHeight = m_configs.value("maskLogoHeight").toInt();
    } else {
        maskLogoHeight = 64;
    }

    QString maskText;
    if (m_configs.contains("maskText")) {
        maskText = m_configs.value("maskText").toString();
    } else {
        maskText = "";
    }

    QString maskTextLayoutAlign;
    if (m_configs.contains("maskTextLayoutAlign")) {
        maskTextLayoutAlign = m_configs.value("maskTextLayoutAlign").toString();
    } else {
        maskTextLayoutAlign = "center";
    }

    QString maskTextColor;
    if (m_configs.contains("maskTextColor")) {
        maskTextColor = m_configs.value("maskTextColor").toString();
    } else {
        maskTextColor = "#04d5f7";
    }

    QString maskTextFontSize;
    if (m_configs.contains("maskTextFontSize")) {
        maskTextFontSize = m_configs.value("maskTextFontSize").toString();
    } else {
        maskTextFontSize = "22px";
    }


    int maskTextWidth;
    if (m_configs.contains("maskTextWidth")) {
        maskTextWidth = m_configs.value("maskTextWidth").toInt();
    } else {
        maskTextWidth = 139;
    }

    int maskTextHeight;
    if (m_configs.contains("maskTextHeight")) {
        maskTextHeight = m_configs.value("maskTextHeight").toInt();
    } else {
        maskTextHeight = 40;
    }

    QString maskTextAlign;
    if (m_configs.contains("maskTextAlign")) {
        maskTextAlign = m_configs.value("maskTextAlign").toString();
    } else {
        maskTextAlign = "center";
    }

    int maskLogoTextSpacing;
    if (m_configs.contains("maskLogoTextSpacing")) {
        maskLogoTextSpacing = m_configs.value("maskLogoTextSpacing").toInt();
    } else {
        maskLogoTextSpacing = 0;
    }

    if (m_configs.contains("maskWidth")) {
        m_maskWidth = m_configs.value("maskWidth").toInt();
    } else {
        m_maskWidth = 239;
    }

    if (m_configs.contains("maskHeight")) {
        m_maskHeight = m_configs.value("maskHeight").toInt();
    } else {
        m_maskHeight = 110;
    }

    if (m_configs.contains("xRightBottom")) {
        m_xRightBottom = m_configs.value("xRightBottom").toInt();
    } else {
        m_xRightBottom = 50;
    }

    if (m_configs.contains("yRightBottom")) {
        m_yRightBottom = m_configs.value("yRightBottom").toInt();
    } else {
        m_yRightBottom = 98;
    }

    m_logoLabel = new QLabel(this);
    m_logoLabel->setPixmap(QPixmap(maskLogoUri).scaled(maskLogoWidth, maskLogoHeight));
    m_logoLabel->setFixedSize(maskLogoWidth, maskLogoHeight);

    m_textLabel = new QLabel(this);
    m_textLabel->setText(maskText);
    m_textLabel->setFixedSize(maskTextWidth, maskTextHeight);

    if (maskTextAlign == "left") {
        m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (maskTextAlign == "right") {
        m_textLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    } else if (maskTextAlign == "center") {
        m_textLabel->setAlignment(Qt::AlignCenter);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->addStretch();

    if (maskLogoUri.length() != 0) {
        if (maskLogoLayoutAlign == "left") {
            mainLayout->addWidget(m_logoLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
        } else if (maskLogoLayoutAlign == "right") {
            mainLayout->addWidget(m_logoLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
        } else if (maskLogoLayoutAlign == "center") {
            mainLayout->addWidget(m_logoLabel, 0, Qt::AlignCenter);
        }
    }

    mainLayout->addSpacing(maskLogoTextSpacing);

    if (maskText.length() != 0) {
        if (maskTextLayoutAlign == "left") {
            mainLayout->addWidget(m_textLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
        } else if (maskTextLayoutAlign == "right") {
            mainLayout->addWidget(m_textLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
        } else if (maskTextLayoutAlign == "center") {
            mainLayout->addWidget(m_textLabel, 0, Qt::AlignCenter);
        }
    }
    mainLayout->addStretch();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    setFixedSize(m_maskWidth, m_maskHeight);
    QString color(maskTextColor);
    QString fontsize(maskTextFontSize);
    QString style = QString("QLabel {color: %1; font-size: %2}").arg(color, fontsize);
    setStyleSheet(style);
}

void WaterMaskFrame::updatePosition()
{
    int x = static_cast<QWidget *>(parent())->width() - m_xRightBottom - m_maskWidth;
    int y = static_cast<QWidget *>(parent())->height() - m_yRightBottom - m_maskHeight;
    move(x, y);
}
