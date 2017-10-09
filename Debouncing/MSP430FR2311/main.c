/*AUTHOR: STEPHEN SZYMCZAK
 * PROJECT: BIT1 DEBOUNCE FR2311
 */

#include <msp430.h> 

int state = 0;
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    // Disable the GPIO power-on default high-impedance mode to activate
        // previously configured port settings
        PM5CTL0 &= ~LOCKLPM5;
//TIMER SETUP
    TB0CCTL0 = CCIE;        // CCR0 interrupt enable
    TB0CCR0 = 21000;        //overflow every 10ms

//BIT1 SETUP
    //BIT1 2
        P1DIR &= ~BIT1;         //SET P1.1 AS INPUT
        P1REN |= BIT1;          //ENABBIT0 PULL UP OR DOWN FOR P1.1
        P1OUT |= BIT1;          //SPECIFIED AS A PULLUP FOR P1.1
        P1IE |= BIT1;           //SET P1.1 INTERRUPT ENABLE (S2)
        P1IFG &= ~BIT1;         //P1.1 IFG CLEARED
        P1IES |= BIT1;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW

//BIT0 SETUP
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
            TB0CTL = TBSSEL__SMCLK + MC__UP + TBCLR + ID_2;       // ACTIVATE TIMER (goes into case 0 in the timer ISR)
            P1IFG &= BIT1;                         // CLEAR FLAG FOR P1.1
            P1IE &= ~BIT1;                          // disable interrupts for P1.1
            break;
        case 1: //ON -> GOING OFF
            TB0CTL = TBSSEL__SMCLK + MC__UP + TBCLR + ID_2;       //ACTIVATE TIMER (goes into case 1 in the timer ISR)
            P1IFG &= ~BIT1;                         // CLEAR FLAG FOR P1.1
            P1IE &= ~BIT1;                          // disable interrupts for P1.1
            break;
        }
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer_B0 (void)
{
        switch(state)
        {
        case 0://GOING ON -> ON
            P1OUT ^= BIT0;              //BLINK BIT0 (OUTPUT)
            P1IE |= BIT1;               //RE-ENABLE INTERRUPTS
            P1IES &= ~BIT1;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (BIT1 release)
            TB0CTL &= ~TBSSEL__SMCLK;        //STOP TIMER
            TB0CTL |= TBCLR;            //CLEAR TIMER
            state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
            break;
        case 1://GOING OFF -> OFF
            P1IE |= BIT1;               //SET P1.1 INTERRUPT ENABBIT0 (S2)
            P1IFG &= ~BIT1;             //P1.1 IFG CLEARED
            P1IES |= BIT1;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
            TB0CTL &= ~TBSSEL__SMCLK;        //STOP TIMER
            TB0CTL |= TBCLR;            //CLEAR TIMER
            state = 0;                  //UPON ANOTHER BUTTON PRESS, WE WILL ENTER CASE 0 OF PORT 1 ISR
            break;
        }


}

