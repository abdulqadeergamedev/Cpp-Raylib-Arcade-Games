#include <iostream>
#include <raylib.h>
#include <vector>

int rockSizes[] = {32, 64, 128};
int activeRocks = 0;

class Rock;

Rock& CreatePooledRock();

class Rock
{
public:
    Vector2 position;
    Vector2 speed;
    int sizeIndex = 0;
    float gravity = 0.2f;
    int health;
    int radius;
    
    bool isActive = false;
        
    Rock()
    {
        speed = { 5.0f, 4.0f };
        position = { GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
        health = GetRandomValue(50, 100);
        sizeIndex = GetRandomValue(0, std::size(rockSizes) - 1);
        radius = rockSizes[sizeIndex];
    }
    
    void Update()
    {
        position.x += speed.x;
        position.y += speed.y;

        speed.y += gravity;

        // Check walls collision for bouncing
        if ((position.x >= (GetScreenWidth() - radius)) || (position.x <= radius)) speed.x *= -1.0f;
        if ((position.y >= (GetScreenHeight() - radius)) || (position.y <= radius)) speed.y *= -0.95f;
    }
    
    void Draw()
    {
        DrawCircle(position.x, position.y, radius, RED);
        
        const char* text = TextFormat("%i", health);
        
        float textHeight = radius / 1.5f;
        float textWidth = MeasureText(text, textHeight);

        DrawText(text, position.x - textWidth / 2, position.y - textHeight / 2, textHeight, WHITE);
    }

    void TakeDamage(int damage)
    {
        health -= damage;
        if (health <= 0)
        {
            sizeIndex--;
            
            if (sizeIndex < 0)
            {
                DeActivate();
            }
            else
            {
                // Shrink current rock
                radius = rockSizes[sizeIndex];
                health = GetRandomValue(25, 50);
                speed.x *= -1.0f;
                speed.y *= -0.95f;

                // Spawn split rock
                Rock& splitRock = CreatePooledRock();
                splitRock.sizeIndex = sizeIndex;
                splitRock.radius = rockSizes[sizeIndex];
                splitRock.position = position;
                splitRock.speed.x = -speed.x; // Opposite horizontal velocity
                splitRock.speed.y = speed.y;
                splitRock.health = health;
            }
        }
    }
    
    void DeActivate()
    {
        if (isActive)
        {
            isActive = false;
            activeRocks--;
        }
    }
};

class Bullet
{
public:
    Vector2 position;
    float radius = 15;
    float speed = 1500;
    bool isActive = false;
    
    void Draw()
    {
        DrawCircle(position.x, position.y, radius, YELLOW);
    }
    
    void Update()
    {
        position.y -= speed * GetFrameTime();
        
        if (position.y <= 0) DeActivate();
    }
    
    void SetPosition(int posX, int posY)
    {
        position.x = posX;
        position.y = posY;
    }
    
    void DeActivate()
    {
        if (isActive)
        {
            isActive = false;
        }
    }
};

int windowWidth = 800;
int windowHeight = 1200;
bool isGameActive = false;

Vector2 playerPos = {windowWidth / 2.0f,  windowHeight - 40.0f};
float playerSpeed = 500.0f;
float playerSize = 32;

float fireInterval = 0.08f;
double lastFireTime = 0.0f;
int bulletDamage = 10;
Bullet bullets[50];
std::vector<Sound> shootSounds;

double lastRockSpawnTime = 0.0f;
float rockSpawnInterval = 5;
int maxSpawnRocks = 3;
Rock rocks[25];

bool eventTriggered(double& lastTriggerTime, double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastTriggerTime >= interval)
    {
        lastTriggerTime = currentTime;
        return true;
    }
    return false;
}

std::vector<Sound> LoadSounds(const char* prefix, const char* extension)
{
    std::vector<Sound> sounds;
    int index = 1;

    while (true)
    {
        // Formats filename as "assets/sounds/shoot_01.ogg", "assets/sounds/shoot_02.ogg", etc.
        const char* filePath = TextFormat("%s_%02i%s", prefix, index, extension);

        // Check if the file exists on disk
        if (FileExists(filePath))
        {
            sounds.push_back(LoadSound(filePath));
            index++;
        }
        else
        {
            // Stop looping as soon as a numbered file is not found (e.g., shoot_06.ogg doesn't exist)
            break; 
        }
    }

    return sounds;
}

