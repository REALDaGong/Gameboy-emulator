# 公告板

简单封装了一下。

俄罗斯方块玩不来，只出长条-_- 

halt指令有神奇bug

可能有其他的神奇bug

不想写了。。。等我闲的蛋疼再去改。。。。反正能玩坦克大战了ˊ_>ˋ

给gui开发的解释:

你只需要

1.将所有文件加进项目

2.在你的GUI代码中头部 #include "GB_Main.h"

你就可以用

GameBoyScreen[SCREEN_MAX_X][SCREEN_MAX_Y]    

存储的是颜色数据，0最浅，3最深，前面一个是x坐标，后一个是y坐标。以左上为原点，右为x正，下为y正，按照顺序输出即可

GameboyInit()

程序运行先调用，然后调用LoadRom，每次load之后都要调用。

LoadRom（参数）

传入string或char* 的路径，具体到 .gb文件的位置。 例C:/a/bbb/c.gb

KeyEvent(键,行为)

第一个是按了什么键(指gameboy上的ab上下左右等键)，第二个是按下还是抬起,你在捕获键盘事件后以相应的值来调用,具体值在这个文件的define里有.

RunANewFrame()

cpu运行，直到新的一帧生成后返回。
