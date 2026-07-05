#include "character_creator.h"
#include "character_renderer.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define ARRAY_COUNT(items) ((int)(sizeof(items) / sizeof((items)[0])))

enum {
    SCREEN_WIDTH = 1280,
    SCREEN_HEIGHT = 720,
    TARGET_FPS = 60,
    TRAIL_POINT_COUNT = 72,
    STAR_COUNT = 360,
    CLOUD_BAND_COUNT = 5,
    CLOUDS_PER_BAND = 36,
    PLANET_SEGMENTS = 16,
    PLANET_RINGS = 8,
    LIMB_SEGMENTS = 6,
    MAX_TARGETS = 10,
    MAX_NAME_LENGTH = 32
};

typedef enum CharacterEditField {
    CHARACTER_EDIT_ARCHETYPE = 0,
    CHARACTER_EDIT_HEIGHT,
    CHARACTER_EDIT_TORSO,
    CHARACTER_EDIT_LEGS,
    CHARACTER_EDIT_HEAD,
    CHARACTER_EDIT_ASYMMETRY,
    CHARACTER_EDIT_PRIMARY_ACCESSORY,
    CHARACTER_EDIT_SECONDARY_ACCESSORY,
    CHARACTER_EDIT_TERTIARY_ACCESSORY,
    CHARACTER_EDIT_FLOATING,
    CHARACTER_EDIT_FIELD_COUNT
} CharacterEditField;

static const float CAMERA_FOV_DEGREES = 65.0f;
static const float SPACE_CAMERA_DISTANCE = 18.0f;
static const float SURFACE_CAMERA_DISTANCE = 7.0f;
static const float FIRST_PERSON_CAMERA_FORWARD = 0.45f;
static const float FIRST_PERSON_CAMERA_UP = 0.55f;
static const float THIRD_PERSON_CAMERA_UP = 3.0f;
static const float SURFACE_VIEW_ALTITUDE = 15.0f;
static const float ATMOSPHERE_ENTRY_ALTITUDE = 28.0f;
static const float PLANET_RADIUS = 24.0f;
static const float PLANET_ORBIT_Y = -6.0f;
static const float SHIP_ACCELERATION = 16.0f;
static const float TARGET_APPROACH_ACCELERATION = 21.0f;
static const float TARGET_RETREAT_ACCELERATION = 18.0f;
static const float DRAG_PER_SECOND = 0.28f;
static const float MAX_SHIP_SPEED = 48.0f;
static const float LIGHTSPEED_REFERENCE = 54.0f;
static const float RELATIVISTIC_FOV_BOOST = 32.0f;
static const float RELATIVISTIC_COLOR_BOOST = 0.65f;
static const float TARGET_MARKER_RADIUS = 1.2f;
static const float TARGET_NEAREST_MAX_DISTANCE = 100000.0f;
static const float SURFACE_CHARACTER_SCALE = 1.2f;
static const float CHARACTER_GLOW_RADIUS = 1.8f;
static const float TRAIL_MIN_ALPHA = 0.08f;
static const float TRAIL_MAX_ALPHA = 0.62f;
static const float CLOUD_ALTITUDE = 1.04f;
static const float CLOUD_SIZE = 0.55f;
static const float SURFACE_GRID_SIZE = 36.0f;
static const float SURFACE_GRID_SPACING = 2.0f;
static const float SKYBOX_RADIUS = 170.0f;
static const float RANDOM_SYSTEM_SPREAD = 78.0f;
static const float MULTIPLAYER_BOT_SPEED = 0.35f;
static const float MOUSE_SENSITIVITY = 0.0035f;
static const float PITCH_LIMIT = 1.25f;
static const float TARGET_CYCLE_COOLDOWN = 0.16f;
static const float CHARACTER_EDIT_SCALE_STEP = 0.05f;
static const float CHARACTER_EDIT_ASYMMETRY_STEP = 0.08f;
static const float CHARACTER_EDIT_HEIGHT_MIN = 0.75f;
static const float CHARACTER_EDIT_HEIGHT_MAX = 1.45f;
static const float CHARACTER_EDIT_TORSO_MIN = 0.25f;
static const float CHARACTER_EDIT_TORSO_MAX = 0.9f;
static const float CHARACTER_EDIT_LEGS_MIN = 0.75f;
static const float CHARACTER_EDIT_LEGS_MAX = 1.65f;
static const float CHARACTER_EDIT_HEAD_MIN = 0.65f;
static const float CHARACTER_EDIT_HEAD_MAX = 1.45f;
static const float RAGDOLL_SPEED_REFERENCE = 34.0f;
static const float RAGDOLL_TUMBLE_RATE = 2.8f;
static const float RAGDOLL_ROLL_MAX = 0.9f;
static const float RAGDOLL_PITCH_MAX = 0.55f;
static const float RAGDOLL_LIMB_SWING_MAX = 1.0f;
static const float RAGDOLL_COLLAPSE_MAX = 0.7f;

