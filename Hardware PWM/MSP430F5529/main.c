/*AUTHOR: STEPHEN SZYMCZAK
* PROJECT: HARDWARE PWM FOR F5529
*/
#include <msp430.h> 

int pwm = 500;

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;   // stop watchdog timer

//led setup:
    P1DIR |= BIT2;            // P1.0 as output
    P1SEL |= BIT2;                  // P1.0 option select
//debounce timer setup:
    TA1CCTL0 = CCIE;                    // CCR1 interrupt enabled
    TA1CCR0 = 10000;                    //overflow every 10ms FOR DEBOUNCE
//pwm timer setup
    TA0CCTL1 = OUTMOD_7;                // CCR1 reset/set
    TA0CCR0 = 1000-1;                   //overflow every 1ms FOR PWM PERIOD
    TA0CCR1 = 500;                      //PWM starts at 50%
    TA0CTL = TASSEL_2 | MC_1;           //SMCLK, up mode
//button setup:
    P1DIR &= ~BIT1;                 //SET P1.3 AS INPUT
    P1REN |= BIT1;                  //ENABLED PULL UP OR DOWN FOR P1.3
    P1OUT |= BIT1;                  //SPECIFIED AS A PULLUP FOR P1.3
    P1IE |= BIT1;                   //SET P1.3 INTERRUPT ENABLED (S2)
    P1IFG &= ~BIT1;                 //P1.3 IFG CLEARED

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

    return 0;
}

#pragma vector=PORT1_VECTOR//button interrupt
__interrupt void PORT_1(void)
{

    TA1CTL = TASSEL_2 + MC_1;                  // ACTIVATE TIMER

    P1IFG &= ~BIT1;     //P1.3 IFG CLEARED
    P1IES &= ~BIT1;      //TOGGLE INTERRUPT EDGE
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
    if(pwm < 1000) pwm += 100;
    else pwm = 0;
    P1IE |= BIT1;           //ENABLE INTERRUPT
    TA0CCR1 = pwm;    //PWM starts at 50% and increase by PWM (100) then reset to zero
    TA1CTL = 0x00;          //STOP TIMER
}

