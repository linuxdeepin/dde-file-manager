// config.h.in platfrom plugin output
// set(DFM_PLUGIN_PATH) in cmake
// if setting new definition, please use:
//  #undef DFM_PLUGIN_PATH
//  #define DFM_PLUGIN_PATH xxx
#ifndef DFM_PLUGIN_PATH
#define DFM_PLUGIN_PATH "/usr/lib/x86_64-linux-gnu/plugins/dfm-framework"
#elif
    #warning "cmake unseting definition DFM_PLUGIN_PATH"
#endif

#ifndef DFM_BUILD_OUT_PLGUN_DIR
#define DFM_BUILD_OUT_PLGUN_DIR "/home/workspace/recode/dde-file-manager/build/Release"
#elif
    #warning "cmake unseting definition DFM_BUILD_OUT_PLGUN_DIR"
#endif
