#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <getopt.h>
#include "commif.h"
#include "internal_commif.h"

#define CAP_VERSION                 1.4

#define AFU_BASE                    0xA0002E00
#define SIGNAL_BASE                 0xA0002E40

#define UPLINK_DATA                 0
#define DOWNLINK_DATA               1

#define AFU_CAPTURE                 2
#define SIGNAL_CAPTURE              3

#define AFU_FILE_NAME               "/coredump/Capture_AFU.txt"
#define SIGNAL_FILE_NAME            "/coredump/Capture_Signal.txt"

#define TYPE_DECIDE(y)              (y==UPLINK_DATA)?"UPLINK":"DOWNLINK"

enum
{
    AFU_CAPTURE_RE=0x380,
    AFU_CAPTURE_BUSY,
    AFU_CAPTURE_SEL,
    AFU_CAPTURE_ADDR,
    AFU_CAPTURE_DATAOUTL,
    AFU_CAPTURE_DATAOUTH
};

enum
{
    SIGNAL_CAPTURE_RE=0x390,
    SIGNAL_CAPTURE_BUSY,
    SIGNAL_CAPTURE_SEL,
    SIGNAL_CAPTURE_ADDR,
    SIGNAL_CAPTURE_DATAOUT
};

static void print_usage(FILE *stream, int exit_code)
{
    printf("\tCapture version: %02.02f\n", CAP_VERSION);
    fprintf(stream,
            "\t-h  --help     Display this usage information\n"
            "\t-m  --mode     AFU or signal\n"
            "\t-t  --type     uplink(ul) or downlink(dl)\n"
            "\t-n  --node     port name\n"
            "-------------------------------------------------------------------------------------------------\n"
            "|   \\   |                   carrier\\port CPRI name                                            |\n"
            "-------------------------------------------------------------------------------------------------\n"
            "|  AFU  |   A    |   B    |   C    |   D    |        |        |        |        |               |\n"
            "-------------------------------------------------------------------------------------------------\n"
            "| SIGNAL| port1A | port1B | port2A | port2B | port3A | port3B | port4A | port4B | carrier(0~23) |\n"
            "-------------------------------------------------------------------------------------------------\n"
            "Example:rau_capture -m xxx -t xx -n xxx\n");
    exit(exit_code);
}


int register_handle(char *file_name, uint32_t addr, int node, int mode)
{
    volatile uint32_t re_value=0;
    uint32_t *map_base, *virt_addr;

    uint16_t i=0;
    char data_total[36]={0};

    FILE *fp;

    /*打开mem的这个虚拟设备*/
    int fd=0;
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd < 0)
    {
        COMMIF_ERROR("Capture open /dev/mem fail\n");
        exit(1);
    }

    //open record the file
    fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        COMMIF_ERROR("Capture open %s file fail\n",file_name);
        exit(1);
    }

    /*mmap函数的调用*/
    volatile off_t target;
    target = addr;
    map_base = (uint32_t *)mmap(NULL, DEVMEM_MAP_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, target & ~DEVMEM_MAP_MASK);
    /*用mmap的函数返回值，判断是否申请虚拟地址成功*/
    if (map_base == MAP_FAILED)
    {
        COMMIF_ERROR("Capture create mmap MAP_FAILED\n");
        close(fd);
        exit(-1);
    }

    if(mode==AFU_CAPTURE)
    {
        //restart capture
        virt_addr = map_base + AFU_CAPTURE_RE;
        *(volatile uint32_t *)virt_addr = 1;

        // //choice capture port
        virt_addr = map_base + AFU_CAPTURE_SEL;
        *(volatile uint32_t *)virt_addr = node;

        printf("wait..\n");
        while(*(map_base + AFU_CAPTURE_BUSY))
        {
            sleep(0.1);
        }

        virt_addr=map_base + AFU_CAPTURE_ADDR;
        for(i=0;i<=32767;i++)
        {
            *(volatile uint32_t *)virt_addr = i;

            re_value=*(map_base + AFU_CAPTURE_DATAOUTH);
            re_value=re_value<<2;

            // sleep(0.01);
            sprintf(data_total,"%08X%07X\n",re_value,*(map_base + AFU_CAPTURE_DATAOUTL));

            fputs(data_total,fp);
        }
    }
    else
    {
        virt_addr = map_base + SIGNAL_CAPTURE_RE;
        *(volatile uint32_t *)virt_addr = 1;

        // //choice capture port
        virt_addr = map_base + SIGNAL_CAPTURE_SEL;
        *(volatile uint32_t *)virt_addr = node;

        printf("wait..\n");
        while(*(map_base + SIGNAL_CAPTURE_BUSY))
        {
            sleep(0.1);
        }

        virt_addr=map_base + SIGNAL_CAPTURE_ADDR;
        for(i=0;i<=32767;i++)
        {
            *(volatile uint32_t *)virt_addr = i;
            usleep(0.1);

            re_value=*(map_base + SIGNAL_CAPTURE_DATAOUT);
            sprintf(data_total,"%08X\n",re_value);

            fputs(data_total,fp);
        }
    }

    /* 删除分配的虚拟地址 */
    if (munmap(map_base, DEVMEM_MAP_SIZE) == -1)
        COMMIF_ERROR("Delete DEVMEM FAIL DEVMEM_MAP_SIZE\n");

    fclose(fp);
    close(fd);
    return 0;
}


