# AFTERLIGHT Vertical Slice — Production Plan

Status: **Design package.** No Unreal implementation in this milestone.

Slice: **Afterlight Window** · ~8:15 · three spaces · one choice · one sequence.

This plan maps the screenplay onto systems that already exist, names the smallest extensions, and keeps first production buildable on the current C++ foundation.

---

## 1. Feasibility by beat

| Beat | Existing system supports | Small extension | New capability | High-risk cinematic |
|---|---|---|---|---|
| B01 Wake | Explore, Full/Constrained, protagonist pawn | Idle/wake montage without sequence | — | Wet cloth / rain on body (visual) |
| B02 Contact | Talk interact, Dialogue register, Constrained | — | — | Maya performance |
| B03 Choice | Dialogue choices, Trust/Suspicion deltas, follow tags | Intimate hold only on Follow path | — | — |
| B04 Walk-and-talk | Explore, Full, companion follow | **Companion lines while Full** (lab dialogue currently assumes Constrained Talk) | — | — |
| B05 Sweep | Threat register, Constrained | **Hide volume** + fail-soft pull; simple drone actor on a spline/timeline | No combat AI | Drone as readable civic object |
| B06 Door | Use interact, Explore | Door locked until flag | — | — |
| B07 Quiet | Intimate, Scripted hold, follow tags | Mug handoff as scripted attach or montage | — | Acting quality |
| B08 Tin | Inspect, Reveal insert | Interactable gated by QuietBeat | — | Handwriting decal |
| B09 Recording | Cinematic coordinator, Level Sequence, Locked | `LS_Slice01_Warning` content | Media still + audio on a slate mesh | If we demand full video lipsync in pass 1 |
| B10 Title | Sequence or simple fade + text | Witness LED material switch | — | Restraint (do not over-score) |

**Pass 1 rule:** still + audio on the slate, not a full in-world movie file. Upgrade later.

No beat requires combat, inventory, World Partition, MetaHuman (placeholders can block), Wwise, or networking.

---

## 2. Small extensions (named, still later)

Do not build these in this milestone. Greybox pass 1 should expect:

1. **Walk-and-talk:** dialogue runner (or a tiny companion bark component) can present lines while input is Full.
2. **Hide volume:** overlap that sets Constrained, Threat register, grants `SweepPassed` on timeline end.
3. **Flag-gated interactables:** tin/door already match Inspect/Use; they need required-flag checks.
4. **Relationship presentation beyond distance:** sleeve vs palm, sit vs stand, joke branch — can be animation notifies + two takes, still driven by existing High Trust / High Suspicion tags.
5. **LED material:** dead/live on Witness posts.

None of these are new camera registers or a new relationship model.

---

## 3. Environment assets

### Essential (greybox can stand in; final listed)

| Item | Greybox | Final |
|---|---|---|
| Underdeck corridor, stair | Boxes, rain planes | Concrete, pipes, tide marks |
| Witness post (2–3) | Box + emissive LED | Helion civic prop |
| Lantern Cut, shutters, clock | Corridor + sign decals | Stack housing facades (modular, small) |
| Kiosk + chained stools | Cube stall | Steam, broth, radio |
| Helion notice | Decal | Laminated prop, handwritten underline |
| Drone | Moving light + speaker | Slow civic quad, spotlight |
| Pump House 12 | One room | Gauges, pump, work lamp |
| Two mugs, thermos, towel, coat | Primitives | Hero props |
| Tin + cracked slate | Inspectable | Handwriting lid, still image |
| Face-down photo | Optional inspectable | Print prop |

### Can use placeholder

- Rain: particle or simple mesh sheets
- Distant train: audio only
- Child’s toy: optional
- Crowd: none in pass 1 (Afterlight empty is the point)

### Final polish later

- Hero Maya / Eli MetaHumans
- Wetness, breath vapor, proper puddle reflections
- Full lantern-drone mesh and lights
- Video on slate
- City beyond the Cut (we never go there)

**Do not download marketplace cities.** The spine is three rooms.

---

## 4. Lighting / weather

| Space | Mood | Pass 1 | Later |
|---|---|---|---|
| Underdeck | Sodium line, rain curtains | One key + fill, rain cards | Lumen wet response (Tier B) |
| Lantern Cut | Warm kiosk vs cold LEDs | Localized lights, drone spotlight | Volumetrics |
| Pump house | Single work lamp | Interior key | Practicals, tick sparks |

No final post stack in pass 1. HUD-free capture is the picture target.

---

## 5. Animation / performance

See beat sheet for the split. Pass 1 can ship with:

- Engine walk/idle
- Additive look-at (already)
- A handful of montage: sleeve, palm, mug pass, sit, aborted reach, crouch hide
- Face: even crude visemes + Maya’s “checklist” stare

Pass 2 (not this milestone): MetaHuman, mocap hide, recorded video.

---

## 6. Audio (no Wwise/FMOD required in pass 1)

Engine cues + a few waves:

- Rain_Metal, Rain_Far, Train_Distant
- Kiosk_Radio_Low, Pump_Tick
- Drone_Hum, Drone_VO_CourtesyPass
- Witness_LED_Tick
- Recording_Eli (VO hero)
- Maya / Eli loc (even temp VO)

Music: two stems max (thin pad, tighter pulse). Silence is a cue.

---

## 7. Camera

Reuse recipes from the camera spike. Author lab-equivalent shots in the real spaces:

- OTS Maya, OTS Eli, TwoShot, Close-up Maya, Reveal tin, Threat hide, LED insert

New content, not new architecture:

- `LS_Slice01_Warning` (~18s)
- Optional recipe `Dialogue.TwoShot.Quiet`

Coordinator already owns Locked + return register.

---

## 8. Edge cases

Canonical list lives in `vertical-slice-beats.md`. Production must implement at least:

- Soft map-edge on Underdeck rain curtain
- Maya can start Talk if the player stalls in range
- Door locked until sweep
- Tin locked until quiet beat
- Hide fail-soft (no death)
- Cine mode hides debug
- Load cannot stick Cinematic authority

---

## 9. YouTube capture workflow (dev)

Already exists: **H** HUD-free. For slice capture:

1. Cine mode on before B01 or at Wake.
2. Debug overlay off.
3. Dialogue captions allowed if we treat them as burned-in subtitles; otherwise VO only.
4. No F8/F7/F9/F10 in the take.
5. Record from spawn through title. One take preferred; hide fail-soft exists so a missed cover does not ruin the file.

Full checklist: `docs/production/youtube-movie-test.md`.

---

## 10. Suggested implementation order (next milestone, not now)

1. Greybox three spaces on a new map (not the lab as ship target; lab remains a camera gym).
2. Spawn Maya, Witness posts, door, tin, drone timeline.
3. Port dialogue asset from the screenplay.
4. Walk-and-talk Full lines.
5. Hide volume + Threat.
6. Choice deltas (already lab-identical).
7. Quiet Intimate + relationship texture (distance first, extra anim later).
8. `LS_Slice01_Warning` still+audio.
9. HUD-free full take.
10. Only then MetaHuman / hero env.

---

## 11. Risks this plan accepts

- Walk-and-talk is the main *code* gap vs the lab.
- Maya idling like an NPC will kill the YouTube take; even placeholder motion must be busy (wipe rain, watch cut, hold thermos).
- Recording still vs video: still is the buildable reveal.
- Temp VO will make the recording land less than hero VO; schedule VO before calling the slice “cinematic done.”
