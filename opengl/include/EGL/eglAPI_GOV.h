#ifndef GAMEGOVERNOR_EGL_API_H_
#define GAMEGOVERNOR_EGL_API_H__

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "EGL-INFO", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "EGL-WARN", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "EGL-ERR", __VA_ARGS__))
#define LOGFPS(...) ((void)__android_log_print(ANDROID_LOG_INFO, "EGL-FPS, ", __VA_ARGS__))

#define CONFIG_PATH "/data/local/gamegovernor/GovernorGames.conf"

#endif // GAMEGOVERNOR_EGL_API_H_
