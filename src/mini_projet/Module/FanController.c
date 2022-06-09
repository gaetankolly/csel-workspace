/* Project:	CSEL1-Miniproject
 *
 * Abstract: System programming -  file system
 *
 * Purpose:	Module for controlling the fan
 *
 * Autĥor:	Andrea Enrile and Gaëtan Kolly
 * Date:	09.06.2022
 */

#include <linux/cdev.h>             /* needed for char device driver      */
#include <linux/fs.h>               /* needed for device drivers          */
#include <linux/init.h>             /* needed for macros                  */
#include <linux/kernel.h>           /* needed for debugging               */
#include <linux/module.h>           /* needed by all modules              */
#include <linux/moduleparam.h>      /* needed for module parameters       */
#include <linux/uaccess.h>          /* needed to copy data to/from user   */
#include <linux/string.h>           /* needed for string operations       */
#include <linux/device.h>           /* needed for sysfs                   */
#include <linux/platform_device.h>  /* needed for sysfs                   */
#include <linux/timer.h>            /* needed for timer operations        */
#include <linux/jiffies.h>          /* needed for timer operations        */
#include <linux/kthread.h>          /* needed for thread operations       */
#include <linux/wait.h>             /* needed for wait operations         */
#include <linux/delay.h>            /* needed for wait operations         */
#include <linux/gpio.h>             /* needed for GPIO operations         */
#include <linux/thermal.h>          /* needed for thermal operations      */

/* ------------------------------- Defines ------------------------------- */

#define GPIO_LED    (10)
#define PWM_PERIOD  (1000)
#define PWM_TIMER   (PWM_PERIOD / 2) // PWM_TIMER / pwmFreq value -> new frequency
#define TEMP_TIMER  (1000)         // time in ms that need to be waited by the timer

/* --------------------------- queue and flags --------------------------- */

DECLARE_WAIT_QUEUE_HEAD(queue);
static int pwm_flag = 0;
static int temp_flag = 0;

/* ------------------------------ Attributs ------------------------------ */

static int mode = 0;        // if 0 -> automatic, if 1 -> manual
static int pwmFreq = 5;
static int temp = 0;

/* -------------------------- Attributs Operations ------------------------- */
/*------------------------------ mode attribut ----------------------------- */

ssize_t mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  sprintf(buf, "%d\n", mode);
  return strlen(buf);
}

ssize_t mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  int ret = -EINVAL; // init to error value
  ret = kstrtoint(buf, 0, &mode);
  if(ret != 0){
    pr_info("Error on string to in conversion in mode_store\n");
    return ret;
  }

  // Check if the mode is 1, if not set it to 0 (all values except for 1 will set automatic mode)
  if (mode == 1){
    mode = 1;
  }
  else{
    mode = 0;
  }
  pr_info("Mode value changed to %d\n", mode);
  return count;
}

DEVICE_ATTR_RW(mode); // __ATTR_RW(name): assumes default name_show, name_store and setting mode to 0644.

/*---------------------------- pwmFreq attribut ---------------------------- */

ssize_t pwmFreq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  sprintf(buf, "%d\n", pwmFreq);
  return strlen(buf);
}

ssize_t pwmFreq_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  int ret = -EINVAL; // init to error value
  ret = kstrtoint(buf, 0, &pwmFreq);
  if (ret != 0){
    pr_info("Error on string to in conversion in mode_store\n");
    return ret;
  }

  // If the given pwmFreq value is bigger than 20, the value is forced to 20
  if (pwmFreq > 20)
  {
    pwmFreq = 20;
  }

  // Wake-up thread
  pwm_flag = 1;
  wake_up_interruptible(&queue);

  pr_info("PwmFreq value changed to %d\n", pwmFreq);

  return count;
}

DEVICE_ATTR_RW(pwmFreq); // __ATTR_RW(name): assumes default name_show, name_store and setting mode to 0644.

/*------------------------------ temp attribut ----------------------------- */

ssize_t temp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  sprintf(buf, "%d\n", temp);
  return strlen(buf);
}

ssize_t temp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  // Nothing to do as the temperature is read from a sensor
  return count;
}

DEVICE_ATTR_RW(temp); // __ATTR_RW(name): assumes default name_show, name_store and setting mode to 0644.

/* ------------------------------- Attr group ------------------------------ */

static struct attribute *dev_attrs[] = {
  &dev_attr_mode.attr,
  &dev_attr_pwmFreq.attr,
  &dev_attr_temp.attr,
  NULL,
};

// ATTRIBUTE_GROUPS(NAME): This macro will create a attr group called NAME_groups and if fed
// to the device->groups field it should create and remove the file for the above attr automatically
ATTRIBUTE_GROUPS(dev);

/* ---------------------------- platform_device ---------------------------- */

static void sysfs_dev_release(struct device *dev){
  pr_info("FanControllerDriver: device released\n");
}

