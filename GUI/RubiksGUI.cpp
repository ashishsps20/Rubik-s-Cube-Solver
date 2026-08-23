#include "RubiksGUI.h"
#include <rlgl.h>

RubiksGUI::RubiksGUI(GenericRubiksCube* cube) : cube(cube) {
    cameraRadius = 10.0f;
    cameraAngleX = 0.785f; // 45 degrees
    cameraAngleY = 0.785f; // 45 degrees
    lastMove = "None";
    
    isCalculating = false;
    currentState = GUIState::NORMAL;
    animProgress = 0.0f;
    currentMoveIndex = 0;
    cancel_solve = false;
    paintColor = static_cast<GenericRubiksCube::COLOR>(0); // 0 is WHITE
    solverFunc = nullptr;
    fullSolutionText = "";

    camera = { 0 };
    camera.position = (Vector3){ 6.0f, 6.0f, 6.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
}

void RubiksGUI::init() {
    InitWindow(800, 600, "Rubik's Cube Solver 3D");
    SetTargetFPS(60);
}

// Undefine Raylib color macros to prevent collision with GenericRubiksCube::COLOR enum
#undef WHITE
#undef GREEN
#undef RED
#undef BLUE
#undef ORANGE
#undef YELLOW
#undef BLACK

Color RubiksGUI::getRaylibColor(GenericRubiksCube::COLOR color) {
    switch (color) {
        case GenericRubiksCube::COLOR::WHITE: return (Color){ 255, 255, 255, 255 };
        case GenericRubiksCube::COLOR::GREEN: return (Color){ 0, 228, 48, 255 };
        case GenericRubiksCube::COLOR::RED: return (Color){ 230, 41, 55, 255 };
        case GenericRubiksCube::COLOR::BLUE: return (Color){ 0, 121, 241, 255 };
        case GenericRubiksCube::COLOR::ORANGE: return (Color){ 255, 161, 0, 255 };
        case GenericRubiksCube::COLOR::YELLOW: return (Color){ 253, 249, 0, 255 };
    }
    return (Color){ 0, 0, 0, 255 };
}

void RubiksGUI::drawSticker(GenericRubiksCube::FACE face, unsigned row, unsigned col, Color color) {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    
    // Size of a sticker
    float sizeX = 0.9f;
    float sizeY = 0.9f;
    float sizeZ = 0.9f;
    
    // Map row (0..2) and col (0..2) to physical coordinates (-1, 0, 1)
    float rc = col - 1.0f; // horizontal
    float rr = 1.0f - row; // vertical (row 0 is top, so +1 y)
    
    float dist = 1.51f; // Slightly more than 1.5 to avoid Z-fighting with inner core

    bool affected = false;
    if (currentState == GUIState::SOLVING) {
        affected = isStickerAffected(currentAnimMove, face, row, col);
    }
    if (affected) {
        rlPushMatrix();
        Vector3 axis = getMoveAxis(currentAnimMove);
        float angle = getMoveAngle(currentAnimMove, animProgress);
        rlRotatef(angle, axis.x, axis.y, axis.z);
    }

    Vector3 pos;
    Vector3 sz;

    switch (face) {
        case GenericRubiksCube::FACE::FRONT:
            pos = (Vector3){rc, rr, dist}; sz = (Vector3){sizeX, sizeY, 0.01f}; break;
        case GenericRubiksCube::FACE::BACK:
            pos = (Vector3){-rc, rr, -dist}; sz = (Vector3){sizeX, sizeY, 0.01f}; break;
        case GenericRubiksCube::FACE::LEFT:
            pos = (Vector3){-dist, rr, rc}; sz = (Vector3){0.01f, sizeY, sizeZ}; break;
        case GenericRubiksCube::FACE::RIGHT:
            pos = (Vector3){dist, rr, -rc}; sz = (Vector3){0.01f, sizeY, sizeZ}; break;
        case GenericRubiksCube::FACE::UP:
            pos = (Vector3){rc, dist, -rr}; sz = (Vector3){sizeX, 0.01f, sizeZ}; break;
        case GenericRubiksCube::FACE::DOWN:
            pos = (Vector3){rc, -dist, rr}; sz = (Vector3){sizeX, 0.01f, sizeZ}; break;
    }

    DrawCube(pos, sz.x, sz.y, sz.z, color);
    DrawCubeWires(pos, sz.x, sz.y, sz.z, (Color){0, 0, 0, 255});

    if (affected) {
        rlPopMatrix();
    }

    if (currentState == GUIState::PAINTING) {
        Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
        BoundingBox box = { 
            (Vector3){pos.x - sz.x/2, pos.y - sz.y/2, pos.z - sz.z/2},
            (Vector3){pos.x + sz.x/2, pos.y + sz.y/2, pos.z + sz.z/2}
        };
        RayCollision collision = GetRayCollisionBox(ray, box);
        if (collision.hit && collision.distance < closestHit.distance) {
            closestHit.hit = true;
            closestHit.distance = collision.distance;
            closestHit.face = face;
            closestHit.row = row;
            closestHit.col = col;
        }
    }
}

bool RubiksGUI::isStickerAffected(GenericRubiksCube::MOVE move, GenericRubiksCube::FACE face, unsigned row, unsigned col) {
    using FACE = GenericRubiksCube::FACE;
    using MOVE = GenericRubiksCube::MOVE;
    if (move == MOVE::F || move == MOVE::FPRIME || move == MOVE::F2) {
        if (face == FACE::FRONT) return true;
        if (face == FACE::UP && row == 2) return true;
        if (face == FACE::DOWN && row == 0) return true;
        if (face == FACE::LEFT && col == 2) return true;
        if (face == FACE::RIGHT && col == 0) return true;
    }
    if (move == MOVE::B || move == MOVE::BPRIME || move == MOVE::B2) {
        if (face == FACE::BACK) return true;
        if (face == FACE::UP && row == 0) return true;
        if (face == FACE::DOWN && row == 2) return true;
        if (face == FACE::LEFT && col == 0) return true;
        if (face == FACE::RIGHT && col == 2) return true;
    }
    if (move == MOVE::U || move == MOVE::UPRIME || move == MOVE::U2) {
        if (face == FACE::UP) return true;
        if (face == FACE::FRONT && row == 0) return true;
        if (face == FACE::BACK && row == 0) return true;
        if (face == FACE::LEFT && row == 0) return true;
        if (face == FACE::RIGHT && row == 0) return true;
    }
    if (move == MOVE::D || move == MOVE::DPRIME || move == MOVE::D2) {
        if (face == FACE::DOWN) return true;
        if (face == FACE::FRONT && row == 2) return true;
        if (face == FACE::BACK && row == 2) return true;
        if (face == FACE::LEFT && row == 2) return true;
        if (face == FACE::RIGHT && row == 2) return true;
    }
    if (move == MOVE::L || move == MOVE::LPRIME || move == MOVE::L2) {
        if (face == FACE::LEFT) return true;
        if (face == FACE::UP && col == 0) return true;
        if (face == FACE::DOWN && col == 0) return true;
        if (face == FACE::FRONT && col == 0) return true;
        if (face == FACE::BACK && col == 2) return true;
    }
    if (move == MOVE::R || move == MOVE::RPRIME || move == MOVE::R2) {
        if (face == FACE::RIGHT) return true;
        if (face == FACE::UP && col == 2) return true;
        if (face == FACE::DOWN && col == 2) return true;
        if (face == FACE::FRONT && col == 2) return true;
        if (face == FACE::BACK && col == 0) return true;
    }
    return false;
}

Vector3 RubiksGUI::getMoveAxis(GenericRubiksCube::MOVE move) {
    using MOVE = GenericRubiksCube::MOVE;
    if (move == MOVE::F || move == MOVE::FPRIME || move == MOVE::F2) return (Vector3){0, 0, -1};
    if (move == MOVE::B || move == MOVE::BPRIME || move == MOVE::B2) return (Vector3){0, 0, 1};
    if (move == MOVE::U || move == MOVE::UPRIME || move == MOVE::U2) return (Vector3){0, -1, 0};
    if (move == MOVE::D || move == MOVE::DPRIME || move == MOVE::D2) return (Vector3){0, 1, 0};
    if (move == MOVE::L || move == MOVE::LPRIME || move == MOVE::L2) return (Vector3){1, 0, 0};
    if (move == MOVE::R || move == MOVE::RPRIME || move == MOVE::R2) return (Vector3){-1, 0, 0};
    return (Vector3){0, 0, 0};
}

float RubiksGUI::getMoveAngle(GenericRubiksCube::MOVE move, float progress) {
    using MOVE = GenericRubiksCube::MOVE;
    float maxAngle = 90.0f;
    if (move == MOVE::FPRIME || move == MOVE::BPRIME || move == MOVE::UPRIME || 
        move == MOVE::DPRIME || move == MOVE::LPRIME || move == MOVE::RPRIME) {
        maxAngle = -90.0f;
    } else if (move == MOVE::F2 || move == MOVE::B2 || move == MOVE::U2 || 
               move == MOVE::D2 || move == MOVE::L2 || move == MOVE::R2) {
        maxAngle = 180.0f;
    }
    return maxAngle * progress;
}

bool RubiksGUI::isValidCube() {
    int counts[6] = {0};
    for (int f = 0; f < 6; f++) {
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                counts[(int)cube->getColor((GenericRubiksCube::FACE)f, r, c)]++;
            }
        }
    }
    for (int i = 0; i < 6; i++) {
        if (counts[i] != 9) return false;
    }
    using FACE = GenericRubiksCube::FACE;
    using COLOR = GenericRubiksCube::COLOR;
    if (cube->getColor(FACE::UP, 1, 1) != COLOR::WHITE) return false;
    if (cube->getColor(FACE::DOWN, 1, 1) != COLOR::YELLOW) return false;
    if (cube->getColor(FACE::LEFT, 1, 1) != COLOR::GREEN) return false;
    if (cube->getColor(FACE::RIGHT, 1, 1) != COLOR::BLUE) return false;
    if (cube->getColor(FACE::FRONT, 1, 1) != COLOR::RED) return false;
    if (cube->getColor(FACE::BACK, 1, 1) != COLOR::ORANGE) return false;
    return true;
}

