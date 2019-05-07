<<<<<<< HEAD
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_WEBSOCKET_STREAM_HPP
#define BOOST_BEAST_WEBSOCKET_STREAM_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/websocket/error.hpp>
#include <boost/beast/websocket/option.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/beast/websocket/stream_base.hpp>
#include <boost/beast/websocket/stream_fwd.hpp>
#include <boost/beast/websocket/detail/hybi13.hpp>
#include <boost/beast/websocket/detail/impl_base.hpp>
#include <boost/beast/websocket/detail/pmd_extension.hpp>
#include <boost/beast/websocket/detail/prng.hpp>
#include <boost/beast/core/role.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/core/string.hpp>
#include <boost/beast/core/detail/type_traits.hpp>
#include <boost/beast/http/detail/type_traits.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/error.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <random>

namespace boost {
namespace beast {
namespace websocket {

/** The type of received control frame.

    Values of this type are passed to the control frame
    callback set using @ref stream::control_callback.
*/
enum class frame_type
{
    /// A close frame was received
    close,

    /// A ping frame was received
    ping,

    /// A pong frame was received
    pong
};

namespace detail {
class frame_test;
} // detail

//--------------------------------------------------------------------

/** Provides message-oriented functionality using WebSocket.

    The @ref stream class template provides asynchronous and blocking
    message-oriented functionality necessary for clients and servers
    to utilize the WebSocket protocol.

    For asynchronous operations, the application must ensure
    that they are are all performed within the same implicit
    or explicit strand.

    @par Thread Safety
    @e Distinct @e objects: Safe.@n
    @e Shared @e objects: Unsafe.
    The application must also ensure that all asynchronous
    operations are performed within the same implicit or explicit strand.

    @par Example
    To declare the @ref stream object with a @ref tcp_stream in a
    multi-threaded asynchronous program using a strand, you may write:
    @code
    websocket::stream<tcp_stream> ws{net::io_context::strand(ioc)};
    @endcode
    Alternatively, for a single-threaded or synchronous application
    you may write:
    @code
    websocket::stream<tcp_stream> ws(ioc);
    @endcode

    @tparam NextLayer The type representing the next layer, to which
    data will be read and written during operations. For synchronous
    operations, the type must support the <em>SyncStream</em> concept.
    For asynchronous operations, the type must support the
    <em>AsyncStream</em> concept.

    @tparam deflateSupported A `bool` indicating whether or not the
    stream will be capable of negotiating the permessage-deflate websocket
    extension. Note that even if this is set to `true`, the permessage
    deflate options (set by the caller at runtime) must still have the
    feature enabled for a successful negotiation to occur.

    @note A stream object must not be moved or destroyed while there
    are pending asynchronous operations associated with it.

    @par Concepts
        @li <em>AsyncStream</em>
        @li <em>DynamicBuffer</em>
        @li <em>SyncStream</em>

    @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket Closing Handshake (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">Websocket Close (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">WebSocket Ping (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">WebSocket Pong (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
*/
template<
    class NextLayer,
    bool deflateSupported>
class stream
#if ! BOOST_BEAST_DOXYGEN
    : private stream_base
#endif
{
    struct impl_type;

    boost::shared_ptr<impl_type> impl_;

    using time_point = typename
        std::chrono::steady_clock::time_point;

    using control_cb_type =
        std::function<void(frame_type, string_view)>;

    friend class close_test;
    friend class frame_test;
    friend class ping_test;
    friend class read2_test;
    friend class read3_test;
    friend class stream_test;
    friend class write_test;

    /*  The read buffer has to be at least as large
        as the largest possible control frame including
        the frame header.
    */
    static std::size_t constexpr max_control_frame_size = 2 + 8 + 4 + 125;
    static std::size_t constexpr tcp_frame_size = 1536;

    static time_point never() noexcept
    {
        return (time_point::max)();
    }

public:
    /// Indicates if the permessage-deflate extension is supported
    using is_deflate_supported =
        std::integral_constant<bool, deflateSupported>;

    /// The type of the next layer.
    using next_layer_type =
        typename std::remove_reference<NextLayer>::type;

    /// The type of the executor associated with the object.
    using executor_type =
        beast::executor_type<next_layer_type>;

    /** Destructor

        Destroys the stream and all associated resources.

        @note A stream object must not be destroyed while there
        are pending asynchronous operations associated with it.
    */
    ~stream();

    /** Constructor

        If `NextLayer` is move constructible, this function
        will move-construct a new stream from the existing stream.

        After the move, the only valid operation on the moved-from
        object is destruction.
    */
    stream(stream&&) = default;

    /// Move assignment (deleted)
    stream& operator=(stream&&) = delete;

    /** Constructor

        This constructor creates a websocket stream and initializes
        the next layer object.

        @throws Any exceptions thrown by the NextLayer constructor.

        @param args The arguments to be passed to initialize the
        next layer object. The arguments are forwarded to the next
        layer's constructor.
    */
    template<class... Args>
    explicit
    stream(Args&&... args);

    //--------------------------------------------------------------------------

    /** Get the executor associated with the object.

        This function may be used to obtain the executor object that the
        stream uses to dispatch handlers for asynchronous operations.

        @return A copy of the executor that stream will use to dispatch handlers.
    */
    executor_type
    get_executor() const noexcept;

    /** Get a reference to the next layer

        This function returns a reference to the next layer
        in a stack of stream layers.

        @return A reference to the next layer in the stack of
        stream layers.
    */
    next_layer_type&
    next_layer() noexcept;

    /** Get a reference to the next layer

        This function returns a reference to the next layer in a
        stack of stream layers.

        @return A reference to the next layer in the stack of
        stream layers.
    */
    next_layer_type const&
    next_layer() const noexcept;

    //--------------------------------------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------------------------------------

    /** Returns `true` if the stream is open.

        The stream is open after a successful handshake, and when
        no error has occurred.
    */
    bool
    is_open() const noexcept;

    /** Returns `true` if the latest message data indicates binary.

        This function informs the caller of whether the last
        received message frame represents a message with the
        binary opcode.

        If there is no last message frame, the return value is
        undefined.
    */
    bool
    got_binary() const noexcept;

    /** Returns `true` if the latest message data indicates text.

        This function informs the caller of whether the last
        received message frame represents a message with the
        text opcode.

        If there is no last message frame, the return value is
        undefined.
    */
    bool
    got_text() const
    {
        return ! got_binary();
    }

    /// Returns `true` if the last completed read finished the current message.
    bool
    is_message_done() const noexcept;

    /** Returns the close reason received from the remote peer.

        This is only valid after a read completes with error::closed.
    */
    close_reason const&
    reason() const noexcept;

    /** Returns a suggested maximum buffer size for the next call to read.

        This function returns a reasonable upper limit on the number
        of bytes for the size of the buffer passed in the next call
        to read. The number is determined by the state of the current
        frame and whether or not the permessage-deflate extension is
        enabled.

        @param initial_size A non-zero size representing the caller's
        desired buffer size for when there is no information which may
        be used to calculate a more specific value. For example, when
        reading the first frame header of a message.
    */
    std::size_t
    read_size_hint(
        std::size_t initial_size = +tcp_frame_size) const;

    /** Returns a suggested maximum buffer size for the next call to read.

        This function returns a reasonable upper limit on the number
        of bytes for the size of the buffer passed in the next call
        to read. The number is determined by the state of the current
        frame and whether or not the permessage-deflate extension is
        enabled.

        @param buffer The buffer which will be used for reading. The
        implementation will query the buffer to obtain the optimum
        size of a subsequent call to `buffer.prepare` based on the
        state of the current frame, if any.
    */
    template<class DynamicBuffer
#if ! BOOST_BEAST_DOXYGEN
        , class = typename std::enable_if<
            ! std::is_integral<DynamicBuffer>::value>::type
#endif
    >
    std::size_t
    read_size_hint(
        DynamicBuffer& buffer) const;

    //--------------------------------------------------------------------------
    //
    // Settings
    //
    //--------------------------------------------------------------------------

#if BOOST_BEAST_DOXYGEN
    template<class Option>
    void
    get_option(Option& opt);

    template<class Option>
    void
    set_option(Option opt);
#else

    void set_option(decorator opt);

    void get_option(timeout& opt);
    void set_option(timeout const& opt);
#endif

    /** Set the permessage-deflate extension options

        @throws invalid_argument if `deflateSupported == false`, and either
        `client_enable` or `server_enable` is `true`.
    */
    void
    set_option(permessage_deflate const& o);

    /// Get the permessage-deflate extension options
    void
    get_option(permessage_deflate& o);

    /** Set the automatic fragmentation option.

        Determines if outgoing message payloads are broken up into
        multiple pieces.

        When the automatic fragmentation size is turned on, outgoing
        message payloads are broken up into multiple frames no larger
        than the write buffer size.

        The default setting is to fragment messages.

        @param value A `bool` indicating if auto fragmentation should be on.

        @par Example
        Setting the automatic fragmentation option:
        @code
            ws.auto_fragment(true);
        @endcode
    */
    void
    auto_fragment(bool value);

    /// Returns `true` if the automatic fragmentation option is set.
    bool
    auto_fragment() const;

    /** Set the binary message write option.

        This controls whether or not outgoing message opcodes
        are set to binary or text. The setting is only applied
        at the start when a caller begins a new message. Changing
        the opcode after a message is started will only take effect
        after the current message being sent is complete.

        The default setting is to send text messages.

        @param value `true` if outgoing messages should indicate
        binary, or `false` if they should indicate text.

        @par Example
        Setting the message type to binary.
        @code
            ws.binary(true);
        @endcode
        */
    void
    binary(bool value);

    /// Returns `true` if the binary message write option is set.
    bool
    binary() const;

    /** Set a callback to be invoked on each incoming control frame.

        Sets the callback to be invoked whenever a ping, pong,
        or close control frame is received during a call to one
        of the following functions:

        @li @ref beast::websocket::stream::read
        @li @ref beast::websocket::stream::read_some
        @li @ref beast::websocket::stream::async_read
        @li @ref beast::websocket::stream::async_read_some

        Unlike completion handlers, the callback will be invoked
        for each control frame during a call to any synchronous
        or asynchronous read function. The operation is passive,
        with no associated error code, and triggered by reads.

        For close frames, the close reason code may be obtained by
        calling the function @ref reason.

        @param cb The function object to call, which must be
        invocable with this equivalent signature:
        @code
        void
        callback(
            frame_type kind,       // The type of frame
            string_view payload    // The payload in the frame
        );
        @endcode
        The implementation type-erases the callback which may require
        a dynamic allocation. To prevent the possibility of a dynamic
        allocation, use `std::ref` to wrap the callback.
        If the read operation which receives the control frame is
        an asynchronous operation, the callback will be invoked using
        the same method as that used to invoke the final handler.

        @note Incoming ping and close frames are automatically
        handled. Pings are responded to with pongs, and a close frame
        is responded to with a close frame leading to the closure of
        the stream. It is not necessary to manually send pings, pongs,
        or close frames from inside the control callback.
        Attempting to manually send a close frame from inside the
        control callback after receiving a close frame will result
        in undefined behavior.
    */
    void
    control_callback(std::function<void(frame_type, string_view)> cb);

    /** Reset the control frame callback.

        This function removes any previously set control frame callback.
    */
    void
    control_callback();

    /** Set the maximum incoming message size option.

        Sets the largest permissible incoming message size. Message
        frame fields indicating a size that would bring the total
        message size over this limit will cause a protocol failure.

        The default setting is 16 megabytes. A value of zero indicates
        a limit of the maximum value of a `std::uint64_t`.

        @par Example
        Setting the maximum read message size.
        @code
            ws.read_message_max(65536);
        @endcode

        @param amount The limit on the size of incoming messages.
    */
    void
    read_message_max(std::size_t amount);

    /// Returns the maximum incoming message size setting.
    std::size_t
    read_message_max() const;

    /** Set whether the PRNG is cryptographically secure

        This controls whether or not the source of pseudo-random
        numbers used to produce the masks required by the WebSocket
        protocol are of cryptographic quality. When the setting is
        `true`, a strong algorithm is used which cannot be guessed
        by observing outputs. When the setting is `false`, a much
        faster algorithm is used.
        Masking is only performed by streams operating in the client
        mode. For streams operating in the server mode, this setting
        has no effect.
        By default, newly constructed streams use a secure PRNG.

        If the WebSocket stream is used with an encrypted SSL or TLS
        next layer, if it is known to the application that intermediate
        proxies are not vulnerable to cache poisoning, or if the
        application is designed such that an attacker cannot send
        arbitrary inputs to the stream interface, then the faster
        algorithm may be used.

        For more information please consult the WebSocket protocol RFC.

        @param value `true` if the PRNG algorithm should be
        cryptographically secure.
    */
    void
    secure_prng(bool value);

    /** Set the write buffer size option.

        Sets the size of the write buffer used by the implementation to
        send frames. The write buffer is needed when masking payload data
        in the client role, compressing frames, or auto-fragmenting message
        data.

        Lowering the size of the buffer can decrease the memory requirements
        for each connection, while increasing the size of the buffer can reduce
        the number of calls made to the next layer to write data.

        The default setting is 4096. The minimum value is 8.

        The write buffer size can only be changed when the stream is not
        open. Undefined behavior results if the option is modified after a
        successful WebSocket handshake.

        @par Example
        Setting the write buffer size.
        @code
            ws.write_buffer_bytes(8192);
        @endcode

        @param amount The size of the write buffer in bytes.
    */
    void
    write_buffer_bytes(std::size_t amount);

    /// Returns the size of the write buffer.
    std::size_t
    write_buffer_bytes() const;

    /** Set the text message write option.

        This controls whether or not outgoing message opcodes
        are set to binary or text. The setting is only applied
        at the start when a caller begins a new message. Changing
        the opcode after a message is started will only take effect
        after the current message being sent is complete.

        The default setting is to send text messages.

        @param value `true` if outgoing messages should indicate
        text, or `false` if they should indicate binary.

        @par Example
        Setting the message type to text.
        @code
            ws.text(true);
        @endcode
    */
    void
    text(bool value);

    /// Returns `true` if the text message write option is set.
    bool
    text() const;

    /*
        timer settings

        * Timer is disabled
        * Close on timeout
            - no complete frame received, OR
            - no complete frame sent
        * Ping on timeout
            - ping on no complete frame received
                * if can't ping?
    */

    //--------------------------------------------------------------------------
    //
    // Handshaking (Client)
    //
    //--------------------------------------------------------------------------

    /** Perform the WebSocket handshake in the client role.

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.

        @throws system_error Thrown on failure.

        @par Example
        @code
        ws.handshake("localhost", "/");
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
    */
    void
    handshake(
        string_view host,
        string_view target);

    /** Perform the WebSocket handshake in the client role.

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols.

        @param res The HTTP Upgrade response returned by the remote
        endpoint. The caller may use the response to access any
        additional information sent by the server.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.

        @throws system_error Thrown on failure.

        @par Example
        @code
        response_type res;
        ws.handshake(res, "localhost", "/");
        std::cout << res;
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
    */
    void
    handshake(
        response_type& res,
        string_view host,
        string_view target);

    /** Perform the WebSocket handshake in the client role.

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.

        @param ec Set to indicate what error occurred, if any.

        @par Example
        @code
        error_code ec;
        ws.handshake("localhost", "/", ec);
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
    */
    void
    handshake(
        string_view host,
        string_view target,
        error_code& ec);

    /** Perform the WebSocket handshake in the client role.

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols.

        @param res The HTTP Upgrade response returned by the remote
        endpoint. The caller may use the response to access any
        additional information sent by the server.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.

        @param ec Set to indicate what error occurred, if any.

        @par Example
        @code
        error_code ec;
        response_type res;
        ws.handshake(res, "localhost", "/", ec);
        if(! ec)
            std::cout << res;
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
    */
    void
    handshake(
        response_type& res,
        string_view host,
        string_view target,
        error_code& ec);

    /** Perform the WebSocket handshake asynchronously in the client role.

        This initiating function is used to asynchronously begin performing the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.
        The implementation will not access the string data after the
        initiating function returns.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.
        The implementation will not access the string data after the
        initiating function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @par Example
        @code
        ws.async_handshake("localhost", "/",
            [](error_code ec)
            {
                if(ec)
                    std::cerr << "Error: " << ec.message() << "\n";
            });
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
    */
    template<class HandshakeHandler>
    BOOST_BEAST_ASYNC_RESULT1(HandshakeHandler)
    async_handshake(
        string_view host,
        string_view target,
        HandshakeHandler&& handler);

    /** Perform the WebSocket handshake asynchronously in the client role.

        This initiating function is used to asynchronously begin performing the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        The handshake is successful if the received HTTP response
        indicates the upgrade was accepted by the server, represented by a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols.

        @param res The HTTP Upgrade response returned by the remote
        endpoint. The caller may use the response to access any
        additional information sent by the server. This object will
        be assigned before the completion handler is invoked.

        @param host The name of the remote host. This is required by
        the HTTP protocol to set the "Host" header field.
        The implementation will not access the string data after the
        initiating function returns.

        @param target The request-target, in origin-form. The server may use the
        target to distinguish different services on the same listening port.
        The implementation will not access the string data after the
        initiating function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @par Example
        @code
        response_type res;
        ws.async_handshake(res, "localhost", "/",
            [&res](error_code ec)
            {
                if(ec)
                    std::cerr << "Error: " << ec.message() << "\n";
                else
                    std::cout << res;

            });
        @endcode

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
    */
    template<class HandshakeHandler>
    BOOST_BEAST_ASYNC_RESULT1(HandshakeHandler)
    async_handshake(
        response_type& res,
        string_view host,
        string_view target,
        HandshakeHandler&& handler);

    //--------------------------------------------------------------------------
    //
    // Handshaking (Server)
    //
    //--------------------------------------------------------------------------

    /** Perform the WebSocket handshake in the server role.

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols is sent to
        the peer, otherwise a non-successful error is associated with
        the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @throws system_error Thrown on failure.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
    */
    void
    accept();

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols is sent to
        the peer, otherwise a non-successful error is associated with
        the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param ec Set to indicate what error occurred, if any.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
    */
    void
    accept(error_code& ec);

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols is sent to
        the peer, otherwise a non-successful error is associated with
        the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param buffers Caller provided data that has already been
        received on the stream. The implementation will copy the
        caller provided data before the function returns.

        @throws system_error Thrown on failure.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
    */
    template<class ConstBufferSequence>
#if BOOST_BEAST_DOXYGEN
    void
#else
    typename std::enable_if<! http::detail::is_header<
        ConstBufferSequence>::value>::type
#endif
    accept(ConstBufferSequence const& buffers);

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols is sent to
        the peer, otherwise a non-successful error is associated with
        the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param buffers Caller provided data that has already been
        received on the stream. The implementation will copy the
        caller provided data before the function returns.

        @param ec Set to indicate what error occurred, if any.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
    */
    template<class ConstBufferSequence>
#if BOOST_BEAST_DOXYGEN
    void
#else
    typename std::enable_if<! http::detail::is_header<
        ConstBufferSequence>::value>::type
#endif
    accept(
        ConstBufferSequence const& buffers,
        error_code& ec);

    /** Respond to a WebSocket HTTP Upgrade request

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols is sent to
        the peer, otherwise a non-successful error is associated with
        the operation.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not
        access this object from other threads.

        @throws system_error Thrown on failure.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
    */
    template<class Body, class Allocator>
    void
    accept(http::request<Body,
        http::basic_fields<Allocator>> const& req);

    /** Respond to a WebSocket HTTP Upgrade request

        This function is used to perform the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        The call blocks until one of the following conditions is true:

        @li The response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        If a valid upgrade request is received, an HTTP response with a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols is sent to
        the peer, otherwise a non-successful error is associated with
        the operation.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not
        access this object from other threads.

        @param ec Set to indicate what error occurred, if any.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
    */
    template<class Body, class Allocator>
    void
    accept(http::request<Body,
        http::basic_fields<Allocator>> const& req,
            error_code& ec);

    /** Perform the WebSocket handshake asynchronously in the server role.

        This initiating function is used to asynchronously begin performing the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        If a valid upgrade request is received, an HTTP response with a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols is sent to
        the peer, otherwise a non-successful error is associated with
        the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::async_read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
    */
    template<class AcceptHandler>
    BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)
    async_accept(AcceptHandler&& handler);

