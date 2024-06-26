#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <thread>
#include <conio.h>
#include <cctype>
#include <mmsystem.h>
#include <string>
#include <fstream>
#include <iomanip>
#include "fixconsolewindows.h"

#pragma comment(lib, "winmm.lib")

using namespace std;

struct HighScore {
    string username;
    int score;
    int level;
    char date[11];
};

void FixConsoleWindow() {
    HWND consoleWindow = GetConsoleWindow();
    LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
    style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
    SetWindowLong(consoleWindow, GWL_STYLE, style);
}

void GotoXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setColor(WORD color)
{
    HANDLE hConsoleOutput;
    hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO screen_buffer_info;
    GetConsoleScreenBufferInfo(hConsoleOutput, &screen_buffer_info);

    WORD wAttributes = screen_buffer_info.wAttributes;
    color &= 0x000f;
    wAttributes &= 0xfff0;
    wAttributes |= color;

    SetConsoleTextAttribute(hConsoleOutput, wAttributes);
}

bool matchCoordinate(POINT A, POINT B) {
    return (A.x == B.x && A.y == B.y);
}

//Constants
#define MAX_SIZE_SNAKE 40
#define MAX_SIZE_FOOD 4
#define MAX_LEVEL 4
#define BLACK 240
#define LIGHTBLUE 243
#define RED 244
#define PURPLE 245
#define GOLDEN 246
#define GREY 247
#define GREEN 250
#define WHITE 255


//Global variables
POINT snake[MAX_SIZE_SNAKE]; //snake
POINT food[MAX_SIZE_FOOD]; // food
POINT gateIn; //snake will enter this gate
POINT gateOut; //snake will go from this gate when moving to next level
POINT dummyGate = { -10, -10 };
int numsOfWalls; //use with array wall[] to create the obstacles for each level
int Level; //use to determine the level at some point
int checkMoney = 0;
int checkBomb = 0;
POINT money;
POINT bombs;
int Food; //count food that the snake has eaten at current level
POINT wall[200]; //array of obstacles
int CHAR_LOCK;//used to determine the direction my snake cannot move (At a moment, there is one direction my snake cannot move to)
int MOVING;//used to determine the direction my snake moves (At a moment, there are three directions my snake can move)
int SPEED;// Standing for level, the higher the level, the quicker the speed
int HEIGHT_CONSOLE, WIDTH_CONSOLE;// Width and height of console-screen
int FOOD_INDEX; // current food-index
int SIZE_SNAKE; // size of snake, initially maybe 6 units and maximum size may be 10
int STATE; // State of snake: dead or alive
int SCORE;
string ID_STUDENT = "2312019323120194231201952312020023120209";
bool isPlayedGameOverSound;//use to determine whether the "game-over sound" is played or not
bool isNextLevel;//use to determine whether the entering gate is generated or not
int menu_choice;
bool gameIsPaused;
bool backgroundMusic;
int volume = 0;

bool IsValidFood(int x, int y) {
    //Body
    for (int i = 0; i < SIZE_SNAKE; i++) {
        if (snake[i].x == x && snake[i].y == y) {
            return false;
        }
    }

    //Wall
    for (int i = 0; i < numsOfWalls; i++) {
        if (wall[i].x == x && wall[i].y == y) {
            return false;
        }
    }

    return true;
}

void GenerateMoney() {

    if (Level == 1 || FOOD_INDEX != 2 || checkMoney >= 1)

        return;

    int x, y;

    srand(time(NULL));

    do {

        x = rand() % (WIDTH_CONSOLE - 1) + 1;

        y = rand() % (HEIGHT_CONSOLE - 1) + 1;

    } while (IsValidFood(x, y) == false);

    money = { x,y };

    setColor(GOLDEN);

    GotoXY(money.x, money.y);

    cout << '$';

    checkMoney++;

}



void GenerateBomb() {

    if (Level == 1 || Level == 2 || FOOD_INDEX != 1 || checkBomb >= 1)

        return;

    int x, y;

    srand(time(NULL));

    do {

        x = rand() % (WIDTH_CONSOLE - 1) + 1;

        y = rand() % (HEIGHT_CONSOLE - 2) + 2; //bomb's height == 2

    } while (IsValidFood(x, y) == false || IsValidFood(x, y - 1) == false);



    bombs = { x,y };

    setColor(RED);

    GotoXY(bombs.x, bombs.y);

    cout << (char)219;

    GotoXY(bombs.x, bombs.y - 1);

    cout << (char)218;

    checkBomb++;

}

void GenerateFood() {
    int x, y;
    srand(time(NULL));
    for (int i = 0; i < MAX_SIZE_FOOD; i++) {
        do {
            x = rand() % (WIDTH_CONSOLE - 1) + 1;
            y = rand() % (HEIGHT_CONSOLE - 1) + 1;
        } while (IsValidFood(x, y) == false);

        food[i] = { x,y };
    }
}

bool IsValidGate(int x, int y) {
    POINT point1, point2;
    point1.x = x - 1;
    point1.y = y + 1;

    point2.x = x + 1;
    point2.y = y + 1;


    for (int i = 0; i < SIZE_SNAKE; i++) {
        if (matchCoordinate(snake[i], point1) || matchCoordinate(snake[i], point2))
            return false;
    }

    return true;
}


void DrawGateIn() {
    //Draw
    int x = gateIn.x;
    int y = gateIn.y;
    setColor(RED);
    GotoXY(x - 1, y);
    cout << (char)201;
    setColor(GOLDEN);
    cout << 'O';
    setColor(RED);
    cout << (char)187;
    GotoXY(x - 1, y + 1);
    cout << (char)202;
    GotoXY(x + 1, y + 1);
    cout << (char)202;
}

void GenerateGateIn() {
    int x, y = 0; // Fix the gateIn on the top-border
    srand(time(NULL));

    do {
        x = rand() % (WIDTH_CONSOLE - 4) + 3;
    } while (!IsValidGate(x, y));

    //Save
    gateIn.x = x;
    gateIn.y = y;

    DrawGateIn();
}

void GenerateGateOut() {
    int x = gateOut.x = gateIn.x;
    int y = gateOut.y = HEIGHT_CONSOLE;

    //Draw
    setColor(RED);
    GotoXY(x - 1, y);
    cout << (char)200;
    setColor(GOLDEN);
    cout << 'O';
    setColor(RED);
    cout << (char)188;
    GotoXY(x + 1, y - 1);
    cout << (char)203;
    GotoXY(x - 1, y - 1);
    cout << (char)203;
    //setColor(WHITE);
}

void ClearGateIn() {
    int x = gateIn.x;
    int y = gateIn.y;
    setColor(GOLDEN);
    GotoXY(x - 1, y);
    cout << (char)205 << (char)205 << (char)205; //3 spaces
    setColor(WHITE);
    GotoXY(x - 1, y + 1);
    cout << " ";
    GotoXY(x + 1, y + 1);
    cout << " ";
}

void ClearGateOut() {
    int x = gateOut.x;
    int y = gateOut.y;
    setColor(GOLDEN);
    GotoXY(x - 1, y);
    cout << (char)205 << (char)205 << (char)205; //3 spaces
    setColor(WHITE);
    GotoXY(x - 1, y - 1);
    cout << " ";
    GotoXY(x + 1, y - 1);
    cout << " ";
}

//Sample
void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
    setColor(GOLDEN);
    GotoXY(x, y); cout << (char)201;
    for (int i = 1; i < width; i++)cout << (char)205;
    cout << (char)187;
    GotoXY(x, height + y); cout << (char)200;
    for (int i = 1; i < width; i++)cout << (char)205;
    cout << (char)188;
    for (int i = y + 1; i < height + y; i++) {
        GotoXY(x, i); cout << (char)186;
        GotoXY(x + width, i); cout << (char)186;
    }
    GotoXY(curPosX, curPosY);
}

