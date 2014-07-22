################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../icomet/deps/jemalloc-3.4.0/test/ALLOCM_ARENA.c \
../icomet/deps/jemalloc-3.4.0/test/aligned_alloc.c \
../icomet/deps/jemalloc-3.4.0/test/allocated.c \
../icomet/deps/jemalloc-3.4.0/test/allocm.c \
../icomet/deps/jemalloc-3.4.0/test/bitmap.c \
../icomet/deps/jemalloc-3.4.0/test/mremap.c \
../icomet/deps/jemalloc-3.4.0/test/posix_memalign.c \
../icomet/deps/jemalloc-3.4.0/test/rallocm.c \
../icomet/deps/jemalloc-3.4.0/test/thread_arena.c \
../icomet/deps/jemalloc-3.4.0/test/thread_tcache_enabled.c 

OBJS += \
./icomet/deps/jemalloc-3.4.0/test/ALLOCM_ARENA.o \
./icomet/deps/jemalloc-3.4.0/test/aligned_alloc.o \
./icomet/deps/jemalloc-3.4.0/test/allocated.o \
./icomet/deps/jemalloc-3.4.0/test/allocm.o \
./icomet/deps/jemalloc-3.4.0/test/bitmap.o \
./icomet/deps/jemalloc-3.4.0/test/mremap.o \
./icomet/deps/jemalloc-3.4.0/test/posix_memalign.o \
./icomet/deps/jemalloc-3.4.0/test/rallocm.o \
./icomet/deps/jemalloc-3.4.0/test/thread_arena.o \
./icomet/deps/jemalloc-3.4.0/test/thread_tcache_enabled.o 

C_DEPS += \
./icomet/deps/jemalloc-3.4.0/test/ALLOCM_ARENA.d \
./icomet/deps/jemalloc-3.4.0/test/aligned_alloc.d \
./icomet/deps/jemalloc-3.4.0/test/allocated.d \
./icomet/deps/jemalloc-3.4.0/test/allocm.d \
./icomet/deps/jemalloc-3.4.0/test/bitmap.d \
./icomet/deps/jemalloc-3.4.0/test/mremap.d \
./icomet/deps/jemalloc-3.4.0/test/posix_memalign.d \
./icomet/deps/jemalloc-3.4.0/test/rallocm.d \
./icomet/deps/jemalloc-3.4.0/test/thread_arena.d \
./icomet/deps/jemalloc-3.4.0/test/thread_tcache_enabled.d 


# Each subdirectory must supply rules for building sources it contributes
icomet/deps/jemalloc-3.4.0/test/%.o: ../icomet/deps/jemalloc-3.4.0/test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


