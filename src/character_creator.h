#ifndef RFL_CHARACTER_CREATOR_H
#define RFL_CHARACTER_CREATOR_H

#include "raylib.h"
#include <stdbool.h>

#define RFL_CHARACTER_NAME_LENGTH 32
#define RFL_CHARACTER_PALETTE_SIZE 6

typedef enum CharacterArchetype {
    CHARACTER_ARCHETYPE_PUNK_RACER = 0,
    CHARACTER_ARCHETYPE_FESTIVAL_RAVER,
    CHARACTER_ARCHETYPE_ROBED_DRIFTER,
    CHARACTER_ARCHETYPE_LONG_COAT_RIVAL,
    CHARACTER_ARCHETYPE_CRYSTAL_VISITOR,
    CHARACTER_ARCHETYPE_TATTOOED_MYSTIC,
    CHARACTER_ARCHETYPE_COUNT
} CharacterArchetype;

typedef enum CharacterAccessory {
    CHARACTER_ACCESSORY_NONE = 0,
    CHARACTER_ACCESSORY_GOGGLES,
    CHARACTER_ACCESSORY_FEATHERS,
    CHARACTER_ACCESSORY_LONG_COAT,
    CHARACTER_ACCESSORY_LONG_PROP,
    CHARACTER_ACCESSORY_BODY_GLOW_LINES,
    CHARACTER_ACCESSORY_TATTOO_LINES,
    CHARACTER_ACCESSORY_BELT_POUCH,
    CHARACTER_ACCESSORY_FLOATING_DISC
} CharacterAccessory;

typedef struct CharacterDesign {
    char name[RFL_CHARACTER_NAME_LENGTH];
    CharacterArchetype archetype;
    Color palette[RFL_CHARACTER_PALETTE_SIZE];
    Color glow_color;
    float height_scale;
    float torso_width;
    float leg_length;
    float head_scale;
    float shoulder_slope;
    float asymmetry;
    CharacterAccessory primary_accessory;
    CharacterAccessory secondary_accessory;
    CharacterAccessory tertiary_accessory;
    bool uses_neon_accents;
    bool floats_above_ground;
} CharacterDesign;

CharacterDesign CharacterDesignCreate(unsigned int seed, CharacterArchetype archetype);
CharacterDesign CharacterDesignCreateForActor(unsigned int actor_index, bool is_player);
const char *CharacterArchetypeName(CharacterArchetype archetype);
const char *CharacterAccessoryName(CharacterAccessory accessory);

#endif
