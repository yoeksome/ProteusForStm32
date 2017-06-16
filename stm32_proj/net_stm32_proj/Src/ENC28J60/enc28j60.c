#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"


#include <stdio.h>
#include "enc28j60.h"	  


struct uip_eth_addr {
  uint8_t addr[6];
};

static uint8_t ENC28J60BANK;
static uint32_t NextPacketPtr;
extern SPI_HandleTypeDef hspi1;

//uint8_t uip_buf[2048 + 2];   /* The packet buffer that contains
//				    incoming packets. */
extern uint16_t  uip_slen;
uint8_t uip_buf[100];

uint16_t uip_len;
//struct uip_eth_addr uip_ethaddr = {{0,0,0,0,0,0}};

//MAC地址,必须唯一
//如果你有两个战舰开发板,想连入路由器,则需要修改MAC地址不一样!
const uint8_t mymac[6]={0x04,0x02,0x35,0x00,0x00,0x01};	//MAC地址
																				  
//配置网卡硬件，并设置MAC地址 
//返回值：0，正常；1，失败；
uint8_t tapdev_init(void)
{   	 
	uint8_t i,res=0;					  
	res=ENC28J60_Init((uint8_t*)mymac);	//初始化ENC28J60					  
	if(res)
		printf("init error %d\r\n",res);
	else
		printf("initial ok \r\n");
	
//	//把IP地址和MAC地址写入缓存区
// 	for (i = 0; i < 6; i++)uip_ethaddr.addr[i]=mymac[i];  
    //指示灯状态:0x476 is PHLCON LEDA(绿)=links status, LEDB(红)=receive/transmit
 	//PHLCON：PHY 模块LED 控制寄存器	    
	ENC28J60_PHY_Write(PHLCON,0x0476);
	return res;	
}

//读取一包数据  
uint16_t tapdev_read(void)
{	
	return  ENC28J60_Packet_Receive(MAX_FRAMELEN,uip_buf);
}
//发送一包数据  
void tapdev_send(void)
{
	ENC28J60_Packet_Send(uip_len,uip_buf);
}

//复位ENC28J60
//包括SPI初始化/IO初始化等

//static void ENC28J60_SPI2_Init(void)
//{
//   	SPI_InitTypeDef  SPI_InitStructure;
//	GPIO_InitTypeDef  GPIO_InitStructure;
//	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );//SPI2时钟使能 	
//   	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOG, ENABLE );//PORTB,D,G时钟使能 
// 
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 // 端口配置
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
// 	GPIO_Init(GPIOD, &GPIO_InitStructure);					 //根据设定参数初始化GPIOD.2
// 	GPIO_SetBits(GPIOD,GPIO_Pin_2);						 //PD.2上拉
//			  
//	//这里PG7和PB12拉高,是为了防止NRF24L01和SPI FLASH影响.
//	//因为他们共用一个SPI口.  

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 // PB12 推挽   上拉
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.12
// 	GPIO_SetBits(GPIOB,GPIO_Pin_12);						 //PB.12上拉

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;//PG6/7/8 推挽  上拉			 
// 	GPIO_Init(GPIOG, &GPIO_InitStructure);					 //根据设定参数初始化//PG6/7/8
// 	GPIO_SetBits(GPIOG,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8);//PG6/7/8上拉	
//			
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB

// 	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);  //PB13/14/15上拉


//	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
//	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
//	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为低电平
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第一个跳变沿（上升或下降）数据被采样
//	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
//	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
//	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
//	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
// 
//	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
//	
//	SPI2_ReadWriteByte(0xff);//启动传输		 

