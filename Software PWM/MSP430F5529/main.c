/*AUTHOR: STEPHEN SZYMCZAK
* PROJECT: SOFTWARE PWM FOR F5529
*/
#include <msp430.h>

int pwm = 500; // for PWM
int state = 0; //for debounce
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

//output configure
    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

//PWM TIMER CONFIGURE
    TA0CCR0 = 1000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; //smclock, up mode, clear timer to zero, enable timer a interrupts

//debounce timer configure
    TA1CCTL0 = CCIE;                    // CCR1 interrupt enabled
    TA1CCR0 = 50000;                    //overflow every 10ms FOR DEBOUNCE

//button configure
    P1DIR &= ~BIT1;                         //SET P1.1 AS INPUT
    P1REN |= BIT1;                          //ENABLED PULL UP OR DOWN FOR P1.1
    P1OUT |= BIT1;                          //SPECIFIED AS A PULLUP FOR P1.1
    P1IE |= BIT1;                           //SET P1.1 INTERRUPT ENABLED (S2)
    P1IFG &= ~BIT1;                         //P1.1 IFG CLEARED
    P1IES |= BIT1;                          //TOGGLE INTERRUPT EDGE: HIGH TO LOW
    __bis_SR_register(GIE);     // Enter LPM3, enable interrupts
    for(;;) //pwm loop
    {
        if(TA0R <= pwm) P1OUT ^= BIT0; // pwm/1000 * 100% = duty cycle
        else if (TA0R > pwm) P1OUT &= ~BIT0;
    }
}
#pragma vector=PORT1_VECTOR
__interrupt void PORT_1(void)
{
        switch(state)
        {
        case 0: //OFF -> GOING ON
            TA1CTL = TASSEL_2 + MC_1 + TACLR;       // ACTIVATE TIMER (goes into case 0 in the timer ISR)
            P1IFG &= ~BIT1;                         // CLEAR FLAG FOR P1.1
            P1IE &= ~BIT1;                          // disable interrupts for P1.1 (button 2)
            break;
        case 1: //ON -> GOING OFF
            TA1CTL = TASSEL_2 + MC_1 + TACLR;       //ACTIVATE TIMER (goes into case 1 in the timer ISR)
            P1IFG &= ~BIT1;                         // CLEAR FLAG FOR P1.1
            P1IE &= ~BIT1;                          // disable interrupts for P1.1 (button 2)
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
            P1IE |= BIT1;               //RE-ENABLE INTERRUPTS
            P1IES &= ~BIT1;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
            TA1CTL &= ~TASSEL_2;        //STOP TIMER
            TA1CTL |= TACLR;            //CLEAR TIMER
            state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
            break;
        case 1://GOING OFF -> OFF
            P1IE |= BIT1;               //SET P1.1 INTERRUPT ENABLED (S2)
            P1IFG &= ~BIT1;             //P1.1 IFG CLEARED
            P1IES |= BIT1;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
            TA1CTL &= ~TASSEL_2;        //STOP TIMER
            TA1CTL |= TACLR;            //CLEAR TIMER
            state = 0;                  //UPON ANOTHER BUTTON PRESS, WE WILL ENTER CASE 0 OF PORT 1 ISR
            break;
        }


}
