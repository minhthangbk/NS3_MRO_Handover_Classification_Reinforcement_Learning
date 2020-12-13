################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/bridge/helper/bridge-helper.cc 

CC_DEPS += \
./src/bridge/helper/bridge-helper.d 

OBJS += \
./src/bridge/helper/bridge-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/bridge/helper/%.o: ../src/bridge/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


