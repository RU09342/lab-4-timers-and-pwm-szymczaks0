/*AUTHOR: STEPHEN SZYMCZAK
* PROJECT: HARDWARE PWM FOR FR2311
*/
#include <msp430.h> 

int pwm = 500;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;            // Disable the GPIO power-on default high-impedance mode
                                     // to activate previously configured port settings
//led setup:
    P2DIR |= BIT0;            // P2.0 as output
    P2SEL0 |= BIT0;                  // P2.0 option select
//debounce timer setup:
    TB0CCTL0 = CCIE;                    // CCR1 interrupt enabled
    TB0CCR0 = 10000;                    //overflow every 10ms FOR DEBOUNCE
//pwm timer setup
    TB1CCTL1 = OUTMOD_7;                // CCR1 reset/set
    TB1CCR0 = 1000-1;                   //overflow every 1ms FOR PWM PERIOD
    TB1CCR1 = 500;                      //PWM starts at 50%
    TB1CTL = TBSSEL__SMCLK + MC__UP + TBCLR + ID_2;           //SMCLK, up mode
//button setup:
    P1DIR &= ~BIT1;                 //SET P1.1 AS INPUT
    P1REN |= BIT1;                  //ENABLED PULL UP OR DOWN FOR P1.1
    P1OUT |= BIT1;                  //SPECIFIED AS A PULLUP FOR P1.1
    P1IE |= BIT1;                   //SET P1.1 INTERRUPT ENABLED (S2)
    P1IFG &= ~BIT1;                 //P1.1 IFG CLEARED

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

    return 0;
}

#pragma vector=PORT1_VECTOR//button interrupt
__interrupt void PORT_1(void)
{

    TB0CTL = TBSSEL__SMCLK + MC__UP + TBCLR + ID_2;                  // ACTIVATE TIMER

    P1IFG &= ~BIT1;     //P1.1 IFG CLEARED
    P1IES &= ~BIT1;      //TOGGLE INTERRUPT EDGE
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer0_A0 (void)
{
    if(pwm < 1000) pwm += 100;
    else pwm = 0;
    P1IE |= BIT1;           //ENABLE INTERRUPT
    TB1CCR1 = pwm;    //PWM starts at 50% and increase by PWM (100) then reset to zero
    TB0CTL = 0x00;          //STOP TIMER
}

