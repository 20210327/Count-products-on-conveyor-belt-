#include <REGX52.H>
#include <stdio.h>
#include <string.h>
#define SCK P3_0
#define SDA P3_1
#define LCD_RS  P0_0
#define LCD_RW  P0_1
#define LCD_EN  P0_2
#define LCD_D4  P0_3
#define LCD_D5  P0_4
#define LCD_D6  P0_5
#define LCD_D7  P0_6
#define R1 P2_0
#define R2 P2_1
#define R3 P2_2
#define R4 P2_3
#define C1 P2_4
#define C2 P2_5
#define C3 P2_6
#define C4 P2_7
#define out P3_2
#define run P3_4
#define stop P3_3
#define stop_led P1_2
#define led P1_0
#define in1 P1_7
#define in2 P1_6
#define in3 P1_5
#define in4 P1_4
#define lcd_clear 0x01

/////////////////*Cac ham giao tiep LCD 4 bit*//////////////////
////////////////////////////////////////////////////////////////
void delay_us(unsigned int t) 
{
        unsigned int i;
        for(i=0; i < t ;i++);
}

void delay_ms(unsigned int t){ 
        unsigned int i;
	      while( t --){
				   for( i = 0 ; i< 123; i++){
					 }
				}
}

void LCD_Enable(void){
        LCD_EN =1;
        delay_us(3);
        LCD_EN =0;
        delay_us(50); 
}

void LCD_Send4Bit(unsigned char Data){                   //Ham Gui 4 Bit Du Lieu Ra LCD
        LCD_D4=Data & 0x01;
        LCD_D5=(Data>>1)&1;
        LCD_D6=(Data>>2)&1;
        LCD_D7=(Data>>3)&1;
}

void LCD_SendCommand(unsigned char command){            // Ham Gui 1 Lenh Cho LCD
        LCD_Send4Bit(command >>4);                      /* Gui 4 bit cao */
        LCD_Enable();
        LCD_Send4Bit(command);                          /* Gui 4 bit thap*/
        LCD_Enable();
}

void LCD_Init(){              // Ham Khoi Tao LCD
        LCD_Send4Bit(0x00);	  //turn on LCD
        
        LCD_RS=0;
        LCD_RW=0;
        
     LCD_Enable();
        LCD_Send4Bit(0x03);	  //function setting
          
     LCD_Enable();
	
        LCD_Send4Bit(0x02);	  //di chuyen con tro ve dau man hnh
     LCD_Enable();
        LCD_SendCommand( 0x28 ); //lua chon che do 4 bit
        LCD_SendCommand( 0x0c);  // bat hien thi va tat con tro di
        LCD_SendCommand( 0x06 ); // tang ID, khong dich khung
        LCD_SendCommand(0x01);  //xoa toan bo khung hinh
}

void LCD_Gotoxy(unsigned char x, unsigned char y){ // chon vi tri con tro LCD
        unsigned char address;
        if(!y)address=(0x80+x);
        else address=(0xc0+x);
        delay_us(1000);
        LCD_SendCommand(address);
        delay_us(50);
}

void LCD_Put(unsigned char Data){
        LCD_RS=1;
        LCD_SendCommand(Data);
        LCD_RS=0 ;
}

void LCD_Puts(char *s){ // ghi mot chuoi len LCD
        while (*s){
                LCD_Put(*s);
                s++;
        }
}

void LCD_Clear(){ // xoa man hinh LCD
	      LCD_SendCommand(lcd_clear);
	      delay_us(10);
}
////////////////*cac ham giao tiep I2C voi eeprom*///////////////
/////////////////////////////////////////////////////////////////
void I2C_Start() // Bat dau I2C
{
	SCK = 0;
	SDA = 1;
	delay_us(1);
	SCK = 1;
	delay_us(1);
	SDA = 0;
	delay_us(1);
	SCK = 0;
}



void I2C_Stop() // ket thuc I2C
{
	SCK = 0;
	delay_us(1);
	SDA = 0;
	delay_us(1);
	SCK = 1;
	delay_us(1);
	SDA = 1;
}
 void I2C_Ack()
{
	
	SDA = 0;
	delay_us(1);
	SCK = 1;
	delay_us(1);
	SCK = 0;
	SDA = 1;
}



