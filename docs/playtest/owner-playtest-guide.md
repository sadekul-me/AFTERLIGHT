# AFTERLIGHT — Owner Playtest Guide

This is for playing the current vertical slice. You do not need Unreal knowledge beyond the steps below.

## How to launch

1. Open PowerShell in `D:\Programming\AFTERLIGHT`
2. Run:

```
.\afterlight.ps1 play
```

3. Wait until the log says Slice01 is loaded. Play should start itself in a **New Editor Window** on the primary monitor (`AFTERLIGHT Preview`, 854x480). If it does not, click the editor and press **Alt+P**.
4. Click inside that Preview window. On the black **AFTERLIGHT** card, **Left Click**, or press **Space** / **Enter**. Do not use the editor Outliner. Do not minimize the editor.

The map is `/Game/Environments/Slice01/L_Slice01_Greybox`. You should not need to pick a map.

## Who you are

You are **Eli**, waking in Underdeck 9. **Maya** is down the corridor. Follow her.

## Controls

| Action | Input |
|---|---|
| Move | WASD |
| Look | Mouse |
| Talk / Open / Inspect | E (when the prompt appears) |
| Choose a line | 1 or 2 |
| Exit the ended slice | Esc |
| Replay after the end card | R |
| Stop Play in the Editor | Esc |

Developer keys (F5–F10, F8 debug) still exist. Ignore them for this playtest.

## What happens

1. Wake. Look, then walk toward Maya.
2. Talk. Choose **1** or **2**.
3. Follow Maya through Lantern Cut.
4. When the drone comes, stay out of the light. Step into the side recess with Maya.
5. Follow her to the hatch. **E Open**.
6. Sit through the quiet mug scene.
7. Look where Maya looks. **E Inspect** the tin.
8. Watch the warning. Do not try to move.
9. **AFTERLIGHT**, then **END OF VERTICAL SLICE**.

Runtime is about **7–9 minutes** if you walk with Maya. Faster if you never linger.

## Exit / restart

- During play in the Editor: **Esc** stops Play.
- On the end card: **Esc** exits a standalone session; in the Editor it also stops Play. **R** replays from wake.

## Greybox limits

Manny/Quinn placeholders instead of final faces. Boxes instead of rooms. Placeholder rain/hum, no human voices. Subtitles stand in for speech. This is still a cinematic demo, not the finished film.

Other commands: `.\afterlight.ps1 build` · `test` · `smoke` · `verify` · `all`.