    /** Perform the WebSocket handshake asynchronously in the server role.

        This initiating function is used to asynchronously begin performing the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        If a valid upgrade request is received, an HTTP response with a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols is sent to
        the peer, otherwise a non-successful error is associated with
        the operation.

        If the request size exceeds the capacity of the stream's
        internal buffer, the error @ref error::buffer_overflow will be
        indicated. To handle larger requests, an application should
        read the HTTP request directly using @ref http::async_read and then
        pass the request to the appropriate overload of @ref accept or
        @ref async_accept

        @param buffers Caller provided data that has already been
        received on the stream. This may be used for implementations
        allowing multiple protocols on the same stream. The
        buffered data will first be applied to the handshake, and
        then to received WebSocket frames. The implementation will
        copy the caller provided data before the function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
    */
    template<
        class ConstBufferSequence,
        class AcceptHandler>
#if BOOST_BEAST_DOXYGEN
    void_or_deduced
#else
    typename std::enable_if<
        ! http::detail::is_header<ConstBufferSequence>::value,
        BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)>::type
#endif
    async_accept(
        ConstBufferSequence const& buffers,
        AcceptHandler&& handler);

    /** Perform the WebSocket handshake asynchronously in the server role.

        This initiating function is used to asynchronously begin performing the
        <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
        required before messages can be sent and received. During the handshake,
        the client sends the Websocket Upgrade HTTP request, and the server
        replies with an HTTP response indicating the result of the handshake.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The request is received and the response is sent.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. No other operation may be performed
        on the stream until this operation completes.

        If a valid upgrade request is received, an HTTP response with a
        <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
        of @ref beast::http::status::switching_protocols is sent to
        the peer, otherwise a non-successful error is associated with
        the operation.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not access
        this object from other threads.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec    // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
    */
    template<
        class Body, class Allocator,
        class AcceptHandler>
    BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)
    async_accept(
        http::request<Body,
            http::basic_fields<Allocator>> const& req,
        AcceptHandler&& handler);

    //--------------------------------------------------------------------------
    //
    // Close Frames
    //
    //--------------------------------------------------------------------------

    /** Send a websocket close control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">close frame</a>,
        which begins the websocket closing handshake. The session ends when
        both ends of the connection have sent and received a close frame.

        The call blocks until one of the following conditions is true:

        @li The close frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        After beginning the closing handshake, the program should not write
        further message data, pings, or pongs. Instead, the program should
        continue reading message data until an error occurs. A read returning
        @ref error::closed indicates a successful connection closure.

        @param cr The reason for the close.
        If the close reason specifies a close code other than
        @ref beast::websocket::close_code::none, the close frame is
        sent with the close code and optional reason string. Otherwise,
        the close frame is sent with no payload.

        @throws system_error Thrown on failure.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket Closing Handshake (RFC6455)</a>
    */
    void
    close(close_reason const& cr);

    /** Send a websocket close control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">close frame</a>,
        which begins the websocket closing handshake. The session ends when
        both ends of the connection have sent and received a close frame.

        The call blocks until one of the following conditions is true:

        @li The close frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        After beginning the closing handshake, the program should not write
        further message data, pings, or pongs. Instead, the program should
        continue reading message data until an error occurs. A read returning
        @ref error::closed indicates a successful connection closure.

        @param cr The reason for the close.
        If the close reason specifies a close code other than
        @ref beast::websocket::close_code::none, the close frame is
        sent with the close code and optional reason string. Otherwise,
        the close frame is sent with no payload.

        @param ec Set to indicate what error occurred, if any.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket Closing Handshake (RFC6455)</a>
    */
    void
    close(close_reason const& cr, error_code& ec);

    /** Send a websocket close control frame asynchronously.

        This function is used to asynchronously send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">close frame</a>,
        which begins the websocket closing handshake. The session ends when
        both ends of the connection have sent and received a close frame.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The close frame finishes sending.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_write_some`
        function. No other operations except for message reading operations
        should be initiated on the stream after a close operation is started.

        After beginning the closing handshake, the program should not write
        further message data, pings, or pongs. Instead, the program should
        continue reading message data until an error occurs. A read returning
        @ref error::closed indicates a successful connection closure.

        @param cr The reason for the close.
        If the close reason specifies a close code other than
        @ref beast::websocket::close_code::none, the close frame is
        sent with the close code and optional reason string. Otherwise,
        the close frame is sent with no payload.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec     // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.

        @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket Closing Handshake (RFC6455)</a>
    */
    template<class CloseHandler>
    BOOST_BEAST_ASYNC_RESULT1(CloseHandler)
    async_close(close_reason const& cr, CloseHandler&& handler);

    //--------------------------------------------------------------------------
    //
    // Ping/Pong Frames
    //
    //--------------------------------------------------------------------------

    /** Send a websocket ping control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">ping frame</a>,
        which usually elicits an automatic pong control frame response from
        the peer.

        The call blocks until one of the following conditions is true:

        @li The ping frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        @param payload The payload of the ping message, which may be empty.

        @throws system_error Thrown on failure.
    */
    void
    ping(ping_data const& payload);

    /** Send a websocket ping control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">ping frame</a>,
        which usually elicits an automatic pong control frame response from
        the peer.

        The call blocks until one of the following conditions is true:

        @li The ping frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        @param payload The payload of the ping message, which may be empty.

        @param ec Set to indicate what error occurred, if any.
    */
    void
    ping(ping_data const& payload, error_code& ec);

    /** Send a websocket ping control frame asynchronously.

        This function is used to asynchronously send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">ping frame</a>,
        which usually elicits an automatic pong control frame response from
        the peer.

        @li The ping frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_write_some`
        function. The program must ensure that no other calls to @ref ping,
        @ref pong, @ref async_ping, or @ref async_pong are performed until
        this operation completes.

        If a close frame is sent or received before the ping frame is
        sent, the error received by this completion handler will be
        `net::error::operation_aborted`.

        @param payload The payload of the ping message, which may be empty.
        The implementation will not access the contents of this object after
        the initiating function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec     // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template<class WriteHandler>
    BOOST_BEAST_ASYNC_RESULT1(WriteHandler)
    async_ping(ping_data const& payload, WriteHandler&& handler);

    /** Send a websocket pong control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">pong frame</a>,
        which is usually sent automatically in response to a ping frame
        from the remote peer.

        The call blocks until one of the following conditions is true:

        @li The pong frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        WebSocket allows pong frames to be sent at any time, without first
        receiving a ping. An unsolicited pong sent in this fashion may
        indicate to the remote peer that the connection is still active.

        @param payload The payload of the pong message, which may be empty.

        @throws system_error Thrown on failure.
    */
    void
    pong(ping_data const& payload);

    /** Send a websocket pong control frame.

        This function is used to send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">pong frame</a>,
        which is usually sent automatically in response to a ping frame
        from the remote peer.

        The call blocks until one of the following conditions is true:

        @li The pong frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        WebSocket allows pong frames to be sent at any time, without first
        receiving a ping. An unsolicited pong sent in this fashion may
        indicate to the remote peer that the connection is still active.

        @param payload The payload of the pong message, which may be empty.

        @param ec Set to indicate what error occurred, if any.
    */
    void
    pong(ping_data const& payload, error_code& ec);

    /** Send a websocket pong control frame asynchronously.

        This function is used to asynchronously send a
        <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">pong frame</a>,
        which is usually sent automatically in response to a ping frame
        from the remote peer.

        @li The pong frame is written.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_write_some`
        function. The program must ensure that no other calls to @ref ping,
        @ref pong, @ref async_ping, or @ref async_pong are performed until
        this operation completes.

        If a close frame is sent or received before the pong frame is
        sent, the error received by this completion handler will be
        `net::error::operation_aborted`.

        WebSocket allows pong frames to be sent at any time, without first
        receiving a ping. An unsolicited pong sent in this fashion may
        indicate to the remote peer that the connection is still active.

        @param payload The payload of the pong message, which may be empty.
        The implementation will not access the contents of this object after
        the initiating function returns.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec     // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template<class WriteHandler>
    BOOST_BEAST_ASYNC_RESULT1(WriteHandler)
    async_pong(ping_data const& payload, WriteHandler&& handler);

    //--------------------------------------------------------------------------
    //
    // Reading
    //
    //--------------------------------------------------------------------------

    /** Read a complete message.

        This function is used to read a complete message.

        The call blocks until one of the following is true:

        @li A complete message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to append message data to.

        @throws system_error Thrown on failure.
    */
    template<class DynamicBuffer>
    std::size_t
    read(DynamicBuffer& buffer);

