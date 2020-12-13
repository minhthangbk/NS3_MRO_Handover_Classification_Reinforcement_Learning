################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/sixlowpan/test/error-channel-sixlow.cc \
../src/sixlowpan/test/sixlowpan-fragmentation-test.cc \
../src/sixlowpan/test/sixlowpan-hc1-test.cc \
../src/sixlowpan/test/sixlowpan-iphc-test.cc 

CC_DEPS += \
./src/sixlowpan/test/error-channel-sixlow.d \
./src/sixlowpan/test/sixlowpan-fragmentation-test.d \
./src/sixlowpan/test/sixlowpan-hc1-test.d \
./src/sixlowpan/test/sixlowpan-iphc-test.d 

OBJS += \
./src/sixlowpan/test/error-channel-sixlow.o \
./src/sixlowpan/test/sixlowpan-fragmentation-test.o \
./src/sixlowpan/test/sixlowpan-hc1-test.o \
./src/sixlowpan/test/sixlowpan-iphc-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/sixlowpan/test/%.o: ../src/sixlowpan/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