void I2C_Nak()
{
	SDA = 1;
	delay_us(1);
	SCK = 1;
	delay_us(1);
	SCK = 0;
	SDA = 1;
}

void I2C_Send(unsigned char Data) // gui du lieu den slave
{
	 unsigned char i;
	 for (i = 0; i < 8; i++) {
		
		SDA=Data & 0x80;
		SCK = 1;
	 	SCK = 0;
		Data<<=1;
	 }
	 I2C_Ack();
}


unsigned char I2C_Read() // doc du lieu tu slave
{
	unsigned char i, Data=0;
	for (i = 0; i < 8; i++) {
		delay_us(1);
		SCK = 1;
		Data|=SDA;
		if(i<7)
		Data<<=1;
		SCK = 0;
	}
	return Data;
}

void _24C04_Write(unsigned char StartAddress, unsigned char BufferToWrite){ // ghi 1 byte vao eeprom 24C04
	I2C_Start();
	I2C_Send(0xA0);
	I2C_Send(StartAddress);
	I2C_Send(BufferToWrite);
	I2C_Stop();
}

unsigned char _24C04_Read(unsigned char StartAddress){ // doc 1 byte tu eeprom 24C04
	unsigned char BufferToRead;
	I2C_Start();
	I2C_Send(0xA0);
	I2C_Send(StartAddress);
	I2C_Start();
	I2C_Send(0xA1);
	BufferToRead=I2C_Read();
	I2C_Nak();
	I2C_Stop();
	
	return BufferToRead;
}

void _24C04_Saved(void); // hien thi man hinh cho phep chon du lieu da luu truoc do

idata int sensor_1=1;
idata int pre_out;
idata int soSanPham=0;
idata int soDem=0;
idata int soLuong[10]={10,10,10,10,10,10,10,10,10,10};
idata int so_dem[10]={10,10,10,10,10,10,10,10,10,10};
idata int i=0;
idata int k;
idata int number;
idata int dem=0;
idata char Lcd_Buff[10];
idata int check_dem;
idata int check_quet= 1;
idata int MP;

void hien_thi(){ // hien thi len LCD du lieu trong qua trinh chay
	LCD_Gotoxy(0,0);
	LCD_Puts("NUMBER: ");
	sprintf(Lcd_Buff,"%d",soSanPham);
	strcat(Lcd_Buff,"    ");
	LCD_Puts(Lcd_Buff);
	LCD_Gotoxy(0,1);
	LCD_Puts("COUNT: ");
	sprintf(Lcd_Buff,"%d",dem);
	for (i=strlen(Lcd_Buff)-1;i>=0;i--){ // ghi bien dem vao eeprom
		_24C04_Write(24-strlen(Lcd_Buff)+i,Lcd_Buff[i]);
		delay_ms(1);
	}
	strcat(Lcd_Buff,"    ");
	LCD_Gotoxy(8,1);
	LCD_Puts(Lcd_Buff);
}

void bang_tai(int x){// bang tai hoat dong 
    if ( x == 0){
		  in1=0;
      in2=0;
		}
		else{
			in1=1;
      in2=0;
		}
}
void dong_thung(int x){ // dong goi san pham
    if ( x == 0){
		  in3=0;
      in4=0;
		}
		else{
			in3=1;
      in4=0;
		}   	
}

void sensor1() interrupt 3
{
	TF1=0;
	sensor_1=0;
}

void dem_so() interrupt 0
{
	if (out==0 && pre_out==1){
		if (sensor_1==1)
		{
			if(check_dem==1){
				dem++;
			}
			else if(check_dem==2){
				dem--;
			}
		}
		if (sensor_1==0)
		{
			if (dem >0)
			{
				if(check_dem==1){
					dem--;
				}
				else if(check_dem==2){
					dem++;
				}
			}
		}
		delay_ms(300);
	}
	pre_out=out;
	sensor_1=1;
}

void stop_he() interrupt 2 // che do dung khan cap
{
		EX0=0;
		TR1=0;
		stop_led=~stop_led;
		bang_tai(0);
		dong_thung(0);
		while(1){
			if (run==0){
				stop_led=~stop_led;
				bang_tai(1);
				hien_thi();
				EX0=1;
				TR1=1;
				break;
			}
		}
}