    /** Read a complete message.

        This function is used to read a complete message.

        The call blocks until one of the following is true:

        @li A complete message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to append message data to.

        @param ec Set to indicate what error occurred, if any.
    */
    template<class DynamicBuffer>
    std::size_t
    read(DynamicBuffer& buffer, error_code& ec);

    /** Read a complete message asynchronously.

        This function is used to asynchronously read a complete message.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li A complete message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. The program must ensure that no other
        calls to @ref read, @ref read_some, @ref async_read, or @ref async_read_some
        are performed until this operation completes.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the operation completes, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        Pong frames and close frames sent by the implementation while the
        read operation is outstanding do not prevent the application from
        also writing message data, sending pings, sending pongs, or sending
        close frames.

        @param buffer A dynamic buffer to append message data to.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,       // Result of operation
            std::size_t bytes_written   // Number of bytes appended to buffer
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template<class DynamicBuffer, class ReadHandler>
    BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
    async_read(
        DynamicBuffer& buffer,
        ReadHandler&& handler);

    //--------------------------------------------------------------------------

    /** Read some message data.

        This function is used to read some message data.

        The call blocks until one of the following is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to append message data to.

        @param limit An upper limit on the number of bytes this function
        will append into the buffer. If this value is zero, then a reasonable
        size will be chosen automatically.

        @throws system_error Thrown on failure.
    */
    template<class DynamicBuffer>
    std::size_t
    read_some(
        DynamicBuffer& buffer,
        std::size_t limit);

    /** Read some message data.

        This function is used to read some message data.

        The call blocks until one of the following is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to append message data to.

        @param limit An upper limit on the number of bytes this function
        will append into the buffer. If this value is zero, then a reasonable
        size will be chosen automatically.

        @param ec Set to indicate what error occurred, if any.
    */
    template<class DynamicBuffer>
    std::size_t
    read_some(
        DynamicBuffer& buffer,
        std::size_t limit,
        error_code& ec);

    /** Read some message data asynchronously.

        This function is used to asynchronously read some message data.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. The program must ensure that no other
        calls to @ref read, @ref read_some, @ref async_read, or @ref async_read_some
        are performed until this operation completes.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the operation completes, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        Pong frames and close frames sent by the implementation while the
        read operation is outstanding do not prevent the application from
        also writing message data, sending pings, sending pongs, or sending
        close frames.

        @param buffer A dynamic buffer to append message data to.

        @param limit An upper limit on the number of bytes this function
        will append into the buffer. If this value is zero, then a reasonable
        size will be chosen automatically.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,       // Result of operation
            std::size_t bytes_written   // Number of bytes appended to buffer
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template<class DynamicBuffer, class ReadHandler>
    BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
    async_read_some(
        DynamicBuffer& buffer,
        std::size_t limit,
        ReadHandler&& handler);

    //--------------------------------------------------------------------------

    /** Read some message data.

        This function is used to read some message data.

        The call blocks until one of the following is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffers A buffer sequence to write message data into.
        The previous contents of the buffers will be overwritten, starting
        from the beginning.

        @throws system_error Thrown on failure.
    */
    template<class MutableBufferSequence>
    std::size_t
    read_some(
        MutableBufferSequence const& buffers);

    /** Read some message data.

        This function is used to read some message data.

        The call blocks until one of the following is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `read_some` and `write_some`
        functions.

        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        Until the call returns, the implementation will read incoming control
        frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket closing handshake is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffers A buffer sequence to write message data into.
        The previous contents of the buffers will be overwritten, starting
        from the beginning.

        @param ec Set to indicate what error occurred, if any.
    */
    template<class MutableBufferSequence>
    std::size_t
    read_some(
        MutableBufferSequence const& buffers,
        error_code& ec);

    /** Read some message data asynchronously.

        This function is used to asynchronously read some message data.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li Some message data is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's `async_read_some`
        and `async_write_some` functions. The program must ensure that no other
        calls to @ref read, @ref read_some, @ref async_read, or @ref async_read_some
        are performed until this operation completes.

        Received message data is appended to the buffer.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        Until the operation completes, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        Pong frames and close frames sent by the implementation while the
        read operation is outstanding do not prevent the application from
        also writing message data, sending pings, sending pongs, or sending
        close frames.

        @param buffers A buffer sequence to write message data into.
        The previous contents of the buffers will be overwritten, starting
        from the beginning.
        The implementation will make copies of this object as needed, but
        but ownership of the underlying memory is not transferred. The
        caller is responsible for ensuring that the memory locations
        pointed to by the buffer sequence remain valid until the
        completion handler is called.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,       // Result of operation
            std::size_t bytes_written   // Number of bytes written to the buffers
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template<class MutableBufferSequence, class ReadHandler>
    BOOST_BEAST_ASYNC_RESULT2(ReadHandler)
    async_read_some(
        MutableBufferSequence const& buffers,
        ReadHandler&& handler);

    //--------------------------------------------------------------------------
    //
    // Writing
    //
    //--------------------------------------------------------------------------

    /** Write a complete message.

        This function is used to write a complete message.

        The call blocks until one of the following is true:

        @li The message is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        The current setting of the @ref binary option controls
        whether the message opcode is set to text or binary. If the
        @ref auto_fragment option is set, the message will be split
        into one or more frames as necessary. The actual payload contents
        sent may be transformed as per the WebSocket protocol settings.

        @param buffers The buffers containing the message to send.

        @return The number of bytes sent from the buffers.

        @throws system_error Thrown on failure.
    */
    template<class ConstBufferSequence>
    std::size_t
    write(ConstBufferSequence const& buffers);

    /** Write a complete message.

        This function is used to write a complete message.

        The call blocks until one of the following is true:

        @li The complete message is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        The current setting of the @ref binary option controls
        whether the message opcode is set to text or binary. If the
        @ref auto_fragment option is set, the message will be split
        into one or more frames as necessary. The actual payload contents
        sent may be transformed as per the WebSocket protocol settings.

        @param buffers The buffers containing the message to send.

        @param ec Set to indicate what error occurred, if any.

        @return The number of bytes sent from the buffers.
    */
    template<class ConstBufferSequence>
    std::size_t
    write(ConstBufferSequence const& buffers, error_code& ec);

    /** Write a complete message asynchronously.

        This function is used to asynchronously write a complete message.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The complete message is written.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's
        `async_write_some` function. The program must ensure that no other
        calls to @ref write, @ref write_some, @ref async_write, or
        @ref async_write_some are performed until this operation completes.

        The current setting of the @ref binary option controls
        whether the message opcode is set to text or binary. If the
        @ref auto_fragment option is set, the message will be split
        into one or more frames as necessary. The actual payload contents
        sent may be transformed as per the WebSocket protocol settings.

        @param buffers A buffer sequence containing the entire message
        payload. The implementation will make copies of this object
        as needed, but ownership of the underlying memory is not
        transferred. The caller is responsible for ensuring that
        the memory locations pointed to by buffers remains valid
        until the completion handler is called.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,           // Result of operation
            std::size_t bytes_transferred   // Number of bytes sent from the
                                            // buffers. If an error occurred,
                                            // this will be less than the buffer_size.
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template<
        class ConstBufferSequence,
        class WriteHandler>
    BOOST_BEAST_ASYNC_RESULT2(WriteHandler)
    async_write(
        ConstBufferSequence const& buffers,
        WriteHandler&& handler);

    /** Write some message data.

        This function is used to send part of a message.

        The call blocks until one of the following is true:

        @li The message data is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        If this is the beginning of a new message, the message opcode
        will be set to text or binary based on the current setting of
        the @ref binary (or @ref text) option. The actual payload sent
        may be transformed as per the WebSocket protocol settings.

        @param fin `true` if this is the last part of the message.

        @param buffers The buffers containing the message part to send.

        @return The number of bytes sent from the buffers.

        @throws system_error Thrown on failure.
    */
    template<class ConstBufferSequence>
    std::size_t
    write_some(bool fin, ConstBufferSequence const& buffers);

    /** Write some message data.

        This function is used to send part of a message.

        The call blocks until one of the following is true:

        @li The message data is written.

        @li An error occurs.

        The algorithm, known as a <em>composed operation</em>, is implemented
        in terms of calls to the next layer's `write_some` function.

        If this is the beginning of a new message, the message opcode
        will be set to text or binary based on the current setting of
        the @ref binary (or @ref text) option. The actual payload sent
        may be transformed as per the WebSocket protocol settings.

        @param fin `true` if this is the last part of the message.

        @param buffers The buffers containing the message part to send.

        @param ec Set to indicate what error occurred, if any.

        @return The number of bytes sent from the buffers.

        @return The number of bytes consumed in the input buffers.
    */
    template<class ConstBufferSequence>
    std::size_t
    write_some(bool fin,
        ConstBufferSequence const& buffers, error_code& ec);

    /** Write some message data asynchronously.

        This function is used to asynchronously write part of a message.

        This call always returns immediately. The asynchronous operation
        will continue until one of the following conditions is true:

        @li The message data is written.

        @li An error occurs.

        The algorithm, known as a <em>composed asynchronous operation</em>,
        is implemented in terms of calls to the next layer's
        `async_write_some` function. The program must ensure that no other
        calls to @ref write, @ref write_some, @ref async_write, or
        @ref async_write_some are performed until this operation completes.

        If this is the beginning of a new message, the message opcode
        will be set to text or binary based on the current setting of
        the @ref binary (or @ref text) option. The actual payload sent
        may be transformed as per the WebSocket protocol settings.

        @param fin `true` if this is the last part of the message.

        @param buffers The buffers containing the message part to send.
        The implementation will make copies of this object
        as needed, but ownership of the underlying memory is not
        transferred. The caller is responsible for ensuring that
        the memory locations pointed to by buffers remains valid
        until the completion handler is called.

        @param handler The completion handler to invoke when the operation
        completes. The implementation takes ownership of the handler by
        performing a decay-copy. The equivalent function signature of
        the handler must be:
        @code
        void handler(
            error_code const& ec,           // Result of operation
            std::size_t bytes_transferred   // Number of bytes sent from the
                                            // buffers. If an error occurred,
                                            // this will be less than the buffer_size.
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `net::post`.
    */
    template<class ConstBufferSequence, class WriteHandler>
    BOOST_BEAST_ASYNC_RESULT2(WriteHandler)
    async_write_some(bool fin,
        ConstBufferSequence const& buffers, WriteHandler&& handler);

    //
    // Deprecated
    //

#if ! BOOST_BEAST_DOXYGEN
    template<class RequestDecorator>
    void
    handshake_ex(
        string_view host,
        string_view target,
        RequestDecorator const& decorator);

    template<class RequestDecorator>
    void
    handshake_ex(
        response_type& res,
        string_view host,
        string_view target,
        RequestDecorator const& decorator);

    template<class RequestDecorator>
    void
    handshake_ex(
        string_view host,
        string_view target,
        RequestDecorator const& decorator,
        error_code& ec);

    template<class RequestDecorator>
    void
    handshake_ex(
        response_type& res,
        string_view host,
        string_view target,
        RequestDecorator const& decorator,
        error_code& ec);

    template<class RequestDecorator, class HandshakeHandler>
    BOOST_BEAST_ASYNC_RESULT1(HandshakeHandler)
    async_handshake_ex(
        string_view host,
        string_view target,
        RequestDecorator const& decorator,
        HandshakeHandler&& handler);

    template<class RequestDecorator, class HandshakeHandler>
    BOOST_BEAST_ASYNC_RESULT1(HandshakeHandler)
    async_handshake_ex(
        response_type& res,
        string_view host,
        string_view target,
        RequestDecorator const& decorator,
        HandshakeHandler&& handler);

    template<class ResponseDecorator>
    void
    accept_ex(ResponseDecorator const& decorator);

    template<class ResponseDecorator>
    void
    accept_ex(
        ResponseDecorator const& decorator,
        error_code& ec);

    template<class ConstBufferSequence,
        class ResponseDecorator>
    typename std::enable_if<! http::detail::is_header<
        ConstBufferSequence>::value>::type
    accept_ex(
        ConstBufferSequence const& buffers,
        ResponseDecorator const& decorator);

    template<class ConstBufferSequence, class ResponseDecorator>
    typename std::enable_if<! http::detail::is_header<
        ConstBufferSequence>::value>::type
    accept_ex(
        ConstBufferSequence const& buffers,
        ResponseDecorator const& decorator,
        error_code& ec);

    template<class Body, class Allocator,
        class ResponseDecorator>
    void
    accept_ex(http::request<Body,
        http::basic_fields<Allocator>> const& req,
            ResponseDecorator const& decorator);

    template<class Body, class Allocator,
        class ResponseDecorator>
    void
    accept_ex(http::request<Body,
        http::basic_fields<Allocator>> const& req,
            ResponseDecorator const& decorator,
                error_code& ec);

    template<
        class ResponseDecorator,
        class AcceptHandler>
    BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)
    async_accept_ex(
        ResponseDecorator const& decorator,
        AcceptHandler&& handler);

    template<
        class ConstBufferSequence,
        class ResponseDecorator,
        class AcceptHandler>
    typename std::enable_if<
        ! http::detail::is_header<ConstBufferSequence>::value,
        BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)>::type
    async_accept_ex(
        ConstBufferSequence const& buffers,
        ResponseDecorator const& decorator,
        AcceptHandler&& handler);

    template<
        class Body, class Allocator,
        class ResponseDecorator,
        class AcceptHandler>
    BOOST_BEAST_ASYNC_RESULT1(AcceptHandler)
    async_accept_ex(
        http::request<Body,
            http::basic_fields<Allocator>> const& req,
        ResponseDecorator const& decorator,
        AcceptHandler&& handler);
#endif

private:
    template<class, class>  class accept_op;
    template<class>         class close_op;
    template<class>         class handshake_op;
    template<class>         class ping_op;
    template<class>         class idle_ping_op;
    template<class, class>  class read_some_op;
    template<class, class>  class read_op;
    template<class>         class response_op;
    template<class, class>  class write_some_op;
    template<class, class>  class write_op;

    struct run_accept_op;
    struct run_close_op;
    struct run_handshake_op;
    struct run_ping_op;
    struct run_idle_ping_op;
    struct run_read_some_op;
    struct run_read_op;
    struct run_response_op;
    struct run_write_some_op;
    struct run_write_op;

    static void default_decorate_req(request_type&) {}
    static void default_decorate_res(response_type&) {}

    //
    // accept / handshake
    //

    template<class Buffers, class Decorator>
    void
    do_accept(
        Buffers const& buffers,
        Decorator const& decorator,
        error_code& ec);

    template<
        class Body, class Allocator,
        class Decorator>
    void
    do_accept(
        http::request<Body,
            http::basic_fields<Allocator>> const& req,
        Decorator const& decorator,
        error_code& ec);

    template<class RequestDecorator>
    void
    do_handshake(response_type* res_p,
        string_view host, string_view target,
            RequestDecorator const& decorator,
                error_code& ec);

    //
    // fail
    //

    void
    do_fail(
        std::uint16_t code,
        error_code ev,
        error_code& ec);
};

