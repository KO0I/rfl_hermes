#include "character_renderer.h"

#include "raymath.h"

#include <math.h>

static const int CULTURENIK_LIMB_SEGMENTS = 6;
static const float CULTURENIK_TORSO_HEIGHT = 1.05f;
static const float CULTURENIK_HEAD_HEIGHT = 1.72f;
static const float CULTURENIK_HIP_HEIGHT = 0.32f;
static const float CULTURENIK_ARM_DROP = 0.55f;
static const float CULTURENIK_LEG_DROP = 0.78f;
static const float CULTURENIK_GOGGLE_RADIUS = 0.075f;
static const float CULTURENIK_FEATHER_LENGTH = 0.42f;
static const float CULTURENIK_PROP_LENGTH = 1.45f;
static const float CULTURENIK_GLOW_LINE_RADIUS = 0.018f;
static const float CULTURENIK_FLOAT_OFFSET = 0.28f;
static const float CULTURENIK_BOOT_WIDTH = 0.22f;
static const float CULTURENIK_BOOT_HEIGHT = 0.12f;
static const float CULTURENIK_BOOT_DEPTH = 0.32f;
static const float CULTURENIK_DISC_RADIUS = 0.28f;
static const float CULTURENIK_DISC_HEIGHT = 0.08f;

static bool HasAccessory(const CharacterDesign *design, CharacterAccessory accessory) {
    return design->primary_accessory == accessory || design->secondary_accessory == accessory || design->tertiary_accessory == accessory;
}

static Color FadeCharacterColor(Color color, float alpha) {
    color.a = (unsigned char)Clamp(alpha * 255.0f, 0.0f, 255.0f);
    return color;
}

static Vector3 Offset(Vector3 position, float x, float y, float z) {
    return Vector3Add(position, (Vector3){ x, y, z });
}

static Vector3 ApplyPose(Vector3 joint, Vector3 origin, const CharacterRagdollPose *pose) {
    if (pose == 0) {
        return joint;
    }
    Vector3 local = Vector3Subtract(joint, origin);
    float roll_cos = cosf(pose->body_roll);
    float roll_sin = sinf(pose->body_roll);
    float pitch_cos = cosf(pose->body_pitch);
    float pitch_sin = sinf(pose->body_pitch);
    Vector3 rolled = {
        local.x * roll_cos - local.y * roll_sin,
        local.x * roll_sin + local.y * roll_cos,
        local.z
    };
    Vector3 pitched = {
        rolled.x,
        rolled.y * pitch_cos - rolled.z * pitch_sin,
        rolled.y * pitch_sin + rolled.z * pitch_cos
    };
    return Vector3Add(origin, pitched);
}

static void DrawLowPolyLimb(Vector3 start, Vector3 end, float radius, Color color) {
    DrawCylinderEx(start, end, radius, radius * 0.82f, CULTURENIK_LIMB_SEGMENTS, color);
}

static void DrawAngularHair(Vector3 head, float scale, Color color, float asymmetry) {
    DrawCube(Offset(head, -0.12f * scale, 0.20f * scale, 0.02f * scale), 0.32f * scale, 0.20f * scale, 0.28f * scale, color);
    DrawCube(Offset(head, 0.18f * scale, 0.10f * scale, 0.00f), 0.28f * scale, 0.16f * scale, 0.24f * scale, color);
    if (asymmetry > 0.4f) {
        DrawCube(Offset(head, 0.34f * scale, -0.02f * scale, 0.0f), 0.24f * scale, 0.12f * scale, 0.20f * scale, color);
    }
}

static void DrawGoggles(Vector3 head, float scale, Color lens_color, Color strap_color) {
    Vector3 left_lens = Offset(head, -0.12f * scale, 0.02f * scale, 0.28f * scale);
    Vector3 right_lens = Offset(head, 0.12f * scale, 0.02f * scale, 0.28f * scale);
    DrawSphere(left_lens, CULTURENIK_GOGGLE_RADIUS * scale, lens_color);
    DrawSphere(right_lens, CULTURENIK_GOGGLE_RADIUS * scale, lens_color);
    DrawCylinderEx(left_lens, right_lens, 0.018f * scale, 0.018f * scale, CULTURENIK_LIMB_SEGMENTS, strap_color);
}

static void DrawFeathers(Vector3 head, float scale, Color first_color, Color second_color) {
    Vector3 root = Offset(head, 0.02f * scale, 0.28f * scale, 0.0f);
    DrawCylinderEx(root, Offset(root, 0.24f * scale, CULTURENIK_FEATHER_LENGTH * scale, 0.0f), 0.035f * scale, 0.005f * scale, CULTURENIK_LIMB_SEGMENTS, first_color);
    DrawCylinderEx(root, Offset(root, -0.18f * scale, CULTURENIK_FEATHER_LENGTH * 0.86f * scale, 0.0f), 0.035f * scale, 0.005f * scale, CULTURENIK_LIMB_SEGMENTS, second_color);
}

