################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/click/test/ipv4-click-routing-test.cc 

CC_DEPS += \
./src/click/test/ipv4-click-routing-test.d 

OBJS += \
./src/click/test/ipv4-click-routing-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/click/test/%.o: ../src/click/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


