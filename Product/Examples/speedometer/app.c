/*
  Copyright 2014-2015 juma.io

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "juma_sdk_api.h"
#include "LIS2DH12.h"
#include "stdio.h"

/**********ȫ�ֱ�������************/
uint8_t 	g_mode;
uint8_t 	light_en;
uint8_t 	acc_en;
uint32_t 	vcc_en;
uint8_t 	g_vcc_value;

/************����������*************/
void on_vcc_complete(void* args);
void ble_device_on_connect(void);
void ble_device_on_disconnect(uint8_t reason);
void ble_device_on_message(uint8_t type, uint16_t length, uint8_t* value);
void led_on(void* args);
void led_off(void* args);
void send_acc_data(void * args);


/****************���ܺ�����**************/
//��LED
void led_on(void* args)
{
	if(light_en)
  {
		gpio_write(5, 1);
		switch(g_mode)
		{
			case 0: gpio_write(7, 0); break;
			case 1: gpio_write(6, 0); break;
		}
	}
	run_after_delay(led_off, NULL, 500);
}


//�ر�LED
void led_off(void* args)
{
	gpio_write(5, 1);
	gpio_write(6, 1);
	gpio_write(7, 1);
	run_after_delay(led_on, NULL, 500);
}


//�ش����ݻ��ֻ���
//���ݸ�ʽ: 8���ֽ�[x_h|x_l|y_h|y_l|z_h|z_l|vcc|temp]
void send_acc_data(void * args)
{
	uint8_t data[8];
	int16_t acc_value;
  
  if(acc_en)
  {
    //x��
    acc_value = LIS2DH12_Get_Chan_Data(LIS2DH12_ACC_CHAN_X);
    data[0] = (acc_value >> 8) & 0xFF;
    data[1] = acc_value & 0xFF;
    //y��
    acc_value = LIS2DH12_Get_Chan_Data(LIS2DH12_ACC_CHAN_Y);
    data[2] = (acc_value >> 8) & 0xFF;
    data[3] = acc_value & 0xFF;  
    //z��
    acc_value = LIS2DH12_Get_Chan_Data(LIS2DH12_ACC_CHAN_Z);
    data[4] = (acc_value >> 8) & 0xFF;
    data[5] = acc_value & 0xFF;
		//����
		data[6] = g_vcc_value;
		//�¶�
		data[7] = get_temperature();
		
    ble_device_send(0x01, 8, data);
  }
	if(!vcc_en)
	{
		//һ���Ӽ��һ�ε���
		vcc_en = 600;
		vcc_measure(on_vcc_complete);
	}
	vcc_en--;
  run_after_delay(send_acc_data, NULL, 100);
}


//�ϵ�����ִ�У�����������ں���
void on_ready()
{
	//��ʼ��ȫ�ֱ���
	g_mode 		= 1;
	light_en 	= 0;
	acc_en 		= 0;
	vcc_en 		= 600;
	
	//��ʼ�����ᴫ����
	LIS2DH12_InitStruct LIS = 
		{
			 .MISO 	= 10,
			 .MOSI 	= 9,
			 .CSN 	= 11,
			 .SCK 	= 12,
			 .INT1 	= 13,
			 .INT2 	= 14,
			 .FREQUENCY = LIS2DH12_FREQUENCY, 
		};  
	LIS2DH12_Config(& LIS);  
	LIS2DH12_Set_Data_Rate(LIS2DH12_DATA_RAT_10HZ);  
	
	//��ʼ��RGB��GPIO
	gpio_setup(5, GPIO_OUTPUT);
	gpio_setup(6, GPIO_OUTPUT);
	gpio_setup(7, GPIO_OUTPUT);

		
	//���������Ĺ㲥���ڡ��㲥��
	ble_device_set_advertising_interval(1000);
	ble_device_set_name("Speedometer");
	//���������㲥
	ble_device_start_advertising();
	
	led_off(NULL);
}


/************�ص�������**************/
//���õ�����⺯���󣬸��ݺ���ָ��ص��˺���
void on_vcc_complete(void* args)
{
	adc_result_t *result = (adc_result_t*)args;
	
	//�͵���
	if(result->value <= 790 )
	{
		g_mode = 0;
		g_vcc_value = 1;
	}
	//һ��
	if(790 < result->value <= 850)
	{
		g_mode = 1;
		g_vcc_value = 2;
	}
	//����
	if(850 < result->value <= 900)
	{
		g_mode = 1;
		g_vcc_value = 3;
	}
	//����
	if(900 < result->value <= 960)
	{
		g_mode = 1;
		g_vcc_value = 4;
	}
	//�ĵ�
	if(960 < result->value <= 1024)
	{
		g_mode = 1;
		g_vcc_value = 5;
	}
	return;
}
//���������ӳɹ�ʱ��ϵͳ�Զ����ô˺���
void ble_device_on_connect(void)
{
  acc_en 		= 1;
  light_en 	= 1;
	vcc_measure(on_vcc_complete);
  run_after_delay(send_acc_data, NULL, 100);
}

//�����Ͽ�����ʱ��ϵͳ�Զ����ô˺���
void ble_device_on_disconnect(uint8_t reason)
{
  acc_en 		= 0;
  light_en 	= 0;
  ble_device_start_advertising();
}

//�豸���н��յ�����ʱ��ϵͳ���ô˺���
//������type:�û��Զ�������;length:���ݳ���;value:��������
void ble_device_on_message(uint8_t type, uint16_t length, uint8_t* value)
{
  g_mode = value[0];
  LIS2DH12_SET_SELF_TEST_MODE((LIS2DH12_SELF_TEST_MODE)g_mode);
}