/** Manually provide a one-time seed to initialize the PRNG

    This function invokes the specified seed sequence to produce a seed
    suitable for use with the pseudo-random number generator used to
    create masks and perform WebSocket protocol handshakes.

    If a seed is not manually provided, the implementation will
    perform a one-time seed generation using `std::random_device`. This
    function may be used when the application runs in an environment
    where the random device is unreliable or does not provide sufficient
    entropy.

    @par Preconditions

    This function may not be called after any websocket @ref stream objects
    have been constructed.

    @param ss A reference to a `std::seed_seq` which will be used to seed
    the pseudo-random number generator. The seed sequence should have at
    least 256 bits of entropy.

    @see stream::secure_prng
*/
inline
void
seed_prng(std::seed_seq& ss)
{
    detail::prng_seed(&ss);
}

} // websocket
} // beast
} // boost

#include <boost/beast/websocket/impl/stream_impl.hpp> // must be first
#include <boost/beast/websocket/impl/accept.hpp>
#include <boost/beast/websocket/impl/close.hpp>
#include <boost/beast/websocket/impl/handshake.hpp>
#include <boost/beast/websocket/impl/ping.hpp>
#include <boost/beast/websocket/impl/read.hpp>
#include <boost/beast/websocket/impl/stream.hpp>
#include <boost/beast/websocket/impl/write.hpp>

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

#ifndef BOOST_BEAST_WEBSOCKET_STREAM_HPP
#define BOOST_BEAST_WEBSOCKET_STREAM_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/websocket/error.hpp>
#include <boost/beast/websocket/option.hpp>
#include <boost/beast/websocket/role.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/beast/websocket/stream_fwd.hpp>
#include <boost/beast/websocket/detail/frame.hpp>
#include <boost/beast/websocket/detail/hybi13.hpp>
#include <boost/beast/websocket/detail/mask.hpp>
#include <boost/beast/websocket/detail/pausation.hpp>
#include <boost/beast/websocket/detail/pmd_extension.hpp>
#include <boost/beast/websocket/detail/stream_base.hpp>
#include <boost/beast/websocket/detail/utf8_checker.hpp>
#include <boost/beast/core/static_buffer.hpp>
#include <boost/beast/core/string.hpp>
#include <boost/beast/core/detail/type_traits.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/detail/type_traits.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/error.hpp>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>

namespace boost {
namespace beast {
namespace websocket {

/// The type of object holding HTTP Upgrade requests
using request_type = http::request<http::empty_body>;

/// The type of object holding HTTP Upgrade responses
using response_type = http::response<http::string_body>;

/** The type of received control frame.

    Values of this type are passed to the control frame
    callback set using @ref stream::control_callback.
*/
enum class frame_type
{
    /// A close frame was received
    close,

    /// A ping frame was received
    ping,

