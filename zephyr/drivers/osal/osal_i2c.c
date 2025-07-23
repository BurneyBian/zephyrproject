/*
* Copyright (c) 2021, Supper Acme SW Platform
* All rights reserved.
*
* Name:osal_i2c.c
* Identification :
* Summary:i2c osal functions
*
* Author:Wynn 
* Date:2022.11.23
*
*/
#include <string.h>
#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "osal.h"

#define I2C_MAX_NUM 4

#define OSAL_PROC_DEBUG 0

static struct rt_i2c_bus_device *general_client[I2C_MAX_NUM];

int osal_i2c_write(unsigned char i2c_dev, unsigned char dev_addr,
                               unsigned int reg_addr, unsigned int reg_addr_num,
                               unsigned int data, unsigned int data_byte_num)
{
    unsigned char tmp_buf[8];
    int ret = 0;
    int idx = 0;
    struct rt_i2c_bus_device *client;
    unsigned int u32Tries = 0;
    unsigned short flags = RT_I2C_WR; // send
    struct rt_i2c_msg msg[1] = {0};

    if (i2c_dev >= I2C_MAX_NUM) {
        return -EINVAL;
    }

    if (general_client[i2c_dev] == NULL) {
        return -EINVAL;
    }

    //osal_memcpy(&client, general_client[i2c_dev], sizeof(struct i2c_client));
    client = general_client[i2c_dev];

    /* reg_addr config */
    if (reg_addr_num == 1) {
        tmp_buf[idx++] = reg_addr & 0xff;
    } else {
        tmp_buf[idx++] = (reg_addr >> 8) & 0xff;
        tmp_buf[idx++] = reg_addr & 0xff;
    }

    /* data config */
    if (data_byte_num == 1) {
        tmp_buf[idx++] = data & 0xff;
    } else {
        tmp_buf[idx++] = (data >> 8) & 0xff;
        tmp_buf[idx++] = data & 0xff;
    }

    msg[0].addr = (dev_addr & 0xff);
    msg[0].flags = flags;
    msg[0].len = idx;
    msg[0].buf = tmp_buf;

    while (1) {
        ret = rt_i2c_transfer(client, &msg[0], 1);
        if (1 == ret) {
            break;
        } else {
            u32Tries++;
            if (u32Tries > 5) {
                osal_printk("[%s %d] i2c_master_send error, ret=%d. \n", __func__, __LINE__, ret);
                return -1;
            }
        }
    }

    return 0;
}
RTM_EXPORT(osal_i2c_write);

int osal_i2c_write_burst(unsigned char i2c_dev, unsigned char dev_addr,
                               unsigned int reg_addr, unsigned int reg_addr_num,
                               unsigned char *data, unsigned int data_burst_byte_len)
{
    unsigned char *tmp_buf;
    int ret = 0;
    int idx = 0;
    struct rt_i2c_bus_device *client;
    unsigned int u32Tries = 0;
    unsigned short flags = RT_I2C_WR; // send
    struct rt_i2c_msg msg[1] = {0};

    if (i2c_dev >= I2C_MAX_NUM) {
        return -EINVAL;
    }

    if (general_client[i2c_dev] == NULL) {
        return -EINVAL;
    }

    tmp_buf = osal_kmalloc(sizeof(char)*(reg_addr_num+data_burst_byte_len), osal_gfp_kernel);
    if (NULL == tmp_buf) {
        osal_printk_err("[%s %d] kmalloc failed buf 0x%px buflen %d . \n", __func__, __LINE__, tmp_buf, sizeof(char)*(reg_addr_num+data_burst_byte_len));
    }

    //osal_memcpy(&client, general_client[i2c_dev], sizeof(struct i2c_client));
    client = general_client[i2c_dev];

    /* reg_addr config */
    if (reg_addr_num == 1) {
        tmp_buf[idx++] = reg_addr & 0xff;
    } else {
        tmp_buf[idx++] = (reg_addr >> 8) & 0xff;
        tmp_buf[idx++] = reg_addr & 0xff;
    }

    /* data config */
    osal_memcpy(tmp_buf+idx, data, data_burst_byte_len);
    idx += data_burst_byte_len;

    msg[0].addr = (dev_addr & 0xff);
    msg[0].flags = flags;
    msg[0].len = idx;
    msg[0].buf = tmp_buf;

    while (1) {
        ret = rt_i2c_transfer(client, &msg[0], 1);
        if (1 == ret) {
            break;
        } else {
            u32Tries++;
            if (u32Tries > 5) {
                osal_printk("[%s %d] i2c_master_send error, ret=%d. \n", __func__, __LINE__, ret);
                return -1;
            }
        }
    }

    return 0;
}
RTM_EXPORT(osal_i2c_write_burst);

