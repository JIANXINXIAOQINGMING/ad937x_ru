#ifndef _COMMON_INTERFACE_H
#define _COMMON_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define DEVMEM_MAP_SIZE 4096
#define DEVMEM_MAP_MASK (off_t)(DEVMEM_MAP_SIZE - 1)

#define RETURN_SUCCESSFUL       0
#define ERROR_INPUT             -2
#define ERROR_DATA_EMPTY        -3
#define ERROR_STRUCT_APPLY      -4
#define ERROR_OPEN_DEV          -5
#define ERROR_WR                -6
#define ERROR_SET               -7

#ifdef __cplusplus
}
#endif
#endif  //_COMMON_INTERFACE_H