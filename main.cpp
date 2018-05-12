#include <iostream>
#include <array>
#include <algorithm>

#include <cmath>
#include <cstring>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "constglobalvariables.h"

using namespace std;

//======================================TYPES======================================//
enum ArrowDirection {
    TOP,
    LEFT,
    RIGHT,
    BOTTOM,
    NO_DIRECTION
};

enum Buttons {
    Button_NoButton,
    Button_NewGame,
    Button_LoadGame,
    Button_Highscores,
    Button_Back,
    Button_ExitGame,
    Button_Yes,
    Button_No
};

enum GameSceneType {
    GameScene_MainMenu = 0,
    GameScene_Game,
    GameScene_Highscores,
    GameScene_YesNo,
    GameScene_CountScenes
};

struct GameMouse {
    int x;
    int y;
    bool mouseOnCell;
    int cell_x;
    int cell_y;
    bool mouseOnButton;
    int buttonId;
    bool mouseClick;

    bool awaitUpdate;
} g_gameMouse;

struct GameScene {
    void (*renderFunction)(void);
    void (*mouseFunction)(int x, int y);
    void (*keyboardFunction)(unsigned char key, int x, int y);
} g_gameScenes[GameScene_CountScenes];

struct Time {
    int hours;
    int minutes;
    int seconds;
};

struct Animation {
    float status;
    float cell_moveSpeed_x;
    float cell_moveSpeed_y;
    float boost;
    int cell_number;
} g_animation;

struct Game {
    Time time;
    int steps;
    int gameField[4][4];
} g_game;

//================================GLOBAL VARIABLES================================//

float g_deltaTime = 0;

//===============================RENDER FUNCTIONS===============================//
void mainRenderer();

//-- Interface
void drawBackground();
void drawBottomPanel();
void drawDrawTimeAndSteps();

//-- Other
// Рисует клетку
void drawCell(float x, float y, float size);
// Рисует все фишки
void drawChips();
// Рисует фишку
void drawChip(int x, int y);

// Рисует текст
void drawText(const char* text, float x, float y, float size);
// Рисует стрелку
void drawArrow(int x, int y);

//--- Decor
void drawWintage();


//=============================CALLBACK FUNCTIONS=============================//
void mouseFuction(int x, int y);
void mouseClickFunction(int button, int state, int x, int y);
void mainResize(int width, int height);

//-- Timers
void gameLongTimer(int value);
void animationTimer(int value);
void gameTimer(int value);


//=============================LOGICK FUNCTIONS=============================//
void initialization();

float getChipPos(int chip_x, int chip_y, float& pos_x, float& pos_y);
float convertWindowCordsToGlWorldCords(int window_x, int window_y, float& world_x, float& world_y);
ArrowDirection getChipMoveDirection(int chip_x, int chip_y);

void makeGame();

void makeMove();
void checkVictory();





//==============================REALIZATION==============================//


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(g_width, g_height);


    if (!glutCreateWindow("Alena Slawgorodskaya: Pyatnashki"))
    {
        cout << "Error on create window!" << endl;
        return -1;
    }

    glutMouseFunc(mouseClickFunction);
    glutPassiveMotionFunc(mouseFuction);
    glutDisplayFunc(mainRenderer);
    glutIdleFunc(mainRenderer);
    glutReshapeFunc(mainResize);

    glutTimerFunc(20, gameTimer, 0);

    makeGame();

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.75f, 0.55f, 0.25f, 1.0f);
    glutMainLoop();


    return 0;
}

void mainRenderer()
{
    glClear(GL_COLOR_BUFFER_BIT);

    static int oldTime = 0;
    int newTime = glutGet(GLUT_ELAPSED_TIME);
    g_deltaTime = 0.01f * float(newTime - oldTime);
    oldTime = newTime;

    glLoadIdentity();

    drawBottomPanel();
    drawDrawTimeAndSteps();
    drawBackground();
    drawChips();
    drawWintage();

    glutSwapBuffers();
}

void mainResize(int width, int height)
{
    glutReshapeWindow(g_width, g_height);
}

void drawBackground()
{
    glColor3fv(g_fieldBackgroundColor);
    drawCell(0, 0, 0.8);
}


