/* Defined as offset from gpio base address (GPIO_PA_BASE) = 0x4000600
 * Do not hardcode base addresses of I/O registers or IRQ numbers.
 * This information is known by the platform device.
 */

#define GPIO_PC 		0x48

#define GPIO_PC_MODEL	(GPIO_PC + 0x04)
#define GPIO_PC_DOUT    (GPIO_PC + 0x0c)
#define GPIO_PC_DIN     (GPIO_PC + 0x1c)

#define GPIO_EXTIPSELL 	0x100
#define GPIO_EXTIRISE  	0x108
#define GPIO_EXTIFALL  	0x10c
#define GPIO_IEN       	0x110
#define GPIO_IFC       	0x11c
#define GPIO_IF        	0x114