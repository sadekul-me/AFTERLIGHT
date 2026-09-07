# AFTERLIGHT

Premium cinematic narrative sci-fi/cyberpunk game.

**Playable Cinema** — gameplay that can be recorded HUD-free and still read as a directed sci-fi film.

## Engine

- Unreal Engine **5.8.2**
- Target: Windows PC first (Win64, DX12, single-player vertical slice)
- Project type: Blank C++ (`AFTERLIGHT.uproject`, module **Afterlight**)

## First milestone

A highly polished **5–10 minute Cinematic Vertical Slice**.

This repository currently contains the **engineering foundation**: movement, interaction, companion, dialogue/relationship verbs, thin cinematic camera, HUD-free mode, and `L_Dev_CinematicLab`.

See:

- `docs/architecture/foundation.md`
- `docs/implementation/vertical-slice-foundation.md`

## Build / launch

```
"D:\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" AFTERLIGHTEditor Win64 Development -Project="D:\Programming\AFTERLIGHT\AFTERLIGHT.uproject"
```

Open `AFTERLIGHT.uproject` in Unreal Editor 5.8.2 (startup map: `L_Dev_CinematicLab`).

## Source control

- Remote pushes are performed **manually by the project owner**.
- Do not commit generated Unreal directories (`Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, and similar local output).
- Binary assets use Git LFS.