void printInterface() {

    //print "SCORE" with ascii art
    ifstream ScoreFile("SCORE.txt");
    int y = 1;
    if (ScoreFile) {
        while (ScoreFile.good()) {
            string TempLine;
            getline(ScoreFile, TempLine);
            setColor(GREEN);
            GotoXY(WIDTH_CONSOLE + 5, y);
            cout << TempLine << endl;
            y++;
        }
    }
    ScoreFile.close();

    //print the board of score
    setColor(GREEN);
    y = 7;
    GotoXY(WIDTH_CONSOLE + 3, y); cout << (char)201;
    for (int i = 1; i < 44; i++) cout << (char)205;
    GotoXY(WIDTH_CONSOLE + 47, y); cout << (char)187;
    GotoXY(WIDTH_CONSOLE + 3, y + 9); cout << (char)200;
    for (int i = 1; i < 44; i++) cout << (char)205;
    cout << (char)188;
    for (int i = 8; i < y + 9; i++) {
        GotoXY(WIDTH_CONSOLE + 3, i); cout << (char)186;
        GotoXY(WIDTH_CONSOLE + 47, i); cout << (char)186;
    }

    //print "TEAM 2" with ascii art
    ifstream TeamFile("TEAM2.txt");
    y = HEIGHT_CONSOLE;
    if (TeamFile) {
        while (TeamFile.good()) {
            string TempLine;
            getline(TeamFile, TempLine);
            setColor(PURPLE);
            GotoXY(WIDTH_CONSOLE + 15, y);
            cout << TempLine << endl;
            y++;
        }
    }
    TeamFile.close();

    //print "DEVLOR" with ascii art
    ifstream DEVLORFile("DEVLOR.txt");
    y = HEIGHT_CONSOLE + 5;
    if (DEVLORFile) {
        while (DEVLORFile.good()) {
            string TempLine;
            getline(DEVLORFile, TempLine);
            setColor(PURPLE);
            GotoXY(WIDTH_CONSOLE + 6, y);
            cout << TempLine << endl;
            y++;
        }
    }
    DEVLORFile.close();

    //print "LEVEL 1"/"LEVEL 2"/"LEVEL 3" with ascii art
    switch (Level) {
    case 1: {
        ifstream Level1File("LEVEL1.txt");
        y = HEIGHT_CONSOLE + 2;
        if (Level1File) {
            while (Level1File.good()) {
                string TempLine;
                getline(Level1File, TempLine);
                setColor(LIGHTBLUE);
                GotoXY(10, y);
                cout << TempLine << endl;
                y++;
            }
        }
        Level1File.close();
        break;
    }
    case 2: {
        ifstream Level2File("LEVEL2.txt");
        y = HEIGHT_CONSOLE + 2;
        if (Level2File) {
            while (Level2File.good()) {
                string TempLine;
                getline(Level2File, TempLine);
                setColor(LIGHTBLUE);
                GotoXY(10, y);
                cout << TempLine << endl;
                y++;
            }
        }
        Level2File.close();
        break;
    }
    case 3: {
        ifstream Level3File("LEVEL3.txt");
        y = HEIGHT_CONSOLE + 2;
        if (Level3File) {
            while (Level3File.good()) {
                string TempLine;
                getline(Level3File, TempLine);
                setColor(LIGHTBLUE);
                GotoXY(10, y);
                cout << TempLine << endl;
                y++;
            }
        }
        Level3File.close();
        break;
    }
    case 4: {
        ifstream Level4File("LEVEL4.txt");
        y = HEIGHT_CONSOLE + 2;
        if (Level4File) {
            while (Level4File.good()) {
                string TempLine;
                getline(Level4File, TempLine);
                setColor(LIGHTBLUE);
                GotoXY(10, y);
                cout << TempLine << endl;
                y++;
            }
        }
        Level4File.close();
        break;
    }
    }

    //print snake with ascii art
    ifstream SnakeFile("SNAKE.txt");
    y = HEIGHT_CONSOLE + 2;
    if (SnakeFile) {
        while (SnakeFile.good()) {
            string TempLine;
            getline(SnakeFile, TempLine);
            setColor(LIGHTBLUE);
            GotoXY(45, y);
            cout << TempLine << endl;
            y++;
        }
    }
    TeamFile.close();
}

void printScore() {
    //Print current score (1 food = 10 score)
    GotoXY(WIDTH_CONSOLE + 8, 9);
    setColor(RED);
    cout << "SCORE:";
    GotoXY(WIDTH_CONSOLE + 40, 9);
    cout << SCORE * 10;

    //Print number of foods that the snake has eaten at current level
    GotoXY(WIDTH_CONSOLE + 8, 11);
    setColor(RED);
    cout << "FOOD:";
    GotoXY(WIDTH_CONSOLE + 40, 11);
    cout << Food << "/" << MAX_SIZE_FOOD;

    //Print current snake's length
    GotoXY(WIDTH_CONSOLE + 8, 13);
    setColor(RED);
    cout << "SNAKE'S LENGTH:";
    GotoXY(WIDTH_CONSOLE + 40, 13);
    cout << SIZE_SNAKE;

}

void printGameOverInterface() {

    //print "GAME OVER" with ascii art
    ifstream GameOverFile("GAMEOVER.txt");
    int y = 5;
    if (GameOverFile) {
        while (GameOverFile.good()) {
            string TempLine;
            getline(GameOverFile, TempLine);
            setColor(RED);
            GotoXY(8, y);
            cout << TempLine << endl;
            y++;
        }
    }
    GameOverFile.close();

    //print "CONTINUE" with ascii art
    ifstream ContinueFile("CONTINUE.txt");
    y = 12;
    if (ContinueFile) {
        while (ContinueFile.good()) {
            string TempLine;
            getline(ContinueFile, TempLine);
            setColor(RED);
            GotoXY(13, y);
            cout << TempLine << endl;
            y++;
        }
    }
    GotoXY(11, 16);
    cout << "[  Press 'Y'  ]";
    ContinueFile.close();

    //print "EXIT" with ascii art
    ifstream ExitFile("EXIT.txt");
    y = 12;
    if (ExitFile) {
        while (ExitFile.good()) {
            string TempLine;
            getline(ExitFile, TempLine);
            setColor(RED);
            GotoXY(45, y);
            cout << TempLine << endl;
            y++;
        }
    }
    GotoXY(43, 16);
    cout << "[ Press anykey ]";
    ExitFile.close();

}

void DrawFood(char ch) {
    //DRAW FOOD
    setColor(GREEN);
    GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
    cout << ch;
}

void DrawSnake(string str) {
    setColor(LIGHTBLUE);
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        if (snake[i].x < WIDTH_CONSOLE && snake[i].y < HEIGHT_CONSOLE) {
            GotoXY(snake[i].x, snake[i].y);
            cout << str[SIZE_SNAKE - i - 1];
        }
    }

    setColor(PURPLE);
    GotoXY(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y);
    cout << str[0];

    //setColor(WHITE);
}

void ClearSnake() {
    setColor(WHITE);
    for (int i = 0; i < SIZE_SNAKE; i++) {
        GotoXY(snake[i].x, snake[i].y);
        cout << " ";
    }
}

void ResetData() {
    //Initialize the global values //width 70 height 20
    CHAR_LOCK = 'A'; MOVING = 'D'; SPEED = 1; FOOD_INDEX = 0; WIDTH_CONSOLE = 70; HEIGHT_CONSOLE = 20; SIZE_SNAKE = 4; isPlayedGameOverSound = isNextLevel = false; Level = 1; numsOfWalls = 0; gateIn = gateOut = dummyGate; SCORE = 0; Food = 0;
    // Initialize default values for snake
    snake[0] = { 10, 5 }; snake[1] = { 11, 5 };
    snake[2] = { 12, 5 }; snake[3] = { 13, 5 };
    snake[4] = { 14, 5 }; snake[5] = { 15, 5 };
    //GenerateFood();//Create food array, already created in StartGame() func
}

void StartGame() {
    system("cls");
    ResetData(); //Initialize
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE); //Draw game
    printInterface();
    printScore();
    STATE = 1; //Start
    GenerateFood();
}

void ExitGame(HANDLE t) {
    system("cls");
    //TerminateThread(t, 0);
    exit(0);
}

void PauseGame(HANDLE t) {
    SuspendThread(t);
}

void setVolume()
{
    if (volume != 0)
        volume = 0;
    else
        volume = 65535;

    DWORD dwSpeakers = (DWORD)-1;
    DWORD dwVolume;
    BOOL bSuccess = FALSE;

    bSuccess = waveOutGetVolume((HWAVEOUT)dwSpeakers, &dwVolume);

    waveOutSetVolume((HWAVEOUT)dwSpeakers, (DWORD)volume);

}

void PlayBGM() {
    while (true) {
        if (backgroundMusic == true) {
            PlaySound(TEXT("BGM.wav"), NULL, SND_FILENAME | SND_ASYNC);
            backgroundMusic = false;
        }
    }
}

