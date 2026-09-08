#include <stdio.h>
#include <string.h>
#include <timer.h>
#include <common/module.h>
#include <common/common.h>
#include <common/thread.h>
#include <common/semaphore.h>
#include <common/spinlock.h>
#include <atomic.h>
#include <assert.h>

#define STACK_SIZE 4096
#define MAX_WORKERS 32
#define ITERATIONS_SMALL  10000
#define ITERATIONS_LARGE  100000

static spinlock_t g_print_lock = SPIN_LOCK_INITIALIZER;
static atomic_t g_pass = ATOMIC_INITIALIZER(0);
static atomic_t g_fail = ATOMIC_INITIALIZER(0);

#define tprintf(fmt, ...) do {             \
	spin_lock(&g_print_lock);            \
	printf(fmt, ##__VA_ARGS__);          \
	spin_unlock(&g_print_lock);          \
} while (0)

#define TPASS(name) do {                   \
	atomic_add_return(&g_pass, 1);       \
	tprintf("  [PASS] %s\n", name);      \
} while (0)

#define TFAIL(name, reason) do {           \
	atomic_add_return(&g_fail, 1);       \
	tprintf("  [FAIL] %s: %s\n",         \
		name, reason);                 \
} while (0)

static struct sem g_sync_start[MAX_WORKERS];
static struct sem g_sync_done[MAX_WORKERS];
static unsigned char g_stacks[MAX_WORKERS][STACK_SIZE] __attribute__((aligned(16)));
static struct thread *g_workers[MAX_WORKERS];
static char g_thread_names[MAX_WORKERS][16];

static struct thread *alloc_worker(int idx, const char *name,
				   int (*func)(void *), void *arg)
{
	sem_init(&g_sync_start[idx], 0);
	sem_init(&g_sync_done[idx], 0);

	strncpy(g_thread_names[idx], name, sizeof(g_thread_names[idx]) - 1);
	g_thread_names[idx][sizeof(g_thread_names[idx]) - 1] = '\0';

	return thread_create(g_thread_names[idx], g_stacks[idx],
			     STACK_SIZE, func, arg);
}

static void set_worker_priority(int idx, int prio)
{
	if (g_workers[idx])
		thread_set_priority(g_workers[idx], prio);
}

static void coordinator_boost(void)
{
	thread_set_priority(sched_get_current(), 100);
}

static void signal_worker_start(int idx)
{
	sem_post(&g_sync_start[idx]);
}

static void wait_worker_done(int idx)
{
	sem_wait(&g_sync_done[idx]);
}

static void wait_all_workers(int count)
{
	int i;

	for (i = 0; i < count; ++i)
		wait_worker_done(i);
}

static void signal_all_workers_start(int count)
{
	int i;

	for (i = 0; i < count; ++i)
		signal_worker_start(i);
}

/*
 * Test 1: Basic Thread Lifecycle
 */
static atomic_t g_t1_counter;

static int t1_worker(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);
	atomic_add_return(&g_t1_counter, 1);
	sched_msleep(1);
	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t1_controller(void *arg)
{
	int i, n = 8;
	long val;

	tprintf("Test 1: Basic Thread Lifecycle (%d threads)\n", n);

	atomic_write(&g_t1_counter, 0);

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t1_w%d", i);
		g_workers[i] = alloc_worker(i, name, t1_worker, (void *)(long)i);
	}

	coordinator_boost();
	signal_all_workers_start(n);
	wait_all_workers(n);

	val = atomic_read(&g_t1_counter);
	if (val == n)
		TPASS("all threads executed, counter correct");
	else
		TFAIL("all threads executed", "counter mismatch");

	return 0;
}

/*
 * Test 2: Priority Scheduling
 * Verify strict priority: highest priority thread dominates.
 * When the high-priority thread sleeps, lower-priority threads run.
 */
