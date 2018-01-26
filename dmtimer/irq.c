/* Programming TI Sitara processor's DMTIMER 7 to fire interrupt every 1 second
 * Clock Freq: 24Mhz 
 * IRQ CHIP and Threaded IRQ Handling
 * HW Platform: Beaglebone Black
 * Author: Shobhit Kukreti
 * File: irq.c
 */


#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/interrupt.h>


struct bb_irq_chip {
	struct irq_chip chip;
	unsigned int enabledMask;
	unsigned int base;

}timer_irq_chip;


static void dmtimer_irqmask(struct irq_data *d) 
{
	struct irq_chip *chip = irq_data_get_irq_chip(d);
	struct bb_irq_chip *tmp_irq_chip = container_of(chip, struct bb_irq_chip, chip);
	tmp_irq_chip->enabledMask &= ~(1<<(d->irq - timer_irq_chip.base));
}


static void dmtimer_irqunmask (struct irq_data *d)
{
	struct irq_chip *chip = irq_data_get_irq_chip(d);
	struct bb_irq_chip *tmp_irq_chip = container_of(chip, struct bb_irq_chip, chip);
	tmp_irq_chip->enabledMask |= ~(1<<(d->irq - timer_irq_chip.base));

}

static void dmtimer_irq_alloc_chip (void) 
{
		int i=0;
		timer_irq_chip.base = irq_alloc_descs(-1, 0, 4, 0);
		if(timer_irq_chip.base < 0) {
				pr_err("%s, irq_alloc_descs failed\n", __func__);
				return NULL;
		}

		timer_irq_chip.chip.name = "DMTIMER Generic IRQ CHIP";
		timer_irq_chip.chip.irq_mask = &dmtimer_irqmask;
		timer_irq_chip.chip.irq_unmask = &dmtimer_irqunmask;

		for (i=0; i <4; i++) {
				irq_set_chip(timer_irq_chip.base + i, &timer_irq_chip.chip);
				irq_set_handler(timer_irq_chip.base + i , &handle_simple_irq);
				irq_modify_status(timer_irq_chip.base + i, IRQ_NOREQUEST | IRQ_NOAUTOEN, IRQ_NOPROBE);
		}

}

void timer_irq_free_chip (void)
{
	irq_free_descs(timer_irq_chip.base, 4);
	
}


int timer_irq_init(void)
{
	dmtimer_irq_alloc_chip();

	return 0;
}

int timer_get_irq (int nested_irq_num)
{

	return timer_irq_chip.base + nested_irq_num;
}
