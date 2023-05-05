#define M64P_PLUGIN_PROTOTYPES 1
#define INPUT_PLUGIN_API_VERSION 0x020100

#include "util.h"

#include "main.h"

#include "gc_adapter.h"
#include "config.h"
#include "mapping.h"
#include "log.h"


//
// Local Variables
//

static int l_IsInitialized = 0;

//
// Public Variables
//

ptr_ConfigOpenSection ConfigOpenSection = NULL;
ptr_ConfigSaveSection ConfigSaveSection = NULL;
ptr_ConfigSetDefaultInt ConfigSetDefaultInt = NULL;
ptr_ConfigSetDefaultBool ConfigSetDefaultBool = NULL;
ptr_ConfigGetParamInt ConfigGetParamInt  = NULL;
ptr_ConfigGetParamBool ConfigGetParamBool = NULL;
ptr_ConfigSetParameter ConfigSetParameter = NULL;

void (*debug_callback)(void *, int, const char *);
void *debug_callback_context = NULL;

//
// Basic Plugin Functions
//

EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context, void (*DebugCallback)(void *, int, const char *))
{
    if (l_IsInitialized)
    {
        return M64ERR_ALREADY_INIT;
    }

    // setup debug callback
    debug_callback_context = Context;
    debug_callback         = DebugCallback;

    // setup config function pointers
    ConfigOpenSection = (ptr_ConfigOpenSection)DLSYM(CoreLibHandle, "ConfigOpenSection");
    ConfigSaveSection = (ptr_ConfigSaveSection)DLSYM(CoreLibHandle, "ConfigSaveSection");
    ConfigSetDefaultInt = (ptr_ConfigSetDefaultInt)DLSYM(CoreLibHandle, "ConfigSetDefaultInt");
    ConfigSetDefaultBool = (ptr_ConfigSetDefaultBool)DLSYM(CoreLibHandle, "ConfigSetDefaultBool");
    ConfigGetParamInt = (ptr_ConfigGetParamInt)DLSYM(CoreLibHandle, "ConfigGetParamInt");
    ConfigGetParamBool = (ptr_ConfigGetParamBool)DLSYM(CoreLibHandle, "ConfigGetParamBool");
    ConfigSetParameter = (ptr_ConfigSetParameter)DLSYM(CoreLibHandle, "ConfigSetParameter");

    // try to load the config
    config_load();

    l_IsInitialized = 1;
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    if (!l_IsInitialized)
    {
        return M64ERR_NOT_INIT;
    }

    // reset debug callback
    debug_callback_context = NULL;
    debug_callback         = NULL;

    l_IsInitialized = 0;
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *pluginType, int *pluginVersion, 
    int *apiVersion, const char **pluginNamePtr, int *capabilities)
{
    if (pluginType != NULL)
    {
        *pluginType = M64PLUGIN_INPUT;
    }

    if (pluginVersion != NULL)
    {
        *pluginVersion = 0x010000;
    }

    if (apiVersion != NULL)
    {
        *apiVersion = INPUT_PLUGIN_API_VERSION;
    }

    if (pluginNamePtr != NULL)
    {
        *pluginNamePtr = "mupen64plus-input-gcn";
    }

    if (capabilities != NULL)
    {
        *capabilities = 0;
    }

    return M64ERR_SUCCESS;
}

//
// Custom Plugin Functions
//

/*
EXPORT m64p_error CALL PluginConfig()
{
    // TODO
    return M64ERR_SUCCESS;
}
*/

//
// Input Plugin Functions
//

EXPORT void CALL ControllerCommand(int Control, unsigned char* Command)
{
}

