#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <getopt.h>

#include "common_interface.h"

#define CAP_VERSION                 2.0

#define SIGNAL_CAPTURE_EN           1

#define SIGNAL_CAPTURE_RATE         0x0
#define SIGNAL_CAPTURE_RE           0x4
#define SIGNAL_CAPTURE_BUSY         0x8
#define SIGNAL_CAPTURE_SELECT       0xC
#define SIGNAL_CAPTURE_ADDR         0x10
#define SIGNAL_CAPTURE_DATAOUT      0x14

#define SIGNAL_FILE_NAME            "/coredump/Capture_Signal.txt"

static void print_usage(FILE *stream, int exit_code)
{
    printf("\tCapture version: %02.02f\n", CAP_VERSION);
    fprintf(stream,
            "\t-h  --help            Display this usage information\n"
            "\t-b  --base addr       input base address\n"
            "\t-r  --recapture       recapture enable True or False\n"
            "\t-s  --sample rate     sample rate\n"
            "\t-n  --node            port name\n"
            "Example:rau_capture -b xxx -r xx -s xxx -n xxx\n");
    exit(exit_code);
}


// int register_handle(char *file_name, uint32_t addr, int node, int mode)
// {
//     volatile uint32_t re_value=0;
//     uint32_t *map_base, *virt_addr;

//     uint16_t i=0;
//     char data_total[36]={0};

//     FILE *fp;

//     /*打开mem的这个虚拟设备*/
//     int fd=0;
//     fd = open("/dev/mem", (O_RDWR | O_SYNC));
//     if (fd < 0)
//     {
//         COMMIF_ERROR("Capture open /dev/mem fail\n");
//         exit(1);
//     }

//     //open record the file
//     fp = fopen(file_name, "w");
//     if (fp == NULL)
//     {
//         COMMIF_ERROR("Capture open %s file fail\n",file_name);
//         exit(1);
//     }

//     /*mmap函数的调用*/
//     volatile off_t target;
//     target = addr;
//     map_base = (uint32_t *)mmap(NULL, DEVMEM_MAP_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, target & ~DEVMEM_MAP_MASK);
//     /*用mmap的函数返回值，判断是否申请虚拟地址成功*/
//     if (map_base == MAP_FAILED)
//     {
//         COMMIF_ERROR("Capture create mmap MAP_FAILED\n");
//         close(fd);
//         exit(-1);
//     }

//     if(mode==AFU_CAPTURE)
//     {
//         //restart capture
//         virt_addr = map_base + AFU_CAPTURE_RE;
//         *(volatile uint32_t *)virt_addr = 1;

//         // //choice capture port
//         virt_addr = map_base + AFU_CAPTURE_SEL;
//         *(volatile uint32_t *)virt_addr = node;

//         printf("wait..\n");
//         while(*(map_base + AFU_CAPTURE_BUSY))
//         {
//             sleep(0.1);
//         }

//         virt_addr=map_base + AFU_CAPTURE_ADDR;
//         for(i=0;i<=32767;i++)
//         {
//             *(volatile uint32_t *)virt_addr = i;

//             re_value=*(map_base + AFU_CAPTURE_DATAOUTH);
//             re_value=re_value<<2;

//             // sleep(0.01);
//             sprintf(data_total,"%08X%07X\n",re_value,*(map_base + AFU_CAPTURE_DATAOUTL));

//             fputs(data_total,fp);
//         }
//     }
//     else
//     {
//         virt_addr = map_base + SIGNAL_CAPTURE_RE;
//         *(volatile uint32_t *)virt_addr = 1;

//         // //choice capture port
//         virt_addr = map_base + SIGNAL_CAPTURE_SEL;
//         *(volatile uint32_t *)virt_addr = node;

//         printf("wait..\n");
//         while(*(map_base + SIGNAL_CAPTURE_BUSY))
//         {
//             sleep(0.1);
//         }

//         virt_addr=map_base + SIGNAL_CAPTURE_ADDR;
//         for(i=0;i<=32767;i++)
//         {
//             *(volatile uint32_t *)virt_addr = i;
//             usleep(0.1);

//             re_value=*(map_base + SIGNAL_CAPTURE_DATAOUT);
//             sprintf(data_total,"%08X\n",re_value);

//             fputs(data_total,fp);
//         }
//     }

//     /* 删除分配的虚拟地址 */
//     if (munmap(map_base, DEVMEM_MAP_SIZE) == -1)
//         COMMIF_ERROR("Delete DEVMEM FAIL DEVMEM_MAP_SIZE\n");

//     fclose(fp);
//     close(fd);
//     return 0;
// }


// int start_grab(char *file_name, uint32_t addr, int node,int mode)
// {
//     char cmd[64]={0};

//     sprintf(cmd,"touch %s",file_name);
//     system(cmd);
//     memset(cmd,0,64);

//     register_handle(file_name,addr,node,mode);

//     return 0;
// }

int main(int argc, char *argv[])
{
    int next_option = 1;
    short flag=0;
    char ver_info[128]={0};
    char node_name[16]={0};

    char *input_str = NULL;
    uint32_t base_addr = 0;
    uint16_t sample_rate = 0;
    uint16_t node = 0;
    uint8_t recapture_flag = 0;

    const char *const short_options = "b:s:r:n:";
    const struct option long_options[] = {
        {"help", 0, NULL, 'h'},
        {"base", 1, NULL, 'b'},
        {"rate", 1, NULL, 's'},
        {"recapture", 1, NULL, 'r'},
        {"node", 1, NULL, 'n'},
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
            break;
        case 's':
            sample_rate = strtol(optarg,&input_str,10);
            if(strlen(input_str) != 0 || sample_rate > 255)
                print_usage(stdout,ERROR_INPUT);
            break;
        case 'r':
            if(strcmp("True",optarg)==0)
                recapture_flag=SIGNAL_CAPTURE_EN;
            else if(strcmp("False",optarg)==0)
                recapture_flag=0;
            else
                print_usage(stdout,ERROR_INPUT);
            break;
        case 'n':
            node = strtol(optarg,&input_str,10);
            if(strlen(input_str) != 0)
                print_usage(stdout,ERROR_INPUT);
            break;
        default:
            print_usage(stdout,ERROR_INPUT);
        }
    }


    // if(mode!=-1 && type!=-1 && node!=ERROR_INPUT)
    // {
    //     start_grab(AFU_FILE_NAME,AFU_BASE,node,mode);
    //     sprintf(ver_info,"echo \"Capture_Mode:afu  Type:%s  Node:%s  Capture version: %02.02f\" >> %s\n",TYPE_DECIDE(type),node_name,CAP_VERSION,AFU_FILE_NAME);
    // }
    // else
    // {
    //     print_usage(stdout,ERROR_INPUT);
    // }

    // system(ver_info);

    return 0;
}