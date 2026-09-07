# AFTERLIGHT Vertical Slice — Beat Sheet

Status: **Design locked for Slice 01.** Not implemented.

Runtime target: **8:15**. Range **7:30–9:00** if performance holds.

Related: `vertical-slice-story-bible.md`, `vertical-slice-screenplay.md`, `vertical-slice-state-graph.md`.

Player control uses existing states: **Full / Constrained / Scripted / Locked**.

Camera uses existing registers: **Explore / Dialogue / Intimate / Reveal / Threat / Cinematic**.

Story flags are proposed tags (not in C++ yet). Prefix: `Story.Slice01.*`

Relationship start: Trust **0.50**, Suspicion **0.00**.

---

## Control rhythm (overview)

| Window | State | Why |
|---|---|---|
| Wake / look | Constrained → Full | He is a body first, then a walker |
| First Talk + choice | Constrained | Face her; choose |
| Walk Underdeck → Lantern Cut | **Full** | Playable cinema; walk-and-talk |
| Hide from lantern-drone | Constrained | Look + small shift inside volume; no stroll into spotlight |
| Pump house entry / look | Full | Let the player find the room |
| Quiet mug beat | Constrained → short Scripted hold | Intimate; do not break eyeline |
| Tin inspect | Full to interact | Player does the discovery |
| Recording playback | **Locked** (~18s) | Only long lock in the slice |
| After recording | Scripted then Constrained | Faces, then look |
| Witness LED + title | Scripted → Locked title card | 15–25s ending |

Locked is used twice: recording, title. Everything else keeps the player inside the scene.

---

## Beat list

### B01 — Wake (0:00–0:50) · 50s · Underdeck 9

| Field | Content |
|---|---|
| Story | He is alive in a specific wet place. No name. |
| Control | Constrained look 8s, then **Full** |
| Camera | Explore |
| Dialogue | None at first. Optional: his breath. |
| Emotion | Disorientation, irritation at the body |
| Interaction | None required. Optional look-at Witness post (no prompt). |
| Flag | Grant `Story.Slice01.Woke` |
| Trust/Suspicion | Unchanged |
| Audio | Rain on metal. Distant train. **No music.** |
| Performance | Eli: eyes open, swallow, check hands, stand too fast, catch himself. |
| Transition | Maya’s footsteps in water. |

### B02 — Contact (0:50–1:40) · 50s · Underdeck 9

| Field | Content |
|---|---|
| Story | A woman knows him. He does not perform amnesia. |
| Control | Approach **Full**. On Talk: **Constrained** |
| Camera | Explore → Dialogue (`Dialogue.OTS.Companion`) cubic 0.85s |
| Dialogue | “Don’t stand up fast.” / name “Eli.” He does not take it. |
| Emotion | Curiosity, guard |
| Interaction | **Talk** (E) when close/facing |
| Flag | Grant `Story.Slice01.MetMaya` |
| Trust/Suspicion | Unchanged until choice |
| Audio | Rain continues. One thin string pad, very low, after she says the name. |
| Performance | Maya: wet, tired, checklist glance at his eyes. Eli: doesn’t answer to Eli. |
| Transition | She says they have to move. Choice. |

### B03 — Choice (1:40–2:25) · 45s · Underdeck 9

| Field | Content |
|---|---|
| Story | Psychology, not nice/mean. Follow vs demand a self. |
| Control | **Constrained** |
| Camera | Dialogue two-shot available; stay OTS unless blocked |
| Dialogue | See screenplay. Prompts are actions, not morality labels. |
| Emotion | Fragile connection **or** early refusal to be led |
| Interaction | Choice **1** or **2** (existing 1/2 keys) |
| Flag | `Story.Slice01.ChoseFollow` **or** `Story.Slice01.ChoseQuestion` (mutually exclusive) |
| Trust/Suspicion | **Follow:** Trust **+0.25** (→ 0.75, High, close follow). **Question:** Suspicion **+0.65** (→ 0.65, High, far follow). |
| Audio | Pad holds. No sting on choice. |
| Performance | Immediate: walk-off texture (distance, name, whether she looks back). |
| Transition | They take the stair. Blend Dialogue → Explore 0.9s, **Full**. Short Intimate hold **only** if Follow (0.5s, Scripted) then Explore. Question path: no Intimate hold. |