    /// A pong frame was received
    pong
};

namespace detail {
class frame_test;
} // detail

//--------------------------------------------------------------------

/** Provides message-oriented functionality using WebSocket.

    The @ref stream class template provides asynchronous and blocking
    message-oriented functionality necessary for clients and servers
    to utilize the WebSocket protocol.
    
    For asynchronous operations, the application must ensure
    that they are are all performed within the same implicit
    or explicit strand.

    @par Thread Safety
    @e Distinct @e objects: Safe.@n
    @e Shared @e objects: Unsafe.
    The application must also ensure that all asynchronous
    operations are performed within the same implicit or explicit strand.

    @par Example

    To use the @ref stream template with an `ip::tcp::socket`,
    you would write:

    @code
    websocket::stream<ip::tcp::socket> ws{io_context};
    @endcode
    Alternatively, you can write:
    @code
    ip::tcp::socket sock{io_context};
    websocket::stream<ip::tcp::socket&> ws{sock};
    @endcode

    @tparam NextLayer The type representing the next layer, to which
    data will be read and written during operations. For synchronous
    operations, the type must support the @b SyncStream concept.
    For asynchronous operations, the type must support the
    @b AsyncStream concept.

    @tparam deflateSupported A `bool` indicating whether or not the
    stream will be capable of negotiating the permessage-deflate websocket
    extension. Note that even if this is set to `true`, the permessage
    deflate options (set by the caller at runtime) must still have the
    feature enabled for a successful negotiation to occur.

    @note A stream object must not be moved or destroyed while there
    are pending asynchronous operations associated with it.

    @par Concepts
        @b AsyncStream,
        @b DynamicBuffer,
        @b SyncStream
*/
template<
    class NextLayer,
    bool deflateSupported>
class stream
#if ! BOOST_BEAST_DOXYGEN
    : private detail::stream_base<deflateSupported>
#endif
{
    friend class close_test;
    friend class frame_test;
    friend class ping_test;
    friend class read1_test;
    friend class read2_test;
    friend class stream_test;
    friend class write_test;
    
    /*  The read buffer has to be at least as large
        as the largest possible control frame including
        the frame header.
    */
    static std::size_t constexpr max_control_frame_size = 2 + 8 + 4 + 125;
    static std::size_t constexpr tcp_frame_size = 1536;

    using control_cb_type =
        std::function<void(frame_type, string_view)>;

    enum class status
    {
        open,
        closing,
        closed,
        failed
    };

    NextLayer               stream_;        // the wrapped stream
    close_reason            cr_;            // set from received close frame
    control_cb_type         ctrl_cb_;       // control callback

    std::size_t             rd_msg_max_     // max message size
                                = 16 * 1024 * 1024;
    std::uint64_t           rd_size_        // total size of current message so far
                                = 0;
    std::uint64_t           rd_remain_      // message frame bytes left in current frame
                                = 0;
    detail::frame_header    rd_fh_;         // current frame header
    detail::prepared_key    rd_key_;        // current stateful mask key
    detail::frame_buffer    rd_fb_;         // to write control frames (during reads)
    detail::utf8_checker    rd_utf8_;       // to validate utf8
    static_buffer<
        +tcp_frame_size>    rd_buf_;        // buffer for reads
    detail::opcode          rd_op_          // current message binary or text
                                = detail::opcode::text;
    bool                    rd_cont_        // `true` if the next frame is a continuation
                                = false;
    bool                    rd_done_        // set when a message is done
                                = true;
    bool                    rd_close_       // did we read a close frame?
                                = false;
    detail::soft_mutex      rd_block_;      // op currently reading

    role_type               role_           // server or client
                                = role_type::client;
    status                  status_
                                = status::closed;

    detail::soft_mutex      wr_block_;      // op currently writing
    bool                    wr_close_       // did we write a close frame?
                                = false;
    bool                    wr_cont_        // next write is a continuation
                                = false;
    bool                    wr_frag_        // autofrag the current message
                                = false;
    bool                    wr_frag_opt_    // autofrag option setting
                                = true;
    bool                    wr_compress_    // compress current message
                                = false;
    detail::opcode          wr_opcode_      // message type
                                = detail::opcode::text;
    std::unique_ptr<
        std::uint8_t[]>     wr_buf_;        // write buffer
    std::size_t             wr_buf_size_    // write buffer size (current message)
                                = 0;
    std::size_t             wr_buf_opt_     // write buffer size option setting
                                = 4096;
    detail::fh_buffer       wr_fb_;         // header buffer used for writes

    detail::pausation       paused_rd_;     // paused read op
    detail::pausation       paused_wr_;     // paused write op
    detail::pausation       paused_ping_;   // paused ping op
    detail::pausation       paused_close_;  // paused close op
    detail::pausation       paused_r_rd_;   // paused read op (async read)
    detail::pausation       paused_r_close_;// paused close op (async read)

public:
    /// Indicates if the permessage-deflate extension is supported
    using is_deflate_supported =
        std::integral_constant<bool, deflateSupported>;

    /// The type of the next layer.
    using next_layer_type =
        typename std::remove_reference<NextLayer>::type;

    /// The type of the lowest layer.
    using lowest_layer_type = get_lowest_layer<next_layer_type>;

    /// The type of the executor associated with the object.
    using executor_type = typename next_layer_type::executor_type;

    /** Destructor

        Destroys the stream and all associated resources.

        @note A stream object must not be destroyed while there
        are pending asynchronous operations associated with it.
    */
    ~stream() = default;

    /** Constructor

        If `NextLayer` is move constructible, this function
        will move-construct a new stream from the existing stream.

        @note The behavior of move assignment on or from streams
        with active or pending operations is undefined.
    */
    stream(stream&&) = default;

    /** Assignment

        If `NextLayer` is move assignable, this function
        will move-assign a new stream from the existing stream.

        @note The behavior of move assignment on or from streams
        with active or pending operations is undefined.
    */
    stream& operator=(stream&&) = default;

    /** Constructor

        This constructor creates a websocket stream and initializes
        the next layer object.

        @throws Any exceptions thrown by the NextLayer constructor.

        @param args The arguments to be passed to initialize the
        next layer object. The arguments are forwarded to the next
        layer's constructor.
    */
    template<class... Args>
    explicit
    stream(Args&&... args);

    //--------------------------------------------------------------------------

    /** Get the executor associated with the object.
    
        This function may be used to obtain the executor object that the
        stream uses to dispatch handlers for asynchronous operations.

        @return A copy of the executor that stream will use to dispatch handlers.
    */
    executor_type
    get_executor() noexcept
    {
        return stream_.get_executor();
    }

    /** Get a reference to the next layer

        This function returns a reference to the next layer
        in a stack of stream layers.

        @return A reference to the next layer in the stack of
        stream layers.
    */
    next_layer_type&
    next_layer()
    {
        return stream_;
    }

    /** Get a reference to the next layer

        This function returns a reference to the next layer in a
        stack of stream layers.

        @return A reference to the next layer in the stack of
        stream layers.
    */
    next_layer_type const&
    next_layer() const
    {
        return stream_;
    }

    /** Get a reference to the lowest layer

        This function returns a reference to the lowest layer
        in a stack of stream layers.

        @return A reference to the lowest layer in the stack of
        stream layers.
    */
    lowest_layer_type&
    lowest_layer()
    {
        return stream_.lowest_layer();
    }

    /** Get a reference to the lowest layer

        This function returns a reference to the lowest layer
        in a stack of stream layers.

        @return A reference to the lowest layer in the stack of
        stream layers. Ownership is not transferred to the caller.
    */
    lowest_layer_type const&
    lowest_layer() const
    {
        return stream_.lowest_layer();
    }

    //--------------------------------------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------------------------------------

    /** Returns `true` if the stream is open.

        The stream is open after a successful handshake, and when
        no error has occurred.
    */
    bool
    is_open() const
    {
        return status_ == status::open;
    }

    /** Returns `true` if the latest message data indicates binary.

        This function informs the caller of whether the last
        received message frame represents a message with the
        binary opcode.

        If there is no last message frame, the return value is
        undefined.
    */
    bool
    got_binary() const
    {
        return rd_op_ == detail::opcode::binary;
    }

    /** Returns `true` if the latest message data indicates text.

        This function informs the caller of whether the last
        received message frame represents a message with the
        text opcode.

        If there is no last message frame, the return value is
        undefined.
    */
    bool
    got_text() const
    {
        return ! got_binary();
    }

    /// Returns `true` if the last completed read finished the current message.
    bool
    is_message_done() const
    {
        return rd_done_;
    }

    /** Returns the close reason received from the peer.

        This is only valid after a read completes with error::closed.
    */
    close_reason const&
    reason() const
    {
        return cr_;
    }

    /** Returns a suggested maximum buffer size for the next call to read.

        This function returns a reasonable upper limit on the number
        of bytes for the size of the buffer passed in the next call
        to read. The number is determined by the state of the current
        frame and whether or not the permessage-deflate extension is
        enabled.

        @param initial_size A non-zero size representing the caller's
        desired buffer size for when there is no information which may
        be used to calculate a more specific value. For example, when
        reading the first frame header of a message.
    */
    std::size_t
    read_size_hint(
        std::size_t initial_size = +tcp_frame_size) const
    {
        return read_size_hint(initial_size,
            is_deflate_supported{});
    }

    /** Returns a suggested maximum buffer size for the next call to read.

        This function returns a reasonable upper limit on the number
        of bytes for the size of the buffer passed in the next call
        to read. The number is determined by the state of the current
        frame and whether or not the permessage-deflate extension is
        enabled.

        @param buffer The buffer which will be used for reading. The
        implementation will query the buffer to obtain the optimum
        size of a subsequent call to `buffer.prepare` based on the
        state of the current frame, if any.
    */
    template<class DynamicBuffer
#if ! BOOST_BEAST_DOXYGEN
        , class = typename std::enable_if<
            ! std::is_integral<DynamicBuffer>::value>::type
#endif
    >
    std::size_t
    read_size_hint(
        DynamicBuffer& buffer) const;

    //--------------------------------------------------------------------------
    //
    // Settings
    //
    //--------------------------------------------------------------------------

    /** Set the permessage-deflate extension options

        @throws invalid_argument if `deflateSupported == false`, and either
        `client_enable` or `server_enable` is `true`.
    */
    void
    set_option(permessage_deflate const& o)
    {
        set_option(o, is_deflate_supported{});
    }

    /// Get the permessage-deflate extension options
    void
    get_option(permessage_deflate& o)
    {
        get_option(o, is_deflate_supported{});
    }

    /** Set the automatic fragmentation option.

        Determines if outgoing message payloads are broken up into
        multiple pieces.

        When the automatic fragmentation size is turned on, outgoing
        message payloads are broken up into multiple frames no larger
        than the write buffer size.

        The default setting is to fragment messages.

        @param value A `bool` indicating if auto fragmentation should be on.

        @par Example
        Setting the automatic fragmentation option:
        @code
            ws.auto_fragment(true);
        @endcode
    */
    void
    auto_fragment(bool value)
    {
        wr_frag_opt_ = value;
    }

    /// Returns `true` if the automatic fragmentation option is set.
    bool
    auto_fragment() const
    {
        return wr_frag_opt_;
    }

    /** Set the binary message write option.

        This controls whether or not outgoing message opcodes
        are set to binary or text. The setting is only applied
        at the start when a caller begins a new message. Changing
        the opcode after a message is started will only take effect
        after the current message being sent is complete.

        The default setting is to send text messages.

        @param value `true` if outgoing messages should indicate
        binary, or `false` if they should indicate text.

        @par Example
        Setting the message type to binary.
        @code
            ws.binary(true);
        @endcode
        */
    void
    binary(bool value)
    {
        wr_opcode_ = value ?
            detail::opcode::binary :
            detail::opcode::text;
    }

    /// Returns `true` if the binary message write option is set.
    bool
    binary() const
    {
        return wr_opcode_ == detail::opcode::binary;
    }

    /** Set a callback to be invoked on each incoming control frame.

        Sets the callback to be invoked whenever a ping, pong,
        or close control frame is received during a call to one
        of the following functions:

        @li @ref beast::websocket::stream::read
        @li @ref beast::websocket::stream::read_some
        @li @ref beast::websocket::stream::async_read
        @li @ref beast::websocket::stream::async_read_some

        Unlike completion handlers, the callback will be invoked
        for each control frame during a call to any synchronous
        or asynchronous read function. The operation is passive,
        with no associated error code, and triggered by reads.

        For close frames, the close reason code may be obtained by
        calling the function @ref reason.

        @param cb The function object to call, which must be
        invocable with this equivalent signature:
        @code
        void
        callback(
            frame_type kind,       // The type of frame
            string_view payload    // The payload in the frame
        );
        @endcode
        The implementation type-erases the callback which may require
        a dynamic allocation. To prevent the possibility of a dynamic
        allocation, use `std::ref` to wrap the callback.
        If the read operation which receives the control frame is
        an asynchronous operation, the callback will be invoked using
        the same method as that used to invoke the final handler.

        @note Incoming ping and close frames are automatically
        handled. Pings are responded to with pongs, and a close frame
        is responded to with a close frame leading to the closure of
        the stream. It is not necessary to manually send pings, pongs,
        or close frames from inside the control callback.
        Attempting to manually send a close frame from inside the
        control callback after receiving a close frame will result
        in undefined behavior.
    */
    void
    control_callback(std::function<void(frame_type, string_view)> cb)
    {
        ctrl_cb_ = std::move(cb);
    }

    /** Reset the control frame callback.

        This function removes any previously set control frame callback.
    */
    void
    control_callback()
    {
        ctrl_cb_ = {};
    }

    /** Set the maximum incoming message size option.

        Sets the largest permissible incoming message size. Message
        frame fields indicating a size that would bring the total
        message size over this limit will cause a protocol failure.

        The default setting is 16 megabytes. A value of zero indicates
        a limit of the maximum value of a `std::uint64_t`.

        @par Example
        Setting the maximum read message size.
        @code
            ws.read_message_max(65536);
        @endcode

        @param amount The limit on the size of incoming messages.
    */
    void
    read_message_max(std::size_t amount)
    {
        rd_msg_max_ = amount;
    }

    /// Returns the maximum incoming message size setting.
    std::size_t
    read_message_max() const
    {
        return rd_msg_max_;
    }

    /** Set whether the PRNG is cryptographically secure

        This controls whether or not the source of pseudo-random
        numbers used to produce the masks required by the WebSocket
        protocol are of cryptographic quality. When the setting is
        `true`, a strong algorithm is used which cannot be guessed
        by observing outputs. When the setting is `false`, a much
        faster algorithm is used.
        Masking is only performed by streams operating in the client
        mode. For streams operating in the server mode, this setting
        has no effect.
        By default, newly constructed streams use a secure PRNG.

        If the WebSocket stream is used with an encrypted SSL or TLS
        next layer, if it is known to the application that intermediate
        proxies are not vulnerable to cache poisoning, or if the
        application is designed such that an attacker cannot send
        arbitrary inputs to the stream interface, then the faster
        algorithm may be used.

        For more information please consult the WebSocket protocol RFC.

        @param value `true` if the PRNG algorithm should be
        cryptographically secure.
    */
    void
    secure_prng(bool value)
    {
        this->secure_prng_ = value;
    }

    /** Set the write buffer size option.

        Sets the size of the write buffer used by the implementation to
        send frames. The write buffer is needed when masking payload data
        in the client role, compressing frames, or auto-fragmenting message
        data.

        Lowering the size of the buffer can decrease the memory requirements
        for each connection, while increasing the size of the buffer can reduce
        the number of calls made to the next layer to write data.

        The default setting is 4096. The minimum value is 8.

        The write buffer size can only be changed when the stream is not
        open. Undefined behavior results if the option is modified after a
        successful WebSocket handshake.

        @par Example
        Setting the write buffer size.
        @code
            ws.write_buffer_size(8192);
        @endcode

        @param amount The size of the write buffer in bytes.
    */
    void
    write_buffer_size(std::size_t amount)
    {
        if(amount < 8)
            BOOST_THROW_EXCEPTION(std::invalid_argument{
                "write buffer size underflow"});
        wr_buf_opt_ = amount;
    };

    /// Returns the size of the write buffer.
    std::size_t
    write_buffer_size() const
    {
        return wr_buf_opt_;
    }

    /** Set the text message write option.

        This controls whether or not outgoing message opcodes
        are set to binary or text. The setting is only applied
        at the start when a caller begins a new message. Changing
        the opcode after a message is started will only take effect
        after the current message being sent is complete.

        The default setting is to send text messages.

        @param value `true` if outgoing messages should indicate
        text, or `false` if they should indicate binary.

        @par Example
        Setting the message type to text.
        @code
            ws.text(true);
        @endcode
    */
    void
    text(bool value)
    {
        wr_opcode_ = value ?
            detail::opcode::text :
            detail::opcode::binary;
    }

    /// Returns `true` if the text message write option is set.
    bool
    text() const
    {
        return wr_opcode_ == detail::opcode::text;
    }

    //--------------------------------------------------------------------------
    //
    // Handshaking (Client)
    //
    //--------------------------------------------------------------------------

    /** Send an HTTP WebSocket Upgrade request and receive the response.

        This function is used to synchronously send the WebSocket
        upgrade HTTP request. The call blocks until one of the
        following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This function is implemented in terms of one or more calls to the
        next layer's `read_some` and `write_some` functions.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param host The name of the remote host,
        required by the HTTP protocol.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol.

        @throws system_error Thrown on failure.

        @par Example
        @code
        websocket::stream<ip::tcp::socket> ws{io_context};
        ...
        try
        {
            ws.handshake("localhost", "/");
        }
        catch(...)
        {
            // An error occurred.
        }
        @endcode
    */
    void
    handshake(
        string_view host,
        string_view target);

    /** Send an HTTP WebSocket Upgrade request and receive the response.

        This function is used to synchronously send the WebSocket
        upgrade HTTP request. The call blocks until one of the
        following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This function is implemented in terms of one or more calls to the
        next layer's `read_some` and `write_some` functions.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param res The HTTP Upgrade response returned by the remote
        endpoint.

        @param host The name of the remote host,
        required by the HTTP protocol.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol.

        @throws system_error Thrown on failure.

        @par Example
        @code
        websocket::stream<ip::tcp::socket> ws{io_context};
        ...
        try
        {
            response_type res;
            ws.handshake(res, "localhost", "/");
        }
        catch(...)
        {
            // An error occurred.
        }
        @endcode
    */
    void
    handshake(
        response_type& res,
        string_view host,
        string_view target);

    /** Send an HTTP WebSocket Upgrade request and receive the response.

        This function is used to synchronously send the WebSocket
        upgrade HTTP request. The call blocks until one of the
        following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This function is implemented in terms of one or more calls to the
        next layer's `read_some` and `write_some` functions.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param host The name of the remote host,
        required by the HTTP protocol.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol.

        @param decorator A function object which will be called to modify
        the HTTP request object generated by the implementation. This
        could be used to set the User-Agent field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            request_type& req
        ); @endcode

        @throws system_error Thrown on failure.

        @par Example
        @code
        websocket::stream<ip::tcp::socket> ws{io_context};
        ...
        try
        {
            ws.handshake("localhost", "/",
                [](request_type& req)
                {
                    req.set(field::user_agent, "Beast");
                });
        }
        catch(...)
        {
            // An error occurred.
        }
        @endcode
    */
    template<class RequestDecorator>
    void
    handshake_ex(
        string_view host,
        string_view target,
        RequestDecorator const& decorator);

    /** Send an HTTP WebSocket Upgrade request and receive the response.

        This function is used to synchronously send the WebSocket
        upgrade HTTP request. The call blocks until one of the
        following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This function is implemented in terms of one or more calls to the
        next layer's `read_some` and `write_some` functions.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param res The HTTP Upgrade response returned by the remote
        endpoint.

        @param host The name of the remote host,
        required by the HTTP protocol.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol.

        @param decorator A function object which will be called to modify
        the HTTP request object generated by the implementation. This
        could be used to set the User-Agent field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            request_type& req
        ); @endcode

        @throws system_error Thrown on failure.

        @par Example
        @code
        websocket::stream<ip::tcp::socket> ws{io_context};
        ...
        try
        {
            response_type res;
            ws.handshake(res, "localhost", "/",
                [](request_type& req)
                {
                    req.set(field::user_agent, "Beast");
                });
        }
        catch(...)
        {
            // An error occurred.
        }
        @endcode
    */
    template<class RequestDecorator>
    void
    handshake_ex(
        response_type& res,
        string_view host,
        string_view target,
        RequestDecorator const& decorator);

    /** Send an HTTP WebSocket Upgrade request and receive the response.

        This function is used to synchronously send the WebSocket
        upgrade HTTP request. The call blocks until one of the
        following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This function is implemented in terms of one or more calls to the
        next layer's `read_some` and `write_some` functions.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param host The name of the remote host,
        required by the HTTP protocol.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol.

        @param ec Set to indicate what error occurred, if any.

        @par Example
        @code
        websocket::stream<ip::tcp::socket> ws{io_context};
        ...
        error_code ec;
        ws.handshake(host, target, ec);
        if(ec)
        {
            // An error occurred.
        }
        @endcode
    */
    void
    handshake(
        string_view host,
        string_view target,
        error_code& ec);

    /** Send an HTTP WebSocket Upgrade request and receive the response.

        This function is used to synchronously send the WebSocket
        upgrade HTTP request. The call blocks until one of the
        following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This function is implemented in terms of one or more calls to the
        next layer's `read_some` and `write_some` functions.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param res The HTTP Upgrade response returned by the remote
        endpoint. If `ec` is set, the returned value is undefined.

        @param host The name of the remote host,
        required by the HTTP protocol.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol.

        @param ec Set to indicate what error occurred, if any.

        @par Example
        @code
        websocket::stream<ip::tcp::socket> ws{io_context};
        ...
        error_code ec;
        response_type res;
        ws.handshake(res, host, target, ec);
        if(ec)
        {
            // An error occurred.
        }
        @endcode
    */
    void
    handshake(
        response_type& res,
        string_view host,
        string_view target,
        error_code& ec);

    /** Send an HTTP WebSocket Upgrade request and receive the response.

        This function is used to synchronously send the WebSocket
        upgrade HTTP request. The call blocks until one of the
        following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This function is implemented in terms of one or more calls to the
        next layer's `read_some` and `write_some` functions.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param host The name of the remote host,
        required by the HTTP protocol.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol.

        @param decorator A function object which will be called to modify
        the HTTP request object generated by the implementation. This
        could be used to set the User-Agent field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            request_type& req
        ); @endcode

        @param ec Set to indicate what error occurred, if any.

        @par Example
        @code
        websocket::stream<ip::tcp::socket> ws{io_context};
        ...
        error_code ec;
        ws.handshake("localhost", "/",
            [](request_type& req)
            {
                req.set(field::user_agent, "Beast");
            },
            ec);
        if(ec)
        {
            // An error occurred.
        }
        @endcode
    */
    template<class RequestDecorator>
    void
    handshake_ex(
        string_view host,
        string_view target,
        RequestDecorator const& decorator,
        error_code& ec);

    /** Send an HTTP WebSocket Upgrade request and receive the response.

        This function is used to synchronously send the WebSocket
        upgrade HTTP request. The call blocks until one of the
        following conditions is true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This function is implemented in terms of one or more calls to the
        next layer's `read_some` and `write_some` functions.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param res The HTTP Upgrade response returned by the remote
        endpoint.

        @param host The name of the remote host,
        required by the HTTP protocol.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol.

        @param decorator A function object which will be called to modify
        the HTTP request object generated by the implementation. This
        could be used to set the User-Agent field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            request_type& req
        ); @endcode

        @param ec Set to indicate what error occurred, if any.

        @par Example
        @code
        websocket::stream<ip::tcp::socket> ws{io_context};
        ...
        error_code ec;
        response_type res;
        ws.handshake(res, "localhost", "/",
            [](request_type& req)
            {
                req.set(field::user_agent, "Beast");
            },
            ec);
        if(ec)
        {
            // An error occurred.
        }
        @endcode
    */
    template<class RequestDecorator>
    void
    handshake_ex(
        response_type& res,
        string_view host,
        string_view target,
        RequestDecorator const& decorator,
        error_code& ec);

    /** Start an asynchronous operation to send an upgrade request and receive the response.

        This function is used to asynchronously send the HTTP WebSocket
        upgrade request and receive the HTTP WebSocket Upgrade response.
        This function call always returns immediately. The asynchronous
        operation will continue until one of the following conditions is
        true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This operation is implemented in terms of one or more calls to the
        next layer's `async_read_some` and `async_write_some` functions, and
        is known as a <em>composed operation</em>. The program must ensure
        that the stream performs no other operations until this operation
        completes.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param host The name of the remote host, required by
        the HTTP protocol. Copies may be made as needed.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol. Copies of this parameter may
        be made as needed.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec    // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class HandshakeHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        HandshakeHandler, void(error_code))
    async_handshake(
        string_view host,
        string_view target,
        HandshakeHandler&& handler);

    /** Start an asynchronous operation to send an upgrade request and receive the response.

        This function is used to asynchronously send the HTTP WebSocket
        upgrade request and receive the HTTP WebSocket Upgrade response.
        This function call always returns immediately. The asynchronous
        operation will continue until one of the following conditions is
        true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This operation is implemented in terms of one or more calls to the
        next layer's `async_read_some` and `async_write_some` functions, and
        is known as a <em>composed operation</em>. The program must ensure
        that the stream performs no other operations until this operation
        completes.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param res The HTTP Upgrade response returned by the remote
        endpoint. The caller must ensure this object is valid for at
        least until the completion handler is invoked.

        @param host The name of the remote host, required by
        the HTTP protocol. Copies may be made as needed.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol. Copies of this parameter may
        be made as needed.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec     // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class HandshakeHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        HandshakeHandler, void(error_code))
    async_handshake(
        response_type& res,
        string_view host,
        string_view target,
        HandshakeHandler&& handler);

    /** Start an asynchronous operation to send an upgrade request and receive the response.

        This function is used to asynchronously send the HTTP WebSocket
        upgrade request and receive the HTTP WebSocket Upgrade response.
        This function call always returns immediately. The asynchronous
        operation will continue until one of the following conditions is
        true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This operation is implemented in terms of one or more calls to the
        next layer's `async_read_some` and `async_write_some` functions, and
        is known as a <em>composed operation</em>. The program must ensure
        that the stream performs no other operations until this operation
        completes.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param host The name of the remote host, required by
        the HTTP protocol. Copies may be made as needed.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol. Copies of this parameter may
        be made as needed.

        @param decorator A function object which will be called to modify
        the HTTP request object generated by the implementation. This
        could be used to set the User-Agent field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            request_type& req
        ); @endcode

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec     // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class RequestDecorator, class HandshakeHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        HandshakeHandler, void(error_code))
    async_handshake_ex(
        string_view host,
        string_view target,
        RequestDecorator const& decorator,
        HandshakeHandler&& handler);

    /** Start an asynchronous operation to send an upgrade request and receive the response.

        This function is used to asynchronously send the HTTP WebSocket
        upgrade request and receive the HTTP WebSocket Upgrade response.
        This function call always returns immediately. The asynchronous
        operation will continue until one of the following conditions is
        true:

        @li The request is sent and the response is received.

        @li An error occurs on the stream

        This operation is implemented in terms of one or more calls to the
        next layer's `async_read_some` and `async_write_some` functions, and
        is known as a <em>composed operation</em>. The program must ensure
        that the stream performs no other operations until this operation
        completes.

        The operation is successful if the received HTTP response indicates
        a successful HTTP Upgrade (represented by a Status-Code of 101,
        "switching protocols").

        @param res The HTTP Upgrade response returned by the remote
        endpoint. The caller must ensure this object is valid for at
        least until the completion handler is invoked.

        @param host The name of the remote host, required by
        the HTTP protocol. Copies may be made as needed.

        @param target The Request Target, which may not be empty,
        required by the HTTP protocol. Copies of this parameter may
        be made as needed.

        @param decorator A function object which will be called to modify
        the HTTP request object generated by the implementation. This
        could be used to set the User-Agent field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            request_type& req
        ); @endcode

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec     // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class RequestDecorator, class HandshakeHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        HandshakeHandler, void(error_code))
    async_handshake_ex(
        response_type& res,
        string_view host,
        string_view target,
        RequestDecorator const& decorator,
        HandshakeHandler&& handler);

    //--------------------------------------------------------------------------
    //
    // Handshaking (Server)
    //
    //--------------------------------------------------------------------------

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to synchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The call blocks
        until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @throws system_error Thrown on failure.
    */
    void
    accept();

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to synchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The call blocks
        until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param decorator A function object which will be called to modify
        the HTTP response object delivered by the implementation. This
        could be used to set the Server field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            response_type& res
        ); @endcode

