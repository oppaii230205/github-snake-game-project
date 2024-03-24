#include <iostream>
#include <Windows.h>
#include <thread>
#include <conio.h>
#include <cctype>
#include <mmsystem.h>
#include <string>

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

void SetOutputColor(int foregroundColor, int backgroundColor = 15) { //15 is White Background Color
    int finalColor = foregroundColor + 16 * backgroundColor;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), finalColor);
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

bool matchCoordinate(POINT A, POINT B) {
    return (A.x == B.x && A.y == B.y);
}

//Constants
#define MAX_SIZE_SNAKE 10
#define MAX_SIZE_FOOD 4
#define MAX_SPEED 3
#define BLACK 240
#define LIGHTBLUE 243
#define RED 244
#define PURPLE 245
#define GOLDEN 246
#define GREEN 250
#define WHITE 255



//Global variables5
POINT snake[10]; //snake
POINT food[4]; // food
POINT gate; //gate
POINT gate_2;
int pos_2;
POINT wall[50];
int CHAR_LOCK;//used to determine the direction my snake cannot move (At a moment, there is one direction my snake cannot move to)
int MOVING;//used to determine the direction my snake moves (At a moment, there are three directions my snake can move)
int SPEED;// Standing for level, the higher the level, the quicker the speed
int HEIGHT_CONSOLE, WIDTH_CONSOLE;// Width and height of console-screen
int FOOD_INDEX; // current food-index
int SIZE_SNAKE; // size of snake, initially maybe 6 units and maximum size may be 10
int STATE; // State of snake: dead or alive
string ID_STUDENT = "2312020023120194";
bool isPlayedGameOverSound;
bool isNextLevel;

bool IsValidFood(int x, int y) {
    //Body
    for (int i = 0; i < SIZE_SNAKE; i++) {
        if (snake[i].x == x && snake[i].y == y) {
            return false;
        }
    }

    //Wall
    for (int i = 0; i < pos_2; i++) {
        if (wall[i].x == x && wall[i].y == y) {
            return false;
        }
    }

    return true;
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

bool IsValidWall(int x, int y) {
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

void GenerateGate_1() {
    int x, y = 0; // Fix the gate on the top-border
    srand(time(NULL));

    do {
        x = rand() % (WIDTH_CONSOLE - 4) + 3;   // (-2) + 2 vẫn oke, nma de 3 don vi cho cai cong no dep :v
    } while (!IsValidGate(x, y));

    //Save
    gate.x = x;
    gate.y = y;

    //Draw
    setColor(244);
    GotoXY(x - 1, y);
    cout << (char)201;
    setColor(246);
    cout << 'O';
    setColor(244);
    cout << (char)187;
    GotoXY(x - 1, y + 1);
    cout << (char)202;
    GotoXY(x + 1, y + 1);
    cout << (char)202;
    setColor(255);
}

void GenerateGate_2() {
    int x = gate_2.x = gate.x;
    int y = gate_2.y = HEIGHT_CONSOLE;;

    //Draw
    setColor(244);
    GotoXY(x - 1, y);
    cout << (char)200;
    setColor(246);
    cout << 'O';
    setColor(244);
    cout << (char)188;
    GotoXY(x + 1, y - 1);
    cout << (char)203;
    GotoXY(x - 1, y - 1);
    cout << (char)203;
    //setColor(255);
}

void ClearGate_1() {
    int x = gate.x;
    int y = gate.y;
    GotoXY(x, y);
    cout << " ";
    GotoXY(x - 1, y);
    cout << " ";
    GotoXY(x - 1, y + 1);
    cout << " ";
    GotoXY(x + 1, y + 1);
    cout << " ";
}

void ClearGate_2() {
    int x = gate_2.x;
    int y = gate_2.y;
    setColor(GOLDEN);
    GotoXY(x - 1, y);
    cout << char(205) << char(205) << char(205); //3 spaces
    setColor(WHITE);
    GotoXY(x - 1, y - 1);
    cout << " ";
    GotoXY(x + 1, y - 1);
    cout << " ";
}

void ResetData() {
    //Initialize the global values //width 70 height 20
    CHAR_LOCK = 'A', MOVING = 'D', SPEED = 1; FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGHT_CONSOLE = 20, SIZE_SNAKE = 4; isPlayedGameOverSound = isNextLevel = false;
    // Initialize default values for snake
    snake[0] = { 10, 5 }; snake[1] = { 11, 5 };
    snake[2] = { 12, 5 }; snake[3] = { 13, 5 };
    snake[4] = { 14, 5 }; snake[5] = { 15, 5 };
    GenerateFood();//Create food array
}

//Sample
void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
    setColor(246);
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
    TerminateThread(t, 0);
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

        if (SPEED == MAX_SPEED) { //Reset
            SPEED = 1;
            SIZE_SNAKE = 4;
        }
        else { //Move to next level
            ClearFood();
            GenerateGate_1();
            // Level_2(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
            SPEED++;
            SIZE_SNAKE++;
        }
        //GenerateFood();
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
    setColor(243);
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        if (snake[i].x < WIDTH_CONSOLE && snake[i].y <= HEIGHT_CONSOLE) {
            GotoXY(snake[i].x, snake[i].y);
            cout << str[SIZE_SNAKE - i - 1];
        }
    }

    setColor(245);
    GotoXY(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y);
    cout << str[0];

    setColor(255);
}

