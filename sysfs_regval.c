#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

struct regval
{
    u16 addr;
    u8 val;
};

static struct regval registers[] = {
    {0x01, 0x0A},
    {0x02, 0x09},
    {0x03, 0x08},
    {0x04, 0x07},
    {0x05, 0x06},
    {0x06, 0x05},
    {0x07, 0x04},
    {0x08, 0x03},
    {0x09, 0x02},
    {0x0A, 0x01},
};

struct kobject *regval_kobject;

static ssize_t registers_show(struct kobject *kobj, struct kobj_attribute *attr,
                              char *buf)
{
    int res = 0;

    for (int i = 0; i < ARRAY_SIZE(registers); i++)
    {
        res += sprintf(buf + res, "register: %02X: %02X\n", registers[i].addr, registers[i].val);
    }

    return res;
}

static ssize_t registers_store(struct kobject *kobj, struct kobj_attribute *attr,
                               const char *buf, size_t count)
{
    char *copy, *found;
    struct regval rv;
    int state = 0, res = 0;

    copy = kzalloc(sizeof(buf), GFP_KERNEL);
    if (!copy)
    {
        printk(KERN_INFO "%s: Failed to copy store buf\n", module_name(THIS_MODULE));
        return count;
    }

    strcpy(copy, buf);
    while ((found = strsep(&copy, " ")) != NULL)
    {
        if (state == 0)
        {
            res = kstrtou16(found, 0, &rv.addr);
            if (res != 0)
            {
                break;
            }
            state++;
        }
        else
        {
            res = kstrtou8(found, 0, &rv.val);
            if (res != 0)
            {
                break;
            }

            for (int i = 0; i < ARRAY_SIZE(registers); i++)
            {
                if (registers[i].addr == rv.addr)
                {
                    registers[i] = rv;
                    break;
                }
            }

            break;
        }
    }

    kfree(copy);
    return count;
}

static struct kobj_attribute registers_attribute = __ATTR(registers, 0600, registers_show,
                                                          registers_store);

static struct attribute *dev_attrs[] = {
	&registers_attribute.attr,
	NULL,
};

static struct attribute_group dev_attr_grp = {
    .attrs = dev_attrs,
};

static int __init hello_start(void)
{
    int error = 0;

    printk(KERN_INFO "%s: Loading sysfs-regval\n", module_name(THIS_MODULE));

    regval_kobject = kobject_create_and_add("regval", kernel_kobj);
    if (!regval_kobject)
        return -ENOMEM;

    error = sysfs_create_group(regval_kobject, &dev_attr_grp);
    if (error)
    {
        printk(KERN_INFO "%s: Failed to create registers in sysfs\n", module_name(THIS_MODULE));
    }

    return error;
}

static void __exit hello_end(void)
{
    kobject_put(regval_kobject);
}

module_init(hello_start);
module_exit(hello_end);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SERHII YAKOVENKO");
MODULE_DESCRIPTION("sysfs module example to change some register - value variable arrays");