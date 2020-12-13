################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/wifi/examples/wifi-phy-test.cc 

CC_DEPS += \
./src/wifi/examples/wifi-phy-test.d 

OBJS += \
./src/wifi/examples/wifi-phy-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/wifi/examples/%.o: ../src/wifi/examples/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


