################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../icomet/deps/jemalloc-3.4.0/src/arena.c \
../icomet/deps/jemalloc-3.4.0/src/atomic.c \
../icomet/deps/jemalloc-3.4.0/src/base.c \
../icomet/deps/jemalloc-3.4.0/src/bitmap.c \
../icomet/deps/jemalloc-3.4.0/src/chunk.c \
../icomet/deps/jemalloc-3.4.0/src/chunk_dss.c \
../icomet/deps/jemalloc-3.4.0/src/chunk_mmap.c \
../icomet/deps/jemalloc-3.4.0/src/ckh.c \
../icomet/deps/jemalloc-3.4.0/src/ctl.c \
../icomet/deps/jemalloc-3.4.0/src/extent.c \
../icomet/deps/jemalloc-3.4.0/src/hash.c \
../icomet/deps/jemalloc-3.4.0/src/huge.c \
../icomet/deps/jemalloc-3.4.0/src/jemalloc.c \
../icomet/deps/jemalloc-3.4.0/src/mb.c \
../icomet/deps/jemalloc-3.4.0/src/mutex.c \
../icomet/deps/jemalloc-3.4.0/src/prof.c \
../icomet/deps/jemalloc-3.4.0/src/quarantine.c \
../icomet/deps/jemalloc-3.4.0/src/rtree.c \
../icomet/deps/jemalloc-3.4.0/src/stats.c \
../icomet/deps/jemalloc-3.4.0/src/tcache.c \
../icomet/deps/jemalloc-3.4.0/src/tsd.c \
../icomet/deps/jemalloc-3.4.0/src/util.c \
../icomet/deps/jemalloc-3.4.0/src/zone.c 

OBJS += \
./icomet/deps/jemalloc-3.4.0/src/arena.o \
./icomet/deps/jemalloc-3.4.0/src/atomic.o \
./icomet/deps/jemalloc-3.4.0/src/base.o \
./icomet/deps/jemalloc-3.4.0/src/bitmap.o \
./icomet/deps/jemalloc-3.4.0/src/chunk.o \
./icomet/deps/jemalloc-3.4.0/src/chunk_dss.o \
./icomet/deps/jemalloc-3.4.0/src/chunk_mmap.o \
./icomet/deps/jemalloc-3.4.0/src/ckh.o \
./icomet/deps/jemalloc-3.4.0/src/ctl.o \
./icomet/deps/jemalloc-3.4.0/src/extent.o \
./icomet/deps/jemalloc-3.4.0/src/hash.o \
./icomet/deps/jemalloc-3.4.0/src/huge.o \
./icomet/deps/jemalloc-3.4.0/src/jemalloc.o \
./icomet/deps/jemalloc-3.4.0/src/mb.o \
./icomet/deps/jemalloc-3.4.0/src/mutex.o \
./icomet/deps/jemalloc-3.4.0/src/prof.o \
./icomet/deps/jemalloc-3.4.0/src/quarantine.o \
./icomet/deps/jemalloc-3.4.0/src/rtree.o \
./icomet/deps/jemalloc-3.4.0/src/stats.o \
./icomet/deps/jemalloc-3.4.0/src/tcache.o \
./icomet/deps/jemalloc-3.4.0/src/tsd.o \
./icomet/deps/jemalloc-3.4.0/src/util.o \
./icomet/deps/jemalloc-3.4.0/src/zone.o 

C_DEPS += \
./icomet/deps/jemalloc-3.4.0/src/arena.d \
./icomet/deps/jemalloc-3.4.0/src/atomic.d \
./icomet/deps/jemalloc-3.4.0/src/base.d \
./icomet/deps/jemalloc-3.4.0/src/bitmap.d \
./icomet/deps/jemalloc-3.4.0/src/chunk.d \
./icomet/deps/jemalloc-3.4.0/src/chunk_dss.d \
./icomet/deps/jemalloc-3.4.0/src/chunk_mmap.d \
./icomet/deps/jemalloc-3.4.0/src/ckh.d \
./icomet/deps/jemalloc-3.4.0/src/ctl.d \
./icomet/deps/jemalloc-3.4.0/src/extent.d \
./icomet/deps/jemalloc-3.4.0/src/hash.d \
./icomet/deps/jemalloc-3.4.0/src/huge.d \
./icomet/deps/jemalloc-3.4.0/src/jemalloc.d \
./icomet/deps/jemalloc-3.4.0/src/mb.d \
./icomet/deps/jemalloc-3.4.0/src/mutex.d \
./icomet/deps/jemalloc-3.4.0/src/prof.d \
./icomet/deps/jemalloc-3.4.0/src/quarantine.d \
./icomet/deps/jemalloc-3.4.0/src/rtree.d \
./icomet/deps/jemalloc-3.4.0/src/stats.d \
./icomet/deps/jemalloc-3.4.0/src/tcache.d \
./icomet/deps/jemalloc-3.4.0/src/tsd.d \
./icomet/deps/jemalloc-3.4.0/src/util.d \
./icomet/deps/jemalloc-3.4.0/src/zone.d 


# Each subdirectory must supply rules for building sources it contributes
icomet/deps/jemalloc-3.4.0/src/%.o: ../icomet/deps/jemalloc-3.4.0/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


