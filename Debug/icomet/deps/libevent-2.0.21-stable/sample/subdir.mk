################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../icomet/deps/libevent-2.0.21-stable/sample/dns-example.c \
../icomet/deps/libevent-2.0.21-stable/sample/event-test.c \
../icomet/deps/libevent-2.0.21-stable/sample/hello-world.c \
../icomet/deps/libevent-2.0.21-stable/sample/http-server.c \
../icomet/deps/libevent-2.0.21-stable/sample/le-proxy.c \
../icomet/deps/libevent-2.0.21-stable/sample/signal-test.c \
../icomet/deps/libevent-2.0.21-stable/sample/time-test.c 

OBJS += \
./icomet/deps/libevent-2.0.21-stable/sample/dns-example.o \
./icomet/deps/libevent-2.0.21-stable/sample/event-test.o \
./icomet/deps/libevent-2.0.21-stable/sample/hello-world.o \
./icomet/deps/libevent-2.0.21-stable/sample/http-server.o \
./icomet/deps/libevent-2.0.21-stable/sample/le-proxy.o \
./icomet/deps/libevent-2.0.21-stable/sample/signal-test.o \
./icomet/deps/libevent-2.0.21-stable/sample/time-test.o 

C_DEPS += \
./icomet/deps/libevent-2.0.21-stable/sample/dns-example.d \
./icomet/deps/libevent-2.0.21-stable/sample/event-test.d \
./icomet/deps/libevent-2.0.21-stable/sample/hello-world.d \
./icomet/deps/libevent-2.0.21-stable/sample/http-server.d \
./icomet/deps/libevent-2.0.21-stable/sample/le-proxy.d \
./icomet/deps/libevent-2.0.21-stable/sample/signal-test.d \
./icomet/deps/libevent-2.0.21-stable/sample/time-test.d 


# Each subdirectory must supply rules for building sources it contributes
icomet/deps/libevent-2.0.21-stable/sample/%.o: ../icomet/deps/libevent-2.0.21-stable/sample/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