static const Color COLOR_SPACE_BACKGROUND = { 5, 7, 16, 255 };
static const Color COLOR_ATMOSPHERE = { 90, 145, 255, 90 };
static const Color COLOR_ENTRY_FIRE = { 255, 118, 28, 190 };
static const Color COLOR_TARGET = { 255, 236, 120, 255 };
static const Color COLOR_PLAYER_GLOW = { 90, 220, 255, 255 };
static const Color COLOR_BOT_GLOW = { 255, 90, 210, 255 };
static const Color COLOR_SURFACE_LINE = { 76, 120, 132, 110 };
static const Color COLOR_SURFACE_FILL = { 28, 44, 52, 255 };

struct Trail {
    Vector3 points[TRAIL_POINT_COUNT];
    int head;
    bool filled;
};

struct Actor {
    char name[MAX_NAME_LENGTH];
    Vector3 position;
    Vector3 velocity;
    Vector3 color_seed;
    bool is_player;
    CharacterDesign character_design;
    CharacterRagdollPose ragdoll_pose;
    float ragdoll_time;
    struct Trail trail;
};

struct WorldObject {
    char name[MAX_NAME_LENGTH];
    Vector3 position;
    float radius;
    Color color;
};

struct GameState {
    Camera3D camera;
    struct Actor player;
    struct Actor bots[3];
    struct WorldObject objects[6];
    Vector3 stars[STAR_COUNT];
    int target_count;
    int target_index;
    bool first_person;
    bool surface_mode;
    bool character_preview_mode;
    int preview_archetype;
    CharacterEditField character_edit_field;
    float yaw;
    float pitch;
    float target_cycle_timer;
    Texture2D planet_texture;
    Model planet_model;
    bool has_planet_texture;
};

static Vector3 Vector3ClampLength(Vector3 vector, float max_length) {
    float length = Vector3Length(vector);
    if (length <= max_length || length <= 0.0f) {
        return vector;
    }
    return Vector3Scale(vector, max_length / length);
}

static float ClampUnit(float value) {
    return Clamp(value, 0.0f, 1.0f);
}

static float PlayerSpeedFraction(const struct Actor *player) {
    return ClampUnit(Vector3Length(player->velocity) / LIGHTSPEED_REFERENCE);
}

static Vector3 PlanetCenter(void) {
    return (Vector3){ 0.0f, PLANET_ORBIT_Y, 0.0f };
}

static float AltitudeAbovePlanet(Vector3 position) {
    return Vector3Distance(position, PlanetCenter()) - PLANET_RADIUS;
}

static bool IsSurfaceMode(Vector3 position) {
    return AltitudeAbovePlanet(position) < SURFACE_VIEW_ALTITUDE;
}

static Vector3 ForwardFromAngles(float yaw, float pitch) {
    return Vector3Normalize((Vector3){
        sinf(yaw) * cosf(pitch),
        sinf(pitch),
        cosf(yaw) * cosf(pitch)
    });
}

static Vector3 RightFromYaw(float yaw) {
    return Vector3Normalize((Vector3){ cosf(yaw), 0.0f, -sinf(yaw) });
}

static void PushTrailPoint(struct Trail *trail, Vector3 point) {
    trail->points[trail->head] = point;
    trail->head = (trail->head + 1) % TRAIL_POINT_COUNT;
    if (trail->head == 0) {
        trail->filled = true;
    }
}

static Vector3 ReadTrailPoint(const struct Trail *trail, int age) {
    int index = trail->head - 1 - age;
    while (index < 0) {
        index += TRAIL_POINT_COUNT;
    }
    return trail->points[index % TRAIL_POINT_COUNT];
}

static void InitTrail(struct Trail *trail, Vector3 point) {
    for (int i = 0; i < TRAIL_POINT_COUNT; ++i) {
        trail->points[i] = point;
    }
    trail->head = 0;
    trail->filled = false;
}

static Color FadeColor(Color color, float alpha) {
    color.a = (unsigned char)Clamp(alpha * 255.0f, 0.0f, 255.0f);
    return color;
}

static void InitActor(struct Actor *actor, const char *name, Vector3 position, Color color, bool is_player, unsigned int actor_index) {
    snprintf(actor->name, MAX_NAME_LENGTH, "%s", name);
    actor->position = position;
    actor->velocity = Vector3Zero();
    actor->color_seed = (Vector3){ color.r / 255.0f, color.g / 255.0f, color.b / 255.0f };
    actor->is_player = is_player;
    actor->character_design = CharacterDesignCreateForActor(actor_index, is_player);
    actor->ragdoll_pose = (CharacterRagdollPose){ 0 };
    actor->ragdoll_time = (float)actor_index * 0.37f;
    InitTrail(&actor->trail, position);
}

static Color ActorColor(const struct Actor *actor) {
    return (Color){
        (unsigned char)(actor->color_seed.x * 255.0f),
        (unsigned char)(actor->color_seed.y * 255.0f),
        (unsigned char)(actor->color_seed.z * 255.0f),
        255
    };
}

