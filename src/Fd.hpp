#ifndef  __swaystatus_Fd_HPP__
# define __swaystatus_Fd_HPP__

namespace swaystatus {
class Fd {
    int fd = -1;

public:
    Fd() = default;

    /**
     * @param must be a valid file descripter
     */
    Fd(int fd) noexcept;

    Fd(const Fd&) = delete;
    Fd(Fd&&) noexcept;

    Fd& operator = (const Fd&) = delete;
    Fd& operator = (Fd&&) noexcept;

    void destroy() noexcept;

    ~Fd();

    /**
     * @return true if Fd contains a valid file descripter
     */
    explicit operator bool () const noexcept;
    /**
     * @return valid fd if this->operator bool() returns true.
     */
    int get() const noexcept;
};
} /* namespace swaystatus */

#endif
