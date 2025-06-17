# sysfs regval

This is example for changing registers values for device driver.

## In production

Change
```
static struct kobj_attribute registers_attribute = __ATTR(registers, 0600, registers_show,
                                                          registers_store);
```
to
```
static DEVICE_ATTR(registers, 0600,
		   registers_show, registers_store);
```

There also no need to create kobject.
```
&client->dev.kobj
```
where dev is device from yours driver.

In this case i2c_client