static const char *CharacterEditFieldName(CharacterEditField field) {
    switch (field) {
        case CHARACTER_EDIT_ARCHETYPE: return "archetype";
        case CHARACTER_EDIT_HEIGHT: return "height";
        case CHARACTER_EDIT_TORSO: return "torso width";
        case CHARACTER_EDIT_LEGS: return "leg length";
        case CHARACTER_EDIT_HEAD: return "head scale";
        case CHARACTER_EDIT_ASYMMETRY: return "asymmetry";
        case CHARACTER_EDIT_PRIMARY_ACCESSORY: return "primary accessory";
        case CHARACTER_EDIT_SECONDARY_ACCESSORY: return "secondary accessory";
        case CHARACTER_EDIT_TERTIARY_ACCESSORY: return "tertiary accessory";
        case CHARACTER_EDIT_FLOATING: return "floating";
        case CHARACTER_EDIT_FIELD_COUNT: break;
    }
    return "unknown";
}

static CharacterAccessory CycleAccessory(CharacterAccessory accessory, int direction) {
    int accessory_count = (int)CHARACTER_ACCESSORY_FLOATING_DISC + 1;
    int next = ((int)accessory + direction + accessory_count) % accessory_count;
    return (CharacterAccessory)next;
}

static void ClampCharacterDesign(CharacterDesign *design) {
    design->height_scale = Clamp(design->height_scale, CHARACTER_EDIT_HEIGHT_MIN, CHARACTER_EDIT_HEIGHT_MAX);
    design->torso_width = Clamp(design->torso_width, CHARACTER_EDIT_TORSO_MIN, CHARACTER_EDIT_TORSO_MAX);
    design->leg_length = Clamp(design->leg_length, CHARACTER_EDIT_LEGS_MIN, CHARACTER_EDIT_LEGS_MAX);
    design->head_scale = Clamp(design->head_scale, CHARACTER_EDIT_HEAD_MIN, CHARACTER_EDIT_HEAD_MAX);
    design->asymmetry = Clamp(design->asymmetry, 0.0f, 1.0f);
}

static void AdjustCharacterEditor(struct GameState *game, int direction) {
    CharacterDesign *design = &game->player.character_design;
    switch (game->character_edit_field) {
        case CHARACTER_EDIT_ARCHETYPE:
            game->preview_archetype = (game->preview_archetype + direction + CHARACTER_ARCHETYPE_COUNT) % CHARACTER_ARCHETYPE_COUNT;
            game->player.character_design = CharacterDesignCreate(2003u + (unsigned int)game->preview_archetype, (CharacterArchetype)game->preview_archetype);
            break;
        case CHARACTER_EDIT_HEIGHT:
            design->height_scale += CHARACTER_EDIT_SCALE_STEP * (float)direction;
            break;
        case CHARACTER_EDIT_TORSO:
            design->torso_width += CHARACTER_EDIT_SCALE_STEP * (float)direction;
            break;
        case CHARACTER_EDIT_LEGS:
            design->leg_length += CHARACTER_EDIT_SCALE_STEP * (float)direction;
            break;
        case CHARACTER_EDIT_HEAD:
            design->head_scale += CHARACTER_EDIT_SCALE_STEP * (float)direction;
            break;
        case CHARACTER_EDIT_ASYMMETRY:
            design->asymmetry += CHARACTER_EDIT_ASYMMETRY_STEP * (float)direction;
            break;
        case CHARACTER_EDIT_PRIMARY_ACCESSORY:
            design->primary_accessory = CycleAccessory(design->primary_accessory, direction);
            break;
        case CHARACTER_EDIT_SECONDARY_ACCESSORY:
            design->secondary_accessory = CycleAccessory(design->secondary_accessory, direction);
            break;
        case CHARACTER_EDIT_TERTIARY_ACCESSORY:
            design->tertiary_accessory = CycleAccessory(design->tertiary_accessory, direction);
            break;
        case CHARACTER_EDIT_FLOATING:
            design->floats_above_ground = !design->floats_above_ground;
            break;
        case CHARACTER_EDIT_FIELD_COUNT:
            break;
    }
    ClampCharacterDesign(design);
}

static const char *CharacterEditValueText(const CharacterDesign *design, CharacterEditField field) {
    switch (field) {
        case CHARACTER_EDIT_ARCHETYPE: return CharacterArchetypeName(design->archetype);
        case CHARACTER_EDIT_PRIMARY_ACCESSORY: return CharacterAccessoryName(design->primary_accessory);
        case CHARACTER_EDIT_SECONDARY_ACCESSORY: return CharacterAccessoryName(design->secondary_accessory);
        case CHARACTER_EDIT_TERTIARY_ACCESSORY: return CharacterAccessoryName(design->tertiary_accessory);
        case CHARACTER_EDIT_FLOATING: return design->floats_above_ground ? "yes" : "no";
        case CHARACTER_EDIT_HEIGHT:
        case CHARACTER_EDIT_TORSO:
        case CHARACTER_EDIT_LEGS:
        case CHARACTER_EDIT_HEAD:
        case CHARACTER_EDIT_ASYMMETRY:
        case CHARACTER_EDIT_FIELD_COUNT:
            break;
    }
    return "numeric";
}

