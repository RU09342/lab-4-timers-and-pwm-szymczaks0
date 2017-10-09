/*AUTHOR: STEPHEN SZYMCZAK
* PROJECT: SOFTWARE PWM FOR G2553
*/
#include <msp430.h>

int pwm = 500; // for PWM
int state = 0; //for debounce
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

//output configure
    P1OUT &= ~BIT6;
    P1DIR |= BIT6;

//PWM TIMER CONFIGURE
    TA0CCR0 = 1000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; //smclock, up mode, clear timer to zero, enable timer a interrupts

//debounce timer configure
    TA1CCTL0 = CCIE;                    // CCR1 interrupt enabled
    TA1CCR0 = 50000;                    //overflow every 10ms FOR DEBOUNCE

//button configure
    P1DIR &= ~BIT3;                         //SET P1.3 AS INPUT
    P1REN |= BIT3;                          //ENABLED PULL UP OR DOWN FOR P1.3
    P1OUT |= BIT3;                          //SPECIFIED AS A PULLUP FOR P1.3
    P1IE |= BIT3;                           //SET P1.3 INTERRUPT ENABLED (S2)
    P1IFG &= ~BIT3;                         //P1.3 IFG CLEARED
    P1IES |= BIT3;                          //TOGGLE INTERRUPT EDGE: HIGH TO LOW
    __bis_SR_register(GIE);     // Enter LPM3, enable interrupts
    for(;;) //pwm loop
    {
        if(TA0R <= pwm) P1OUT ^= BIT6; // pwm/1000 * 100% = duty cycle
        else if (TA0R > pwm) P1OUT &= ~BIT6;
    }
}
#pragma vector=PORT1_VECTOR
__interrupt void PORT_1(void)
{
        switch(state)
        {
        case 0: //OFF -> GOING ON
            TA1CTL = TASSEL_2 + MC_1 + TACLR;       // ACTIVATE TIMER (goes into case 0 in the timer ISR)
            P1IFG &= ~BIT3;                         // CLEAR FLAG FOR P1.3
            P1IE &= ~BIT3;                          // disable interrupts for P1.3 (button 2)
            break;
        case 1: //ON -> GOING OFF
            TA1CTL = TASSEL_2 + MC_1 + TACLR;       //ACTIVATE TIMER (goes into case 1 in the timer ISR)
            P1IFG &= ~BIT3;                         // CLEAR FLAG FOR P1.3
            P1IE &= ~BIT3;                          // disable interrupts for P1.3 (button 2)
            break;
        }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A0 (void)
{
        switch(state)
        {
        case 0://GOING ON -> ON
            if(pwm < 1000) pwm += 100;
            else pwm = 0;              //BLINK LED (OUTPUT)
            P1IE |= BIT3;               //RE-ENABLE INTERRUPTS
            P1IES &= ~BIT3;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
            TA1CTL &= ~TASSEL_2;        //STOP TIMER
            TA1CTL |= TACLR;            //CLEAR TIMER
            state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
            break;
        case 1://GOING OFF -> OFF
            P1IE |= BIT3;               //SET P1.3 INTERRUPT ENABLED (S2)
            P1IFG &= ~BIT3;             //P1.3 IFG CLEARED
            P1IES |= BIT3;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
            TA1CTL &= ~TASSEL_2;        //STOP TIMER
            TA1CTL |= TACLR;            //CLEAR TIMER
            state = 0;                  //UPON ANOTHER BUTTON PRESS, WE WILL ENTER CASE 0 OF PORT 1 ISR
            break;
        }


}
