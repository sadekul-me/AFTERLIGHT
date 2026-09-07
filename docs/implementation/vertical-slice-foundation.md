# Vertical Slice Foundation — Implementation Notes

## How to build

```
"D:\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" AFTERLIGHTEditor Win64 Development -Project="D:\Programming\AFTERLIGHT\AFTERLIGHT.uproject"
```

MSVC 14.44 is pinned in `Source/AFTERLIGHT.Target.cs` and `DefaultEngine.ini`.

## How to launch

Open `D:\Programming\AFTERLIGHT\AFTERLIGHT.uproject` (starts on `L_Dev_CinematicLab`).

Default map is `Content/Environments/Slice01/L_Dev_CinematicLab` (no World Partition). The map holds one directional light, one sky light, a PlayerStart, and `AAfterlightLabDirector`. The director still builds greybox geometry, companion, inspectable, and cine cameras at runtime.

Unattended systems smoke (standalone, then exit):

```
"D:\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Programming\AFTERLIGHT\AFTERLIGHT.uproject" /Game/Environments/Slice01/L_Dev_CinematicLab -game -unattended -nop4 -NullRHI -AfterlightSmoke
```

## Playable technical flow (~30–90s)

1. Spawn in the greybox corridor.
2. Walk toward the companion (`WASD`, mouse look).
3. When close/facing, prompt **Talk** (`E`).
4. Camera blends Explore → Dialogue.
5. Companion: “Can you hear me?”
6. Choose **1** “I'm fine.” (Trust +0.25 → close follow) or **2** “Who are you?” (Suspicion +0.65 → far follow).
7. Companion follow distance changes (close vs far).
8. Camera returns to Explore; input restored.
9. Inspect the world object (`E`).
10. Story flag `Story.Test.InspectedObject` is granted; cinematic register is requested then released without a level reload.

## Debug controls

| Key | Action |
|---|---|
| H | Toggle cine / HUD-free mode |
| F8 | Toggle debug overlay |
| F5 | Developer save |
| F6 | Developer load |
| 1 / 2 | Dialogue choices |
| `Afterlight.ResetLab` | Reset technical lab flags/relationship |
| `Afterlight.SmokeLab` | Drive talk/choice/inspect/save/load without clicking |
| `-AfterlightSmoke` | Standalone command-line: run smoke then exit with 0/1 |

Debug overlay shows flags, beat, Trust/Suspicion, camera register, input state, cinematic active, follow distance.

## Tests

Automation group `Afterlight`:

- Relationship apply/clamp/thresholds/defaults
- Beat requirement evaluation
- Dialogue choice transition
- Camera register names
- Save-state struct round trip

```
UnrealEditor-Cmd.exe AFTERLIGHT.uproject -unattended -nop4 -NullRHI -ExecCmds="Automation RunTests Afterlight; Quit"
```

## Limitations

- Placeholder capsule characters, engine cubes, no authored animation.
- Runtime IMC instead of Content Input assets (same Enhanced Input API; assets can replace later).
- Inspect cinematic is a timed register hold, not a final Level Sequence.
- Editor PIE can be driven with `Tools/EditorPieSmoke.py`; standalone `-game` and `-AfterlightSmoke` remain the unattended proof path.
- Interaction traces use a forward overlap probe on Pawn, WorldStatic, and WorldDynamic so capsules and greybox props are detectable.
- No player-facing save UI.
- Tier A laptop: hardware RT off, VSM off. Not a final picture-quality target.
