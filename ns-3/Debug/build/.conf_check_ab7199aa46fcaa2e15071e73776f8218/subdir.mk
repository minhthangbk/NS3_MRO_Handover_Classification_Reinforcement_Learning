################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../build/.conf_check_ab7199aa46fcaa2e15071e73776f8218/test.cpp 

OBJS += \
./build/.conf_check_ab7199aa46fcaa2e15071e73776f8218/test.o 

CPP_DEPS += \
./build/.conf_check_ab7199aa46fcaa2e15071e73776f8218/test.d 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_ab7199aa46fcaa2e15071e73776f8218/%.o: ../build/.conf_check_ab7199aa46fcaa2e15071e73776f8218/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


