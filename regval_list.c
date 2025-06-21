#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>

struct regval
{
    u8 reg;
    u8 val;
};

struct regval_list
{
    int pos;
    struct regval data;
    struct list_head list;
};

struct list_head head;
struct kobject *regval_kobject;

static void regval_list_init(struct list_head *list)
{
    INIT_LIST_HEAD(list);
}

static int regval_list_add(struct list_head *head, u8 reg, u8 val)
{
    int ret = 0;
    int pos = 0;
    struct regval_list *l;

    if (!list_empty(head))
    {
        l = list_first_entry(head, struct regval_list, list);
        pos = l->pos + 1;
    }

    l = kzalloc(sizeof(struct regval_list), GFP_KERNEL);
    if (!l)
    {
        printk(KERN_INFO "%s: Failed to add regval to list\n",
               module_name(THIS_MODULE));
        return -EACCES;
    }

    l->pos = pos;
    l->data.reg = reg;
    l->data.val = val;
    list_add(&l->list, head);

    return ret;
}

static void regval_list_delete(struct list_head *head, int pos)
{
    struct list_head *temp = NULL;
    if (list_empty(head))
        return;

    list_for_each(temp, head)
    {
        struct regval_list *list = list_entry(temp, struct regval_list, list);
        if (list->pos == pos)
        {
            list_del(temp);
            return;
        }
    }
}

static void regval_edit(struct list_head *head, int pos, int reg, int val)
{
    struct regval_list *temp = NULL;
    if (list_empty(head))
        return;
    list_for_each_entry_reverse(temp, head, list)
    {
        if (temp->pos == pos)
        {
            temp->data.reg = reg;
            temp->data.val = val;
            return;
        }
    }
}

static void regval_swap(struct list_head *head, int pos1, int pos2)
{
    struct list_head *temp = NULL;
    struct list_head *p1 = NULL;
    struct list_head *p2 = NULL;

    list_for_each(temp, head)
    {
        struct regval_list *list = list_entry(temp, struct regval_list, list);
        if (list->pos == pos1)
            p1 = temp;
        if (list->pos == pos2)
            p2 = temp;
    }

    if (p1 && p2) {
        list_swap(p1, p2);
    }
}

static ssize_t registers_show(struct kobject *kobj,
                              struct kobj_attribute *attr,
                              char *buf)
{
    ssize_t res = 0;
    int count = 0;
    struct regval_list *temp = NULL;

    list_for_each_entry_reverse(temp, &head, list)
    {
        res += sprintf(buf + res, "pos:%d, reg: 0x%02X, val: 0x%02X\n",
                       temp->pos, temp->data.reg, temp->data.val);
        count++;

        if (count >= 100)
            return sprintf(buf, "error\n");
    }

    return res;
}

static ssize_t registers_store(struct kobject *kobj, struct kobj_attribute *attr,
                               const char *buf, size_t count)
{
    int args = 0;
    char action;
    int reg;
    int val;
    int pos;

    args = sscanf(buf, "%c %x %x %d", &action, &reg, &val, &pos);

    if (args == 3 && action == 'e')
        regval_list_add(&head, reg, val);
    else if (args == 2 && action == 'd')
        regval_list_delete(&head, reg);
    else if (args == 4 && action == 'c')
        regval_edit(&head, pos, reg, val);
    else if (args == 3 && action == 's')
        regval_swap(&head, reg, val);

    return count;
}

static struct kobj_attribute registers_attribute =
    __ATTR(registers, 0600, registers_show,
           registers_store);

static struct attribute *dev_attrs[] = {
    &registers_attribute.attr,
    NULL,
};

static struct attribute_group dev_attr_grp = {
    .attrs = dev_attrs,
};

static int __init regval_list_start(void)
{
    int error = 0;

    printk(KERN_INFO "%s: Loading sysfs-regval\n", module_name(THIS_MODULE));
    regval_list_init(&head);
    printk(KERN_INFO "%s: Created list\n", module_name(THIS_MODULE));
    regval_list_add(&head, 0x35, 0x21);
    regval_list_add(&head, 0x45, 0x4A);
    regval_list_add(&head, 0x65, 0xF1);
    regval_list_add(&head, 0x67, 0x26);

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

static void __exit regval_list_end(void)
{
    kobject_put(regval_kobject);
}

module_init(regval_list_start);
module_exit(regval_list_end);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SERHII YAKOVENKO");
MODULE_DESCRIPTION("sysfs module example to change some register - value variable arrays");