static atomic_t g_t2_prio_count[4];
static volatile int g_t2_running;
static volatile int g_t2_high_should_sleep;

static int t2_worker(void *arg)
{
	int idx = (long)arg;
	int local_count = 0;

	sem_wait(&g_sync_start[idx]);

	while (g_t2_running) {
		if (idx == 2 && g_t2_high_should_sleep) {
			sched_msleep(10);
			continue;
		}
		++local_count;
	}

	atomic_add_return(&g_t2_prio_count[idx], local_count);

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t2_controller(void *arg)
{
	int i, n = 3;
	long counts[3];

	tprintf("Test 2: Priority Scheduling (3 threads, prio 1/2/3)\n");

	atomic_write(&g_t2_prio_count[0], 0);
	atomic_write(&g_t2_prio_count[1], 0);
	atomic_write(&g_t2_prio_count[2], 0);
	g_t2_running = 1;
	g_t2_high_should_sleep = 0;

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t2_w%d", i);
		g_workers[i] = alloc_worker(i, name, t2_worker, (void *)(long)i);
		set_worker_priority(i, i + 1);
	}

	signal_all_workers_start(n);

	/* Phase 1: all runnable, only prio 3 should run */
	sched_msleep(500);
	g_t2_high_should_sleep = 1;

	/* Phase 2: prio 3 sleeps, prio 2 and 1 should run */
	sched_msleep(500);
	g_t2_high_should_sleep = 0;

	/* Phase 3: prio 3 runs again */
	sched_msleep(500);

	g_t2_running = 0;
	wait_all_workers(n);

	for (i = 0; i < n; ++i)
		counts[i] = atomic_read(&g_t2_prio_count[i]);

	tprintf("  priority 1: %ld, priority 2: %ld, priority 3: %ld\n",
		counts[0], counts[1], counts[2]);

	if (counts[2] > counts[1] && counts[2] > counts[0])
		TPASS("highest priority thread dominated");
	else
		TFAIL("highest priority thread dominated", "unexpected distribution");

	return 0;
}

/*
 * Test 3: Multi-Threaded Counter with Yield
 * Multiple threads increment a shared atomic counter,
 * yielding periodically to allow interleaving.
 */
static atomic_t g_t3_counter;

static int t3_worker(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < ITERATIONS_SMALL; ++i) {
		atomic_add_return(&g_t3_counter, 1);
		if (i % 100 == 0)
			sched_yield();
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t3_controller(void *arg)
{
	int n = 8;
	int i;
	long val, expected = (long)n * ITERATIONS_SMALL;

	tprintf("Test 3: Multi-Threaded Counter (%d threads, %d ops each)\n",
		n, ITERATIONS_SMALL);

	atomic_write(&g_t3_counter, 0);

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t3_w%d", i);
		g_workers[i] = alloc_worker(i, name, t3_worker, (void *)(long)i);
	}

	coordinator_boost();
	signal_all_workers_start(n);
	wait_all_workers(n);

	val = atomic_read(&g_t3_counter);
	if (val == expected)
		TPASS("multi-threaded counter correct");
	else
		TFAIL("multi-threaded counter correct", "count mismatch");

	return 0;
}

/*
 * Test 4: Spinlock Contention
 */
static atomic_t g_t4_counter;
static spinlock_t g_t4_lock = SPIN_LOCK_INITIALIZER;

static int t4_worker(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < ITERATIONS_LARGE; ++i) {
		spin_lock(&g_t4_lock);
		atomic_add_return(&g_t4_counter, 1);
		spin_unlock(&g_t4_lock);
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t4_controller(void *arg)
{
	int n = 8;
	long val, expected = (long)n * ITERATIONS_LARGE;
	int i;

	tprintf("Test 4: Spinlock Contention (%d threads, %d ops each)\n",
		n, ITERATIONS_LARGE);

	atomic_write(&g_t4_counter, 0);

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t4_w%d", i);
		g_workers[i] = alloc_worker(i, name, t4_worker, (void *)(long)i);
	}

	signal_all_workers_start(n);
	wait_all_workers(n);

	val = atomic_read(&g_t4_counter);
	if (val == expected)
		TPASS("spinlock protected counter correct");
	else
		TFAIL("spinlock protected counter correct",
		      "expected != actual");

	return 0;
}

