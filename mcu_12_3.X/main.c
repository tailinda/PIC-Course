#include <xc.h>
#include <stdio.h>
#include "main.h"
#include "keypad.h"
#include "LCD.h"

/****************** CRITICAL SETUP, MODIFY THEM CAREFULLY *******************/
//F80000 0x00CF
//F80002 0x00CF
//F80004
_FGS(0x0007);
//F80006
_FOSCSEL(0x00A3);
//F80008
_FOSC(0x00E6);
//F8000A
_FWDT(0x77);
//F8000C
_FPOR(0x00E7);
//F8000E
_FICD(0xC2);

//definiation
#define wait_key_over {while( keypad_task() != -1 );}
#define START_str "Press a number"  //15
#define Clear_str    "                    " //21
#define Toolong_str1 "Number must between " //21
#define Toolong_str2 " -2147483648 ~      " //21
#define Toolong_str3 "      +2147483647   " //21
#define Overflow_str "Overflow!" //10
#define notAnumber   " Not a number!!     " //21
#define reinput_str  "Re-enter your value."

//Global variables
long             numA,numB,numC,numD;
int charA[10],charC[10],charD[10];
//unsigned char A[6],B[6],C[6];
char lengthA = 0 ,lengthB = 0 ,lengthC = 0,intemp;

void delay(unsigned long delay_count)
{
    unsigned long       temp;
    for (temp=delay_count; temp>0; temp--);

}

void delay200usX(unsigned long delay_count)
{
    unsigned long       temp;

    for (temp=delay_count; temp>0; temp--)
        delay(Delay_200uS_Cnt);
}

void initPLL(void)
{
    int i,j;

    CLKDIVbits.ROI = 0;
    CLKDIVbits.DOZEN = 0;
    CLKDIVbits.DOZE = 0;

    CLKDIVbits.PLLPRE = 18;       // N1 = PLLPRE + 2 = 18+2 =20
    PLLFBD = 158;               // M = 158+2=160
    CLKDIVbits.PLLPOST = 0;       // N2 = 2           sum: Fosc= {OSC*M/(N1*N2)}=20M*160/(20*2)=80MHz
                                    // Tcy=1/80M*4=50nS
    SRbits.IPL = 0 ;

    //wait for Fosc stablize
    for (i=1000; i>0; i--)
        for (j=1000; j>0; j--);

}


void PORTS_Test_Initial(void)
{
    PORTA = 0xffff;
    TRISA = 0x0000;
    PORTA = 0xffff;

    PORTB = 0xffff;
    TRISB = 0x0000;
    PORTB = 0xffff;

    PORTC = 0xffff;
    TRISC = 0x0000;
    PORTC = 0xffff;

    PORTD = 0xffff;
    TRISD = 0x0000;
    PORTD = 0xffff;

    PORTE = 0xffff;
    TRISE = 0x0000;
    PORTE = 0xffff;

    PORTF = 0xffff;
    TRISF = 0x0000;
    PORTF = 0xffff;

    PORTG = 0xffff;
    TRISG = 0x0000;
    PORTG = 0xffff;
}

unsigned char num2char(int key)     //convert a number to char
{
        switch (key) 
        {
            case 0:
                return '0';
            case 1:
                return '1';
            case 2:
                return '2';
            case 3:
                return '3';
            case 4:
                return '4';
            case 5:
                return '5';
            case 6:
                return '6';
            case 7:
                return '7';
            case 8:
                return '8';
            case 9:
                return '9';
            case 10:
                return 'A';
            case 11:
                return 'B';
            case 12:
                return 'C';
            case 13:
                return 'D';
            case 14:
                return 'E';
            case 15:
                return 'F';
            default:
                break;
        }
}

int num_check(int value[6])
{
    /*return 2 :too long
      return 0 :Okey*/
    if(value[0]>=7) /*length check*/
    {
        if(value[1]>=6)
        {
            if(value[2]>=7)
            {
                if(value[3]>=2)
                {
                    if(value[4]>3)
                    {
                        return 2;
                    }
                }
            }       
        }
    }
    return 0;
}

