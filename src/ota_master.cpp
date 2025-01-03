#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include "ota_master.h"
#include "build.h"
#include "i2c_master.h"
#include "interface.h"
#include <esp_ota_ops.h>
#include <SPIFFS.h>
#include <SD.h>
#ifndef FSYS
#define FSYS SD
#endif


static uint8_t ota_buf[8192];
static uint8_t ota_checksum(const uint8_t* data, size_t data_length,
                        uint8_t seed = 0xFE) {
    uint32_t res = seed;
    while (data_length--) {
        res ^= *(data++);
    }
    return res;
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
long ota_version(const char* prefix) {
    uint8_t addr = ota_addr_from_prefix(prefix);
    if(addr==0) {
        return build_time();
    } 
    Wire.beginTransmission(addr);
    uint8_t cmd = CMD_OTA_VER;
    Wire.write(&cmd,1);
    if(0!=Wire.endTransmission()) {
        return 0;
    }
    Wire.requestFrom(addr,sizeof(long));
    long ver = 0;
    if(Wire.available()) {
        uint8_t buf[sizeof(long)];
        if(sizeof(long)==Wire.readBytes(buf,sizeof(long))) {
            ver = *(long*)buf;
        }
    }
    if(0!=Wire.endTransmission()) {
        return 0;
    }    
    return ver;

}
bool ota_need_update(const char* prefix) {
    uint8_t addr = ota_addr_from_prefix(prefix);
    long fver;
    long id = ota_version(prefix);
    if(id==0) {
        //puts("Build time version was zero");
        return false;
    }
    fver = ota_file_version(prefix);
    if(fver==0) {
       //puts("File version was zero");
        return false;
    }
    return fver>id;
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
        uint8_t buf[1024];
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
            cmd_ota_block_t& block=*(cmd_ota_block_t*)(buf+1);
            size_t bytesread = file.read(block.data,UPDATE_BLOCK_SIZE);
            if(bytesread==0) {
                break;
            }
            block.seq = blocks;
            block.chk = ota_checksum(block.data,bytesread);
            block.length = bytesread;
            //printf("Checksum: %02X\n",block.chk);
            buf[0]=CMD_OTA_BLOCK;
            Wire.beginTransmission(addr);
            Wire.write(buf,1+sizeof(cmd_ota_block_t));
            int ret = Wire.endTransmission();
            if(0!=ret) {
                printf("I2C transmission failed: %d\n",ret);
                file.close();
                return false;
            }   
            ++blocks;
        }
        cmd_ota_done_t done;
        done.blocks = blocks;
        buf[0]=CMD_OTA_DONE;
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
    if(ESP_OK!=esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL))) {
        file.close();
        return false;
    }
    file.close();
    return true;
}
