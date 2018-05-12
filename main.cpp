#include <iostream>
#include <fstream>
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
    Button_Ok,
    Button_Back,
    Button_ExitGame,
};

enum GameSceneType {
    GameScene_MainMenu = 0,
    GameScene_Game,
    GameScene_Highscores,
    GameScene_Victory,
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
    void (*mouseClickFunction)(int button, int state, int x, int y);
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
    int gameId;
} g_game;

struct Highscore {
    char playerName[16];
    Time time;
    int steps;
};
array<Highscore, 11> g_highscoresTable;

//================================GLOBAL VARIABLES================================//

float g_deltaTime = 0;
GameSceneType g_currentGameScene = GameScene_MainMenu;

//===============================RENDER FUNCTIONS===============================//
//-- Interface
void drawBackgroundSquareWithRounding(float x, float y, float width, float height);
void drawBackground();
void drawButton(const char* text, float x, float y, float width, float height, Buttons button_id);
void drawDrawTimeAndSteps();

void drawHighscoresTable(float x, float y);

//-- Other
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
//-- Timers
void animationTimer(int value);
void gameTimer(int value);

//======----- Main menu -----======//
void mouseClickFunction_MainMenu(int button, int state, int x, int y);
void mouseMotionFunction_MainMenu(int x, int y);
void renderer_MainMenu(void);

//======----- Game -----======//
void mouseClickFunction_Game(int button, int state, int x, int y);
void mouseMotionFunction_Game(int x, int y);
void renderer_Game(void);

//======----- Victory -----======//
void mouseClickFunction_Victory(int button, int state, int x, int y);
void mouseMotionFunction_Victory(int x, int y);
void renderer_Victory(void);
void keyboardFunction_Victory(unsigned char key, int x, int y);

//======----- Highscores -----======//
void mouseClickFunction_Highscores(int button, int state, int x, int y);
void mouseMotionFunction_Highscores(int x, int y);
void renderer_Highscores(void);



void mainRendererFunction();
void mainKeyboardFunction(unsigned char key, int x, int y);
void mainMouseClickFunction(int button, int state, int x, int y);
void mainMouseMotionFunction(int x, int y);
void mainResizeFunction(int width, int height);


//=============================LOGICK FUNCTIONS=============================//
void initializationGame();
void initializationGlut();
void initializationGl();

float getChipPos(int chip_x, int chip_y, float& pos_x, float& pos_y);
float convertWindowCordsToGlWorldCords(int window_x, int window_y, float& world_x, float& world_y);
ArrowDirection getChipMoveDirection(int chip_x, int chip_y);

void makeGame();
void makeMove();
void checkVictory();

void saveGame();
void loadGame();
void saveHighscoresTable();
void loadHighscoresTable();




//==============================REALIZATION==============================//
//======----- Main functions

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

    initializationGl();
    initializationGlut();
    initializationGame();

    g_currentGameScene = GameScene_Victory;
    glutMainLoop();


    return 0;
}

void mainRendererFunction()
{
    glClear(GL_COLOR_BUFFER_BIT);

    static int oldTime = 0;
    int newTime = glutGet(GLUT_ELAPSED_TIME);
    g_deltaTime = 0.01f * float(newTime - oldTime);
    oldTime = newTime;

    glLoadIdentity();

    if (g_gameScenes[g_currentGameScene].renderFunction)
    {
        g_gameScenes[g_currentGameScene].renderFunction();
    }

    glutSwapBuffers();
}

void mainKeyboardFunction(unsigned char key, int x, int y)
{
    if (g_gameScenes[g_currentGameScene].keyboardFunction)
    {
        g_gameScenes[g_currentGameScene].keyboardFunction(key, x, y);
    }
}

void mainMouseClickFunction(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && !g_gameMouse.awaitUpdate)
    {
        g_gameMouse.mouseClick = true;
    }
    else
    {
        g_gameMouse.mouseClick = false;
    }
    if (g_gameScenes[g_currentGameScene].mouseClickFunction)
    {
        g_gameScenes[g_currentGameScene].mouseClickFunction(button, state, x, y);
    }
}

