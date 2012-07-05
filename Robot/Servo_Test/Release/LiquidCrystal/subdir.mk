################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/Applications/Developer/Arduino.app/Contents/Resources/Java/libraries/LiquidCrystal/LiquidCrystal.cpp 

OBJS += \
./LiquidCrystal/LiquidCrystal.o 

CPP_DEPS += \
./LiquidCrystal/LiquidCrystal.d 


# Each subdirectory must supply rules for building sources it contributes
LiquidCrystal/LiquidCrystal.o: /Applications/Developer/Arduino.app/Contents/Resources/Java/libraries/LiquidCrystal/LiquidCrystal.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"/Applications/Developer/Arduino.app/Contents/Resources/Java/hardware/arduino/cores/arduino" -I"/Applications/Developer/Arduino.app/Contents/Resources/Java/hardware/arduino/variants/standard" -I"/Users/derek/Arduino/Robot/Servo_Test" -I/Users/derek/Arduino/libraries -I"/Applications/Developer/Arduino.app/Contents/Resources/Java/libraries/LiquidCrystal" -I"/Users/derek/Arduino/libraries/Servo" -D__IN_ECLIPSE__=1 -DARDUINO=10.1 -Wall -Os -ffunction-sections -fdata-sections -fno-exceptions -g -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" -x c++ "$<"
	@echo 'Finished building: $<'
	@echo ' '


