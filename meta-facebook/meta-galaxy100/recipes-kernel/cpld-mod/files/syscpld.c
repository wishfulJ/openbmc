/*
 * syscpld.c - The i2c driver for SYSCPLD
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

//#define DEBUG

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/i2c.h>

#include <i2c_dev_sysfs.h>

#ifdef DEBUG

#define PP_DEBUG(fmt, ...) do {                   \
  printk(KERN_DEBUG "%s:%d " fmt "\n",            \
         __FUNCTION__, __LINE__, ##__VA_ARGS__);  \
} while (0)

#else /* !DEBUG */

#define PP_DEBUG(fmt, ...)

#endif
#define SYSCPLD_SWE_ID 	0x05
#define SYSCPLD_SWE_ID_MASK 	0x03

static ssize_t syscpld_slotid_show(struct device *dev,
                                    struct device_attribute *attr,
                                    char *buf)
{
  struct i2c_client *client = to_i2c_client(dev);
  i2c_dev_data_st *data = i2c_get_clientdata(client);
  i2c_sysfs_attr_st *i2c_attr = TO_I2C_SYSFS_ATTR(attr);
  const i2c_dev_attr_st *dev_attr = i2c_attr->isa_i2c_attr;
  int val_msb, val_lsb;
  int val_mask;
  int slot_id;

  val_mask = ~(((-1) >> (dev_attr->ida_n_bits)) << (dev_attr->ida_n_bits));

  mutex_lock(&data->idd_lock);

  val_msb = i2c_smbus_read_byte_data(client, dev_attr->ida_reg);
  val_lsb = i2c_smbus_read_byte_data(client, SYSCPLD_SWE_ID);
  mutex_unlock(&data->idd_lock);

  if (val_msb < 0 || val_lsb < 0) {
    /* error case */
    return -1;
  }

  val_msb = (val_msb >> dev_attr->ida_bit_offset) & val_mask;

  if(val_msb < 4) { /*LC*/
    /*val_lsb: 00->Left, 01->Right*/
	val_lsb &= SYSCPLD_SWE_ID_MASK;
	slot_id = (val_msb << 4) | val_lsb;
  } else {
    slot_id = val_msb;
  }

  return scnprintf(buf, PAGE_SIZE, "%u\n", slot_id + 1);
}


