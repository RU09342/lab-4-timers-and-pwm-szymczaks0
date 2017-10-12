###### Stephen Szymczak

# LAB 4: Software PWM
Assuming the concept of PWM is understood from the Hardware PWM portion of the lab, let's just discuss how software PWM differs from hardware PWM. For hardware PWM we sent signal straight to a pin to produce PWM. For software PWM, we need to write code that sets a pin high for a period and low for another period. This code was implemented very differently for the FR5994 than the other 4 boards, and thus an explanation of the implementations will be explained separately.

## Implementation Method 1: Timer ISRs on the FR5994
Two timer ISRs that are triggered by CCR0 and CCR1 are used here to rapidly toggle the LED in a PWM fashion. CCR0 is set again to 1000 to produce a 1kHz signal, and CCR1 is initially set to 500 for a 50% duty cycle. Timer A interrupts are also enabled:
```c
int pwm = 500;
TA0CCR1 = pwm;
TA0CCR0 = 1000;
TA0CTL = TASSEL__SMCLK | MC__UP | TACLR | TAIE;
```
A second timer is also used for debouncing, the breakdown of which can be found in the previous button interrupt and button debouncing projects.

The duty cycle code is set up with two timer ISRs. The first one handles CCR1 interrupt, and the second handles CCR0 interrupt. The CCR0 ISR sets the pin high, and the CCR1 ISR sets the pin low. This order was chosen so that the pin is high up until CCR1 is reached. This makes the duty cycle dependent on the value that CCR1 holds:
```c
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
    if(pwm == 0) P1OUT &= ~BIT1;
    else P1OUT |= BIT1; //CCR0 PWM
}
```
CCR1 is adjusted with the same button interrupt code from the button debouncing project. In short, when the button is pressed CCR1 increases by 100, resetting to 0 after going over 1000.

##### NOTE: This method was essentially done as proof of concept, the following implementation was much simpler.

## Implementation Method 2: Polling For All The Other Boards

After doing this method once, it was apparent that this method is much easier to implement, so for the sake of translating the code from board to board, this method was used for most of the boards.

Instead of using CCR1 and CCR0 to handle the duty cycle, CCR0 and TA0R is used instead in a short for loop.
Since TA0R counts up to CCR0 and then resets, TA0R was simply compared with the pwm variable that is changed with the same button code as described in Implementation 1.

The following is the for loop that checks TA0R's value and turns the LED on or off depending on if TA0R is greater than or less than the varying pwm value:
```c
for(;;) //pwm loop
    {
        if(TA0R <= pwm) P1OUT |= BIT0; // pwm/1000 * 100% = duty cycle
        else if (TA0R > pwm) P1OUT &= ~BIT0;
    }
```
Comparing the method used for FR5994 and this method, it is hopefully apparent why this method was used for translating from board to board after this method was found. For one, timer ISR code varies from processor to processor, and that is not used here, leaving only the timer set up and button set up code needing changes. Additionally, when issues were found, finding solutions for them was much easier when only dealing with the two lines inside this for loop.

##### NOTE: While this method was easier to implement, it is worth noting that if power consumption is something that needs to be considered, this method will use considerably more as it is constantly polling in the for loop.
