//
// Created by Henrygongzy on 25-11-16.
//

#include "dng.h"


int main() {
    std::string filePath = R"(C:\Users\Henrygongzy\Desktop\Projects\OpenSource\PyDNG\extern\sample_files\12_ImageStats_WeightedAverage.dng)";

    Dng dng;
    dng.Read(filePath, false);
    auto m = dng.GetImage(false);
    dng.SetImage(m, false);
    dng.Write("11.dng");

    return 0;
}
