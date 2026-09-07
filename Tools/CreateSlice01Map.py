# Creates Content/Environments/Slice01/L_Slice01_Greybox with lights, PlayerStart, and Slice01 director.
# Greybox geometry is spawned at runtime by AfterlightSlice01Director.

import unreal

MAP_PATH = "/Game/Environments/Slice01/L_Slice01_Greybox"

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

unreal.log("AFTERLIGHT creating " + MAP_PATH)
if not level_editor.new_level(MAP_PATH, False):
    unreal.log_error("AFTERLIGHT_MAP_CREATE_FAIL new_level")
    raise SystemExit(1)

for actor in list(actors.get_all_level_actors()):
    if isinstance(actor, (unreal.DirectionalLight, unreal.SkyLight)):
        unreal.log("AFTERLIGHT removing default light " + actor.get_name())
        actors.destroy_actor(actor)

sun = actors.spawn_actor_from_class(
    unreal.DirectionalLight,
    unreal.Vector(0.0, 0.0, 400.0),
    unreal.Rotator(-50.0, -20.0, 0.0),
)
if sun:
    light = sun.get_component_by_class(unreal.DirectionalLightComponent)
    if light:
        light.set_editor_property("intensity", 4.5)
        light.set_editor_property("use_temperature", True)
        light.set_editor_property("temperature", 4300.0)

sky = actors.spawn_actor_from_class(
    unreal.SkyLight,
    unreal.Vector(0.0, 0.0, 480.0),
    unreal.Rotator(0.0, 0.0, 0.0),
)
if sky:
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    if sky_comp:
        sky_comp.set_editor_property("real_time_capture", True)
        sky_comp.set_editor_property("intensity", 0.55)

actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator())
actors.spawn_actor_from_class(
    unreal.PlayerStart,
    unreal.Vector(180.0, 0.0, 100.0),
    unreal.Rotator(0.0, 0.0, 0.0),
)
actors.spawn_actor_from_class(
    unreal.AfterlightSlice01Director,
    unreal.Vector(0.0, 0.0, 0.0),
    unreal.Rotator(0.0, 0.0, 0.0),
)

if not level_editor.save_current_level():
    unreal.log_error("AFTERLIGHT_MAP_CREATE_FAIL save")
    raise SystemExit(1)

unreal.log("AFTERLIGHT_MAP_CREATE_OK " + MAP_PATH)
unreal.SystemLibrary.execute_console_command(None, "QUIT")
