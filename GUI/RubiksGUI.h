#ifndef RUBIKS_GUI_H
#define RUBIKS_GUI_H

#include "../Model/GenericRubiksCube.h"
#include <raylib.h>
#include <string>
#include <vector>
#include <functional>
#include <future>

class RubiksGUI {
private:
    enum class GUIState { NORMAL, SOLVING, PAINTING };
    GUIState currentState;

    struct HitResult {
        bool hit;
        float distance;
        GenericRubiksCube::FACE face;
        unsigned row, col;
    };
    HitResult closestHit;

    GenericRubiksCube* cube;
    Camera3D camera;
    float cameraRadius;
    float cameraAngleX;
    float cameraAngleY;
    std::string lastMove;

    // Painter State
    GenericRubiksCube::COLOR paintColor;
    std::string errorMessage;

    // Auto-solving state
    std::function<std::vector<GenericRubiksCube::MOVE>()> solverFunc;
    std::future<std::vector<GenericRubiksCube::MOVE>> solveFuture;
    bool isCalculating;
    std::vector<GenericRubiksCube::MOVE> solutionMoves;
    size_t currentMoveIndex;
    
    // Animation state
    float animProgress; // 0.0 to 1.0
    GenericRubiksCube::MOVE currentAnimMove;
    std::string fullSolutionText;

    // Helper to convert Model color to Raylib Color
    Color getRaylibColor(GenericRubiksCube::COLOR color);

    // Helper to draw a single face sticker
    void drawSticker(GenericRubiksCube::FACE face, unsigned row, unsigned col, Color color);

    // Helpers for animation math
    bool isStickerAffected(GenericRubiksCube::MOVE move, GenericRubiksCube::FACE face, unsigned row, unsigned col);
    Vector3 getMoveAxis(GenericRubiksCube::MOVE move);
    float getMoveAngle(GenericRubiksCube::MOVE move, float progress);
    
    // Cube Validator
    bool isValidCube();

public:
    RubiksGUI(GenericRubiksCube* cube);
    void setSolverFunc(std::function<std::vector<GenericRubiksCube::MOVE>()> func) { solverFunc = func; }
    void init();
    void run();
};

#endif // RUBIKS_GUI_H
