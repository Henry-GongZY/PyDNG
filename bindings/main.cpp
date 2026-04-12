//
// Created by Henrygongzy on 25-11-16.
//

#include "utils.h"
#include "dng.h"



int main() {
    std::string filePath = R"(C:\Users\Henrygongzy\Desktop\Projects\OpenSource\PyDNG\extern\sample_files\12_ImageStats_WeightedAverage.dng)";

    Dng dng;
    dng.Read(filePath, false);
    auto m = dng.GetData(false);
    dng.SetData(m, false);
    dng.Write("11.dng");
    UTF8ToWChar(R"(123313)", 0, nullptr, 0);
    return 0;
}
