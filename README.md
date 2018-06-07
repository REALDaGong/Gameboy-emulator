# 代码文档

整份代码主要分为4个部分 ，CPU,MMU,GPU,Timer,keypress(包含在Memory里)

## CPU

CPU是程序执行的核心，使用一次step方法后，将执行下面的步骤

1.从_REG.PC位置的存储器中取出指令

2.执行对应的指令

3.为Timer和GPU增加时间计数

4.检查中断是否发生，如发生则处理中断

指令将会通过MMU更新存储器的数值或者更新_REG内寄存器的数值。

## Memory

MMU管理所有的存储器。

MemoryRead 传入一个地址，返回一个8位无符号整型的该地址内的值，

MemoryWrite 传入一个地址和一个8位无符号整型，修改相应地址的值。

合法地址的范围为[0x0,0xFFFF]，但不包括[FEA0,FF00) 及 [FF80,FFA0)，不要传入不合法的地址。

你不可以访问CPU寄存器。

## GPU

GPU管理图像的处理和输出。

最终的输出结果在_Screen[SCREEN_MAX_X][SCREEN_MAX_Y]中。
			   160            144
_Window和_Sprite 分别存放窗口层和精灵层，窗口层与主显示层同步更新，精灵层在前两者全部更新完成后更新，更新完毕之后重叠，生成完整的新一帧。

其中的数据是8位无符号整型的0,1,2,3 代表了黑，深灰，浅灰，白 四种颜色。

每接收到458个时钟单位后更新一个横行的背景和窗口的数据，共计更新144个横行之后更新所有的精灵数据，此时经过了65952个时钟单位，最后4580个时钟单位内不作任何操作。

更新完毕所有的精灵数据后NewFrame标志会被设为1,使用GetNewFrameFlag检查它的值，SetNewFrameFlag(值)来重设它的值，这一值不会自动重置，应当输出帧后手动处理。

## Timer

Timer管理整个gameboy的时间计数。

keypress被集合在Memory文件里，其接口被写在GB_Main中，在按下和释放按键时调用KeyEvent函数，并传入正确的值即可，为了便于记忆，我将值定义了宏，见GB_Main的开头部分。

我在GB_Main中定义了GBCore中的全局变量cpu,gpu,memory和timer，下面接口的实现都依赖这些变量，如果你不想使用任何的全局变量，你需要重写这些接口，具体重写方法仍然参照我在GB_Main中的实例。

你如果真的要重写，创建新对象时要严格按照我的顺序创建，因为这些类是互相依赖的。

你如果想直接使用这些接口，在gui的实现文件中,include GB_Main.h。

## 对于GBMain的解释：

GameBoyScreen数组 其实就是gpu._Screen换了个名字，显得不那么丑。

GameBoyInit 初始化所有组件的状态。

RunANewFrame 调用这个函数后，cpu将运行到gpu._Screen一次完全更新完毕的时间点后返回。

LoadRom 传入一个string或char*的路径，载入这个文件，你在每次载入之后都要初始化一次（GameBoyInit）

KeyEvent 处理按键。

# 还有不懂的及时在qq上与我联系。
