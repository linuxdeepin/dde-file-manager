/*
 * Copyright (C) 2016 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "funcwrapper.h"

#include <libffmpegthumbnailer/videothumbnailer.h>

#include <iostream>
#include <sstream>
#include <cstring>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        return -1;
    }

    try {
        std::vector<uint8_t> imageData;
        std::stringstream stream(argv[1]);
        std::string path(argv[2]);

        int size;

        stream >> size;

        ffmpegthumbnailer::VideoThumbnailer vt(size, false, true, 20, false);
        vt.generateThumbnail(path, ThumbnailerImageTypeEnum::Png, imageData);

        char *base64_data = toBase64(imageData.data(), static_cast<int>(imageData.size()), Base64Encoding);

        printf("%s", base64_data);
        fflush(stdout);
    } catch (std::logic_error &e) {
        std::cerr << e.what();
        return -1;
    }

    return 0;
}
