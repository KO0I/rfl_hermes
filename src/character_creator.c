#include "character_creator.h"

#include <stdio.h>

static const float CULTURENIK_BASE_HEIGHT = 1.0f;
static const float CULTURENIK_TALL_HEIGHT = 1.18f;
static const float CULTURENIK_SLIM_TORSO = 0.42f;
static const float CULTURENIK_WIDE_COAT_TORSO = 0.62f;
static const float CULTURENIK_LONG_LEGS = 1.25f;
static const float CULTURENIK_LARGE_HEAD = 1.08f;
static const float CULTURENIK_SHARP_SHOULDER = 0.28f;
static const float CULTURENIK_HIGH_ASYMMETRY = 0.65f;
static const float CULTURENIK_LOW_ASYMMETRY = 0.18f;

static Color ColorFromBytes(unsigned char r, unsigned char g, unsigned char b) {
    return (Color){ r, g, b, 255 };
}

static unsigned int HashSeed(unsigned int seed) {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}

static float SeedRange(unsigned int seed, float minimum, float maximum) {
    const unsigned int range_mask = 0xffffu;
    float t = (float)(HashSeed(seed) & range_mask) / (float)range_mask;
    return minimum + (maximum - minimum) * t;
}

static void SetPalette(CharacterDesign *design, Color a, Color b, Color c, Color d, Color e, Color f) {
    design->palette[0] = a;
    design->palette[1] = b;
    design->palette[2] = c;
    design->palette[3] = d;
    design->palette[4] = e;
    design->palette[5] = f;
}

const char *CharacterArchetypeName(CharacterArchetype archetype) {
    switch (archetype) {
        case CHARACTER_ARCHETYPE_PUNK_RACER: return "Hesh-like punk racer";
        case CHARACTER_ARCHETYPE_FESTIVAL_RAVER: return "Sa-like festival raver";
        case CHARACTER_ARCHETYPE_ROBED_DRIFTER: return "Sma-like robed drifter";
        case CHARACTER_ARCHETYPE_LONG_COAT_RIVAL: return "Long-coat rival";
        case CHARACTER_ARCHETYPE_CRYSTAL_VISITOR: return "Crystal visitor";
        case CHARACTER_ARCHETYPE_TATTOOED_MYSTIC: return "Tattooed mystic";
        case CHARACTER_ARCHETYPE_COUNT: break;
    }
    return "Unknown";
}

const char *CharacterAccessoryName(CharacterAccessory accessory) {
    switch (accessory) {
        case CHARACTER_ACCESSORY_NONE: return "none";
        case CHARACTER_ACCESSORY_GOGGLES: return "goggles";
        case CHARACTER_ACCESSORY_FEATHERS: return "feathers";
        case CHARACTER_ACCESSORY_LONG_COAT: return "long coat";
        case CHARACTER_ACCESSORY_LONG_PROP: return "long prop";
        case CHARACTER_ACCESSORY_BODY_GLOW_LINES: return "glow lines";
        case CHARACTER_ACCESSORY_TATTOO_LINES: return "tattoo lines";
        case CHARACTER_ACCESSORY_BELT_POUCH: return "belt pouch";
        case CHARACTER_ACCESSORY_FLOATING_DISC: return "floating disc";
    }
    return "unknown";
}

