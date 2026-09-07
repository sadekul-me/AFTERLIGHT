# AFTERLIGHT Foundation Architecture

## Vision

AFTERLIGHT is a premium cinematic narrative sci-fi/cyberpunk game.

**Playable Cinema:** recorded gameplay with minimal HUD should read as directed science fiction, not as a game interrupted by cutscenes.

## Engine

- Unreal Engine **5.8.2** (`D:\Epic Games\UE_5.8`)
- Module: **Afterlight** (single runtime module)
- Target: Windows Win64, DX12, Desktop/Console
- Compiler pinned: **MSVC 14.44.35207**

## C++ / Blueprint / Data

| Layer | Owns |
|---|---|
| C++ | Rules, state, save, narrative verbs, relationship math, camera policy, interaction, debug |
| Data assets | Beats, dialogue graphs, camera recipes (placeholder graphs currently constructed in C++) |
| Blueprint / UMG | Presentation widgets (C++ `UAfterlightHUDWidget` until authored WBPs exist) |
| Sequencer | Future cinematic authority; coordinator API is ready |

**Content knows plot. Code knows verbs.**

## Implemented systems

- GameMode / GameInstance / PlayerController / Protagonist character
- `UAfterlightPlayerContextSubsystem` — resolve protagonist without scattering `GetFirstPlayerController()`
- Enhanced Input (runtime IMC: Move, Look, Interact, Cine toggle, debug, choices, save/load)
- Interaction interface + component (Talk / Inspect / Use tags; forward overlap probe on Pawn/WorldStatic/WorldDynamic)
- Generic companion character (look-at, follow-distance presentation hooks; Trust high → close, Suspicion high → far)
- Narrative flags + beat evaluation + dialogue runner
- Relationship axes: **Trust**, **Suspicion** (hidden; debug overlay only)
- Thin camera registers: Explore / Dialogue / Cinematic / Threat
- Cinematic coordinator: request control → lock input → change register → release
- HUD-free cine mode (`H`) and debug overlay (`F8`)
- Developer save/load (`F5` / `F6`) for flags + relationship + beat id
- Runtime greybox lab via `AAfterlightLabDirector`

## Intentionally not implemented

MetaHuman, final characters, screenplay, combat, inventory, GAS/Lyra, World Partition, multiplayer, Wwise/FMOD, city packs, full five-axis relationship model, final MRQ.

## Camera

`UAfterlightCameraSubsystem` blends Player ViewTarget to registered cine cameras. Recipes store focal length, aperture, blend time, framing bias. No automatic director AI.

## Interaction

`IAfterlightInteractable` + `UAfterlightInteractableComponent`. Detection is a forward sweep from the protagonist.

## Narrative

Flags are Gameplay Tags. Beats declare required/granted flags, optional dialogue, camera register, sequence, next beat. Dialogue runner is data → events → UI, not UMG-owned logic.

## Relationship

State lives in `UAfterlightRelationshipSubsystem`. Deltas clamp to `[0,1]`. Thresholds emit presentation tags (`Relationship.Trust.High`, `Companion.Follow.Close`, etc.).