### B04 — Cut (2:25–3:15) · 50s · Underdeck stair → Lantern Cut

| Field | Content |
|---|---|
| Story | The city in policy and steam. Breathing room. |
| Control | **Full** (walk-and-talk) |
| Camera | Explore. Maya in frame via follow distance. |
| Dialogue | Companion lines while moving. Not a cutscene. |
| Emotion | Uneasy companionship |
| Interaction | Look. Optional glance at Helion notice (Inspect, optional flag `Story.Slice01.ReadNotice`). |
| Flag | `Story.Slice01.EnteredCut` on volume |
| Trust/Suspicion | Visible only through Maya’s walk |
| Audio | Afterlight hush. Kiosk radio under steam. |
| Performance | Maya walk cycle: Trust = ahead + check back; Suspicion = off-shoulder, less eye. |
| Transition | Distant lantern-drone hum. |

### B05 — Sweep (3:15–4:25) · 70s · Lantern Cut (kiosk recess)

| Field | Content |
|---|---|
| Story | Stakes without combat. Maya is too practiced. |
| Control | Enter hide volume: **Constrained**. Look + tiny strafe inside the recess. Spotlight = fail-soft (Maya yanks / hisses; no death). |
| Camera | Threat (`Threat.Pressure` recipe). Tighter, faster blend. |
| Dialogue | Cadence count. Trust: sleeve. Suspicion: “Down.” |
| Emotion | Urgency; a crack in her cover (she has done this) |
| Interaction | Be in volume before the spotlight crosses the stools. Prompt: **Cover** if player is late. |
| Flag | Grant `Story.Slice01.SweepPassed` when drone leaves |
| Trust/Suspicion | No numeric change. Performance already branched. |
| Audio | Polite drone VO. Spotlight hum. Music tightens, still quiet. |
| Performance | Maya mouths the count. Drone is slow, civic, uninterested in drama. |
| Transition | Steam, then the teal service door. Explore, **Full**. |

### B06 — Hatch (4:25–5:10) · 45s · Lantern Cut → Pump House 12

| Field | Content |
|---|---|
| Story | Apparent safety. The city muffles. |
| Control | **Full** |
| Camera | Explore. Brief Reveal opportunity on the door stencil `PMP-12`. |
| Dialogue | Almost none. Maybe: “Inside.” |
| Emotion | Relief that should not be complete |
| Interaction | **Use** door |
| Flag | Grant `Story.Slice01.ReachedBolt` |
| Trust/Suspicion | Unchanged |
| Audio | Rain drops off. Interior tick of settling metal. |
| Performance | Maya’s shoulders drop one inch. She still watches his hands. |
| Transition | Work lamp on. |

### B07 — Quiet (5:10–6:25) · 75s · Pump House 12

| Field | Content |
|---|---|
| Story | Warmth. The later reveal needs this or it is only a twist. |
| Control | Full to approach. **Constrained** during mug. 0.7s **Scripted** Intimate hold after the hand-switch. Then Full to look around. |
| Camera | Intimate (`Dialogue.CloseUp.Companion` / new tight two-shot if needed). Then Explore for room. |
| Dialogue | Mug in the wrong hand. Trust: dishwasher joke completes. Suspicion: joke dies. |
| Emotion | “I might actually trust her.” |
| Interaction | None required after she hands the mug (scripted give). Optional **Inspect** face-down photo. |
| Flag | `Story.Slice01.QuietBeat` |
| Trust/Suspicion | No new delta. Texture only. |
| Audio | Almost silence. Rain far. No score, or a single held note. |
| Performance | Essential: mug handoff, his switch, her “Right. Sorry.” See production plan. |
| Transition | His eye catches the tin / drawer. |

### B08 — Tin (6:25–6:50) · 25s · Pump House 12

