# Starts New Editor Window PIE only when afterlight.ps1 play passes -AfterlightAutoPlay.
# Normal editor/test launches are unchanged.

import unreal

_state = {
    "phase": "wait_map",
    "map_ok": 0,
    "ticks": 0,
}


def _log(message):
    unreal.log("AFTERLIGHT_AUTOPLAY " + message)


def _autoplay_requested():
    try:
        return "AfterlightAutoPlay" in unreal.SystemLibrary.get_command_line()
    except Exception:
        return False


def _editor_world():
    try:
        ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        world = ues.get_editor_world()
        if world:
            return world
    except Exception:
        pass
    try:
        return unreal.EditorLevelLibrary.get_editor_world()
    except Exception:
        return None


def _world_is_slice01(world):
    if not world:
        return False
    blob = (str(world.get_name()) + " " + str(world.get_path_name())).lower()
    return "slice01" in blob or "greybox" in blob


def _assets_ready():
    try:
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        return not registry.is_loading_assets()
    except Exception:
        return True


def _request_pie():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if hasattr(les, "editor_request_begin_play"):
        les.editor_request_begin_play()
        return
    if hasattr(les, "editor_request_play_session"):
        les.editor_request_play_session()
        return
    unreal.SystemLibrary.execute_console_command(None, "PlayWorld.RepeatLastPlay")


def _tick(_delta):
    _state["ticks"] += 1
    if _state["phase"] == "done":
        return False
    if _state["phase"] == "wait_map":
        if _world_is_slice01(_editor_world()) and _assets_ready():
            _state["map_ok"] += 1
        if _state["map_ok"] >= 180:
            _log("request_pie")
            try:
                _request_pie()
            except Exception as exc:
                _log("pie_failed " + str(exc))
            _state["phase"] = "done"
            return False
        if _state["ticks"] > 4800:
            _log("timeout_waiting_for_map")
            _state["phase"] = "done"
            return False
        return True
    return False


if _autoplay_requested():
    unreal.register_slate_post_tick_callback(_tick)
    _log("loaded")