void ClearSnake() {
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
    //GotoXY(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y);
    //cout << "X";
    BlinkSnake();
    setColor(244);
    GotoXY(0, HEIGHT_CONSOLE + 2);
    cout << "Dead, type \'y\' to continue or anykey to exit!";
    setColor(255);
}

void Level_2(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
    system("cls");
    /*FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGHT_CONSOLE = 20; SPEED = 2, isPlayedGameOverSound = false;
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);*/
    CHAR_LOCK = 'S', MOVING = 'W', SPEED = 2; FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGHT_CONSOLE = 20; isPlayedGameOverSound = isNextLevel = false;
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
    STATE = 1;
    GenerateGate_2();
    snake[SIZE_SNAKE - 1].x = gate_2.x;
    snake[SIZE_SNAKE - 1].y = gate_2.y - 1;
    /*for (int i = SIZE_SNAKE - 1; i > 1; i--) {
        snake[i - 1].x = snake[i].x;
        snake[i - 1].y = snake[i].y; //+ 1;
    }*/

    //Set other coordinate to NULL
    for (int i = 0; i < SIZE_SNAKE - 1; i++) { //chua thang snake[0] lai
        snake[i].x = 80;
        snake[i].y = 80;
    }
    //ClearGate_2();

    setColor(RED);
    pos_2 = 0;
    wall[0] = { 28,7 };
    for (int i = 28; i <= 42; i++) {
        wall[pos_2++] = { i,7 };
        GotoXY(i, 7);
        cout << (char)254;
    }

    for (int i = 28; i <= 42; i++) {
        wall[pos_2++] = { i, 13};
        GotoXY(i, 13);
        cout << (char)254;
    }

    GenerateFood();
}

void ProcessGate() {
    if (matchCoordinate(snake[SIZE_SNAKE - 1], gate)) {
        PlaySound(TEXT("goodresult.wav"), NULL, SND_FILENAME | SND_ASYNC);
        /*for (int i = SIZE_SNAKE - 1; i >= 0; i--) {
            GotoXY(snake[i].x, snake[i].y);
            cout << " ";
        }*/
        ClearSnakeAndFood();
        Level_2(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE);
    }
    else {
        if (!isNextLevel)
            DrawFood((char)254);
        
        //Clear gate
        POINT temp = gate_2;
        temp.y--;
        if (matchCoordinate(snake[0], temp)) {
            ClearGate_2();
        }

        //Draw snake
        DrawSnake(ID_STUDENT);
    }
}

bool hitObstacle(POINT newPoint) {
    //Enter Gate
    if (matchCoordinate(newPoint, gate))
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
    point1.x = gate.x - 1;
    point1.y = gate.y - 1;

    point2.x = gate.x + 1;
    point2.y = gate.y - 1;

    if (matchCoordinate(snake[SIZE_SNAKE - 1], point1) || matchCoordinate(snake[SIZE_SNAKE - 1], point2))
        return true;

    //Crash Wall
    for (int i = 0; i < pos_2; i++) {
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
        else {
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

int main() {
    system("color F0");
    int temp;
    FixConsoleWindow();
    StartGame();
    //setcursor(0, 0);  //!!!
    ShowConsoleCursor(false);
    thread t1(ThreadFunc); //Create thread for snake
    HANDLE handle_t1 = t1.native_handle(); //Take handle of thread

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
            else {
                ExitGame(handle_t1);
                return 0;
            }
        }
    }
    return 0;
}