        @throws system_error Thrown on failure.
    */
    template<class ResponseDecorator>
    void
    accept_ex(ResponseDecorator const& decorator);

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to synchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The call blocks
        until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param ec Set to indicate what error occurred, if any.
    */
    void
    accept(error_code& ec);

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to synchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The call blocks
        until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param decorator A function object which will be called to modify
        the HTTP response object delivered by the implementation. This
        could be used to set the Server field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            response_type& res
        ); @endcode

        @param ec Set to indicate what error occurred, if any.
    */
    template<class ResponseDecorator>
    void
    accept_ex(
        ResponseDecorator const& decorator,
        error_code& ec);

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to synchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The call blocks
        until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param buffers Caller provided data that has already been
        received on the stream. The implementation will copy the
        caller provided data before the function returns.

        @throws system_error Thrown on failure.
    */
    template<class ConstBufferSequence>
#if BOOST_BEAST_DOXYGEN
    void
#else
    typename std::enable_if<! http::detail::is_header<
        ConstBufferSequence>::value>::type
#endif
    accept(ConstBufferSequence const& buffers);

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to synchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The call blocks
        until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param buffers Caller provided data that has already been
        received on the stream. The implementation will copy the
        caller provided data before the function returns.

        @param decorator A function object which will be called to modify
        the HTTP response object delivered by the implementation. This
        could be used to set the Server field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            response_type& res
        ); @endcode

        @throws system_error Thrown on failure.
    */
    template<class ConstBufferSequence,
        class ResponseDecorator>
#if BOOST_BEAST_DOXYGEN
    void
#else
    typename std::enable_if<! http::detail::is_header<
        ConstBufferSequence>::value>::type
#endif
    accept_ex(
        ConstBufferSequence const& buffers,
        ResponseDecorator const& decorator);

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to synchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The call blocks
        until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param buffers Caller provided data that has already been
        received on the stream. The implementation will copy the
        caller provided data before the function returns.

        @param ec Set to indicate what error occurred, if any.
    */
    template<class ConstBufferSequence>
#if BOOST_BEAST_DOXYGEN
    void
#else
    typename std::enable_if<! http::detail::is_header<
        ConstBufferSequence>::value>::type
#endif
    accept(
        ConstBufferSequence const& buffers,
        error_code& ec);

    /** Read and respond to a WebSocket HTTP Upgrade request.

        This function is used to synchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The call blocks
        until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param buffers Caller provided data that has already been
        received on the stream. The implementation will copy the
        caller provided data before the function returns.

        @param decorator A function object which will be called to modify
        the HTTP response object delivered by the implementation. This
        could be used to set the Server field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            response_type& res
        ); @endcode

        @param ec Set to indicate what error occurred, if any.
    */
    template<class ConstBufferSequence, class ResponseDecorator>
#if BOOST_BEAST_DOXYGEN
    void
#else
    typename std::enable_if<! http::detail::is_header<
        ConstBufferSequence>::value>::type
#endif
    accept_ex(
        ConstBufferSequence const& buffers,
        ResponseDecorator const& decorator,
        error_code& ec);

    /** Respond to a WebSocket HTTP Upgrade request

        This function is used to synchronously send the HTTP response
        to an HTTP request possibly containing a WebSocket Upgrade.
        The call blocks until one of the following conditions is true:

        @li The response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not
        access this object from other threads.

        @throws system_error Thrown on failure.
    */
    template<class Body, class Allocator>
    void
    accept(http::request<Body,
        http::basic_fields<Allocator>> const& req);

    /** Respond to a WebSocket HTTP Upgrade request

        This function is used to synchronously send the HTTP response
        to an HTTP request possibly containing a WebSocket Upgrade.
        The call blocks until one of the following conditions is true:

        @li The response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not
        access this object from other threads.

        @param decorator A function object which will be called to modify
        the HTTP response object delivered by the implementation. This
        could be used to set the Server field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            response_type& res
        ); @endcode

        @throws system_error Thrown on failure.
    */
    template<class Body, class Allocator,
        class ResponseDecorator>
    void
    accept_ex(http::request<Body,
        http::basic_fields<Allocator>> const& req,
            ResponseDecorator const& decorator);

    /** Respond to a WebSocket HTTP Upgrade request

        This function is used to synchronously send the HTTP response
        to an HTTP request possibly containing a WebSocket Upgrade.
        The call blocks until one of the following conditions is true:

        @li The response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not
        access this object from other threads.

        @param ec Set to indicate what error occurred, if any.
    */
    template<class Body, class Allocator>
    void
    accept(http::request<Body,
        http::basic_fields<Allocator>> const& req,
            error_code& ec);

    /** Respond to a WebSocket HTTP Upgrade request

        This function is used to synchronously send the HTTP response
        to an HTTP request possibly containing a WebSocket Upgrade.
        The call blocks until one of the following conditions is true:

        @li The response finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to
        the next layer's `read_some` and `write_some` functions.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When this call returns, the stream is then ready to send and
        receive WebSocket protocol frames and messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not
        access this object from other threads.

        @param decorator A function object which will be called to modify
        the HTTP response object delivered by the implementation. This
        could be used to set the Server field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            response_type& res
        ); @endcode

        @param ec Set to indicate what error occurred, if any.
    */
    template<class Body, class Allocator,
        class ResponseDecorator>
    void
    accept_ex(http::request<Body,
        http::basic_fields<Allocator>> const& req,
            ResponseDecorator const& decorator,
                error_code& ec);

    /** Start reading and responding to a WebSocket HTTP Upgrade request.

        This function is used to asynchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The function call
        always returns immediately. The asynchronous operation will
        continue until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to
        the next layer's `async_read_some` and `async_write_some`
        functions, and is known as a <em>composed operation</em>. The
        program must ensure that the stream performs no other
        asynchronous operations until this operation completes.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When the completion handler is invoked, the stream is then
        ready to send and receive WebSocket protocol frames and
        messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure, and
        the completion handler will be invoked with a suitable error
        code set.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec    // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class AcceptHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        AcceptHandler, void(error_code))
    async_accept(AcceptHandler&& handler);

    /** Start reading and responding to a WebSocket HTTP Upgrade request.

        This function is used to asynchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The function call
        always returns immediately. The asynchronous operation will
        continue until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to
        the next layer's `async_read_some` and `async_write_some`
        functions, and is known as a <em>composed operation</em>. The
        program must ensure that the stream performs no other
        asynchronous operations until this operation completes.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When the completion handler is invoked, the stream is then
        ready to send and receive WebSocket protocol frames and
        messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure, and
        the completion handler will be invoked with a suitable error
        code set.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param decorator A function object which will be called to modify
        the HTTP response object delivered by the implementation. This
        could be used to set the Server field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            response_type& res
        ); @endcode

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec    // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<
        class ResponseDecorator,
        class AcceptHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        AcceptHandler, void(error_code))
    async_accept_ex(
        ResponseDecorator const& decorator,
        AcceptHandler&& handler);

    /** Start reading and responding to a WebSocket HTTP Upgrade request.

        This function is used to asynchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The function call
        always returns immediately. The asynchronous operation will
        continue until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to
        the next layer's `async_read_some` and `async_write_some`
        functions, and is known as a <em>composed operation</em>. The
        program must ensure that the stream performs no other
        asynchronous operations until this operation completes.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When the completion handler is invoked, the stream is then
        ready to send and receive WebSocket protocol frames and
        messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure, and
        the completion handler will be invoked with a suitable error
        code set.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param buffers Caller provided data that has already been
        received on the stream. This may be used for implementations
        allowing multiple protocols on the same stream. The
        buffered data will first be applied to the handshake, and
        then to received WebSocket frames. The implementation will
        copy the caller provided data before the function returns.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec    // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<
        class ConstBufferSequence,
        class AcceptHandler>
#if BOOST_BEAST_DOXYGEN
    void_or_deduced
#else
    typename std::enable_if<
        ! http::detail::is_header<ConstBufferSequence>::value,
        BOOST_ASIO_INITFN_RESULT_TYPE(
            AcceptHandler, void(error_code))>::type
#endif
    async_accept(
        ConstBufferSequence const& buffers,
        AcceptHandler&& handler);

    /** Start reading and responding to a WebSocket HTTP Upgrade request.

        This function is used to asynchronously read an HTTP WebSocket
        Upgrade request and send the HTTP response. The function call
        always returns immediately. The asynchronous operation will
        continue until one of the following conditions is true:

        @li The request is received and the response finishes sending.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to
        the next layer's `async_read_some` and `async_write_some`
        functions, and is known as a <em>composed operation</em>. The
        program must ensure that the stream performs no other
        asynchronous operations until this operation completes.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When the completion handler is invoked, the stream is then
        ready to send and receive WebSocket protocol frames and
        messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure, and
        the completion handler will be invoked with a suitable error
        code set.

        The implementation uses fixed size internal storage to
        receive the request. If the request is too large, the error
        @ref error::buffer_overflow will be indicated. Applications
        that wish to receive larger requests should first read the
        request using their own buffer and a suitable overload of
        @ref http::read or @ref http::async_read, then call @ref accept
        or @ref async_accept with the request.

        @param buffers Caller provided data that has already been
        received on the stream. This may be used for implementations
        allowing multiple protocols on the same stream. The
        buffered data will first be applied to the handshake, and
        then to received WebSocket frames. The implementation will
        copy the caller provided data before the function returns.

        @param decorator A function object which will be called to modify
        the HTTP response object delivered by the implementation. This
        could be used to set the Server field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            response_type& res
        ); @endcode

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec    // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<
        class ConstBufferSequence,
        class ResponseDecorator,
        class AcceptHandler>
#if BOOST_BEAST_DOXYGEN
    void_or_deduced
#else
    typename std::enable_if<
        ! http::detail::is_header<ConstBufferSequence>::value,
        BOOST_ASIO_INITFN_RESULT_TYPE(
            AcceptHandler, void(error_code))>::type
#endif
    async_accept_ex(
        ConstBufferSequence const& buffers,
        ResponseDecorator const& decorator,
        AcceptHandler&& handler);

    /** Start responding to a WebSocket HTTP Upgrade request.

        This function is used to asynchronously send the HTTP response
        to an HTTP request possibly containing a WebSocket Upgrade
        request. The function call always returns immediately. The
        asynchronous operation will continue until one of the following
        conditions is true:

        @li The response finishes sending.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to
        the next layer's `async_write_some` functions, and is known as
        a <em>composed operation</em>. The program must ensure that the
        stream performs no other operations until this operation
        completes.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When the completion handler is invoked, the stream is then
        ready to send and receive WebSocket protocol frames and
        messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure, and
        the completion handler will be invoked with a suitable error
        code set.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not access
        this object from other threads.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec    // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<
        class Body, class Allocator,
        class AcceptHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        AcceptHandler, void(error_code))
    async_accept(
        http::request<Body,
            http::basic_fields<Allocator>> const& req,
        AcceptHandler&& handler);

    /** Start responding to a WebSocket HTTP Upgrade request.

        This function is used to asynchronously send the HTTP response
        to an HTTP request possibly containing a WebSocket Upgrade
        request. The function call always returns immediately. The
        asynchronous operation will continue until one of the following
        conditions is true:

        @li The response finishes sending.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to
        the next layer's `async_write_some` functions, and is known as
        a <em>composed operation</em>. The program must ensure that the
        stream performs no other operations until this operation
        completes.

        If the stream receives a valid HTTP WebSocket Upgrade request,
        an HTTP response is sent back indicating a successful upgrade.
        When the completion handler is invoked, the stream is then
        ready to send and receive WebSocket protocol frames and
        messages.
        If the HTTP Upgrade request is invalid or cannot be satisfied,
        an HTTP response is sent indicating the reason and status code
        (typically 400, "Bad Request"). This counts as a failure, and
        the completion handler will be invoked with a suitable error
        code set.

        @param req An object containing the HTTP Upgrade request.
        Ownership is not transferred, the implementation will not access
        this object from other threads.

        @param decorator A function object which will be called to modify
        the HTTP response object delivered by the implementation. This
        could be used to set the Server field, subprotocols, or other
        application or HTTP specific fields. The object will be called
        with this equivalent signature:
        @code void decorator(
            response_type& res
        ); @endcode

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec    // Result of operation
        ); @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<
        class Body, class Allocator,
        class ResponseDecorator,
        class AcceptHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        AcceptHandler, void(error_code))
    async_accept_ex(
        http::request<Body,
            http::basic_fields<Allocator>> const& req,
        ResponseDecorator const& decorator,
        AcceptHandler&& handler);

    //--------------------------------------------------------------------------
    //
    // Control Frames
    //
    //--------------------------------------------------------------------------

    /** Send a WebSocket close frame.

        This function is used to synchronously send a close frame on
        the stream. The call blocks until one of the following is true:

        @li The close frame finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls
        to the next layer's `write_some` functions.

        If the close reason specifies a close code other than
        @ref beast::websocket::close_code::none, the close frame is
        sent with the close code and optional reason string. Otherwise,
        the close frame is sent with no payload.

        Callers should not attempt to write WebSocket data after
        initiating the close. Instead, callers should continue
        reading until an error occurs. A read returning @ref error::closed
        indicates a successful connection closure.

        @param cr The reason for the close.

        @throws system_error Thrown on failure.
    */
    void
    close(close_reason const& cr);