void mainMouseMotionFunction(int x, int y)
{
    if (g_gameScenes[g_currentGameScene].mouseFunction)
    {
        g_gameScenes[g_currentGameScene].mouseFunction(x, y);
    }
}

void mainResizeFunction(int width, int height)
{
    glutReshapeWindow(g_width, g_height);
}


//======----- Main menu -----======//
void mouseClickFunction_MainMenu(int button, int state, int x, int y)
{
    if (!g_gameMouse.mouseClick)
    {
        return;
    }

    if (g_gameMouse.mouseOnButton)
    {
        switch(g_gameMouse.buttonId)
        {
        case Button_NewGame: {
            glutTimerFunc(1000, gameTimer, 0);
            g_gameMouse.buttonId = Button_NoButton;
            makeGame();
            g_currentGameScene = GameScene_Game;
        } break;
        case Button_LoadGame: {
            loadGame();
            glutTimerFunc(1000, gameTimer, 0);
            g_currentGameScene = GameScene_Game;
        } break;
        case Button_Highscores: {} break;
        case Button_ExitGame: {
            exit(0);
        } break;
        }
        if (g_gameMouse.buttonId == Button_NewGame)
        {
            makeGame();
        }
        else if (g_gameMouse.buttonId == Button_Back)
        {
            g_currentGameScene = GameScene_MainMenu;
        }
    }
}

void mouseMotionFunction_MainMenu(int x, int y)
{
    float pos_x = 0;
    float pos_y = 0;
    convertWindowCordsToGlWorldCords(x, y, pos_x, pos_y);

    memset(&g_gameMouse, 0, sizeof(g_gameMouse));

    if (pos_x >= -g_mainMenuButtonsWidth && pos_x <= g_mainMenuButtonsWidth)
    {
        if (pos_y >= g_mainMenuButtonsPosY[0] - g_mainMenuButtonsHeight
                && pos_y <= g_mainMenuButtonsPosY[0] + g_mainMenuButtonsHeight)
        {
            g_gameMouse.mouseOnButton = true;
            g_gameMouse.buttonId = Button_NewGame;
        }
        else if (pos_y >= g_mainMenuButtonsPosY[1] - g_mainMenuButtonsHeight
                && pos_y <= g_mainMenuButtonsPosY[1] + g_mainMenuButtonsHeight)
        {
            g_gameMouse.mouseOnButton = true;
            g_gameMouse.buttonId = Button_LoadGame;
        }
        else if (pos_y >= g_mainMenuButtonsPosY[2] - g_mainMenuButtonsHeight
                && pos_y <= g_mainMenuButtonsPosY[2] + g_mainMenuButtonsHeight)
        {
            g_gameMouse.mouseOnButton = true;
            g_gameMouse.buttonId = Button_Highscores;
        }
        else if (pos_y >= g_mainMenuButtonsPosY[3] - g_mainMenuButtonsHeight
                && pos_y <= g_mainMenuButtonsPosY[3] + g_mainMenuButtonsHeight)
        {
            g_gameMouse.mouseOnButton = true;
            g_gameMouse.buttonId = Button_ExitGame;
        }
    }
}

void renderer_MainMenu(void)
{
    drawBackground();

    int button_id = 0;
    drawButton("New game", 0, g_mainMenuButtonsPosY[button_id++],
               g_mainMenuButtonsWidth, g_mainMenuButtonsHeight, Button_NewGame);
    drawButton("Continue game", 0, g_mainMenuButtonsPosY[button_id++],
               g_mainMenuButtonsWidth, g_mainMenuButtonsHeight, Button_LoadGame);
    drawButton("Highscores", 0, g_mainMenuButtonsPosY[button_id++],
               g_mainMenuButtonsWidth, g_mainMenuButtonsHeight, Button_Highscores);
    drawButton("Exit from game", 0, g_mainMenuButtonsPosY[button_id++],
               g_mainMenuButtonsWidth, g_mainMenuButtonsHeight, Button_ExitGame);

    drawWintage();
}