void drawCell(float x, float y, float size)
{
    // Center
    glBegin(GL_QUADS);
    {
        glVertex2f(x - size + g_cellRoundingSize, y + size);
        glVertex2f(x - size + g_cellRoundingSize, y - size);
        glVertex2f(x + size - g_cellRoundingSize, y - size);
        glVertex2f(x + size - g_cellRoundingSize, y + size);

        glVertex2f(x - size, y + size - g_cellRoundingSize);
        glVertex2f(x - size, y - size + g_cellRoundingSize);
        glVertex2f(x + size, y - size + g_cellRoundingSize);
        glVertex2f(x + size, y + size - g_cellRoundingSize);
    }
    glEnd();

    // Top left
    glBegin(GL_TRIANGLE_STRIP);
    {
        float center_x = x - size + g_cellRoundingSize;
        float center_y = y + size - g_cellRoundingSize;

        for (int i = 80; i <= 190; i += g_cellRoundingInterval)
        {
            float angle = float(i) * float(M_PI) / 180.0f;
            glVertex2f(center_x, center_y);
            glVertex2f(center_x + cos(angle) * g_cellRoundingSize,
                       center_y + sin(angle) * g_cellRoundingSize);
        }
    }
    glEnd();

    // Bottom left
    glBegin(GL_TRIANGLE_STRIP);
    {
        float center_x = x - size + g_cellRoundingSize;
        float center_y = y - size + g_cellRoundingSize;

        for (int i = 170; i <= 280; i += g_cellRoundingInterval)
        {
            float angle = float(i) * float(M_PI) / 180.0f;
            glVertex2f(center_x, center_y);
            glVertex2f(center_x + cos(angle) * g_cellRoundingSize,
                       center_y + sin(angle) * g_cellRoundingSize);
        }
    }
    glEnd();

    // Bottom right
    glBegin(GL_TRIANGLE_STRIP);
    {
        float center_x = x + size - g_cellRoundingSize;
        float center_y = y - size + g_cellRoundingSize;

        for (int i = 260; i <= 370; i += g_cellRoundingInterval)
        {
            float angle = float(i) * float(M_PI) / 180.0f;
            glVertex2f(center_x, center_y);
            glVertex2f(center_x + cos(angle) * g_cellRoundingSize,
                       center_y + sin(angle) * g_cellRoundingSize);
        }
    }
    glEnd();

    // Top right
    glBegin(GL_TRIANGLE_STRIP);
    {
        float center_x = x + size - g_cellRoundingSize;
        float center_y = y + size - g_cellRoundingSize;

        for (int i = -10; i <= 100; i += g_cellRoundingInterval)
        {
            float angle = float(i) * float(M_PI) / 180.0f;
            glVertex2f(center_x, center_y);
            glVertex2f(center_x + cos(angle) * g_cellRoundingSize,
                       center_y + sin(angle) * g_cellRoundingSize);
        }
    }
    glEnd();
}


void drawChips()
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            drawChip(x, y);
            drawArrow(x, y);
        }
    }
}

void drawChip(int x, int y)
{
    int chipNumber = g_game.gameField[y][x];
    glColor4fv(g_NumbersColors[chipNumber]);

    float pos_x = 0;
    float pos_y = 0;
    getChipPos(x, y, pos_x, pos_y);

    float textPos_x = chipNumber > 9 ? pos_x - 0.03 : pos_x - 0.015;
    float textPos_y = pos_y - 0.015;

    if (g_game.gameField[y][x] == g_animation.cell_number)
    {
        float move_x = g_animation.cell_moveSpeed_x * g_animation.status;
        float move_y = g_animation.cell_moveSpeed_y * g_animation.status;
        pos_x += move_x;
        pos_y += move_y;
        textPos_x += move_x;
        textPos_y += move_y;
    }

    drawCell(pos_x, pos_y, g_cellSize);
    glColor3fv(g_cellTextColor);
    drawText(g_NumbersStrings[chipNumber], textPos_x, textPos_y, 2);

}

void drawText(const char* text, float x, float y, float size)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    glLoadIdentity();
    glOrtho(0 ,g_width, 0, g_height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    int pos_x = (x + 1) * (g_width / 2);
    int pos_y = (y + 1) * (g_height / 2);

    glRasterPos2i(pos_x, pos_y);
    glScalef(1, size, size);
    int textLen = strlen(text);
    for(int i = 0; i < textLen; i++)
    {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,(int)text[i]);
    }

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

