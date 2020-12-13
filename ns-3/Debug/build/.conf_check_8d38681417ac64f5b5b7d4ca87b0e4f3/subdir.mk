################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../build/.conf_check_8d38681417ac64f5b5b7d4ca87b0e4f3/test.cpp 

OBJS += \
./build/.conf_check_8d38681417ac64f5b5b7d4ca87b0e4f3/test.o 

CPP_DEPS += \
./build/.conf_check_8d38681417ac64f5b5b7d4ca87b0e4f3/test.d 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_8d38681417ac64f5b5b7d4ca87b0e4f3/%.o: ../build/.conf_check_8d38681417ac64f5b5b7d4ca87b0e4f3/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


