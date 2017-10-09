/*AUTHOR: STEPHEN SZYMCZAK
* PROJECT: SOFTWARE PWM FOR FR5994
*/
#include <msp430.h>

int pwm = 500;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

    PM5CTL0 &= ~LOCKLPM5;   // Disable the GPIO power-on default high-impedance mode to activate
                               // previously configured port settings
//output configure
    P1OUT &= ~BIT1;
    P1DIR |= BIT1;

//PWM TIMER CONFIGURE
    TA0CCTL1 = CCIE;
    TA0CCTL0 = CCIE;
    TA0CCR1 = pwm;
    TA0CCR0 = 1000;
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR | TAIE; //smclock, up mode, clear timer to zero, enable timer a interrupts

//debounce timer configure
    TA1CCTL0 = CCIE;                    // CCR1 interrupt enabled
    TA1CCR0 = 10000;                    //overflow every 10ms FOR DEBOUNCE

//button configure
    P5DIR &= ~BIT5;                         //SET P5.5 AS INPUT
    P5REN |= BIT5;                          //ENABLED PULL UP OR DOWN FOR P5.5
    P5OUT |= BIT5;                          //SPECIFIED AS A PULLUP FOR P5.5
    P5IE |= BIT5;                           //SET P5.5 INTERRUPT ENABLED (S2)
    P5IFG &= ~BIT5;                         //P5.5 IFG CLEARED

    __bis_SR_register(LPM3_bits | GIE);     // Enter LPM3, enable interrupts
    __no_operation();                       // For debugger
}


#pragma vector=TIMER0_A1_VECTOR // ISR THAT HANDLES CCR1 INTERRUPT (INTERRUPTS AT VALUE OF int pwm, where pwm <= 1000)
__interrupt void TIMER0_A1_ISR(void)

{
    switch(__even_in_range(TA0IV, TAIV__TAIFG))
    {

        case TAIV__TACCR1:
            P1OUT &= ~BIT1; // CCR1 PWM
            break;
        default: break;
    }
}
#pragma vector=TIMER0_A0_VECTOR // ISR THAT HANDLES CCR0 INTERRUPT (INTERRUPTS AT 1000)
__interrupt void TIMER0_A0_ISR(void)

{
    if(pwm == 0);
    else P1OUT |= BIT1; //CCR0 PWM
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
    if(TA1CCR0)         //IF THERE HASN'T BEEN AN INTERRUPT BY THE BUTTON IN 10MS
    {                   //COMMIT TO BUTTON PRESS
    if(pwm < 1000) pwm += 100;
    else pwm = 0;
    P5IE |= BIT5;           //ENABLE INTERRUPT
    TA0CCR1 = pwm;          //PWM starts at 50% and increase by PWM (100) then reset to zero
    TA1CTL = 0x00;          //STOP TIMER
    }
}