void RubiksGUI::run() {
    while (!WindowShouldClose()) {
        // Manual Orbital Camera
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            cameraAngleX -= delta.x * 0.01f;
            cameraAngleY += delta.y * 0.01f;
            
            // Clamp pitch to avoid flipping upside down
            if (cameraAngleY > 1.5f) cameraAngleY = 1.5f;
            if (cameraAngleY < -1.5f) cameraAngleY = -1.5f;
        }

        // Apply spherical coordinates to camera position
        camera.position.x = cameraRadius * cos(cameraAngleY) * cos(cameraAngleX);
        camera.position.y = cameraRadius * sin(cameraAngleY);
        camera.position.z = cameraRadius * cos(cameraAngleY) * sin(cameraAngleX);

        // Check for modifier keys (Shift or Control)
        bool isPrime = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) || 
                       IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

        if (currentState == GUIState::NORMAL || currentState == GUIState::PAINTING) {
            // Check if background calculation is done
            if (isCalculating) {
                if (solveFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    solutionMoves = solveFuture.get();
                    isCalculating = false;
                    if (!solutionMoves.empty()) {
                        currentState = GUIState::SOLVING;
                        currentMoveIndex = 0;
                        animProgress = 0.0f;
                        currentAnimMove = solutionMoves[0];
                        
                        fullSolutionText = "Solution: ";
                        for (auto m : solutionMoves) {
                            fullSolutionText += GenericRubiksCube::getMove(m) + " ";
                        }
                    } else {
                        if (cube->isSolved()) {
                            lastMove = "Already Solved!";
                        } else {
                            lastMove = "No solution found!";
                            errorMessage = "Cube parity is physically impossible or too complex.";
                        }
                        fullSolutionText = "";
                    }
                }
            } else {
                // Toggle Paint Mode
                if (IsKeyPressed(KEY_E)) {
                    if (currentState == GUIState::NORMAL) currentState = GUIState::PAINTING;
                    else currentState = GUIState::NORMAL;
                    errorMessage = "";
                }

                // Handle Paint Color Selection
                if (currentState == GUIState::PAINTING) {
                    if (IsKeyPressed(KEY_ONE)) paintColor = GenericRubiksCube::COLOR::WHITE;
                    if (IsKeyPressed(KEY_TWO)) paintColor = GenericRubiksCube::COLOR::GREEN;
                    if (IsKeyPressed(KEY_THREE)) paintColor = GenericRubiksCube::COLOR::RED;
                    if (IsKeyPressed(KEY_FOUR)) paintColor = GenericRubiksCube::COLOR::BLUE;
                    if (IsKeyPressed(KEY_FIVE)) paintColor = GenericRubiksCube::COLOR::ORANGE;
                    if (IsKeyPressed(KEY_SIX)) paintColor = GenericRubiksCube::COLOR::YELLOW;
                }

                // Trigger solve
                if (IsKeyPressed(KEY_ENTER) && solverFunc) {
                    if (isValidCube()) {
                        isCalculating = true;
                        errorMessage = "";
                        cancel_solve = false;
                        solveFuture = std::async(std::launch::async, solverFunc, &cancel_solve);
                        currentState = GUIState::NORMAL;
                    } else {
                        errorMessage = "Invalid Cube! Cannot solve. Please fix colors.";
                    }
                }

                // Handle Manual Input (only in NORMAL mode)
                if (currentState == GUIState::NORMAL) {
                    if (IsKeyPressed(KEY_F)) { if (isPrime) { cube->fPrime(); lastMove = "F'"; } else { cube->f(); lastMove = "F"; } }
                    if (IsKeyPressed(KEY_B)) { if (isPrime) { cube->bPrime(); lastMove = "B'"; } else { cube->b(); lastMove = "B"; } }
                    if (IsKeyPressed(KEY_U)) { if (isPrime) { cube->uPrime(); lastMove = "U'"; } else { cube->u(); lastMove = "U"; } }
                    if (IsKeyPressed(KEY_D)) { if (isPrime) { cube->dPrime(); lastMove = "D'"; } else { cube->d(); lastMove = "D"; } }
                    if (IsKeyPressed(KEY_L)) { if (isPrime) { cube->lPrime(); lastMove = "L'"; } else { cube->l(); lastMove = "L"; } }
                    if (IsKeyPressed(KEY_R)) { if (isPrime) { cube->rPrime(); lastMove = "R'"; } else { cube->r(); lastMove = "R"; } }
                }
            }
        } else if (currentState == GUIState::SOLVING) {
            // Animation logic (1.2 seconds per move)
            animProgress += GetFrameTime() / 0.5f; // 0.5 sec animation
            if (animProgress >= 1.0f) {
                cube->move(currentAnimMove);
                lastMove = GenericRubiksCube::getMove(currentAnimMove);
                currentMoveIndex++;
                animProgress = 0.0f;
                if (currentMoveIndex < solutionMoves.size()) {
                    currentAnimMove = solutionMoves[currentMoveIndex];
                } else {
                    currentState = GUIState::NORMAL;
                }
            }
        }

        BeginDrawing();
        ClearBackground((Color){245, 245, 245, 255}); // RAYWHITE

        BeginMode3D(camera);

        // Draw central core cube (black plastic)
        DrawCube((Vector3){0.0f, 0.0f, 0.0f}, 2.9f, 2.9f, 2.9f, (Color){80, 80, 80, 255}); // DARKGRAY
        DrawCubeWires((Vector3){0.0f, 0.0f, 0.0f}, 2.9f, 2.9f, 2.9f, (Color){0, 0, 0, 255}); // BLACK

        // Reset HitResult for this frame
        if (currentState == GUIState::PAINTING) {
            closestHit.hit = false;
            closestHit.distance = 999999.0f;
        }

        // Draw faces
        GenericRubiksCube::FACE faces[] = {
            GenericRubiksCube::FACE::UP, GenericRubiksCube::FACE::LEFT, 
            GenericRubiksCube::FACE::FRONT, GenericRubiksCube::FACE::RIGHT, 
            GenericRubiksCube::FACE::BACK, GenericRubiksCube::FACE::DOWN
        };

        for (auto face : faces) {
            for (unsigned row = 0; row < 3; row++) {
                for (unsigned col = 0; col < 3; col++) {
                    Color c = getRaylibColor(cube->getColor(face, row, col));
                    drawSticker(face, row, col, c);
                }
            }
        }

        // Handle Mouse Click Paint
        if (currentState == GUIState::PAINTING && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (closestHit.hit) {
                cube->setColor(closestHit.face, closestHit.row, closestHit.col, paintColor);
                errorMessage = "";
            }
        }

        EndMode3D();

        DrawText("Controls:", 10, 10, 20, (Color){80, 80, 80, 255});
        DrawText("Hold Left Mouse to rotate camera", 10, 40, 20, (Color){130, 130, 130, 255});
        if (currentState == GUIState::PAINTING) {
            DrawText("PAINT MODE ACTIVE - Press E to Exit", 10, 70, 20, (Color){220, 0, 0, 255});
            DrawText("1:W, 2:G, 3:R, 4:B, 5:O, 6:Y. Click stickers to paint.", 10, 100, 20, (Color){130, 130, 130, 255});
            DrawRectangle(450, 95, 30, 30, getRaylibColor(paintColor));
            DrawRectangleLines(450, 95, 30, 30, (Color){0, 0, 0, 255});
        } else {
            DrawText("F, B, U, D, L, R to turn faces. Press E to Paint.", 10, 70, 20, (Color){130, 130, 130, 255});
            DrawText("Hold SHIFT (or CTRL) + Key for Prime (') moves", 10, 100, 20, (Color){130, 130, 130, 255});
        }
        DrawText("Press ENTER to Auto-Solve!", 10, 130, 20, (Color){0, 100, 200, 255}); // Blue
        
        // Error Message
        if (!errorMessage.empty()) {
            DrawText(errorMessage.c_str(), 10, 250, 20, (Color){255, 0, 0, 255});
        }

        // Display the last move so the user knows the input worked
        std::string moveText = "Last Move: " + lastMove;
        if (isCalculating) moveText = "Calculating Solution... Please Wait";
        DrawText(moveText.c_str(), 10, 160, 20, (Color){0, 150, 0, 255});
        
        // Display full solution and current step during and after solving
        if (currentState == GUIState::SOLVING || !fullSolutionText.empty()) {
            DrawText(fullSolutionText.c_str(), 10, 190, 20, (Color){220, 100, 0, 255}); // Orange
            if (currentState == GUIState::SOLVING) {
                std::string stepInfo = "Step " + std::to_string(currentMoveIndex + 1) + " of " + std::to_string(solutionMoves.size());
                DrawText(stepInfo.c_str(), 10, 220, 20, (Color){0, 100, 200, 255}); // Blue
            } else if (!isCalculating && lastMove != "Already Solved!") {
                DrawText("Solved!", 10, 220, 20, (Color){0, 150, 0, 255}); // Green
            }
        }

        EndDrawing();
    }
    cancel_solve = true; // Signal the solver thread to abort if window is closed
    CloseWindow();
}