| Field | Content |
|---|---|
| Story | He finds what she hoped was later. |
| Control | **Full** until Inspect. Then Constrained as she speaks. |
| Camera | Reveal insert on tin. Blend 0.7s. |
| Dialogue | Trust: “Don’t—” then she stops. Suspicion: “Eli—” too fast. |
| Emotion | The warmth cracks |
| Interaction | **Inspect** tin (required) |
| Flag | Grant `Story.Slice01.FoundTin` |
| Trust/Suspicion | Unchanged numerically |
| Audio | Metal lid. |
| Performance | Her aborted reach. |
| Transition | Device in his hand. Playback. |

### B09 — Warning (6:50–7:50) · 60s · Pump House 12

| Field | Content |
|---|---|
| Story | Dual suspicion: Maya, and the man on the slate. |
| Control | **Locked** for playback (~18s sequence). Then **Scripted** on faces (~8s). Then Constrained look. |
| Camera | Cinematic (Level Sequence on the slate: still of his face + waveform/subtitles optional). Then Intimate Maya. Then two-shot. |
| Dialogue | Recording (see screenplay). Then: “He sounds so sure.” / “Was he?” / “No.” |
| Emotion | Disturbance; love as operational risk |
| Interaction | None during playback |
| Flag | Grant `Story.Slice01.HeardWarning` |
| Trust/Suspicion | No automatic delta. Player meaning is the point. |
| Audio | His recorded voice, drier, more composed. Room tone underneath. Music out. |
| Performance | Eli listens like it’s a stranger. Maya does not cry. She looks at the man, not the device. |
| Transition | Outside, a Witness LED ticks from dead to live. |

### B10 — Afterlight (7:50–8:15) · 25s · Pump House 12 → black

| Field | Content |
|---|---|
| Story | The city returns. She does not ask him to choose. Title earned. |
| Control | Scripted look toward the hatch LED, then **Locked** title |
| Camera | Reveal on the LED through the door glass / seam. Then black. |
| Dialogue | None after “No.” Silence is the line. |
| Emotion | Unresolved pull |
| Interaction | None |
| Flag | Grant `Story.Slice01.Complete` |
| Trust/Suspicion | Held |
| Audio | Small electronic tick of Witness going live. Rain leaks back in. Title: silence or one low tone. |
| Performance | Maya looks at the door, then at him. She waits. He does not answer. |
| Transition | **AFTERLIGHT**. End slice. |

---

## Choice specification (single branch texture)

**Prompt context:** Maya: they have to move; Witness will write him if he stays.

| | Follow | Question |
|---|---|---|
| Player-facing | **1. Walk.** | **2. “You talk like I belong to you.”** |
| Psychology | Instinctive survival / competence-following | Refuse to be led until he has a self |
| Flag | `ChoseFollow` | `ChoseQuestion` |
| Math | Trust +0.25 | Suspicion +0.65 |
| Immediate Maya | Relief she hides. Ahead + check back. Uses Eli once more. | “I don’t. That’s the problem.” Off-shoulder. Drops the name. |
| Sweep | Sleeve grab | Palm, no contact |
| Quiet | Sits near him; finishes joke | Stands; kills joke |
| Tin | Shame, lets him | Aborted grab |
| Ending | Same recording, same LED, same title | Same |

No second map. No extra shoot day. Convergence at B04.

---

## Flag list (proposed)

```
Story.Slice01.Woke
Story.Slice01.MetMaya
Story.Slice01.ChoseFollow
Story.Slice01.ChoseQuestion
Story.Slice01.EnteredCut
Story.Slice01.ReadNotice          (optional)
Story.Slice01.SweepPassed
Story.Slice01.ReachedBolt
Story.Slice01.QuietBeat
Story.Slice01.LookedPhoto         (optional)
Story.Slice01.FoundTin
Story.Slice01.HeardWarning
Story.Slice01.Complete
```

`ChoseFollow` and `ChoseQuestion` must not both be present.

---

## Camera mapping vs existing recipes

