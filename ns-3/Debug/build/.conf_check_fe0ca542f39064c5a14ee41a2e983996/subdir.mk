################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../build/.conf_check_fe0ca542f39064c5a14ee41a2e983996/test.cc 

CC_DEPS += \
./build/.conf_check_fe0ca542f39064c5a14ee41a2e983996/test.d 

OBJS += \
./build/.conf_check_fe0ca542f39064c5a14ee41a2e983996/test.o 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_fe0ca542f39064c5a14ee41a2e983996/%.o: ../build/.conf_check_fe0ca542f39064c5a14ee41a2e983996/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


