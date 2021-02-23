#include <poll.h>

#include <err.h>
#include <errno.h>

#include "utility.h"
#include "poller.h"

struct callback_storer {
    poller_callback callback;
    void *data;
};

static struct pollfd *fds;
static nfds_t nfds;
static struct callback_storer *callbacks;

void init_poller()
{}

static short fromEvent(enum Event events)
{
    short result = 0;
    if (events & read_ready)
        result |= POLLIN;
    return result;
}
static enum Event toEvent(short events)
{
    if (events & POLLNVAL)
        return invalid_fd;

    enum Event result = 0;

    if (events & POLLIN)
        result |= read_ready;
    if (events & POLLERR)
        result |= error;
    if (events & POLLHUP)
        result |= hup;

    return result;
}

void request_polling(int fd, enum Event events, poller_callback callback, void *data)
{
    ++nfds;
    reallocate(fds,       nfds);
    reallocate(callbacks, nfds);

    fds[nfds - 1] = (struct pollfd){
        .fd = fd,
        .events = fromEvent(events)
    };
    callbacks[nfds - 1] = (struct callback_storer){
        .callback = callback,
        .data = data
    };
}

static nfds_t skip_empty_revents(nfds_t i)
{
    for (; fds[i].revents == 0; ++i)
        ;
    return i;
}

void perform_polling(int timeout)
{
    if (nfds == 0)
        return;

    int result = 0;
    do {
        result = poll(fds, nfds, timeout);
    } while (result == -1 && errno == EINTR);
    /* Use < 0 here to tell the compiler that in the loop below, result cannot be less than 0 */
    if (result < 0)
        err(1, "%s failed", "poll");

    nfds_t i = 0;
    for (int cnt = 0; cnt != result; ++cnt) {
        i = skip_empty_revents(i);

        struct callback_storer *storer = &callbacks[i];
        storer->callback(fds[i].fd, toEvent(fds[i].revents), storer->data);

        ++i;
    }
}
