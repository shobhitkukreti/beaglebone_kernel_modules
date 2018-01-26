The file ehrpwm.c has the code to control EHRPW0B at P9_21.

The dts file is modified. The compatible string has been changed so that my custom
driver's probe function is called.

In brief, the tough part is to enable all the clks.

Starting from enabling clocks from the clock modules with CM_PERS

TI AM335X document is huge. Scanning through all the pages is tough

The Control Module registers have a base addresss of 0X44E10000.
At offset 0x664, there is pwmss_ctrl_register which effectively controls the 
PWM CLK input to PWMSS0 submodule.
Reserving this memory through request_mem_region.

Thereafter, PWMSS0 submodule has another registers called CLKCONFIG which
can further gate the clock to EHRPWM0. 
The driver of PWMSS0 has a function which is exported and can be called
from any module.

extern u16 pwmss_submodule_state_change(struct device *dev, int set)


Beaglebone Black kernel is not fully reliant on device tree. 

Some board specific code lies in arch/arm/mach-omap2



#define CM_PERS_REGBASE 0x44E00000
#define CM_PERS_REGLEN 0x400
#define PWMSS_BASE 0x48300000
#define PWMSS_REGLEN 0x10
#define CTRL_BASE (0X44E10000 + 0x664) /*Control Module */



