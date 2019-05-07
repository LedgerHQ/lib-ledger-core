<<<<<<< HEAD
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_FLAT_BUFFER_HPP
#define BOOST_BEAST_FLAT_BUFFER_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/detail/allocator.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/core/empty_value.hpp>
#include <limits>
#include <memory>
#include <type_traits>

namespace boost {
namespace beast {

/** A dynamic buffer providing buffer sequences of length one.

    A dynamic buffer encapsulates memory storage that may be
    automatically resized as required, where the memory is
    divided into two regions: readable bytes followed by
    writable bytes. These memory regions are internal to
    the dynamic buffer, but direct access to the elements
    is provided to permit them to be efficiently used with
    I/O operations.

    Objects of this type meet the requirements of <em>DynamicBuffer</em>
    and have the following additional properties:

    @li A mutable buffer sequence representing the readable
    bytes is returned by @ref data when `this` is non-const.

    @li A configurable maximum buffer size may be set upon
    construction. Attempts to exceed the buffer size will throw
    `std::length_error`.

    @li Buffer sequences representing the readable and writable
    bytes, returned by @ref data and @ref prepare, will have
    length one.

    Upon construction, a maximum size for the buffer may be
    specified. If this limit is exceeded, the `std::length_error`
    exception will be thrown.

    @note This class is designed for use with algorithms that
    take dynamic buffers as parameters, and are optimized
    for the case where the input sequence or output sequence
    is stored in a single contiguous buffer.
*/
template<class Allocator>
class basic_flat_buffer
#if ! BOOST_BEAST_DOXYGEN
    : private boost::empty_value<
        typename detail::allocator_traits<Allocator>::
            template rebind_alloc<char>>
#endif
{
    template<class OtherAlloc>
    friend class basic_flat_buffer;

    using base_alloc_type = typename
        detail::allocator_traits<Allocator>::
            template rebind_alloc<char>;

    static bool constexpr default_nothrow =
        std::is_nothrow_default_constructible<Allocator>::value;

    using alloc_traits =
        beast::detail::allocator_traits<base_alloc_type>;

    using pocma = typename
        alloc_traits::propagate_on_container_move_assignment;

    using pocca = typename
        alloc_traits::propagate_on_container_copy_assignment;

    static
    std::size_t
    dist(char const* first, char const* last) noexcept
    {
        return static_cast<std::size_t>(last - first);
    }

    char* begin_;
    char* in_;
    char* out_;
    char* last_;
    char* end_;
    std::size_t max_;

public:
    /// The type of allocator used.
    using allocator_type = Allocator;

    /// Destructor
    ~basic_flat_buffer();

    /** Constructor

        After construction, @ref capacity will return zero, and
        @ref max_size will return the largest value which may
        be passed to the allocator's `allocate` function.
    */
    basic_flat_buffer() noexcept(default_nothrow);

    /** Constructor

        After construction, @ref capacity will return zero, and
        @ref max_size will return the specified value of `limit`.

        @param limit The desired maximum size.
    */
    explicit
    basic_flat_buffer(
        std::size_t limit) noexcept(default_nothrow);

    /** Constructor

        After construction, @ref capacity will return zero, and
        @ref max_size will return the largest value which may
        be passed to the allocator's `allocate` function.

        @param alloc The allocator to use for the object.

        @esafe

        No-throw guarantee.
    */
    explicit
    basic_flat_buffer(Allocator const& alloc) noexcept;

    /** Constructor

        After construction, @ref capacity will return zero, and
        @ref max_size will return the specified value of `limit`.

        @param limit The desired maximum size.

        @param alloc The allocator to use for the object.

        @esafe

        No-throw guarantee.
    */
    basic_flat_buffer(
        std::size_t limit,
        Allocator const& alloc) noexcept;

    /** Move Constructor

        The container is constructed with the contents of `other`
        using move semantics. The maximum size will be the same
        as the moved-from object.

        Buffer sequences previously obtained from `other` using
        @ref data or @ref prepare remain valid after the move.

        @param other The object to move from. After the move, the
        moved-from object will have zero capacity, zero readable
        bytes, and zero writable bytes.

        @esafe

        No-throw guarantee.
    */
    basic_flat_buffer(basic_flat_buffer&& other) noexcept;

    /** Move Constructor

        Using `alloc` as the allocator for the new container, the
        contents of `other` are moved. If `alloc != other.get_allocator()`,
        this results in a copy. The maximum size will be the same
        as the moved-from object.

        Buffer sequences previously obtained from `other` using
        @ref data or @ref prepare become invalid after the move.

        @param other The object to move from. After the move,
        the moved-from object will have zero capacity, zero readable
        bytes, and zero writable bytes.

        @param alloc The allocator to use for the object.

        @throws std::length_error if `other.size()` exceeds the
        maximum allocation size of `alloc`.
    */
    basic_flat_buffer(
        basic_flat_buffer&& other,
        Allocator const& alloc);

    /** Copy Constructor

        This container is constructed with the contents of `other`
        using copy semantics. The maximum size will be the same
        as the copied object.

        @param other The object to copy from.

        @throws std::length_error if `other.size()` exceeds the
        maximum allocation size of the allocator.
    */
    basic_flat_buffer(basic_flat_buffer const& other);

    /** Copy Constructor

        This container is constructed with the contents of `other`
        using copy semantics and the specified allocator. The maximum
        size will be the same as the copied object.

        @param other The object to copy from.

        @param alloc The allocator to use for the object.

        @throws std::length_error if `other.size()` exceeds the
        maximum allocation size of `alloc`.
    */
    basic_flat_buffer(
        basic_flat_buffer const& other,
        Allocator const& alloc);

    /** Copy Constructor

        This container is constructed with the contents of `other`
        using copy semantics. The maximum size will be the same
        as the copied object.

        @param other The object to copy from.

        @throws std::length_error if `other.size()` exceeds the
        maximum allocation size of the allocator.
    */
    template<class OtherAlloc>
    basic_flat_buffer(
        basic_flat_buffer<OtherAlloc> const& other)
            noexcept(default_nothrow);

    /** Copy Constructor

        This container is constructed with the contents of `other`
        using copy semantics. The maximum size will be the same
        as the copied object.

        @param other The object to copy from.

        @param alloc The allocator to use for the object.

        @throws std::length_error if `other.size()` exceeds the
        maximum allocation size of `alloc`.
    */
    template<class OtherAlloc>
    basic_flat_buffer(
        basic_flat_buffer<OtherAlloc> const& other,
        Allocator const& alloc);

    /** Move Assignment

        The container is assigned with the contents of `other`
        using move semantics. The maximum size will be the same
        as the moved-from object.

        Buffer sequences previously obtained from `other` using
        @ref data or @ref prepare remain valid after the move.

        @param other The object to move from. After the move,
        the moved-from object will have zero capacity, zero readable
        bytes, and zero writable bytes.

        @esafe

        No-throw guarantee.
    */
    basic_flat_buffer&
    operator=(basic_flat_buffer&& other) noexcept;

    /** Copy Assignment

        The container is assigned with the contents of `other`
        using copy semantics. The maximum size will be the same
        as the copied object.

        After the copy, `this` will have zero writable bytes.

        @param other The object to copy from.

        @throws std::length_error if `other.size()` exceeds the
        maximum allocation size of the allocator.
    */
    basic_flat_buffer&
    operator=(basic_flat_buffer const& other);

    /** Copy assignment

        The container is assigned with the contents of `other`
        using copy semantics. The maximum size will be the same
        as the copied object.

        After the copy, `this` will have zero writable bytes.

        @param other The object to copy from.

        @throws std::length_error if `other.size()` exceeds the
        maximum allocation size of the allocator.
    */
    template<class OtherAlloc>
    basic_flat_buffer&
    operator=(basic_flat_buffer<OtherAlloc> const& other);

    /// Returns a copy of the allocator used.
    allocator_type
    get_allocator() const
    {
        return this->get();
    }

    /** Set the maximum allowed capacity

        This function changes the currently configured upper limit
        on capacity to the specified value.

        @param n The maximum number of bytes ever allowed for capacity.

        @esafe

        No-throw guarantee.
    */
    void
    max_size(std::size_t n) noexcept
    {
        max_ = n;
    }

    /** Guarantee a minimum capacity

        This function adjusts the internal storage (if necessary)
        to guarantee space for at least `n` bytes.

        Buffer sequences previously obtained using @ref data or
        @ref prepare become invalid.

        @param n The minimum number of byte for the new capacity.
        If this value is greater than the maximum size, then the
        maximum size will be adjusted upwards to this value.

        @esafe

        Basic guarantee.

        @throws std::length_error if n is larger than the maximum
        allocation size of the allocator.
    */
    void
    reserve(std::size_t n);

    /** Reallocate the buffer to fit the readable bytes exactly.

        Buffer sequences previously obtained using @ref data or
        @ref prepare become invalid.

        @esafe

        Strong guarantee.
    */
    void
    shrink_to_fit();

    /** Set the size of the readable and writable bytes to zero.

        This clears the buffer without changing capacity.
        Buffer sequences previously obtained using @ref data or
        @ref prepare become invalid.

        @esafe

        No-throw guarantee.
    */
    void
    clear() noexcept;

    /// Exchange two dynamic buffers
    template<class Alloc>
    friend
    void
    swap(
        basic_flat_buffer<Alloc>&,
        basic_flat_buffer<Alloc>&);

    //--------------------------------------------------------------------------

    /// The ConstBufferSequence used to represent the readable bytes.
    using const_buffers_type = net::const_buffer;

    /// The MutableBufferSequence used to represent the readable bytes.
    using mutable_data_type = net::mutable_buffer;

    /// The MutableBufferSequence used to represent the writable bytes.
    using mutable_buffers_type = net::mutable_buffer;

    /// Returns the number of readable bytes.
    std::size_t
    size() const noexcept
    {
        return dist(in_, out_);
    }

    /// Return the maximum number of bytes, both readable and writable, that can ever be held.
    std::size_t
    max_size() const noexcept
    {
        return max_;
    }

    /// Return the maximum number of bytes, both readable and writable, that can be held without requiring an allocation.
    std::size_t
    capacity() const noexcept
    {
        return dist(begin_, end_);
    }

    /// Returns a constant buffer sequence representing the readable bytes
    const_buffers_type
    data() const noexcept
    {
        return {in_, dist(in_, out_)};
    }

    /// Returns a constant buffer sequence representing the readable bytes
    const_buffers_type
    cdata() const noexcept
    {
        return data();
    }

    /// Returns a mutable buffer sequence representing the readable bytes
    mutable_data_type
    data() noexcept
    {
        return {in_, dist(in_, out_)};
    }

    /** Returns a mutable buffer sequence representing writable bytes.
    
        Returns a mutable buffer sequence representing the writable
        bytes containing exactly `n` bytes of storage. Memory may be
        reallocated as needed.

        All buffers sequences previously obtained using
        @ref data or @ref prepare become invalid.

        @param n The desired number of bytes in the returned buffer
        sequence.

        @throws std::length_error if `size() + n` exceeds either
        `max_size()` or the allocator's maximum allocation size.

        @esafe

        Strong guarantee.
    */
    mutable_buffers_type
    prepare(std::size_t n);

    /** Append writable bytes to the readable bytes.

        Appends n bytes from the start of the writable bytes to the
        end of the readable bytes. The remainder of the writable bytes
        are discarded. If n is greater than the number of writable
        bytes, all writable bytes are appended to the readable bytes.

        All buffers sequences previously obtained using
        @ref data or @ref prepare become invalid.

        @param n The number of bytes to append. If this number
        is greater than the number of writable bytes, all
        writable bytes are appended.

        @esafe

        No-throw guarantee.
    */
    void
    commit(std::size_t n) noexcept
    {
        out_ += (std::min)(n, dist(out_, last_));
    }

    /** Remove bytes from beginning of the readable bytes.

        Removes n bytes from the beginning of the readable bytes.

        All buffers sequences previously obtained using
        @ref data or @ref prepare become invalid.

        @param n The number of bytes to remove. If this number
        is greater than the number of readable bytes, all
        readable bytes are removed.

        @esafe

        No-throw guarantee.
    */
    void
    consume(std::size_t n) noexcept;

private:
    template<class OtherAlloc>
    void copy_from(basic_flat_buffer<OtherAlloc> const& other);
    void move_assign(basic_flat_buffer&, std::true_type);
    void move_assign(basic_flat_buffer&, std::false_type);
    void copy_assign(basic_flat_buffer const&, std::true_type);
    void copy_assign(basic_flat_buffer const&, std::false_type);
    void swap(basic_flat_buffer&);
    void swap(basic_flat_buffer&, std::true_type);
    void swap(basic_flat_buffer&, std::false_type);
    char* alloc(std::size_t n);
};

/// A flat buffer which uses the default allocator.
using flat_buffer =
    basic_flat_buffer<std::allocator<char>>;

} // beast
} // boost