void PlayEatingSound() {
    PlaySound(TEXT("retro-coin-02.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void PlayGameOverSound() {
    PlaySound(TEXT("game-over.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void PlayChooseSound() {
    PlaySound(TEXT("choose"), NULL, SND_FILENAME | SND_ASYNC);
}

void PlayIntroSound() {
    PlaySound(TEXT("Intro.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void ClearFood() {
    GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
    cout << " ";
}

//Function to update global data
void Eat() {
    PlayEatingSound();
    SCORE++;
    Food++;
    snake[SIZE_SNAKE] = food[FOOD_INDEX];

    if (FOOD_INDEX == MAX_SIZE_FOOD - 1) { //Eat all the food at this level
        FOOD_INDEX = 0;
        isNextLevel = true;
        GenerateGateIn();
        SIZE_SNAKE++;
    }
    else {
        FOOD_INDEX++;
        SIZE_SNAKE++;
    }
    printScore();
}

void ClearSnakeAndFood() {
    setColor(WHITE);
    //DRAW FOOD
    GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
    cout << " ";

    //DRAW SNAKE
    for (int i = 0; i < SIZE_SNAKE; i++) {
        GotoXY(snake[i].x, snake[i].y);
        cout << " ";
    }
}

void BlinkSnake() {
    for (int j = 0; j < 5; j++) {
        Sleep(200);
        DrawSnake(ID_STUDENT);
        Sleep(100);
        ClearSnake();
    }
}

//Function to process the dead of snake
void ProcessDead() {
    if (!isPlayedGameOverSound) {
        PlayGameOverSound();
        isPlayedGameOverSound = true;
    }
    STATE = 2;
    BlinkSnake();
    ClearGateIn();
    ClearGateOut();
    //Clear the game board inside
    for (int y = 1; y < HEIGHT_CONSOLE; y++) {
        GotoXY(1, y);
        for (int x = 1; x < WIDTH_CONSOLE; x++) cout << " ";
    }
    printGameOverInterface();
}

void wallGeneration() {
    setColor(RED);
    numsOfWalls = 0;

    switch (Level) {
    case 2:
    {
        wall[0] = { 28,7 };
        for (int i = 28; i <= 42; i++) {
            wall[numsOfWalls++] = { i,7 };
            GotoXY(i, 7);
            cout << (char)254;
        }

        for (int i = 28; i <= 42; i++) {
            wall[numsOfWalls++] = { i, 13 };
            GotoXY(i, 13);
            cout << (char)254;
        }
        break;
    }
    case 3:
    {
        int j = 10;
        for (int i = 15; i <= 20; i++) {
            GotoXY(i, j);
            cout << (char)174;
            wall[numsOfWalls++] = { i, j-- };
        }
        for (int i = 16; i <= 20; i++) {
            GotoXY(i, i - 5);
            cout << (char)174;
            wall[numsOfWalls++] = { i,i - 5 };
        }
        for (int i = 50; i <= 55; i++) {
            GotoXY(i, i - 45);
            cout << (char)175;
            wall[numsOfWalls++] = { i,i - 45 };
        }
        j = 15;
        for (int i = 50; i <= 54; i++) {
            GotoXY(i, j);
            cout << (char)175;
            wall[numsOfWalls++] = { i, j-- };
        }
        break;
    }
    case 4:
    {
        for (int i = 10; i <= 60; i++) {
            if (i < 33 || i > 37) {
                GotoXY(i, 5);
                cout << (char)205;
                wall[numsOfWalls++] = { i, 5 };
            }
        }
        for (int i = 10; i <= 60; i++) {
            if (i < 33 || i > 37) {
                GotoXY(i, 15);
                cout << (char)205;
                wall[numsOfWalls++] = { i, 15 };
            }
        }
        for (int i = 6; i <= 14; i++) {
            if (i < 9 || i > 11) {
                GotoXY(10, i);
                cout << (char)186;
                wall[numsOfWalls++] = { 10,i };
            }
        }
        for (int i = 6; i <= 14; i++) {
            if (i < 9 || i > 11) {
                GotoXY(60, i);
                cout << (char)186;
                wall[numsOfWalls++] = { 60,i };
            }
        }
        for (int i = 8; i <= 14; i++) {
            GotoXY(20, i);
            cout << (char)186;
            wall[numsOfWalls++] = { 20,i };
        }
        for (int i = 8; i <= 14; i++) {
            GotoXY(40, i);
            cout << (char)186;
            wall[numsOfWalls++] = { 40,i };
        }
        for (int i = 6; i <= 12; i++) {
            GotoXY(30, i);
            cout << (char)186;
            wall[numsOfWalls++] = { 30,i };
        }
        for (int i = 6; i <= 12; i++) {
            GotoXY(50, i);
            cout << (char)186;
            wall[numsOfWalls++] = { 50,i };
        }
        GotoXY(60, 15);
        wall[numsOfWalls++] = { 60,15 };
        cout << (char)188;
        GotoXY(60, 5);
        wall[numsOfWalls++] = { 60,5 };
        cout << (char)187;
        GotoXY(10, 5);
        wall[numsOfWalls++] = { 10,5 };
        cout << (char)201;
        GotoXY(10, 15);
        wall[numsOfWalls++] = { 10,15 };
        cout << (char)200;
        GotoXY(30, 5);
        wall[numsOfWalls++] = { 30,5 };
        cout << (char)203;
        GotoXY(50, 5);
        wall[numsOfWalls++] = { 50,5 };
        cout << (char)203;
        GotoXY(20, 15);
        wall[numsOfWalls++] = { 20,15 };
        cout << (char)202;
        GotoXY(40, 15);
        wall[numsOfWalls++] = { 40,15 };
        cout << (char)202;
    }
    default:
        break;
    }

}

void Level_2(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
    system("cls");
    CHAR_LOCK = 'S', MOVING = 'W', SPEED = 2; FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGHT_CONSOLE = 20; isPlayedGameOverSound = false; isNextLevel = false; Level = 2; Food = 0; checkMoney = 0; checkBomb = 0;
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
    printInterface();
    printScore();
    STATE = 1;
    //ClearGateOut();
    GenerateGateOut();
    snake[SIZE_SNAKE - 1].x = gateOut.x;
    snake[SIZE_SNAKE - 1].y = gateOut.y - 1;
    //Set other coordinate to NULL
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        //snake[i].x = 888;
        //snake[i].y = 888;
        snake[i] = snake[SIZE_SNAKE - 1];
    }
    wallGeneration();
    GenerateFood();
}

void Level_3(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
    system("cls");
    CHAR_LOCK = 'S', MOVING = 'W', SPEED = 2; FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGHT_CONSOLE = 20; isPlayedGameOverSound = isNextLevel = false; Level = 3; Food = 0; checkMoney = 0; checkBomb = 0;
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
    printInterface();
    printScore();
    STATE = 1;
    GenerateGateOut();
    snake[SIZE_SNAKE - 1].x = gateOut.x;
    snake[SIZE_SNAKE - 1].y = gateOut.y - 1;
    //Set other coordinate to NULL
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        //snake[i].x = 888;
        //snake[i].y = 888;
        snake[i] = snake[SIZE_SNAKE - 1];
    }
    wallGeneration();
    GenerateFood();
}

void Level_4(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
    system("cls");
    CHAR_LOCK = 'S', MOVING = 'W', SPEED = 2; FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGHT_CONSOLE = 20; isPlayedGameOverSound = isNextLevel = false; Level = 4; Food = 0; checkMoney = 0; checkBomb = 0;
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
    printInterface();
    printScore();
    STATE = 1;
    GenerateGateOut();
    snake[SIZE_SNAKE - 1].x = gateOut.x;
    snake[SIZE_SNAKE - 1].y = gateOut.y - 1;
    //Set other coordinate to NULL
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        //snake[i].x = 888;
        //snake[i].y = 888;
        snake[i] = snake[SIZE_SNAKE - 1];
    }
    wallGeneration();
    GenerateFood();
}

void ChangeLevel() {
    switch (Level) {
    case 2:
        Level_2(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
        break;
    case 3:
        Level_3(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
        break;
    case 4:
        Level_4(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
        break;
    default:
        StartGame();
        break;
    }
}

void ProcessGate() {
    if (!matchCoordinate(dummyGate, gateIn) && matchCoordinate(snake[SIZE_SNAKE - 1], gateIn)) {
        Level += 1;
        PlaySound(TEXT("goodresult.wav"), NULL, SND_FILENAME | SND_ASYNC);
        //ClearSnakeAndFood();
        ChangeLevel();
    }
    else {
        if (!isNextLevel) {
            DrawFood((char)254);
            GenerateMoney();
            if (matchCoordinate(snake[SIZE_SNAKE - 1], money)) {
                PlaySound(TEXT("announcement-sound-4-21464.wav"), NULL, SND_FILENAME | SND_ASYNC);
                SCORE += 3;
                printScore();
            }
            GenerateBomb();
            if (matchCoordinate(snake[SIZE_SNAKE - 1], bombs) || (snake[SIZE_SNAKE - 1].x == bombs.x && snake[SIZE_SNAKE - 1].y == bombs.y - 1)) {
                PlaySound(TEXT("hq-explosion-6288.wav"), NULL, SND_FILENAME | SND_ASYNC);
                isPlayedGameOverSound = true;
                STATE = 0;
            }
        }
        DrawSnake(ID_STUDENT);
    }

    //Clear gateOut
    if (!matchCoordinate(dummyGate, gateOut)) {
        POINT temp = gateOut;
        temp.y--;
        if (matchCoordinate(snake[0], temp)) { //snake_tail
            ClearGateOut();
            gateIn = dummyGate;
            gateOut = dummyGate;
        }
    }

}

bool isExistedFileName(string fileName) {
    ifstream fileInp(".\\Data\\username.txt");

    string name = "";

    while (fileInp >> name) {
        if (name == fileName)
            return true;
    }

    fileInp.close();
    return false;
}

bool isValidFileName(string fileName) {
    for (int i = 0; i < fileName.length(); i++)
        if ((fileName[i] >= 32 && fileName[i] <= 47) || (fileName[i] >= 58 && fileName[i] <= 64)
            || (fileName[i] >= 91 && fileName[i] <= 96) || (fileName[i] >= 123))
            return false;
    
    return true;
}

void SaveData() {
    string FileName;

    int column = 30;
    int row = 8;
    int xgame = (WIDTH_CONSOLE / 2) - 15;
    int ygame = (HEIGHT_CONSOLE / 2) - 3;

    setColor(PURPLE);
    for (int i = 0; i < row; i++)
    {
        GotoXY(xgame, ygame + i);
        for (int j = 0; j < column; j++)
        {
            if (i == 0)
                cout << (unsigned char)220;
            else if (i == row - 1)
                cout << (unsigned char)223;
            else if (j == 0 || j == column - 1)
                cout << (unsigned char)219;
            else
                cout << " ";
        }
    }

    GotoXY(xgame + 9, ygame + 2);
    cout << "Save and Exit";
    GotoXY(xgame + 14, ygame + 5);
    GotoXY(xgame + 3, ygame + 3);
    cout << "Name: ";

    //Entering file name
    do
    {
        GotoXY(xgame + 9, ygame + 3);
        cin >> FileName;
        GotoXY(xgame + 3, ygame + 4);
        
        for (int i = 0; i < 22; i++)
            cout << " ";

        GotoXY(xgame + 3, ygame + 4);
        if (isExistedFileName(FileName))
            cout << "File existed, re-type!";
        else if (!isValidFileName(FileName))
            cout << "Invalid char, re-type!";
        if (FileName.length() > 15)
            cout << "Too long, re-type!";

        if (isExistedFileName(FileName) || !isValidFileName(FileName) || FileName.length() > 18)
        {
            GotoXY(xgame + 9, ygame + 3);
            for (int i = 0; i < 18; i++)
                cout << " ";
        }
    } while (isExistedFileName(FileName) || !isValidFileName(FileName) || FileName.length() > 18);
    
    //Saving data to files
    ofstream fo(".\\Data\\" + FileName);

    ofstream f_user;
    f_user.open(".\\Data\\username.txt", ios::app);
    f_user << FileName << endl;
    f_user.close();

    ///Highscore
    f_user.open(".\\Data\\highscore.txt", ios::app);
    f_user << FileName << " " << SCORE << " " << Level << " ";
    //Date
    time_t now = time(0);
    tm* ltm = localtime(&now);
    f_user << ltm->tm_mday << "/" << ltm->tm_mon << "/" << 1900 + ltm->tm_year << endl;
    f_user.close();

    fo << SCORE << endl;

    fo << Food << endl;//for printScore()

    fo << SIZE_SNAKE << " " << endl;

    for (int i = 0; i < SIZE_SNAKE; i++)
        fo << snake[i].x << " " << snake[i].y << endl;

    fo << FOOD_INDEX << endl;

    for (int i = 0; i < 4; i++)
        fo << food[i].x << " " << food[i].y << endl;

    fo << Level << endl;

    if (isNextLevel)
        fo << gateIn.x << endl;
    else
        fo << -10 << endl;

    fo << MOVING << endl;

    fo << SPEED << endl;

    fo.close();
}

void LoadData() {
    string FileName;
    int column = 30;
    int row = 8;
    int xgame = (WIDTH_CONSOLE / 2) - 15;
    int ygame = (HEIGHT_CONSOLE / 2) - 3;

    setColor(PURPLE);
    for (int i = 0; i < row; i++)
    {
        GotoXY(xgame, ygame + i);
        for (int j = 0; j < column; j++)
        {
            if (i == 0)
                cout << (unsigned char)220;
            else if (i == row - 1)
                cout << (unsigned char)223;
            else if (j == 0 || j == column - 1)
                cout << (unsigned char)219;
            else
                cout << " ";
        }
    }

    GotoXY(xgame + 11, ygame + 2);
    cout << "Load data";
    GotoXY(xgame + 14, ygame + 5);
    GotoXY(xgame + 3, ygame + 3);
    cout << "Name: ";

    do
    {
        GotoXY(xgame + 9, ygame + 3);
        cin >> FileName;
        if (!isExistedFileName(FileName))
        {
            GotoXY(xgame + 3, ygame + 4);
            cout << "Not existed user!";
            GotoXY(xgame + 9, ygame + 3);
            for (int i = 0; i < 18; i++)
                cout << " ";
        }
    } while (!isExistedFileName(FileName));

    //Start saved game
    system("cls");
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE); //Draw game
    STATE = 1; //Start

    ifstream fi(".\\Data\\" + FileName);

    fi >> SCORE;

    fi >> Food; //for printScore()

    fi >> SIZE_SNAKE;

    for (int i = 0; i < SIZE_SNAKE; i++)
        fi >> snake[i].x >> snake[i].y;
    DrawSnake(ID_STUDENT);

    fi >> FOOD_INDEX;
    for (int i = 0; i < MAX_SIZE_FOOD; i++)
        fi >> food[i].x >> food[i].y;

    fi >> Level;
    wallGeneration(); //only call when Level is updated

    int gateInX;

    fi >> gateInX;

    if (gateInX != -10) { //gateIn exist, or isNextLevel == true
        isNextLevel = true;
        gateIn.x = gateInX;
        gateIn.y = 0;
        DrawGateIn();
    }
    else {
        isNextLevel = false;
        DrawFood((char)254); //Case FOOD_INDEX == 0 when eat all 4 foods
    }

    fi >> MOVING;

    fi >> SPEED;

    printInterface();
    printScore();

    fi.close();
}

void printASCII(string fileName, int x, int y)
{
    string line = "";
    ifstream inFile;
    inFile.open(fileName);
    if (inFile.is_open()) {
        while (getline(inFile, line)) {
            GotoXY(x, y++);
            cout << line << endl;
        }
    }
    else
        cout << "File failed to load" << endl;
    inFile.close();
}

void clearfile(string filename) {
    ofstream file;
    file.open(filename, ofstream::out | ofstream::trunc);
    file.close();
}

int DrawScoreBoard(int cornerX, int cornerY, int width, int height) {

    system("cls");

    fstream file;
    string Name;

    //Calculate the number of users
    file.open(".\\Data\\username.txt", ios::in);
    int x = 0; //number of users

    while (file >> Name) {
        x++;
    }
    Name = "";
    file.close();

    file.open(".\\Data\\highscore.txt", ios::in);
    
    //file.seekg(0, ios_base::end);
    //int x = file.tellg();
    //x = x / 29;

    //emty file ?
    if (x == 0) {
        DrawBoard(cornerX, cornerY, WIDTH_CONSOLE, HEIGHT_CONSOLE);
        printASCII("highscoreText.txt", cornerX + 9, cornerY - 4);
        printASCII("highscoreMess.txt", 1, 7);
        GotoXY(60, 14); cout << "Nothing here :(";
        GotoXY(55, 15); cout << "Press ESC to  back to MENU";
        file.close();
        char ch1;
        while (1) {
            if (_kbhit()) {
                ch1 = _getch();
                if (ch1 == 27) {
                    PlaySound(TEXT("choose.wav"), NULL, SND_SYNC);
                    break;
                }
            }
        }
        system("cls");
        return 1; //End
    }

    if (x == 16) x = 15; //Limit the total users appearing in the highscore board
    //Sorting HighScore before printing
    HighScore a[15]; ///Why initialize = {0} causing error?
    for (int i = 0; i < x; i++) {
        file >> a[i].username >> a[i].score >> a[i].level;
        file.ignore();
        file.getline(a[i].date, 11);
    }

    for (int i = 0; i < x - 1; i++) {
        for (int j = i + 1; j < x; j++) {
            if (a[i].score < a[j].score) {
                HighScore temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        }
    }
    file.close();
    
    GotoXY(cornerX + 5, cornerY + 2);
    int space = width / 5 - 1;
    cout << setw(space + 1) << left << "Top"
        << setw(space) << left << "Name"
        << setw(space + 3) << left << "Score"
        << setw(space) << left << "Lv"
        << setw(space) << left << "Date";

    //file.seekg(0, ios_base::beg);
    for (int i = 0; i < x; i++) {
        //file >> Name >> Score >> Level;
        //file.seekg(2, ios_base::cur);
        //file.getline(date, 11);
        GotoXY(cornerX + 5, cornerY + 4 + i);
        cout << setw(space) << left << i + 1
            << setw(space + 2) << left << a[i].username
            << setw(space + 3) << left << a[i].score * 10
            << setw(space - 2) << left << a[i].level
            << setw(space) << left << a[i].date;
    }
    // draw frame of load board
    setColor(BLACK);
    int c = 0;
    int space1 = width / 5;
    DrawBoard(cornerX, cornerY, width, height);
    DrawBoard(cornerX, cornerY, width, 3);
    do
    {
        c += space1;
        DrawBoard(cornerX + c, cornerY, width / 5, height);
    } while (c < WIDTH_CONSOLE / 2);

    GotoXY(cornerX, cornerY + 3); cout << char(204);
    GotoXY(cornerX + width, cornerY + 3); cout << char(185);
    c = 1;
    while (c <= 4)
    {
        GotoXY(cornerX + c * space1, cornerY); cout << char(203);
        GotoXY(cornerX + c * space1, cornerY + 3); cout << char(206);
        GotoXY(cornerX + c * space1, cornerY + height); cout << char(202);
        c++;
    }
    setColor(BLACK);
    printASCII("highscoreText.txt", cornerX + 9, cornerY - 4);
    printASCII("highscoreMess.txt", 1, 7);
    char ch1;
    while (1) {
        if (_kbhit()) {
            ch1 = _getch();
            if (ch1 == 27) {
                PlaySound(TEXT("choose.wav"), NULL, SND_SYNC);
                break;
            }
            else if (ch1 == 'q' || ch1 == 'Q') {
                PlaySound(TEXT("choose.wav"), NULL, SND_SYNC);
                clearfile(".//Data//highscore.txt");
                clearfile(".//Data//username.txt");
                break;
            }
        }
    }
    system("cls");
    return 0;
}

bool hitObstacle(POINT newPoint) {
    //Enter Gate
    if (matchCoordinate(newPoint, gateIn))
        return false;

    //Crash Border
    if (newPoint.x == 0 || newPoint.x == WIDTH_CONSOLE || newPoint.y == 0 || newPoint.y == HEIGHT_CONSOLE)
        return true;

    //Crash Body
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        if (matchCoordinate(newPoint, snake[i]))
            return true;
    }

    //Crash Gate
    POINT point1, point2;
    point1.x = gateIn.x - 1;
    point1.y = gateIn.y + 1;

    point2.x = gateIn.x + 1;
    point2.y = gateIn.y + 1;

    if (matchCoordinate(snake[SIZE_SNAKE - 1], point1) || matchCoordinate(snake[SIZE_SNAKE - 1], point2))
        return true;

    //Crash Wall
    for (int i = 0; i < numsOfWalls; i++) {
        if (matchCoordinate(snake[SIZE_SNAKE - 1], wall[i]))
            return true;
    }

    return false;
}


//Functions for moving the snake
//snake[0] is the tail, snake[SIZE_SNAKE - 1] is the head
void MoveRight() {
    //Touch the "wall"
    // if (snake[SIZE_SNAKE - 1].x + 1 == WIDTH_CONSOLE) {
    //     ProcessDead();
    // }
    POINT temp = snake[SIZE_SNAKE - 1];
    temp.x += 1;
    if (hitObstacle(temp)) {
        //ProcessDead();
        STATE = 0;
    }

    //Move
    else {
        //Eat
        if (snake[SIZE_SNAKE - 1].x + 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y) {
            Eat();
        }
        //Draw snake when it moves
        for (int i = 0; i < SIZE_SNAKE - 1; i++) {
            snake[i].x = snake[i + 1].x;
            snake[i].y = snake[i + 1].y;
        }
        snake[SIZE_SNAKE - 1].x++;
    }
}

void MoveLeft() {
    //Touch the "wall"
    // if (snake[SIZE_SNAKE - 1].x - 1 == 0) {
    //     ProcessDead();
    // }
    POINT temp = snake[SIZE_SNAKE - 1];
    temp.x -= 1;
    if (hitObstacle(temp)) {
        //ProcessDead();
        STATE = 0;
    }

    //Move
    else {
        //Eat
        if (snake[SIZE_SNAKE - 1].x - 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y) {
            Eat();
        }
        //Draw snake when it moves
        for (int i = 0; i < SIZE_SNAKE - 1; i++) {
            snake[i].x = snake[i + 1].x;
            snake[i].y = snake[i + 1].y;
        }
        snake[SIZE_SNAKE - 1].x--;
    }
}

void MoveDown() {
    //Touch the "wall"
    // if (snake[SIZE_SNAKE - 1].y + 1 == HEIGHT_CONSOLE) {
    //     ProcessDead();
    // }
    POINT temp = snake[SIZE_SNAKE - 1];
    temp.y += 1;
    if (hitObstacle(temp)) {
        //ProcessDead();
        STATE = 0;
    }

    //Move
    else {
        //Eat
        if (snake[SIZE_SNAKE - 1].y + 1 == food[FOOD_INDEX].y && snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x) {
            Eat();
        }
        //Draw snake when it moves
        for (int i = 0; i < SIZE_SNAKE - 1; i++) {
            snake[i].x = snake[i + 1].x;
            snake[i].y = snake[i + 1].y;
        }
        snake[SIZE_SNAKE - 1].y++;
    }
}

void MoveUp() {
    //Touch the "wall"
    // if (snake[SIZE_SNAKE - 1].y - 1 == 0) {
    //     ProcessDead();
    // }
    POINT temp = snake[SIZE_SNAKE - 1];
    temp.y -= 1;
    if (hitObstacle(temp)) {
        //ProcessDead();
        STATE = 0;
    }

    //Move
    else {
        //Eat
        if (snake[SIZE_SNAKE - 1].y - 1 == food[FOOD_INDEX].y && snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x) {
            Eat();
        }
        //Draw snake when it moves
        for (int i = 0; i < SIZE_SNAKE - 1; i++) {
            snake[i].x = snake[i + 1].x;
            snake[i].y = snake[i + 1].y;
        }
        snake[SIZE_SNAKE - 1].y--;
    }
}

//Subfunction for thread
void ThreadFunc() {
    while (true) {
        if (STATE == 1) {
            ClearSnakeAndFood();
            switch (MOVING) {
            case 'A':
                MoveLeft();
                break;
            case 'D':
                MoveRight();
                break;
            case 'W':
                MoveUp();
                break;
            case 'S':
                MoveDown();
                break;
            }
            /*DrawFood((char)254);
            DrawSnake(ID_STUDENT);*/
            ProcessGate();
            Sleep(200 / SPEED);
        }
        else if (STATE == 0) {
            ProcessDead();
        }
    }
}

bool isValidKey(int key) {
    if ((key != CHAR_LOCK) && (key == 'A' || key == 'D' || key == 'W' || key == 'S'))
        return true;
    return false;
}

void setcursor(bool visible, DWORD size) // set bool visible = 0 - invisible, bool visible = 1 - visible
{
    if (size == 0)
    {
        size = 20;	// default cursor size Changing to numbers from 1 to 20, decreases cursor width
    }
    CONSOLE_CURSOR_INFO lpCursor;
    lpCursor.bVisible = visible;
    lpCursor.dwSize = size;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &lpCursor);
}

void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO     cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
}

void Draw_Menu(int choose)
{
    system("cls");
    system("color F0");
    switch (choose)
    {
    case 0:

        GotoXY(32, 10);
        cout << "BAT DAU";
        break;
    case 1:

        GotoXY(32, 10);
        cout << "OPTION";
        break;
    case 2:

        GotoXY(32, 10);
        cout << "SCORE";
        break;
    case 3:

        GotoXY(32, 10);
        cout << "EXIT";
        break;
    }

}

void ChooseMenu(int& choose)
{
    int temp = 0;
    Draw_Menu(choose);
    while (temp != 32)
    {
        if (_kbhit())
        {
            temp = toupper(_getch());
            if (temp == 'W')
                choose--;
            else if (temp == 'S')
                choose++;
            if (choose < 0)
                choose = 3;
            else if (choose > 3)
                choose = 0;
            Draw_Menu(choose);
        }
    }
}

int Choose_Option()
{
    system("cls");
    cout << "WILL BE UPDATED SOON!!";
    while (true)
    {
        if (_kbhit())
        {
            int temp = toupper(_getch());
            if (temp == 27)
                break;
        }
    }
    return 0;
}

int Choose_Score()
{
    system("cls");
    cout << "WILL BE UPDATED SOON!!";
    while (true)
    {
        if (_kbhit())
        {
            int temp = toupper(_getch());
            if (temp == 27)
                break;
        }
    }
    return 0;
}

void printLine(int n, char c) {
    for (int i = 0; i < n; i++) {
        cout << c;
    }
}

void printColumn(int n, char c, int x, int y) {
    GotoXY(x, y);

    for (int i = y; i < n + y; ++i) {
        cout << c;
        GotoXY(x, i + 1);
    }
}

void moveText(int x, int y, const string& text, int speed) {
    for (char c : text) {
        GotoXY(x++, y);
        cout << c;
        Sleep(speed);
    }
}

void drawSnake(int x_snake, int y_snake) {
    GotoXY(x_snake, y_snake++);
    cout << "    --..,_                     _,.--.";
    GotoXY(x_snake, y_snake++);
    cout << "       `'.'.                .'`__ o  `;__.      ";
    GotoXY(x_snake, y_snake++);
    cout << "          '.'.            .'.'`  '---'`  `";
    GotoXY(x_snake, y_snake++);
    cout << "            '.`'--....--'`.'";
    GotoXY(x_snake, y_snake++);
    cout << "              `'--....--'`";
}

void clearScreen(int x_snake, int y_snake) {
    for (int i = 0; i < 5; i++) {
        GotoXY(x_snake, y_snake + i);
        cout << "                                          "; // Clear the previous drawing
    }
}

void intro() {

    // Clear screen
    system("cls");
   

    int x = 20;
    int x_team = 33;
    int x_name = 50;
    int x_snake = 1;
    int y_snake = 22;
    int y_name = 26;
    int y = 1;

    //title
    setColor(LIGHTBLUE);
    GotoXY(x - 5, y++);
    cout << char(218);
    printLine(93, char(196));
    cout << char(191);
    GotoXY(x, y++);
    cout << " _______  ______   _______  ___ ___  _______     _______  _______  ___ ___  _______ ";
    GotoXY(x, y++);
    cout << "|   _   ||   _  \\ |   _   ||   Y   )|   _   |   |   _   ||   _   ||   Y   ||   _   |";
    GotoXY(x, y++);
    cout << "|   1___||.  |   ||.  1   ||.  1  / |.  1___|   |.  |___||.  1   ||.      ||.  1___|";
    GotoXY(x, y++);
    cout << "|____   ||.  |   ||.  _   ||.  _  \\ |.  __)_    |.  |   ||.  _   ||. \\_/  ||.  __)_ ";
    GotoXY(x, y++);
    cout << "|:  1   ||:  |   ||:  |   ||:  |   \\|:  1   |   |:  1   ||:  |   ||:  |   ||:  1   |";
    GotoXY(x, y++);
    cout << "|::.. . ||::.|   ||::.|:. ||::.| .  )::.. . |   |::.. . ||::.|:. ||::.|:. ||::.. . |";
    GotoXY(x, y++);
    cout << "`-------'`--- ---'`--- ---'`--- ---'`-------'   `-------'`--- ---'`--- ---'`-------'";
    GotoXY(x - 5, y++);
    cout << char(192);
    printLine(93, char(196));
    cout << char(217);
    Sleep(1000);

    //team
    setColor(RED);
    GotoXY(x_team, y++);
    cout << "   .-') _     ('-.   ('-.     _   .-')                    ";
    GotoXY(x_team, y++);
    cout << "  (  OO) )  _(  OO) ( OO ).-.( '.( OO )_                  ";
    GotoXY(x_team, y++);
    cout << "  /     '._(,------./ . --. / ,--.   ,--.)       .-----.  ";
    GotoXY(x_team, y++);
    cout << "  |'--...__)|  .---'| \\-.  \\  |   `.'   |       / ,-.   \\ ";


    GotoXY(x_team - 13, y++);
    printLine(13, char(196));
    cout << "  '--.  .--'|  |  .-'-'  |  | |         |       '-'  |  | ";
    printLine(13, char(196));

    GotoXY(x_team - 13, y++);
    printLine(13, char(196));
    cout << "     |  |  (|  '--.\\| |_.'  | |  |'.'|  |          .'  /  ";
    printLine(13, char(196));

    GotoXY(x_team, y++);
    cout << "     |  |   |  .--' |  .-.  | |  |   |  |        .'  /__  ";
    GotoXY(x_team, y++);
    cout << "     |  |   |  `---.|  | |  | |  |   |  |       |       | ";
    GotoXY(x_team, y++);
    cout << "     `--'   `------'`--' `--' `--'   `--'       `-------' ";

    //snake
    setColor(PURPLE);
    int count = 0;
    while (count < 78) {
        drawSnake(x_snake, y_snake);
        Sleep(40);
        clearScreen(x_snake, y_snake);
        x_snake++; // Move to the right
        count++;
    }

    string instructor = "INSTRUCTOR: TRUONG TOAN THINH";
    string university = "UNIVERSITY OF SCIENCE";
    moveText(x_name, y_name++, instructor, 50);
    moveText(x_name + 4, y_name++, university, 50);

    Sleep(2000);
    //PlaySound(NULL, NULL, SND_ASYNC);
   
    Sleep(500);
    system("cls");
}

void menu() {

    system("cls");

    

    int x = 48, y = 9;
    int xmove = 48;
    static int ymove = 9;
    int x_menu = 32;
    int y_menu = 1;
    char c;

    int x_snake1 = 4;
    int y_snake1 = 3;
    int x_snake2 = 100;
    int y_snake2 = 3;


    setColor(LIGHTBLUE);
    GotoXY(x_menu, y_menu++);
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177;
    cout << (char)178 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176;

    GotoXY(x_menu, y_menu++);
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << "     ";
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176;

    GotoXY(x_menu, y_menu++);
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << "     ";
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176;

    GotoXY(x_menu, y_menu++);
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)178 << (char)177 << (char)176 << " ";
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176;

    GotoXY(x_menu, y_menu++);
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << "     ";
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176;

    GotoXY(x_menu, y_menu++);
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << "     ";
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176;

    GotoXY(x_menu, y_menu++);
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)177 << (char)178 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)178 << (char)177;
    cout << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)178 << (char)177 << (char)176 << (char)176 << (char)177 << (char)178 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)219 << (char)178 << (char)177 << (char)176;

    //snake1
    GotoXY(x_snake1, y_snake1++);
    cout << "       __";
    GotoXY(x_snake1, y_snake1++);
    cout << "      {0O}";
    GotoXY(x_snake1, y_snake1++);
    cout << "      \\__/";
    GotoXY(x_snake1, y_snake1++);
    cout << "      /^/";
    GotoXY(x_snake1, y_snake1++);
    cout << "     ( (              ";
    GotoXY(x_snake1, y_snake1++);
    cout << "     \\_\\_____";
    GotoXY(x_snake1, y_snake1++);
    cout << "     (_______)";
    GotoXY(x_snake1, y_snake1++);
    cout << "    (_________()Oo";

    //snake2
    GotoXY(x_snake2, y_snake2++);
    cout << "           __";
    GotoXY(x_snake2, y_snake2++);
    cout << "          {O0}";
    GotoXY(x_snake2, y_snake2++);
    cout << "          \\__/";
    GotoXY(x_snake2, y_snake2++);
    cout << "           \\^\\";
    GotoXY(x_snake2, y_snake2++);
    cout << "            ) )        ";
    GotoXY(x_snake2, y_snake2++);
    cout << "      _____/_/";
    GotoXY(x_snake2, y_snake2++);
    cout << "     (_______)";
    GotoXY(x_snake2, y_snake2++);
    cout << "oO()_________)";


    setColor(GREY);
    GotoXY(x_menu, 8);
    printLine(59, char(254));
    GotoXY(x_menu, 29);
    printLine(59, char(254));
    GotoXY(x_menu, 9);
    printColumn(20, char(254), x_menu, 9);
    printColumn(20, char(254), x_menu + 58, 9);
    GotoXY(x_menu + 1, 27);
    printLine(57, char(196));
    GotoXY(x_menu + 2, 28);
    cout << "PRESS <W>/<S> TO MOVE UP/ DOWM. PRESS <ENTER> TO CHOOSE";


    int check = 0;
    while (true)
    {
        setColor(LIGHTBLUE);

        //new game
        GotoXY(x + 2, 9);
        cout << (char)201 << (char)187 << (char)201 << (char)201 << (char)205 << (char)187 << (char)203 << " " << (char)203 << "  " << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)201 << (char)205 << (char)187;
        GotoXY(x + 2, 10);
        cout << (char)186 << (char)186 << (char)186 << (char)186 << (char)185 << " " << (char)186 << (char)186 << (char)186 << "  " << (char)186 << " " << (char)203 << (char)204 << (char)205 << (char)185 << (char)186 << (char)186 << (char)186 << (char)186 << (char)185;
        GotoXY(x + 2, 11);
        cout << (char)188 << (char)200 << (char)188 << (char)200 << (char)205 << (char)188 << (char)200 << (char)202 << (char)188 << "  " << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)202 << (char)202 << " " << (char)202 << (char)200 << (char)205 << (char)188;

        //load game
        GotoXY(x + 1, 12);
        cout << (char)203 << "  " << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << "  " << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)201 << (char)205 << (char)187;
        GotoXY(x + 1, 13);
        cout << (char)186 << "  " << (char)186 << " " << (char)186 << (char)204 << (char)205 << (char)185 << (char)186 << (char)186 << (char)186 << "  " << (char)186 << " " << (char)203 << (char)204 << (char)205 << (char)185 << (char)186 << (char)186 << (char)186 << (char)186 << (char)185;
        GotoXY(x + 1, 14);
        cout << (char)202 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)202 << (char)205 << (char)202 << (char)188 << "  " << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)202 << (char)202 << " " << (char)202 << (char)200 << (char)205 << (char)188;

        //high score 
        GotoXY(x, 15);
        cout << (char)203 << " " << (char)203 << (char)203 << (char)201 << (char)205 << (char)187 << (char)203 << " " << (char)203 << "  " << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187;
        GotoXY(x, 16);
        cout << (char)204 << (char)205 << (char)185 << (char)186 << (char)186 << " " << (char)203 << (char)204 << (char)205 << (char)185 << "  " << (char)200 << (char)205 << (char)187 << (char)186 << "  " << (char)186 << " " << (char)186 << (char)204 << (char)203 << (char)188 << (char)186 << (char)185;
        GotoXY(x, 17);
        cout << (char)202 << " " << (char)202 << (char)202 << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)202 << "  " << (char)200 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << (char)202 << (char)200 << (char)205 << (char)200 << (char)205 << (char)188;

        //setting
        GotoXY(x + 4, 18);
        cout << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)201 << (char)203 << (char)187 << (char)203 << (char)201 << (char)187 << (char)201 << (char)201 << (char)205 << (char)187;
        GotoXY(x + 4, 19);
        cout << (char)200 << (char)205 << (char)187 << (char)186 << (char)185 << "  " << (char)186 << "  " << (char)186 << " " << (char)186 << (char)186 << (char)186 << (char)186 << (char)186 << " " << (char)203;
        GotoXY(x + 4, 20);
        cout << (char)200 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << " " << (char)202 << "  " << (char)202 << " " << (char)202 << (char)188 << (char)200 << (char)188 << (char)200 << (char)205 << (char)188;

        //introduction
        GotoXY(x - 2, 21);
        cout << (char)203 << (char)201 << (char)187 << (char)201 << (char)201 << (char)203 << (char)187 << (char)203 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)203 << " " << (char)203 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)203 << (char)201 << (char)205 << (char)187 << (char)201 << (char)187 << (char)201;
        GotoXY(x - 2, 22);
        cout << (char)186 << (char)186 << (char)186 << (char)186 << " " << (char)186 << " " << (char)204 << (char)203 << (char)188 << (char)186 << " " << (char)186 << " " << (char)186 << (char)186 << (char)186 << " " << (char)186 << (char)186 << "   " << (char)186 << " " << (char)186 << (char)186 << " " << (char)186 << (char)186 << (char)186 << (char)186;
        GotoXY(x - 2, 23);
        cout << (char)202 << (char)188 << (char)200 << (char)188 << " " << (char)202 << " " << (char)202 << (char)200 << (char)205 << (char)200 << (char)205 << (char)188 << (char)205 << (char)202 << (char)188 << (char)200 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << " " << (char)202 << " " << (char)202 << (char)200 << (char)205 << (char)188 << (char)188 << (char)200 << (char)188;


        //exit
        GotoXY(x + 8, 24);
        cout << (char)201 << (char)205 << (char)187 << (char)205 << (char)187 << " " << (char)203 << (char)203 << (char)201 << (char)203 << (char)187;
        GotoXY(x + 8, 25);
        cout << (char)186 << (char)185 << " " << (char)201 << (char)202 << (char)203 << (char)188 << (char)186 << " " << (char)186;
        GotoXY(x + 8, 26);
        cout << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)200 << (char)205 << (char)202 << " " << (char)202;

        if (_kbhit() == true)
        {
            c = _getch();
            if (c == 'w' || c == 'W')
            {
                ymove -= 3;
            }
            if (c == 's' || c == 'S')
            {
                ymove += 3;
            }
            if (c == '\r')
            {
                PlayChooseSound();
                if (ymove == 9)
                {
                    menu_choice = 1;
                    system("cls");
                    break;
                }
                else if (ymove == 12)
                {
                    menu_choice = 2;
                    system("cls");
                    break;
                }
                else if (ymove == 15)
                {
                    menu_choice = 3;
                    system("cls");
                    break;
                }
                else if (ymove == 18)
                {
                    menu_choice = 4;
                    system("cls");
                    break;
                }
                else if (ymove == 21)
                {
                    menu_choice = 5;
                    system("cls");
                    break;
                }
                else if (ymove == 24)
                {
                    menu_choice = 6;
                    system("cls");
                    break;
                }
            }
        }
        if (ymove < 9)
            ymove = 24;
        else if (ymove > 24)
            ymove = 9;
        if (ymove == 9)
        {
            setColor(PURPLE);
            GotoXY(xmove + 2, ymove);
            cout << (char)201 << (char)187 << (char)201 << (char)201 << (char)205 << (char)187 << (char)203 << " " << (char)203 << "  " << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)201 << (char)205 << (char)187;
            GotoXY(xmove + 2, ymove + 1);
            cout << (char)186 << (char)186 << (char)186 << (char)186 << (char)185 << " " << (char)186 << (char)186 << (char)186 << "  " << (char)186 << " " << (char)203 << (char)204 << (char)205 << (char)185 << (char)186 << (char)186 << (char)186 << (char)186 << (char)185;
            GotoXY(xmove + 2, ymove + 2);
            cout << (char)188 << (char)200 << (char)188 << (char)200 << (char)205 << (char)188 << (char)200 << (char)202 << (char)188 << "  " << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)202 << (char)202 << " " << (char)202 << (char)200 << (char)205 << (char)188;
        }
        if (ymove == 12)
        {
            setColor(PURPLE);
            GotoXY(xmove + 1, ymove);
            cout << (char)203 << "  " << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << "  " << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)201 << (char)205 << (char)187;
            GotoXY(xmove + 1, ymove + 1);
            cout << (char)186 << "  " << (char)186 << " " << (char)186 << (char)204 << (char)205 << (char)185 << (char)186 << (char)186 << (char)186 << "  " << (char)186 << " " << (char)203 << (char)204 << (char)205 << (char)185 << (char)186 << (char)186 << (char)186 << (char)186 << (char)185;
            GotoXY(xmove + 1, ymove + 2);
            cout << (char)202 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)202 << (char)205 << (char)202 << (char)188 << "  " << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)202 << (char)202 << " " << (char)202 << (char)200 << (char)205 << (char)188;

        }
        if (ymove == 15)
        {
            setColor(PURPLE);
            GotoXY(xmove, ymove);
            cout << (char)203 << " " << (char)203 << (char)203 << (char)201 << (char)205 << (char)187 << (char)203 << " " << (char)203 << "  " << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187;
            GotoXY(xmove, ymove + 1);
            cout << (char)204 << (char)205 << (char)185 << (char)186 << (char)186 << " " << (char)203 << (char)204 << (char)205 << (char)185 << "  " << (char)200 << (char)205 << (char)187 << (char)186 << "  " << (char)186 << " " << (char)186 << (char)204 << (char)203 << (char)188 << (char)186 << (char)185;
            GotoXY(xmove, ymove + 2);
            cout << (char)202 << " " << (char)202 << (char)202 << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)202 << "  " << (char)200 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << (char)202 << (char)200 << (char)205 << (char)200 << (char)205 << (char)188;

        }
        if (ymove == 18)
        {
            setColor(PURPLE);
            GotoXY(xmove + 4, ymove);
            cout << (char)201 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)201 << (char)203 << (char)187 << (char)203 << (char)201 << (char)187 << (char)201 << (char)201 << (char)205 << (char)187;
            GotoXY(xmove + 4, ymove + 1);
            cout << (char)200 << (char)205 << (char)187 << (char)186 << (char)185 << "  " << (char)186 << "  " << (char)186 << " " << (char)186 << (char)186 << (char)186 << (char)186 << (char)186 << " " << (char)203;
            GotoXY(xmove + 4, ymove + 2);
            cout << (char)200 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << " " << (char)202 << "  " << (char)202 << " " << (char)202 << (char)188 << (char)200 << (char)188 << (char)200 << (char)205 << (char)188;

        }
        if (ymove == 21)
        {
            setColor(PURPLE);
            GotoXY(xmove - 2, ymove);
            cout << (char)203 << (char)201 << (char)187 << (char)201 << (char)201 << (char)203 << (char)187 << (char)203 << (char)205 << (char)187 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)203 << " " << (char)203 << (char)201 << (char)205 << (char)187 << (char)201 << (char)203 << (char)187 << (char)203 << (char)201 << (char)205 << (char)187 << (char)201 << (char)187 << (char)201;
            GotoXY(xmove - 2, ymove + 1);
            cout << (char)186 << (char)186 << (char)186 << (char)186 << " " << (char)186 << " " << (char)204 << (char)203 << (char)188 << (char)186 << " " << (char)186 << " " << (char)186 << (char)186 << (char)186 << " " << (char)186 << (char)186 << "   " << (char)186 << " " << (char)186 << (char)186 << " " << (char)186 << (char)186 << (char)186 << (char)186;
            GotoXY(xmove - 2, ymove + 2);
            cout << (char)202 << (char)188 << (char)200 << (char)188 << " " << (char)202 << " " << (char)202 << (char)200 << (char)205 << (char)200 << (char)205 << (char)188 << (char)205 << (char)202 << (char)188 << (char)200 << (char)205 << (char)188 << (char)200 << (char)205 << (char)188 << " " << (char)202 << " " << (char)202 << (char)200 << (char)205 << (char)188 << (char)188 << (char)200 << (char)188;
        }
        if (ymove == 24)
        {
            setColor(PURPLE);
            GotoXY(xmove + 8, ymove);
            cout << (char)201 << (char)205 << (char)187 << (char)205 << (char)187 << " " << (char)203 << (char)203 << (char)201 << (char)203 << (char)187;
            GotoXY(xmove + 8, ymove + 1);
            cout << (char)186 << (char)185 << " " << (char)201 << (char)202 << (char)203 << (char)188 << (char)186 << " " << (char)186;
            GotoXY(xmove + 8, ymove + 2);
            cout << (char)200 << (char)205 << (char)188 << (char)202 << " " << (char)200 << (char)205 << (char)202 << " " << (char)202;

        }
        Sleep(100);
    }
}

