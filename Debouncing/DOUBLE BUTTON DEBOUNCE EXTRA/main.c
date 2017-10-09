/*AUTHOR: STEPHEN SZYMCZAK
 * PROJECT: BUTTON DEBOUNCE TWO BUTTONS FR6989
 */

#include <msp430.h> 
int state = 0;
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                                  // to activate previously configured port settings
//TIMER SETUP
    TA0CCTL0 = CCIE;        // CCR0 interrupt enabled
    TA0CCR0 = 50000;        //overflow every 10ms

//BUTTON SETUP
    //button 2
        P1DIR &= ~BIT2;         //SET P1.5 AS INPUT
        P1REN |= BIT2;          //ENABLED PULL UP OR DOWN FOR P1.5
        P1OUT |= BIT2;          //SPECIFIED AS A PULLUP FOR P1.5
        P1IE |= BIT2;           //SET P1.5 INTERRUPT ENABLED (S2)
        P1IFG &= ~BIT2;         //P1.5 IFG CLEARED
        P1IES |= BIT2;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
    //button 1
        P1DIR &= ~BIT1;         //SET P1.5 AS INPUT
        P1REN |= BIT1;          //ENABLED PULL UP OR DOWN FOR P1.5
        P1OUT |= BIT1;          //SPECIFIED AS A PULLUP FOR P1.5
        P1IE |= BIT1;           //SET P1.5 INTERRUPT ENABLED (S2)
        P1IFG &= ~BIT1;         //P1.5 IFG CLEARED
        P1IES |= BIT1;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
//LED SETUP
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P9DIR |= BIT7;
    P9OUT &= ~BIT7;
    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

    return 0;
}
#pragma vector=PORT1_VECTOR
__interrupt void PORT_1(void)
{
    switch(P1IV)
    {
    case P1IV_P1IFG1:
        switch(state)
        {
        case 0: //OFF -> GOING ON
            TA0CTL = TASSEL_2 + MC_1 + TACLR;       // ACTIVATE TIMER (goes into case 0 in the timer ISR)
            P1IFG &= ~BIT1;                         // CLEAR FLAG FOR P1.2
            P1IE &= ~BIT1;                          // disable interrupts for P1.2 (button 2)
            state = 1;
            break;
        case 3: //ON -> GOING OFF
            TA0CTL = TASSEL_2 + MC_1 + TACLR;       //ACTIVATE TIMER (goes into case 1 in the timer ISR)
            P1IFG &= ~BIT1;                         // CLEAR FLAG FOR P1.2
            P1IE &= ~BIT1;                          // disable interrupts for P1.2 (button 2)
            state = 5;
            break;
        }
        break;
    case P1IV_P1IFG2:
        switch(state)
        {
        case 0: //OFF -> GOING ON
            TA0CTL = TASSEL_2 + MC_1 + TACLR;       // ACTIVATE TIMER (goes into case 0 in the timer ISR)
            P1IFG &= ~BIT2;                         // CLEAR FLAG FOR P1.2
            P1IE &= ~BIT2;                          // disable interrupts for P1.2 (button 2)
            state = 2;
            break;
        case 4: //ON -> GOING OFF
            TA0CTL = TASSEL_2 + MC_1 + TACLR;       //ACTIVATE TIMER (goes into case 1 in the timer ISR)
            P1IFG &= ~BIT2;                         // CLEAR FLAG FOR P1.2
            P1IE &= ~BIT2;                          // disable interrupts for P1.2 (button 2)
            state = 6;
            break;
        }
        break;
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{

            switch(state)
            {
            case 1://GOING ON -> ON
                P1OUT ^= BIT0;              //BLINK LED (OUTPUT)
                P1IE |= BIT1;               //RE-ENABLE INTERRUPTS
                P1IES &= ~BIT1;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
                TA0CTL &= ~TASSEL_2;        //STOP TIMER
                TA0CTL |= TACLR;            //CLEAR TIMER
                state = 3;                  //TO GO TO NEXT STATE IN PORT 1 ISR
                break;
            case 5://GOING OFF -> OFF
                P1IE |= BIT1;               //SET P1.5 INTERRUPT ENABLED (S2)
                P1IFG &= ~BIT1;             //P1.2 IFG CLEARED
                P1IES |= BIT1;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
                TA0CTL &= ~TASSEL_2;        //STOP TIMER
                TA0CTL |= TACLR;            //CLEAR TIMER
                state = 0;                  //UPON ANOTHER BUTTON PRESS, WE WILL ENTER CASE 0 OF PORT 1 ISR
                break;
              case 2://GOING ON -> ON
                  P9OUT ^= BIT7;              //BLINK LED (OUTPUT)
                  P1IE |= BIT2;               //RE-ENABLE INTERRUPTS
                  P1IES &= ~BIT2;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
                  TA0CTL &= ~TASSEL_2;        //STOP TIMER
                  TA0CTL |= TACLR;            //CLEAR TIMER
                  state = 4;                  //TO GO TO NEXT STATE IN PORT 1 ISR
                  break;
              case 6://GOING OFF -> OFF
                  P1IE |= BIT2;               //SET P1.5 INTERRUPT ENABLED (S2)
                  P1IFG &= ~BIT2;             //P1.2 IFG CLEARED
                  P1IES |= BIT2;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
                  TA0CTL &= ~TASSEL_2;        //STOP TIMER
                  TA0CTL |= TACLR;            //CLEAR TIMER
                  state = 0;                  //UPON ANOTHER BUTTON PRESS, WE WILL ENTER CASE 0 OF PORT 1 ISR
                  break;
              }
}