static float CharacterEditNumericValue(const CharacterDesign *design, CharacterEditField field) {
    switch (field) {
        case CHARACTER_EDIT_HEIGHT: return design->height_scale;
        case CHARACTER_EDIT_TORSO: return design->torso_width;
        case CHARACTER_EDIT_LEGS: return design->leg_length;
        case CHARACTER_EDIT_HEAD: return design->head_scale;
        case CHARACTER_EDIT_ASYMMETRY: return design->asymmetry;
        case CHARACTER_EDIT_ARCHETYPE:
        case CHARACTER_EDIT_PRIMARY_ACCESSORY:
        case CHARACTER_EDIT_SECONDARY_ACCESSORY:
        case CHARACTER_EDIT_TERTIARY_ACCESSORY:
        case CHARACTER_EDIT_FLOATING:
        case CHARACTER_EDIT_FIELD_COUNT:
            break;
    }
    return 0.0f;
}

static bool CharacterEditFieldIsNumeric(CharacterEditField field) {
    return field == CHARACTER_EDIT_HEIGHT || field == CHARACTER_EDIT_TORSO || field == CHARACTER_EDIT_LEGS || field == CHARACTER_EDIT_HEAD || field == CHARACTER_EDIT_ASYMMETRY;
}

static void ResetCharacterEditor(struct GameState *game) {
    game->player.character_design = CharacterDesignCreate(2003u + (unsigned int)game->preview_archetype, (CharacterArchetype)game->preview_archetype);
}

static void NextCharacterEditField(struct GameState *game) {
    game->character_edit_field = (CharacterEditField)(((int)game->character_edit_field + 1) % CHARACTER_EDIT_FIELD_COUNT);
}

static void PreviousCharacterEditField(struct GameState *game) {
    game->character_edit_field = (CharacterEditField)(((int)game->character_edit_field + CHARACTER_EDIT_FIELD_COUNT - 1) % CHARACTER_EDIT_FIELD_COUNT);
}

static void InitObjects(struct GameState *game) {
    game->objects[0] = (struct WorldObject){ "Earthlike planet", { 0.0f, PLANET_ORBIT_Y, 0.0f }, PLANET_RADIUS, { 70, 155, 110, 255 } };
    game->objects[1] = (struct WorldObject){ "Oolite beacon", { -34.0f, 14.0f, 26.0f }, 1.7f, { 255, 200, 70, 255 } };
    game->objects[2] = (struct WorldObject){ "Noctis ice moon", { 48.0f, -10.0f, -38.0f }, 6.0f, { 155, 196, 220, 255 } };
    game->objects[3] = (struct WorldObject){ "Comanche debug body", { -18.0f, 24.0f, -52.0f }, 3.2f, { 185, 120, 88, 255 } };
    game->objects[4] = (struct WorldObject){ "Gas giant test", { 72.0f, 22.0f, 58.0f }, 10.0f, { 210, 152, 92, 255 } };
    game->objects[5] = (struct WorldObject){ "Explore seed gate", { -62.0f, -4.0f, -18.0f }, 2.4f, { 156, 92, 255, 255 } };
}

static Vector3 DeterministicStarPosition(int index) {
    float longitude = (float)index * 2.399963f;
    float z = 1.0f - (2.0f * (float)index + 1.0f) / (float)STAR_COUNT;
    float radius = sqrtf(fmaxf(0.0f, 1.0f - z * z));
    return Vector3Scale((Vector3){ cosf(longitude) * radius, z, sinf(longitude) * radius }, SKYBOX_RADIUS);
}

static void InitStars(struct GameState *game) {
    for (int i = 0; i < STAR_COUNT; ++i) {
        game->stars[i] = DeterministicStarPosition(i);
    }
}

static void RandomizeExploreObjects(struct GameState *game) {
    for (int i = 1; i < ARRAY_COUNT(game->objects); ++i) {
        float seed = (float)(i * 37);
        game->objects[i].position = (Vector3){
            sinf(seed * 1.91f) * RANDOM_SYSTEM_SPREAD,
            cosf(seed * 0.73f) * (RANDOM_SYSTEM_SPREAD * 0.35f),
            sinf(seed * 0.41f) * RANDOM_SYSTEM_SPREAD
        };
    }
}

