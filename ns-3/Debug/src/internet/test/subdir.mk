################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/internet/test/codel-queue-test-suite.cc \
../src/internet/test/error-channel.cc \
../src/internet/test/global-route-manager-impl-test-suite.cc \
../src/internet/test/ipv4-address-generator-test-suite.cc \
../src/internet/test/ipv4-address-helper-test-suite.cc \
../src/internet/test/ipv4-forwarding-test.cc \
../src/internet/test/ipv4-fragmentation-test.cc \
../src/internet/test/ipv4-global-routing-test-suite.cc \
../src/internet/test/ipv4-header-test.cc \
../src/internet/test/ipv4-list-routing-test-suite.cc \
../src/internet/test/ipv4-packet-info-tag-test-suite.cc \
../src/internet/test/ipv4-raw-test.cc \
../src/internet/test/ipv4-static-routing-test-suite.cc \
../src/internet/test/ipv4-test.cc \
../src/internet/test/ipv6-address-generator-test-suite.cc \
../src/internet/test/ipv6-address-helper-test-suite.cc \
../src/internet/test/ipv6-dual-stack-test-suite.cc \
../src/internet/test/ipv6-extension-header-test-suite.cc \
../src/internet/test/ipv6-forwarding-test.cc \
../src/internet/test/ipv6-fragmentation-test.cc \
../src/internet/test/ipv6-list-routing-test-suite.cc \
../src/internet/test/ipv6-packet-info-tag-test-suite.cc \
../src/internet/test/ipv6-raw-test.cc \
../src/internet/test/ipv6-ripng-test.cc \
../src/internet/test/ipv6-test.cc \
../src/internet/test/rtt-test.cc \
../src/internet/test/tcp-header-test.cc \
../src/internet/test/tcp-option-test.cc \
../src/internet/test/tcp-test.cc \
../src/internet/test/tcp-timestamp-test.cc \
../src/internet/test/tcp-wscaling-test.cc \
../src/internet/test/udp-test.cc 

CC_DEPS += \
./src/internet/test/codel-queue-test-suite.d \
./src/internet/test/error-channel.d \
./src/internet/test/global-route-manager-impl-test-suite.d \
./src/internet/test/ipv4-address-generator-test-suite.d \
./src/internet/test/ipv4-address-helper-test-suite.d \
./src/internet/test/ipv4-forwarding-test.d \
./src/internet/test/ipv4-fragmentation-test.d \
./src/internet/test/ipv4-global-routing-test-suite.d \
./src/internet/test/ipv4-header-test.d \
./src/internet/test/ipv4-list-routing-test-suite.d \
./src/internet/test/ipv4-packet-info-tag-test-suite.d \
./src/internet/test/ipv4-raw-test.d \
./src/internet/test/ipv4-static-routing-test-suite.d \
./src/internet/test/ipv4-test.d \
./src/internet/test/ipv6-address-generator-test-suite.d \
./src/internet/test/ipv6-address-helper-test-suite.d \
./src/internet/test/ipv6-dual-stack-test-suite.d \
./src/internet/test/ipv6-extension-header-test-suite.d \
./src/internet/test/ipv6-forwarding-test.d \
./src/internet/test/ipv6-fragmentation-test.d \
./src/internet/test/ipv6-list-routing-test-suite.d \
./src/internet/test/ipv6-packet-info-tag-test-suite.d \
./src/internet/test/ipv6-raw-test.d \
./src/internet/test/ipv6-ripng-test.d \
./src/internet/test/ipv6-test.d \
./src/internet/test/rtt-test.d \
./src/internet/test/tcp-header-test.d \
./src/internet/test/tcp-option-test.d \
./src/internet/test/tcp-test.d \
./src/internet/test/tcp-timestamp-test.d \
./src/internet/test/tcp-wscaling-test.d \
./src/internet/test/udp-test.d 

OBJS += \
./src/internet/test/codel-queue-test-suite.o \
./src/internet/test/error-channel.o \
./src/internet/test/global-route-manager-impl-test-suite.o \
./src/internet/test/ipv4-address-generator-test-suite.o \
./src/internet/test/ipv4-address-helper-test-suite.o \
./src/internet/test/ipv4-forwarding-test.o \
./src/internet/test/ipv4-fragmentation-test.o \
./src/internet/test/ipv4-global-routing-test-suite.o \
./src/internet/test/ipv4-header-test.o \
./src/internet/test/ipv4-list-routing-test-suite.o \
./src/internet/test/ipv4-packet-info-tag-test-suite.o \
./src/internet/test/ipv4-raw-test.o \
./src/internet/test/ipv4-static-routing-test-suite.o \
./src/internet/test/ipv4-test.o \
./src/internet/test/ipv6-address-generator-test-suite.o \
./src/internet/test/ipv6-address-helper-test-suite.o \
./src/internet/test/ipv6-dual-stack-test-suite.o \
./src/internet/test/ipv6-extension-header-test-suite.o \
./src/internet/test/ipv6-forwarding-test.o \
./src/internet/test/ipv6-fragmentation-test.o \
./src/internet/test/ipv6-list-routing-test-suite.o \
./src/internet/test/ipv6-packet-info-tag-test-suite.o \
./src/internet/test/ipv6-raw-test.o \
./src/internet/test/ipv6-ripng-test.o \
./src/internet/test/ipv6-test.o \
./src/internet/test/rtt-test.o \
./src/internet/test/tcp-header-test.o \
./src/internet/test/tcp-option-test.o \
./src/internet/test/tcp-test.o \
./src/internet/test/tcp-timestamp-test.o \
./src/internet/test/tcp-wscaling-test.o \
./src/internet/test/udp-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/internet/test/%.o: ../src/internet/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