    /** Send a WebSocket close frame.

        This function is used to synchronously send a close frame on
        the stream. The call blocks until one of the following is true:

        @li The close frame finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls
        to the next layer's `write_some` functions.

        If the close reason specifies a close code other than
        @ref beast::websocket::close_code::none, the close frame is
        sent with the close code and optional reason string. Otherwise,
        the close frame is sent with no payload.

        Callers should not attempt to write WebSocket data after
        initiating the close. Instead, callers should continue
        reading until an error occurs. A read returning @ref error::closed
        indicates a successful connection closure.

        @param cr The reason for the close.

        @param ec Set to indicate what error occurred, if any.
    */
    void
    close(close_reason const& cr, error_code& ec);

    /** Start an asynchronous operation to send a WebSocket close frame.

        This function is used to asynchronously send a close frame on
        the stream. This function call always returns immediately. The
        asynchronous operation will continue until one of the following
        conditions is true:

        @li The close frame finishes sending.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the
        next layer's `async_write_some` functions, and is known as a
        <em>composed operation</em>. The program must ensure that the
        stream performs no other write operations (such as @ref async_ping,
        @ref async_write, @ref async_write_some, or @ref async_close)
        until this operation completes.

        If the close reason specifies a close code other than
        @ref beast::websocket::close_code::none, the close frame is
        sent with the close code and optional reason string. Otherwise,
        the close frame is sent with no payload.

        Callers should not attempt to write WebSocket data after
        initiating the close. Instead, callers should continue
        reading until an error occurs. A read returning @ref error::closed
        indicates a successful connection closure.

        @param cr The reason for the close.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The function signature of the handler must be:
        @code
        void handler(
            error_code const& ec     // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class CloseHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        CloseHandler, void(error_code))
    async_close(close_reason const& cr, CloseHandler&& handler);

    /** Send a WebSocket ping frame.

        This function is used to synchronously send a ping frame on
        the stream. The call blocks until one of the following is true:

        @li The ping frame finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to the
        next layer's `write_some` functions.

        @param payload The payload of the ping message, which may be empty.

        @throws system_error Thrown on failure.
    */
    void
    ping(ping_data const& payload);

    /** Send a WebSocket ping frame.

        This function is used to synchronously send a ping frame on
        the stream. The call blocks until one of the following is true:

        @li The ping frame finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to the
        next layer's `write_some` functions.

        @param payload The payload of the ping message, which may be empty.

        @param ec Set to indicate what error occurred, if any.
    */
    void
    ping(ping_data const& payload, error_code& ec);

    /** Start an asynchronous operation to send a WebSocket ping frame.

        This function is used to asynchronously send a ping frame to
        the stream. The function call always returns immediately. The
        asynchronous operation will continue until one of the following
        is true:

        @li The entire ping frame is sent.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the
        next layer's `async_write_some` functions, and is known as a
        <em>composed operation</em>. The program must ensure that the
        stream performs no other writes until this operation completes.

        If a close frame is sent or received before the ping frame is
        sent, the completion handler will be called with the error
        set to `boost::asio::error::operation_aborted`.

        @param payload The payload of the ping message, which may be empty.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The function signature of the handler must be:
        @code
        void handler(
            error_code const& ec     // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class WriteHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        WriteHandler, void(error_code))
    async_ping(ping_data const& payload, WriteHandler&& handler);

    /** Send a WebSocket pong frame.

        This function is used to synchronously send a pong frame on
        the stream. The call blocks until one of the following is true:

        @li The pong frame finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to the
        next layer's `write_some` functions.

        The WebSocket protocol allows pong frames to be sent from either
        end at any time. It is not necessary to first receive a ping in
        order to send a pong. The remote peer may use the receipt of a
        pong frame as an indication that the connection is not dead.

        @param payload The payload of the pong message, which may be empty.

        @throws system_error Thrown on failure.
    */
    void
    pong(ping_data const& payload);

    /** Send a WebSocket pong frame.

        This function is used to synchronously send a pong frame on
        the stream. The call blocks until one of the following is true:

        @li The pong frame finishes sending.

        @li An error occurs on the stream.

        This function is implemented in terms of one or more calls to the
        next layer's `write_some` functions.

        The WebSocket protocol allows pong frames to be sent from either
        end at any time. It is not necessary to first receive a ping in
        order to send a pong. The remote peer may use the receipt of a
        pong frame as an indication that the connection is not dead.

        @param payload The payload of the pong message, which may be empty.

        @param ec Set to indicate what error occurred, if any.
    */
    void
    pong(ping_data const& payload, error_code& ec);

    /** Start an asynchronous operation to send a WebSocket pong frame.

        This function is used to asynchronously send a pong frame to
        the stream. The function call always returns immediately. The
        asynchronous operation will continue until one of the following
        is true:

        @li The entire pong frame is sent.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the
        next layer's `async_write_some` functions, and is known as a
        <em>composed operation</em>. The program must ensure that the
        stream performs no other writes until this operation completes.

        The WebSocket protocol allows pong frames to be sent from either
        end at any time. It is not necessary to first receive a ping in
        order to send a pong. The remote peer may use the receipt of a
        pong frame as an indication that the connection is not dead.

        If a close frame is sent or received before the pong frame is
        sent, the completion handler will be called with the error
        set to `boost::asio::error::operation_aborted`.

        @param payload The payload of the pong message, which may be empty.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The function signature of the handler must be:
        @code
        void handler(
            error_code const& ec     // Result of operation
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class WriteHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        WriteHandler, void(error_code))
    async_pong(ping_data const& payload, WriteHandler&& handler);

    //--------------------------------------------------------------------------
    //
    // Reading
    //
    //--------------------------------------------------------------------------

    /** Read a message

        This function is used to synchronously read a complete
        message from the stream.
        The call blocks until one of the following is true:

        @li A complete message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the next
        layer's `read_some` and `write_some` functions.

        Received message data, if any, is appended to the input area of the
        buffer. The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        While this operation is active, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to hold the message data after any
        masking or decompression has been applied.

        @throws system_error Thrown to indicate an error. The corresponding
        error code may be retrieved from the exception object for inspection.
    */
    template<class DynamicBuffer>
    std::size_t
    read(DynamicBuffer& buffer);

    /** Read a message

        This function is used to synchronously read a complete
        message from the stream.
        The call blocks until one of the following is true:

        @li A complete message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the next
        layer's `read_some` and `write_some` functions.

        Received message data, if any, is appended to the input area of the
        buffer. The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        While this operation is active, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to hold the message data after any
        masking or decompression has been applied.

        @param ec Set to indicate what error occurred, if any.
    */
    template<class DynamicBuffer>
    std::size_t
    read(DynamicBuffer& buffer, error_code& ec);

    /** Read a message asynchronously

        This function is used to asynchronously read a complete
        message from the stream.
        The function call always returns immediately.
        The asynchronous operation will continue until one of the
        following is true:

        @li A complete message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the
        next layer's `async_read_some` and `async_write_some` functions,
        and is known as a <em>composed operation</em>. The program must
        ensure that the stream performs no other reads until this operation
        completes.

        Received message data, if any, is appended to the input area of the
        buffer. The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.

        While this operation is active, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        Because of the need to handle control frames, asynchronous read
        operations can cause writes to take place. These writes are managed
        transparently; callers can still have one active asynchronous
        read and asynchronous write operation pending simultaneously
        (a user initiated call to @ref async_close counts as a write).

        @param buffer A dynamic buffer to hold the message data after
        any masking or decompression has been applied. This object must
        remain valid until the handler is called.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code
        void handler(
            error_code const& ec,       // Result of operation
            std::size_t bytes_written   // Number of bytes appended to buffer
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class DynamicBuffer, class ReadHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        ReadHandler, void(error_code, std::size_t))
    async_read(
        DynamicBuffer& buffer,
        ReadHandler&& handler);

    //--------------------------------------------------------------------------

    /** Read part of a message

        This function is used to synchronously read some
        message data from the stream.
        The call blocks until one of the following is true:

        @li Some or all of the message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the next
        layer's `read_some` and `write_some` functions.

        Received message data, if any, is appended to the input area of the
        buffer. The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        While this operation is active, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to hold the message data after any
        masking or decompression has been applied.

        @param limit An upper limit on the number of bytes this function
        will append into the buffer. If this value is zero, then a reasonable
        size will be chosen automatically.

        @throws system_error Thrown to indicate an error. The corresponding
        error code may be retrieved from the exception object for inspection.
    */
    template<class DynamicBuffer>
    std::size_t
    read_some(
        DynamicBuffer& buffer,
        std::size_t limit);

    /** Read part of a message

        This function is used to synchronously read some
        message data from the stream.
        The call blocks until one of the following is true:

        @li Some or all of the message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the next
        layer's `read_some` and `write_some` functions.

        Received message data, if any, is appended to the input area of the
        buffer. The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        While this operation is active, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes appended to the buffer.

        @param buffer A dynamic buffer to hold the message data after any
        masking or decompression has been applied.

        @param limit An upper limit on the number of bytes this function
        will append into the buffer. If this value is zero, then a reasonable
        size will be chosen automatically.

        @param ec Set to indicate what error occurred, if any.
    */
    template<class DynamicBuffer>
    std::size_t
    read_some(
        DynamicBuffer& buffer,
        std::size_t limit,
        error_code& ec);

    /** Read part of a message asynchronously

        This function is used to asynchronously read part of a
        message from the stream.
        The function call always returns immediately.
        The asynchronous operation will continue until one of the
        following is true:

        @li Some or all of the message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the
        next layer's `async_read_some` and `async_write_some` functions,
        and is known as a <em>composed operation</em>. The program must
        ensure that the stream performs no other reads until this operation
        completes.

        Received message data, if any, is appended to the input area of the
        buffer. The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        While this operation is active, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        Because of the need to handle control frames, asynchronous read
        operations can cause writes to take place. These writes are managed
        transparently; callers can still have one active asynchronous
        read and asynchronous write operation pending simultaneously
        (a user initiated call to @ref async_close counts as a write).

        @param buffer A dynamic buffer to hold the message data after
        any masking or decompression has been applied. This object must
        remain valid until the handler is called.

        @param limit An upper limit on the number of bytes this function
        will append into the buffer. If this value is zero, then a reasonable
        size will be chosen automatically.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code
        void handler(
            error_code const& ec,       // Result of operation
            std::size_t bytes_written   // Number of bytes appended to buffer
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class DynamicBuffer, class ReadHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        ReadHandler, void(error_code, std::size_t))
    async_read_some(
        DynamicBuffer& buffer,
        std::size_t limit,
        ReadHandler&& handler);

    //--------------------------------------------------------------------------

    /** Read part of a message

        This function is used to synchronously read some
        message data from the stream.
        The call blocks until one of the following is true:

        @li Some or all of the message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the next
        layer's `read_some` and `write_some` functions.

        Received message data, if any, is written to the buffer sequence.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        While this operation is active, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes written to the
        buffer sequence.

        @param buffers A buffer sequence to hold the message data after any
        masking or decompression has been applied.

        @throws system_error Thrown to indicate an error. The corresponding
        error code may be retrieved from the exception object for inspection.
    */
    template<class MutableBufferSequence>
    std::size_t
    read_some(
        MutableBufferSequence const& buffers);

    /** Read part of a message

        This function is used to synchronously read some
        message data from the stream.
        The call blocks until one of the following is true:

        @li Some or all of the message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the next
        layer's `read_some` and `write_some` functions.

        Received message data, if any, is written to the buffer sequence.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        While this operation is active, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        @return The number of message payload bytes written to the
        buffer sequence.

        @param buffers A buffer sequence to hold the message data after any
        masking or decompression has been applied.

        @param ec Set to indicate what error occurred, if any.
    */
    template<class MutableBufferSequence>
    std::size_t
    read_some(
        MutableBufferSequence const& buffers,
        error_code& ec);

