# Loads L_Slice01_Greybox, starts PIE, runs Afterlight.SmokeSlice01, then quits.

import unreal

MAP_PATH = "/Game/Environments/Slice01/L_Slice01_Greybox"
state = {"frames": 0, "started": False, "smoked": False, "done": False}

unreal.EditorPythonScripting.set_keep_python_script_alive(True)

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not level_editor.load_level(MAP_PATH):
    unreal.log_error("AFTERLIGHT_PIE_FAIL load_level")
    unreal.EditorPythonScripting.set_keep_python_script_alive(False)
    unreal.SystemLibrary.quit_editor()
else:
    unreal.log("AFTERLIGHT_PIE_MAP_LOADED " + MAP_PATH)
    unreal.log("AFTERLIGHT_PIE_BEGIN")
    level_editor.editor_request_begin_play()


def _tick(_delta_seconds):
    if state["done"]:
        return True
    state["frames"] += 1
    if (not state["started"]) and state["frames"] >= 20:
        state["started"] = True
        if not level_editor.is_in_play_in_editor():
            level_editor.editor_request_begin_play()
    if state["started"] and (not state["smoked"]) and state["frames"] >= 80:
        state["smoked"] = True
        in_pie = level_editor.is_in_play_in_editor()
        unreal.log("AFTERLIGHT_PIE_ACTIVE=%s" % in_pie)
        unreal.SystemLibrary.execute_console_command(None, "Afterlight.SmokeSlice01")
    if state["frames"] >= 160:
        state["done"] = True
        if level_editor.is_in_play_in_editor():
            level_editor.editor_request_end_play()
        unreal.log("AFTERLIGHT_PIE_DONE")
        unreal.EditorPythonScripting.set_keep_python_script_alive(False)
        unreal.SystemLibrary.quit_editor()
    return True


unreal.register_slate_pre_tick_callback(_tick)