void dem_tien(){ // cho phep dem tien
	while (dem<=soSanPham){
		bang_tai(1);
		EX0=1;
		pre_out=1;
		hien_thi();
		if (dem>=soSanPham){
			EX0=0;
			hien_thi();
			led=1;
			bang_tai(0);
			dong_thung(1);
			delay_ms(1000);
			dong_thung(0);
			led=0;
			dem=soDem;
			LCD_Gotoxy(8,1);
			LCD_Puts("          ");
		}
	}
}

void dem_lui(){ // cho phep dem lui
	while (dem>=soSanPham){
		bang_tai(1);
		EX0=1;
		pre_out=1;
		hien_thi();
		if (dem<=soSanPham){
			EX0=0;
			hien_thi();
			led=1;
			bang_tai(0);
			dong_thung(1);
			delay_ms(1000);
			dong_thung(0);
			led=0;
			dem=soDem;
			LCD_Gotoxy(8,1);
			LCD_Puts("         ");
		}
	}
}

void nhap_phim(){ // cho phep nhap phim
	  R1 = 0; R2 = 1; R3 = 1; R4 = 1;
		while(C1==0){ MP = 7;}
		while(C2==0){ MP = 8;}
		while(C3==0){ MP = 9;}
		while(C4==0){ MP = 96;}
		R1 = 1; R2 = 0; R3 = 1; R4 = 1;
		while(C1==0){ MP = 4;}
		while(C2==0){ MP = 5;}
		while(C3==0){ MP = 6;}
		while(C4==0){ MP = 96;}
		R1 = 1; R2 = 1; R3 = 0; R4 = 1;
		while(C1==0){ MP = 1;}
		while(C2==0){ MP = 2;}
		while(C3==0){ MP = 3;}
		while(C4==0){ MP = 96;}
		R1 = 1; R2 = 1; R3 = 1; R4 = 0;
		while(C1==0){ MP = 42;}
		while(C2==0){ MP = 0;}
		while(C3==0){ MP = 96;}
		while(C4==0){ MP = 96;}
}

void chon_mode(){ // chon che do
	LCD_Clear();
  LCD_Gotoxy(0,0);
	LCD_Puts("1. COUNT UP");
	LCD_Gotoxy(0,1);
	LCD_Puts("2. COUNT DOWN");
	while(1){
		check_dem=0;
		R1=1;R2=1;R3=0;R4=1;
		while(C1==0){check_dem=1;}
		while(C2==0){check_dem=2;}
		R1=1;R2=1;R3=1;R4=0;
		while(C4==0){check_dem=3;}
		if(check_dem==1 | check_dem==2){
			delay_ms(10);
			sprintf(Lcd_Buff,"%d",check_dem);
			_24C04_Write(31,Lcd_Buff[0]);
			delay_ms(1);
			break;
		}
		else if(check_dem==3){
			break;
		}
	}
	if(check_dem==3){
		_24C04_Saved();
	}
}

void _24C04_Saved(void){
	LCD_Clear();
	LCD_Gotoxy(0,0);
	LCD_Puts("3. SAVED DATA");
	while(1){
		MP=0;
		R1=1;R2=1;R3=0;R4=1;
		while(C3==0){MP=3;}
		while(C4==0){MP=96;}
		if(MP==3){
			soSanPham=0;
			dem=0;
			soDem=0;
			for(i=0;i<8;i++){
				number=(int)_24C04_Read(i)-48;
				for(k=0;k<8-i;k++){
					number*=10;
				}
				soSanPham+=number;
			}
			soSanPham=soSanPham/10;
			for(i=0;i<8;i++){
				number=(int)_24C04_Read(i+8)-48;
				for(k=0;k<8-i;k++){
					number*=10;
				}
				soDem+=number;
			}
			soDem=soDem/10;
			for(i=0;i<8;i++){
				number=(int)_24C04_Read(i+16)-48;
				for(k=0;k<8-i;k++){
					number*=10;
				}
				dem+=number;
			}
			dem=dem/10;
			check_dem=(int)_24C04_Read(31)-48;
			delay_ms(1);
			LCD_Clear();
			if(check_dem==1){
				dem_tien();
			}
			if(check_dem==2){
				dem_lui();
			}
		}
		if(MP==96){
			break;
		}
	}
	if(MP==96){
		chon_mode();
	}
}

