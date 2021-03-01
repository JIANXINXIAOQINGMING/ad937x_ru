#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <getopt.h>
#include "cJSON.h"
#include "common_interface.h"
#include "capture_common.h"

#define CAP_VERSION                 2.5

#define SIGNAL_CAPTURE_EN           1
#define SIGNAL_CAPTURE_SET          2

#define SIGNAL_CAPTURE_RATE         0x0
#define SIGNAL_CAPTURE_RE           0x1
#define SIGNAL_CAPTURE_BUSY         0x2
#define SIGNAL_CAPTURE_SELECT       0x3
#define SIGNAL_CAPTURE_ADDR         0x4
#define SIGNAL_CAPTURE_DATAOUT      0x5

#define RE_ENABLE_STATUS(x)         x==1?"True":"False"

static void print_usage(FILE *stream, int exit_code)
{
    printf("\tCapture version: %02.02f\n", CAP_VERSION);
    fprintf(stream,
            "\t-h  --help            Display this usage information\n"
            "\t-b  --base addr       input base address\n"
            "\t-r  --recapture       recapture enable and only set mode\n"
            "\t                      1.T:recapture enable\n"
            "\t                      2.F:recapture disable\n"
            "\t                      3.S:set capture range\n"
            "\t-s  --sample rate     sample rate\n"
            "\t-n  --node            port name\n"
            "\t-t  --total           capture total number\n"
            "Example:capture -b xxx -r xx -s xxx -n xxx -t xx\n");
    exit(exit_code);
}

int register_handle(FILE *fp, uint32_t addr, uint16_t sample_rate, uint16_t node, uint32_t total, short recapture_flag ,uint16_t offset)
{
    volatile uint32_t re_value=0;
    uint32_t *map_base, *virt_addr;

    uint16_t i=0;
    char data_total[36]={0};

    /*打开mem的这个虚拟设备*/
    int fd=0;
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd < 0)
    {
        printf("Capture open /dev/mem fail\n");
        exit(ERROR_OPEN_DEV);
    }

    /*mmap函数的调用*/
    volatile off_t target;
    target = addr;
    map_base = (uint32_t *)mmap(NULL, DEVMEM_MAP_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, target & ~DEVMEM_MAP_MASK);
    /*用mmap的函数返回值，判断是否申请虚拟地址成功*/
    if (map_base == MAP_FAILED)
    {
        printf("Capture create mmap MAP_FAILED\n");
        close(fd);
        exit(ERROR_OPEN_DEV);
    }


    virt_addr = map_base + SIGNAL_CAPTURE_RATE + offset;
    *(volatile uint32_t *)virt_addr = sample_rate;

    //recapture data
    if(recapture_flag == SIGNAL_CAPTURE_EN)
    {
        virt_addr = map_base + SIGNAL_CAPTURE_RE + offset;
        *(volatile uint32_t *)virt_addr = 0;
        usleep(1);
        *(volatile uint32_t *)virt_addr = 1;
    }
    printf("Setting parameters is complete\n");

    //choice capture port
    virt_addr = map_base + SIGNAL_CAPTURE_SELECT + offset;
    *(volatile uint32_t *)virt_addr = node;

    if(recapture_flag != SIGNAL_CAPTURE_SET)
    {
        printf("wait..\n");
        while(*(map_base + SIGNAL_CAPTURE_BUSY + offset))
        {
            sleep(1);
        }

        virt_addr=map_base + SIGNAL_CAPTURE_ADDR + offset;
        *(volatile uint32_t *)virt_addr = total;

        for(i = 0; i <= total; i++)
        {
            *(volatile uint32_t *)virt_addr = i;
            memset(data_total, 0, 36);

            re_value=*(map_base + SIGNAL_CAPTURE_DATAOUT + offset);
            sprintf(data_total,"%08X\n",re_value);

            fputs(data_total,fp);
        }
    }

    /* 删除分配的虚拟地址 */
    if (munmap(map_base, DEVMEM_MAP_SIZE) == -1)
        printf("Delete DEVMEM FAIL DEVMEM_MAP_SIZE\n");

    close(fd);
    return RETURN_SUCCESSFUL;
}

int main(int argc, char *argv[])
{
    int next_option = 1;
    short flag=0;
    char ver_info[168]={0};
    char file_name[64]={0};

    char *input_str = NULL;
    uint32_t base_addr = 0;
    uint32_t total = 0;
    uint16_t sample_rate = 0;
    uint16_t node = 0;
    int recapture_flag = 0;
    short input_flag = 0;

    FILE *fp = NULL;

    const char *const short_options = "b:s:r:n:t:";
    const struct option long_options[] = {
        {"help", 0, NULL, 'h'},
        {"base", 1, NULL, 'b'},
        {"rate", 1, NULL, 's'},
        {"recapture", 1, NULL, 'r'},
        {"node", 1, NULL, 'n'},
        {"total", 1, NULL, 't'},
        {NULL, 0, NULL, 0}};

    while (1)
    {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);

        if(next_option==-1)
            break;
        else
            flag++;

        switch (next_option)
        {
        case 'b':
            base_addr = strtol(optarg,&input_str,16);
            if(strlen(input_str) != 0)
                print_usage(stdout,ERROR_INPUT);
            input_flag++;
            break;
        case 's':
            sample_rate = strtol(optarg,&input_str,10);
            if(strlen(input_str) != 0 || sample_rate > 255)
                print_usage(stdout,ERROR_INPUT);
            input_flag++;
            break;
        case 'r':
            if(strcmp("T",optarg)==0)
                recapture_flag=SIGNAL_CAPTURE_EN;
            else if(strcmp("S",optarg)==0)
                recapture_flag=SIGNAL_CAPTURE_SET;
            else if(strcmp("F",optarg)==0)
                recapture_flag=0;
            else
                print_usage(stdout,ERROR_INPUT);
            break;
        case 'n':
            node = strtol(optarg,&input_str,10);
            if(strlen(input_str) != 0)
                print_usage(stdout,ERROR_INPUT);
            input_flag++;
            break;
        case 't':
            total = strtol(optarg,&input_str,10);
            if(strlen(input_str) != 0)
                print_usage(stdout,ERROR_INPUT);
            input_flag++;
            break;
        default:
            print_usage(stdout,ERROR_INPUT);
        }
    }

    sprintf(file_name,"/tmp/Capture_Signal_%X",base_addr&0x0000ffff);
    if(access(file_name,F_OK))
    {
        remove(file_name);
    }

    if(input_flag == 4 || (recapture_flag == SIGNAL_CAPTURE_SET && input_flag == 3 && total == 0))
    {
        //open record the file
        fp = fopen(file_name, "w");
        if (fp == NULL)
        {
            printf("Error %s file open fail!\n",file_name);
            exit(ERROR_OPEN_DEV);
        }

        register_handle(fp,base_addr,sample_rate,node,total,recapture_flag,(base_addr&0x00000fff)/4);

        sprintf(ver_info,"Capture_Signal  base address:%#x  Sample rate:%d  Node:%d  total:%d  Recapture enable:%s  Capture version: %02.02f\n",base_addr,sample_rate,node,total,RE_ENABLE_STATUS(recapture_flag),CAP_VERSION);
        fputs(ver_info,fp);
        fclose(fp);
    }
    else
    {
        print_usage(stdout,ERROR_INPUT);
    }

    fp = NULL;
    input_str = NULL;

    return RETURN_SUCCESSFUL;
}