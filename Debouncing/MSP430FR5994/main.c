/*AUTHOR: STEPHEN SZYMCZAK
 * PROJECT: BIT5 DEBOUNCE FR5994
 */

#include <msp430.h> 

int state = 0;
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                                  // to activate previously configured port settings
//TIMER SETUP
    TA0CCTL0 = CCIE;        // CCR0 interrupt ENABLE
    TA0CCR0 = 25000;        //overflow every 10ms

//BUTTON SETUP
    //BUTTON
        P5DIR &= ~BIT5;         //SET P5.5 AS INPUT
        P5REN |= BIT5;          //ENABLE PULL UP OR DOWN FOR P5.5
        P5OUT |= BIT5;          //SPECIFIED AS A PULLUP FOR P5.5
        P5IE |= BIT5;           //SET P5.5 INTERRUPT ENABLE (S2)
        P5IFG &= ~BIT5;         //P5.5 IFG CLEARED
        P5IES |= BIT5;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW

//BIT0 SETUP
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

    return 0;
}

#pragma vector=PORT5_VECTOR
__interrupt void PORT_5(void)
{
        switch(state)
        {
        case 0: //OFF -> GOING ON
            TA0CTL = TASSEL_2 + MC_1 + TACLR + ID_2;       // ACTIVATE TIMER (goes into case 0 in the timer ISR)
            P5IFG &= BIT5;                         // CLEAR FLAG FOR P5.5
            P5IE &= ~BIT5;                          // disable interrupts for P5.5 (BUTTON)
            break;
        case 1: //ON -> GOING OFF
            TA0CTL = TASSEL_2 + MC_1 + TACLR + ID_2;       //ACTIVATE TIMER (goes into case 1 in the timer ISR)
            P5IFG &= ~BIT5;                         // CLEAR FLAG FOR P5.5
            P5IE &= ~BIT5;                          // disable interrupts for P5.5 (BUTTON)
            break;
        }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
        switch(state)
        {
        case 0://GOING ON -> ON
            P1OUT ^= BIT0;              //BLINK BIT0 (OUTPUT)
            P5IE |= BIT5;               //RE-ENABLE INTERRUPTS
            P5IES &= ~BIT5;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (BUTTON release)
            TA0CTL &= ~TASSEL_2;        //STOP TIMER
            TA0CTL |= TACLR;            //CLEAR TIMER
            state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
            break;
        case 1://GOING OFF -> OFF
            P5IE |= BIT5;               //SET P5.5 INTERRUPT ENABLE (S2)
            P5IFG &= ~BIT5;             //P5.5 IFG CLEARED
            P5IES |= BIT5;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
            TA0CTL &= ~TASSEL_2;        //STOP TIMER
            TA0CTL |= TACLR;            //CLEAR TIMER
            state = 0;                  //UPON ANOTHER BUTTON PRESS, WE WILL ENTER CASE 0 OF PORT 1 ISR
            break;
        }


}