static const i2c_dev_attr_st syscpld_attr_table[] = {
  {
    "board_ver",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x0, 0, 4,
  },
  {
    "model_id",
    "0x0: Galaxy100 LC\n"
    "0x1: Galaxy100 FC\n"
    "0x3: Galaxy100 CMM\n",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x0, 4, 4,
  },
  {
    "cpld_rev",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x1, 0, 6,
  },
  {
    "cpld_released",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x1, 6, 1,
  },
  {
    "cpld_sub_rev",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x2, 0, 8,
  },
  {
    "slotid",
    NULL,
    syscpld_slotid_show,
    NULL,
    0x3, 0, 4,
  },
  {
    "vcp_id",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x4, 0, 2,
  },
  {
    "led_red",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x6, 0, 1,
  },
  {
    "led_green",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x6, 1, 1,
  },
  {
    "led_blue",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x6, 2, 1,
  },
  {
    "led_blink",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    6, 3, 1,
  },
  {
    "micro_srv_present",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x8, 0, 1,
  },
  {
    "pwr_cyc_all_n",
    "0: power cycle all power\n"
    "1: normal",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x10, 0, 1,
  },
  {
    "pwr_main_n",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x10, 1, 1,
  },
  {
    "pwr_main_force",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x10, 4, 1,
  },
  {
    "hot_rst_req",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x11, 0, 1,
  },
  {
    "warm_rst_req",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x11, 1, 1,
  },
  {
    "cold_rst_req",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x11, 2, 1,
  },
  {
    "pwr_rst_req",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x11, 3, 1,
  },
  {
    "th_sys_rst_n",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x12, 0, 1,
  },
  {
    "th_pcie_rst_n",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x12, 1, 1,
  },
  {
    "usb_hub_rst",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x14, 1, 1,
  },
  {
    "bcm5389_rst",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x14, 2, 1,
  },
  {
    "switch_phy_rst",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x14, 3, 1,
  },
  {
    "bcm54616s_rst",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x14, 4, 1,
  },
  {
    "bmc_main_rest_en",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x20, 0, 1,
  },
  {
    "bmc_rst1_en",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x20, 1, 1,
  },
  {
    "bmc_rst2_en",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x20, 2, 1,
  },
  {
    "bmc_rst3_en",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x20, 3, 1,
  },
  {
    "bmc_rst4_en",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x20, 4, 1,
  },
  {
    "bmc_wdt1_en",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x20, 5, 1,
  },
  {
    "bmc_wdt2_en",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x20, 6, 1,
  },
  {
    "bmc_rst",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    I2C_DEV_ATTR_STORE_DEFAULT,
    0x21, 0, 1,
  },
  {
    "uart_mux",
    "0: micro-server uart 0 connected to BMC\n"
    "1: micro-server uart 2 connected to BMC\n",
    "2: micro-server uart 2 connected to BMC",
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x30, 0, 2,
  },
  {
    "rov_sta",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x40, 0, 4,
  },
  {
    "bd_pwr_sta",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x41, 0, 8,
  },
  {
    "dual_boot_en",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x80, 0, 1,
  },
  {
    "2nd_flash_wp",
    NULL,
    I2C_DEV_ATTR_SHOW_DEFAULT,
    NULL,
    0x80, 1, 1,
  },
};

static i2c_dev_data_st syscpld_data;

/*
 * SYSCPLD i2c addresses.
 * normal_i2c is used in I2C_CLIENT_INSMOD_1()
 */
static const unsigned short normal_i2c[] = {
  0x31, I2C_CLIENT_END
};

/*
 * Insmod parameters
 */
I2C_CLIENT_INSMOD_1(syscpld);

/* SYSCPLD id */
static const struct i2c_device_id syscpld_id[] = {
  { "syscpld", syscpld },
  { },
};
MODULE_DEVICE_TABLE(i2c, syscpld_id);

/* Return 0 if detection is successful, -ENODEV otherwise */
static int syscpld_detect(struct i2c_client *client, int kind,
                          struct i2c_board_info *info)
{
  /*
   * We don't currently do any detection of the SYSCPLD
   */
  strlcpy(info->type, "syscpld", I2C_NAME_SIZE);
  return 0;
}

static int syscpld_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
  int n_attrs = sizeof(syscpld_attr_table) / sizeof(syscpld_attr_table[0]);
  return i2c_dev_sysfs_data_init(client, &syscpld_data,
                                 syscpld_attr_table, n_attrs);
}

static int syscpld_remove(struct i2c_client *client)
{
  i2c_dev_sysfs_data_clean(client, &syscpld_data);
  return 0;
}

static struct i2c_driver syscpld_driver = {
  .class    = I2C_CLASS_HWMON,
  .driver = {
    .name = "syscpld",
  },
  .probe    = syscpld_probe,
  .remove   = syscpld_remove,
  .id_table = syscpld_id,
  .detect   = syscpld_detect,
  /* addr_data is defined through I2C_CLIENT_INSMOD_1() */
  .address_data = &addr_data,
};

static int __init syscpld_mod_init(void)
{
  return i2c_add_driver(&syscpld_driver);
}

static void __exit syscpld_mod_exit(void)
{
  i2c_del_driver(&syscpld_driver);
}

MODULE_AUTHOR("Mickey Zhan");
MODULE_DESCRIPTION("SYSCPLD Driver");
MODULE_LICENSE("GPL");

module_init(syscpld_mod_init);
module_exit(syscpld_mod_exit);