EXPORT void CALL GetKeys(int Control, BUTTONS* Keys)
{
    gc_inputs i;

    int err = gc_get_inputs(Control, &i);
    if (err)
        return;

    Keys->Value = 0;

    if (!gc_is_present(i.status)) {
        return;
    }

    Control = cfg.single_mapping ? 0 : Control;

    gc_inputs id = i;
    process_inputs_digital(&id);
    process_inputs_analog(&i);

    Keys->A_BUTTON     = get_mapping_state(&i, &id, cfg.mapping[Control].a, 0);
    Keys->B_BUTTON     = get_mapping_state(&i, &id, cfg.mapping[Control].b, 0);
    Keys->Z_TRIG       = get_mapping_state(&i, &id, cfg.mapping[Control].z, 0);
    Keys->L_TRIG       = get_mapping_state(&i, &id, cfg.mapping[Control].l, 0);
    Keys->R_TRIG       = get_mapping_state(&i, &id, cfg.mapping[Control].r, 0);
    Keys->START_BUTTON = get_mapping_state(&i, &id, cfg.mapping[Control].start, 0);

    Keys->L_CBUTTON = get_mapping_state(&i, &id, cfg.mapping[Control].c_left, 0);
    Keys->R_CBUTTON = get_mapping_state(&i, &id, cfg.mapping[Control].c_right, 0);
    Keys->D_CBUTTON = get_mapping_state(&i, &id, cfg.mapping[Control].c_down, 0);
    Keys->U_CBUTTON = get_mapping_state(&i, &id, cfg.mapping[Control].c_up, 0);

    Keys->L_DPAD = get_mapping_state(&i, &id, cfg.mapping[Control].d_left, 0);
    Keys->R_DPAD = get_mapping_state(&i, &id, cfg.mapping[Control].d_right, 0);
    Keys->D_DPAD = get_mapping_state(&i, &id, cfg.mapping[Control].d_down, 0);
    Keys->U_DPAD = get_mapping_state(&i, &id, cfg.mapping[Control].d_up, 0);

    Keys->X_AXIS = get_mapping_state(&i, &id, cfg.mapping[Control].analog_up, 1)
                 - get_mapping_state(&i, &id, cfg.mapping[Control].analog_down, 1);

    Keys->Y_AXIS = get_mapping_state(&i, &id, cfg.mapping[Control].analog_right, 1)
                 - get_mapping_state(&i, &id, cfg.mapping[Control].analog_left, 1);
}

EXPORT void CALL InitiateControllers(CONTROL_INFO ControlInfo)
{
    dlog(LOG_INFO, "InitiateControllers()");
    gc_init(cfg.async);
    if (gc_get_init_error() == GCERR_LIBUSB_OPEN) {
        return;
    }

    gc_inputs gc[4];

    // workaround for incorrect status being reported
    // when trying to use the adapter directly after it's been plugged in.
    // i < 2 works for my official adapter, but i've put in i < 10 to be safe
    // (which should stall for only 80ms with the default 125hz pollrate)
    if (!gc_is_async()) { 
        for (int i = 0; i < 10; ++i) { 
            int err = gc_get_all_inputs(gc);
            if (err) return;
        }
    } else {
        msleep(80);
        int err = gc_get_all_inputs(gc);
        if (err) return;
    }

    int concount = 0;

    for (int i = 0; i < 4; ++i) {
        int status = gc[i].status;
        int mi = cfg.single_mapping ? 0 : i;

        if (gc_is_present(status)) {
            ControlInfo.Controls[i].Present = cfg.mapping[mi].enabled ? 1 : 0;
            dlog(LOG_INFO, "Controller %d present, status 0x%X", i, status);
            ++concount;
        } else {
            ControlInfo.Controls[i].Present = cfg.mapping[mi].force_plugged && cfg.mapping[mi].enabled ? 1 : 0;
            dlog(LOG_INFO, "Controller %d not available, status 0x%X", i, status);
        }

        ControlInfo.Controls[i].RawData = 0;
        ControlInfo.Controls[i].Plugin = PLUGIN_NONE + cfg.mapping[mi].accessory;
    }

    if (concount == 0) {
        dlog(LOG_WARN, "No controllers detected\n");
    }
}

EXPORT void CALL ReadController(int Control, unsigned char *Command)
{
}

EXPORT int CALL RomOpen(void)
{
    gc_init(cfg.async);
    return 1;
}

EXPORT void CALL RomClosed(void)
{
    gc_deinit();
}

EXPORT void CALL SDL_KeyDown(int keymod, int keysym)
{
}

EXPORT void CALL SDL_KeyUp(int keymod, int keysym)
{
}

