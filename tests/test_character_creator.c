#include "character_creator.h"

#include <assert.h>
#include <string.h>

int main(void) {
    CharacterDesign first = CharacterDesignCreate(42u, CHARACTER_ARCHETYPE_FESTIVAL_RAVER);
    CharacterDesign second = CharacterDesignCreate(42u, CHARACTER_ARCHETYPE_FESTIVAL_RAVER);
    CharacterDesign crystal = CharacterDesignCreate(99u, CHARACTER_ARCHETYPE_CRYSTAL_VISITOR);
    CharacterDesign punk = CharacterDesignCreate(7u, CHARACTER_ARCHETYPE_PUNK_RACER);
    CharacterDesign robed = CharacterDesignCreate(8u, CHARACTER_ARCHETYPE_ROBED_DRIFTER);

    assert(first.archetype == CHARACTER_ARCHETYPE_FESTIVAL_RAVER);
    assert(strcmp(first.name, "Sa-like festival raver") == 0);
    assert(first.primary_accessory == CHARACTER_ACCESSORY_GOGGLES);
    assert(first.secondary_accessory == CHARACTER_ACCESSORY_FEATHERS);
    assert(first.tertiary_accessory == CHARACTER_ACCESSORY_BELT_POUCH);
    assert(first.palette[0].r == second.palette[0].r);
    assert(first.palette[0].g == second.palette[0].g);
    assert(first.palette[0].b == second.palette[0].b);
    assert(crystal.floats_above_ground);
    assert(crystal.uses_neon_accents);
    assert(punk.primary_accessory == CHARACTER_ACCESSORY_LONG_PROP);
    assert(robed.secondary_accessory == CHARACTER_ACCESSORY_FLOATING_DISC);
    assert(strcmp(CharacterAccessoryName(CHARACTER_ACCESSORY_FLOATING_DISC), "floating disc") == 0);

    return 0;
}