    /** Read part of a message asynchronously

        This function is used to asynchronously read part of a
        message from the stream.
        The function call always returns immediately.
        The asynchronous operation will continue until one of the
        following is true:

        @li Some or all of the message is received.

        @li A close frame is received. In this case the error indicated by
            the function will be @ref error::closed.

        @li An error occurs on the stream.

        This operation is implemented in terms of one or more calls to the
        next layer's `async_read_some` and `async_write_some` functions,
        and is known as a <em>composed operation</em>. The program must
        ensure that the stream performs no other reads until this operation
        completes.

        Received message data, if any, is written to the buffer sequence.
        The functions @ref got_binary and @ref got_text may be used
        to query the stream and determine the type of the last received message.
        The function @ref is_message_done may be called to determine if the
        message received by the last read operation is complete.

        While this operation is active, the implementation will read incoming
        control frames and handle them automatically as follows:

        @li The @ref control_callback will be invoked for each control frame.

        @li For each received ping frame, a pong frame will be
            automatically sent.

        @li If a close frame is received, the WebSocket close procedure is
            performed. In this case, when the function returns, the error
            @ref error::closed will be indicated.

        Because of the need to handle control frames, asynchronous read
        operations can cause writes to take place. These writes are managed
        transparently; callers can still have one active asynchronous
        read and asynchronous write operation pending simultaneously
        (a user initiated call to @ref async_close counts as a write).

        @param buffers The buffer sequence into which message data will
        be placed after any masking or decompresison has been applied.
        The implementation will make copies of this object as needed,
        but ownership of the underlying memory is not transferred.
        The caller is responsible for ensuring that the memory
        locations pointed to by the buffer sequence remains valid
        until the completion handler is called.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code
        void handler(
            error_code const& ec,       // Result of operation
            std::size_t bytes_written   // Number of bytes written to the buffer sequence
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<class MutableBufferSequence, class ReadHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        ReadHandler, void(error_code, std::size_t))
    async_read_some(
        MutableBufferSequence const& buffers,
        ReadHandler&& handler);

    //--------------------------------------------------------------------------
    //
    // Writing
    //
    //--------------------------------------------------------------------------

    /** Write a message to the stream.

        This function is used to synchronously write a message to
        the stream. The call blocks until one of the following conditions
        is met:

        @li The entire message is sent.

        @li An error occurs.

        This operation is implemented in terms of one or more calls to the
        next layer's `write_some` function.

        The current setting of the @ref binary option controls
        whether the message opcode is set to text or binary. If the
        @ref auto_fragment option is set, the message will be split
        into one or more frames as necessary. The actual payload contents
        sent may be transformed as per the WebSocket protocol settings.

        @param buffers The buffers containing the entire message
        payload. The implementation will make copies of this object
        as needed, but ownership of the underlying memory is not
        transferred. The caller is responsible for ensuring that
        the memory locations pointed to by buffers remains valid
        until the completion handler is called.

        @return The number of bytes written from the buffers.
        If an error occurred, this will be less than the sum
        of the buffer sizes.

        @throws system_error Thrown on failure.

        @note This function always sends an entire message. To
        send a message in fragments, use @ref write_some.
    */
    template<class ConstBufferSequence>
    std::size_t
    write(ConstBufferSequence const& buffers);

    /** Write a message to the stream.

        This function is used to synchronously write a message to
        the stream. The call blocks until one of the following conditions
        is met:

        @li The entire message is sent.

        @li An error occurs.

        This operation is implemented in terms of one or more calls to the
        next layer's `write_some` function.

        The current setting of the @ref binary option controls
        whether the message opcode is set to text or binary. If the
        @ref auto_fragment option is set, the message will be split
        into one or more frames as necessary. The actual payload contents
        sent may be transformed as per the WebSocket protocol settings.

        @param buffers The buffers containing the entire message
        payload. The implementation will make copies of this object
        as needed, but ownership of the underlying memory is not
        transferred. The caller is responsible for ensuring that
        the memory locations pointed to by buffers remains valid
        until the completion handler is called.

        @return The number of bytes written from the buffers.
        If an error occurred, this will be less than the sum
        of the buffer sizes.

        @param ec Set to indicate what error occurred, if any.

        @throws system_error Thrown on failure.

        @note This function always sends an entire message. To
        send a message in fragments, use @ref write_some.
    */
    template<class ConstBufferSequence>
    std::size_t
    write(ConstBufferSequence const& buffers, error_code& ec);

    /** Start an asynchronous operation to write a message to the stream.

        This function is used to asynchronously write a message to
        the stream. The function call always returns immediately.
        The asynchronous operation will continue until one of the
        following conditions is true:

        @li The entire message is sent.

        @li An error occurs.

        This operation is implemented in terms of one or more calls
        to the next layer's `async_write_some` functions, and is known
        as a <em>composed operation</em>. The program must ensure that
        the stream performs no other write operations (such as
        @ref async_write, @ref async_write_some, or
        @ref async_close).

        The current setting of the @ref binary option controls
        whether the message opcode is set to text or binary. If the
        @ref auto_fragment option is set, the message will be split
        into one or more frames as necessary. The actual payload contents
        sent may be transformed as per the WebSocket protocol settings.

        @param buffers The buffers containing the entire message
        payload. The implementation will make copies of this object
        as needed, but ownership of the underlying memory is not
        transferred. The caller is responsible for ensuring that
        the memory locations pointed to by buffers remains valid
        until the completion handler is called.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The function signature of the handler must be:
        @code
        void handler(
            error_code const& ec,           // Result of operation
            std::size_t bytes_transferred   // Number of bytes written from the
                                            // buffers. If an error occurred,
                                            // this will be less than the sum
                                            // of the buffer sizes.
        );
        @endcode
        Regardless of whether the asynchronous operation completes
        immediately or not, the handler will not be invoked from within
        this function. Invocation of the handler will be performed in a
        manner equivalent to using `boost::asio::io_context::post`.
    */
    template<
        class ConstBufferSequence,
        class WriteHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        WriteHandler, void(error_code, std::size_t))
    async_write(
        ConstBufferSequence const& buffers,
        WriteHandler&& handler);

    /** Write partial message data on the stream.

        This function is used to write some or all of a message's
        payload to the stream. The call will block until one of the
        following conditions is true:

        @li A frame is sent.

        @li Message data is transferred to the write buffer.

        @li An error occurs.

        This operation is implemented in terms of one or more calls
        to the stream's `write_some` function.

        If this is the beginning of a new message, the message opcode
        will be set to text or binary as per the current setting of
        the @ref binary option. The actual payload sent may be
        transformed as per the WebSocket protocol settings.

        @param fin `true` if this is the last part of the message.

        @param buffers The input buffer sequence holding the data to write.

        @return The number of bytes written from the buffers.
        If an error occurred, this will be less than the sum
        of the buffer sizes.

        @throws system_error Thrown on failure.
    */
    template<class ConstBufferSequence>
    std::size_t
    write_some(bool fin, ConstBufferSequence const& buffers);

    /** Write partial message data on the stream.

        This function is used to write some or all of a message's
        payload to the stream. The call will block until one of the
        following conditions is true:

        @li A frame is sent.

        @li Message data is transferred to the write buffer.

        @li An error occurs.

        This operation is implemented in terms of one or more calls
        to the stream's `write_some` function.

        If this is the beginning of a new message, the message opcode
        will be set to text or binary as per the current setting of
        the @ref binary option. The actual payload sent may be
        transformed as per the WebSocket protocol settings.

        @param fin `true` if this is the last part of the message.

        @param buffers The input buffer sequence holding the data to write.

        @param ec Set to indicate what error occurred, if any.

        @return The number of bytes written from the buffers.
        If an error occurred, this will be less than the sum
        of the buffer sizes.

        @return The number of bytes consumed in the input buffers.
    */
    template<class ConstBufferSequence>
    std::size_t
    write_some(bool fin,
        ConstBufferSequence const& buffers, error_code& ec);

    /** Start an asynchronous operation to send a message frame on the stream.

        This function is used to asynchronously write a message frame
        on the stream. This function call always returns immediately.
        The asynchronous operation will continue until one of the following
        conditions is true:

        @li The entire frame is sent.

        @li An error occurs.

        This operation is implemented in terms of one or more calls
        to the next layer's `async_write_some` functions, and is known
        as a <em>composed operation</em>. The actual payload sent
        may be transformed as per the WebSocket protocol settings. The
        program must ensure that the stream performs no other write
        operations (such as @ref async_write, @ref async_write_some,
        or @ref async_close).

        If this is the beginning of a new message, the message opcode
        will be set to text or binary as per the current setting of
        the @ref binary option. The actual payload sent may be
        transformed as per the WebSocket protocol settings.

        @param fin `true` if this is the last part of the message.

        @param buffers A object meeting the requirements of
        ConstBufferSequence which holds the payload data before any
        masking or compression. Although the buffers object may be copied
        as necessary, ownership of the underlying buffers is retained by
        the caller, which must guarantee that they remain valid until
        the handler is called.

        @param handler Invoked when the operation completes.
        The handler may be moved or copied as needed.
        The equivalent function signature of the handler must be:
        @code void handler(
            error_code const& ec,           // Result of operation
            std::size_t bytes_transferred   // Number of bytes written from the
                                            // buffers. If an error occurred,
                                            // this will be less than the sum
                                            // of the buffer sizes.
        ); @endcode
    */
    template<class ConstBufferSequence, class WriteHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        WriteHandler, void(error_code, std::size_t))
    async_write_some(bool fin,
        ConstBufferSequence const& buffers, WriteHandler&& handler);

private:
    template<class, class>  class accept_op;
    template<class>         class close_op;
    template<class>         class handshake_op;
    template<class>         class ping_op;
    template<class, class>  class read_some_op;
    template<class, class>  class read_op;
    template<class>         class response_op;
    template<class, class>  class write_some_op;
    template<class, class>  class write_op;

    static void default_decorate_req(request_type&) {}
    static void default_decorate_res(response_type&) {}

    void
    set_option(permessage_deflate const& o, std::true_type);

    void
    set_option(permessage_deflate const&, std::false_type);

    void
    get_option(permessage_deflate& o, std::true_type)
    {
        o = this->pmd_opts_;
    }

    void
    get_option(permessage_deflate& o, std::false_type)
    {
        o = {};
        o.client_enable = false;
        o.server_enable = false;
    }

    void open(role_type role);

    void open_pmd(std::true_type);

    void open_pmd(std::false_type)
    {
    }

    void close();

    void close_pmd(std::true_type)
    {
        this->pmd_.reset();
    }

    void close_pmd(std::false_type)
    {
    }
    
    void reset();

    void begin_msg()
    {
        begin_msg(is_deflate_supported{});
    }

    void begin_msg(std::true_type);

    void begin_msg(std::false_type);

    std::size_t
    read_size_hint(
        std::size_t initial_size,
        std::true_type) const;

    std::size_t
    read_size_hint(
        std::size_t initial_size,
        std::false_type) const;

    bool
    check_open(error_code& ec)
    {
        if(status_ != status::open)
        {
            ec = boost::asio::error::operation_aborted;
            return false;
        }
        ec.assign(0, ec.category());
        return true;
    }

    bool
    check_ok(error_code& ec)
    {
        if(ec)
        {
            if(status_ != status::closed)
                status_ = status::failed;
            return false;
        }
        return true;
    }

    template<class DynamicBuffer>
    bool
    parse_fh(
        detail::frame_header& fh,
        DynamicBuffer& b,
        error_code& ec);

    template<class DynamicBuffer>
    void
    write_close(DynamicBuffer& b, close_reason const& rc);

    template<class DynamicBuffer>
    void
    write_ping(DynamicBuffer& b,
        detail::opcode op, ping_data const& data);

    //
    // upgrade
    //

    template<class Decorator>
    request_type
    build_request(detail::sec_ws_key_type& key,
        string_view host,
            string_view target,
                Decorator const& decorator);

    void
    build_request_pmd(request_type& req, std::true_type);

    void
    build_request_pmd(request_type&, std::false_type)
    {
    }

    template<
        class Body, class Allocator, class Decorator>
    response_type
    build_response(
        http::request<Body,
            http::basic_fields<Allocator>> const& req,
        Decorator const& decorator,
        error_code& ec);

    template<class Body, class Allocator>
    void
    build_response_pmd(
        response_type& res,
        http::request<Body,
            http::basic_fields<Allocator>> const& req,
        std::true_type);

    template<class Body, class Allocator>
    void
    build_response_pmd(
        response_type&,
        http::request<Body,
            http::basic_fields<Allocator>> const&,
        std::false_type)
    {
    }

    void
    on_response(
        response_type const& res,
        detail::sec_ws_key_type const& key,
        error_code& ec);

    void
    on_response_pmd(
        response_type const& res,
        std::true_type);

    void
    on_response_pmd(
        response_type const&,
        std::false_type)
    {
    }

    //
    // accept / handshake
    //

    template<class Allocator>
    void
    do_pmd_config(
        http::basic_fields<Allocator> const& h,
        std::true_type)
    {
        pmd_read(this->pmd_config_, h);
    }

    template<class Allocator>
    void
    do_pmd_config(
        http::basic_fields<Allocator> const&,
        std::false_type)
    {
    }

    template<class Decorator>
    void
    do_accept(
        Decorator const& decorator,
        error_code& ec);

    template<
        class Body, class Allocator,
        class Decorator>
    void
    do_accept(
        http::request<Body,
            http::basic_fields<Allocator>> const& req,
        Decorator const& decorator,
        error_code& ec);

    template<class RequestDecorator>
    void
    do_handshake(response_type* res_p,
        string_view host, string_view target,
            RequestDecorator const& decorator,
                error_code& ec);

    //
    // fail
    //

    void
    do_fail(
        std::uint16_t code,
        error_code ev,
        error_code& ec);
};

/** Manually provide a one-time seed to initialize the PRNG

    This function invokes the specified seed sequence to produce a seed
    suitable for use with the pseudo-random number generator used to
    create masks and perform WebSocket protocol handshakes.

    If a seed is not manually provided, the implementation will
    perform a one-time seed generation using `std::random_device`. This
    function may be used when the application runs in an environment
    where the random device is unreliable or does not provide sufficient
    entropy.

    @par Preconditions

    This function may not be called after any websocket @ref stream objects
    have been constructed.

    @param ss A reference to a `std::seed_seq` which will be used to seed
    the pseudo-random number generator. The seed sequence should have at
    least 256 bits of entropy.

    @see stream::secure_prng
*/
inline
void
seed_prng(std::seed_seq& ss)
{
    detail::stream_prng::seed(&ss);
}

} // websocket
} // beast
} // boost

#include <boost/beast/websocket/impl/accept.ipp>
#include <boost/beast/websocket/impl/close.ipp>
#include <boost/beast/websocket/impl/handshake.ipp>
#include <boost/beast/websocket/impl/ping.ipp>
#include <boost/beast/websocket/impl/read.ipp>
#include <boost/beast/websocket/impl/stream.ipp>
#include <boost/beast/websocket/impl/write.ipp>

#endif
>>>>>>> 0ab5dada0... Continuing the XRP segregation.
