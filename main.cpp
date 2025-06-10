#include "raylib.h"
#include "box2d/box2d.h"
#include "box2d/types.h"
#include <vector>
#include <random>

constexpr int windowWidth = 640;
constexpr int windowHeight = 480;
constexpr float ppm = 100.0f;
constexpr float timeStep = 1.0f / 60;
constexpr int subStep = 4;

Vector2 m2PxVec(const b2Vec2 vec) {
    return Vector2{vec.x * ppm, vec.y * ppm};
}

b2Vec2 px2MVec(const Vector2 vec) {
    return b2Vec2{vec.x / ppm, vec.y / ppm};
}

float m2Px(const float n) {
    return n * ppm;
}

float px2M(const float n) {
    return n / ppm;
}

int getRand(const int min, const int max) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> distribution(min, max);
    return distribution(gen);
}

class Platform {
public:
    static std::vector<Platform> platforms;

    int m_X;
    int m_Y;

    int m_width;
    int m_height;

    b2Vec2 m_position{};
    b2BodyDef m_bodyDef;
    b2BodyId m_body{};
    b2Polygon m_boundingBox{};
    b2Vec2 m_boundingBoxSize{};
    b2ShapeDef m_shapeDef{};

    Platform(const int X, const int Y, const int width, const int height, const b2WorldId world) :
    m_X(X),
    m_Y(Y),
    m_width(width),
    m_height(height),
    m_bodyDef(b2DefaultBodyDef())
    {
        m_position = px2MVec({
            static_cast<float>(m_X) + static_cast<float>(m_width) / 2.0f,
            static_cast<float>(m_Y) + static_cast<float>(m_height) / 2.0f});

        m_bodyDef.position = m_position;
        m_bodyDef.type = b2_staticBody;
        m_body = b2CreateBody(world, &m_bodyDef);

        m_boundingBoxSize = px2MVec({
            static_cast<float>(m_width) / 2.0f,
            static_cast<float>(m_height) / 2.0f
        });

        m_boundingBox = b2MakeBox(m_boundingBoxSize.x, m_boundingBoxSize.y);

        m_shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(m_body, &m_shapeDef, &m_boundingBox);

        platforms.emplace_back(*this);
    }

    static void drawPlatforms() {
        for (const auto& platform : platforms) {
            DrawRectangle(
                platform.m_X,
                platform.m_Y,
                platform.m_width,
                platform.m_height,
                WHITE
                );
        }
    }
};

class Box {
public:
    static std::vector<Box> m_boxes;

    mutable int m_X;
    int m_Y;

    int m_width;
    int m_height;

    mutable b2Vec2 m_position{};
    Vector2 m_origin{};
    mutable b2Rot m_rotation{};
    b2BodyDef m_bodyDef;
    b2BodyId m_body{};
    b2Polygon m_boundingBox{};
    b2Vec2 m_boundingBoxSize{};
    b2ShapeDef m_shapeDef{};

    Box(int X, int Y, b2WorldId world) :
    m_X(X),
    m_Y(Y),
    m_width(getRand(4, 20)),
    m_height(getRand(4, 20)),
    m_rotation{0.0f, 0.0f},
    m_bodyDef(b2DefaultBodyDef())
    {
        m_position = px2MVec({
            static_cast<float>(m_X) + static_cast<float>(m_width) / 2,
            static_cast<float>(m_Y) + static_cast<float>(m_height) / 2
        });

        m_bodyDef.position = m_position;
        m_bodyDef.type = b2_dynamicBody;
        m_body = b2CreateBody(world, &m_bodyDef);

        m_boundingBoxSize = px2MVec({
            static_cast<float>(m_width) / 2.0f,
            static_cast<float>(m_height) / 2.0f
        });

        m_boundingBox = b2MakeBox(m_boundingBoxSize.x, m_boundingBoxSize.y);
        m_shapeDef = b2DefaultShapeDef();
        m_shapeDef.material.restitution = 0.40f;
        m_shapeDef.material.friction = 0.10f;
        b2CreatePolygonShape(m_body, &m_shapeDef, &m_boundingBox);
    }

    static void generateBoxAtMouse(const int mouseX, const int mouseY, const b2WorldId world) {
        Box box = Box(mouseX, mouseY, world);
        m_boxes.emplace_back(box);
    }

    static void updateBoxes() {
        for (const auto& box : m_boxes) {
            box.m_position = b2Body_GetPosition(box.m_body);
            box.m_rotation = b2Body_GetRotation(box.m_body);
        }
    }

    static void drawBoxes() {
        for (const auto& box : m_boxes) {
            const Rectangle rect = {
                m2Px(box.m_position.x),
                m2Px(box.m_position.y),
                static_cast<float>(box.m_width),
                static_cast<float>(box.m_height)
            };

            const Vector2 origin = {
                static_cast<float>(box.m_width) / 2.0f,
                static_cast<float>(box.m_height) / 2.0f};

            const float rotation = b2Rot_GetAngle(box.m_rotation) * RAD2DEG;

            DrawRectanglePro(rect, origin, rotation, RED);
        }
    }
};

std::vector<Box> Box::m_boxes = {};
std::vector<Platform> Platform::platforms = {};

int main() {
    InitWindow(windowWidth, windowHeight, "Box2D test");
    SetTargetFPS(60);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, +10.0f};

    const b2WorldId worldId = b2CreateWorld(&worldDef);

    Platform(30, 420, 250, 30, worldId);
    Platform(280, 310, 320, 30, worldId);

    int boxClock = 0;

    while (!WindowShouldClose()) {
        // Update
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            if (boxClock == 3) {
                Box::generateBoxAtMouse(GetMouseX(), GetMouseY(), worldId);
                boxClock = 0;
            }
            else {
                boxClock++;
            }
        }
        b2World_Step(worldId, timeStep, subStep);
        Box::updateBoxes();

        BeginDrawing();
        ClearBackground(BLACK);
        Box::drawBoxes();
        Platform::drawPlatforms();
        EndDrawing();
    }

    return 0;
}