int osal_i2c_write_batch(unsigned char i2c_dev, unsigned char dev_addr,
                            struct osal_i2c_data_w* data_w, unsigned int data_len,
                            unsigned int reg_addr_num,  unsigned int data_byte_num)
{
    unsigned char *tmp_buf;
    int ret = 0, i = 0;
    int idx = 0;
    struct rt_i2c_bus_device *client;
    unsigned int u32Tries = 0;
    unsigned short flags = RT_I2C_WR; // send
    struct rt_i2c_msg *msg;

    if (i2c_dev >= I2C_MAX_NUM) {
        return -EINVAL;
    }

    if (general_client[i2c_dev] == NULL) {
        return -EINVAL;
    }

    if(NULL == data_w)
    {
        osal_printk_err("[%s %d] data_w NULL error. \n", __func__, __LINE__);
        return -EINVAL;
    }

    if(data_len <= 0)
    {
        osal_printk_err("[%s %d] data_len error %d. \n", __func__, __LINE__, data_len);
        return 0;
    }

    client = general_client[i2c_dev];

    msg = osal_kmalloc(sizeof(struct rt_i2c_msg)*data_len, osal_gfp_kernel);
    tmp_buf = osal_kmalloc(sizeof(char)*(reg_addr_num+data_byte_num)*data_len, osal_gfp_kernel);
    if((NULL == msg) || (NULL == tmp_buf))
    {
        osal_printk_err("[%s %d] kmalloc failed msg 0x%px buf 0x%px buflen %d . \n", __func__, __LINE__, msg, tmp_buf, sizeof(char)*(reg_addr_num+data_byte_num)*data_len);
    }

    //osal_memcpy(&client, general_client[i2c_dev], sizeof(struct i2c_client));
    client = general_client[i2c_dev];

    for(i = 0; i < data_len; i++)
    {
        idx = 0;
        unsigned char *data_buf = tmp_buf + i*(reg_addr_num+data_byte_num);

        if (reg_addr_num == 1) {
            data_buf[idx++] = data_w[i].reg_addr & 0xff;
        } else {
            data_buf[idx++] = (data_w[i].reg_addr >> 8) & 0xff;
            data_buf[idx++] = data_w[i].reg_addr & 0xff;
        }

        /* data config */
        if (data_byte_num == 1) {
            data_buf[idx++] = data_w[i].data & 0xff;
        } else {
            data_buf[idx++] = (data_w[i].data >> 8) & 0xff;
            data_buf[idx++] = data_w[i].data & 0xff;
        }

        msg[i].addr = dev_addr & 0xff;
        msg[i].flags = flags;
        msg[i].len = idx;
        msg[i].buf = data_buf;
    }

    while (1) {
        ret = rt_i2c_transfer(client, &msg[0], data_len);
        if(data_len == ret)
        {
            break;
        }
        else {
            u32Tries++;
            if (u32Tries > 5) {
                osal_printk_err("[%s %d] i2c_master_send error, ret=%d. \n", __func__, __LINE__, ret);
                osal_kfree(msg);
                osal_kfree(tmp_buf);
                return -3;
            }
        }
    }

    osal_kfree(msg);
    osal_kfree(tmp_buf);

    return 0;
}
RTM_EXPORT(osal_i2c_write_batch);

