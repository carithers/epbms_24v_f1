

/*变量定义区*/
//GPIO_InitTypeDef GPIO_InitStructure;     												//定义一个结构体变量GPIO_InitStructure，用于初始化GPIO操作
//ErrorStatus HSEStartUpStatus;
u8 count=0;

/*函数声明区*/
void NVIC_Configuration(void);
void Delay(vu32 nCount);
void LED_Runing(u8 LED_NUM);
