// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MUSICMESSAGEVIEW_H
#define MUSICMESSAGEVIEW_H

#include <QFrame>
#include <QMediaPlayer>
class QLabel;

/**
 * @brief MediaMeta 音频文件信息结构体
 */
struct MediaMeta {
    QString title;
    QString artist;
    QString album;
    QString codec;
};

class MusicMessageView : public QFrame
{
    Q_OBJECT
public:
    explicit MusicMessageView(const QString &uri = "", QWidget *parent = nullptr);
    void initUI();
    void updateElidedText();

signals:

public slots:
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief detectEncodings 检测字符串字符编码
     * @param rawData         需要检测的字符串
     * @return                返回检测到的字符编码列表
     */
    QList<QByteArray> detectEncodings(const QByteArray &rawData);

    /**
     * @brief isChinese     检测字符是否是中文
     * @param c             需要检测的字符
     * @return              true 为中文字符，反之亦然
     */
    bool isChinese(const QChar &c);

    /**
     * @brief tagOpenMusicFile  使用taglib打开音频文件获取信息
     * @param path              音频文件路径
     * @return                  音频文件信息
     */
    MediaMeta tagOpenMusicFile(const QString &path);

    /**
     * @brief characterEncodingTransform    对音频文件信息字符数据进行转码
     * @param meta                          音频文件信息对象,输出参数
     * @param obj                           taglib打开的文件对象
     */
    void characterEncodingTransform(MediaMeta &meta, void *obj);

private:
    QString m_uri;
    QLabel *m_titleLabel;
    QLabel *m_artistLabel;
    QLabel *m_albumLabel;
    QLabel *m_imgLabel;
    QLabel *m_artistValue;
    QLabel *m_albumValue;


    QMediaPlayer *m_player;

    QString m_title;
    QString m_artist;
    QString m_album;
    int m_margins;
    QMap<QString, QByteArray> localeCodes;  //!  区域与编码
};

#endif // MUSICMESSAGEVIEW_H
