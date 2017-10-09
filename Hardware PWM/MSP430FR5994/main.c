/*AUTHOR: STEPHEN SZYMCZAK
* PROJECT: HARDWARE PWM FOR FR5994
*/
#include <msp430.h> 

int pwm = 500;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;            // Disable the GPIO power-on default high-impedance mode
                                     // to activate previously configured port settings
//led setup:
    P1DIR |= BIT0;            // P1.0 as output
    P1SEL0 |= BIT0;                  // P1.0 option select
//debounce timer setup:
    TA1CCTL0 = CCIE;                    // CCR1 interrupt enabled
    TA1CCR0 = 10000;                    //overflow every 10ms FOR DEBOUNCE
//pwm timer setup
    TA0CCTL1 = OUTMOD_7;                // CCR1 reset/set
    TA0CCR0 = 1000-1;                   //overflow every 1ms FOR PWM PERIOD
    TA0CCR1 = 500;                      //PWM starts at 50%
    TA0CTL = TASSEL_2 | MC_1;           //SMCLK, up mode
//button setup:
    P5DIR &= ~BIT5;                 //SET P5.5 AS INPUT
    P5REN |= BIT5;                  //ENABLED PULL UP OR DOWN FOR P5.5
    P5OUT |= BIT5;                  //SPECIFIED AS A PULLUP FOR P5.5
    P5IE |= BIT5;                   //SET P5.5 INTERRUPT ENABLED (S2)
    P5IFG &= ~BIT5;                 //P5.5 IFG CLEARED

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

    return 0;
}

#pragma vector=PORT5_VECTOR//button interrupt
__interrupt void PORT_5(void)
{

    TA1CTL = TASSEL_2 + MC_1;                  // ACTIVATE TIMER

    P5IFG &= ~BIT5;     //P5.5 IFG CLEARED
    P5IES &= ~BIT5;      //TOGGLE INTERRUPT EDGE
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
    if(pwm < 1000) pwm += 100;
    else pwm = 0;
    P5IE |= BIT5;           //ENABLE INTERRUPT
    TA0CCR1 = pwm;    //PWM starts at 50% and increase by PWM (100) then reset to zero
    TA1CTL = 0x00;          //STOP TIMER
}