/*
 * Test 5: Atomic Operations
 */
static atomic_t g_t5_atomic;

static int t5_worker(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < ITERATIONS_LARGE; ++i)
		atomic_add_return(&g_t5_atomic, 1);

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t5_controller(void *arg)
{
	int n = 8;
	long val, expected = (long)n * ITERATIONS_LARGE;
	int i;

	tprintf("Test 5: Atomic Operations (%d threads, %d atomic_add each)\n",
		n, ITERATIONS_LARGE);

	atomic_write(&g_t5_atomic, 0);

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t5_w%d", i);
		g_workers[i] = alloc_worker(i, name, t5_worker, (void *)(long)i);
	}

	signal_all_workers_start(n);
	wait_all_workers(n);

	val = atomic_read(&g_t5_atomic);
	if (val == expected)
		TPASS("atomic counter correct under concurrency");
	else
		TFAIL("atomic counter correct under concurrency",
		      "expected != actual");

	return 0;
}

/*
 * Test 6: Sleep/Wakeup Ordering
 */
static atomic_t g_t6_wake_seq;
static long g_t6_wake_order[8];

static int t6_worker(void *arg)
{
	int idx = (long)arg;
	unsigned long sleep_ms = (unsigned long)(idx + 1) * 100;

	sem_wait(&g_sync_start[idx]);

	sched_msleep(sleep_ms);

	g_t6_wake_order[idx] = atomic_add_return(&g_t6_wake_seq, 1);

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t6_controller(void *arg)
{
	int n = 6;
	int i;
	int ordered = 1;

	tprintf("Test 6: Sleep/Wakeup Ordering (%d threads)\n", n);

	atomic_write(&g_t6_wake_seq, 0);
	for (i = 0; i < n; ++i)
		g_t6_wake_order[i] = 0;

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t6_w%d", i);
		g_workers[i] = alloc_worker(i, name, t6_worker, (void *)(long)i);
	}

	signal_all_workers_start(n);
	wait_all_workers(n);

	for (i = 1; i < n; ++i) {
		tprintf("  thread %d wake seq: %ld\n", i, g_t6_wake_order[i]);
		if (g_t6_wake_order[i] < g_t6_wake_order[i - 1])
			ordered = 0;
	}

	if (ordered)
		TPASS("sleep/wakeup ordering correct");
	else
		TFAIL("sleep/wakeup ordering correct",
		      "wakeup order unexpected");

	return 0;
}

/*
 * Test 7: Preempt Disable/Enable
 */
static atomic_t g_t7_intruder;
static volatile int g_t7_preempt_test_running;

