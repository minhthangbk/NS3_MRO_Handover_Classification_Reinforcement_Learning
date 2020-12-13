################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/wifi/test/block-ack-test-suite.cc \
../src/wifi/test/dcf-manager-test.cc \
../src/wifi/test/power-rate-adaptation-test.cc \
../src/wifi/test/tx-duration-test.cc \
../src/wifi/test/wifi-aggregation-test.cc \
../src/wifi/test/wifi-test.cc 

CC_DEPS += \
./src/wifi/test/block-ack-test-suite.d \
./src/wifi/test/dcf-manager-test.d \
./src/wifi/test/power-rate-adaptation-test.d \
./src/wifi/test/tx-duration-test.d \
./src/wifi/test/wifi-aggregation-test.d \
./src/wifi/test/wifi-test.d 

OBJS += \
./src/wifi/test/block-ack-test-suite.o \
./src/wifi/test/dcf-manager-test.o \
./src/wifi/test/power-rate-adaptation-test.o \
./src/wifi/test/tx-duration-test.o \
./src/wifi/test/wifi-aggregation-test.o \
./src/wifi/test/wifi-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/wifi/test/%.o: ../src/wifi/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


