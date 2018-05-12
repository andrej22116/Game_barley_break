#ifndef CONSTGLOBALVARIABLES_H
#define CONSTGLOBALVARIABLES_H

//================================GLOBAL VARIABLES================================//

// Window size
const int g_width = 600;
const int g_height = 600;

// Interface
const float g_fieldBackgroundColor[3] = { 0.8f, 0.8f, 0.8f };
const float g_fieldColor[3] = { 0.6f, 0.6f, 0.6f };

const float g_highscoresTableBackgroundColor[3] = { 0.8f, 0.7f, 0.6f };
const float g_highscoresTableWidth = 0.75f;
const float g_highscoresTableHeight = 0.5f;

const float g_fieldSize = 0.8f;

const float g_infoTextColor[3] = { 1, 1, 1 };


const float g_buttonColor[3] = { 0.3f, 0.5f, 0.5f };
const float g_buttonColor_hover[3] = { 1.0f, 0.5f, 0.5f };

//-- Decor
const float g_wintageColor[4] = { 0.937f, 0.855f, 0.651f, 0.3f };

// Cells
const float g_cellTextColor[3] = { 1, 1, 1 };
const float g_cellRoundingSize = 0.05;
const int g_cellRoundingInterval = 20;

const float g_cellsBetweenInterval = 0.05f;
const float g_cellSize = (g_fieldSize - (g_fieldSize * g_cellsBetweenInterval * 3.15)) / 4.0f;

const float g_cellArrowColor[3] = { 0.8f, 0.1f, 0.1f };
const float g_selectCellArrowColor[3] = { 0.8f, 0.1f, 0.5f };

// Buttons
const float g_mainMenuButtonsWidth = g_fieldSize - g_cellsBetweenInterval;
const float g_mainMenuButtonsHeight = 0.1f;
const float g_mainMenuButtonsPosY[] = {
    g_fieldSize - 0.1f - g_cellsBetweenInterval,
    g_fieldSize - 0.4f - g_cellsBetweenInterval,
    g_fieldSize - 0.7f - g_cellsBetweenInterval,
    -g_fieldSize + 0.1f + g_cellsBetweenInterval
};

const float g_gameButtonsWidth = 0.35f;
const float g_gameButtonsHeight = 0.075f;
const float g_gameButtonsPosY = g_fieldSize + 0.05f + g_cellsBetweenInterval;
const float g_gameButtonsPosX[] = {
    -g_fieldSize / 2 - 0.05f,
    g_fieldSize / 2 + 0.05f
};

const float g_victoryButtonsWidth = g_fieldSize - g_cellsBetweenInterval;
const float g_victoryButtonsHeight = 0.1f;
const float g_victoryButtonsPosY = -g_fieldSize + 0.1f + g_cellsBetweenInterval;

const char g_NumbersStrings[16][3] = {
    "", "1", "2", "3", "4", "5", "6", "7", "8",
    "9", "10", "11", "12", "13", "14", "15"
};

const float g_NumbersColors[16][4] = {
    { 0, 0, 0, 0 },
    { 0.3f, 0.5f, 0.5f, 1 },
    { 0.35f, 0.5f, 0.5f, 1 },
    { 0.4f, 0.5f, 0.5f, 1 },
    { 0.45f, 0.5f, 0.5f, 1 },
    { 0.5f, 0.5f, 0.5f, 1 },
    { 0.55f, 0.5f, 0.5f, 1 },
    { 0.6f, 0.5f, 0.5f, 1 },
    { 0.65f, 0.5f, 0.5f, 1 },
    { 0.7f, 0.5f, 0.5f, 1 },
    { 0.75f, 0.5f, 0.5f, 1 },
    { 0.8f, 0.5f, 0.5f, 1 },
    { 0.85f, 0.5f, 0.5f, 1 },
    { 0.9f, 0.5f, 0.5f, 1 },
    { 0.95f, 0.5f, 0.5f, 1 },
    { 1.0f, 0.5f, 0.5f, 1 }
};

const float g_animationSpeed = 0.7f;
const float g_animationBoostDownSpeed = 2.5f;

// Game
static const int g_VictoryMartix[4][4] = {
    { 1, 2, 3, 4 },
    { 5, 6, 7, 8 },
    { 9, 10, 11, 12},
    { 13, 14, 15, 0}
};

#endif // CONSTGLOBALVARIABLES_H
