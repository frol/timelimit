timelimit utility
=================

This is the fork of [Peter Pentchev's timelimit util](http://devel.ringlet.net/sysutils/timelimit/).

The timelimit utility executes a given command with the supplied arguments and terminates the
spawned process after a given time with a given signal. If the process exits before the time
limit has elapsed, timelimit will silently exit, too.

The Fork Notes
--------------

This fork added a feature to limit child process CPU time via `setitimer(ITIMER_VIRTUAL)`, use `-c`
as you would use `-t`, but unfortunately, you cannot change the signal sent to a child process by
design, it will be always `SIGVTALRM` (see details about `ITIMER_VIRTUAL` in man pages to
`setitimer`).

It is important to understand that:

* `setitimer(ITIMER_VIRTUAL)` is applied directly to the child process and does not count CPU
time of grand children;
* `setitimer(ITIMER_VIRTUAL)` sends SIGVTALRM to the child process and if it ignores it (see an
example below), the limitation does not work at all, so I suggest to use `-c` in a combination
with `-t`.


Other tries to implement this:

* `timer_create(CLOCK_PROCESS_CPUTIME_ID)` is not inherited by `fork`/`execve`, so it cannot be
applied here.

Other added features:

* `-Q` - silence child program stdout/stderr/stdin;
* `-u <uid>` - drop privileges to the specified user id;
* `-g <gid>` - drop privileges to the specified group id.


Example of bypassing `setitimer(ITIMER_VIRTUAL)`
------------------------------------------------

```c
#include <stdlib.h>
#include <signal.h>

int main()
{
    sigset_t mask;

    /* Block timer signal */
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_SETMASK, &mask, NULL);
   
    int i, a = 0;
    for (i = 0; i < 1000000000; ++i)
        a += i;
}
```

If you compile this and try to limit CPU time, say, with 0.1 second (`timelimit -c 0.1 ./example`),
this won't have any effect as the `./example` program ignores `SIGVTALARM`.

I couldn't find any solution to this issue, so if anybody will help me to solve it I will be very
pleased.

For now, I use real time timer in addition to the CPU time timer and see if CPU time was exceeded
after I kill the process:

```bash
$ time timelimit -s KILL -t 1.0 -c 0.1 ./blackbox_binary
```

Extra notes
-----------

I use `timelimit -t 1.0 -c 0.1` in a combination with
`cgmemtime --kill-children-on-next --memory-limit <bytes>` (there is also my fork with added options)
to test black-box solutions for online programming contests.