void PlayerMovement();

Rock& CreatePooledRock()
{
    for (Rock& rock : rocks)
    {
        if (!rock.isActive)
        {
            rock.isActive = true;
            activeRocks++;
            return rock;
        }
    }
    
    // Fallback if pool is completely exhausted: return the first rock safely
    rocks[0].isActive = true;
    return rocks[0];
}

void GameOver()
{
    for (Rock& rock : rocks)
    {
        rock.DeActivate();
    }
    
    for (Bullet& bullet : bullets)
    {
        bullet.DeActivate();
    }
    
    playerPos = {windowWidth / 2.0f,  windowHeight - 40.0f};
    activeRocks = 0;
    
    isGameActive = false; 
}

void UpdateRocks()
{
    // Spawn Rocks
    if (eventTriggered(lastRockSpawnTime, rockSpawnInterval) && activeRocks < maxSpawnRocks)
    {
        Rock* pooledRock = nullptr;
        for (Rock& rock : rocks)
        {
            if (!rock.isActive)
            {
                pooledRock = &rock;
                break;
            }
        }

        if (pooledRock != nullptr)
        {
            *pooledRock = Rock(); // Reset rock properties
            pooledRock->isActive = true;
            activeRocks++;
        }
        else
        {
            CreatePooledRock();
        }
    }
        
    // Update rocks and check for game-over
    for (Rock& rock : rocks)
    {
        if (rock.isActive)
        {
            rock.Update();
                
            if (CheckCollisionCircles(playerPos, playerSize, rock.position, rock.radius))
            {
                GameOver();
            }
        }
    }
}

void UpdateBullets()
{
    // Shoot bullets
    if (eventTriggered(lastFireTime, fireInterval))
    {
        for (Bullet& bullet : bullets)
        {
            if (!bullet.isActive)
            {
                bullet.SetPosition(playerPos.x, playerPos.y);
                bullet.isActive = true;
                if (!shootSounds.empty()) PlaySound(shootSounds[GetRandomValue(0, (int)shootSounds.size() - 1)]);
                break;
            }
        }
    }
       
    // Update bullets
    for (Bullet& bullet : bullets)
    {
        if (bullet.isActive)
        {
            bullet.Update();
            
            for (Rock& rock : rocks)
            {
                if (rock.isActive)
                {
                    if (CheckCollisionCircles(bullet.position, bullet.radius, rock.position, rock.radius))
                    {
                        bullet.DeActivate();
                        rock.TakeDamage(bulletDamage);
                        break;
                    }   
                }
            }
        }
    }
}

int main() 
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(windowWidth, windowHeight, "Ball Blast");
    InitAudioDevice();
    SetTargetFPS(60);
    
    shootSounds = LoadSounds("assets/sounds/shoot", ".ogg");

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            isGameActive = !isGameActive;
        }
        
        if (isGameActive)
        {
            PlayerMovement();
        
            UpdateRocks();
            UpdateBullets();
        }

        BeginDrawing();
     
        ClearBackground(BLACK);
        
        DrawCircle(playerPos.x, playerPos.y, playerSize, WHITE);

        for (Bullet& bullet : bullets)
        {
            if (bullet.isActive) bullet.Draw();
        }
        
        for (Rock& rock : rocks)
        {
           if (rock.isActive) rock.Draw();
        }
        
        if (!isGameActive)
        {
            DrawRectangle(0, 0, windowWidth, windowHeight, {0, 0, 0, 150});
            
            const char* text = "PRESS [SPACE] TO PLAY";
            float textHeight = 48;
            float textWidth = MeasureText(text, textHeight);
            DrawText(text, (windowWidth - textWidth) / 2, (windowHeight - textHeight) / 2, textHeight, WHITE);
        }
        
        EndDrawing();
    }

    for (Sound shoot_sound : shootSounds) UnloadSound(shoot_sound);
    
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}

void PlayerMovement()
{
    if (IsKeyDown(KEY_LEFT))
        playerPos.x -= playerSpeed * GetFrameTime();
    else if (IsKeyDown(KEY_RIGHT))
        playerPos.x += playerSpeed * GetFrameTime();

    if (playerPos.x - playerSize <= 0) playerPos.x = playerSize;
    else if (playerPos.x + playerSize >= windowWidth) playerPos.x = windowWidth - playerSize;
}