static void InitGame(struct GameState *game) {
    InitActor(&game->player, "Player", (Vector3){ 0.0f, 8.0f, 62.0f }, COLOR_PLAYER_GLOW, true, 0u);
    InitActor(&game->bots[0], "Remote racer A", (Vector3){ 16.0f, 10.0f, 54.0f }, COLOR_BOT_GLOW, false, 1u);
    InitActor(&game->bots[1], "Remote racer B", (Vector3){ -20.0f, 4.0f, 70.0f }, (Color){ 120, 255, 130, 255 }, false, 2u);
    InitActor(&game->bots[2], "Remote racer C", (Vector3){ 28.0f, 18.0f, 36.0f }, (Color){ 255, 170, 80, 255 }, false, 3u);
    InitObjects(game);
    InitStars(game);
    RandomizeExploreObjects(game);
    game->target_count = ARRAY_COUNT(game->objects) + ARRAY_COUNT(game->bots);
    if (game->target_count > MAX_TARGETS) {
        game->target_count = MAX_TARGETS;
    }
    game->target_index = 0;
    game->first_person = false;
    game->surface_mode = false;
    game->character_preview_mode = false;
    game->preview_archetype = 0;
    game->character_edit_field = CHARACTER_EDIT_ARCHETYPE;
    game->yaw = PI;
    game->pitch = -0.06f;
    game->target_cycle_timer = 0.0f;
    game->camera = (Camera3D){ 0 };
    game->camera.fovy = CAMERA_FOV_DEGREES;
    game->camera.projection = CAMERA_PERSPECTIVE;
    game->planet_texture = LoadTexture("earthlike.bmp");
    game->has_planet_texture = game->planet_texture.id != 0;
    if (game->has_planet_texture) {
        game->planet_model = LoadModelFromMesh(GenMeshSphere(PLANET_RADIUS, PLANET_RINGS, PLANET_SEGMENTS));
        game->planet_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = game->planet_texture;
    }
}

static Vector3 TargetPosition(const struct GameState *game, int index) {
    int object_count = ARRAY_COUNT(game->objects);
    if (index < object_count) {
        return game->objects[index].position;
    }
    return game->bots[index - object_count].position;
}

static const char *TargetName(const struct GameState *game, int index) {
    int object_count = ARRAY_COUNT(game->objects);
    if (index < object_count) {
        return game->objects[index].name;
    }
    return game->bots[index - object_count].name;
}

static void CycleTarget(struct GameState *game, int direction) {
    game->target_index = (game->target_index + direction + game->target_count) % game->target_count;
    game->target_cycle_timer = TARGET_CYCLE_COOLDOWN;
}

static void TargetNearest(struct GameState *game) {
    float best_distance = TARGET_NEAREST_MAX_DISTANCE;
    int best_index = game->target_index;
    for (int i = 0; i < game->target_count; ++i) {
        float distance = Vector3Distance(game->player.position, TargetPosition(game, i));
        if (distance < best_distance) {
            best_distance = distance;
            best_index = i;
        }
    }
    game->target_index = best_index;
    game->target_cycle_timer = TARGET_CYCLE_COOLDOWN;
}

static void UpdateControls(struct GameState *game, float delta) {
    Vector2 mouse_delta = GetMouseDelta();
    game->yaw -= mouse_delta.x * MOUSE_SENSITIVITY;
    game->pitch -= mouse_delta.y * MOUSE_SENSITIVITY;
    game->pitch = Clamp(game->pitch, -PITCH_LIMIT, PITCH_LIMIT);

    if (IsKeyPressed(KEY_V)) {
        game->first_person = !game->first_person;
    }
    if (IsKeyPressed(KEY_F6)) {
        game->character_preview_mode = !game->character_preview_mode;
    }
    if (game->character_preview_mode && IsKeyPressed(KEY_TAB)) {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
            PreviousCharacterEditField(game);
        } else {
            NextCharacterEditField(game);
        }
    }
    if (game->character_preview_mode && IsKeyPressed(KEY_R)) {
        ResetCharacterEditor(game);
    }
    if (game->character_preview_mode && IsKeyPressed(KEY_RIGHT)) {
        AdjustCharacterEditor(game, 1);
    }
    if (game->character_preview_mode && IsKeyPressed(KEY_LEFT)) {
        AdjustCharacterEditor(game, -1);
    }
    if (IsKeyPressed(KEY_F5)) {
        RandomizeExploreObjects(game);
    }

    game->target_cycle_timer = fmaxf(0.0f, game->target_cycle_timer - delta);
    if (game->target_cycle_timer <= 0.0f) {
        if (IsKeyPressed(KEY_LEFT_BRACKET)) {
            CycleTarget(game, -1);
        }
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
            CycleTarget(game, 1);
        }
        if (IsKeyPressed(KEY_SLASH)) {
            TargetNearest(game);
        }
    }

    Vector3 forward = ForwardFromAngles(game->yaw, game->pitch);
    Vector3 right = RightFromYaw(game->yaw);
    Vector3 up = Vector3CrossProduct(right, forward);
    Vector3 acceleration = Vector3Zero();

    if (IsKeyDown(KEY_W)) {
        acceleration = Vector3Add(acceleration, Vector3Scale(forward, SHIP_ACCELERATION));
    }
    if (IsKeyDown(KEY_S)) {
        acceleration = Vector3Subtract(acceleration, Vector3Scale(forward, SHIP_ACCELERATION));
    }
    if (IsKeyDown(KEY_A)) {
        acceleration = Vector3Subtract(acceleration, Vector3Scale(right, SHIP_ACCELERATION));
    }
    if (IsKeyDown(KEY_D)) {
        acceleration = Vector3Add(acceleration, Vector3Scale(right, SHIP_ACCELERATION));
    }
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        acceleration = Vector3Add(acceleration, Vector3Scale(up, SHIP_ACCELERATION));
    }
    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        acceleration = Vector3Subtract(acceleration, Vector3Scale(up, SHIP_ACCELERATION));
    }

    Vector3 target_vector = Vector3Subtract(TargetPosition(game, game->target_index), game->player.position);
    if (Vector3Length(target_vector) > 0.0f) {
        Vector3 target_direction = Vector3Normalize(target_vector);
        if (IsKeyDown(KEY_C)) {
            acceleration = Vector3Add(acceleration, Vector3Scale(target_direction, TARGET_APPROACH_ACCELERATION));
        }
        if (IsKeyDown(KEY_SPACE)) {
            acceleration = Vector3Subtract(acceleration, Vector3Scale(target_direction, TARGET_RETREAT_ACCELERATION));
        }
    }

    game->player.velocity = Vector3Add(game->player.velocity, Vector3Scale(acceleration, delta));
    game->player.velocity = Vector3Scale(game->player.velocity, fmaxf(0.0f, 1.0f - DRAG_PER_SECOND * delta));
    game->player.velocity = Vector3ClampLength(game->player.velocity, MAX_SHIP_SPEED);
    game->player.position = Vector3Add(game->player.position, Vector3Scale(game->player.velocity, delta));
    PushTrailPoint(&game->player.trail, game->player.position);
}

