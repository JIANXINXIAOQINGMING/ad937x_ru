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

#define CAP_VERSION                                 3.0

static void print_usage(FILE *stream, int exit_code)
{
    printf("\tCapture version: %02.02f\n", CAP_VERSION);
    fprintf(stream,
            "\t-h  --help            Display this usage information\n"
            "\t-b  --base addr       input base address\n"
            "\t-r  --read            only read\n"
            "\t-c  --capture         capture\n"
            "\t-s  --sample rate     sample rate -1\n"
            "\t-n  --node            port name\n"
            "\t-t  --total           capture total number\n");
    exit(exit_code);
}

int register_handle(char *file_name, uint32_t addr, uint16_t sample_rate, uint16_t node, uint32_t total, short recapture_flag ,uint16_t offset)
{
    volatile uint32_t re_value=0;
    uint32_t *map_base, *virt_addr;

    uint16_t i=0;
    char data_total[36]={0};

    FILE *fp = NULL;

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

    if(RATE_BIT_DECIDE(recapture_flag))
    {
        virt_addr = map_base + SIGNAL_CAPTURE_RATE + offset;
        *(volatile uint32_t *)virt_addr = sample_rate;

        printf("Setting sample rate is complete\n");
    }

    if(SELECT_BIT_DECIDE(recapture_flag))
    {
        //choice capture port
        virt_addr = map_base + SIGNAL_CAPTURE_SELECT + offset;
        *(volatile uint32_t *)virt_addr = node;

        printf("Setting node is complete\n");
    }

    //recapture data
    if(RE_BIT_DECIDE(recapture_flag))
    {
        virt_addr = map_base + SIGNAL_CAPTURE_RE + offset;
        *(volatile uint32_t *)virt_addr = 0;
        usleep(1);
        *(volatile uint32_t *)virt_addr = 1;

        printf("Setting recapture is complete\n");
    }

    if(TOTAL_BIT_DECIDE(recapture_flag))
    {
        virt_addr=map_base + SIGNAL_CAPTURE_ADDR + offset;
        *(volatile uint32_t *)virt_addr = total;

        printf("Setting total is complete\n");
    }

    if(RE_BIT_DECIDE(recapture_flag) || ONLY_READ_BIT_DECIDE(recapture_flag))
    {
        if(total == 0)
        {
            total = *(map_base + SIGNAL_CAPTURE_ADDR + offset);
            if(total == 0)
            {
                return ERROR_INPUT;
            }
        }

        printf("wait busy..\n");
        while(*(map_base + SIGNAL_CAPTURE_BUSY + offset))
        {
            sleep(1);
        }

        //open record the file
        creat(file_name,0766);
        fp = fopen(file_name, "w");
        if (fp == NULL)
        {
            printf("Error %s file open fail!\n",file_name);
            exit(ERROR_OPEN_DEV);
        }

        for(i = 0; i <= total; i++)
        {
            *(volatile uint32_t *)virt_addr = i;
            memset(data_total, 0, 36);

            re_value=*(map_base + SIGNAL_CAPTURE_DATAOUT + offset);
            sprintf(data_total,"%08X\n",re_value);

            fputs(data_total,fp);
        }

        fclose(fp);
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
    char file_name[64]={0};

    char *input_str = NULL;
    uint32_t base_addr = 0;
    uint32_t total = 0;
    uint16_t sample_rate = 0;
    uint16_t node = 0;
    uint8_t recapture_flag = 0;         //0:recapture  1:total  2: node  3: rate  4:capture

    const char *const short_options = "crb:s:n:t:";
    const struct option long_options[] = {
        {"help", 0, NULL, 'h'},
        {"base", 1, NULL, 'b'},
        {"rate", 1, NULL, 's'},
        {"capture", 0, NULL, 'c'},
        {"read", 0, NULL, 'r'},
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
            base_addr = strtoul(optarg,&input_str,16);
            if(strlen(input_str) != 0)
                print_usage(stdout,ERROR_INPUT);
            break;
        case 's':
            sample_rate = strtoul(optarg,&input_str,10);
            if(strlen(input_str) != 0 || sample_rate > 255)
                print_usage(stdout,ERROR_INPUT);
            recapture_flag = RATE_BIT_COMPUTE(recapture_flag);
            break;
        case 'r':
            if(!RE_BIT_DECIDE(recapture_flag))
                recapture_flag = ONLY_READ_BIT_COMPUTE(recapture_flag);
            else
                print_usage(stdout,ERROR_INPUT);
            break;
        case 'c':
            if(!ONLY_READ_BIT_DECIDE(recapture_flag))
                recapture_flag = RE_BIT_COMPUTE(recapture_flag);
            else
                print_usage(stdout,ERROR_INPUT);
            break;
        case 'n':
            node = strtoul(optarg,&input_str,10);
            if(strlen(input_str) != 0)
                print_usage(stdout,ERROR_INPUT);
            recapture_flag = SELECT_BIT_COMPUTE(recapture_flag);
            break;
        case 't':
            total = strtoul(optarg,&input_str,10) - 1;
            if(strlen(input_str) != 0)
                print_usage(stdout,ERROR_INPUT);
            recapture_flag = TOTAL_BIT_COMPUTE(recapture_flag);
            break;
        default:
            print_usage(stdout,ERROR_INPUT);
        }
    }

    sprintf(file_name,"/tmp/Capture_Signal_%X.txt",base_addr&0x0000ffff);
    if(access(file_name,F_OK))
    {
        remove(file_name);
    }

    register_handle(file_name,base_addr,sample_rate,node,total,recapture_flag,(base_addr&0x00000fff)/4);

    input_str = NULL;
    return RETURN_SUCCESSFUL;
}