#include "./tcp-checker.h"

// Output the instance status to stdout if debug
// Declared as inline, it will be optimized if empty
inline void DEBUG_LOG_STATUS(int color, TcpChecker *checker) {
  #ifdef DEBUG
  auto name = checker->status ? uv_err_name(checker->status) : "OK";
  cout << "\033[" << color << "mfd: " << checker->fd << ", status: " << name << "\033[0m" << endl;
  #endif
}

// Constructor
TcpChecker::TcpChecker(char *ip, unsigned int port, function<void(int status)> _callback) {

  // Many uv_* functions will return a non-zero status code, check it here and goto close
  #define CALL_UV(func, ...)                                                                                \
    status = func(__VA_ARGS__);                                                                             \
    if (status) {                                                                                           \
      close();                                                                                              \
      return;                                                                                               \
    }

  // Save _callback argument as a member property, it will be called on done
  callback = _callback;

  // Init socket with libuv
  CALL_UV(uv_tcp_init_ex, uv_default_loop(), &tcp_t, AF_INET);
  tcp_t.data = (void *)this;

  // Find socket fd and set socket options
  CALL_UV(uv_fileno, (const uv_handle_s *)&tcp_t, &fd);
  // int setsockopt(int socket, int level, int option_name,const void *option_value, socklen_t option_len);
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms740476(v=vs.85).aspx
  // 具体的 option的意思看上边的链接
  // SO_RCVTIMEO and SO_SNDTIMEO: tcp链接的超时，分别是接受和发送
  setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
  setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &send_tv, sizeof(send_tv));
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_tv, sizeof(rcv_tv));
  // The TCP_QUICKACK option is linux only

  // https://gcc.gnu.org/onlinedocs/cpp/Ifdef.html
  // This block is called a conditional group. controlled text will be included in the output 
  // of the preprocessor if and only if MACRO is defined. We say that the conditional succeeds
  // if MACRO is defined, fails if it is not.
  #ifdef TCP_QUICKACK
  // Set the TCP_QUICKACK to 0 means delay to send the last ACK in handshaking
  // So, the server-side will not changed to ESTABLISHED state
  int quick_act = 0;
  setsockopt(fd, SOL_TCP, TCP_QUICKACK, &quick_act, sizeof(quick_act));
  #endif

  // Init connection

  // http://docs.libuv.org/en/v1.x/misc.html
  // Convert a string containing an IPv4 addresses to a binary structure.
  CALL_UV(uv_ip4_addr, ip, port, &dest);
  CALL_UV(uv_tcp_connect, &connect_t, (uv_tcp_t *)&tcp_t, (const struct sockaddr *)&dest, on_connect);
  connect_t.data = (void *)this;

  DEBUG_LOG_STATUS(32, this);

  #undef CALL_UV

}

// Try to close fd and goto "done"
void TcpChecker::close() {
  DEBUG_LOG_STATUS(34, this);
  if (fd != -1) {
    uv_close((uv_handle_s *)&tcp_t, on_close);
    fd = -1;
  } else {
    done();
  }
}

// Call the callback function and delete self
// Don't do anything after deleted!
void TcpChecker::done() {
  DEBUG_LOG_STATUS(35, this);
  callback(status);
  delete this;
}

// This function will be called on ACK received
void TcpChecker::on_connect(uv_connect_t *req, int status) {
  auto that = (TcpChecker *)req->data;
  that->status = status;
  DEBUG_LOG_STATUS(33, that);
  that->close();
};

// This function will be called on socket fd closed
void TcpChecker::on_close(uv_handle_t *handle) {
  auto that = (TcpChecker *)handle->data;
  that->done();
};