int osal_i2c_read(unsigned char i2c_dev, unsigned char dev_addr,
                               unsigned int reg_addr, unsigned int reg_addr_num,
                               unsigned int *data, unsigned int data_byte_num)
{
    unsigned char tmp_buf[2];
    int ret = 0;
    int idx = 0;
    struct rt_i2c_bus_device *client;
    unsigned int u32Tries = 0;
    unsigned short flags = RT_I2C_WR; // send
    struct rt_i2c_msg msg[2] = {0};

    if (i2c_dev >= I2C_MAX_NUM) {
        return -EINVAL;
    }

    if (general_client[i2c_dev] == NULL) {
        return -EINVAL;
    }

    //osal_memcpy(&client, general_client[i2c_dev], sizeof(struct i2c_client));
    client = general_client[i2c_dev];

    char client_addr = (dev_addr & 0xff);

    /* reg_addr config */
    if (reg_addr_num == 1) {
        tmp_buf[idx++] = reg_addr & 0xff;
    } else {
        tmp_buf[idx++] = (reg_addr >> 8) & 0xff;
        tmp_buf[idx++] = reg_addr & 0xff;
    }

    msg[0].addr = client_addr;
    msg[0].flags = flags;
    msg[0].len = idx;
    msg[0].buf = tmp_buf;

    flags = RT_I2C_RD;
    msg[1].addr = client_addr;
    msg[1].flags = flags;
    msg[1].len = data_byte_num;
    msg[1].buf = (unsigned char*)data;
    
    while (1) {
        ret = rt_i2c_transfer(client, &msg[0], 1);
        if (1 == ret) {
            ret = rt_i2c_transfer(client, &msg[1], 1);
            if (1 == ret) {
                break;
            } else {
                osal_printk("[%s %d] i2c_master_recv error, ret=%d. \n", __func__, __LINE__, ret);
                //return ret;
            }
        } else {
            u32Tries++;
            if (u32Tries > 5) {
                osal_printk("[%s %d] i2c_master_send error, ret=%d. \n", __func__, __LINE__, ret);
                return -1;
            }
        }
    }

    return 0;
}
RTM_EXPORT(osal_i2c_read);

int osal_i2c_init(void)
{
    int i = 0;
    char i2c_name[5] = {'i', '2', 'c'};
    int ret;
    uint32_t devopen = 1;

    #define I2C_DEFINED     1
    #define I2C_NODEFINED   0
    int i2c_use_list[I2C_MAX_NUM] = 
    {
    #ifdef BSP_USING_I2C0
        I2C_DEFINED,
    #else
        I2C_NODEFINED,
    #endif
    #ifdef BSP_USING_I2C1
        I2C_DEFINED,
    #else
        I2C_NODEFINED,
    #endif
    #ifdef BSP_USING_I2C2
        I2C_DEFINED,
    #else
        I2C_NODEFINED,
    #endif
    #ifdef BSP_USING_I2C3
        I2C_DEFINED,
    #else
        I2C_NODEFINED,
    #endif
    };

    for (i = 0; i < I2C_MAX_NUM; i++) {
        if (!i2c_use_list[i])
        {
            continue;
        }

        i2c_name[3] = i + 0x30;
        general_client[i] = (struct rt_i2c_bus_device *)rt_i2c_bus_device_find(i2c_name);
        if (RT_NULL == general_client[i]) {
            osal_printk_err("[%s %d] i2c_get_adapter error. \n", __func__, i);
        } else {
            ret = rt_i2c_control(general_client[i], RT_I2C_DEV_CTRL_PWR, &devopen);
            if (RT_EOK != ret) {
                osal_printk_war("ctrl pwr i2c%d open failed %d\n", i, ret);
            }
            ret = rt_i2c_bus_device_open(general_client[i], 0);
            if (ret < 0) {
                osal_printk_war("rt_i2c_bus_device_open %d error %d!\n", i, ret);
            }
        }
    }

    return 0;
}
RTM_EXPORT(osal_i2c_init);

void osal_i2c_exit(void)
{
    int i = 0;
    int ret;
    uint32_t devopen = 0;

    for (i = 0; i < I2C_MAX_NUM; i++) {
        if (general_client[i]) {
            ret = rt_i2c_control(general_client[i], RT_I2C_DEV_CTRL_PWR, &devopen);
            if (RT_EOK != ret) {
                osal_printk_war("ctrl pwr i2c%d close failed %d\n", i, ret);
            }
            ret = rt_i2c_bus_device_close(general_client[i]);
            if (ret < 0) {
                osal_printk_war("rt_i2c_bus_device_close %d error %d!\n", i, ret);
            }
        }
        general_client[i] = RT_NULL;
    }
}
RTM_EXPORT(osal_i2c_exit);