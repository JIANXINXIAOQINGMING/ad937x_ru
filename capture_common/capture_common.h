#ifndef _CAPTRUE_COMMON_H
#define _CAPTRUE_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#define CAPTURE_RE_BIT                              0x1
#define CAPTURE_TOTAL_BIT                           0x2
#define CAPTURE_SELECT_BIT                          0x4
#define CAPTURE_RATE_BIT                            0x8
#define ONLY_READ_BIT                               0x10

#define SIGNAL_CAPTURE_RE                           0x0
#define SIGNAL_CAPTURE_BUSY                         0x1
#define SIGNAL_CAPTURE_SELECT                       0x2
#define SIGNAL_CAPTURE_RATE                         0x3
#define SIGNAL_CAPTURE_ADDR                         0x4
#define SIGNAL_CAPTURE_DATAOUT                      0x5

#define BIT_COMPUTE(x,y)                            x | y
#define RE_BIT_COMPUTE(x)                           BIT_COMPUTE(x,CAPTURE_RE_BIT)
#define TOTAL_BIT_COMPUTE(x)                        BIT_COMPUTE(x,CAPTURE_TOTAL_BIT)
#define SELECT_BIT_COMPUTE(x)                       BIT_COMPUTE(x,CAPTURE_SELECT_BIT)
#define RATE_BIT_COMPUTE(x)                         BIT_COMPUTE(x,CAPTURE_RATE_BIT)
#define ONLY_READ_BIT_COMPUTE(x)                    BIT_COMPUTE(x,ONLY_READ_BIT)

#define BIT_DECIDE(x,y)                             (x & y)!=0
#define RE_BIT_DECIDE(x)                            BIT_DECIDE(x,CAPTURE_RE_BIT)
#define TOTAL_BIT_DECIDE(x)                         BIT_DECIDE(x,CAPTURE_TOTAL_BIT)
#define SELECT_BIT_DECIDE(x)                        BIT_DECIDE(x,CAPTURE_SELECT_BIT)
#define RATE_BIT_DECIDE(x)                          BIT_DECIDE(x,CAPTURE_RATE_BIT)
#define ONLY_READ_BIT_DECIDE(x)                     BIT_DECIDE(x,ONLY_READ_BIT)

#ifdef __cplusplus
}
#endif
#endif  //_COMMON_INTERFACE_H