//}
void ENC28J60_Reset(void)
{
	uint8_t dat=0xff,datr;
//	ENC28J60_SPI2_Init();//SPI2初始化
//	SPI2_SetSpeed(SPI_BaudRatePrescaler_4);	//SPI2 SCK频率为36M/4=9Mhz
// 	TIM6_Int_Init(1000,719);//100Khz计数频率，计数到1000为10ms
//	ENC28J60_RST=0;			//复位ENC28J60
	HAL_GPIO_WritePin(GPIOA,enc28j60_rst_Pin,GPIO_PIN_RESET);
	HAL_Delay(10);	 
	HAL_GPIO_WritePin(GPIOA,enc28j60_rst_Pin,GPIO_PIN_SET);
//	ENC28J60_RST=1;			//复位结束				    
	HAL_Delay(10);	 
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_RESET);	
	HAL_SPI_TransmitReceive(&hspi1,&dat,&datr,1,10);
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_SET);
	HAL_Delay(10);	
}
//读取ENC28J60寄存器(带操作码) 
//op：操作码
//addr:寄存器地址/参数
//返回值:读到的数据
uint8_t ENC28J60_Read_Op(uint8_t op,uint8_t addr)
{
	uint8_t dat=0,datr;	 
//	ENC28J60_CS=0;	
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_RESET);	
	dat=op|(addr&ADDR_MASK);
	HAL_SPI_TransmitReceive(&hspi1,&dat,&datr,1,10);
	dat=0xff;
	HAL_SPI_TransmitReceive(&hspi1,&dat,&datr,1,10);
//	HAL_SPI_Transmit(&hspi1,&dat,1,10);
//	HAL_SPI_Receive(&hspi1,&dat,1,10);
//	SPI2_ReadWriteByte(dat);
//	dat=SPI2_ReadWriteByte(0xFF);
	//如果是读取MAC/MII寄存器,则第二次读到的数据才是正确的,见手册29页
 	if(addr&0x80){HAL_SPI_TransmitReceive(&hspi1,&dat,&datr,1,10);}
//	ENC28J60_CS=1;
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_SET);	
//	printf("dat:%x datr:%x\r\n",op|(addr&ADDR_MASK),datr);
//	HAL_Delay(1);
	return datr;
}
//读取ENC28J60寄存器(带操作码) 
//op：操作码
//addr:寄存器地址
//data:参数
void ENC28J60_Write_Op(uint8_t op,uint8_t addr,uint8_t data)
{
	uint8_t dat = 0,datr;	    
//	ENC28J60_CS=0;
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_RESET);	
	dat=op|(addr&ADDR_MASK);
//	SPI2_ReadWriteByte(dat);	  
//	SPI2_ReadWriteByte(data);
	HAL_SPI_TransmitReceive(&hspi1,&dat,&datr,1,10);
	HAL_SPI_TransmitReceive(&hspi1,&data,&datr,1,10);
//	HAL_SPI_Transmit(&hspi1,&dat,1,10);
//	HAL_SPI_Transmit(&hspi1,&data,1,10);
//	ENC28J60_CS=1;
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_SET);	
//	printf("dat:%x data:%x\r\n",dat,data);
//	HAL_Delay(1);
}
//读取ENC28J60接收缓存数据
//len:要读取的数据长度
//data:输出数据缓存区(末尾自动添加结束符)
void ENC28J60_Read_Buf(uint32_t len,uint8_t* data)
{
	uint8_t tmp,datr;
//	ENC28J60_CS=0;	
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_RESET);		
//	SPI2_ReadWriteByte(ENC28J60_READ_BUF_MEM);
	tmp = ENC28J60_READ_BUF_MEM;
	HAL_SPI_TransmitReceive(&hspi1,&tmp,&datr,1,10);
	tmp = 0;
	HAL_SPI_TransmitReceive(&hspi1,&tmp,data,len,10);
	data = data + len;
//	*data='\0';
//	while(len)
//	{
//		len--;			  
////		*data=(uint8_t)SPI2_ReadWriteByte(0);
////		HAL_SPI_Receive(&hspi1,data,1,10);
//		HAL_SPI_TransmitReceive(&hspi1,data,&datr,1,10);
//		data++;
//	}
//	*data='\0';
//	ENC28J60_CS=1;
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_SET);
}
//向ENC28J60写发送缓存数据
//len:要写入的数据长度
//data:数据缓存区 
void ENC28J60_Write_Buf(uint32_t len,uint8_t* data)
{
	uint8_t tmp,datr,res;
//	ENC28J60_CS=0;		
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_RESET);	
//	SPI2_ReadWriteByte(ENC28J60_WRITE_BUF_MEM);		 
	tmp = ENC28J60_READ_BUF_MEM;