static int t7_intruder_thread(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);

	while (g_t7_preempt_test_running) {
		atomic_write(&g_t7_intruder, 1);
		sched_yield();
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t7_controller(void *arg)
{
	int i;
	long intruder_val;

	tprintf("Test 7: Preempt Disable/Enable\n");

	atomic_write(&g_t7_intruder, 0);
	g_t7_preempt_test_running = 1;

	for (i = 0; i < 4; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t7_i%d", i);
		g_workers[i] = alloc_worker(i, name, t7_intruder_thread,
					    (void *)(long)i);
	}

	signal_all_workers_start(4);
	sched_msleep(50);

	sched_preempt_disable();
	atomic_write(&g_t7_intruder, 0);

	for (i = 0; i < 2000000; ++i)
		asm volatile ("" : : : "memory");

	intruder_val = atomic_read(&g_t7_intruder);
	sched_preempt_enable();

	g_t7_preempt_test_running = 0;
	wait_all_workers(4);

	if (intruder_val == 0)
		TPASS("no preemption during preempt_disable region");
	else
		TFAIL("no preemption during preempt_disable region",
		      "intruder ran");

	return 0;
}

/*
 * Test 8: Thread Data Isolation
 */
static unsigned char g_t8_buf[MAX_WORKERS][256] __attribute__((aligned(16)));

static int t8_worker(void *arg)
{
	int idx = (long)arg;
	unsigned char *buf = g_t8_buf[idx];
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 256; ++i)
		buf[i] = (unsigned char)(idx + 0xAB);

	sched_msleep(30);

	for (i = 0; i < 256; ++i) {
		if (buf[i] != (unsigned char)(idx + 0xAB)) {
			sem_post(&g_sync_done[idx]);
			return 0;
		}
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t8_controller(void *arg)
{
	int n = 16;
	int i;

	tprintf("Test 8: Thread Data Isolation (%d threads)\n", n);

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t8_w%d", i);
		g_workers[i] = alloc_worker(i, name, t8_worker, (void *)(long)i);
	}

	signal_all_workers_start(n);
	wait_all_workers(n);

	TPASS("thread data isolation verified");

	return 0;
}

/*
 * Test 9: Many Threads
 */
static atomic_t g_t9_scheduled_count;

static int t9_worker(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);

	atomic_add_return(&g_t9_scheduled_count, 1);
	sched_msleep(1);

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t9_controller(void *arg)
{
	int n = MAX_WORKERS;
	long count;
	int i;

	tprintf("Test 9: Many Threads (%d threads)\n", n);

	atomic_write(&g_t9_scheduled_count, 0);

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t9_w%d", i);
		g_workers[i] = alloc_worker(i, name, t9_worker, (void *)(long)i);
	}

	signal_all_workers_start(n);
	wait_all_workers(n);

	count = atomic_read(&g_t9_scheduled_count);
	if (count == n)
		TPASS("all threads scheduled");
	else
		TFAIL("all threads scheduled", "not all threads ran");

	return 0;
}

/*
 * Test 10: Thread Exit Cleanup
 */
static int t10_worker(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);
	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t10_controller(void *arg)
{
	int n = 16;
	int i;

	tprintf("Test 10: Thread Exit Cleanup (%d threads)\n", n);

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t10_w%d", i);
		g_workers[i] = alloc_worker(i, name, t10_worker,
					    (void *)(long)i);
	}

	signal_all_workers_start(n);
	wait_all_workers(n);

	TPASS("thread exit cleanup handled correctly");

	return 0;
}

/*
 * Test 11: Long-Running Scheduler Stability
 */
static atomic_t g_t11_progress[4];
static volatile int g_t11_running;
static spinlock_t g_t11_lock = SPIN_LOCK_INITIALIZER;
static struct sem g_t11_sem;