static void UpdateBots(struct GameState *game, float delta) {
    for (int i = 0; i < ARRAY_COUNT(game->bots); ++i) {
        float phase = (float)i * 2.2f + GetTime() * MULTIPLAYER_BOT_SPEED;
        Vector3 orbit_center = PlanetCenter();
        Vector3 desired = Vector3Add(orbit_center, (Vector3){ cosf(phase) * 38.0f, 12.0f + sinf(phase * 1.7f) * 9.0f, sinf(phase) * 38.0f });
        Vector3 to_desired = Vector3Subtract(desired, game->bots[i].position);
        game->bots[i].velocity = Vector3Scale(to_desired, 0.75f);
        game->bots[i].position = Vector3Add(game->bots[i].position, Vector3Scale(game->bots[i].velocity, delta));
        PushTrailPoint(&game->bots[i].trail, game->bots[i].position);
    }
}

static void UpdateActorRagdoll(struct Actor *actor, float delta, float phase_offset) {
    float speed = Vector3Length(actor->velocity);
    float speed_factor = Clamp(speed / RAGDOLL_SPEED_REFERENCE, 0.0f, 1.0f);
    float tumble_rate = RAGDOLL_TUMBLE_RATE * (0.35f + speed_factor);
    actor->ragdoll_time += delta * tumble_rate;
    float phase = actor->ragdoll_time + phase_offset;
    actor->ragdoll_pose.body_roll = sinf(phase * 0.73f) * RAGDOLL_ROLL_MAX * speed_factor;
    actor->ragdoll_pose.body_pitch = cosf(phase * 0.61f) * RAGDOLL_PITCH_MAX * speed_factor;
    actor->ragdoll_pose.left_arm_swing = sinf(phase * 1.37f) * RAGDOLL_LIMB_SWING_MAX;
    actor->ragdoll_pose.right_arm_swing = cosf(phase * 1.19f) * RAGDOLL_LIMB_SWING_MAX;
    actor->ragdoll_pose.left_leg_swing = cosf(phase * 1.11f) * RAGDOLL_LIMB_SWING_MAX;
    actor->ragdoll_pose.right_leg_swing = sinf(phase * 1.29f) * RAGDOLL_LIMB_SWING_MAX;
    actor->ragdoll_pose.collapse = RAGDOLL_COLLAPSE_MAX * speed_factor;
}

static void UpdateRagdolls(struct GameState *game, float delta) {
    UpdateActorRagdoll(&game->player, delta, 0.0f);
    for (int i = 0; i < ARRAY_COUNT(game->bots); ++i) {
        UpdateActorRagdoll(&game->bots[i], delta, (float)(i + 1) * 0.83f);
    }
}

static void UpdateRaceCamera(struct GameState *game) {
    Vector3 forward = ForwardFromAngles(game->yaw, game->pitch);
    float speed_fraction = PlayerSpeedFraction(&game->player);
    game->surface_mode = IsSurfaceMode(game->player.position);
    game->camera.fovy = CAMERA_FOV_DEGREES + RELATIVISTIC_FOV_BOOST * speed_fraction * speed_fraction;

    if (game->first_person) {
        game->camera.position = Vector3Add(game->player.position, (Vector3){ 0.0f, FIRST_PERSON_CAMERA_UP, 0.0f });
        game->camera.target = Vector3Add(game->camera.position, Vector3Scale(forward, FIRST_PERSON_CAMERA_FORWARD));
    } else {
        float camera_distance = game->surface_mode ? SURFACE_CAMERA_DISTANCE : SPACE_CAMERA_DISTANCE;
        game->camera.position = Vector3Subtract(game->player.position, Vector3Scale(forward, camera_distance));
        game->camera.position.y += THIRD_PERSON_CAMERA_UP;
        game->camera.target = game->player.position;
    }
    game->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
}

