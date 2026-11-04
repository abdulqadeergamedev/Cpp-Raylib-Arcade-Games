#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <deque>

int cellCount = 16;
int cellSize = 32;
int borderPadding = 124;
int windowSize = cellSize * cellCount + borderPadding * 2;
float gridThickness = 5;

double lastUpateTime = 0;
bool isGameActive = false;

bool IsElementInDeque(std::deque<Vector2> deque, Vector2 element, int skip);
void GameOver();

class Snake
{
public:
    std::deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;
    
    void Draw()
    {
        for (int i = 0; i < body.size(); ++i)
        {
            float x = body[i].x;
            float y = body[i].y;
            // DrawRectangleRounded({x * cellSize + borderPadding, y * cellSize + borderPadding, (float)cellSize, (float)cellSize}, 0.5f, 0, GREEN);
            DrawRectangle(x * cellSize + borderPadding, y * cellSize + borderPadding, cellSize, cellSize, GREEN);
        }
    }
    
    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        
        if (!addSegment) body.pop_back();
        
        addSegment = false;
    }
    
    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food
{
public:
    Vector2 position = {0, 0};
    
private:
    float size = cellSize / 3;
    
public:
    void Draw()
    {
        DrawCircle(position.x * cellSize + borderPadding + (cellSize / 2.0f), position.y * cellSize + borderPadding + (cellSize / 2.0f), size, RED);
    }
    
    Vector2 GenerateCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        
        return {x, y}; 
    }
    
    Vector2 Generate(std::deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateCell();

        while (IsElementInDeque(snakeBody, position, -1))
        {
            position = GenerateCell();
        }
        
        return position;
    }
    
    Food(std::deque<Vector2> snakeBody)
    {
       position = Generate(snakeBody);
    }
};

class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    int score = 0;
    
    Sound eatSound, gameOverSound;
    
    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("assets/sounds/eat.ogg");
        gameOverSound = LoadSound("assets/sounds/game_over.ogg");
    }
    
    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(gameOverSound);
        CloseAudioDevice();
    }
    
    void Draw()
    {
        food.Draw();
        snake.Draw();
    }
    
    void Update()
    {
        snake.Update();
        CheckCollisionWithFood();
        CheckCollisionWithEdges();
        CheckCollisionWithSelf();
    }
    
    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.Generate(snake.body);
            score++;
            snake.addSegment = true;
            
            PlaySound(eatSound);
        }
    }
    
    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1)
        {
            GameOver();
        }
        else if (snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }
    
    void CheckCollisionWithSelf()
    {
        if (IsElementInDeque(snake.body, snake.body[0], 0))
        {
            GameOver();
        }
    }
    
    void GameOver()
    {
        snake.Reset();
        food.position = food.Generate(snake.body);
        score = 0;
        
        PlaySound(gameOverSound);
        
        isGameActive = false;
    }
};

bool IsElementInDeque(std::deque<Vector2> deque, Vector2 element, int skip)
{
    for (int i = 0; i < deque.size(); ++i)
    {
        if (i == skip) continue;
        
        if (Vector2Equals(deque[i], element)) return true;
    }
    
    return false;
}

bool eventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpateTime >= interval)
    {
        lastUpateTime = currentTime;
        return true;
    }
    return false;
}

int main()
{
    InitWindow(windowSize, windowSize, "Snake Game");
    SetTargetFPS(60);
    
    Game game = Game();
    
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            isGameActive = !isGameActive;
        }
        
        if (isGameActive)
        {
            if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
            {
                game.snake.direction = {0, -1};
            }
            else if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
            {
                game.snake.direction = {0, 1};
            }
            else if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
            {
                game.snake.direction = {-1, 0};
            }
            else if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
            {
                game.snake.direction = {1, 0};
            }
        
            if (eventTriggered(0.2f))
            {
                game.Update();
            }
        }
      
        BeginDrawing();
        ClearBackground(BLACK);
        
        DrawRectangleLinesEx({(float)borderPadding - gridThickness, (float)borderPadding - gridThickness, (float)cellSize * cellCount + gridThickness * 2, (float)cellSize * cellCount + gridThickness * 2}, gridThickness, WHITE);
        // for (int i = 0; i < cellCount; ++i)
        // {
        //     DrawLineEx({(float)cellSize * i + borderPadding, (float)borderPadding}, {(float)cellSize * i + borderPadding, (float)cellSize * cellCount + borderPadding}, gridThickness, WHITE);
        //     DrawLineEx({(float)borderPadding, (float)cellSize * i + borderPadding },{ (float)cellSize * cellCount + borderPadding, (float)cellSize * i + borderPadding}, gridThickness, WHITE);
        // }
        
        DrawText("Snake Arcade", borderPadding - 5, borderPadding - 50, 40, WHITE);
        DrawText(TextFormat("%i", game.score), borderPadding - 5, cellSize * cellCount + borderPadding + 20, 40, WHITE);
        
        game.Draw();
        
        if (!isGameActive)
        {
            DrawRectangle(0, 0, windowSize, windowSize, {0, 0, 0, 150});
        
            const char* message = "PRESS [SPACE] TO PLAY";
            int msg_width = MeasureText(message, 36);
            DrawText(message, (windowSize - msg_width) / 2, (windowSize / 2) - 35, 36, WHITE);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    
    return 0;
}
