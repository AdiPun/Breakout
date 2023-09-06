#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr float DISPLAY_WIDTH{ 1280 };
constexpr float DISPLAY_HEIGHT{ 720 };
constexpr int DISPLAY_SCALE{ 1 };
constexpr int BALL_RADIUS{ 48 };

// Define a value for our ball's default velocity and acceleration
const Vector2D BALL_VELOCITY_DEFAULT(5.0f, 0.f);
const Vector2D BALL_ACCELERATION_DEFAULT(0.f, 0.33f);

// Define a size for the paddle's AABB box
const Vector2D PADDLE_AABB{ 100.f, 20.f };
const Vector2D BALL_AABB{ 48,48 };

enum CollisionState
{
	ST_COLLIDING = 0,
	ST_NOTCOLLIDING
};

struct GameState
{
	CollisionState cState{ ST_NOTCOLLIDING };
};

GameState gState;

enum GameObjectType
{
	TYPE_BALL = 0,
	TYPE_PADDLE = 1
};

// Forward-declaration of functions
void Draw();
void UpdateBall();
void UpdatePaddle();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	// Setup PlayBuffer
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CentreAllSpriteOrigins(); // this function makes it so that obj.pos values represent the center of a sprite instead of its top-left corner

	// Create a ball object and a paddle object
	Play::CreateGameObject(TYPE_BALL, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, BALL_RADIUS, "ball");
	Play::CreateGameObject(TYPE_PADDLE, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100 }, BALL_RADIUS, "spanner");
	// ... "ball", "spanner" etc. are the filenames of sprites stored in the Data folder alongside this solution, you can put any .PNGs in there if you feel creative :)

	GameObject& ballObj = Play::GetGameObjectByType(TYPE_BALL);
	// Spawns ball in a 50/50 direction
	ballObj.velocity = BALL_VELOCITY_DEFAULT * Play::RandomRollRange(1, -1);
	ballObj.acceleration = BALL_ACCELERATION_DEFAULT;
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	UpdateBall();
	UpdatePaddle();
	Draw();
	return Play::KeyDown(VK_ESCAPE);
}

// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

void UpdateBall()
{
	// Grab the objects we created in Entry
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };

	// Bounds-Restricted Movement
	if (ballObj.pos.x < 0 || ballObj.pos.x > DISPLAY_WIDTH)
	{
		ballObj.pos.x = std::clamp(ballObj.pos.x, 0.f, DISPLAY_WIDTH);
		ballObj.velocity.x *= -1;
		ballObj.velocity.x += -0.5;
		ballObj.velocity.y += -0.5;
	}

	// No upper limit as this makes the ball bounce exponentially faster
	if (/*ballObj.pos.y < 0 || */ballObj.pos.y > DISPLAY_HEIGHT)
	{
		ballObj.pos.y = std::clamp(ballObj.pos.y, 0.f, DISPLAY_HEIGHT);
		ballObj.velocity.y *= -1;
		ballObj.acceleration.y *= 0.99;
		ballObj.velocity.x += -0.5;
		ballObj.velocity.y += -0.5;
	}

	//Rotation of ball
	ballObj.rotSpeed = 0.05f;
	ballObj.rotation += ballObj.rotSpeed;

	// Apply ball acceleration
	ballObj.velocity += ballObj.acceleration;

	// Apply ball velocity (remember, this is really just displacing the position once per frame)
	ballObj.pos += ballObj.velocity;

	// What happens when the paddle and ball collide
	if (paddleObj.pos.y + PADDLE_AABB.y > ballObj.pos.y - BALL_RADIUS
		&& paddleObj.pos.y - PADDLE_AABB.y < ballObj.pos.y + BALL_RADIUS)
	{
		if (paddleObj.pos.x + PADDLE_AABB.x > ballObj.pos.x - BALL_RADIUS
			&& paddleObj.pos.x - PADDLE_AABB.x < ballObj.pos.x + BALL_RADIUS)
		{
			ballObj.pos = ballObj.oldPos;
			ballObj.velocity.y *= -1;

			// Hits back up but adds a random velocity between -1.5 and 1.5
			// So the ball will rarely leave the top of the screen
			ballObj.velocity.y += Play::RandomRollRange(-1.5f, 1.5f);
		}
	}


	// Some extra code to let you 'reset' the ball if you press space
	if (Play::KeyDown(VK_SPACE))
	{
		ballObj.pos = Vector2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
		// Spawns in a 50/50 direction
		ballObj.velocity = BALL_VELOCITY_DEFAULT * Play::RandomRollRange(1, -1);
		ballObj.acceleration = BALL_ACCELERATION_DEFAULT;
	}

	Play::UpdateGameObject(ballObj);
	Play::UpdateGameObject(paddleObj);
}
void UpdatePaddle()
{
	// Grabbing paddle obj we created in entry
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };

	// left right controls for paddle
	if (Play::KeyDown(VK_LEFT))
	{
		paddleObj.pos.x += -10;
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		paddleObj.pos.x += 10;
	}
	else
	{
		paddleObj.velocity.x *= 0.9f;
	}
}

// Our draw function. Called by MainGameUpdate to render each frame. 
void Draw()
{
	// Reset our drawing buffer so it is white
	Play::ClearDrawingBuffer(Play::cWhite);

	Play::DrawBackground();

	// Draw the 'paddle'
	Play::DrawObject(Play::GetGameObjectByType(TYPE_PADDLE));

	// Draw the AABB box for our paddle
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };

	// Play::DrawRect(paddleObj.pos - PADDLE_AABB, paddleObj.pos + PADDLE_AABB, Play::cWhite);

	Play::DrawRect(paddleObj.pos - PADDLE_AABB, paddleObj.pos + PADDLE_AABB, Play::cWhite);

	// Draw the ball. This version of the function is slower, but uses the rotation variable stored in GameObjects.
	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_BALL));

	// Example Code for drawing text with an integer value to the screen
	int val{ 0 };
	Play::DrawFontText("64px", "High Score: " + std::to_string(val), Point2D(DISPLAY_WIDTH / 2, 100), Play::CENTRE);
	// Monitoring the ball's velocity and acceleration
	Play::DrawDebugText(Point2D(100, 70), "ballObj.velocity", Play::cRed);
	Play::DrawDebugText(Point2D(100, 80), "ballObj.acceleration", Play::cRed);
	// 'Paste' our drawing buffer to the visible screen so we can see it
	Play::PresentDrawingBuffer();
}