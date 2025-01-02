#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include "ota_master.h"
#include "build.h"
#include "i2c_master.h"
#include "interface.h"
#include <SPIFFS.h>
#include <esp_ota_ops.h>
#define FSYS SPIFFS
static uint8_t ota_buf[8192];
static uint8_t ota_checksum(const uint8_t* data, size_t data_length,
                        uint8_t seed = 0xFE) {
    while (data_length--) {
        seed ^= *(data++);
    }
    return seed;
}
static File ota_file(const char* prefix) {
    FSYS.begin();
    char buf[9];
    buf[0]=0;
    buf[8]=0;
    File root = FSYS.open("/");
    if(root) {
        File file = root.openNextFile();
        while (file) {
            size_t len = strlen(file.name());
            if(len==15 && 0==strcmp(file.name()+(len-4),".bin")) {
                if(0==strncmp(file.name(),prefix,3)) {
                    strncpy(buf,file.name()+3,8);
                    char* end;
                    long res = strtol(buf,&end,16);
                    if(res!=0) {
                        root.close();
                        return file;
                    }
                }
            }
            file = root.openNextFile();
        }
        if(file) {
            file.close();
        }
        root.close();
    }
    return root; // return closed file
}
static uint8_t ota_addr_from_prefix(const char* prefix) {
    if(0==strcmp("drv",prefix)) {
        return I2C_DRVHL_ADDR;
    } else if(0==strcmp("pas",prefix)) {
        return I2C_PASHL_ADDR;
    }
    return 0;
}
static long ota_file_version(const char* prefix) {
    char buf[9];
    buf[0]=0;
    buf[8]=0;
    File file=ota_file(prefix);
    if(file) {
        size_t len = strlen(file.name());
        if(len==15 && 0==strcmp(file.name()+(len-4),".bin")) {
            if(0==strncmp(file.name(),prefix,3)) {
                strncpy(buf,file.name()+3,8);
                char* end;
                long res = strtol(buf,&end,16);
                if(res!=0) {
                    file.close();
                    return res;
                }
            }
        }
        file.close();
    }
    return 0;
}
bool ota_need_update(const char* prefix) {
    uint8_t addr = ota_addr_from_prefix(prefix);
    long fver;
    if(addr==0) {
        long id = (long)build_time();
        fver = ota_file_version(prefix);
        if(fver==0) {
            return false;
        }
        return fver>id;
    }
    Wire.beginTransmission(addr);
    uint8_t cmd = CMD_OTA_VER;
    Wire.write(&cmd,1);
    Wire.requestFrom(addr,sizeof(long));
    long ver = 0;
    if(Wire.available()) {
        if(sizeof(long)==Wire.readBytes(ota_buf,sizeof(long))) {
            ver = *reinterpret_cast<long*>(ota_buf);
        }
    }
    if(0!=Wire.endTransmission()) {
        return false;
    }
    fver = ota_file_version(prefix);
    return ver!=0 && fver!=0 && fver>ver;
}
bool ota_update(const char* prefix) {
    uint8_t addr = ota_addr_from_prefix(prefix);
    File file = ota_file(prefix);
    if(!file) {
        return false;
    }
    // update a slave
    if(addr!=0) {
        size_t blocks = 0;
        uint8_t buf[257+sizeof(cmd_ota_block_t)];
        buf[0]=CMD_OTA_START;
        cmd_ota_start_t start;
        start.size = file.size();
        memcpy(buf+1,&start,sizeof(start));
        Wire.beginTransmission(addr);
        Wire.write(buf,1+sizeof(start));
        if(0!=Wire.endTransmission()) {
            file.close();
            return false;
        }
        while(true) {
            cmd_ota_block_t block;
            size_t bytesread = file.read(buf+1+sizeof(block),sizeof(buf)-1-sizeof(block));
            if(bytesread==0) {
                break;
            }
            block.seq = blocks;
            block.chk = ota_checksum(buf+1+sizeof(block),sizeof(buf)-1-sizeof(block));
            block.length = bytesread;
            Wire.beginTransmission(addr);
            Wire.write(buf,1+sizeof(block)+bytesread);
            if(0!=Wire.endTransmission()) {
                file.close();
                return false;
            }   
            ++blocks;
        }
        cmd_ota_done_t done;
        done.blocks = blocks;
        memcpy(buf+1,&start,sizeof(start));
        Wire.beginTransmission(addr);
        Wire.write(buf,1+sizeof(start));
        if(0!=Wire.endTransmission()) {
            file.close();
            return false;
        }
        return true;
    }
    esp_ota_handle_t hota;
    if(ESP_OK!=esp_ota_begin(esp_ota_get_next_update_partition(NULL),file.size(),&hota)) {
        return false;
    }
    while(true) {
        size_t bytesread = file.read(ota_buf,sizeof(ota_buf));
        if(bytesread==0) {
            break;
        }
        if(ESP_OK!=esp_ota_write(hota,ota_buf,bytesread)) {
            file.close();
            return false;
        }
    }
    if(ESP_OK!=esp_ota_end(hota)) {
        file.close();
        return false;
    }
    return true;
}
