/*
 * This file is part of the Trezor project, https://trezor.io/
 *
 * Copyright (c) SatoshiLabs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
Random delay interrupts (RDI) is a contermeasure against side channel attacks.
It consists of an interrupt handler that is supposed to be called every
millisecond or so. The handler waits for a random number of cpu ticks that is a
sample of so called floating mean distribution. That means that the number is
the sum of two numbers generated uniformly at random in the interval [0, 255].
The first number is generated freshly for each call of the handler, the other
number is supposed to be refreshed when the device performs an operation that
leaks the current state of the execution flow, such as sending or receiving an
usb packet.

See Differential Power Analysis in the Presence of Hardware Countermeasures by
Christophe Clavier, Jean-Sebastien Coron, Nora Dabbous and Efficient Use of
Random Delays in Embedded Software by Michael Tunstall, Olivier Benoit:
https://link.springer.com/content/pdf/10.1007%2F3-540-44499-8_20.pdf
https://link.springer.com/content/pdf/10.1007%2F978-3-540-72354-7_3.pdf
*/
#include "freertos/FreeRTOS.h"
#include "esp_freertos_hooks.h"

#include "random_delays.h"

#include <stdatomic.h>
#include <stdbool.h>

#include "chacha_drbg.h"
#include "common.h"
#include "memzero.h"
#include "rand.h"
#include "esp_system.h"

// from util.s
//extern void shutdown_privileged(void);
#ifndef RANDI
//#error "Compile with -DRDI=1"
#pragma message("Compile with -DRDI=1")
#endif

#define DRBG_RESEED_INTERVAL_CALLS 1000
#define DRBG_TRNG_ENTROPY_LENGTH 50
_Static_assert(CHACHA_DRBG_OPTIMAL_RESEED_LENGTH(1) == DRBG_TRNG_ENTROPY_LENGTH,
               "");
#define BUFFER_LENGTH 64

static CHACHA_DRBG_CTX drbg_ctx;
static secbool drbg_initialized = secfalse;
static uint8_t session_delay;
static bool refresh_session_delay;
static secbool rdi_disabled = sectrue;

static void drbg_init() {
  uint8_t entropy[DRBG_TRNG_ENTROPY_LENGTH] = {0};
  random_buffer(entropy, sizeof(entropy));
  chacha_drbg_init(&drbg_ctx, entropy, sizeof(entropy), NULL, 0);
  memzero(entropy, sizeof(entropy));

  drbg_initialized = sectrue;
}

static IRAM_ATTR void drbg_reseed() {
  ensure(drbg_initialized, NULL);

  uint8_t entropy[DRBG_TRNG_ENTROPY_LENGTH] = {0};
  random_buffer(entropy, sizeof(entropy));
  chacha_drbg_reseed(&drbg_ctx, entropy, sizeof(entropy), NULL, 0);
  memzero(entropy, sizeof(entropy));
}

static IRAM_ATTR void drbg_generate(uint8_t *buffer, size_t length) {
  ensure(drbg_initialized, NULL);

  if (drbg_ctx.reseed_counter > DRBG_RESEED_INTERVAL_CALLS) {
    drbg_reseed();
  }
  chacha_drbg_generate(&drbg_ctx, buffer, length);
}

// WARNING: Returns a constant if the function's critical section is locked
static IRAM_ATTR uint32_t drbg_random8(void) {
  // Since the function is called both from an interrupt (rdi_handler,
  // wait_random) and the main thread (wait_random), we use a lock to
  // synchronise access to global variables
  static volatile atomic_flag locked = ATOMIC_FLAG_INIT;

  if (atomic_flag_test_and_set(&locked))
  // locked_old = locked; locked = true; locked_old
  {
    // If the critical section is locked we return a non-random value, which
    // should be ok for our purposes
    return 128;
  }

  static size_t buffer_index = 0;
  static uint8_t buffer[BUFFER_LENGTH] = {0};

  if (buffer_index == 0) {
    drbg_generate(buffer, sizeof(buffer));
  }

  // To be extra sure there is no buffer overflow, we use a local copy of
  // buffer_index
  size_t buffer_index_local = buffer_index % sizeof(buffer);
  uint8_t value = buffer[buffer_index_local];
  memzero(&buffer[buffer_index_local], 1);
  buffer_index = (buffer_index_local + 1) % sizeof(buffer);

  atomic_flag_clear(&locked);  // locked = false
  return value;
}

