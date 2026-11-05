#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
// Windows控制台API
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

// ===================== 数据定义 =====================
// 坐标点
struct Point
{
    int x;
    int y;
};

// 方向枚举
enum Dir
{
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// 游戏全局类（封装所有游戏状态，推荐写法）
class SnakeGame
{
public:
    // 地图尺寸
    const int width = 40;
    const int height = 20;

    // 蛇
    std::vector<Point> snake;
    Dir dir;
    Point food;
    bool gameOver;
    int score;

    // 构造函数：初始化游戏
    SnakeGame()
    {
        Setup();
    }

    // 初始化游戏
    void Setup()
    {
        gameOver = false;
        dir = RIGHT;
        score = 0;
        snake.clear();
        // 蛇初始3节身体，放在屏幕中间
        snake.push_back({width / 2, height / 2});
        snake.push_back({width / 2 - 1, height / 2});
        snake.push_back({width / 2 - 2, height / 2});
        // 随机生成食物
        SpawnFood();
    }

    // 随机生成食物，不能生成在蛇身上
    void SpawnFood()
    {
        bool onSnake;
        do
        {
            onSnake = false;
            food.x = rand() % width;
            food.y = rand() % height;
            // 判断食物是否在蛇身体上
            for(auto &seg : snake)
            {
                if(seg.x == food.x && seg.y == food.y)
                {
                    onSnake = true;
                    break;
                }
            }
        } while(onSnake);
    }

    // 绘制画面
    void Draw()
    {
#ifdef _WIN32
        // Windows清屏
        system("cls");
#else
        system("clear");
#endif
        // 上围墙
        for(int i = 0; i < width + 2; i++)
            std::cout << "#";
        std::cout << std::endl;

        // 逐行绘制地图
        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                if(x == 0)
                    std::cout << "#"; // 左墙

                if(x == snake[0].x && y == snake[0].y)
                {
                    std::cout << "O"; // 蛇头
                }
                else
                {
                    bool isBody = false;
                    for(int i = 1; i < snake.size(); i++)
                    {
                        if(snake[i].x == x && snake[i].y == y)
                        {
                            std::cout << "o"; // 蛇身体
                            isBody = true;
                            break;
                        }
                    }
                    if(!isBody)
                    {
                        if(x == food.x && y == food.y)
                            std::cout << "F"; // 食物
                        else
                            std::cout << " "; // 空地
                    }
                }

                if(x == width - 1)
                    std::cout << "#"; // 右墙
            }
            std::cout << std::endl;
        }

        // 下围墙
        for(int i = 0; i < width + 2; i++)
            std::cout << "#";
        std::cout << std::endl;

        std::cout << "Score: " << score << std::endl;
        std::cout << "WASD控制方向, X退出游戏" << std::endl;
    }

    // 处理键盘输入
    void Input()
    {
#ifdef _WIN32
        if(_kbhit()) // 检测是否有按键按下（非阻塞）
        {
            switch(_getch())
            {
                case 'a':
                    if(dir != RIGHT) dir = LEFT;
                    break;
                case 'd':
                    if(dir != LEFT) dir = RIGHT;
                    break;
                case 'w':
                    if(dir != DOWN) dir = UP;
                    break;
                case 's':
                    if(dir != UP) dir = DOWN;
                    break;
                case 'x':
                    gameOver = true;
                    break;
            }
        }
#endif
    }

    // 更新游戏逻辑（最重要！每一帧状态更新）
    void Logic()
    {
        // 1. 获取蛇头坐标
        Point head = snake[0];
        switch(dir)
        {
            case LEFT:  head.x--; break;
            case RIGHT: head.x++; break;
            case UP:    head.y--; break;
            case DOWN:  head.y++; break;
            default: break;
        }

        // 碰撞检测：撞墙
        if(head.x < 0 || head.x >= width || head.y <0 || head.y >= height)
        {
            gameOver = true;
            return;
        }

        // 碰撞检测：撞到自己身体
        for(size_t i = 1; i < snake.size(); i++)
        {
            if(head.x == snake[i].x && head.y == snake[i].y)
            {
                gameOver = true;
                return;
            }
        }

        // 把新蛇头插入到vector头部
        snake.insert(snake.begin(), head);

        // 判断是否吃到食物
        if(head.x == food.x && head.y == food.y)
        {
            score += 10;
            SpawnFood();
            // 吃到食物：不删除尾巴，蛇变长
        }
        else
        {
            // 没吃到食物：删掉尾巴，蛇整体前进
            snake.pop_back();
        }
    }

    // 游戏主循环
    void Run()
    {
        while(!gameOver)
        {
            Draw();
            Input();
            Logic();
#ifdef _WIN32
            Sleep(80); // 延时ms，控制游戏速度
#endif
        }
        std::cout << "Game Over! Final Score: " << score << std::endl;
    }
};

int main()
{
    srand((unsigned int)time(NULL)); // 设置随机种子
    SnakeGame game;
    game.Run();
    return 0;
}
