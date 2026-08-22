//
// Created by Ashish on 21-08-2026.
//

#ifndef RUBIKS_CORNERDBMAKER_H
#define RUBIKS_CORNERDBMAKER_H
#include "CornerPatternDatabase.h"
#include "../Model/RubiksCubeBitboard.cpp"

using namespace std;

class CornerDBMaker {
private:
    string fileName;
    CornerPatternDatabase cornerDB;

public:
    CornerDBMaker(string _fileName);
    CornerDBMaker(string _fileName, uint8_t init_val);

    bool bfsAndStore();
};


#endif //RUBIKS_CORNERDBMAKER_H