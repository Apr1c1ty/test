#include <msp430.h>
//测试
#define BSenser_0    ((P1IN&BIT1)==0)         //B-->P1.1
#define BSenser_1    ((P1IN&BIT1)==BIT1)
#define ASenser_0    ((P1IN&BIT0)==0)         //A-->P1.0
#define ASenser_1    ((P1IN&BIT0)==BIT0)

#define BSenEdgSelect_Up    P1IES &=~BIT1  //P1.1上升沿触发中断
#define BSenEdgSelect_Down  P1IES |=BIT1   //P1.1下降沿触发中断
#define ASenEdgSelect_Up    P1IES &=~BIT0  //P1.0上升沿触发中断
#define ASenEdgSelect_Down  P1IES |=BIT0   //P1.0下降沿触发中断



////////////////////////////////////


enum SENSER_status
{
    status_00,
    status_01,
    status_10,
    status_11
};
enum SENSER_status gcurrentstatus;//用于将中断中的状态传递到状态判断函数中


enum MOVEMENT
{
    stop,     //没有旋转
    right,  //从右往左转顺时针
    left,   // 从左往右转逆时针
    error  //错误
};
int flowDirection;
int oldFlowDirection=stop;
int posiPulseNum = 0;
int cnt =  0;
const unsigned char lcd_num[10] = {
           0x7D,  //* "0" *///ZERO
           0x60,  //* "1" *///ONE
           0x3E,  //* "2" *///TWO
           0x7A,  //* "3" *///THREE
           0x63,  //* "4" *///FOUR
           0x5B,  //* "5" *///FIVE
           0x5F,  //* "6" *///SIX
           0x70,  //* "7" *///SEVEN
           0x7F,  //* "8" *///EIGHT
           0x7B,  //* "9" *///NINE
};
// 状态判断

enum MOVEMENT upgatestatus(enum SENSER_status currentstatus)
{
    enum MOVEMENT flag=stop;
    static enum SENSER_status previousstatus;
    switch(currentstatus)
    {
        case status_00:
            switch(previousstatus)
                {
                    case status_00:
                    flag = stop;
                    break;
                    case status_01:
                    flag = left;
                    break;
                    case status_10:
                    flag = right;
                    break;
                    case status_11:
                    flag = error;
                    break;
            }
            break;

        case status_01:
            switch(previousstatus)
                {
                    case status_00:
                    flag = right;
                    break;
                    case status_01:
                     flag = stop;
                    break;
                    case status_10:
                    flag = error;
                    break;
                    case status_11:
                    flag = left;
                    break;
            }
            break;
         case status_10:
            switch(previousstatus)
                {
                    case status_00:
                    flag = left;
                    break;
                    case status_01:
                     flag = error;
                    break;
                    case status_10:
                    flag = stop;
                    break;
                    case status_11:
                    flag = right;
                    break;
            }
            break;

        case status_11:
            switch(previousstatus)
                {
                    case status_00:
                    flag = error;
                    break;
                    case status_01:
                     flag = right;
                    break;
                    case status_10:
                    flag = left;
                    break;
                    case status_11:
                    flag = stop;
                    break;
            }
            break;
        default:
             break;

    }
    previousstatus = currentstatus;
    return flag;
}



void init(void)
{
     WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
     //PJSEL0 = BIT4 | BIT5;                   // For LFXT
     P1DIR &= ~(BIT0|BIT1|BIT2);//输入
     P1IE |= BIT0|BIT1|BIT2;//开中断
    // Initialize LCD segments 0 - 21; 26 - 43
     LCDCPCTL0 = 0xFFFF;
     LCDCPCTL1 = 0xFFFF;
     LCDCPCTL2 = 0x0FFF;
     PM5CTL0 &= ~LOCKLPM5;
         // Configure LFXT 32kHz crystal
     //CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
     //CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
       /* do
         {
          CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
           SFRIFG1 &= ~OFIFG;
        }while (SFRIFG1 & OFIFG);               // Test oscillator fault flag
        CSCTL0_H = 0;           */                // Lock CS registers

         LCDCCTL0 = LCDDIV__5 | LCDPRE__32 | LCD4MUX | LCDLP;
         LCDCVCTL = VLCD_0 | VLCDREF_0 | LCDCPEN;
         LCDCCPCTL = LCDCPCLKSYNC;               // Clock synchronization enabled
         LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory
         LCDCCTL0 |= LCDON;
             LCDM1 = lcd_num[0];
             LCDM2 = lcd_num[0];
             LCDM3 = lcd_num[0];
             LCDM4 = lcd_num[0];
             LCDM5 = lcd_num[0];
             LCDM6 = lcd_num[0];
             LCDM7 = lcd_num[0];
             LCDM7 |= 0X80;
             LCDM11 = lcd_num[0];
             LCDM13 = lcd_num[0];
             LCDM14 = lcd_num[0];
}

