#include "GameMain.h"
#include <stdio.h>
#include <time.h>

// 关闭
#define OP_CLOSE        0x00
// 有效的端口
#define OP_ACTIVE_PORT  0x01
// 发送数据
#define OP_SEND_DATA    0x02
// 发送子弹
#define OP_SEND_FIRE    0x03
// 发送命中鱼
#define OP_SEND_FISH    0x04

// 设置反汇编DLL路径
#define OP_SETDSDLLPATH 0x10