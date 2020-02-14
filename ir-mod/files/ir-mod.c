#include <linux/module.h>   
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>

#define IR_OUTPUT_PIN 	18
#define IR_INPUT_PIN 	17
#define MY_GPIO_INT_NAME "ir_mod_int"
#define MY_DEV_NAME "ir_mod_device"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nobody.com");
MODULE_DESCRIPTION("IR Device Driver module");

// This directory entry will point to `/sys/kernel/debug/ir`.
static struct dentry *dir = 0;

static short int ir_irq = 0;
static struct timeval lasttv = { 0, 0 };
static int sense = 1;
static long slist[1024];
static int sdlist[1024];
static int count=0; 
static int ir_status=0;

static int dump_write_op(void *data, u64 value)
{
	int i;
	int  j=0;
		
	for(i=0;i<count;i++)
	{
		if(slist[i] > 10000 )
			printk("................[%d]\n",j++);
		if(sdlist[i]==1)
			printk("[%2d][+]%ld\n",i,slist[i]);
		else
			printk("[%2d][-]%ld\n",i,slist[i]);
		if(i>0 && sdlist[i]==sdlist[i-1])
		{
			
			printk("------break: count=%d/%d------\n",i,count);			
			goto err;
		}		
	}
	printk("------done: count=%d------\n",count);
err:
	if(value==0)
		count=0;
	ir_status=count;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(dump_fops, NULL, dump_write_op, "%llu\n");

#define PULSE_MASK      0x00FFFFFF
static irqreturn_t ir_isr(int irq, void *data0)
{
	struct timeval tv;
	long deltv;
	long data2;
	int signal;

	/* use the GPIO signal level */
	signal = gpio_get_value(IR_INPUT_PIN);
																																																																																																																																																																																																																																																							
	if (sense != -1) {
		/* get current time */
		do_gettimeofday(&tv);

		/* calc time since last interrupt in microseconds */
		deltv = tv.tv_sec-lasttv.tv_sec;
		if (tv.tv_sec < lasttv.tv_sec ||
		    (tv.tv_sec == lasttv.tv_sec &&
		     tv.tv_usec < lasttv.tv_usec)) {
			printk(": AIEEEE: your clock just jumped backwards\n");
			printk(": %d %d %lx %lx %lx %lx\n", signal, sense,
			       tv.tv_sec, lasttv.tv_sec,
			       tv.tv_usec, lasttv.tv_usec);
			data2 = PULSE_MASK;
		} else if (deltv > 15) {
			data2 = PULSE_MASK; /* really long time */
			if (!(signal^sense)) {
				/* sanity check */
				printk(": AIEEEE: %d %d %lx %lx %lx %lx\n",
				       signal, sense, tv.tv_sec, lasttv.tv_sec,
				       tv.tv_usec, lasttv.tv_usec);
				/*
				 * detecting pulse while this
				 * MUST be a space!
				 */
				sense = sense ? 0 : 1;
			}
		} else {
			data2 = (int) (deltv*1000000 +
				      (tv.tv_usec - lasttv.tv_usec));

			/* debounce */
			if(data2 < 20)
				return IRQ_HANDLED;
			
			/* save IR signal */
			if(signal != -1)
			{
				if(count>=1024)
					count=0;
				slist[count]=data2;
				sdlist[count]=!signal;
				gpio_set_value(IR_OUTPUT_PIN, signal);
				count++;
			}	
		}
		lasttv = tv;		
	}

	return IRQ_HANDLED;
}

int init_module(void)
{
	struct dentry *junk;

    dir = debugfs_create_dir("ir", 0);
    if (!dir) {
        // Abort module load.
        printk(KERN_ALERT "debugfs_ir: failed to create /sys/kernel/debug/ir\n");
        return -1;
    }
    junk = debugfs_create_file(
            "dump",
            0222,
            dir,
            NULL,
            &dump_fops);
    if (!junk) {
        // Abort module load.
        printk(KERN_ALERT "debugfs_ir: failed to create /sys/kernel/debug/ir/dump\n");
        return -1;
    }
    junk = debugfs_create_u32("ir_status", 0444, dir, &ir_status);
    if (!junk) {
        // Abort module load.
        printk(KERN_ALERT "debugfs_ir: failed to create /sys/kernel/debug/ir/ir_status\n");
        return -1;
    }

	// -- setup the led gpio as output
	printk("%s: %s .\n",__FILE__,__func__);
	if(gpio_is_valid(IR_OUTPUT_PIN) < 0) return -1;
	if(gpio_request(IR_OUTPUT_PIN, "IR_OUTPUT_PIN") < 0) return -1;
	gpio_direction_output(IR_OUTPUT_PIN, 0 );

	// -- setup the button gpio as input and request irq
	if(gpio_request(IR_INPUT_PIN,"IR_INPUT_PIN") < 0) return -1;
	if(gpio_is_valid(IR_INPUT_PIN) < 0) return -1;
	if( (ir_irq = gpio_to_irq(IR_INPUT_PIN)) < 0 )  return -1;

	request_irq(ir_irq,
			     (irq_handler_t) ir_isr,
			     IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING,
			     MY_GPIO_INT_NAME, MY_DEV_NAME);
	return 0;

}


void cleanup_module(void)
{
	debugfs_remove_recursive(dir);
	gpio_set_value(IR_OUTPUT_PIN, 0);
	gpio_free(IR_OUTPUT_PIN);
	free_irq(ir_irq, MY_DEV_NAME);
	gpio_free(IR_INPUT_PIN);
}

