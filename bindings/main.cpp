//
// Created by Henrygongzy on 25-11-16.
//

#include "utils.h"
#include "dng.h"



int main() {
    std::string filePath = R"(C:\Users\Henrygongzy\Desktop\tt\IMG_20230102_075736_051.dng)";

    Dng dng;
    try {
        dng.Read(filePath, false);
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    // Get and print image dimension info
    DngMeta info = dng.GetImageInfo();
    std::cout << "--- DNG Image Dimensions ---" << std::endl;
    std::cout << "Raw Width:      " << info.rawWidth << std::endl;
    std::cout << "Raw Height:     " << info.rawHeight << std::endl;
    std::cout << "Cropped Width:  " << info.width << std::endl;
    std::cout << "Cropped Height: " << info.height << std::endl;
    std::cout << "----------------------------" << std::endl;

    // Get and print white balance info
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
