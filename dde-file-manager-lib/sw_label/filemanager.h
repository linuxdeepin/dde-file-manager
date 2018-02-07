/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __cplusplus
extern "C"
{
	/*********************************************
	函数功能：动态加载右键菜单
	参数说明：path 文件全路径
	返回值：如文件需要动态加载右键菜单，则返回值为Json格式的字符串，
 	如不需要加载右键菜单，则返回值为字符串No。
	Json中的选项为：id,label，icon,tip,sub.
	id为右键菜单的唯一标识，要求唯一，id用于识别哪个菜单触发点击事件。
	label为在右键菜单显示的文字或标识。
	icon为在右键菜单上显示的图标全路径。
	tip为提示性字符串，可为空。
	sub内容为子菜单，内容为Json格式(目前只使用到一级菜单）
	**********************************************/
	char *auto_add_rightmenu(char *path);
	/*********************************************
	函数功能： 点击右键操作函数
		 该函数根据strid，实现用户对选中文件进行的具体操作
	参数说明：path 文件全路径
	 	strid：右键唯一标识码（该值在auto_add_rightmenu函数中设置）
	返回值：无
	**********************************************/
	void *auto_operation(char *path,char *strid);
	/*********************************************
	函数功能：获取角标
	参数说明：path 文件全路径
	返回值：角标文件全路径
	*********************************************/
	char *auto_add_emblem(char *path);
	/*********************************************
	函数功能：初始化标签系统（确保整个进程只调用一次）
	参数说明：无
	返回值：无
	*********************************************/
	void InitLLSInfo();
	/*********************************************
	函数功能：释放内存（针对返回的数据内存，操作系统使用完成后，需要释放）
	参数说明：无
	返回值：无
	*********************************************/
	void free_memory(void*);
}
#endif
