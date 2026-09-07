# Vertical Slice Production Pass 1 — Greybox Implementation

This is an implementation record. Story canon remains:

- `docs/narrative/vertical-slice-story-bible.md`
- `docs/narrative/vertical-slice-beats.md`
- `docs/narrative/vertical-slice-screenplay.md`
- `docs/narrative/vertical-slice-state-graph.md`
- `docs/production/vertical-slice-production-plan.md`
- `docs/production/youtube-movie-test.md`

No story rewrite was required.

## Map

**Path:** `/Game/Environments/Slice01/L_Slice01_Greybox`  
**File:** `Content/Environments/Slice01/L_Slice01_Greybox.umap`

Single persistent level. No World Partition. No loading screen.

Authored map contents: directional light (warm, modest), sky light, sky atmosphere, PlayerStart at Underdeck wake, `AAfterlightSlice01Director`.

Runtime greybox (director):

| Space | Approx X | Purpose |
|---|---|---|
| Underdeck 9 | 0–1600 | Wake, Witness post, Maya contact, choice |
| Lantern Cut | 1600–3400 | Walk-and-talk, AFTERLIGHT 02:17 notice, hide recess, lantern-drone sweep, hatch |
| Pump House 12 | 3400–4220 | Quiet mug, tin, warning playback, Witness LED, title |

`L_Dev_CinematicLab` is unchanged and remains the systems gym.

Default / editor startup map is now the slice. Open the lab map explicitly to test camera/lab smoke.

## Beat mapping

Progression lives in `UAfterlightNarrativeSubsystem` flags and beat ids, not Level Blueprint booleans.

| Beat id | Flag | Trigger |
|---|---|---|
| `Slice01.Wake` | `Story.Slice01.Woke` | BeginPlay / short Constrained wake |
| `Slice01.Contact` | `Story.Slice01.MetMaya` | Talk or auto-talk in range |
| `Slice01.Choice` | `ChoseFollow` / `ChoseQuestion` | Dialogue choice node |
| `Slice01.LanternCut` | `Story.Slice01.EnteredCut` | Walk-and-talk + X > 1680 |
| `Slice01.Sweep` | `Story.Slice01.SweepPassed` | Threat sweep + hide / fail-soft |
| `Slice01.Hatch` | `Story.Slice01.ReachedBolt` | Hatch Use after sweep |
| `Slice01.Quiet` | `Story.Slice01.QuietBeat` | Pump House mug dialogue |
| `Slice01.Tin` | `Story.Slice01.FoundTin` | Tin inspect (once) |
| `Slice01.Warning` | `Story.Slice01.HeardWarning` | `LS_Slice01_Warning` complete |
| `Slice01.Title` | `Story.Slice01.Complete` | Witness LED + AFTERLIGHT card |

## Choice

Start: Trust 0.50, Suspicion 0.00.

| Option | Flags | Deltas | Result | Presentation |
|---|---|---|---|---|
| **Walk.** | `ChoseFollow` | Trust +0.25 | Trust 0.75 | Close follow |
| **You talk like I belong to you.** | `ChoseQuestion` | Suspicion +0.65 | Suspicion 0.65 | Far follow |

No numeric meter. Pass 1.5 removed `[1]` / `[2]` numbering; stacked choice text remains. See `docs/implementation/vertical-slice-pass1-5.md`.

## Branch texture (Pass 1)

Both paths use the same rooms and reveal. Visible differences:

1. Maya follow distance (140 vs 280).
2. Follow: extra “Eli.” after the choice. Question: “I don't. That's the problem.”
3. Walk-and-talk path offset and Pump House stand-off (closer vs farther).
4. Sweep line: Follow “Four. They turn at four.” / Question “Down.”
5. Quiet: Follow finishes the dishwasher/pump joke. Question kills it (“Drink it before it tastes like the pipe.”).
6. Tin: Follow “Don’t—” / Question “Eli—”
7. Threat vs Intimate composition already differs by register; Follow uses closer Maya placement at the mug.

No final animation. Sleeve vs palm is dialogue/position proxy only.

## Walk-and-talk

Existing `UAfterlightDialogueRunner` gained `NextNodeId`, `AutoAdvanceSeconds`, and `bKeepGameplayInput`. Linear nodes stay active until `Advance()`. Cut dialogue sets `bKeepGameplayInput` so movement/look stay Full. Maya uses an authored waypoint path and waits if the player lags (`follow distance + 160` as of Pass 1.5).

If the player stops, lines still advance. If they never enter the Cut, a 14s fail-soft starts the sweep anyway.

## Threat / hide / fail-soft

`AAfterlightLanternDrone`: sphere + downward spotlight + `HELION` label. Authored lerp, no perception, no combat, no death screen.

Hide recess is a physical alcove (Y > 240 around X 2420). Threat register + Constrained look during the pass.

Fail-soft (Pass 1.5):

- Slightly late: scripted ease into the recess (smoke still teleports).
- Ignore the beat / walk to the hatch: grant `SweepPassed` near X 3280 so the slice cannot soft-lock.
- Compromise: late hide is a camera-assisted pull, not a visible pop.

## Quiet beat

Intimate close-up. Mug is a greybox block. Wrong-hand event is dialogue + Maya offset (closer if Follow). Essential lines unchanged:

