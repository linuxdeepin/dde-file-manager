桌面水印配置说明

####水印配置文件安装路径：
    
    /usr/share/deepin/dde-desktop-watermask.json

####水印参数说明

        {
            "isMaskAlwaysOn": true,
            "maskLogoUri" : "/home/djf/test/deepin-mask.png",
            "maskLogoLayoutAlign" : "center",
            "maskLogoWidth": 239,
            "maskLogoHeight": 64,
            "maskText" : "  深度操作系统（专业版）",
            "maskTextLayoutAlign" : "center",
            "maskTextColor" : "#04d5f7",
            "maskTextFontSize": "22px",
            "maskTextAlign": "center",
            "maskTextWidth": 239,
            "maskTextHeight": 40,
            "maskLogoTextSpacing": 0,
            "maskWidth": 239,
            "maskHeight": 110,
            "xRightBottom": 50,
            "yRightBottom": 98
        }

+ **isMaskAlwaysOn**：true表示水印总是显示，false表示水印仅仅显示在默认壁纸上
+ **maskLogoUri**： 水印图片logo路径
+ **maskLogoLayoutAlign**：水印图片logo的布局策略，默认"center" 可取"left", "center", "right"
+ **maskLogoWidth** ：水印图片logo的高度
+ **maskLogoHeight**:：水印图片logo的宽度
+ **maskText**：水印文本
+ **maskTextLayoutAlign**：水印文本的布局策略，默认"center", 可取"left", "center", "right"
+ **maskTextColor**： 水印文本的颜色，默认"#04d5f7",
+ **maskTextFontSize**：水印文本的大小，默认"22px",
+ **maskTextAlign**：水印文本的对齐方式，默认 "center", 可取"left", "center", "right"
+ **maskTextWidth**:：水印文本的宽度，默认239,
+ **maskTextHeight**： 水印文本的高度，默认40,
+ **maskLogoTextSpacing**： 水印logo和文本的布局间隔，默认为0,
+ **maskWidth**： 水印总宽度，默认239
+ **maskHeight**： 水印总高度，默认110
+ **xRightBottom**: 距离屏幕右边缘的宽度，默认50
+ **yRightBottom**: 距离屏幕底边缘的高度，默认98 = 50 + 48(dock的高度)


注意：一般而言 maskLogoWidth、maskTextWidth、maskWidth最好相等；maskLogoHeight、maskTextHeight、maskHeight最好相等。