static void DrawLongProp(Vector3 position, float scale, Color body_color, Color accent_color) {
    Vector3 start = Offset(position, 0.42f * scale, 1.05f * scale, 0.18f * scale);
    Vector3 end = Offset(position, 0.42f * scale, (1.05f - CULTURENIK_PROP_LENGTH) * scale, 0.18f * scale);
    DrawCylinderEx(start, end, 0.055f * scale, 0.06f * scale, CULTURENIK_LIMB_SEGMENTS, body_color);
    DrawCube(Offset(start, 0.0f, 0.18f * scale, 0.0f), 0.14f * scale, 0.36f * scale, 0.12f * scale, accent_color);
    DrawCube(Offset(end, 0.05f * scale, -0.10f * scale, 0.0f), 0.20f * scale, 0.10f * scale, 0.12f * scale, accent_color);
}

static void DrawBeltPouch(Vector3 hip, float scale, Color body_color, Color accent_color) {
    DrawCube(Offset(hip, 0.36f * scale, 0.06f * scale, 0.16f * scale), 0.20f * scale, 0.28f * scale, 0.12f * scale, body_color);
    DrawCube(Offset(hip, 0.36f * scale, 0.19f * scale, 0.23f * scale), 0.22f * scale, 0.04f * scale, 0.04f * scale, accent_color);
}

static void DrawFloatingDisc(Vector3 head, float scale, Color body_color, Color accent_color) {
    Vector3 disc_center = Offset(head, -0.48f * scale, 0.20f * scale, 0.0f);
    DrawCylinderEx(Offset(disc_center, 0.0f, -CULTURENIK_DISC_HEIGHT * scale, 0.0f), Offset(disc_center, 0.0f, CULTURENIK_DISC_HEIGHT * scale, 0.0f), CULTURENIK_DISC_RADIUS * scale, CULTURENIK_DISC_RADIUS * scale, CULTURENIK_LIMB_SEGMENTS, body_color);
    DrawCylinderEx(Offset(disc_center, -0.12f * scale, 0.0f, 0.0f), Offset(disc_center, 0.12f * scale, 0.0f, 0.0f), 0.035f * scale, 0.035f * scale, CULTURENIK_LIMB_SEGMENTS, accent_color);
}

static void DrawBodyLine(Vector3 start, Vector3 end, float scale, Color glow_color) {
    DrawCylinderEx(start, end, CULTURENIK_GLOW_LINE_RADIUS * scale, CULTURENIK_GLOW_LINE_RADIUS * scale, CULTURENIK_LIMB_SEGMENTS, glow_color);
}

