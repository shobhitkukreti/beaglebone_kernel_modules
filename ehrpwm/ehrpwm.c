/* The code generates PWM Signal on a GPIO using EHRPWM Module
 * DTS disables ehrpwm by default 
 * Adding a platform device
 * Author: Shobhit Kukreti
 * Platform: Beaglebone Black
 * File: ehrpwm.c
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "pwm.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("shobhit kukreti");
MODULE_DESCRIPTION("Learn EHRPWM Peripheral");

#define PWM_IRQ 86
#define CM_PERS_REGBASE 0x44E00000
#define CM_PERS_REGLEN 0x400
#define PWMSS_BASE 0x48300000
#define PWMSS_REGLEN 0x10
#define CTRL_BASE (0X44E10000 + 0x664) /*Control Module */


/* Enable CLKCONFIG Register of PWMSS0. The driver is from TI 
with this function exported */
extern u16 pwmss_submodule_state_change(struct device *dev, int set);

struct pwmss_info {
	void __iomem *pwmss_base;
	struct mutex lock;
	u16 clkconfig;

};


struct ehrpwm_struct {
struct platform_device *pdev;
struct resource *r_mem;
void __iomem *pwm_base;
void __iomem *cm_pers_base;
void __iomem *pwmss_base;
void __iomem *ctrl;
unsigned int virq;
struct task_struct *my_thread;
}epwm;



static int read_pwm(void *data)
{
	while(epwm.pwm_base) {
		if(kthread_should_stop())
			break;
		pr_info("TBCNT: %x\n", ioread16(epwm.pwm_base +TBCNT));
		pr_info("TBCTL: %x\n", ioread16(epwm.pwm_base + TBCTL));
		pr_info("TBPRD: %x\n", ioread16(epwm.pwm_base + TBPRD));
		pr_info("CM_0: %x\n", ioread32(epwm.cm_pers_base));
		pr_info("CM_60: %x\n", ioread32(epwm.cm_pers_base + 0x60));
		pr_info("CM_D4: %x\n", ioread32(epwm.cm_pers_base + 0xD4));
		msleep(300);
		}
return 0;
}


static int  pwm_probe ( struct platform_device *pdev ) 
{
	uint16_t reg=0;
	uint32_t status = 0;
	struct pwmss_info *pwmss;
	printk(KERN_INFO"PWM_PROBE\n");

	epwm.r_mem = platform_get_resource (pdev, IORESOURCE_MEM, 0);
	pr_info("2\n");

	if(!epwm.r_mem) 
		pr_err("Resource MEM EHRPWM not available \n");


	/* Check if IO Memory is available for use by the driver */
	
	if(!(request_mem_region(epwm.r_mem->start, resource_size(epwm.r_mem),"SK-EHRPWM")))
	{
		pr_err ("%s, EHRPWM Memory  Used by Another Driver\n",__func__);
	
	}

		
	if(!(request_mem_region(CM_PERS_REGBASE, CM_PERS_REGLEN,"PWM-CLOCK-Enable")))
	{
		pr_err ("%s, CM PERS REG  Memory  Used by Another Driver\n",__func__);
	
	}

	if(!(request_mem_region(CTRL_BASE, 4 ,"Control_Module")))
	{
		pr_err ("%s, Control Module  Memory  Used by Another Driver\n",__func__);
	
	}

	epwm.ctrl = ioremap_nocache(CTRL_BASE, 4);

	epwm.pwm_base = ioremap_nocache(epwm.r_mem->start, resource_size(epwm.r_mem));
	epwm.cm_pers_base = ioremap_nocache(CM_PERS_REGBASE, CM_PERS_REGLEN);

	iowrite32(0x01, epwm.ctrl);

	pwmss = dev_get_drvdata(pdev->dev.parent);

	iowrite32(1<<8, pwmss->pwmss_base + 0x08); // PWM CLK EN
	iowrite32(1<<2, pwmss->pwmss_base + 0x04);
	status = ioread32(pwmss->pwmss_base + 0x0C);
	wmb();
	printk("CLK_Status:%x\n", status);
	status = ioread32(pwmss->pwmss_base + 0x4);
	wmb();	
	printk("SYSCONFIG:%x\n", status);
	iowrite32(0x02, epwm.cm_pers_base);
	iowrite32(0x02, epwm.cm_pers_base + 0x60);
	iowrite32(0x02, epwm.cm_pers_base + 0xD4);
	wmb();

	/* TBCTL Register */
	reg = 1;
	reg |= 7<<7;
	reg |= 7<<10; 
	
	printk(KERN_INFO "TBCTL :%x\n", reg);
	
	iowrite16(reg, epwm.pwm_base + TBCTL);
	iowrite16(0XFFFF, epwm.pwm_base + TBCNT);
	iowrite16(0xFFFF, epwm.pwm_base + TBPRD);
	iowrite16(0x00, epwm.pwm_base + CMPCTL); /* Default value */
	reg =0;
	iowrite16(0xFF00, epwm.pwm_base + CMPA);
	iowrite16(0xFF00, epwm.pwm_base +CMPB);
	
	/* Set Action Qualifier */
	reg =1;
	reg |= 3<<6;
	iowrite16(reg, epwm.pwm_base + AQCTLB);

	epwm.my_thread = kthread_run (read_pwm, (void *)NULL, "PWM0-REG Read");		

	printk(KERN_INFO"All is good \n");

	return 0;
}

static int pwm_remove (struct platform_device *pdev) 
{

iounmap(epwm.cm_pers_base);
iounmap(epwm.pwm_base);
iounmap(epwm.ctrl);
releaae_mem_region(CTRL_BASE, 4); 
release_mem_region(epwm.r_mem->start, resource_size(epwm.r_mem));
release_mem_region(CM_PERS_REGBASE, CM_PERS_REGLEN);
printk(KERN_INFO"SK,PWM-DEV Remove");

return 0;
}

static const struct of_device_id pwm_match[]={
        {.compatible = "sk,pwm-dev", },
        {},
};

MODULE_DEVICE_TABLE(of, pwm_match);


struct platform_driver pwm_driver = {

	.probe = &pwm_probe,
	.remove = &pwm_remove,
	.driver = {
		.name = "sk,pwm-dev",
		.of_match_table = of_match_ptr(pwm_match),
	
	},

};

static int __init pwm_init(void) {

        int ret=0;
	printk(KERN_INFO"PWM_INIT"); 
        ret=platform_driver_register(&pwm_driver);
        return ret;
}


static void __exit pwm_exit(void) 
{
	printk(KERN_INFO"PWM_EXIT\n");
	if(epwm.my_thread)
		kthread_stop(epwm.my_thread);

	platform_driver_unregister(&pwm_driver);


}

module_init(pwm_init);
module_exit(pwm_exit);