static struct platform_device fan_controller_device = {
  .name = "FanControllerDriver",
  .id = -1,
  .dev.release = sysfs_dev_release,
  .dev.groups = dev_groups
  };

/* --------------------------------- Timers -------------------------------- */

static struct timer_list temp_timer;
void temp_timer_callback(struct timer_list *data)
{
  // Wake-up thread
  temp_flag = 1;
  wake_up_interruptible(&queue);
}

static struct timer_list pwm_timer;
void pwm_timer_callback(struct timer_list *data)
{
  static int led_state = 0;
  // Set LED value and toggle it
  gpio_set_value(GPIO_LED, led_state);
  led_state ^= 1;

  // restart timer
  mod_timer(&pwm_timer, jiffies + (msecs_to_jiffies(PWM_TIMER / pwmFreq)));
}

/* -------------------------------- Threads -------------------------------- */

static struct task_struct *temp_task;
static struct thermal_zone_device *thermal_zone;

int temp_thread(void *data){

  // Temperature check will be based on a timer of 1 second
  timer_setup(&temp_timer, temp_timer_callback, 0);
  mod_timer(&temp_timer, jiffies + msecs_to_jiffies(TEMP_TIMER));

  while (!kthread_should_stop()){

    int res = -ERESTARTSYS;

    res = wait_event_interruptible(queue, temp_flag);

    if (res != 0){
      pr_info("Wake-up, but no condition were true\n");
    }else{
      // check if the system is in manual mode
      if (!mode){
        // restart temp timer
        mod_timer(&temp_timer, jiffies + msecs_to_jiffies(TEMP_TIMER));

        // Get CPU temperature
        if (thermal_zone_get_temp(thermal_zone, &temp)){
          pr_info("failed to get temp\n");
          pr_info("mode value is %d\n", mode);
        }
        else
        {
          // Set pwmFreq based on temperature
          if (temp < 35000){
            pwmFreq = 2;
          }
          else if (temp < 40000){
            pwmFreq = 5;
          }
          else if (temp < 45000){
            pwmFreq = 10;
          }
          else{
            pwmFreq = 20;
          }
        }
      }
    }
  }

  // Free resources
  del_timer(&temp_timer);
  
  return 0;
}

static struct task_struct *fan_management_task;

int fan_management_thread(void *data){

  // Software PWM generated via a timer  (pwm = 20 -> timer = 50 ms)
  timer_setup(&pwm_timer, pwm_timer_callback, 0);
  mod_timer(&pwm_timer, jiffies + (msecs_to_jiffies(PWM_TIMER / pwmFreq)));

  while (!kthread_should_stop()){

    int res = -ERESTARTSYS;

    res = wait_event_interruptible(queue, pwm_flag);

    if (res != 0){
      pr_info("Wake-up, but no condition were true\n");
    }
    else{
      if(pwm_flag){
        // Reset pwm flag
        pwm_flag = 0;
        // If the system is in manual mode reset manually the timer
        if (mode){
          mod_timer(&pwm_timer, jiffies + (msecs_to_jiffies(PWM_TIMER / pwmFreq)));
        }
      }
    }
 }

  // Free resources
  del_timer(&pwm_timer);

  return 0;
}

/* ---------------------------------- Init --------------------------------- */

static int __init fan_controller_init(void)
{
  int status;

  // Request the LED used to simulate the fan
  if (gpio_request(GPIO_LED, "Fan_Simulator_LED")){
    pr_info("failed to request GPIO_LED\n");
    return 1;
  }
  // Configure the GPIO_LED as output
  if (gpio_direction_output(GPIO_LED, 0)){
    pr_info("failed to set GPIO_LED as output\n");
    return 1;
  }

  // Get CPU thermal zone
  thermal_zone = thermal_zone_get_zone_by_name("cpu-thermal");
  if (!thermal_zone){
    pr_info("failed to get cpu thermal zone\n");
    return 1;
  }

  temp_task = kthread_run(temp_thread, 0, "s/temp_thread");
  fan_management_task = kthread_run(fan_management_thread, 0, "s/fan_management_thread");

  // Install device in sysfs
  status = platform_device_register(&fan_controller_device);
  if (status != 0){
    pr_info("Error registering the device\n");
    return status;
  }

  pr_info("Fan controller module loaded\n");
  return 0;
}

/* ---------------------------------- Exit --------------------------------- */

static void __exit fan_controller_exit(void)
{
  // Stop threads
  kthread_stop(temp_task);
  kthread_stop(fan_management_task);

  gpio_free(GPIO_LED);

  // Unregister device
  platform_device_unregister(&fan_controller_device);

  pr_info("Fan controller module unloaded\n");
}

module_init(fan_controller_init);
module_exit(fan_controller_exit);

MODULE_AUTHOR("Andrea Enrile, Gaëtan Kolly");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");