- “Don't know which one is mine.”
- “Right. Sorry.”

## Warning

`UAfterlightLevelSequenceFactory::CreateWarningSequence` builds runtime `LS_Slice01_Warning` (~22s Camera Cut on the Reveal insert camera as of Pass 1.5). Coordinator: Gameplay → Cinematic/Locked → play → release → Intimate after-lines.

Approved recording text plays as speaker `Recording` (subtitles, spoken hold). No AI voice. No downloaded VO. Pass 1.5 adds procedural temp beds (rain, electric, drone, pump, warning crackle, sting).

## Ending

Maya: “He sounds so sure.” / “Was he?” / “No.”  
Witness LED intensity comes up. Reveal register. Short silence → sting → black scrim → **AFTERLIGHT** (no tagline).

## Controls

| Input | Action |
|---|---|
| WASD / look | Move / look when Full |
| E | Talk / Hatch / Tin when allowed (hidden in cine) |
| 1 / 2 | Choice |
| H | Toggle cine / HUD-free (slice **starts** in cine) |
| F8 | Debug overlay (suppressed while cine or sequence active) |
| F5 / F6 | Developer save / load |
| `Afterlight.Slice01.Jump Wake\|Choice\|Sweep\|Quiet\|Warning` | Debug checkpoints |
| `Afterlight.ResetSlice` | Restart slice |
| `Afterlight.SmokeSlice01` | Accelerated state smoke |
| `-AfterlightSliceSmoke` | Standalone smoke then exit 0/1 |

Lab commands (`Afterlight.ResetLab`, `-AfterlightSmoke`) still apply on `L_Dev_CinematicLab`.

## Temporary presentation

- Capsule Maya / protagonist, engine cubes, text signs.
- Zone lights only (cool Underdeck, warmer Cut, warm Pump House). No Lumen quality target.
- Pass 1.5: procedural temp beds replace clean silence. See `vertical-slice-pass1-5.md`.
- Warning is a cracked-slate silhouette + flicker + camera push, not a rendered movie.

## Runtime

Authored holds (wake, lines, sweep, warning, title) are about **5–7 minutes** if the player never lingers (Pass 1.5 lengthened spoken holds). A normal viewing run that waits for Maya, reads the Cut notices, and sits in the quiet beat is estimated **7–9 minutes**. Walking was not padded; Maya is slower (165) than the player (280), so wait-for-player is the extra texture.

Command-line smoke is accelerated and is not a runtime measurement.

On title, the log prints `AFTERLIGHT_SLICE_RUNTIME=<seconds>` from world time.

## Movie-test findings (Pass 1)

Evaluated from HUD-free default cine, NullRHI smoke, and directed layout — not a recorded 8-minute GPU take on this machine (commit charge was already near the pagefile ceiling).

Works:

- No debug overlay in cine; prompts suppressed; choices and subtitles remain.
- Title is a black card, not text over gameplay.
- Camera uses existing register blends (no UnPossess).
- Branch texture is visible without meters.
- Threat is a polite spotlight, not combat.
- Slice completes without developer intervention (smoke path).

Still game-like after Pass 1.5: capsule Maya, greybox rooms, E on hatch/tin, no human VO. Details in `vertical-slice-pass1-5.md`.

## Tests

Automation group `Afterlight` (existing +):

- `Afterlight.Slice01.ChoiceOutcomes`
- `Afterlight.Slice01.FlagProgression`
- `Afterlight.Narrative.DialogueLinearAdvance`
- `Afterlight.Slice01.WarningSequenceName`
- Pass 1.5: `Afterlight.Slice01.HideRecoveryLerp`, `Afterlight.UI.ChoicePresentation`, `Afterlight.UI.SpokenHold`

Smoke:

```
UnrealEditor-Cmd.exe AFTERLIGHT.uproject /Game/Environments/Slice01/L_Slice01_Greybox -game -unattended -nop4 -NullRHI -AfterlightSliceSmoke
```

Lab gym still:

```
UnrealEditor-Cmd.exe AFTERLIGHT.uproject /Game/Environments/Slice01/L_Dev_CinematicLab -game -unattended -nop4 -NullRHI -AfterlightSmoke
```

Editor PIE helper: `Tools/EditorPieSliceSmoke.py`.

## Performance (Tier A)

- HW RT off, VSM off, point lights do not cast shadows.
- Runtime greybox + one drone spotlight.
- Warning sequence is a short possessable Camera Cut.
- Laptop compile needed `-NoPCH` when commit charge was ~45/46 GB; that is a machine constraint, not a shipping setting.

## Known limitations / technical debt

- `LS_Slice01_Warning` is created at runtime (same pattern as lab inspect), not a Content `.uasset`.
- No MetaHuman / final Maya mesh / human VO.
- Save/load restores flags/relationship/beat, not actor transforms; use debug jumps after load if the pawn is in the wrong room.
- GameMode still auto-spawns `AAfterlightLabDirector` on maps that have neither director.

Presentation polish from this baseline is recorded in `docs/implementation/vertical-slice-pass1-5.md`.

## Next recommended milestone

**Character Animation & Performance Prototype** — do not start until Pass 1.5 is signed off.