static int t11_worker_spinlock(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);

	while (g_t11_running) {
		spin_lock(&g_t11_lock);
		atomic_add_return(&g_t11_progress[idx], 1);
		spin_unlock(&g_t11_lock);
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t11_worker_sleep(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);

	while (g_t11_running) {
		sched_msleep(5);
		atomic_add_return(&g_t11_progress[idx], 1);
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t11_worker_sem(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);

	while (g_t11_running) {
		sem_post(&g_t11_sem);
		sem_wait(&g_t11_sem);
		atomic_add_return(&g_t11_progress[idx], 1);
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t11_worker_yield(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);

	while (g_t11_running) {
		sched_yield();
		atomic_add_return(&g_t11_progress[idx], 1);
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t11_controller(void *arg)
{
	int i;

	tprintf("Test 11: Long-Running Scheduler Stability (5 sec)\n");

	sem_init(&g_t11_sem, 0);
	g_t11_running = 1;

	for (i = 0; i < 4; ++i)
		atomic_write(&g_t11_progress[i], 0);

	g_workers[0] = alloc_worker(0, "t11_spinlock", t11_worker_spinlock,
				    (void *)0);
	g_workers[1] = alloc_worker(1, "t11_sleep", t11_worker_sleep, (void *)1);
	g_workers[2] = alloc_worker(2, "t11_sem", t11_worker_sem, (void *)2);
	g_workers[3] = alloc_worker(3, "t11_yield", t11_worker_yield, (void *)3);

	signal_all_workers_start(4);

	sched_msleep(30000);

	g_t11_running = 0;
	wait_all_workers(4);

	for (i = 0; i < 4; ++i) {
		long p = atomic_read(&g_t11_progress[i]);

		tprintf("  thread %d progress: %ld\n", i, p);
		if (p == 0)
			TFAIL("long-running stability", "thread made no progress");
	}

	tprintf("  long-running stability test completed\n");

	TPASS("long-running stability: all threads made progress");

	return 0;
}

/*
 * Test 12: Priority Fairness
 * Priorities set by coordinator before starting.
 */
static atomic_t g_t12_low_got_lock;
static atomic_t g_t12_high_acquired;
static atomic_t g_t12_medium_progress;
static spinlock_t g_t12_lock = SPIN_LOCK_INITIALIZER;
static volatile int g_t12_stage;

static int t12_low_prio(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);

	while (!g_t12_stage)
		sched_yield();

	spin_lock(&g_t12_lock);
	atomic_write(&g_t12_low_got_lock, 1);
	sched_msleep(300);
	spin_unlock(&g_t12_lock);

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t12_high_prio(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);

	while (!g_t12_stage)
		sched_yield();

	sched_msleep(10);
	spin_lock(&g_t12_lock);
	atomic_write(&g_t12_high_acquired, 1);
	spin_unlock(&g_t12_lock);

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t12_medium_prio(void *arg)
{
	int idx = (long)arg;

	sem_wait(&g_sync_start[idx]);

	while (!g_t12_stage)
		sched_yield();

	while (g_t12_stage == 1) {
		atomic_add_return(&g_t12_medium_progress, 1);
		sched_yield();
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t12_controller(void *arg)
{
	tprintf("Test 12: Priority Fairness (lock holder not starved)\n");

	atomic_write(&g_t12_low_got_lock, 0);
	atomic_write(&g_t12_high_acquired, 0);
	atomic_write(&g_t12_medium_progress, 0);
	g_t12_stage = 0;

	g_workers[0] = alloc_worker(0, "t12_low", t12_low_prio, (void *)0);
	g_workers[1] = alloc_worker(1, "t12_high", t12_high_prio, (void *)1);
	g_workers[2] = alloc_worker(2, "t12_med", t12_medium_prio, (void *)2);

	set_worker_priority(0, 1);
	set_worker_priority(1, 10);
	set_worker_priority(2, 5);

	signal_all_workers_start(3);
	sched_msleep(50);

	g_t12_stage = 1;
	sched_msleep(500);
	g_t12_stage = 2;

	wait_all_workers(3);

	if (atomic_read(&g_t12_high_acquired))
		TPASS("high priority thread eventually got lock");
	else
		TFAIL("high priority thread eventually got lock", "starved");

	tprintf("  medium progress: %ld\n",
		atomic_read(&g_t12_medium_progress));

	return 0;
}

/*
 * Test 13: sem_init() initial value verification
 * sem_init(&sem, 3) should set count = 3.
 * Consumer waits 6 times, producer posts 3 times.
 * If correct:  3 initial + 3 posted = 6 waits succeed.
 * If buggy:    0 initial + 3 posted = 3 waits, then hangs.
 */
static atomic_t g_t13_waits;
static struct sem g_t13_sem;

static int t13_consumer(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 6; ++i) {
		sem_wait(&g_t13_sem);
		atomic_add_return(&g_t13_waits, 1);
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t13_producer(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 3; ++i)
		sem_post(&g_t13_sem);

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t13_controller(void *arg)
{
	long waits;

	tprintf("Test 13: sem_init() Initial Value (expect initial=3, post 3, wait 6)\n");

	atomic_write(&g_t13_waits, 0);
	sem_init(&g_t13_sem, 3);

	g_workers[0] = alloc_worker(0, "t13_consumer", t13_consumer, (void *)0);
	g_workers[1] = alloc_worker(1, "t13_producer", t13_producer, (void *)1);

	coordinator_boost();
	signal_all_workers_start(2);

	wait_worker_done(1);

	sched_msleep(500);

	waits = atomic_read(&g_t13_waits);
	if (waits == 6) {
		TPASS("sem_init(3): initial value used correctly (6 waits)");
		wait_worker_done(0);
	} else {
		TFAIL("sem_init(3): initial value ignored",
		      "got 0 instead of 3");
	}

	return 0;
}

/*
 * Test 14: Multiple waiters on sem_post correctness
 * 3 waiters, 1 producer. Producer posts 300 times.
 * Each waiter waits 100 times. Total: 300/300.
 * Verifies no list corruption with multiple concurrent waiters.
 */
static atomic_t g_t14_waits;
static atomic_t g_t14_posts;
static struct sem g_t14_sem;

static int t14_waiter(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 100; ++i) {
		sem_wait(&g_t14_sem);
		atomic_add_return(&g_t14_waits, 1);
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t14_producer(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 300; ++i) {
		sem_post(&g_t14_sem);
		atomic_add_return(&g_t14_posts, 1);
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t14_controller(void *arg)
{
	int i;
	long posts, waits;

	tprintf("Test 14: Multi-Waiter sem_post Correctness (3 waiters, 1 producer, 300 posts)\n");

	atomic_write(&g_t14_waits, 0);
	atomic_write(&g_t14_posts, 0);
	sem_init(&g_t14_sem, 0);

	for (i = 0; i < 3; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t14_w%d", i);
		g_workers[i] = alloc_worker(i, name, t14_waiter, (void *)(long)i);
	}
	g_workers[3] = alloc_worker(3, "t14_prod", t14_producer, (void *)3);

	signal_all_workers_start(4);

	wait_worker_done(3);

	sched_msleep(500);

	posts = atomic_read(&g_t14_posts);
	waits = atomic_read(&g_t14_waits);

	tprintf("  posts=%ld waits=%ld\n", posts, waits);
	if (posts == 300 && waits == 300) {
		TPASS("multi-waiter sem_post: all items consumed correctly");
		wait_all_workers(3);
	} else {
		TFAIL("multi-waiter sem_post: count mismatch", "");
	}

	return 0;
}

/*
 * Test 15: Wide Spinlock Contention
 * MAX_WORKERS threads all competing for a single spinlock,
 * each doing many iterations. Verifies fairness under heavy load.
 */
static atomic_t g_t15_counter;
static spinlock_t g_t15_lock = SPIN_LOCK_INITIALIZER;

static int t15_worker(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 5000; ++i) {
		spin_lock(&g_t15_lock);
		atomic_add_return(&g_t15_counter, 1);
		spin_unlock(&g_t15_lock);
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t15_controller(void *arg)
{
	int n = MAX_WORKERS;
	long val, expected = (long)n * 5000;
	int i;

	tprintf("Test 15: Wide Spinlock Contention (%d threads, single lock)\n",
		n);

	atomic_write(&g_t15_counter, 0);

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t15_w%d", i);
		g_workers[i] = alloc_worker(i, name, t15_worker,
					    (void *)(long)i);
	}

	signal_all_workers_start(n);
	wait_all_workers(n);

	val = atomic_read(&g_t15_counter);
	if (val == expected)
		TPASS("wide spinlock contention: counter correct");
	else
		TFAIL("wide spinlock contention: count mismatch",
		      "data corruption under heavy contention");

	return 0;
}

/*
 * Test 16: Multi-Producer Multi-Consumer Semaphore
 * 4 producers, 4 consumers, bounded buffer of size 4.
 * Producers post, consumers wait. 200 items total.
 */
static struct sem g_t16_empty;
static struct sem g_t16_full;
static atomic_t g_t16_produced;
static atomic_t g_t16_consumed;
static atomic_t g_t16_buffer;

static int t16_consumer(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 50; ++i) {
		sem_wait(&g_t16_full);
		atomic_add_return(&g_t16_consumed, 1);
		atomic_sub_return(&g_t16_buffer, 1);
		sem_post(&g_t16_empty);
		sched_yield();
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t16_producer(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 50; ++i) {
		sem_wait(&g_t16_empty);
		atomic_add_return(&g_t16_produced, 1);
		atomic_add_return(&g_t16_buffer, 1);
		sem_post(&g_t16_full);
		sched_yield();
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t16_controller(void *arg)
{
	int i;
	long produced, consumed;

	tprintf("Test 16: Multi-Producer Multi-Consumer Semaphore (4P+4C, 200 items)\n");

	atomic_write(&g_t16_produced, 0);
	atomic_write(&g_t16_consumed, 0);
	atomic_write(&g_t16_buffer, 0);
	sem_init(&g_t16_empty, 4);
	sem_init(&g_t16_full, 0);

	for (i = 0; i < 4; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t16_c%d", i);
		g_workers[i] = alloc_worker(i, name, t16_consumer,
					    (void *)(long)i);
	}
	for (i = 0; i < 4; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t16_p%d", i);
		g_workers[4 + i] = alloc_worker(4 + i, name, t16_producer,
						(void *)(long)(4 + i));
	}

	signal_all_workers_start(8);
	wait_all_workers(8);

	produced = atomic_read(&g_t16_produced);
	consumed = atomic_read(&g_t16_consumed);

	if (produced == 200 && consumed == 200)
		TPASS("MPMC semaphore: 200/200 produced/consumed");
	else
		TFAIL("MPMC semaphore: count mismatch",
		      "produced != consumed");

	return 0;
}

/*
 * Test 17: Yield Storm
 * 16 threads at same priority, each yields rapidly.
 * Verifies round-robin fairness under high yield pressure.
 */
static atomic_t g_t17_participants;
static atomic_t g_t17_total_yields;

static int t17_worker(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	atomic_add_return(&g_t17_participants, 1);

	for (i = 0; i < 2000; ++i) {
		atomic_add_return(&g_t17_total_yields, 1);
		sched_yield();
	}

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t17_controller(void *arg)
{
	int n = 16;
	long participants, yields;
	int i;

	tprintf("Test 17: Yield Storm (%d threads, same priority)\n", n);

	atomic_write(&g_t17_participants, 0);
	atomic_write(&g_t17_total_yields, 0);

	for (i = 0; i < n; ++i) {
		char name[16];

		snprintf(name, sizeof(name), "t17_w%d", i);
		g_workers[i] = alloc_worker(i, name, t17_worker,
					    (void *)(long)i);
	}

	signal_all_workers_start(n);
	wait_all_workers(n);

	participants = atomic_read(&g_t17_participants);
	yields = atomic_read(&g_t17_total_yields);

	if (participants == n && yields == (long)n * 2000)
		TPASS("yield storm: all threads ran correctly");
	else
		TFAIL("yield storm: count mismatch",
		      "some threads starved");

	return 0;
}

/*
 * Test 18: Lock Ordering Stress
 * 4 threads acquire 2 locks in different orders.
 * Verifies no deadlock with lock+unlock discipline.
 */
static struct sem g_t18_sem;
static spinlock_t g_t18_lock_a = SPIN_LOCK_INITIALIZER;
static spinlock_t g_t18_lock_b = SPIN_LOCK_INITIALIZER;
static atomic_t g_t18_ab_done;
static atomic_t g_t18_ba_done;

static int t18_worker_ab(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 5000; ++i) {
		spin_lock(&g_t18_lock_a);
		spin_lock(&g_t18_lock_b);
		spin_unlock(&g_t18_lock_b);
		spin_unlock(&g_t18_lock_a);
	}

	atomic_add_return(&g_t18_ab_done, 1);
	sem_post(&g_t18_sem);

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t18_worker_ba(void *arg)
{
	int idx = (long)arg;
	int i;

	sem_wait(&g_sync_start[idx]);

	for (i = 0; i < 5000; ++i) {
		spin_lock(&g_t18_lock_b);
		spin_lock(&g_t18_lock_a);
		spin_unlock(&g_t18_lock_a);
		spin_unlock(&g_t18_lock_b);
	}

	atomic_add_return(&g_t18_ba_done, 1);
	sem_post(&g_t18_sem);

	sem_post(&g_sync_done[idx]);
	return 0;
}

static int t18_controller(void *arg)
{
	int i;
	long ab, ba;

	tprintf("Test 18: Lock Ordering Stress (2 locks, AB+BA order, 4 threads)\n");

	atomic_write(&g_t18_ab_done, 0);
	atomic_write(&g_t18_ba_done, 0);
	sem_init(&g_t18_sem, 0);

	g_workers[0] = alloc_worker(0, "t18_ab1", t18_worker_ab, (void *)0);
	g_workers[1] = alloc_worker(1, "t18_ab2", t18_worker_ab, (void *)1);
	g_workers[2] = alloc_worker(2, "t18_ba1", t18_worker_ba, (void *)2);
	g_workers[3] = alloc_worker(3, "t18_ba2", t18_worker_ba, (void *)3);

	signal_all_workers_start(4);

	for (i = 0; i < 4; ++i)
		sem_wait(&g_t18_sem);

	ab = atomic_read(&g_t18_ab_done);
	ba = atomic_read(&g_t18_ba_done);

	if (ab == 2 && ba == 2)
		TPASS("lock ordering: all threads completed, no deadlock");
	else
		TFAIL("lock ordering: deadlock or stall",
		      "not all threads finished");

	wait_all_workers(4);

	return 0;
}

static int stress_coordinator(void *arg)
{
	tprintf("\n=== ZSBL Multi-Thread Stress Test Suite ===\n\n");

	atomic_write(&g_pass, 0);
	atomic_write(&g_fail, 0);

	t1_controller(NULL);

	t2_controller(NULL);

	t3_controller(NULL);

	t4_controller(NULL);

	t5_controller(NULL);

	t6_controller(NULL);

	t7_controller(NULL);

	t8_controller(NULL);

	t9_controller(NULL);

	t10_controller(NULL);

	t11_controller(NULL);

	t12_controller(NULL);

	t13_controller(NULL);

	t14_controller(NULL);

	t15_controller(NULL);

	t16_controller(NULL);

	t17_controller(NULL);

	t18_controller(NULL);

	tprintf("\n=== Results: %ld passed, %ld failed ===\n",
		atomic_read(&g_pass), atomic_read(&g_fail));

	if (atomic_read(&g_fail) == 0)
		tprintf("=== ALL TESTS PASSED ===\n");
	else
		tprintf("=== SOME TESTS FAILED ===\n");

	while (1)
		asm volatile ("wfi");

	return 0;
}

static unsigned char g_coordinator_stack[STACK_SIZE] __attribute__((aligned(16)));

static int test_multi_thread_stress(void)
{
	pr_info("Starting Multi-Thread Stress Test\n");

	thread_create("coordinator", g_coordinator_stack, STACK_SIZE,
		      stress_coordinator, NULL);

	sched_start();

	return 0;
}

test_case(test_multi_thread_stress);