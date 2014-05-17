
#include <microhttpd.h>

class HTTPServer {
private:
  struct MHD_Daemon *daemon;

public:
   HTTPServer(int port, bool isThreaded=true);
   ~HTTPServer();

  int handleRequest(
    struct MHD_Connection *conn,
    const char *url, const char *method, const char *version,
    const char *upload_data, size_t *upload_data_size,
    void **ptr
  );

};
