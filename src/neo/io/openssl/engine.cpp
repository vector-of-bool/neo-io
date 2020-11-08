#include "./engine.hpp"

#include <neo/io/config.hpp>

#include <neo/as_buffer.hpp>

#if NEO_FeatureIsEnabled(neo_io, OpenSSL_Support)

#include "./error.hpp"
#include "./openssl.hpp"

using namespace neo::ssl;

#define MY_BIO_PTR (static_cast<::BIO*>(_bio_ptr))
#define MY_SSL_PTR (static_cast<::SSL*>(_ssl_ptr))

#if _WIN32
#include <windows.h>
static auto last_error() noexcept { return ::GetLastError(); }
#else
static auto last_error() noexcept { return errno; }
#endif

namespace neo::ssl::detail {

/**
 * Engine operations are written on engine_impl, so that internal methods and
 * types don't need to be visible in engine.hpp. This is a friend class of
 * engine_base.
 */
struct engine_impl {
    // The three states of the SSL engine
    enum state_t {
        /// No error occurred.
        ok,
        /// The operation is finished, and there is nothing left to do.
        stop,
    };

    /**
     * @brief Continually call fn() until SSL declares completion or an error occurs
     */
    template <typename Func>
    static void run(engine_base& eng, std::error_code& ec, Func&& fn) {
        ec = {};
        for (;;) {
            auto state = one_step(static_cast<::SSL*>(eng._ssl_ptr), ec, fn);
            if (ec) {
                break;
            }
            try {
                ec = make_error_code(flush_io(eng));
            } catch (const std::system_error& err) {
                ec = err.code();
            }
            if (state == stop || ec) {
                break;
            }
        }
    }

    /**
     * Call fn() once, and determine what we need to do next.
     */
    template <typename Func>
    static state_t one_step(::SSL* ssl, std::error_code& ec, Func&& fn) {
        // Reset the error, call the function
        ::ERR_clear_error();
        auto op_res = fn();

        // Get our errors
        auto ssl_err   = ::SSL_get_error(ssl, op_res);
        auto sys_error = static_cast<int>(::ERR_get_error());

        switch (ssl_err) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            // We have data to write, but we may not be done
            return ok;

        case SSL_ERROR_NONE:
            // We are finished with the current operation
            return stop;

        case SSL_ERROR_SYSCALL:
            ec = std::error_code(last_error(), std::system_category());
            return stop;
        case SSL_ERROR_SSL:
            ec = std::error_code(sys_error, neo::ssl::error_category());
            return stop;

        case SSL_ERROR_ZERO_RETURN:
            // Huh?
            ec = make_error_code(std::errc::io_error);
            return stop;

        default:
            // Something else?
            ec = make_error_code(std::errc::protocol_error);
            return stop;
        }
    }

    /**
     * Advance the buffer_sink and buffer_source of this engine.
     * @return std::errc A possible error
     */
    static std::errc flush_io(engine_base& eng) {
        auto bio = static_cast<::BIO*>(eng._bio_ptr);

        // Continually flush the output
        while (auto n_pending = ::BIO_ctrl_pending(bio)) {
            auto outbuf = eng.do_next_output(n_pending);
            if (!outbuf) {
                // No room for more output
                break;
            }
            auto n_out = ::BIO_read(bio, outbuf.data(), static_cast<int>(outbuf.size()));
            eng.do_commit_output((std::max)(n_out, 0));
        }

        // If we want to read, only read once
        if (SSL_want_read(static_cast<::SSL*>(eng._ssl_ptr))) {
            auto inbuf = eng.do_next_input();
            if (!inbuf) {
                // No more input.
                return std::errc::no_message;
            }
            auto n_in = ::BIO_write(bio, inbuf.data(), static_cast<int>(inbuf.size()));
            eng.do_consume_input((std::max)(n_in, 0));
        }

        return {};
    }
};

}  // namespace neo::ssl::detail

engine_base::engine_base(context& ctx_) {
    auto ctx = static_cast<::SSL_CTX*>(ctx_.c_ptr());
    auto ssl = ::SSL_new(ctx);
    if (!ssl) {
        throw_current("Failed to create a new SSL engine");
    }
    _ssl_ptr = ssl;

    ::SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
    ::SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

    ::BIO* inner_bio = nullptr;
    ::BIO* outer_bio = nullptr;
    ::BIO_new_bio_pair(&inner_bio, 0, &outer_bio, 0);
    ::SSL_set_bio(ssl, inner_bio, inner_bio);
    _bio_ptr = outer_bio;
}

void engine_base::_free() noexcept {
    if (_bio_ptr) {
        ::BIO_free(MY_BIO_PTR);
    }
    if (_ssl_ptr) {
        ::SSL_free(MY_SSL_PTR);
    }
}

void engine_base::connect(std::error_code& ec) noexcept {
    detail::engine_impl::run(*this, ec, [&] { return ::SSL_connect(MY_SSL_PTR); });
}

neo::basic_transfer_result engine_base::read_some(mutable_buffer mb) noexcept {
    std::error_code ec;
    std::size_t     total_read = 0;
    detail::engine_impl::run(*this, ec, [&] {
        auto nread = ::SSL_read(MY_SSL_PTR, mb.data(), static_cast<int>(mb.size()));
        if (nread > 0) {
            mb += nread;
            total_read += nread;
        }
        return nread;
    });
    return {total_read, ec};
}

neo::basic_transfer_result engine_base::write_some(const_buffer cb) noexcept {
    std::error_code ec;
    std::size_t     total_written = 0;
    detail::engine_impl::run(*this, ec, [&] {
        auto nwritten = ::SSL_write(MY_SSL_PTR, cb.data(), static_cast<int>(cb.size()));
        if (nwritten > 0) {
            cb += nwritten;
            total_written += nwritten;
        }
        return nwritten;
    });
    return {total_written, ec};
}

void engine_base::shutdown(std::error_code& ec) noexcept {
    detail::engine_impl::run(*this, ec, [&] { return ::SSL_shutdown(MY_SSL_PTR); });
}

bool engine_base::needs_input() const noexcept { return SSL_want_read(MY_SSL_PTR); }

#endif
