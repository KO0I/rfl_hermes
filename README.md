# RFL Hermes

A low-poly space and surface racing prototype built with C and raylib.

## Implemented vertical slice

- Space flight around a textured low-poly planet generated from `planet_tool` output (`earthlike.bmp`).
- First- and third-person camera toggle with `V`.
- Relativistic-speed visual treatment: FOV expands and a blue overlay strengthens as velocity approaches the local lightspeed reference.
- Atmospheric entry treatment: near-planet speed produces an orange glow around the player.
- Generated low-poly cloud bands around the planet.
- Single-object debug targets plus constrained explore-mode reseeding with `F5`.
- Simple multiplayer stand-ins: three remote racers orbit the planet.
- Target-relative controls:
  - `[` and `]` cycle targets.
  - `/` targets the nearest object or player.
  - `c` approaches the current target.
  - `spacebar` moves away from the current target.
- Characters are Kentucky Route Zero-inspired culturenik figures in space, with ragdoll motion, glow, and light trails.
- Procedural culturenik character creator with GameDesign-inspired archetypes for racer figures.
- `F6` opens a character editor preview; `Tab` selects a field, left/right arrows edit it, and `R` resets the current archetype.

## Controls

- `W/A/S/D`: fly relative to camera direction
- `Left Shift` / `Left Ctrl`: vertical thrust
- Mouse: look
- `V`: first/third person
- `F5`: reseed explore-mode objects
- `F6`: character editor preview
- `Tab` / `Shift+Tab`: select editor field while preview is active
- `Left` / `Right`: edit selected character field while preview is active
- `R`: reset the active archetype while preview is active
- `[` / `]`: cycle target
- `/`: nearest target
- `c`: approach target
- `spacebar`: move away from target
- `Esc`: quit

## Build and run

```sh
make
make run
```

The project expects raylib to be available through `pkg-config raylib`.

## Asset references

`planet_tool --export-game-assets` was run against the prior work in `~/proj/codex_godot/original_c/planet_tool`; the exported BMPs are kept as editable source-style image assets in the repository root for now.

The Kentucky Route Zero decomp character references inspected under `~/build/kr0_decomp/krz_shannon_godot_project/assets/rigged_npcs/` are summarized in `docs/art_reference.md`. Shannon and Conway palettes/manifests are copied under `assets/kr0_reference/` for local reference.

The culturenik character creator is based on `assets/kr0_reference/cultureniks.png`, `hesh.png`, `sa.png`, and `sma.png` copied from `~/Documents/ChipChirp/Hermes-Agent/GameDesign`, plus KR0 rigged NPC palettes/manifests.

There is no separate surface-mode character presentation now: the culturenik figures are the normal racer bodies and ragdoll directly in space.