int num_m2v(int value[6])
{
    int result = 0;
    result = value[0]        + 
             value[1] * 10   +
             value[2] * 100  +
             value[3] * 1000 +
             value[4] * 10000;
    if(value[0]==1)
    {
        LATDbits.LATD11 = 0;                        /* turn the LCM back light */
    }
    if(value[5] != 0)
    {
        result = 0-result;
        return result;
    }
    else
    {
        return result;
    }
}




void num_print(long value,int pos,int line)
{
    //char temp1,temp2,temp3,temp4,temp5,temp6,temp;
    /* In XC16 compiler ,long type has 32 bits, value from -2,147,483,648 ~ +2,147,483,647*/
    long temp;
    unsigned char numcount = 0,forcount;
    if(value < 0)
    {
        temp = value*(-1);
    }
    else
    {
        temp = value;
    }

    LCD_Cursor_New(line, pos);
    
    if(temp > 999999999)
    {
        numcount = 10;
    }
    else if(temp > 99999999)
    {
        numcount = 9;
    }
    else if(temp > 9999999)
    {
        numcount = 8;
    }
    else if(temp > 999999)
    {
        numcount = 7;
    }
    else if(temp > 99999)
    {
        numcount = 6;
    }
    else if(temp > 9999)
    {
        numcount = 5;
    }
    else if(temp > 999)
    {
        numcount = 4;
    }
    else if(temp > 99)
    {
        numcount = 3;
    }
    else if(temp > 9)
    {
        numcount = 2;
    }
    else if(temp <= 9)
    {
        numcount = 1;
    }
    for(;numcount >0;numcount--)
    {
        LCD_Cursor_New(line, pos);
        putcLCD( num2char(temp%10) );
        temp = temp/10;
        pos--;
    }
    if(value < 0)
    {
        LCD_Cursor_New(line, pos);
        putcLCD('-');
        LATDbits.LATD11 = 0;
    }
    
}

void error(int status)
{
    if(status == 1)     //input number too long
    {
        LCD_clear();
        LCD_Cursor_New(0, 0);
        putsLCD(Toolong_str1);
        LCD_Cursor_New(1, 0);
        putsLCD(Toolong_str2);
        LCD_Cursor_New(2, 0);
        putsLCD(Toolong_str3);
        delay200usX(10000);
    }
    if(status == 2)
    {
        LCD_clear();
        LCD_Cursor_New(0, 0);
        putsLCD(Clear_str);
        
        delay200usX(10000);
    }
}

void caculate_print(char value , char posX , char posY)
{
    switch(value)
    {
        LCD_Cursor_New(posX, posY);
        case 0:
            putcLCD('+');
        case 1:
            putcLCD('-');
        case 2:
            putcLCD('×');
        case 3:
            putcLCD('÷');
    }
}

char function(char how)
{
    switch(how)
    {
        case 0:
            numC = numA+numB;
            if(numA>0 && numC<0)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        case 1:
            numC = numA-numB;
            if(numA<0 && numC>=0)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        case 2:
            numC=numA*numB;
            if(numA>0 && numB>0 && numC>0)
            {
                return 1;
            }
            else if(numA<0 && numB>0 && numC<0)
            {
                return 1;
            }
            else if(numA>0 && numB<0 && numC<0)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        case 3:
            numC=numA/numB;
            numD=numA/numB;
            return 1;
            
    }
}

