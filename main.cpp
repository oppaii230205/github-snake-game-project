#include <iostream>
#include <Windows.h>
#include <thread>
#include <conio.h>
#include <cctype>
#include <mmsystem.h>
#include <string>
#include "fixconsolewindows.h" //Visual Studio

#pragma comment(lib, "winmm.lib")

using namespace std;

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
#define MAX_LEVEL 3
#define BLACK 240
#define LIGHTBLUE 243
#define RED 244
#define PURPLE 245
#define GOLDEN 246
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
POINT wall[50]; //array of obstacles
int CHAR_LOCK;//used to determine the direction my snake cannot move (At a moment, there is one direction my snake cannot move to)
int MOVING;//used to determine the direction my snake moves (At a moment, there are three directions my snake can move)
int SPEED;// Standing for level, the higher the level, the quicker the speed
int HEIGHT_CONSOLE, WIDTH_CONSOLE;// Width and height of console-screen
int FOOD_INDEX; // current food-index
int SIZE_SNAKE; // size of snake, initially maybe 6 units and maximum size may be 10
int STATE; // State of snake: dead or alive
string ID_STUDENT = "2312019323120194231201952312020023120209";
bool isPlayedGameOverSound;//use to determine whether the "game-over sound" is played or not
bool isNextLevel;//use to determine whether the entering gate is generated or not

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

void ResetData() {
    //Initialize the global values //width 70 height 20
    CHAR_LOCK = 'A'; MOVING = 'D'; SPEED = 1; FOOD_INDEX = 0; WIDTH_CONSOLE = 70; HEIGHT_CONSOLE = 20; SIZE_SNAKE = 4; isPlayedGameOverSound = isNextLevel = false; Level = 1; numsOfWalls = 0; gateIn = gateOut = dummyGate;
    // Initialize default values for snake
    snake[0] = { 10, 5 }; snake[1] = { 11, 5 };
    snake[2] = { 12, 5 }; snake[3] = { 13, 5 };
    snake[4] = { 14, 5 }; snake[5] = { 15, 5 };
    GenerateFood();//Create food array
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


void GenerateGateIn() {
    int x, y = 0; // Fix the gateIn on the top-border
    srand(time(NULL));

    do {
        x = rand() % (WIDTH_CONSOLE - 4) + 3;   // (-2) + 2 vẫn oke, nma de 3 don vi cho cai cong no dep :v
    } while (!IsValidGate(x, y));

    //Save
    gateIn.x = x;
    gateIn.y = y;

    //Draw
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
    //setColor(WHITE);
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

void StartGame() {
    system("cls");
    ResetData(); //Initialize
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE); //Draw game
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

void PlayEatingSound() {
    PlaySound(TEXT("retro-coin-02.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void PlayGameOverSound() {
    PlaySound(TEXT("game-over.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void ClearFood() {
    GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
    cout << " ";
}

//Function to update global data
void Eat() {
    PlayEatingSound();
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
    STATE = 0;
    BlinkSnake();
    setColor(244);
    GotoXY(0, HEIGHT_CONSOLE + 2);
    cout << "Dead, type \'y\' to continue or anykey to exit!";
    //setColor(WHITE);
}

void Level_2(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
    system("cls");
    CHAR_LOCK = 'S', MOVING = 'W', SPEED = 2; FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGHT_CONSOLE = 20; isPlayedGameOverSound = false; isNextLevel = false;
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
    STATE = 1;
    //ClearGateOut();
    GenerateGateOut();
    snake[SIZE_SNAKE - 1].x = gateOut.x;
    snake[SIZE_SNAKE - 1].y = gateOut.y - 1;
    //Set other coordinate to NULL
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        snake[i].x = 888;
        snake[i].y = 888;
    }
    
    //gateIn.x = -10, gateIn.y = -10;

    setColor(RED);
    numsOfWalls = 0;
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
    GenerateFood();
}

void Level_3(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
    system("cls");
    CHAR_LOCK = 'S', MOVING = 'W', SPEED = 2; FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGHT_CONSOLE = 20; isPlayedGameOverSound = isNextLevel = false;
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
    STATE = 1;
    GenerateGateOut();
    snake[SIZE_SNAKE - 1].x = gateOut.x;
    snake[SIZE_SNAKE - 1].y = gateOut.y - 1;
    //Set other coordinate to NULL
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        snake[i].x = 888;
        snake[i].y = 888;
    }
    //gateIn.x = -10, gateIn.y = -10;

    setColor(RED);
    numsOfWalls = 0;
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
        if (!isNextLevel)
            DrawFood((char)254);

        /*
        //Clear gate
        POINT temp = gateOut;
        temp.y--;
        if (matchCoordinate(snake[0], temp)) { //snake_tail
            ClearGateOut();
        }
        */
        //Draw snake
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
    system("cls");
}


int main() {
    system("color F0");
    FixConsoleWindow();
    ShowConsoleCursor(false);
    SetConsoleCP(437);
    SetConsoleOutputCP(437);

    int temp;
    bool TurnOnThread = false;

    intro();
    //StartGame();
    //setcursor(0, 0);  //!!!
    //if (TurnOnThread == false)
    //{
    thread t1(ThreadFunc); //Create thread for snake
    HANDLE handle_t1 = t1.native_handle(); //Take handle of thread

    PauseGame(handle_t1);

    //}

    int choose = 0;
MENU:
    {
        ChooseMenu(choose);
        if (choose == 0)
        {

            StartGame();
            DrawSnake(ID_STUDENT);
            while (true) {
                temp = toupper(_getch()); // ???
                if (STATE == 1) {
                    if (temp == 'P') {
                        PauseGame(handle_t1);
                    }
                    else if (temp == 27) { // 27 is ASCII value of 'ESC' key
                        ExitGame(handle_t1);
                        return 0;
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
        else if (choose == 1)
        {
            Choose_Option();
            goto MENU;
        }
        else if (choose == 2)
        {
            Choose_Score();
            goto MENU;
        }
        else if (choose == 3)
            ExitGame(handle_t1);
    }
    return 0;
}