| Beat | Register | Reuse | New? |
|---|---|---|---|
| B01, B04, B06, room look | Explore | Yes (arm 360, FOV 58) | No |
| B02–B03 | Dialogue | `Dialogue.OTS.Companion`, TwoShot fallback | Lab OTS compositions, reauthored to Maya |
| B03 Follow hold | Intimate | Close-up companion | Optional skip if time is tight |
| B05 | Threat | `Threat.Pressure` | Hide framing in the kiosk, not a new register |
| B07 | Intimate | Close-up + two-shot | **One new recipe optional:** `Dialogue.TwoShot.Quiet` slightly tighter than lab TwoShot |
| B08 | Reveal | `Reveal.Insert` | Tin insert |
| B09 | Cinematic | Coordinator + Level Sequence | `LS_Slice01_Warning` (~18s) — new sequence, same authority model |
| B10 | Reveal → Cinematic title | Reveal insert on LED | Title card can be UMG-free sequence or simple fade |

Do not add registers. At most one quiet two-shot recipe.

---

## Audio map (restrained)

| Beat | Ambience | Music | Voice | Cue |
|---|---|---|---|---|
| B01 | Rain, train | None | Breath | — |
| B02 | Rain | Barely-there pad after “Eli” | Maya, Eli | Name |
| B03 | Rain | Hold | Choice | No sting |
| B04 | Afterlight hush, radio | Pad off or thinner | Walk lines | — |
| B05 | Drone VO, spotlight | Tight pulse, low | Maya count | Threat |
| B06 | Rain off, metal tick | Down | Minimal | Door |
| B07 | Far rain | **Silence** | Quiet lines | Warmth |
| B08 | Lid | Silence | Her interrupt | — |
| B09 | Room tone | **Out** | Recorded Eli | Reveal |
| B10 | Witness tick, rain leak | Silence | None | Title |

The slice must not sound like a trailer for eight minutes.

---

## Animation / performance requirements

### Essential for first cinematic implementation

**Maya:** tired walk; look-back (Trust); off-shoulder idle (Suspicion); eye checklist; sleeve grab; palm-stop; mug handoff; sit-on-floor vs stand; aborted reach to tin; listen-without-crying; look door then him.

**Eli:** wake; stand too fast; not-answering to a name (stillness); walk; hide crouch/shift; receive mug; switch hands; inspect tin; listen to his own voice as a stranger.

**Both:** wet cloth, breath in cold, not mannequin idles. If a line is playing, they are doing a small physical task (wiping rain, watching the cut, holding a mug).

### High-end polish later

MetaHuman eyes/wetness, microexpressions, rain interaction on hair, full body mocap for hide, recorded-video on the slate instead of still+audio, crowd silhouettes above the Cut.

---

## Edge cases (seamless slice only)

| Player does | Response |
|---|---|
| Walks away from Maya in Underdeck | She does not chase-bark. After ~8s: “Eli.” If still leaving: she steps to block the open bay, not a quest marker. Volume soft-gate at the far rain curtain (invisible wall only at map edge). |
| Delays Talk | She waits. After a long look she can initiate Talk if he is in range facing her (she speaks first). No spam. |
| Looks away during Dialogue | Camera stays on register. She does not freeze. Constrained look still allowed. |
| Looks at Witness too long in B01/B04 | Maya steps into eyeline: “Don’t.” No numeric punishment in v1. |
| Enters hide late | Soft fail: spotlight graze, she pulls him, drone VO continues, still `SweepPassed`. No death, no combat. |
| Reaches pump door early | Door locked until `SweepPassed`. Maya: “Not yet.” |
| Re-talk Maya | After MetMaya, Talk is disabled except choice. After choice, companion lines are walk-and-talk, not a new tree. |
| Inspect tin early | Tin is not interactable until `QuietBeat`. |
| Cine / HUD-free (H) | Required for YouTube path. Dialogue captions may remain; no debug, no Trust numbers, no prompts except Talk/Cover/Use/Inspect when needed. |
| Save/load (debug) | Allowed. Restore beat id + flags + relationship. Do not leave register stuck on Cinematic. Coordinator cancel on load if sequence was mid-play. |
| Skip/interrupt recording | Not in v1. Playback is short. Esc/cancel later. |

---

## Runtime check

50+50+45+50+70+45+75+25+60+25 = **495s = 8:15**.

If walk-and-talk runs long, cut B04 optional notice inspect, not the quiet beat or the recording.