int start_grab(char *file_name, uint32_t addr, int node,int mode)
{
    char cmd[64]={0};

    sprintf(cmd,"touch %s",file_name);
    system(cmd);
    memset(cmd,0,64);

    register_handle(file_name,addr,node,mode);

    sprintf(cmd,"dfe_print >> %s",file_name);
    system(cmd);
    memset(cmd,0,64);
    sprintf(cmd,"dfe_cpri print ul >> %s",file_name);
    system(cmd);
    memset(cmd,0,64);
    sprintf(cmd,"dfe_cpri print dl >> %s",file_name);
    system(cmd);

    return 0;
}

static int return_value(int type,int start, int skip)
{
    if(type==DOWNLINK_DATA)
        return start;
    else
        return skip+start;
}

static int transform_info(char *info_name,int mode,int type)
{
    if(mode==AFU_CAPTURE)
    {
        if(strcmp("A",info_name)==0)
        {
            return return_value(type,0,4);
        }
        else if(strcmp("B",info_name)==0)
        {
            return return_value(type,1,4);
        }
        else if(strcmp("C",info_name)==0)
        {
            return return_value(type,2,4);
        }
        else if(strcmp("D",info_name)==0)
        {
            return return_value(type,3,4);
        }
        else
        {
            return ERROR_INPUT;
        }
    }
    else
    {
        if(strcmp("port1A",info_name)==0)
        {
            return return_value(type,0,8);
        }
        else if(strcmp("port1B",info_name)==0)
        {
            return return_value(type,1,8);
        }
        else if(strcmp("port2A",info_name)==0)
        {
            return return_value(type,2,8);
        }
        else if(strcmp("port2B",info_name)==0)
        {
            return return_value(type,3,8);
        }
        else if(strcmp("port3A",info_name)==0)
        {
            return return_value(type,4,8);
        }
        else if(strcmp("port3B",info_name)==0)
        {
            return return_value(type,5,8);
        }
        else if(strcmp("port4A",info_name)==0)
        {
            return return_value(type,6,8);
        }
        else if(strcmp("port4B",info_name)==0)
        {
            return return_value(type,7,8);
        }
        else if(strcmp("carrier0",info_name)==0)
        {
            return return_value(type,16,24);
        }
        else if(strcmp("carrier1",info_name)==0)
        {
            return return_value(type,17,24);
        }
        else if(strcmp("carrier2",info_name)==0)
        {
            return return_value(type,18,24);
        }
        else if(strcmp("carrier3",info_name)==0)
        {
            return return_value(type,19,24);
        }
        else if(strcmp("carrier4",info_name)==0)
        {
            return return_value(type,20,24);
        }
        else if(strcmp("carrier5",info_name)==0)
        {
            return return_value(type,21,24);
        }
        else if(strcmp("carrier6",info_name)==0)
        {
            return return_value(type,22,24);
        }
        else if(strcmp("carrier7",info_name)==0)
        {
            return return_value(type,23,24);
        }
        else if(strcmp("carrier8",info_name)==0)
        {
            return return_value(type,24,24);
        }
        else if(strcmp("carrier9",info_name)==0)
        {
            return return_value(type,25,24);
        }
        else if(strcmp("carrier10",info_name)==0)
        {
            return return_value(type,26,24);
        }
        else if(strcmp("carrier11",info_name)==0)
        {
            return return_value(type,27,24);
        }
        else if(strcmp("carrier12",info_name)==0)
        {
            return return_value(type,28,24);
        }
        else if(strcmp("carrier13",info_name)==0)
        {
            return return_value(type,29,24);
        }
        else if(strcmp("carrier14",info_name)==0)
        {
            return return_value(type,30,24);
        }
        else if(strcmp("carrier15",info_name)==0)
        {
            return return_value(type,31,24);
        }
        else if(strcmp("carrier16",info_name)==0)
        {
            return return_value(type,32,24);
        }
        else if(strcmp("carrier17",info_name)==0)
        {
            return return_value(type,33,24);
        }
        else if(strcmp("carrier18",info_name)==0)
        {
            return return_value(type,34,24);
        }
        else if(strcmp("carrier19",info_name)==0)
        {
            return return_value(type,35,24);
        }
        else if(strcmp("carrier20",info_name)==0)
        {
            return return_value(type,36,24);
        }
        else if(strcmp("carrier21",info_name)==0)
        {
            return return_value(type,37,24);
        }
        else if(strcmp("carrier22",info_name)==0)
        {
            return return_value(type,38,24);
        }
        else if(strcmp("carrier23",info_name)==0)
        {
            return return_value(type,39,24);
        }
        else
        {
            return ERROR_INPUT;
        }
    }
}

