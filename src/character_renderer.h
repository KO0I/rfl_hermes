#ifndef RFL_CHARACTER_RENDERER_H
#define RFL_CHARACTER_RENDERER_H

#include "character_creator.h"
#include "raylib.h"

typedef struct CharacterRagdollPose {
    float body_roll;
    float body_pitch;
    float left_arm_swing;
    float right_arm_swing;
    float left_leg_swing;
    float right_leg_swing;
    float collapse;
} CharacterRagdollPose;

void DrawCulturenikCharacter(const CharacterDesign *design, Vector3 position, float base_scale);
void DrawCulturenikCharacterPose(const CharacterDesign *design, const CharacterRagdollPose *pose, Vector3 position, float base_scale);

#endif
