// conditiontest.cc
//	Simple test case for the condition variable assignment.

#include <unistd.h>

#include "system.h"
#include "synch.h"

static Lock lock("lock condition");
static Condition empty("empty list", &lock);
static List<int> list;

void
Consumer(void *name)
{
	while (1) {
		lock.Acquire();

		while (list.IsEmpty())
			empty.Wait();

		int r = list.Remove();
		printf("Got %d from the producer\n", r);

		lock.Release();
	}
}

void
Producer(void *name)
{
	int i = 10;

	while (i--) {
			currentThread->Yield();
			lock.Acquire();
			sleep(1);
			int n = 123;
			list.Append(n);
			printf("Produced a %d\n", n);
			empty.Signal();
			lock.Release();
	}
}

void
ConditionTest()
{
	DEBUG('t', "Entering ConditionTest");

	Thread* newThread = new Thread ("Producer");
	newThread->Fork (Producer, NULL);

	Consumer(NULL);
}