//======----- Game -----======//
void mouseClickFunction_Game(int button, int state, int x, int y)
{
    if (!g_gameMouse.mouseClick)
    {
        return;
    }

    if (g_gameMouse.mouseOnCell)
    {
        makeMove();
    }
    else if (g_gameMouse.mouseOnButton)
    {
        if (g_gameMouse.buttonId == Button_NewGame)
        {
            makeGame();
        }
        else if (g_gameMouse.buttonId == Button_Back)
        {
            saveGame();
            g_currentGameScene = GameScene_MainMenu;
        }
    }
}

void mouseMotionFunction_Game(int x, int y)
{
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
    else if (pos_y >= g_gameButtonsPosY - g_gameButtonsHeight
             && pos_y <= g_gameButtonsPosY + g_gameButtonsHeight)
    {
        if (pos_x >= g_gameButtonsPosX[0] - g_gameButtonsWidth
                && pos_x <= g_gameButtonsPosX[0] + g_gameButtonsWidth)
        {
            g_gameMouse.mouseOnButton = true;
            g_gameMouse.buttonId = Button_NewGame;
        }
        else if (pos_x >= g_gameButtonsPosX[1] - g_gameButtonsWidth
                && pos_x <= g_gameButtonsPosX[1] + g_gameButtonsWidth)
        {
            g_gameMouse.mouseOnButton = true;
            g_gameMouse.buttonId = Button_Back;
        }
    }
}

void renderer_Game(void)
{
    //drawBottomPanel();
    drawDrawTimeAndSteps();
    drawBackground();
    drawChips();

    drawButton("New game", g_gameButtonsPosX[0], g_gameButtonsPosY,
               g_gameButtonsWidth, g_gameButtonsHeight, Button_NewGame);
    drawButton("Main menu", g_gameButtonsPosX[1], g_gameButtonsPosY,
               g_gameButtonsWidth, g_gameButtonsHeight, Button_Back);


    drawWintage();
}


//======----- Victory -----======//
void mouseClickFunction_Victory(int button, int state, int x, int y)
{

}

void mouseMotionFunction_Victory(int x, int y)
{

}

void renderer_Victory(void)
{
    drawBackground();

    drawHighscoresTable(0, 0.0f);
    drawDrawTimeAndSteps();

    drawButton("Yahooooo!", 0, g_victoryButtonsPosY,
               g_victoryButtonsWidth, g_victoryButtonsHeight, Button_Ok);

    drawWintage();
}

void keyboardFunction_Victory(unsigned char key, int x, int y)
{

}


//======----- Highscores -----======//
void mouseClickFunction_Highscores(int button, int state, int x, int y)
{

}

void mouseMotionFunction_Highscores(int x, int y)
{

}

void renderer_Highscores(void)
{

}


//===========
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

    drawBackgroundSquareWithRounding(pos_x, pos_y, g_cellSize, g_cellSize);
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


float convertWindowCordsToGlWorldCords(int window_x, int window_y, float& world_x, float& world_y)
{
    world_x = (2.0f * window_x - g_width) / g_width;
    world_y = (g_height - 2.0f * window_y) / g_height;
}

void gameTimer(int value)
{
    if (g_game.gameId != value)
    {
        return;
    }
    if (g_currentGameScene != GameScene_Game)
    {
        glutTimerFunc(1000, gameTimer, value);
        return;
    }

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

    if (g_game.time.seconds % 10 == 0)
    {
        saveGame();
    }

    glutTimerFunc(1000, gameTimer, value);
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

    memset(&g_game.time, 0, sizeof(g_game.time));
    g_game.steps = 0;

    g_game.gameId++;
    glutTimerFunc(1000, gameTimer, g_game.gameId);

    saveGame();
}


void drawDrawTimeAndSteps()
{
    char buffer[100] = {0};
    sprintf(buffer, "Time: %2.2d:%2.2d:%2.2d", g_game.time.hours, g_game.time.minutes, g_game.time.seconds);

    glColor3fv(g_infoTextColor);
    drawButton(buffer, -0.45f, -0.9f, 0.35f, 0.075f, Button_NoButton);

    sprintf(buffer, "Steps: %d", g_game.steps);
    drawButton(buffer, 0.45f, -0.9f, 0.35f, 0.075f, Button_NoButton);
}