static IRAM_ATTR void wait(uint32_t delay) {
	for(uint32_t i = 0; i < delay; i++);	//ypl 这部分汇编不知道咋写，留在以后吧
/*
  // wait (30 + delay) ticks
  asm volatile(
      "ldr r0, %0;"  // r0 = delay
      "loop:"
      "subs r0, $3;"  // r0 -= 3
      "bhs loop;"     // if (r0 >= 3): goto loop
      // loop (delay // 3) times
      // every loop takes 3 ticks
      // r0 == (delay % 3) - 3
      "add r0, $3;"  // r0 += 3
      // r0 == delay % 3
      "and r0, r0, $3;"  // r0 %= 4, make sure that 0 <= r0 < 4
      "ldr r1, =table;"  // r1 = &table
      "tbb [r1, r0];"    // jump 2*r1[r0] bytes forward, that is goto wait_r0
      "base:"
      "table:"  // table of branch lengths
      ".byte (wait_0 - base)/2;"
      ".byte (wait_1 - base)/2;"
      ".byte (wait_2 - base)/2;"
      ".byte (wait_2 - base)/2;"  // next instruction must be 2-byte aligned
      "wait_2:"
      "add r0, $1;"  // wait one tick
      "wait_1:"
      "add r0, $1;"  // wait one tick
      "wait_0:"
      :
      : "m"(delay)
      : "r0", "r1");
*/	  
}

void rdi_handler(uint32_t uw_tick);

IRAM_ATTR void rdi_tick_hook( void )
{
	uint32_t uw_tick = 0;
#ifdef RANDI
    rdi_handler(uw_tick);		//ypl 请确保中断处理程序访问的所有数据和函数（包括其调用的数据和函数）都存储在 IRAM 或 DRAM 中
    							//在函数或符号未被正确放入 IRAM/DRAM 的情况下，中断处理程序在 flash 操作期间从 flash cache 中读取数据时，会导致程序崩溃
#endif

}

void random_delays_init() { 
	drbg_init(); 
	/*乐鑫通过注册代码到systemtick的钩子函数里面;
	  我理解是通过定期中断方式占用PC指针随机的clock，而使代码执行的时间无规律，
	  从而达到防信道攻击的目的
	*/
	esp_register_freertos_tick_hook(rdi_tick_hook);
}

void rdi_start(void) {
  ensure(drbg_initialized, NULL);

  if (rdi_disabled == sectrue) {  // if rdi disabled
    refresh_session_delay = true;
    rdi_disabled = secfalse;
  }
}

void rdi_stop(void) {
  if (rdi_disabled == secfalse) {  // if rdi enabled
    rdi_disabled = sectrue;
    session_delay = 0;
  }
}

void rdi_refresh_session_delay(void) {
  if (rdi_disabled == secfalse)  // if rdi enabled
    refresh_session_delay = true;
}

IRAM_ATTR void rdi_handler(uint32_t uw_tick) {
  if (rdi_disabled == secfalse) {  // if rdi enabled
    if (refresh_session_delay) {
      session_delay = drbg_random8();
      refresh_session_delay = false;
    }

    wait(drbg_random8() + session_delay);

  } else {  // if rdi disabled or rdi_disabled corrupted
    ensure(rdi_disabled, "Fault detected");
  }
}
/*
 * Generates a delay of random length. Use this to protect sensitive code
 * against fault injection.
 */
void wait_random(void) {
  int wait = drbg_random8();
  volatile int i = 0;
  volatile int j = wait;
  while (i < wait) {
    if (i + j != wait) {
      //shutdown_privileged();		//ypl 暂时用重启代替，牵扯到ESP32的汇编我都不会写，这个功能留在以后吧
      esp_restart();
    }
    ++i;
    --j;
  }
  // Double-check loop completion.
  if (i != wait || j != 0) {
    //shutdown_privileged();
    esp_restart();
  }
}