void quet_phim(){ // quet phim de nhap so lieu
	soSanPham=0;
	soDem=0;
	i=0;
	LCD_Clear();
	LCD_Gotoxy(0,0);
	LCD_Puts("NUMBER: ");
	while(1){ // nhap soSanPham
		MP=100;
		nhap_phim();
		if(MP!=100){
		if ((MP/10)==0){
			soLuong[i]=MP;
			i++;
			sprintf(Lcd_Buff,"%d",MP);
			LCD_Puts(Lcd_Buff);	
		}
		else if(MP==42){
			number=i-1;
			for (i=number;i>=0;i--){
				for (k=0;k<=number-i;k++){
					soLuong[i]*=10;
				}
				soSanPham+=soLuong[i];
			}
			soSanPham=soSanPham/10;
			sprintf(Lcd_Buff,"%d",soSanPham);
			for (i=0;i<24;i++){ // khoi tao eeprom voi cac so 0
				_24C04_Write(i,0x30);
				delay_ms(1);
			}
			for (i=strlen(Lcd_Buff)-1;i>=0;i--){ // ghi so san pham vao eeprom
				_24C04_Write(8-strlen(Lcd_Buff)+i,Lcd_Buff[i]);
				delay_ms(1);
			}
			strcat(Lcd_Buff,"    ");
			LCD_Gotoxy(8,0);
			LCD_Puts(Lcd_Buff);
			break;
		}
		else{ 
			if(i>0){
				soLuong[i-1]=0;
				LCD_Gotoxy(7+i,0);
				LCD_Puts(" ");
				i--;
				LCD_Gotoxy(8+i,0);
			}
		}
	}
	}
	LCD_Gotoxy(0,1);
	LCD_Puts("COUNT:  ");
	i=0;
	while(1){ // nhap soDem
		MP=100;
		nhap_phim();
		if(MP!=100){
		if ((MP/10)==0){
			so_dem[i]=MP;
			i++;
			sprintf(Lcd_Buff,"%d",MP);
			LCD_Puts(Lcd_Buff);	
		}
		else if(MP==42){
			number=i-1;
			for (i=number;i>=0;i--){
				for (k=0;k<=number-i;k++){
					so_dem[i]*=10;
				}
				soDem+=so_dem[i];
			}
			soDem=soDem/10;
			sprintf(Lcd_Buff,"%d",soDem);
			for (i=strlen(Lcd_Buff)-1;i>=0;i--){ // ghi so dem vao eeprom
				_24C04_Write(16-strlen(Lcd_Buff)+i,Lcd_Buff[i]);
				delay_ms(1);
			}
			strcat(Lcd_Buff,"    ");
			LCD_Gotoxy(8,1);
			LCD_Puts(Lcd_Buff);
			break;
		}
		else{ 
			if(i>0){
				so_dem[i]=0;
				LCD_Gotoxy(7+i,1);
				LCD_Puts(" ");
				i--;
				LCD_Gotoxy(8+i,1);
			}
		}
	}
	}
	dem=soDem;
	while(1){
		if(check_dem==1 & soSanPham<soDem){
			check_quet=0;
			LCD_Clear();
			LCD_Gotoxy(0,0);
			LCD_Puts("ERROR! TRY AGAIN!"); // nhap so lieu khong hop le
			delay_ms(1000);
			break;
		}
		else if(check_dem==2 & soSanPham>soDem){
			check_quet=0;
			LCD_Clear();
			LCD_Gotoxy(0,0);
			LCD_Puts("ERROR! TRY AGAIN!"); // nhap so lieu khong hop le
			delay_ms(1000);
			break;
		}
		if(check_dem==1){
			dem_tien();
		}
		if(check_dem==2){
			dem_lui();
		}
	}
}

void init(){ // khoi tao
	EA = 1;
	EX0 = 1;
	EX1=1;
	ET1=1;
	IT0=1;
	IT1=1;
	TMOD=0x60;
	TL1=0xFF;
	TH1=0xFF;
	TR1=1;
	led=0;
	stop_led=0;
	bang_tai(0);
	dong_thung(0);
}

void main(){
	init();
	LCD_Init();
	chon_mode();
	quet_phim();
	while(check_quet==0){
		check_quet=1;
		chon_mode();
		quet_phim();
	}
}