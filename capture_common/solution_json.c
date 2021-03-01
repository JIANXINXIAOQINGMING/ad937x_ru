#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include "cJSON.h"
#include "common_interface.h"

int setJson(cJSON *root) //以递归的方式打印json的最内层键值对
{
    // int SN_data_length,MAC_data_length,BATCH_data_length,TIME_data_length;

    // if (!root)
    // {
    //     return -1; //JSON formatting error
    // }
    // //MAC解码
    // cJSON *RAU_MAC = cJSON_GetObjectItem(root, "MAC");
    // if (RAU_MAC)
    // {
    //     uint8_t MAC[6]={0};
    //     char *MAC_str = RAU_MAC->valuestring;
    //     check_mac(MAC_str,MAC);

    //     if (is_valid_ethaddr(MAC))
    //     {
    //         MAC_data_length=sizeof(MAC);
    //         if(MAC_data_length==6)
    //             set_eeprom(EEPROM_NODE_0,(char *)MAC, MAC_OFFSET, MAC_LEN, MAC_data_length);
    //     }
    //     else
    //     {
    //         printf("MAC address format error!\n");
    //         exit(-1);
    //     }
    // }
    // //SN解码
    // cJSON *RAU_SN = cJSON_GetObjectItem(root, "SN");
    // if (RAU_SN)
    // {
    //     char *SN = RAU_SN->valuestring;
    //     SN_data_length=strlen(SN);
    //     if(SN_data_length>64)
    //     {
    //         SN_data_length=64;
    //     }
    //     set_eeprom(EEPROM_NODE_0, SN, SN_OFFSET, SN_LEN,SN_data_length);

    // }
    // //生产批次
    // cJSON *RAU_BATCH = cJSON_GetObjectItem(root, "BATCH_INFO");
    // if (RAU_BATCH)
    // {
    //     char *BATCH = RAU_BATCH->valuestring;
    //     BATCH_data_length=strlen(BATCH);
    //     if(BATCH_data_length>BATCH_LEN)
    //     {
    //         BATCH_data_length=BATCH_LEN;
    //     }
    //     set_eeprom(EEPROM_NODE_1,BATCH, BATCH_OFFSET, BATCH_LEN,BATCH_data_length);
    // }
    // //生产日期
    // cJSON *RAU_TIME = cJSON_GetObjectItem(root, "BuildTime");
    // if (RAU_TIME)
    // {
    //     char *TIME = RAU_TIME->valuestring;
    //     TIME_data_length=strlen(TIME);
    //     if(TIME_data_length>TIME_LEN)
    //     {
    //         TIME_data_length=TIME_LEN;
    //     }
    //     set_eeprom(EEPROM_NODE_1,TIME, TIME_OFFSET, TIME_LEN,TIME_data_length);
    // }
    return 0;
}

int json_read(void)
{
    int check_val = 0;
    int j = 0;
    char *st_data, *st_data_z, *read_json_origin;
    cJSON *read_json_parse;

    st_data = (char *)malloc(sizeof(char) * 1024);
    st_data_z = (char *)malloc(sizeof(char) * 1024);

    while (j <= 30)
    {
        read_json_origin = fgets(st_data, 150, stdin);
        if (read_json_origin == NULL)
            break;

        strcat(st_data_z, st_data);
        j++;
    }

    read_json_parse = cJSON_Parse(st_data_z);
    check_val = setJson(read_json_parse);

    if (check_val != 0)
    {
        printf("Write error!");
    }
    cJSON_Delete(read_json_parse);

    return 0;
}