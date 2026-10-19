#include "stm32f10x.h"
#include "onenet.h"
#include "esp8266.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "dht11.h"
#include "oled.h" 
#include "BH1750.h"
#include "key.h"
#include "led.h"
#include "timer.h"
#include "exti.h"
#include "Buzzer.h"
#include <string.h>
#include <stdio.h>

#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"broker.emqx.io\",1883\r\n"

u8 temperature=0,humidity=0;
float Light = 0;
u8 alarmFlag = 0;
u8 alarm_is_free = 10;
u8 LED_Status = 0;

char PUB_BUF[256];
const char *devSubTopic[] = {"/mysmarthomeriverchen/sub"};
const char devPubTopic[] = "/mysmarthomeriverchen/pub";

int main(void)
{
	
	unsigned short timeCount = 0;	
	
	unsigned char *dataPtr = NULL;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	
	Delay_Init();									
	LED_Init();
	KEY_Init();
	EXTIX_Init();
	Buzzer_Init();
	BH1750_Init();
	
	Usart1_Init(115200);							
	Usart2_Init(115200);							
	OLED_Init();
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	
	 while(DHT11_Init())
	{
		delay_ms(200);
	}
	OLED_ShowString(20,24,"Networking",16,1);
	ESP8266_Init();					
	OLED_ShowString(0,24,"                ",16,1);
	OLED_ShowString(0,24," Connected to",16,1);
	OLED_ShowString(30,48,"ONENET",16,1);
	UsartPrintf(USART_DEBUG, "Connect MQTTs Server...\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		delay_ms(500);
	UsartPrintf(USART_DEBUG, "NET_OK\r\n");
	
	while(OneNet_DevLink())			
		delay_ms(500);
	
	Buzzer_ON();
	delay_ms(250);
	Buzzer_OFF();
	
	OLED_Clear();
	
	TIM2_Int_Init(2499,7199);
	TIM3_Int_Init(2499,7199);
	
	OneNET_Subscribe(devSubTopic,1);
	
while(1)
	{
		if(timeCount % 40 == 0){
			DHT11_Read_Data(&temperature,&humidity);
			UsartPrintf(USART1,"tempurture:%d humi:%d",temperature,humidity);
			Light = bh_data_read();
			UsartPrintf(USART1,"Light:%.1f lx\r\n",Light);
			if(alarm_is_free == 10){
				if (humidity < 80 && temperature < 30 && Light < 10000)alarmFlag = 0;
				else alarmFlag = 1;
			}
			if(alarm_is_free < 10)alarm_is_free++;
		}
		if(++timeCount >= 200){
				
				LED_Status = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
				UsartPrintf(USART_DEBUG, "OneNet_Publish\r\n");
				sprintf(PUB_BUF,"{\"Temp\":%d,\"Hum\":%d,\"Light\":%.1lf,\"Led\":%d,\"Beep\":%d}",temperature,humidity,Light,LED_Status?0:1,alarmFlag);
				OneNET_Publish(devPubTopic, PUB_BUF);
			
				timeCount = 0;
				ESP8266_Clear();
			}
		dataPtr = ESP8266_GetIPD(3);
		if(dataPtr != NULL)
		OneNet_RevPro(dataPtr);
			
		delay_ms(10);
	}

}
