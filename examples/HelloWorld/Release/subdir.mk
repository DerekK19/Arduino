################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../HelloWorld.cpp 

OBJS += \
./HelloWorld.o 

CPP_DEPS += \
./HelloWorld.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"/Applications/Developer/Arduino.app/Contents/Resources/Java/hardware/arduino/cores/arduino" -I"/Applications/Developer/Arduino.app/Contents/Resources/Java/hardware/arduino/variants/standard" -I"/Users/derek/Arduino/HelloWorld" -D__IN_ECLIPSE__=1 -DARDUINO=101 -Wall -Os -ffunction-sections -fdata-sections -fno-exceptions -g -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" -x c++ "$<"
	@echo 'Finished building: $<'
	@echo ' '


