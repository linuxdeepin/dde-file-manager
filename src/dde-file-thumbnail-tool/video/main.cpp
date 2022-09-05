// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
