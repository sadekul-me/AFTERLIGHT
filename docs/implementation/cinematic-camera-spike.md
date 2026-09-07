# Cinematic Camera Spike

This document describes the AFTERLIGHT camera language added on top of the foundation ViewTarget architecture. It does **not** replace `UAfterlightCameraSubsystem` with an AI director.

## Implemented

### Authority model

| Authority | Meaning |
|---|---|
| Gameplay | Player pawn is ViewTarget (Explore recipe on spring-arm camera) |
| Register | A registered Cine Camera / shot is ViewTarget via `SetViewTargetWithBlend` |
| Sequencer | `UAfterlightCinematicCoordinator` owns input + camera; Level Sequence may apply Camera Cuts |

`FAfterlightCinematicSession` stores: active flag, sequence name, return register, authority. Cancel/complete are idempotent.

### Registers

| Register | Intent |
|---|---|
| Explore | Playable third-person, cinematic FOV, collision, lag |
| Dialogue | Medium OTS, 50mm, target focus |
| Intimate | Tight close-up, 75mm, slower blend, DOF |
| Reveal | Composed insert + optional push-in |
| Threat | Tighter / faster blend |
| Cinematic | Sequencer-owned |

Unknown register names fall back to **Explore**.

### Input states

| State | Move | Look | Interact | Used for |
|---|---|---|---|---|
| Full | yes | yes | yes | Explore |
| Constrained | no | yes | no | Dialogue choices |
| Scripted | no | no | no | Intimate hold after a line |
| Locked | no | no | no | Level Sequence authority |

Input is never owned only by `DisableInput`. The controller applies these states to the pawn.

### Recipes (`UAfterlightCameraRecipe`)

Optional fields: focal length, aperture, gameplay FOV, focus mode, manual focus distance, DOF flag, shoulder side, height, arm length, blend time/function, movement/rotation lag, push-in distance/time.

Default recipes are constructed in C++ (`UAfterlightCameraRecipe::CreateDefault`). Content DataAssets can replace them later via `SetRecipe`.

Tier A (this laptop): `Afterlight.Camera.AllowDOF 1` by default, modest aperture (2.0–3.5), no hardware RT, no focus hunting (`bSmoothFocusChanges` off). Set the cvar to `0` to disable DOF.

Tier B later: same recipe fields, stronger aperture / cine-camera quality, without rewriting the camera subsystem.

### Shots in `L_Dev_CinematicLab`

Runtime Cine Camera Actors (not a huge library):

- `Dialogue.OTS.Companion`
- `Dialogue.OTS.Protagonist`
- `Dialogue.TwoShot`
- `Dialogue.CloseUp.Companion`
- `Reveal.Insert`
- `Threat.Pressure`

If a shot is visibility-blocked, fallback is TwoShot then Explore.

### Framing targets

`UAfterlightFramingTargetsComponent` exposes Head / Chest / DialogueLook / CinematicFocus as **scene offsets**, not skeletal bones, so MetaHuman can replace meshes later without renaming sockets in camera code.

### Dialogue transition

Talk → `Dialogue.OTS.Companion` (0.85s cubic) + input **Constrained** → choice → Intimate close-up + input **Scripted** (0.7s hold) → blend 0.9s back to Explore + **Full**.

No black screen, no UnPossess, no `DisableInput`.

### Level Sequence proof

Inspect cube → Reveal shot → `UAfterlightLevelSequenceFactory` builds `LS_LabInspectReveal` (~3s Camera Cut possessable) → coordinator Locked → OnFinished → Explore blend 0.85s → Full.

### HUD-free / Movie Test

`H` hides debug and prompts. Dialogue UI still shows when a line is active. Active Level Sequence also hides debug/prompts even if debug overlay was on.

### Debug

| Control | Action |
|---|---|
| F7 | Force Explore |
| F9 | Force Dialogue OTS |
| F10 / `Afterlight.Camera.PlayReveal` | Play inspect sequence |
| `Afterlight.Camera.Explore` | Force Explore |
| `Afterlight.Camera.Dialogue` | Force Dialogue OTS |
| `Afterlight.Camera.AllowDOF` | 0/1 |
| `Afterlight.Camera.Cycle` | Cycle authored shots |

Overlay: register, recipe, shot, ViewTarget, authority, sequence, input, cine mode.

## Known limitations (implemented, not blockers)

- Recipes and `LS_LabInspectReveal` are constructed in C++ at runtime, not Content `.uasset` files.
- Explore collision is spring-arm probe only; dialogue fallback is a visibility trace to TwoShot/Explore, not an auto-director.
- Placeholder capsules have no authored animation; framing uses scene targets, not bones.
- `Afterlight.SmokeLab` in PIE can teleport the pawn and log a CharacterMovement max-iteration warning; that is test teleport, not gameplay.
- UnrealEditor-Cmd may access-violate after `Log file closed` on quit; gameplay and PIE smoke complete before that.

## Planned

- Authored `.uasset` recipes and Level Sequences instead of runtime-constructed ones
- MetaHuman look-at / eye sockets mapped onto the same framing component
- Tier B stronger DOF / anamorphic without API changes
- Sequencer-authored push instead of timer lerp

## Deferred

Automatic AI director, final post, MRQ trailer, combat cameras, World Partition.

## Manual camera-quality smoke

1. PIE `L_Dev_CinematicLab` in Explore; walk the corridor (camera should not clip walls).
2. Talk to companion; confirm smooth OTS, no snap.
3. Choose 1 or 2; close-up hold; companion follow distance changes; blend back to Explore.
4. Inspect cube; ~3s Reveal sequence; control returns; register is not stuck on Cinematic.
5. Toggle `H` through the flow; no debug text in Movie Test mode.
6. Stop PIE; no fatal log spam.
