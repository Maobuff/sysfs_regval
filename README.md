# sysfs regval

This is example for changing registers values for device driver.

## Example
Module will create kernel object in /sys/kernel/regval named registers

Its possible to show (cat) registers
```
# cat registers
pos:0, reg: 0x35, val: 0x21
pos:1, reg: 0x45, val: 0x4A
pos:2, reg: 0x65, val: 0xF1
pos:3, reg: 0x67, val: 0x26
```
add to the end
```
# echo "e 0x55 0xAA" > registers
# cat registers
pos:0, reg: 0x35, val: 0x21
pos:1, reg: 0x45, val: 0x4A
pos:2, reg: 0x65, val: 0xF1
pos:3, reg: 0x67, val: 0x26
pos:4, reg: 0x55, val: 0xAA
```
delete
```
# echo "d 2" > registers
# cat registers 
pos:0, reg: 0x35, val: 0x21
pos:1, reg: 0x45, val: 0x4A
pos:3, reg: 0x67, val: 0x26
pos:4, reg: 0x55, val: 0xAA
```
swap
```
# echo "s 3 1" > registers
# cat registers 
pos:0, reg: 0x35, val: 0x21
pos:3, reg: 0x67, val: 0x26
pos:1, reg: 0x45, val: 0x4A
pos:4, reg: 0x55, val: 0xAA
```
change
```
# echo "c 0x78 0x21 1" > registers
# cat registers
pos:0, reg: 0x01, val: 0x78
pos:3, reg: 0x67, val: 0x26
pos:1, reg: 0x78, val: 0x21
pos:4, reg: 0x55, val: 0xAA
```

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