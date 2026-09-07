# Load L_Dev_CinematicLab and leave exactly one DirectionalLight and one SkyLight.

import unreal

MAP_PATH = "/Game/Environments/Slice01/L_Dev_CinematicLab"

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

if not level_editor.load_level(MAP_PATH):
    unreal.log_error("AFTERLIGHT_LIGHT_FIX_FAIL load_level")
    unreal.SystemLibrary.execute_console_command(None, "QUIT")
    raise SystemExit(1)

suns = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.DirectionalLight)]
skies = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.SkyLight)]
unreal.log("AFTERLIGHT lights before sun=%s sky=%s" % (len(suns), len(skies)))

for extra in suns[1:]:
    actors.destroy_actor(extra)
for extra in skies[1:]:
    actors.destroy_actor(extra)

if not suns:
    sun = actors.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0.0, 0.0, 400.0),
        unreal.Rotator(-46.0, -35.0, 0.0),
    )
    if sun:
        light = sun.get_component_by_class(unreal.DirectionalLightComponent)
        if light:
            light.set_editor_property("intensity", 8.0)

if not skies:
    sky = actors.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 480.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    if sky:
        sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
        if sky_comp:
            sky_comp.set_editor_property("real_time_capture", True)
            sky_comp.set_editor_property("intensity", 1.0)

suns = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.DirectionalLight)]
skies = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.SkyLight)]
unreal.log("AFTERLIGHT_LIGHT_FIX sun=%s sky=%s" % (len(suns), len(skies)))

if not level_editor.save_current_level():
    unreal.log_error("AFTERLIGHT_LIGHT_FIX_FAIL save")
else:
    unreal.log("AFTERLIGHT_LIGHT_FIX_OK")

unreal.SystemLibrary.execute_console_command(None, "QUIT")