static void DrawLowPolyPlanet(const struct GameState *game) {
    Vector3 center = PlanetCenter();
    if (game->has_planet_texture) {
        DrawModel(game->planet_model, center, 1.0f, WHITE);
    } else {
        DrawSphere(center, PLANET_RADIUS, game->objects[0].color);
    }
    DrawSphereWires(center, PLANET_RADIUS * 1.002f, PLANET_RINGS, PLANET_SEGMENTS, FadeColor(BLACK, 0.25f));
    DrawSphereWires(center, PLANET_RADIUS * 1.08f, PLANET_RINGS, PLANET_SEGMENTS, COLOR_ATMOSPHERE);
}

static void DrawStars(const struct GameState *game) {
    for (int i = 0; i < STAR_COUNT; ++i) {
        DrawSphere(game->stars[i], 0.08f + (float)(i % 3) * 0.025f, FadeColor(WHITE, 0.78f));
    }
}

static void DrawClouds(void) {
    Vector3 center = PlanetCenter();
    for (int band = 0; band < CLOUD_BAND_COUNT; ++band) {
        float latitude = -0.7f + (float)band * 0.35f;
        for (int i = 0; i < CLOUDS_PER_BAND; ++i) {
            float longitude = ((float)i / (float)CLOUDS_PER_BAND) * 2.0f * PI + (float)band * 0.21f;
            Vector3 normal = Vector3Normalize((Vector3){ cosf(longitude) * cosf(latitude), sinf(latitude), sinf(longitude) * cosf(latitude) });
            Vector3 position = Vector3Add(center, Vector3Scale(normal, PLANET_RADIUS * CLOUD_ALTITUDE));
            DrawCube(position, CLOUD_SIZE * 2.2f, CLOUD_SIZE * 0.32f, CLOUD_SIZE, FadeColor(WHITE, 0.48f));
        }
    }
}

static void DrawWorldObjects(const struct GameState *game) {
    for (int i = 1; i < ARRAY_COUNT(game->objects); ++i) {
        DrawSphere(game->objects[i].position, game->objects[i].radius, game->objects[i].color);
        DrawSphereWires(game->objects[i].position, game->objects[i].radius * 1.05f, PLANET_RINGS, PLANET_SEGMENTS, FadeColor(WHITE, 0.28f));
    }
}

static void DrawActorTrail(const struct Actor *actor) {
    Color color = ActorColor(actor);
    for (int age = 1; age < TRAIL_POINT_COUNT - 1; ++age) {
        float t = 1.0f - (float)age / (float)TRAIL_POINT_COUNT;
        float alpha = TRAIL_MIN_ALPHA + (TRAIL_MAX_ALPHA - TRAIL_MIN_ALPHA) * t;
        Vector3 start = ReadTrailPoint(&actor->trail, age);
        Vector3 end = ReadTrailPoint(&actor->trail, age + 1);
        DrawCylinderEx(start, end, 0.08f * t, 0.03f * t, LIMB_SEGMENTS, FadeColor(color, alpha));
    }
}

static void DrawActor(const struct Actor *actor) {
    Color color = ActorColor(actor);
    DrawActorTrail(actor);
    DrawSphereWires(actor->position, CHARACTER_GLOW_RADIUS, LIMB_SEGMENTS, LIMB_SEGMENTS, FadeColor(color, 0.42f));
    DrawCulturenikCharacterPose(&actor->character_design, &actor->ragdoll_pose, actor->position, SURFACE_CHARACTER_SCALE);
}

static void DrawSurfaceGrid(void) {
    Vector3 center = PlanetCenter();
    float ground_y = center.y + PLANET_RADIUS;
    for (float x = -SURFACE_GRID_SIZE; x <= SURFACE_GRID_SIZE; x += SURFACE_GRID_SPACING) {
        DrawLine3D((Vector3){ x, ground_y, -SURFACE_GRID_SIZE }, (Vector3){ x, ground_y, SURFACE_GRID_SIZE }, COLOR_SURFACE_LINE);
    }
    for (float z = -SURFACE_GRID_SIZE; z <= SURFACE_GRID_SIZE; z += SURFACE_GRID_SPACING) {
        DrawLine3D((Vector3){ -SURFACE_GRID_SIZE, ground_y, z }, (Vector3){ SURFACE_GRID_SIZE, ground_y, z }, COLOR_SURFACE_LINE);
    }
    DrawCube((Vector3){ 0.0f, ground_y - 0.08f, 0.0f }, SURFACE_GRID_SIZE * 2.0f, 0.05f, SURFACE_GRID_SIZE * 2.0f, COLOR_SURFACE_FILL);
}

