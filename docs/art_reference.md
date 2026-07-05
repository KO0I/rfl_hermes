# Art Reference

## Kentucky Route Zero character assets inspected

Source directory:

`~/build/kr0_decomp/krz_shannon_godot_project/assets/rigged_npcs/`

The exported rigged NPC set is appropriate for surface-mode reference because the characters are low-poly, palette-textured, and caricatured rather than realistic.

Observed from `manifest.json`:

- Character meshes generally sit in the 300-750 vertex range and 300-950 triangle range.
- The source uses `color_mode: palette-texture`.
- Characters carry small palettes, usually around 11-29 unique colors.
- Rigged characters commonly have 25-32 joints and a small number of named animation clips.

Specific inspected references:

| Character | Mesh | Vertices | Triangles | Joints | Unique colors | Notes |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Shannon | `Mesh/ShannonBody.asset` | 523 | 639 | 29 | 26 | Muted greys, browns, dark outlines; missing glasses path in exported clips. |
| Conway | `Mesh/ConwayBodyWithoutHat.asset` | 417 | 498 | 26 | 27 | Muted skin/brown/blue-grey palette, compact rig, clean exported clips. |

The current prototype does not import these glTF rigs at runtime yet. Instead, it uses them as style guidance for the surface-mode stand-in figures:

- larger head and simplified block torso
- muted palette-friendly forms
- visible limb cylinders with low segment count
- glow and trailing light layered over the character silhouette

Copied local references:

- `assets/kr0_reference/Shannon_palette.png`
- `assets/kr0_reference/Conway_palette.png`
- `assets/kr0_reference/Shannon_manifest.json`
- `assets/kr0_reference/Conway_manifest.json`
- `assets/kr0_reference/cultureniks.png`
- `assets/kr0_reference/hesh.png`
- `assets/kr0_reference/sa.png`
- `assets/kr0_reference/sma.png`

## Space vs surface character rule

- Characters now use culturenik caricature figures directly in space instead of a separate surface-mode presentation.
- Ragdoll motion keeps racers readable as bodies rather than icons.
- Characters glow and leave light trails.

## Culturenik Creator Style Rules

Reference folder copied from Obsidian GameDesign (`~/Documents/ChipChirp/Hermes-Agent/GameDesign`, visible in the sandbox as `/app/GameDesign`):

- `assets/kr0_reference/cultureniks.png`
- `assets/kr0_reference/hesh.png`
- `assets/kr0_reference/sa.png`
- `assets/kr0_reference/sma.png`

The procedural creator should produce archetypes, not random noise:

1. Hesh-like punk racer: long prop, teal goggles, cap/headwrap, asymmetrical boots/straps, magenta/teal accents, belt pouch.
2. Sa-like festival raver: round goggles, feather/plume headwear, braid details, saturated magenta/green wraps, mismatched high boots.
3. Sma-like robed drifter: long yellow robe, black trim, bare feet, angular black hair, optional floating disc prop.
4. Long-coat rival: black coat, angular hair, dark muted palette, carried prop.
5. Crystal visitor: purple body, glowing eyes, cyan body light lines.
6. Tattooed mystic: grey/black palette, body pattern lines, reserved silhouette.

Renderer constraints:

- Keep meshes primitive and low-poly.
- Prefer flat-color facets over gradients.
- Use 6-12 named colors per generated character.
- Maintain readable silhouettes at racer scale.
- Generated figures are the normal space racer bodies; do not gate them behind surface-mode.
