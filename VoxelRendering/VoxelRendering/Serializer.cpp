#include "Serializer.h"

//vector<vector<vector<Voxel>>> Serializer::deserizlize_from_xraw(string path) {
//    ifstream input(path, std::ios::binary);
//
//    // copies all data into buffer
//    vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
//    cout << "file type: " << char(buffer[0]) << char(buffer[1]) << char(buffer[2]) << char(buffer[3]) << "\n";
//    cout << "color channel data type: " << int(buffer[4]) << " - ";
//    switch (int(buffer[4])) {
//    case 0:
//        cout << "unsigned integer";
//        break;
//    case 1:
//        cout << "signed initeger";
//        break;
//    case 2:
//        cout << "float";
//        break;
//    }
//    cout << "\n";
//    cout << "num of color channels: " << int(buffer[5]) << " - ";
//    switch (int(buffer[5])) {
//    case 1:
//        cout << "R";
//        break;
//    case 2:
//        cout << "RG";
//        break;
//    case 3:
//        cout << "RGB";
//        break;
//    case 4:
//        cout << "RGBA";
//    }
//    cout << "\n";
//    cout << "bits per channel: " << int(buffer[6]) << "\n";
//    cout << "bits per index: " << int(buffer[7]) << "\n";
//
//    //cout << "width: " << i
//}