void drawArrow(int x, int y)
{
    ArrowDirection arrowDir = getChipMoveDirection(x, y);
    if (arrowDir == NO_DIRECTION)
    {
        return;
    }

    float pos_x = 0;
    float pos_y = 0;
    getChipPos(x, y, pos_x, pos_y);

    float arrowSize = g_cellSize - g_cellRoundingSize;

    if (g_game.gameField[y][x] == g_animation.cell_number)
    {
        pos_x += g_animation.cell_moveSpeed_x * g_animation.status;
        pos_y += g_animation.cell_moveSpeed_y * g_animation.status;
    }

    if (g_gameMouse.mouseOnCell
            && g_gameMouse.cell_x == x
            && g_gameMouse.cell_y == y)
    {
        glColor3fv(g_selectCellArrowColor);
    }
    else
    {
        glColor3fv(g_cellArrowColor);
    }

    glBegin(GL_TRIANGLES);
    {
        switch(arrowDir)
        {
        case TOP: {
            glVertex2f(pos_x, pos_y + g_cellSize + g_cellsBetweenInterval);
            glVertex2f(pos_x - arrowSize, pos_y + g_cellSize);
            glVertex2f(pos_x + arrowSize, pos_y + g_cellSize);
        } break;
        case LEFT: {
            glVertex2f(pos_x - g_cellSize - g_cellsBetweenInterval, pos_y);
            glVertex2f(pos_x - g_cellSize, pos_y - arrowSize);
            glVertex2f(pos_x - g_cellSize, pos_y + arrowSize);
        } break;
        case RIGHT: {
            glVertex2f(pos_x + g_cellSize + g_cellsBetweenInterval, pos_y);
            glVertex2f(pos_x + g_cellSize, pos_y + arrowSize);
            glVertex2f(pos_x + g_cellSize, pos_y - arrowSize);
        } break;
        case BOTTOM: {
            glVertex2f(pos_x, pos_y - g_cellSize - g_cellsBetweenInterval);
            glVertex2f(pos_x + arrowSize, pos_y - g_cellSize);
            glVertex2f(pos_x - arrowSize, pos_y - g_cellSize);
        } break;
        }
    }
    glEnd();
}

float getChipPos(int chip_x, int chip_y, float& pos_x, float& pos_y)
{
    pos_x = (-g_fieldSize + g_cellsBetweenInterval)
                + ((g_cellsBetweenInterval + g_cellSize * 2.0f) * chip_x) + g_cellSize;
    pos_y = (g_fieldSize - g_cellsBetweenInterval)
                - ((g_cellsBetweenInterval + g_cellSize * 2.0f) * chip_y) - g_cellSize;
}

ArrowDirection getChipMoveDirection(int chip_x, int chip_y)
{
    ArrowDirection arrowDir = NO_DIRECTION;
    if (g_game.gameField[chip_y][chip_x] == 0)
    {
        return arrowDir;
    }

    if (chip_x < 3 && g_game.gameField[chip_y][chip_x + 1] == 0)
    {
        arrowDir = RIGHT;
    }
    else if (chip_x > 0 && g_game.gameField[chip_y][chip_x - 1] == 0)
    {
        arrowDir = LEFT;
    }
    else if (chip_y < 3 && g_game.gameField[chip_y + 1][chip_x] == 0)
    {
        arrowDir = BOTTOM;
    }
    else if (chip_y > 0 && g_game.gameField[chip_y - 1][chip_x] == 0)
    {
        arrowDir = TOP;
    }

    return arrowDir;
}

void drawWintage()
{
    glColor4fv(g_wintageColor);
    glBegin(GL_QUADS);
    {
        glVertex2f(-1, 1);
        glVertex2f(-1, -1);
        glVertex2f(1, -1);
        glVertex2f(1, 1);
    }
    glEnd();
}


void mouseFuction(int x, int y)
{
    if (g_gameMouse.awaitUpdate)
    {
        return;
    }

    float pos_x = 0;
    float pos_y = 0;
    convertWindowCordsToGlWorldCords(x, y, pos_x, pos_y);

    memset(&g_gameMouse, 0, sizeof(g_gameMouse));

    if ( ( pos_x >= -g_fieldSize && pos_x <= g_fieldSize )
        && ( pos_y >= -g_fieldSize && pos_y <= g_fieldSize ) )
    {
        int cell_x = int((pos_x + g_fieldSize) / (g_fieldSize / 2.0f));
        int cell_y = int((g_fieldSize - pos_y) / (g_fieldSize / 2.0f));

        if (cell_x > 3 || cell_y > 3)
        {
            return;
        }

        float chip_x = 0;
        float chip_y = 0;
        getChipPos(cell_x, cell_y, chip_x, chip_y);

        if ( ( pos_x >= chip_x - g_cellSize && pos_x <= chip_x + g_cellSize )
            && ( pos_y >= chip_y - g_cellSize && pos_y <= chip_y + g_cellSize ) )
        {
            g_gameMouse.mouseOnCell = true;
            g_gameMouse.cell_x = cell_x;
            g_gameMouse.cell_y = cell_y;
        }

        return;
    }
}

void mouseClickFunction(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && !g_gameMouse.awaitUpdate)
    {
        g_gameMouse.mouseClick = true;
        g_gameMouse.awaitUpdate = true;
    }
    else
    {
        g_gameMouse.mouseClick = false;
    }
}


float convertWindowCordsToGlWorldCords(int window_x, int window_y, float& world_x, float& world_y)
{
    world_x = (2.0f * window_x - g_width) / g_width;
    world_y = (g_height - 2.0f * window_y) / g_height;
}