//-- Draw display
void drawBackground()
{
    glColor3fv(g_fieldBackgroundColor);
    drawBackgroundSquareWithRounding(0, 0, g_fieldSize, g_fieldSize);
}

void drawButton(const char* text, float x, float y, float width, float height, Buttons button_id)
{
    if (g_gameMouse.mouseOnButton && button_id == g_gameMouse.buttonId)
    {
        glColor3fv(g_buttonColor_hover);
    }
    else
    {
        glColor3fv(g_buttonColor);
    }
    drawBackgroundSquareWithRounding(x, y, width, height);

    glColor3fv(g_infoTextColor);
    int textLen = strlen(text);
    float textPos_x = x - (1.0f - (1.0f / textLen)) + 0.75f;
    float textPos_y = y - 0.02;

    drawText(text, textPos_x, textPos_y, 1);
}

void drawHighscoresTable(float x, float y)
{
    glColor3fv(g_highscoresTableBackgroundColor);
    drawBackgroundSquareWithRounding(x, y, g_highscoresTableWidth, g_highscoresTableHeight);

    glColor3f(0.2f, 0.1f, 0.4f);
    char buffer[200];
    sprintf(buffer, "%3s%7s%16s%9s%12s%11s%s", "num", "", "player name","", "time", "", "steps");
    float start_x = x - g_highscoresTableWidth + g_cellsBetweenInterval;
    float start_y = y + g_highscoresTableHeight - 0.1;
    drawText(buffer, start_x, start_y, 1);

    start_y -= 0.1;
    for (int i = 0; i < 10; i++)
    {
        sprintf(buffer, "%3d", i + 1);
        drawText(buffer, start_x, start_y, 1);

        sprintf(buffer, "%-20s", g_highscoresTable[i].playerName);
        drawText(buffer, start_x + 0.2, start_y, 1);

        sprintf(buffer, "%2.2d:%2.2d:%2.2d",
                g_highscoresTable[i].time.hours,
                g_highscoresTable[i].time.minutes,
                g_highscoresTable[i].time.seconds);
        drawText(buffer, start_x + 0.865, start_y, 1);

        sprintf(buffer, "%d", g_highscoresTable[i].steps);
        drawText(buffer, start_x + 1.2, start_y, 1);

        start_y -= 0.08;
    }

    glBegin(GL_LINES);
    {
        float line_x_start = x - g_highscoresTableWidth + g_cellsBetweenInterval + 0.15;
        float line_x_end = line_x_start;
        float line_y_start = y + g_highscoresTableHeight;
        float line_y_end = y - g_highscoresTableHeight;
        glVertex2f(line_x_start, line_y_start);
        glVertex2f(line_x_end, line_y_end);

        line_x_start = line_x_end = line_x_end += 0.65;
        glVertex2f(line_x_start, line_y_start);
        glVertex2f(line_x_end, line_y_end);

        line_x_start = line_x_end = line_x_end += 0.37;
        glVertex2f(line_x_start, line_y_start);
        glVertex2f(line_x_end, line_y_end);

        line_x_start = x - g_highscoresTableWidth;
        line_x_end = x + g_highscoresTableWidth;

        line_y_start = line_y_end = y + g_highscoresTableHeight - 0.1275;

        for (int i = 0; i < 11; i++)
        {
            glVertex2f(line_x_start, line_y_start);
            glVertex2f(line_x_end, line_y_end);
            line_y_start = line_y_end = line_y_end - 0.08;
        }
    }
    glEnd();
}

