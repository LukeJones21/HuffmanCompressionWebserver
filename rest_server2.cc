#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include "huffman.h"

std::vector<char> convert(const std::vector<bool> &data) {
  unsigned int num_bytes = (data.size() + 7) / 8;
  std::vector<char> buffer(num_bytes);
  for (unsigned int i = 0; i < num_bytes; i++) {
    char b = 0;
    for (unsigned int j = 0; j < 8; j++) {
      if (i * 8 + j < data.size()) {
        b |= (data[i * 8 + j] << (7 - j));
      }
    }
    buffer[i] = b;
  }
  return buffer;
}

std::string base64_encode(const std::vector<bool>& bool_data) {
  // Create a memory BIO
  BIO* bio = BIO_new(BIO_s_mem());

  // Set the close flag for the BIO
  BIO_set_close(bio, BIO_CLOSE);

  std::vector<char> char_data = convert(bool_data);
  // Encode the data and write it to the BIO
  BIO_write(bio, char_data.data(), char_data.size());
  BIO_flush(bio);

  // Get a pointer to the BIO's memory buffer
  char* buffer;
  int length = BIO_get_mem_data(bio, &buffer);

  // Create a string from the buffer
  std::string result(buffer, length);

  // Free the BIO object
  BIO_free_all(bio);

  return result;
}

using namespace Pistache;

class MyServer {
  public:
    MyServer(Address addr) : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    void init(int nWorkers) {
      auto opts = Http::Endpoint::options().threads(nWorkers).flags(Tcp::Options::ReuseAddr).maxRequestSize(1500000).maxResponseSize(1500000);
      httpEndpoint->init(opts);
      setupRoutes();
    }

    void start() {
      httpEndpoint->setHandler(router.handler());
      httpEndpoint->serve();
    }

  private:
    void setupRoutes() {
      using namespace Rest;

      Routes::Post(router, "/compress", Routes::bind(&MyServer::compressHandler, this));
      Routes::Post(router, "/decompress", Routes::bind(&MyServer::decompressHandler, this));
      Routes::Get(router, "/ready", Routes::bind(&MyServer::readyHandler, this));
    }

    void compressHandler(const Rest::Request& request, Http::ResponseWriter response) {

      // Set the "Access-Control-Allow-Origin" header to allow cross-origin requests from the client
      response.headers().add<Http::Header::AccessControlAllowOrigin>("*");

      // Set the "Access-Control-Allow-Methods" header to allow the POST method
      response.headers().add<Http::Header::AccessControlAllowMethods>("POST");


      auto body = request.body();

        std::vector<bool> output;

        //std::cout << body << std::endl;

        std::cout << "Compressing File..." << std::endl;
        Huffman::Compress(body, output);

        std::cout << "Completed Compressing" << std::endl;

        response.send(Http::Code::Ok, base64_encode(output));
    }

    void decompressHandler(const Rest::Request& request, Http::ResponseWriter response) {
        
      // Set the "Access-Control-Allow-Origin" header to allow cross-origin requests from the client
      response.headers().add<Http::Header::AccessControlAllowOrigin>("*");

      // Set the "Access-Control-Allow-Methods" header to allow the POST method
      response.headers().add<Http::Header::AccessControlAllowMethods>("POST");
        auto body = request.body();

        std::string output;

        std::cout << body << std::endl;

        std::cout << "Decompressing File..." << std::endl;

        //Huffman::Decompress(body, output);

        std::cout << output << std::endl;

        std::cout << "Completed Decompressing" << std::endl;

        response.send(Http::Code::Ok, output);
    }

    void readyHandler(const Rest::Request& request, Http::ResponseWriter response) {

        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");

        std::cout << "Ready" << std::endl;
        response.send(Http::Code::Ok, "Ready");
    }

    

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main() {
  MyServer server(Pistache::Address(Pistache::Ipv4::any(), Pistache::Port(9090)));
  server.init(1);
  server.start();
}