int main() {
    system("color F0");
    fixconsolewindows();
    backgroundMusic = true;
    setVolume();
    SetConsoleCP(437);
    SetConsoleOutputCP(437);

    int temp;
    bool TurnOnThread = false;
    ///thread introSound(PlayIntroSound);
    ///HANDLE handle_Intro = introSound.native_handle();
    ///intro();
    //StartGame();
    //setcursor(0, 0);  //!!!
    //if (TurnOnThread == false)
    //{
    thread t1(ThreadFunc); //Create thread for snake
    HANDLE handle_t1 = t1.native_handle(); //Take handle of thread

    PauseGame(handle_t1);
    thread BGM(PlayBGM);
    HANDLE handle_BGM = BGM.native_handle();
    BGM.detach();

    //}

MENU:
    {

        menu();
        gameIsPaused = true;
        if (menu_choice == 1)
        {
            PlaySound(TEXT("startgame.wav"), NULL, SND_FILENAME | SND_ASYNC);
            StartGame();
            DrawSnake(ID_STUDENT);
            while (true) {
                temp = toupper(_getch()); // ???
                if (STATE == 1) {
                    if (temp == 'P') {
                        PauseGame(handle_t1);
                        gameIsPaused = false;
                    }

                    else if (temp == 'L') {
                        SaveData();
                        ExitGame(handle_t1);
                        return 0;
                    }

                    else if (temp == 'T') {
                        LoadData();
                    }

                    else if (temp == 27) { // 27 is ASCII value of 'ESC' key
                        if (gameIsPaused == true)
                        {
                            ExitGame(handle_t1);
                            return 0;
                        }
                        else
                        {
                            SaveData();
                            backgroundMusic = true;
                            gameIsPaused = true;
                            goto MENU;
                        }
                    }
                    else {
                        ResumeThread(handle_t1);

                        //Logic of snake's move
                        if (isValidKey(temp)) {
                            if (temp == 'D')
                                CHAR_LOCK = 'A';
                            else if (temp == 'A')
                                CHAR_LOCK = 'D';
                            else if (temp == 'W')
                                CHAR_LOCK = 'S';
                            else
                                CHAR_LOCK = 'W';
                            MOVING = temp;
                        }
                    }
                }
                else {
                    if (temp == 'Y')
                        StartGame();
                    else if (temp == 13)
                    {
                        PauseGame(handle_t1);
                        DrawSnake(ID_STUDENT);
                        //menu();
                        goto MENU;
                    }
                    else
                    {
                        ExitGame(handle_t1);
                        return 0;
                    }
                }
            }
        }
        else if (menu_choice == 2) // Load game
        {
            //Choose_Score(); ///!!!!
            //menu();
            StartGame();
            LoadData();
            PlaySound(TEXT("startgame.wav"), NULL, SND_FILENAME | SND_ASYNC);
            DrawSnake(ID_STUDENT);
            while (true) {
                temp = toupper(_getch()); // ???
                if (STATE == 1) {
                    if (temp == 'P') {
                        PauseGame(handle_t1);
                        gameIsPaused = false;
                    }

                    else if (temp == 'L') {
                        SaveData();
                        ExitGame(handle_t1);
                        return 0;
                    }

                    else if (temp == 'T') {
                        LoadData();
                    }

                    else if (temp == 27) { // 27 is ASCII value of 'ESC' key
                        if (gameIsPaused == true)
                        {
                            ExitGame(handle_t1);
                            return 0;
                        }
                        else
                        {
                            SaveData();
                            backgroundMusic = true;
                            gameIsPaused = true;
                            goto MENU;
                        }
                    }
                    else {
                        ResumeThread(handle_t1);

                        //Logic of snake's move
                        if (isValidKey(temp)) {
                            if (temp == 'D')
                                CHAR_LOCK = 'A';
                            else if (temp == 'A')
                                CHAR_LOCK = 'D';
                            else if (temp == 'W')
                                CHAR_LOCK = 'S';
                            else
                                CHAR_LOCK = 'W';
                            MOVING = temp;
                        }
                    }
                }
                else {
                    if (temp == 'Y')
                        StartGame();
                    else if (temp == 13)
                    {
                        PauseGame(handle_t1);
                        DrawSnake(ID_STUDENT);
                        menu();
                    }
                    else
                    {
                        ExitGame(handle_t1);
                        return 0;
                    }
                }
            }
            ///goto MENU; ///!!!
        }
        else if (menu_choice == 3) //Highscore
        {
            ResetData();
            DrawScoreBoard(30, 5, WIDTH_CONSOLE, HEIGHT_CONSOLE);
            /*while (true) //Already handle in DrawScoreBoard()
            {
                if (_kbhit())
                {
                    int temp = toupper(_getch());
                    if (temp == 27)
                        break;
                }
            }
            */
            backgroundMusic = true;
            goto MENU;
        }
        else if (menu_choice == 4)
        {
            if (volume != 0)
                cout << "Type \'M\' to mute!";
            else
                cout << "Type \'M\' to unmute";
            while (true)
            {

                if (_kbhit())
                {
                    int temp = toupper(_getch());
                    if (temp == 27)
                        break;
                    else if (temp == 'M')
                    {
                        setVolume();
                        system("cls");
                        if (volume != 0)
                            cout << "Type \'M\' to mute!";
                        else
                            cout << "Type \'M\' to unmute";
                    }
                }
            }
            backgroundMusic = true;
            goto MENU;
        }
        else if (menu_choice == 5)
        {

            cout << "WILL BE UPDATED SOON";
            while (true)
            {
                if (_kbhit())
                {
                    int temp = toupper(_getch());
                    if (temp == 27)
                        break;
                }
            }
            backgroundMusic = true;
            goto MENU;
        }
        else if (menu_choice == 6) {

            ExitGame(handle_t1);
            return 0;
        }
    }   
    return 0;
}
