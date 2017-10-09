/*AUTHOR: STEPHEN SZYMCZAK
 * PROJECT: BUTTON DEBOUNCE G2553
 */

#include <msp430.h> 
int state = 0;
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

//TIMER SETUP
    TA0CCTL0 = CCIE;        // CCR0 interrupt enabled
    TA0CCR0 = 50000;        //overflow every 10ms

//BUTTON SETUP
        P1DIR &= ~BIT3;         //SET P1.3 AS INPUT
        P1REN |= BIT3;          //ENABLED PULL UP OR DOWN FOR P1.3
        P1OUT |= BIT3;          //SPECIFIED AS A PULLUP FOR P1.3
        P1IE |= BIT3;           //SET P1.3INTERRUPT ENABLED
        P1IFG &= ~BIT3;         //P1.3 IFG CLEARED
        P1IES |= BIT3;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW

//LED SETUP
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

    return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT_1(void)
{
        switch(state)
        {
        case 0: //OFF -> GOING ON
            TA0CTL = TASSEL_2 + MC_1 + TACLR;       // ACTIVATE TIMER (goes into case 0 in the timer ISR)
            P1IFG &= ~BIT3;                         // CLEAR FLAG FOR P1.3
            P1IE &= ~BIT3;                          // disable interrupts for P1.3 (button 2)
            break;
        case 1: //ON -> GOING OFF
            TA0CTL = TASSEL_2 + MC_1 + TACLR;       //ACTIVATE TIMER (goes into case 1 in the timer ISR)
            P1IFG &= ~BIT3;                         // CLEAR FLAG FOR P1.3
            P1IE &= ~BIT3;                          // disable interrupts for P1.3 (button 2)
            break;
        }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
        switch(state)
        {
        case 0://GOING ON -> ON
            P1OUT ^= BIT0;              //BLINK LED (OUTPUT)
            P1IE |= BIT3;               //RE-ENABLE INTERRUPTS
            P1IES &= ~BIT3;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
            TA0CTL &= ~TASSEL_2;        //STOP TIMER
            TA0CTL |= TACLR;            //CLEAR TIMER
            state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
            break;
        case 1://GOING OFF -> OFF
            P1IE |= BIT3;               //SET P1.3 INTERRUPT ENABLED (S2)
            P1IFG &= ~BIT3;             //P1.3 IFG CLEARED
            P1IES |= BIT3;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
            TA0CTL &= ~TASSEL_2;        //STOP TIMER
            TA0CTL |= TACLR;            //CLEAR TIMER
            state = 0;                  //UPON ANOTHER BUTTON PRESS, WE WILL ENTER CASE 0 OF PORT 1 ISR
            break;
        }


}

