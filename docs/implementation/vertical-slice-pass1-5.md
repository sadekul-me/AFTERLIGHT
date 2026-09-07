# Vertical Slice Pass 1.5 — Cinematic Performance Polish

Implementation record only. Story canon is unchanged:

- `docs/narrative/vertical-slice-story-bible.md`
- `docs/narrative/vertical-slice-beats.md`
- `docs/narrative/vertical-slice-screenplay.md`
- `docs/narrative/vertical-slice-state-graph.md`
- `docs/production/vertical-slice-production-plan.md`
- `docs/production/youtube-movie-test.md`

Pass 1 greybox remains the playable spine. This pass does not redesign rooms, flags, or choice outcomes.

## Intent

Make the existing 7–9 minute greybox slice feel less like a technical prototype and more like a cinematic viewing, using placeholder-safe tools only.

Not in this pass: MetaHuman, final Maya/Eli, Live Link, human VO, Wwise/FMOD, Fab/Quixel packs, World Partition, combat, stealth AI, MRQ trailer, final rain, final post.

## Maya idle / presence

`AAfterlightCompanionCharacter` stays a lightweight authored companion. No Behavior Tree.

Changes:

- Walk speed 165 (player remains 280).
- Wait-for-player threshold is follow distance + 160.
- Authored world look targets (Witness post, Cut clock, hide, hatch, mug).
- Follow branch prefers looking at the player; Question branch looks at the world more often.
- Glance holds (~1.6s) and a short waypoint pause (~0.42s) instead of constant motion.
- Facing interp 2.4 so turns read as thought, not snaps.
- Contact, cut, sweep, quiet, and warning set glance targets so Maya is participating even when the player is not talking to her.

She is still a capsule. Presence is staging and timing, not animation.

## Walk-and-talk timing

Lantern Cut lines now use spoken-hold durations (`len * 0.055 + 0.8`, clamp 2.0–6.4, authored overrides kept). Examples:

- “Don't look at the posts…” 4.2s
- “They don't stay dead because you asked.” 3.8s
- “A dry room…” 4.2s

If Maya is waiting for the player, the current line gains +0.85s so dialogue does not outrun the pair. Cut path still keeps gameplay input. Fail-soft sweep still starts if the player never enters the Cut.

## Hide recovery

Live play no longer teleports into the recess.

If the player is not in the hide volume when the sweep ends:

1. Input goes Scripted.
2. Threat pressure shot.
3. Capsule collision off for ~1.2s.
4. Smoothstep lerp into `HideLoc`, with a small vertical ease so the move is camera-assisted rather than a floor pop.
5. Collision restored. “Okay.” Maya leads to the hatch.

Command-line / `RunSliceSmoke` still teleports (accelerated, NullRHI-safe).

Walking past X 3280 still grants `SweepPassed` so the slice cannot soft-lock. No game-over. No combat.

## Threat presentation

Same cheap drone (sphere + spotlight + HELION). Sweep is ~9s with yaw/pitch scan and intensity pulse. Drone procedural hum is audible during the pass. Maya glances at the drone, then the hide. Threat register still owns the camera. Environmental fill stays the existing Cut lantern light.

## Temp audio

Engine-native `USoundWaveProcedural` beds in `FAfterlightTempAudio`. No downloads, no Wwise, no FMOD, no copyrighted music.

| Bed | Use |
|---|---|
| Rain | Underdeck / Cut exterior |
| Electric | Distant civic hum |
| Drone | Sweep only |
| Pump | Pump House; reduced during the quiet mug beat |
| Warning | Short crackle on each recording line |
| Sting | Witness LED / title |

Smoke (`-AfterlightSliceSmoke`) does not spawn loops.

## Warning presentation

Still runtime `LS_Slice01_Warning` (now ~22s), not a movie asset.

Added:

- Cracked-slate greybox + silhouette block at the tin wall
- Flickering slate light
- Slow camera push toward the slate
- Maya in frame, looking at the playback
- Recording lines without a `Recording:` speaker prefix
- Spoken-hold timing (minimum 3.6s per line)
- Crackle one-shot per line
- Clean release into Intimate after-lines

No MetaHuman. No rendered slate movie.

## Quiet beat

- Intimate blend ~1.05s
- Maya closer on Follow, farther on Question (unchanged placement math, plus look-at-mug)
- Longer holds on “Tonight.” / “Don't know which one is mine.” / “Right. Sorry.”
- ~2.0s silence after the last line before tin is the beat
- Pump hum drops so the room can go quiet

Wrong-hand event remains dialogue + spacing. Not over-scored.

## Choice UI

`FAfterlightPresentationFormat::FormatChoiceList` stacks the two lines with a blank gap. No `[1]` / `[2]`. No trust/suspicion numbers. Warm off-white, lower-third. 1/2 still select. Cursor is hidden in cine mode. Dialogue speaker is a two-line subtitle (`Maya` then the line), not `Maya:`.

Temporary. Not a design system.

## HUD-free movie mode

Slice still starts in cine mode (`H` toggles).

Hidden: debug overlay, camera/register dump, relationship numbers, developer key legend, interaction prompts except Hatch/Tin (required to progress).

Visible: subtitles, choices, title card.

Playable: move when Full, 1/2 for choices, E on hatch/tin even when the prompt is the only cine exception.

## Title ending

Witness LED → ~1.4s silence → sting → black scrim → ~0.55s → **AFTERLIGHT**. No tagline. No logo animation.

## Temp light / atmosphere

Cheap point lights, no shadow, no Lumen quality target:

- Underdeck: cooler, lower
- Lantern Cut: warmer / amber, hide fill
- Pump House: warmer, drier, Witness LED as a small cold point

## Tests added

- `Afterlight.Slice01.HideRecoveryLerp`
- `Afterlight.UI.ChoicePresentation`
- `Afterlight.UI.SpokenHold`

Existing Afterlight group, Slice01 smoke, and CinematicLab smoke are preserved.

## Real-time movie test

- NullRHI Slice01 smoke: `AFTERLIGHT_SLICE_SMOKE=PASS`
- NullRHI lab smoke: `AFTERLIGHT_SMOKE_RESULT=PASS`
- D3D12 GPU Slice01 smoke (960×540, no NullRHI): `AFTERLIGHT_SLICE_SMOKE=PASS` on AMD Radeon Graphics (485 MB dedicated / shared UMA)
- Live D3D12 load of `L_Slice01_Greybox` completed (engine init ~19s). A full authored 7–9 minute sit was not left running: 15 GB RAM + ~46 GB commit ceiling and 485 MB iGPU VRAM. Stopped a 48s windowed viewing after map play rather than risk pagefile.

Do not treat accelerated smoke as a cinematic duration measurement.

## Remaining game-like elements

- Capsule Maya / Eli, engine cubes, text signs
- No footsteps mix, no human VO
- Hatch/Tin still use E
- Warning is a greybox slate, not a cracked-screen plate
- Drone is a spotlight sphere
- Choice selection is still 1/2 with no diegetic object

## Known Tier A limitations

- HW RT off, VSM off, lights do not cast shadows
- Procedural audio is mono 22kHz noise/tones
- Laptop compile may need `-NoPCH -MaxParallelActions=1` and a UBA temp file under commit/pagefile pressure
- Do not enable final post or MRQ on this machine

## Next recommended milestone

**Character Animation & Performance Prototype**

Do not start it from this document.