//	HAL_SPI_Transmit(&hspi1,&tmp,1,10);
	HAL_SPI_TransmitReceive(&hspi1,&tmp,&datr,1,10);
	res = HAL_SPI_TransmitReceive(&hspi1,data,0,len,10);
	if(res == HAL_TIMEOUT)
		printf("HAL_TIMEOUT\r\n");
//	while(len)
//	{
//		len--;
////		SPI2_ReadWriteByte(*data);
////		HAL_SPI_Transmit(&hspi1,data,1,10);
//		HAL_SPI_TransmitReceive(&hspi1,data,&datr,1,10);
//		data++;
//	}
//	ENC28J60_CS=1;
	HAL_GPIO_WritePin(GPIOA,SPI_NSS_Pin,GPIO_PIN_SET);
}
//设置ENC28J60寄存器Bank
//ban:要设置的bank
void ENC28J60_Set_Bank(uint8_t bank)
{								    
	if((bank&BANK_MASK)!=ENC28J60BANK)//和当前bank不一致的时候,才设置
	{				  
		ENC28J60_Write_Op(ENC28J60_BIT_FIELD_CLR,ECON1,(ECON1_BSEL1|ECON1_BSEL0));
		ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET,ECON1,(bank&BANK_MASK)>>5);
		ENC28J60BANK=(bank&BANK_MASK);
	}
}
//读取ENC28J60指定寄存器 
//addr:寄存器地址
//返回值:读到的数据
uint8_t ENC28J60_Read(uint8_t addr)
{						  
	ENC28J60_Set_Bank(addr);//设置BANK		 
	return ENC28J60_Read_Op(ENC28J60_READ_CTRL_REG,addr);
}
//向ENC28J60指定寄存器写数据
//addr:寄存器地址
//data:要写入的数据		 
void ENC28J60_Write(uint8_t addr,uint8_t data)
{					  
	ENC28J60_Set_Bank(addr);		 
	ENC28J60_Write_Op(ENC28J60_WRITE_CTRL_REG,addr,data);
}
//向ENC28J60的PHY寄存器写入数据
//addr:寄存器地址
//data:要写入的数据		 
void ENC28J60_PHY_Write(uint8_t addr,uint32_t data)
{
	uint16_t retry=0;
	ENC28J60_Write(MIREGADR,addr);	//设置PHY寄存器地址
	ENC28J60_Write(MIWRL,data);		//写入数据
	ENC28J60_Write(MIWRH,data>>8);		   
	while((ENC28J60_Read(MISTAT)&MISTAT_BUSY)&&retry<0XFFF)retry++;//等待写入PHY结束		  
}
//初始化ENC28J60
//macaddr:MAC地址
//返回值:0,初始化成功;
//       1,初始化失败;
uint8_t ENC28J60_Init(uint8_t* macaddr)
{		
	uint16_t retry=0;		  
	ENC28J60_Reset();
	ENC28J60_Write_Op(ENC28J60_SOFT_RESET,0,ENC28J60_SOFT_RESET);//软件复位
	while(!(ENC28J60_Read(ESTAT)&ESTAT_CLKRDY)&&retry<500)//等待时钟稳定
	{
//		retry = ENC28J60_Read(ESTAT);
//		printf("ESTAT:%x \r\n",retry);
		retry++;
		HAL_Delay(1);
	};
	if(retry>=100)return 2;//ENC28J60初始化失败
	HAL_Delay(10);
	// do bank 0 stuff
	// initialize receive buffer
	// 16-bit transfers,must write low byte first
	// set receive buffer start address	   设置接收缓冲区地址  8K字节容量
	NextPacketPtr=RXSTART_INIT;
	// Rx start
	//接收缓冲器由一个硬件管理的循环FIFO 缓冲器构成。
	//寄存器对ERXSTH:ERXSTL 和ERXNDH:ERXNDL 作
	//为指针，定义缓冲器的容量和其在存储器中的位置。
	//ERXST和ERXND指向的字节均包含在FIFO缓冲器内。
	//当从以太网接口接收数据字节时，这些字节被顺序写入
	//接收缓冲器。 但是当写入由ERXND 指向的存储单元
	//后，硬件会自动将接收的下一字节写入由ERXST 指向
	//的存储单元。 因此接收硬件将不会写入FIFO 以外的单
	//元。
	//设置接收起始字节
	ENC28J60_Write(ERXSTL,RXSTART_INIT&0xFF);	
	ENC28J60_Write(ERXSTH,RXSTART_INIT>>8);	  
	//ERXWRPTH:ERXWRPTL 寄存器定义硬件向FIFO 中
	//的哪个位置写入其接收到的字节。 指针是只读的，在成
	//功接收到一个数据包后，硬件会自动更新指针。 指针可
	//用于判断FIFO 内剩余空间的大小  8K-1500。 
	//设置接收读指针字节
	ENC28J60_Write(ERXRDPTL,RXSTART_INIT&0xFF);
	ENC28J60_Write(ERXRDPTH,RXSTART_INIT>>8);
	//设置接收结束字节
	ENC28J60_Write(ERXNDL,RXSTOP_INIT&0xFF);
	ENC28J60_Write(ERXNDH,RXSTOP_INIT>>8);
	//设置发送起始字节
	ENC28J60_Write(ETXSTL,TXSTART_INIT&0xFF);
	ENC28J60_Write(ETXSTH,TXSTART_INIT>>8);
	//设置发送结束字节
	ENC28J60_Write(ETXNDL,TXSTOP_INIT&0xFF);
	ENC28J60_Write(ETXNDH,TXSTOP_INIT>>8);
	// do bank 1 stuff,packet filter:
	// For broadcast packets we allow only ARP packtets
	// All other packets should be unicast only for our mac (MAADR)
	//
	// The pattern to match on is therefore
	// Type     ETH.DST
	// ARP      BROADCAST
	// 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
	// in binary these poitions are:11 0000 0011 1111
	// This is hex 303F->EPMM0=0x3f,EPMM1=0x30
	//接收过滤器
	//UCEN：单播过滤器使能位
	//当ANDOR = 1 时：
	//1 = 目标地址与本地MAC 地址不匹配的数据包将被丢弃
	//0 = 禁止过滤器
	//当ANDOR = 0 时：
	//1 = 目标地址与本地MAC 地址匹配的数据包会被接受
	//0 = 禁止过滤器
	//CRCEN：后过滤器CRC 校验使能位
	//1 = 所有CRC 无效的数据包都将被丢弃
	//0 = 不考虑CRC 是否有效
	//PMEN：格式匹配过滤器使能位
	//当ANDOR = 1 时：
	//1 = 数据包必须符合格式匹配条件，否则将被丢弃
	//0 = 禁止过滤器
	//当ANDOR = 0 时：
	//1 = 符合格式匹配条件的数据包将被接受
	//0 = 禁止过滤器
	ENC28J60_Write(ERXFCON,ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
//	ENC28J60_Write(ERXFCON,0);
	ENC28J60_Write(EPMM0,0x3f);
	ENC28J60_Write(EPMM1,0x30);
	ENC28J60_Write(EPMCSL,0xf9);
	ENC28J60_Write(EPMCSH,0xf7);
	// do bank 2 stuff
	// enable MAC receive
	//bit 0 MARXEN：MAC 接收使能位
	//1 = 允许MAC 接收数据包
	//0 = 禁止数据包接收
	//bit 3 TXPAUS：暂停控制帧发送使能位
	//1 = 允许MAC 发送暂停控制帧（用于全双工模式下的流量控制）
	//0 = 禁止暂停帧发送
	//bit 2 RXPAUS：暂停控制帧接收使能位
	//1 = 当接收到暂停控制帧时，禁止发送（正常操作）
	//0 = 忽略接收到的暂停控制帧
	ENC28J60_Write(MACON1,MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	// bring MAC out of reset
	//将MACON2 中的MARST 位清零，使MAC 退出复位状态。
	ENC28J60_Write(MACON2,0x00);
	// enable automatic padding to 60bytes and CRC operations
	//bit 7-5 PADCFG2:PACDFG0：自动填充和CRC 配置位
	//111 = 用0 填充所有短帧至64 字节长，并追加一个有效的CRC
	//110 = 不自动填充短帧
	//101 = MAC 自动检测具有8100h 类型字段的VLAN 协议帧，并自动填充到64 字节长。如果不
	//是VLAN 帧，则填充至60 字节长。填充后还要追加一个有效的CRC
	//100 = 不自动填充短帧
	//011 = 用0 填充所有短帧至64 字节长，并追加一个有效的CRC
	//010 = 不自动填充短帧
	//001 = 用0 填充所有短帧至60 字节长，并追加一个有效的CRC
	//000 = 不自动填充短帧
	//bit 4 TXCRCEN：发送CRC 使能位
	//1 = 不管PADCFG如何，MAC都会在发送帧的末尾追加一个有效的CRC。 如果PADCFG规定要
	//追加有效的CRC，则必须将TXCRCEN 置1。
	//0 = MAC不会追加CRC。 检查最后4 个字节，如果不是有效的CRC 则报告给发送状态向量。
	//bit 0 FULDPX：MAC 全双工使能位
	//1 = MAC工作在全双工模式下。 PHCON1.PDPXMD 位必须置1。
	//0 = MAC工作在半双工模式下。 PHCON1.PDPXMD 位必须清零。
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET,MACON3,MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
	// set inter-frame gap (non-back-to-back)
	//配置非背对背包间间隔寄存器的低字节
	//MAIPGL。 大多数应用使用12h 编程该寄存器。
	//如果使用半双工模式，应编程非背对背包间间隔
	//寄存器的高字节MAIPGH。 大多数应用使用0Ch
	//编程该寄存器。
	ENC28J60_Write(MAIPGL,0x12);
	ENC28J60_Write(MAIPGH,0x0C);
	// set inter-frame gap (back-to-back)
	//配置背对背包间间隔寄存器MABBIPG。当使用
	//全双工模式时，大多数应用使用15h 编程该寄存
	//器，而使用半双工模式时则使用12h 进行编程。
	ENC28J60_Write(MABBIPG,0x15);
	// Set the maximum packet size which the controller will accept
	// Do not send packets longer than MAX_FRAMELEN:
	// 最大帧长度  1500
	ENC28J60_Write(MAMXFLL,MAX_FRAMELEN&0xFF);	
	ENC28J60_Write(MAMXFLH,MAX_FRAMELEN>>8);
	// do bank 3 stuff
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	//设置MAC地址
	ENC28J60_Write(MAADR5,macaddr[0]);	
	ENC28J60_Write(MAADR4,macaddr[1]);
	ENC28J60_Write(MAADR3,macaddr[2]);
	ENC28J60_Write(MAADR2,macaddr[3]);
	ENC28J60_Write(MAADR1,macaddr[4]);
	ENC28J60_Write(MAADR0,macaddr[5]);
	//配置PHY为全双工  LEDB为拉电流
	ENC28J60_PHY_Write(PHCON1,PHCON1_PDPXMD);	 
	// no loopback of transmitted frames	 禁止环回
	//HDLDIS：PHY 半双工环回禁止位
	//当PHCON1.PDPXMD = 1 或PHCON1.PLOOPBK = 1 时：
	//此位可被忽略。
	//当PHCON1.PDPXMD = 0 且PHCON1.PLOOPBK = 0 时：
	//1 = 要发送的数据仅通过双绞线接口发出
	//0 = 要发送的数据会环回到MAC 并通过双绞线接口发出
	ENC28J60_PHY_Write(PHCON2,PHCON2_HDLDIS);
	// switch to bank 0
	//ECON1 寄存器
	//寄存器3-1 所示为ECON1 寄存器，它用于控制
	//ENC28J60 的主要功能。 ECON1 中包含接收使能、发
	//送请求、DMA 控制和存储区选择位。	   
	ENC28J60_Set_Bank(ECON1);
	// enable interrutps
	//EIE： 以太网中断允许寄存器
	//bit 7 INTIE： 全局INT 中断允许位
	//1 = 允许中断事件驱动INT 引脚
	//0 = 禁止所有INT 引脚的活动（引脚始终被驱动为高电平）
	//bit 6 PKTIE： 接收数据包待处理中断允许位
	//1 = 允许接收数据包待处理中断
	//0 = 禁止接收数据包待处理中断
//	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET,EIE,EIE_INTIE|EIE_PKTIE);
	// enable packet reception
	//bit 2 RXEN：接收使能位
	//1 = 通过当前过滤器的数据包将被写入接收缓冲器
	//0 = 忽略所有接收的数据包
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET,ECON1,ECON1_RXEN);
	if(ENC28J60_Read(MAADR5)== macaddr[0])return 0;//初始化成功
	else return 3; 	  

}
//读取EREVID
uint8_t ENC28J60_Get_EREVID(void)
{
	//在EREVID 内也存储了版本信息。 EREVID 是一个只读控
	//制寄存器，包含一个5 位标识符，用来标识器件特定硅片
	//的版本号
	return ENC28J60_Read(EREVID);
}
//#include "uip.h"
//通过ENC28J60发送数据包到网络
//len:数据包大小
//packet:数据包
void ENC28J60_Packet_Send(uint32_t len,uint8_t* packet)
{
	//设置发送缓冲区地址写指针入口
	ENC28J60_Write(EWRPTL,TXSTART_INIT&0xFF);
	ENC28J60_Write(EWRPTH,TXSTART_INIT>>8);
	//设置TXND指针，以对应给定的数据包大小	   
	ENC28J60_Write(ETXNDL,(TXSTART_INIT+len)&0xFF);
	ENC28J60_Write(ETXNDH,(TXSTART_INIT+len)>>8);
	//写每包控制字节（0x00表示使用macon3的设置） 
	ENC28J60_Write_Op(ENC28J60_WRITE_BUF_MEM,0,0x00);
	//复制数据包到发送缓冲区
	//printf("len:%d\r\n",len);	//监视发送数据长度
 	ENC28J60_Write_Buf(len,packet);
 	//发送数据到网络
	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET,ECON1,ECON1_TXRTS);
	//复位发送逻辑的问题。参见Rev. B4 Silicon Errata point 12.
	if((ENC28J60_Read(EIR)&EIR_TXERIF))ENC28J60_Write_Op(ENC28J60_BIT_FIELD_CLR,ECON1,ECON1_TXRTS);
}
//从网络获取一个数据包内容
//maxlen:数据包最大允许接收长度
//packet:数据包缓存区
//返回值:收到的数据包长度(字节)									  
uint32_t ENC28J60_Packet_Receive(uint32_t maxlen,uint8_t* packet)
{
	uint32_t rxstat;
	uint32_t len;   
	if( !(ENC28J60_Read(EIR)) & EIR_PKTIF)
	{
		if(ENC28J60_Read(EPKTCNT)==0)
			return 0;  //是否收到数据包?	   
	}
	//设置接收缓冲器读指针
	ENC28J60_Write(ERDPTL,(NextPacketPtr));
	ENC28J60_Write(ERDPTH,(NextPacketPtr)>>8);	   
	// 读下一个包的指针
	NextPacketPtr=ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM,0);
	NextPacketPtr|=ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM,0)<<8;
	//读包的长度
	len=ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM,0);
	len|=ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM,0)<<8;
 	len-=4; //去掉CRC计数
	//读取接收状态
	rxstat=ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM,0);
	rxstat|=ENC28J60_Read_Op(ENC28J60_READ_BUF_MEM,0)<<8;
	//限制接收长度	
	if (len>maxlen-1)len=maxlen-1;	
	//检查CRC和符号错误
	// ERXFCON.CRCEN为默认设置,一般我们不需要检查.
	if((rxstat&0x80)==0)
		len=0;//无效
	else 
		ENC28J60_Read_Buf(len,packet);//从接收缓冲器中复制数据包	    
	//RX读指针移动到下一个接收到的数据包的开始位置 
	//并释放我们刚才读出过的内存
	ENC28J60_Write(ERXRDPTL,(NextPacketPtr));
	ENC28J60_Write(ERXRDPTH,(NextPacketPtr)>>8);
	//递减数据包计数器标志我们已经得到了这个包 
 	ENC28J60_Write_Op(ENC28J60_BIT_FIELD_SET,ECON2,ECON2_PKTDEC);
	return(len);
}