#include <boost/beast/core/impl/flat_buffer.hpp>

#endif
=======
//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_FLAT_BUFFER_HPP
#define BOOST_BEAST_FLAT_BUFFER_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/detail/allocator.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/core/empty_value.hpp>
#include <limits>
#include <memory>

namespace boost {
namespace beast {

/** A linear dynamic buffer.

    Objects of this type meet the requirements of @b DynamicBuffer
    and offer additional invariants:
    
    @li Buffer sequences returned by @ref data and @ref prepare
    will always be of length one.

    @li A configurable maximum buffer size may be set upon
    construction. Attempts to exceed the buffer size will throw
    `std::length_error`.

    Upon construction, a maximum size for the buffer may be
    specified. If this limit is exceeded, the `std::length_error`
    exception will be thrown.

    @note This class is designed for use with algorithms that
    take dynamic buffers as parameters, and are optimized
    for the case where the input sequence or output sequence
    is stored in a single contiguous buffer.
*/
template<class Allocator>
class basic_flat_buffer
#if ! BOOST_BEAST_DOXYGEN
    : private boost::empty_value<
        typename detail::allocator_traits<Allocator>::
            template rebind_alloc<char>>
#endif
{
    enum
    {
        min_size = 512
    };

    template<class OtherAlloc>
    friend class basic_flat_buffer;

    using base_alloc_type = typename
        detail::allocator_traits<Allocator>::
            template rebind_alloc<char>;

    using alloc_traits =
        detail::allocator_traits<base_alloc_type>;

    static
    inline
    std::size_t
    dist(char const* first, char const* last)
    {
        return static_cast<std::size_t>(last - first);
    }

    char* begin_;
    char* in_;
    char* out_;
    char* last_;
    char* end_;
    std::size_t max_;

public:
    /// The type of allocator used.
    using allocator_type = Allocator;

    /// The type used to represent the input sequence as a list of buffers.
    using const_buffers_type = boost::asio::const_buffer;

    /// The type used to represent the output sequence as a list of buffers.
    using mutable_buffers_type = boost::asio::mutable_buffer;

    /// Destructor
    ~basic_flat_buffer();

    /** Constructor

        Upon construction, capacity will be zero.
    */
    basic_flat_buffer();

    /** Constructor

        Upon construction, capacity will be zero.

        @param limit The setting for @ref max_size.
    */
    explicit
    basic_flat_buffer(std::size_t limit);

    /** Constructor

        Upon construction, capacity will be zero.

        @param alloc The allocator to construct with.
    */
    explicit
    basic_flat_buffer(Allocator const& alloc);

    /** Constructor

        Upon construction, capacity will be zero.

        @param limit The setting for @ref max_size.

        @param alloc The allocator to use.
    */
    basic_flat_buffer(
        std::size_t limit, Allocator const& alloc);

    /** Constructor

        After the move, `*this` will have an empty output sequence.

        @param other The object to move from. After the move,
        The object's state will be as if constructed using
        its current allocator and limit.
    */
    basic_flat_buffer(basic_flat_buffer&& other);

    /** Constructor

        After the move, `*this` will have an empty output sequence.

        @param other The object to move from. After the move,
        The object's state will be as if constructed using
        its current allocator and limit.

        @param alloc The allocator to use.
    */
    basic_flat_buffer(
        basic_flat_buffer&& other, Allocator const& alloc);

    /** Constructor

        @param other The object to copy from.
    */
    basic_flat_buffer(basic_flat_buffer const& other);

    /** Constructor

        @param other The object to copy from.

        @param alloc The allocator to use.
    */
    basic_flat_buffer(basic_flat_buffer const& other,
        Allocator const& alloc);

    /** Constructor

        @param other The object to copy from.
    */
    template<class OtherAlloc>
    basic_flat_buffer(
        basic_flat_buffer<OtherAlloc> const& other);

    /** Constructor

        @param other The object to copy from.

        @param alloc The allocator to use.
    */
    template<class OtherAlloc>
    basic_flat_buffer(
        basic_flat_buffer<OtherAlloc> const& other,
            Allocator const& alloc);

    /** Assignment

        After the move, `*this` will have an empty output sequence.

        @param other The object to move from. After the move,
        the object's state will be as if constructed using
        its current allocator and limit.
    */
    basic_flat_buffer&
    operator=(basic_flat_buffer&& other);

    /** Assignment

        After the copy, `*this` will have an empty output sequence.

        @param other The object to copy from.
    */
    basic_flat_buffer&
    operator=(basic_flat_buffer const& other);

    /** Copy assignment

        After the copy, `*this` will have an empty output sequence.

        @param other The object to copy from.
    */
    template<class OtherAlloc>
    basic_flat_buffer&
    operator=(basic_flat_buffer<OtherAlloc> const& other);

    /// Returns a copy of the associated allocator.
    allocator_type
    get_allocator() const
    {
        return this->get();
    }

    /// Returns the size of the input sequence.
    std::size_t
    size() const
    {
        return dist(in_, out_);
    }

    /// Return the maximum sum of the input and output sequence sizes.
    std::size_t
    max_size() const
    {
        return max_;
    }

    /// Return the maximum sum of input and output sizes that can be held without an allocation.
    std::size_t
    capacity() const
    {
        return dist(begin_, end_);
    }

    /// Get a list of buffers that represent the input sequence.
    const_buffers_type
    data() const
    {
        return {in_, dist(in_, out_)};
    }

    /** Get a list of buffers that represent the output sequence, with the given size.

        @throws std::length_error if `size() + n` exceeds `max_size()`.

        @note All previous buffers sequences obtained from
        calls to @ref data or @ref prepare are invalidated.
    */
    mutable_buffers_type
    prepare(std::size_t n);

    /** Move bytes from the output sequence to the input sequence.

        @param n The number of bytes to move. If this is larger than
        the number of bytes in the output sequences, then the entire
        output sequences is moved.

        @note All previous buffers sequences obtained from
        calls to @ref data or @ref prepare are invalidated.
    */
    void
    commit(std::size_t n)
    {
        out_ += (std::min)(n, dist(out_, last_));
    }

    /** Remove bytes from the input sequence.

        If `n` is greater than the number of bytes in the input
        sequence, all bytes in the input sequence are removed.

        @note All previous buffers sequences obtained from
        calls to @ref data or @ref prepare are invalidated.
    */
    void
    consume(std::size_t n);

    /** Reallocate the buffer to fit the input sequence.

        @note All previous buffers sequences obtained from
        calls to @ref data or @ref prepare are invalidated.
    */
    void
    shrink_to_fit();

    /// Exchange two flat buffers
    template<class Alloc>
    friend
    void
    swap(
        basic_flat_buffer<Alloc>& lhs,
        basic_flat_buffer<Alloc>& rhs);

private:
    void
    reset();

    template<class DynamicBuffer>
    void
    copy_from(DynamicBuffer const& other);

    void
    move_assign(basic_flat_buffer&, std::true_type);

    void
    move_assign(basic_flat_buffer&, std::false_type);

    void
    copy_assign(basic_flat_buffer const&, std::true_type);

    void
    copy_assign(basic_flat_buffer const&, std::false_type);

    void
    swap(basic_flat_buffer&);

    void
    swap(basic_flat_buffer&, std::true_type);

    void
    swap(basic_flat_buffer&, std::false_type);
};

using flat_buffer =
    basic_flat_buffer<std::allocator<char>>;

} // beast
} // boost

#include <boost/beast/core/impl/flat_buffer.ipp>

#endif
>>>>>>> 0ab5dada0... Continuing the XRP segregation.
