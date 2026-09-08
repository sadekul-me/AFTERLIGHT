# AFTERLIGHT — Autonomous Sprint 01

Validated on the owner laptop (16 GB RAM, AMD iGPU, Unreal 5.8.2). Git is owner-only.

## Implemented and seen

- Underdeck 9 wake reads as a lit industrial corridor: Eli body, Maya down-route, floor strip, signage.
- Maya/Eli keep Quinn/Manny, with local tint plus a dark Eli plate and amber Maya plate.
- Contact two-shot reframed onto the pair with a dressed UD-9 wall behind them.
- Lantern Cut uses cyan/Helion language; Underdeck uses amber; a floor-strip transition sits at the join.
- Pump House has ceiling, extra machinery, pipes, table/mug/tin, warmer light.
- Warning slate is a framed green screen with flicker; camera includes Maya.
- Drone hull is larger, with a ring and scan cone. A later threat frame shows the hull/ring filling the shot (still not a clean player/Maya relationship).
- Owner launch remains `.\afterlight.ps1 play` (DX11, low scalability, 50% screen, 64 MB pool).
- Workflow verbs: `play`, `build`, `test`, `smoke`, `verify`, `all`.

## Visually validated (Unreal-native `Saved/QA/*.png`)

Wake, Maya arrival, Contact, Maya CU, Choice, Lantern Cut, Drone. Pump House / Warning / Ending were not captured in the interrupted 3 FPS sweep stall.

## Measured performance

- Best session: first minute **55–60 FPS** in New Editor Window, then **~3 FPS** from Lantern Cut onward.
- Tight-commit session: **~3 FPS** from wake. Editor often dies after the sweep.
- Bottleneck is Windows commit / paging on this 16 GB laptop, not a missing 60 FPS architecture. When commit headroom exists, the safe DX11 profile is already double-digit.

## Still placeholder

- Manny/Quinn, not MetaHuman.
- BasicShape rooms, not final art.
- Procedural beds, not VO or Wwise.
- One walk + idle animation only.

## MetaHuman

Do **not** install on this laptop. SKM_Manny already requested ~4.6 GB compile headroom. One MetaHuman would destabilize the 16 GB / iGPU box. Hand that step to a stronger machine after this slice is stable.

## Next phase

One only: **second-half presentation + drone-in-frame proof** (Pump House, warning, ending, threat still) on a continuous run that stays out of the hide-alcove trap.
