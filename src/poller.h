#ifndef  __swaystatus_poller_HPP__
# define __swaystatus_poller_HPP__

# ifdef __cplusplus
extern "C" {
# endif

void init_poller();

enum Event {
    read_ready = 1 << 0,
    /**
     * error, hup and invalid_fd can be set in event for the poller_callback,
     * no matter it is registed with it or not.
     */
    error      = 1 << 2,
    hup        = 1 << 3,
    invalid_fd = -1,
};
typedef void (*poller_callback)(int fd, enum Event events, void *data);

void request_polling(int fd, enum Event events, poller_callback callback, void *data);

void perform_polling(int timeout);

# ifdef __cplusplus
}
# endif

#endif
