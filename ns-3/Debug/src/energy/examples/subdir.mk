################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/energy/examples/li-ion-energy-source.cc 

CC_DEPS += \
./src/energy/examples/li-ion-energy-source.d 

OBJS += \
./src/energy/examples/li-ion-energy-source.o 


# Each subdirectory must supply rules for building sources it contributes
src/energy/examples/%.o: ../src/energy/examples/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


