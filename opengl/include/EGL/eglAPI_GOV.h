#ifndef NewFrame_H_
#define NewFrame_H_

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ThreadLogger Info", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ThreadLogger Warning", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "ThreadLogger Error", __VA_ARGS__))
#define LOGLOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "123456789, ", __VA_ARGS__))

#define CONFIG_PATH "/data/local/GameOptimized_Gov_Games.config"

#endif /* NewFrame_H_ */
