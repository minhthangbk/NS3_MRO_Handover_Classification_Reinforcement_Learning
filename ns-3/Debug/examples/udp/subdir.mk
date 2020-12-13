################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../examples/udp/udp-echo.cc 

CC_DEPS += \
./examples/udp/udp-echo.d 

OBJS += \
./examples/udp/udp-echo.o 


# Each subdirectory must supply rules for building sources it contributes
examples/udp/%.o: ../examples/udp/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