CharacterDesign CharacterDesignCreate(unsigned int seed, CharacterArchetype archetype) {
    CharacterDesign design = { 0 };
    design.archetype = archetype;
    snprintf(design.name, RFL_CHARACTER_NAME_LENGTH, "%s", CharacterArchetypeName(archetype));
    design.height_scale = SeedRange(seed + 1u, CULTURENIK_BASE_HEIGHT, CULTURENIK_TALL_HEIGHT);
    design.torso_width = CULTURENIK_SLIM_TORSO;
    design.leg_length = CULTURENIK_LONG_LEGS;
    design.head_scale = CULTURENIK_LARGE_HEAD;
    design.shoulder_slope = CULTURENIK_SHARP_SHOULDER;
    design.asymmetry = CULTURENIK_LOW_ASYMMETRY;
    design.primary_accessory = CHARACTER_ACCESSORY_NONE;
    design.secondary_accessory = CHARACTER_ACCESSORY_NONE;
    design.tertiary_accessory = CHARACTER_ACCESSORY_NONE;
    design.uses_neon_accents = false;
    design.floats_above_ground = false;

    switch (archetype) {
        case CHARACTER_ARCHETYPE_PUNK_RACER:
            SetPalette(&design,
                ColorFromBytes(232, 155, 83), ColorFromBytes(115, 28, 86),
                ColorFromBytes(38, 92, 98), ColorFromBytes(214, 72, 42),
                ColorFromBytes(48, 42, 38), ColorFromBytes(192, 164, 92));
            design.glow_color = ColorFromBytes(255, 82, 145);
            design.primary_accessory = CHARACTER_ACCESSORY_LONG_PROP;
            design.secondary_accessory = CHARACTER_ACCESSORY_GOGGLES;
            design.tertiary_accessory = CHARACTER_ACCESSORY_BELT_POUCH;
            design.uses_neon_accents = true;
            design.asymmetry = CULTURENIK_HIGH_ASYMMETRY;
            break;
        case CHARACTER_ARCHETYPE_FESTIVAL_RAVER:
            SetPalette(&design,
                ColorFromBytes(211, 148, 88), ColorFromBytes(176, 42, 114),
                ColorFromBytes(46, 150, 88), ColorFromBytes(62, 82, 118),
                ColorFromBytes(232, 196, 98), ColorFromBytes(42, 36, 38));
            design.glow_color = ColorFromBytes(88, 255, 150);
            design.primary_accessory = CHARACTER_ACCESSORY_GOGGLES;
            design.secondary_accessory = CHARACTER_ACCESSORY_FEATHERS;
            design.tertiary_accessory = CHARACTER_ACCESSORY_BELT_POUCH;
            design.uses_neon_accents = true;
            design.asymmetry = CULTURENIK_HIGH_ASYMMETRY;
            break;
        case CHARACTER_ARCHETYPE_ROBED_DRIFTER:
            SetPalette(&design,
                ColorFromBytes(211, 143, 32), ColorFromBytes(29, 33, 37),
                ColorFromBytes(201, 128, 74), ColorFromBytes(72, 54, 42),
                ColorFromBytes(238, 186, 64), ColorFromBytes(18, 18, 22));
            design.glow_color = ColorFromBytes(255, 174, 64);
            design.primary_accessory = CHARACTER_ACCESSORY_LONG_COAT;
            design.secondary_accessory = CHARACTER_ACCESSORY_FLOATING_DISC;
            design.torso_width = CULTURENIK_WIDE_COAT_TORSO;
            break;
        case CHARACTER_ARCHETYPE_LONG_COAT_RIVAL:
            SetPalette(&design,
                ColorFromBytes(20, 22, 25), ColorFromBytes(45, 45, 48),
                ColorFromBytes(86, 74, 61), ColorFromBytes(32, 68, 72),
                ColorFromBytes(122, 92, 64), ColorFromBytes(10, 10, 12));
            design.glow_color = ColorFromBytes(80, 170, 190);
            design.primary_accessory = CHARACTER_ACCESSORY_LONG_COAT;
            design.secondary_accessory = CHARACTER_ACCESSORY_LONG_PROP;
            design.torso_width = CULTURENIK_WIDE_COAT_TORSO;
            break;
        case CHARACTER_ARCHETYPE_CRYSTAL_VISITOR:
            SetPalette(&design,
                ColorFromBytes(92, 54, 182), ColorFromBytes(128, 78, 234),
                ColorFromBytes(58, 190, 255), ColorFromBytes(31, 24, 75),
                ColorFromBytes(220, 188, 255), ColorFromBytes(255, 218, 64));
            design.glow_color = ColorFromBytes(94, 205, 255);
            design.primary_accessory = CHARACTER_ACCESSORY_BODY_GLOW_LINES;
            design.uses_neon_accents = true;
            design.floats_above_ground = true;
            break;
        case CHARACTER_ARCHETYPE_TATTOOED_MYSTIC:
            SetPalette(&design,
                ColorFromBytes(92, 92, 88), ColorFromBytes(38, 42, 44),
                ColorFromBytes(143, 143, 132), ColorFromBytes(18, 22, 24),
                ColorFromBytes(70, 74, 72), ColorFromBytes(190, 176, 145));
            design.glow_color = ColorFromBytes(236, 204, 88);
            design.primary_accessory = CHARACTER_ACCESSORY_TATTOO_LINES;
            design.secondary_accessory = CHARACTER_ACCESSORY_LONG_COAT;
            break;
        case CHARACTER_ARCHETYPE_COUNT:
            break;
    }

    return design;
}

CharacterDesign CharacterDesignCreateForActor(unsigned int actor_index, bool is_player) {
    CharacterArchetype archetype = (CharacterArchetype)(actor_index % (unsigned int)CHARACTER_ARCHETYPE_COUNT);
    if (is_player) {
        archetype = CHARACTER_ARCHETYPE_FESTIVAL_RAVER;
    }
    return CharacterDesignCreate(1009u + actor_index * 97u, archetype);
}
