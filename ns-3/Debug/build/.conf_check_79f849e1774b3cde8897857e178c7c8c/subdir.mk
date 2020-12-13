################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../build/.conf_check_79f849e1774b3cde8897857e178c7c8c/test.cpp 

OBJS += \
./build/.conf_check_79f849e1774b3cde8897857e178c7c8c/test.o 

CPP_DEPS += \
./build/.conf_check_79f849e1774b3cde8897857e178c7c8c/test.d 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_79f849e1774b3cde8897857e178c7c8c/%.o: ../build/.conf_check_79f849e1774b3cde8897857e178c7c8c/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


