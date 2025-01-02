#ifndef OTA_MASTER_H
#define OTA_MASTER_H
#include <stdint.h>
#include <stddef.h>
bool ota_need_update(const char* prefix);
bool ota_update(const char* prefix);
long ota_version(const char* prefix);
#endif // OTA_MASTER_H