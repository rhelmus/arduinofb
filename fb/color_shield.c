/*
 * Copyright (C) 2012, Rick Helmus <rhelmus_at_gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive for
 * more details.
 *
 * This file is loosely based on the nokia 6100 lcd driver by Alexander Kudjashev
 * See http://www.at91.com/samphpbb/viewtopic.php?f=12&t=5103&sid=90a754e99893272129ac0bc08174a401
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>


/* lcd resolution */
#define X_RES       130
#define Y_RES       130
#define B_PP        16

#define MEM_LEN     (X_RES * Y_RES * B_PP / 8)


struct color_shield_par
{
    u8                  *buffer;
    u8                  *screen;
    struct task_struct  *color_shield_thread_task;
};

static struct fb_fix_screeninfo color_shield_fix __devinitdata = {
    .id             = "color_shield",
    .type           = FB_TYPE_PACKED_PIXELS,
    .visual         = FB_VISUAL_TRUECOLOR,
    .xpanstep       = 1,
    .ypanstep       = 1,
    .ywrapstep      = 1,
    .line_length    = X_RES * B_PP / 8,
    .accel          = FB_ACCEL_NONE,
};

static struct fb_var_screeninfo color_shield_var __devinitdata = {
    .xres           = X_RES,
    .yres           = Y_RES,
    .xres_virtual   = X_RES,
    .yres_virtual   = Y_RES,
    .bits_per_pixel = B_PP,
    .red            = { 11, 5, 0 },
    .green          = { 5, 6, 0 },
    .blue           = { 0, 5, 0 },
    .nonstd         = 0,
};

static int color_shield_blank(int blank_mode, struct fb_info *info)
{
    // UNDONE
#if 0
    struct color_shield_par *par = info->par;
    
    switch (blank_mode) {
        case FB_BLANK_UNBLANK:
            spi_command(par->spi, DISPON);
            break;
        case FB_BLANK_NORMAL:
        case FB_BLANK_VSYNC_SUSPEND:
        case FB_BLANK_HSYNC_SUSPEND:
        case FB_BLANK_POWERDOWN:
            spi_command(par->spi, DISPOFF);
            break;
        default:
            return -EINVAL;
    }
#endif
    
    return 0;
}

static struct fb_ops color_shield_ops = {
    .owner          = THIS_MODULE,
    .fb_read        = fb_sys_read,
    .fb_write       = fb_sys_write,
    .fb_fillrect    = sys_fillrect,
    .fb_copyarea    = sys_copyarea,
    .fb_imageblit   = sys_imageblit,
    .fb_blank       = color_shield_blank,
};

static int __devinit color_shield_probe(struct platform_device *dev)
{
    struct fb_info *info;
    int retval;
    struct color_shield_par *par;
       
    retval = -ENOMEM;
    
    info = framebuffer_alloc(sizeof(struct color_shield_par), &dev->dev);
    if(!info)
        return retval;
    
    info->screen_base = alloc_pages_exact(MEM_LEN, GFP_DMA | __GFP_ZERO);
    if(!info->screen_base)
        goto err;
    
    info->fbops = &color_shield_ops;
    info->var = color_shield_var;
    info->fix = color_shield_fix;
    
    info->fix.smem_len = MEM_LEN;
    info->fix.smem_start = (unsigned long)virt_to_phys(info->screen_base);
    info->screen_size = info->fix.smem_len;
    info->flags = FBINFO_FLAG_DEFAULT;
    
    par = info->par;
    par->buffer = info->screen_base;
    
    par->screen = kmalloc(MEM_LEN * 2, GFP_KERNEL | GFP_DMA);
    if(!par->screen)
        goto err1;
    memset(par->screen, 0xff, MEM_LEN * 2);
    
    retval = register_framebuffer(info);
    if (retval < 0)
        goto err2;
    
    platform_set_drvdata(dev, info);

    printk(KERN_INFO "fb%d: %s frame buffer device, %dK of video memory\n",
           info->node, info->fix.id, info->fix.smem_len >> 10);
    
    return 0;
    
    err2:
    kfree(par->screen);
    err1:
    free_pages_exact(par->buffer, MEM_LEN);
    err:
    framebuffer_release(info);
    platform_set_drvdata(dev, 0);
    
    return retval;
}

static int __devexit color_shield_remove(struct platform_device *dev)
{
    struct fb_info *info = platform_get_drvdata(dev);

    printk(KERN_INFO "color_shield: remove\n");
    
    if (info) {
        struct color_shield_par *par = info->par;        
        unregister_framebuffer(info);
        free_pages_exact(info->screen_base, MEM_LEN);
        kfree(par->screen);
        framebuffer_release(info);
    }
    return 0;
}

static struct platform_driver color_shield_driver = {
    .probe  = color_shield_probe,
    .remove = __devexit_p(color_shield_remove),
    .driver = {
        .name   = "color_shield",
    },
};

static struct platform_device *color_shield_device = 0;

static int __init color_shield_init(void)
{
    int ret;
    
    printk("arduino color shield fb driver\n");
        
    // UNDONE
    /*if (!arcfb_enable)
        return -ENXIO;*/
    
    ret = platform_driver_register(&color_shield_driver);
    if (!ret) {
        color_shield_device = platform_device_alloc("color_shield", 0);
        if (color_shield_device) {
            ret = platform_device_add(color_shield_device);
        } else {
            ret = -ENOMEM;
        }
        if (ret) {
            platform_device_put(color_shield_device);
            platform_driver_unregister(&color_shield_driver);
        }
    }
    else
        printk(KERN_WARNING "color_shield: Failed to register platform driver\n");
    
    return ret;
}

static void __exit color_shield_exit(void)
{
    platform_device_unregister(color_shield_device);
    platform_driver_unregister(&color_shield_driver);
}

module_init(color_shield_init);
module_exit(color_shield_exit);

MODULE_AUTHOR("Rick Helmus");
MODULE_DESCRIPTION("fb driver for arduino with the Sparkfun color lcd shield");
MODULE_LICENSE("GPL");