void LCDshow(int temp)
{
          // float temp = x*0.06;
            LCDM1 = lcd_num[(unsigned int)temp/1000000%10];
            LCDM2 = lcd_num[(unsigned int)temp/100000%10];
            LCDM3 = lcd_num[(unsigned int)temp/10000%10];
            LCDM7 = lcd_num[(unsigned int)temp/1000%10];
            LCDM5 = lcd_num[(unsigned int)temp/100%10];
            LCDM6 = lcd_num[(unsigned int)temp/10%10];
            LCDM7 = lcd_num[(unsigned int)temp%10];
            LCDM7 |= 0X80;
            //LCDM8 =11111111;
           // LCDM10 = 0X0F;
            LCDM11 = lcd_num[(unsigned int)(temp*1000)/100%10];
            LCDM13 = lcd_num[(unsigned int)(temp*1000)/10%10];
            LCDM14 = lcd_num[(unsigned int)(temp*1000)%10];
}

int flowStatusChange(void)
{
    int flag;
    if(flowDirection == oldFlowDirection)
    {
        flag = 0;
    }
    else
    {
        flag = 1;
    }
    oldFlowDirection = flowDirection;
    return flag;
}
static void turnRightPRG( void )
{
    static unsigned int statusCnt = 0;
    flowDirection = right;
    statusCnt++; // 叶轮转1圈变化16个状态
    if(statusCnt>7)
    {
        statusCnt=0;
        posiPulseNum ++;      // 正向脉冲数自加1（16个状态一个脉冲）
    }
    if(flowStatusChange()==0)
    {

    LCDM10 =0X02;
    }
    else
     {
      LCDM10 = 0;
     }
}
static void turnLeftPRG(void)
{
    flowDirection = left;
    if(flowStatusChange()==0)
       {

    LCDM10 = 0X01;
       }
    else
    {
     LCDM10 = 0;
    }
}

void movementhand(void)
{
     volatile static unsigned int errorCnt=0;
     switch(upgatestatus(gcurrentstatus))
        {
            case right:
            turnRightPRG();
            //float x = posiPulseNum*0.08695621;
            LCDshow(posiPulseNum);
            break;
            case left:
            turnLeftPRG();
            LCDshow(777);
            break;
            case stop:
            case error:
            errorCnt++;
            LCDM10 = 0;
            LCDshow(888);
            break;
            default:
            break;

        }

}
void main(void)
{
    init();
    while(1)
    {
    movementhand();
    LCDCCTL0 |= LCDON;
    __bis_SR_register(LPM3_bits + GIE);     // LPM4 with interrupts enabled
    }
}
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{

    switch(__even_in_range(P1IV, 0x10))  //__even_in_range(P1IV, 0x10) 判断P1IV是否为偶数且小于等于0x10
    {
    case P1IV_P1IFG0:   // P1.0引脚触发中断  A-->P1.1，B-->P1.0

         if( ASenser_0 && BSenser_0 )
         {
            gcurrentstatus=status_00;
             ASenEdgSelect_Up;
             BSenEdgSelect_Up;
             P1IFG &=~BIT0; // 手动清除相应标志位
         }
         else if( ASenser_0 && BSenser_1 )
         {
             gcurrentstatus=status_01;
             ASenEdgSelect_Up;
             BSenEdgSelect_Down;
             P1IFG &=~BIT0; // 手动清除相应标志位
         }
         else if( ASenser_1 && BSenser_0 )
         {
             gcurrentstatus=status_10;
             ASenEdgSelect_Down;
             BSenEdgSelect_Up;
             P1IFG &=~BIT0; // 手动清除相应标志位
         }
         else if( ASenser_1 && BSenser_1 )
         {
             gcurrentstatus=status_11;
             ASenEdgSelect_Down;
             BSenEdgSelect_Down;
             P1IFG &=~BIT0; // 手动清除相应标志位
         }
        cnt++;
         __bic_SR_register_on_exit(LPM3_bits);     // 退出LPM3(LPM3_bits);
         break;
    case P1IV_P1IFG1:  //P1.1引脚触发中断  A-->P1.1，B-->P1.0
    //        WMdata.posiPulseNum++;
      //  delay_us(10);
        if( ASenser_0 && BSenser_0 )
        {
            gcurrentstatus=status_00;
            ASenEdgSelect_Up;
            BSenEdgSelect_Up;
            P1IFG &=~BIT1; // 手动清除相应标志位
        }
        else if( ASenser_0 && BSenser_1 )
        {
            gcurrentstatus=status_01;

            ASenEdgSelect_Up;
            BSenEdgSelect_Down;
            P1IFG &=~BIT1; // 手动清除相应标志位
        }
        else if( ASenser_1 && BSenser_0 )
        {
            gcurrentstatus=status_10;
            ASenEdgSelect_Down;
            BSenEdgSelect_Up;
            P1IFG &=~BIT1; // 手动清除相应标志位
        }
        else if( ASenser_1 && BSenser_1 )
        {
            gcurrentstatus=status_11;
            ASenEdgSelect_Down;
            BSenEdgSelect_Down;
            P1IFG &=~BIT1; // 手动清除相应标志位
        }
        cnt++;
        __bic_SR_register_on_exit(LPM3_bits);     // 退出LPM3(LPM3_bits);
        break;
    case P1IV_P1IFG2:  //P1.2引脚触发中断 干簧管
        P1IFG &=~BIT2; // 手动清除相应标志位
       // _delay_cycles(1000);  // 延时消抖
        if((P1IN&BIT2)==0)
        {
            cnt = 0;
        posiPulseNum = 0;
       // errorCnt = 0;
        LCDshow(0);

        }

        break;
    default:
        break;
    }
}
