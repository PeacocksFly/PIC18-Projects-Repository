# PIC18-Projects-Repository
PIC microcontroller projects from Dogan Ibrahim's book implemented in C using MPLAB X IDE and Proteus

1. Stepper Motor Control - Simple Unipolar Drive (unipolar_stepper_motor.c). A unipolar stepper motor is used and the motor is rotated 100 turns before it is then stopped. https://youtu.be/mTZW4wvh8Gs

2. Stepper Motor Control - Complex Control of Unipolar Motor (complex_control_unipolar_stepper_motor.c). A unipolar stepper motor is used and the motor is rotated 20 revolutions clockwise, delay of 5s, 5 revolutions anticlockwise, delay of 3s, 10 revolutions clockwise, delay of 1s then stop. https://youtu.be/FunM3NzO7Dw

3. Stepper Motor Control - Simple Bipolar Motor Drive (bipolar_stepper_motor.c). A bipolar stepper motor is used and the motor is rotated 10 revolutions clockwise, delay of 5s, 10 revolutions anticlockwise and then stopped. https://youtu.be/qVl8OQfqO_Y

4. DC Motor Control - Simple Motor Drive. The project is about controlling the direction of rotation of a DC motor. One push-button controls the direction and the other one stars/stops the motor. Weak pull-ups are used on PORT B. https://youtu.be/ZvYFq0oZyY8

5. DC Motor Control - Speed Control. The project is about controlling the speed of a DC motor using the PWM function available on PIC18F45K22. The duty cycle is modified by steps of 10%. The initial duty cycle is equal to 50%. Once the speed steady the duty cycle is first increased by 20%, then by 30% to reach a full duty cycle. It is then decreased by 60% and finally by 40% so the motor stops. The push buttons to change the speed triggers interrupts on pin RB0 and RB1. https://youtu.be/Hhr3xz5ib34

6. Chasing Leds. Eight leds are connected to PORT C. The leds turn on alternately with one second delay interval. The leds are connected in current sourcing mode. The forward voltage/current are respectively equal to 2.2V and 10mA so the resistor was set to 280 ohms. https://youtu.be/jF1YvoOWvl0

7. Random Flashing Leds. Eight leds are connected to PORT C. An integer is generated between 1 and 255 every second. Leds are turned on o indicate this number in binary. https://youtu.be/dBKmfi5_PKs

8. Led Dice. Seven leds are organized such that when they turn on they indicate the number as in a real dice. The push button triggers on interrupt that calculates a random number between 1 and 6 and turn on the corresponding leds. https://youtu.be/bAST5OrHQls

9. 7-Segments Leds. A multiplexed two-digits is used to display a counter incrementing every second. Timer interrupt is used to update the counter. https://youtu.be/639xBpMrThg

10. Led Voltmeter. A voltmeter with a led display is designed. The voltmeter is used to measure voltages between 0-5V. The voltage to be measured is applied to an analog input, then converted into a binary number. This number is used to turn on the corresponding leds. The ADC of PIC18F45K22 is used with a 10-bit result. Polling was used to identify conversion completions. https://youtu.be/5qdQn9QUrQI

11. LCD Voltmeter. A voltmeter with an LCD display is designed. The voltmeter is used to measure voltages between 0-5V. The voltage to be measured is applied to an analog input, then converted into a binary number. This number is finally displayed on a LCD screen (one digit after comma). https://youtu.be/RN32fqTV_jE

12. Digital Thermometer. An analog temperature sensor is used to sense the temperature and the temperature is displayed on an LCD. The sensor used is the LM35. The ADC converts the analog temperature provided by the LM35 sensor into a binary 10-bits format. It is finally sent to the LCD screen. Polling was used to identify conversion completions. https://youtu.be/p-rKyXRDqtY

13. Generating Sound. A buzzer is connected to pin RC2. Sounds with different frequencies are generated using this buzzer. The buzzer is excited using square wave signals. The notes' sequence and the notes' frequency corresponding to the melody "happy birthday" are provided in Dogan Ibrahim's book. The frequency of each note is implemented with an ISR while the playing time of the note is obtained with a simple delay. https://youtu.be/EC2czgHOdr4

14. 4-Digit Multiplexed 7-Segment LED Using External Interrupt. An external interrupt-based event counter is designed to count up by one and display on the seven-segment displays every time an external event is detected. ISRs are used for both detecting an external event on pin B0 and refreshing the multiplexed 7-segment led. The multiplexer input is modified every 5 ms. https://youtu.be/FuHWdO0iOVY

15. Calculator with a keypad and an LCD. A 4x4 keypad and an LCD are used to create a simple calculator. When power is applied, calculator is displayed during 3 seconds. Then the user is asked to input the first and second operand. 4 digits are allowed to avoid overflow. Then the operation type must be selected before obtaining the result. Polling is used to detect key hit. https://youtu.be/4Vulx5q7PrM

16. Sawtooth Waveform Generator. This project is about generating three types of waveforms, namely sawtooth, triangle and sine wave. The microcontroller generates a digital signal that is sent via SPI protocol to the MCP4921, i.e. a DAC. The waveforms are then observed on an oscilloscope. A timer interrupt takes care of periodically changing the digital signal (code to be cleaned before publishing the video)

17. Frequency Counter with an LCD Display. A frequency counter is designed with an LCD display. Two different methods are used. In the first method the signal to be measured is connected to the clock input of the pic18f45k22 but configured as a counter. The counter is enabled while a timer is started for a known period of time. The frequency is finally calculated. In the second method the time is measured between two successive edges (code to be cleaned before publishing the video)

18. Reaction Timer. The project tests the reaction time of a person. A led is turned on after 3 seconds plus a random delay. Once turned on the user must push the button as quick as possible. The reaction time in milliseconds is then displayed on the LCD screen (code to be cleaned before publishing the video) 