void gameTimer(int value)
{
    if (g_gameMouse.mouseClick)
    {
        g_gameMouse.mouseClick = false;
        if (g_gameMouse.mouseOnCell)
        {
            makeMove();
        }
    }


    g_gameMouse.awaitUpdate = false;
    glutTimerFunc(20, gameTimer, 0);
    glutPostRedisplay();
}

void gameLongTimer(int value)
{
    g_game.time.seconds++;
    if (g_game.time.seconds == 60)
    {
        g_game.time.minutes++;
        g_game.time.seconds = 0;
    }
    if (g_game.time.minutes == 60)
    {
        g_game.time.hours++;
        g_game.time.minutes = 0;
    }
    glutTimerFunc(1000, gameLongTimer, 0);
    glutPostRedisplay();
}


void makeMove()
{
    const int& x = g_gameMouse.cell_x;
    const int& y = g_gameMouse.cell_y;
    ArrowDirection arrowDir = getChipMoveDirection(x, y);
    if (arrowDir == NO_DIRECTION)
    {
        return;
    }

    float distance = g_cellSize * 2.0f + g_cellsBetweenInterval;

    g_animation.cell_moveSpeed_x = 0;
    g_animation.cell_moveSpeed_y = 0;
    switch(arrowDir)
    {
    case TOP: {
        g_game.gameField[y - 1][x] = g_game.gameField[y][x];
        g_animation.cell_moveSpeed_y = -distance;
    } break;
    case LEFT: {
        g_game.gameField[y][x - 1] = g_game.gameField[y][x];
        g_animation.cell_moveSpeed_x = distance;
    } break;
    case RIGHT: {
        g_game.gameField[y][x + 1] = g_game.gameField[y][x];
        g_animation.cell_moveSpeed_x = -distance;
    } break;
    case BOTTOM: {
        g_game.gameField[y + 1][x] = g_game.gameField[y][x];
        g_animation.cell_moveSpeed_y = distance;
    } break;
    }

    g_game.steps++;

    g_animation.status = 1;
    g_animation.boost = 1;
    g_animation.cell_number = g_game.gameField[y][x];
    glutTimerFunc(1, animationTimer, 0);

    g_game.gameField[y][x] = 0;
    checkVictory();
}

void checkVictory()
{
    if (memcmp(g_game.gameField, g_VictoryMartix, sizeof(int) * 16) == 0)
    {
        cout << "Pabedaaaa!!!!" << endl;
    }
}

void animationTimer(int value)
{
    g_animation.status -= g_animationSpeed * g_deltaTime * pow(g_animation.boost, 2);
    if (g_animation.status <= 0.1 && g_animation.boost >= 0.5)
    {
        g_animation.boost *= g_animationBoostDownSpeed * g_deltaTime;
    }
    if (g_animation.status > 0)
    {
        glutTimerFunc(1, animationTimer, 0);
    }
    else
    {
        g_animation.status = 0;
        g_animation.cell_number = -1;
    }
    glutPostRedisplay();
}

void makeGame()
{
    array<int, 16> matrixLine;
    memcpy(matrixLine.data(), g_VictoryMartix, sizeof(int) * 16);
    srand(time(0));
    std::random_shuffle(matrixLine.begin(), matrixLine.end() - 1);

    int inversionsAmount = 0;
    for (int i = 0; i < 14; i++)
    {
        for (int j = i + 1; j < 15; j++)
        {
            if (matrixLine[i] > matrixLine[j])
            {
                inversionsAmount++;
            }
        }
    }
    if (inversionsAmount % 2 == 1)
    {
        swap(matrixLine[4], matrixLine[5]);
    }
    memcpy(g_game.gameField, matrixLine.data(), sizeof(int) * 16);

    glutTimerFunc(1000, gameLongTimer, 0);
}


void drawBottomPanel()
{
    glBegin(GL_QUADS);
    {
        glColor3fv(g_infoBarColorBottom);
        glVertex2f(-1.5f, -0.95f);
        glVertex2f(-1.5f, -1);
        glVertex2f(1.5f, -1);
        glVertex2f(1.5f, -0.95f);

        glColor3fv(g_infoBarColor);
        glVertex2f(-1.5f, -0.95f);
        glVertex2f(1.5f, -0.95f);

        glColor3f(0.1f, 0.1f, 0.1f);
        glVertex2f(0.825f, -0.6f);
        glVertex2f(-0.825f, -0.6f);
    }
    glEnd();
}

void drawDrawTimeAndSteps()
{
    char buffer[100] = {0};
    sprintf(buffer, "Time: %2.2d:%2.2d:%2.2d", g_game.time.hours, g_game.time.minutes, g_game.time.seconds);

    glColor3fv(g_infoTextColor);
    drawText(buffer, -0.9, -0.9, 1);

    sprintf(buffer, "Steps: %d", g_game.steps);
    drawText(buffer, 0.6, -0.9, 1);
}
