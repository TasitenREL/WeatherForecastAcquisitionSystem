//ディスプレイに文字を表示するプログラム

#include<linux/module.h>
#include<linux/kthread.h>
#include<linux/delay.h>
#include<linux/i2c.h>
#include<linux/interrupt.h>
#include<linux/gpio.h>
//#include<linux/stdio.h>
#include<linux/string.h>
#define SMILE_SW 5
static int irq_num;
static char *msg = " ";
static char *flg = "0";
static char *time = "00:00:00";
module_param(msg, charp, 0664);
module_param(flg, charp, 0664);
module_param(time, charp, 0664);

static struct i2c_board_info info_lcd = { // 雛形を書き換え
    .type = "smile_lcd0802",    // 名前
    .addr = 0x3E,   // アドレス
    .flags = I2C_CLIENT_WAKE,
};

static int smile_thread(void *num)
{
    static struct i2c_client *my_lcd;
    struct i2c_adapter *adap;
    int cnt = 0;

    adap = i2c_get_adapter(1);  // I2Cバス1のアダプタ情報を参照
    if(adap == NULL)
        goto ERR01;
    printk("(o_o) adapter[%s]\n", adap->name);
    my_lcd = i2c_new_client_device(adap, &info_lcd);   // I2Cデバイス登録
    if(my_lcd == NULL)
        goto ERR02;
    printk("(o_o) device[%s:%02x]\n", my_lcd->name, my_lcd->addr);
    i2c_smbus_write_i2c_block_data(my_lcd, 0x00, 6,
     "\x38\x39\x14\x70\x56\x6C");
    msleep(200);    // ↑初期化
    i2c_smbus_write_i2c_block_data(my_lcd, 0x00, 3, "\x38\x0d\x01");
    msleep(2);  // ↑画面クリア・カーソル表示

    
    i2c_smbus_write_i2c_block_data(my_lcd, 0x0D, 1, "\xC0");

    while(!kthread_should_stop()) {// 停止を指示されたらループを抜ける
        msleep(250);
	printk("フラグ");
	printk(flg);
	if (strcmp(flg,"1")==0){
		printk("enter");	
       	cnt++;
        i2c_smbus_write_i2c_block_data(my_lcd, 0x00, 1, "\x80");
		msleep(2);  // ↑カーソルを第1行目の先頭へ
        i2c_smbus_write_i2c_block_data(my_lcd, 0x40, strlen(time), time);
        msleep(2);  // ↑指定された文字列を一列目に表示

		i2c_smbus_write_i2c_block_data(my_lcd, 0x00, 1, "\xC0");
		msleep(2);  // ↑カーソルを第2行目の先頭へ
        	i2c_smbus_write_i2c_block_data(my_lcd, 0x40, strlen(msg), msg);
        	msleep(1000);  // ↑指定された文字列を二行目に表示
	}
	else{
		printk("noenter");
    	i2c_smbus_write_i2c_block_data(my_lcd, 0x00, 3, "\x38\x0d\x01");
    	msleep(2);  // ↑画面クリア・カーソル表示

	}
	printk("天気");
	printk(msg);
	printk("nowTime");
	printk(time);
    }
    i2c_unregister_device(my_lcd);  // I2Cデバイス登録を解除  
ERR02: 
    i2c_put_adapter(adap);  // アダプタの参照終了を通達
ERR01:
    while(!kthread_should_stop())
        msleep(10); 
    return 0;
}

static struct task_struct *mytask = NULL;

int init_module(void)
{
    printk("(o_o)/\n");
    mytask = kthread_run(smile_thread, NULL, "ksmile");//スレッド実行
    return IS_ERR(mytask)? -ENOMEM : 0; 
}

void cleanup_module(void)
{
    kthread_stop(mytask);   // カーネルスレッドに停止を指示
    printk("m(x_x)m\n");
}

MODULE_AUTHOR("takago@neptune.kanazawa-it.ac.jp");
MODULE_LICENSE("GPL");