int main(void)
{
    unsigned char   temp[32];
    unsigned char   input_status = 0;   //0:numA   1:numB   2:result 3:error
    unsigned char   firstA = 1,firstB = 1,firstC = 1,firstD = 1,
                    position = 0,
                    caculate = 0,   //0=+ , 1=- , 2=* , 3=/
                    i;
    RCONbits.SWDTEN = 0;			//disable Watch dog timer
    initPLL();

    while (OSCCONbits.CF == 1);			//check clock failed
    while (OSCCONbits.COSC != 0b011);           // Wait for Clock switch to occur
    while (OSCCONbits.LOCK == 0);		//check PLL locked

    //T1_Initial();
    PORTS_Test_Initial();
    InitializeLCM();
    LATDbits.LATD11 = 1;                        /* turn the LCM back light */
    keypad_init();

    LCD_start();
    LCD_Delay200usX(5000);
    LCD_clear();
    while (1) 
    {
        LCD_Cursor_New(0, 0);
        putsLCD(Clear_str);
        LCD_Cursor_New(0, 0);
        putsLCD(START_str);
        LCD_Cursor_New(0, 0);
        /* input_status == 1 => numA */
        while(input_status == 0)
        {
            if( (intemp == keypad_task() ) && intemp != -1 )    
            {
                wait_key_over;
            }
            
            
            intemp = keypad_task();
            
            if(firstA && intemp<10 && intemp != -1 )    //press a number for first time
            {
                LCD_Cursor_New(0, 0);
                putsLCD(Clear_str);
                LCD_Cursor_New(0, position=19);
                firstA = 0;
            }
            else if(firstA && intemp>=10)    //press not a number or '+','-' for first time
            {
                LCD_Cursor_New(0, 0);
                putsLCD(Clear_str);
                LCD_Cursor_New(0, 0);
                putsLCD(notAnumber);
                delay200usX(5000);
                LCD_clear();
                LCD_Cursor_New(0, 0);
                putsLCD(START_str);
                continue;
            }
            
            if(intemp<10 && intemp != -1 )  // press a number
            {
                if(lengthA==0)
                {
                    numA = intemp;
                    lengthA++;
                }
                else if(numA < 214748364)
                {
                    numA *= 10;
                    numA += intemp;
                    lengthA++;
                }
                else if(numA > 214748364)
                {
                    error(1);
                    numA = 0;
                    lengthA = 0;
                    LCD_clear();
                    LCD_Cursor_New(0, 0);
                    putsLCD(START_str);
                    LCD_Cursor_New(0, 0);
                    firstA = -1;
                    continue;
                }
                else if(numA = 214748364)
                {
                    if(intemp <=7)
                    {
                        numA *= 10;
                        numA += intemp;
                        lengthA++;
                    }
                    else
                    {
                        error(1);
                        numA = 0;
                        lengthA = 0;
                        LCD_clear();
                        LCD_Cursor_New(0, 0);
                        putsLCD(START_str);
                        LCD_Cursor_New(0, 0);
                        firstA = -1;
                        continue;
                    }
                }
                num_print(numA,19,0);
            }
            else if(intemp == 10)   //press '+'
            {
                input_status = 1;
                num_print(numA,19,0);
                LCD_Cursor_New(1, 0);
                putcLCD('+');
                caculate = 0;
                continue;
                
            }
            else if(intemp == 11)   //press '-'
            {
                input_status = 1;
                LCD_Cursor_New(1, 0);
                putcLCD('-');
                caculate = 1;
                continue;
            }
            else if(intemp == 14)   //press '*'
            {
                input_status = 1;
                LCD_Cursor_New(1, 0);
                putcLCD('×');
                caculate = 2;
                continue;
            }
            else if(intemp == 15)   //press '/'
            {
                input_status = 1;
                LCD_Cursor_New(1, 0);
                putcLCD('÷');
                caculate = 3;
                continue;
            }
            else if(intemp == 12)    //press 'clear'
            {
                numA = 0;
                numB = 0;
                numC = 0;
                numD = 0;
                lengthA = 0;
                lengthB = 0;
                lengthC = 0;
                firstA = -1;
                firstB = -1;
                firstC = -1;
                firstD = -1;
                position = 0;
                input_status = 0;
                break;
            }
            else if(intemp == 13)    //press '='
            {
                input_status = 2;
            }
        }
  /********************************************************************************/   
  /********************************************************************************/   
  /**********************************    NUMB    **********************************/         
  /********************************************************************************/         
  /********************************************************************************/      
        while(input_status == 1)    //numA has value to caculate
        {
            if( (intemp == keypad_task() ) && intemp != -1 )    
            {
                wait_key_over;
            }
            intemp = keypad_task();
            
            if(firstB && intemp<10 && intemp != -1 )    //press a number for first time
            {
               // LCD_Cursor_New(2, 0);
                //putsLCD(Clear_str);
                //LCD_Cursor_New(0, position=19);
                firstB = 0;
            }
            else if(firstB && intemp>=10)    //press not a number for first time
            {
                if(intemp == 10)    //+
                {
                    if(caculate != 0)
                    {
                        num_print(numA,0,19);
                        LCD_Cursor_New(1, 0);
                        putcLCD('+');
                    }
                    caculate = 0;
                }
                else if(intemp == 11)       //-
                {
                    if(caculate != 1)
                    {
                        num_print(numA,0,19);
                        LCD_Cursor_New(1, 0);
                        putcLCD('-');
                    }
                    caculate = 1;
                }
                else if(intemp == 12)       //clear
                {
                    numA = 0;
                    numB = 0;
                    numC = 0;
                    numD = 0;
                    lengthA = 0;
                    lengthB = 0;
                    lengthC = 0;
                    firstA = -1;
                    firstB = -1;
                    firstC = -1;
                    firstD = -1;
                    position = 0;
                    input_status = 0;
                    break;
                }
                else if (intemp == 13)      //=
                {
                    
                }
                else if (intemp == 14)      //*
                {
                    if(caculate != 2)
                    {
                        num_print(numA,0,19);
                        LCD_Cursor_New(1, 0);
                        putcLCD('×');
                    }
                    caculate = 2;
                }
                else if (intemp == 15)      ///
                {
                    if(caculate != 3)
                    {
                        num_print(numA,0,19);
                        LCD_Cursor_New(1, 0);
                        putcLCD('÷');
                    }
                    caculate = 3;
                }
                continue;
            }
            
            if(intemp<10 && intemp != -1 )  // press a number
            {
                if(lengthB==0)
                {
                    numB = intemp;
                    lengthB++;
                }
                else if(numB < 214748364)
                {
                    numB *= 10;
                    numB += intemp;
                    lengthB++;
                }
                else if(numB > 214748364)
                {
                    error(1);
                    numB = 0;
                    lengthB = 0;
                    LCD_Cursor_New(3, 0);
                    putsLCD(reinput_str);
                    num_print(numA,0,19);
                    caculate_print(caculate,1,0);
                    firstB = -1;
                    continue;
                }
                else if(numA = 214748364)
                {
                    if(intemp <=7)
                    {
                        numA *= 10;
                        numA += intemp;
                        lengthA++;
                    }
                    else
                    {
                        error(1);
                        numB = 0;
                        lengthB = 0;
                        LCD_Cursor_New(3, 0);
                        putsLCD(reinput_str);
                        num_print(numA,0,19);
                        caculate_print(caculate,1,0);
                        firstB = -1;
                        continue;
                    }
                }
                num_print(numB,1,19);
            }
            else if(intemp == 10)   //press '+'
            {
                
                continue;
                
            }
            else if(intemp == 11)   //press '-'
            {
                
                continue;
            }
            else if(intemp == 14)   //press '*'
            {
                
                continue;
            }
            else if(intemp == 15)   //press '/'
            {
                
                continue;
            }
            else if(intemp == 12)    //press 'clear'
            {
                numA = 0;
                numB = 0;
                numC = 0;
                numD = 0;
                lengthA = 0;
                lengthB = 0;
                lengthC = 0;
                firstA = -1;
                firstB = -1;
                firstC = -1;
                firstD = -1;
                position = 0;
                input_status = 0;
                break;
            }
            else if(intemp == 13)    //press '='
            {
                input_status = 1;
            }
            
            
        }
            
        //num_print(0-numA,19,1);
    }
}


