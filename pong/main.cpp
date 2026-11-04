#include <raylib.h>
#include <algorithm>

int player_score = 0, cpu_score = 0;
bool isGameActive = false;

static Color backgroundColor = {20, 160, 133, 255};
static Color ballColor = {255, 183, 3, 255};
static Color paddleColor = {234, 234, 234, 255};

void UpdateScore(bool isPlayer);

class Ball
{
    public:
    int x, y;
    int speed_x, speed_y;
    float radius;
    
    void StarBall()
    {
        int speed_x_choices[2] = {-speed_x, speed_x};
        int speed_y_choices[2] = {-speed_y, speed_y};
        speed_x = speed_x_choices[GetRandomValue(0, 1)];
        speed_y = speed_y_choices[GetRandomValue(0, 1)];
    }
    
    void Draw()
    {
        DrawCircle(x, y, radius, ballColor);
    }
    
    void Update()
    {
        x += speed_x;
        y += speed_y;
        
        if (y + radius >= GetScreenHeight() || y - radius <= 0) 
        {
            speed_y *= -1;
        }
        
        if (x + radius >= GetScreenWidth() || x - radius <= 0) 
        {
            speed_x *= -1;
        }
        
        if (x + radius >= GetScreenWidth())
        {
            UpdateScore(false);
        }
        else if (x - radius <= 0)
        {
            UpdateScore(true);
        }
    }
    
    void ResetBall()
    {
        x = GetScreenWidth() / 2;
        y = GetScreenHeight() / 2;
    }
};

class PaddleBase
{
    public:
    int x, y;
    int width, height;
    float speed;
    
    void Draw()
    {
        // DrawRectangle(x, y, width, height, paddleColor);
        DrawRectangleRounded(Rectangle{(float)x, (float)y, (float)width, (float)height}, 0.8f, 0, paddleColor);
    }
    
    virtual void Update()
    {
        Clamp();
    }
    
    void ResetPosition()
    {
        y = GetScreenHeight() / 2 - height / 2;
    }
    
protected:
    void Clamp()
    {
        y = std::max(y, 0);
        if (y + height >= GetScreenHeight())
        {
            y = GetScreenHeight() - height;
        }
    }
};

class CPUPaddle: public PaddleBase
{
    public:
    
    void Update(int ball_y, int ball_X)
    {
        if (ball_X > GetScreenWidth() / 2)
        {
            if (y + height / 2 > ball_y)
            {
                y -= speed;
            }
        
            if (y + height / 2 < ball_y)
            {
                y += speed;
            }
        }
        
        Clamp();
    }
};

class PlayerPaddle: public PaddleBase
{
    public:
    
    void Update()
    {
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_RIGHT))
            y += speed;
        else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_LEFT))
            y -= speed;
        
        Clamp();
    }
};

Ball ball;
PlayerPaddle player;
CPUPaddle opponent;
Sound pong_sounds[2];

constexpr int window_width = 1280, window_height = 800;

void DrawGame();
void UpdateGame();

int main()
{
    InitWindow(window_width, window_height,"Pong Game");
    InitAudioDevice();
    
    SetTargetFPS(60);
    
    // init ball
    ball.radius = 20;
    ball.x = window_width / 2;
    ball.y = window_height / 2;
    ball.speed_x = 7;
    ball.speed_y = 7;
    
    // init player
    player.width = 25;
    player.height = 120;
    player.x = player.width;
    player.speed = 7;
    player.ResetPosition();
    
    // init opponent
    opponent.width = 25;
    opponent.height = 120;
    opponent.x = window_width - opponent.width * 2;
    opponent.speed = (int)GetRandomValue(3, 6);
    opponent.ResetPosition();
    
    pong_sounds[0] = LoadSound("assets/sounds/pong_1.ogg");
    pong_sounds[1] = LoadSound("assets/sounds/pong_2.ogg");
    
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_SPACE) && !isGameActive)
        {
            isGameActive = true;
            ball.ResetBall();
            ball.StarBall();
        }
        
        // Update
        if (isGameActive)
        {
            UpdateGame();
        }
        
        DrawGame();
    }

    for (int i = 0; i < std::size(pong_sounds); ++i)
    {
        UnloadSound(pong_sounds[i]);
    }

    CloseAudioDevice();     
    
    CloseWindow();
    
    return 0;
}

void DrawGame()
{
    BeginDrawing();
        
    ClearBackground(BLACK);
    
    // Draw Background
    DrawRectangle(0, 0, window_width / 2, window_height, backgroundColor);
    DrawRectangle(window_width / 2, 0, window_width / 2, window_height, ColorBrightness(backgroundColor, 0.2f));
    DrawCircle(window_width / 2, window_height / 2, 150, {129, 204, 184, 255});
    DrawLine(window_width / 2, 0, window_width / 2, window_height, WHITE);

    opponent.Draw();
    player.Draw();
    if (isGameActive) ball.Draw();
    
    // Center CPU score around window_width / 4
    const char* cpu_text = TextFormat("%i", cpu_score);
    int cpu_width = MeasureText(cpu_text, 80);
    DrawText(cpu_text, (window_width / 4) - (cpu_width / 2), 10, 80, WHITE);
        
    // Center Player score around window_width * 3 / 4
    const char* player_text = TextFormat("%i", player_score);
    int player_width = MeasureText(player_text, 80);
    DrawText(player_text, (window_width * 3 / 4) - (player_width / 2), 10, 80, WHITE);
        
    if (!isGameActive)
    {
        DrawRectangle(0, 0, window_width, window_height, {0, 0, 0, 150});
        
        const char* message = "PRESS [SPACE] TO PLAY";
        int msg_width = MeasureText(message, 70);
        DrawText(message, (window_width - msg_width) / 2, (window_height / 2) - 35, 70, WHITE);
    }
        
    EndDrawing();
}

void UpdateGame()
{
    ball.Update();
    player.Update();
    opponent.Update(ball.y, ball.x);
        
    // collisions
    if (CheckCollisionCircleRec(Vector2{(float)ball.x, (float)ball.y}, ball.radius, Rectangle{(float)player.x, (float)player.y, (float)player.width, (float)player.height}))
    {
        ball.speed_x *= -1;
        PlaySound(pong_sounds[rand() % 2]);
    }
        
    if (CheckCollisionCircleRec(Vector2{(float)ball.x, (float)ball.y}, ball.radius, Rectangle{(float)opponent.x, (float)opponent.y, (float)opponent.width, (float)opponent.height}))
    {
        ball.speed_x *= -1;
        PlaySound(pong_sounds[rand() % 2]);
    }
}

void UpdateScore(bool isPlayer)
{
    if (isPlayer)
    {
        player_score++;
    }
    else
    {
        cpu_score++;
    }
    
    ball.ResetBall();
    player.ResetPosition();
    opponent.ResetPosition();

    isGameActive = false;
}
