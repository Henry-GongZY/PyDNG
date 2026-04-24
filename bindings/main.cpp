//
// Created by Henrygongzy on 25-11-16.
//

#include "utils.h"
#include "dng.h"



int main() {
    std::string filePath = R"(C:\Users\Henrygongzy\Desktop\tt\IMG_20230102_075736_051.dng)";

    Dng dng;
    int errorCode = dng.Read(filePath, false);
    if (errorCode != dng_error_none) {
        std::cerr << "Error reading DNG file: " << errorCode << std::endl;
        return 1;
    }

    // 获取并打印图像尺寸信息
    DngMeta* info = dng.GetImageInfo();
    std::cout << "--- DNG Image Dimensions ---" << std::endl;
    std::cout << "Raw Width:      " << info->rawWidth << std::endl;
    std::cout << "Raw Height:     " << info->rawHeight << std::endl;
    std::cout << "Cropped Width:  " << info->width << std::endl;
    std::cout << "Cropped Height: " << info->height << std::endl;
    std::cout << "----------------------------" << std::endl;

    delete info; // 记得释放内存

    // 获取并打印白平衡信息
    std::vector<double> wb = dng.GetWhiteBalance();
    std::cout << "--- DNG White Balance ---" << std::endl;
    if (wb.empty()) {
        std::cout << "No white balance info found." << std::endl;
    } else {
        std::cout << "Neutral Vector: [";
        for (size_t i = 0; i < wb.size(); ++i) {
            std::cout << wb[i] << (i == wb.size() - 1 ? "" : ", ");
        }
        std::cout << "]" << std::endl;
    }
    std::cout << "-------------------------" << std::endl;
    
    return 0;
}