static void DrawTargetMarker(const struct GameState *game) {
    Vector3 target = TargetPosition(game, game->target_index);
    DrawSphereWires(target, TARGET_MARKER_RADIUS, PLANET_RINGS, PLANET_SEGMENTS, COLOR_TARGET);
    DrawLine3D(game->player.position, target, FadeColor(COLOR_TARGET, 0.32f));
}

static void DrawEntryEffects(const struct GameState *game) {
    float altitude = AltitudeAbovePlanet(game->player.position);
    float entry_factor = ClampUnit((ATMOSPHERE_ENTRY_ALTITUDE - altitude) / ATMOSPHERE_ENTRY_ALTITUDE);
    float speed_factor = PlayerSpeedFraction(&game->player);
    if (entry_factor <= 0.0f || speed_factor <= 0.12f) {
        return;
    }
    Color fire = FadeColor(COLOR_ENTRY_FIRE, entry_factor * speed_factor);
    DrawSphere(game->player.position, 2.0f + 5.0f * speed_factor, fire);
}

static void DrawScene(struct GameState *game) {
    BeginMode3D(game->camera);
    DrawStars(game);
    DrawLowPolyPlanet(game);
    DrawClouds();
    DrawWorldObjects(game);
    if (game->surface_mode) {
        DrawSurfaceGrid();
    }
    for (int i = 0; i < ARRAY_COUNT(game->bots); ++i) {
        DrawActor(&game->bots[i]);
    }
    if (!game->first_person || game->character_preview_mode) {
        DrawActor(&game->player);
    } else {
        DrawActorTrail(&game->player);
    }
    DrawTargetMarker(game);
    DrawEntryEffects(game);
    EndMode3D();
}

static void DrawHud(const struct GameState *game) {
    float speed = Vector3Length(game->player.velocity);
    float speed_fraction = PlayerSpeedFraction(&game->player);
    float altitude = AltitudeAbovePlanet(game->player.position);
    Color relativistic_color = FadeColor((Color){ 120, 200, 255, 255 }, RELATIVISTIC_COLOR_BOOST * speed_fraction);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, relativistic_color);
    DrawText("RFL Hermes low-poly racer", 20, 18, 24, RAYWHITE);
    DrawText(TextFormat("Target: %s   controls: [ ] cycle  / nearest  c approach  space move away", TargetName(game, game->target_index)), 20, 52, 18, COLOR_TARGET);
    DrawText(TextFormat("Speed %.1f / lightspeed-ref %.1f   relativistic %.0f%%   altitude %.1f", speed, LIGHTSPEED_REFERENCE, speed_fraction * 100.0f, altitude), 20, 78, 18, RAYWHITE);
    DrawText(TextFormat("Mode: %s camera, %s-mode   W/A/S/D fly, Shift/Ctrl vertical, V camera, F5 explore seed", game->first_person ? "first-person" : "third-person", game->surface_mode ? "surface" : "space"), 20, 104, 18, RAYWHITE);
    if (game->character_preview_mode) {
        CharacterEditField field = game->character_edit_field;
        const CharacterDesign *design = &game->player.character_design;
        if (CharacterEditFieldIsNumeric(field)) {
            DrawText(TextFormat("Character editor: %s = %.2f   Tab field  Left/Right edit  R reset  F6 exits", CharacterEditFieldName(field), CharacterEditNumericValue(design, field)), 20, 130, 18, COLOR_TARGET);
        } else {
            DrawText(TextFormat("Character editor: %s = %s   Tab field  Left/Right edit  R reset  F6 exits", CharacterEditFieldName(field), CharacterEditValueText(design, field)), 20, 130, 18, COLOR_TARGET);
        }
        DrawText(TextFormat("Archetype: %s   accessories: %s / %s / %s", CharacterArchetypeName(design->archetype), CharacterAccessoryName(design->primary_accessory), CharacterAccessoryName(design->secondary_accessory), CharacterAccessoryName(design->tertiary_accessory)), 20, 156, 18, FadeColor(RAYWHITE, 0.82f));
    } else {
        DrawText("F6 character editor preview", 20, 130, 18, FadeColor(RAYWHITE, 0.72f));
    }
    DrawText("Culturenik figures ragdoll in space; F6 opens the character editor.", 20, SCREEN_HEIGHT - 36, 18, FadeColor(RAYWHITE, 0.78f));
}

int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RFL Hermes low-poly space/surface racer");
    SetTargetFPS(TARGET_FPS);
    DisableCursor();

    struct GameState game = { 0 };
    InitGame(&game);

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        UpdateControls(&game, delta);
        UpdateBots(&game, delta);
        UpdateRagdolls(&game, delta);
        UpdateRaceCamera(&game);

        BeginDrawing();
        ClearBackground(COLOR_SPACE_BACKGROUND);
        DrawScene(&game);
        DrawHud(&game);
        EndDrawing();
    }

    if (game.has_planet_texture) {
        UnloadModel(game.planet_model);
        UnloadTexture(game.planet_texture);
    }
    CloseWindow();
    return 0;
}
