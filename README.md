## Kernel Modules testing Beaglebone Peripherals

1)	dmtimer : Module to create interrupts based on DMTIMER. Uses Threaded IRQ and a Software IRQ_CHIP to manage interrupts.
2)	ehrpwm  : Module to create a PWM waveform to drive a GPIO.
3)  mmap	: Char Driver which creates a memory space accessible to user via mmap.
4)  mmap-app: 'C' Application to open and mmap the buffer created by the MMAP char driver

timer_dts.patch : Patches the DTS file so that my Platform Driver Code for DMTIMER is invoked