void drawBackgroundSquareWithRounding(float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
    {
        glVertex2f(x - width + g_cellRoundingSize, y + height);
        glVertex2f(x - width + g_cellRoundingSize, y - height);
        glVertex2f(x + width - g_cellRoundingSize, y - height);
        glVertex2f(x + width - g_cellRoundingSize, y + height);

        glVertex2f(x - width, y + height - g_cellRoundingSize);
        glVertex2f(x - width, y - height + g_cellRoundingSize);
        glVertex2f(x + width, y - height + g_cellRoundingSize);
        glVertex2f(x + width, y + height - g_cellRoundingSize);
    }
    glEnd();

    // Top left
    glBegin(GL_TRIANGLE_STRIP);
    {
        float center_x = x - width + g_cellRoundingSize;
        float center_y = y + height - g_cellRoundingSize;

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
        float center_x = x - width + g_cellRoundingSize;
        float center_y = y - height + g_cellRoundingSize;

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
        float center_x = x + width - g_cellRoundingSize;
        float center_y = y - height + g_cellRoundingSize;

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
        float center_x = x + width - g_cellRoundingSize;
        float center_y = y + height - g_cellRoundingSize;

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


//=============================LOGICK FUNCTIONS=============================//
void initializationGame()
{
    loadHighscoresTable();
    g_currentGameScene = GameScene_MainMenu;
    memset(&g_game, 0, sizeof(g_game));

    // Initialize Main menu functions
    g_gameScenes[GameScene_MainMenu].renderFunction = renderer_MainMenu;
    g_gameScenes[GameScene_MainMenu].mouseFunction = mouseMotionFunction_MainMenu;
    g_gameScenes[GameScene_MainMenu].keyboardFunction = nullptr;
    g_gameScenes[GameScene_MainMenu].mouseClickFunction = mouseClickFunction_MainMenu;

    // Initialize Game functions
    g_gameScenes[GameScene_Game].renderFunction = renderer_Game;
    g_gameScenes[GameScene_Game].mouseFunction = mouseMotionFunction_Game;
    g_gameScenes[GameScene_Game].keyboardFunction = nullptr;
    g_gameScenes[GameScene_Game].mouseClickFunction = mouseClickFunction_Game;

    // Initialize Highscores functions
    g_gameScenes[GameScene_Highscores].renderFunction = renderer_Highscores;
    g_gameScenes[GameScene_Highscores].mouseFunction = mouseMotionFunction_Highscores;
    g_gameScenes[GameScene_Highscores].keyboardFunction = nullptr;
    g_gameScenes[GameScene_Highscores].mouseClickFunction = mouseClickFunction_Highscores;

    // Initialize Victory functions
    g_gameScenes[GameScene_Victory].renderFunction = renderer_Victory;
    g_gameScenes[GameScene_Victory].mouseFunction = mouseMotionFunction_Victory;
    g_gameScenes[GameScene_Victory].keyboardFunction = keyboardFunction_Victory;
    g_gameScenes[GameScene_Victory].mouseClickFunction = mouseClickFunction_Victory;
}

void initializationGlut()
{
    glutMouseFunc(mainMouseClickFunction);
    glutPassiveMotionFunc(mainMouseMotionFunction);
    glutDisplayFunc(mainRendererFunction);
    glutIdleFunc(mainRendererFunction);
    glutReshapeFunc(mainResizeFunction);
}

void initializationGl()
{
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.75f, 0.55f, 0.25f, 1.0f);
}


void saveGame()
{
    ofstream fout("save.bin", ios::binary);
    if (!fout)
    {
        return;
    }
    fout.write((char*)&g_game, sizeof(g_game));
    fout.close();
}

void loadGame()
{
    ifstream fin("save.bin", ios::binary);
    if (!fin)
    {
        makeGame();
        return;
    }
    fin.read((char*)&g_game, sizeof(g_game));
    fin.close();

    glutTimerFunc(1000, gameTimer, g_game.gameId);
}

void saveHighscoresTable()
{
    ofstream fout("highscores.bin", ios::binary);
    if (!fout)
    {
        return;
    }
    fout.write((char*)g_highscoresTable.data(), sizeof(Highscore) * 10);
    fout.close();
}

void loadHighscoresTable()
{
    ifstream fin("highscores.bin", ios::binary);
    if (!fin)
    {
        memset(g_highscoresTable.data(), 0, sizeof(Highscore) * 10);
        return;
    }
    fin.read((char*)g_highscoresTable.data(), sizeof(Highscore) * 10);
    fin.close();
}