int main(int argc, char *argv[])
{
    int next_option = 1;
    int node=-1;
    int mode=-1;
    int type=-1;
    short flag=0;
    char ver_info[128]={0};

    char node_name[16]={0};

    const char *const short_options = "m:t:n:";
    const struct option long_options[] = {
        {"help", 0, NULL, 'h'},
        {"mode", 1, NULL, 'm'},
        {"type", 1, NULL, 't'},
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
        case 'm':
            if(strcmp("AFU",optarg)==0)
                mode=AFU_CAPTURE;
            else if(strcmp("signal",optarg)==0)
                mode=SIGNAL_CAPTURE;
            else
                print_usage(stdout,ERROR_INPUT);
            break;
        case 't':
            if(strcmp("ul",optarg)==0)
                type=UPLINK_DATA;
            else if(strcmp("dl",optarg)==0)
                type=DOWNLINK_DATA;
            else
                print_usage(stdout,ERROR_INPUT);
            break;
        case 'n':
            sprintf(node_name,"%s",optarg);
            break;
        default:
            print_usage(stdout,ERROR_INPUT);
        }
    }

    node=transform_info(node_name,mode,type);

    if(mode!=-1 && type!=-1 && node!=ERROR_INPUT)
    {
        if(mode==AFU_CAPTURE)
        {
            start_grab(AFU_FILE_NAME,AFU_BASE,node,mode);
            sprintf(ver_info,"echo \"Capture_Mode:afu  Type:%s  Node:%s  Capture version: %02.02f\" >> %s\n",TYPE_DECIDE(type),node_name,CAP_VERSION,AFU_FILE_NAME);
        }
        else
        {
            start_grab(SIGNAL_FILE_NAME,SIGNAL_BASE,node,mode);
            sprintf(ver_info,"echo \"Capture_Mode:signal Type:%s  Node:%s  Capture version: %02.02f\" >> %s\n",TYPE_DECIDE(type),node_name,CAP_VERSION,SIGNAL_FILE_NAME);
        }
    }
    else
    {
        print_usage(stdout,ERROR_INPUT);
    }

    system(ver_info);

    return RETURN_SUCCESSFUL;
}