void DrawCulturenikCharacterPose(const CharacterDesign *design, const CharacterRagdollPose *pose, Vector3 position, float base_scale) {
    float scale = base_scale * design->height_scale;
    if (design->floats_above_ground) {
        position.y += CULTURENIK_FLOAT_OFFSET * scale;
    }

    Color skin = design->palette[0];
    Color clothing = design->palette[1];
    Color accent = design->palette[2];
    Color dark = design->palette[3];
    Color highlight = design->palette[4];
    Color hair = design->palette[5];

    float collapse = pose == 0 ? 0.0f : Clamp(pose->collapse, 0.0f, 1.0f);
    Vector3 pose_origin = position;
    Vector3 hip = ApplyPose(Offset(position, 0.0f, (CULTURENIK_HIP_HEIGHT - collapse * 0.24f) * scale, 0.0f), pose_origin, pose);
    Vector3 torso = ApplyPose(Offset(position, 0.0f, (CULTURENIK_TORSO_HEIGHT - collapse * 0.38f) * scale, 0.0f), pose_origin, pose);
    Vector3 head = ApplyPose(Offset(position, 0.0f, (CULTURENIK_HEAD_HEIGHT - collapse * 0.52f) * scale, 0.0f), pose_origin, pose);
    Vector3 left_shoulder = ApplyPose(Offset(torso, -design->torso_width * scale, (-design->shoulder_slope - collapse * 0.18f) * scale, 0.0f), pose_origin, pose);
    Vector3 right_shoulder = ApplyPose(Offset(torso, design->torso_width * scale, (design->shoulder_slope * 0.2f - collapse * 0.12f) * scale, 0.0f), pose_origin, pose);
    float left_arm_swing = pose == 0 ? 0.0f : pose->left_arm_swing;
    float right_arm_swing = pose == 0 ? 0.0f : pose->right_arm_swing;
    float left_leg_swing = pose == 0 ? 0.0f : pose->left_leg_swing;
    float right_leg_swing = pose == 0 ? 0.0f : pose->right_leg_swing;
    Vector3 left_hand = ApplyPose(Offset(position, (-0.58f - 0.16f * left_arm_swing) * scale, (CULTURENIK_ARM_DROP - collapse * 0.42f + 0.24f * left_arm_swing) * scale, (0.08f + 0.28f * left_arm_swing) * scale), pose_origin, pose);
    Vector3 right_hand = ApplyPose(Offset(position, (0.58f + 0.16f * right_arm_swing) * scale, (CULTURENIK_ARM_DROP + design->asymmetry * 0.18f - collapse * 0.42f + 0.24f * right_arm_swing) * scale, (0.08f + 0.28f * right_arm_swing) * scale), pose_origin, pose);
    Vector3 left_foot = ApplyPose(Offset(position, (-0.25f - 0.22f * left_leg_swing) * scale, (-CULTURENIK_LEG_DROP * design->leg_length + collapse * 0.28f) * scale, (0.05f + 0.32f * left_leg_swing) * scale), pose_origin, pose);
    Vector3 right_foot = ApplyPose(Offset(position, (0.28f + 0.22f * right_leg_swing) * scale, (-CULTURENIK_LEG_DROP * design->leg_length + collapse * 0.28f) * scale, (-0.03f + 0.32f * right_leg_swing) * scale), pose_origin, pose);

    DrawSphereWires(position, 0.62f * scale, CULTURENIK_LIMB_SEGMENTS, CULTURENIK_LIMB_SEGMENTS, FadeCharacterColor(design->glow_color, 0.42f));
    DrawCube(torso, design->torso_width * scale, 0.92f * scale, 0.26f * scale, clothing);
    DrawCube(hip, design->torso_width * 0.82f * scale, 0.28f * scale, 0.22f * scale, dark);
    DrawSphere(head, 0.24f * design->head_scale * scale, skin);
    DrawAngularHair(head, scale, hair, design->asymmetry);

    DrawLowPolyLimb(left_shoulder, left_hand, 0.045f * scale, skin);
    DrawLowPolyLimb(right_shoulder, right_hand, 0.045f * scale, skin);
    DrawLowPolyLimb(hip, left_foot, 0.055f * scale, skin);
    DrawLowPolyLimb(hip, right_foot, 0.055f * scale, skin);

    DrawCube(Offset(left_foot, 0.0f, -0.05f * scale, 0.05f * scale), CULTURENIK_BOOT_WIDTH * scale, CULTURENIK_BOOT_HEIGHT * scale, CULTURENIK_BOOT_DEPTH * scale, dark);
    DrawCube(Offset(right_foot, 0.0f, -0.05f * scale, 0.05f * scale), CULTURENIK_BOOT_WIDTH * scale, CULTURENIK_BOOT_HEIGHT * scale, CULTURENIK_BOOT_DEPTH * scale, dark);

    if (HasAccessory(design, CHARACTER_ACCESSORY_LONG_COAT)) {
        DrawCube(Offset(torso, 0.0f, -0.32f * scale, -0.04f * scale), design->torso_width * 1.35f * scale, 1.42f * scale, 0.20f * scale, FadeCharacterColor(dark, 0.92f));
    }
    if (HasAccessory(design, CHARACTER_ACCESSORY_GOGGLES)) {
        DrawGoggles(head, scale, accent, dark);
    }
    if (HasAccessory(design, CHARACTER_ACCESSORY_FEATHERS)) {
        DrawFeathers(head, scale, accent, highlight);
    }
    if (HasAccessory(design, CHARACTER_ACCESSORY_LONG_PROP)) {
        DrawLongProp(position, scale, dark, accent);
    }
    if (HasAccessory(design, CHARACTER_ACCESSORY_BELT_POUCH)) {
        DrawBeltPouch(hip, scale, dark, accent);
    }
    if (HasAccessory(design, CHARACTER_ACCESSORY_FLOATING_DISC)) {
        DrawFloatingDisc(head, scale, dark, accent);
    }
    if (HasAccessory(design, CHARACTER_ACCESSORY_BODY_GLOW_LINES)) {
        DrawBodyLine(head, torso, scale, design->glow_color);
        DrawBodyLine(torso, left_foot, scale, design->glow_color);
        DrawBodyLine(torso, right_foot, scale, design->glow_color);
    }
    if (HasAccessory(design, CHARACTER_ACCESSORY_TATTOO_LINES)) {
        DrawBodyLine(left_shoulder, right_shoulder, scale, highlight);
        DrawBodyLine(Offset(torso, -0.18f * scale, 0.0f, 0.15f * scale), Offset(hip, 0.16f * scale, -0.16f * scale, 0.15f * scale), scale, highlight);
    }
}

void DrawCulturenikCharacter(const CharacterDesign *design, Vector3 position, float base_scale) {
    DrawCulturenikCharacterPose(design, 0, position, base_scale);
}
