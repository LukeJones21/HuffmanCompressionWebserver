/*
 * SPDX-FileCopyrightText: 2016 Mathieu Stefani
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
   Mathieu Stefani, 07 février 2016

   Example of a REST endpoint with routing
*/

#include <algorithm>

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

#include <fstream>
#include "huffman.h"

using namespace Pistache;

void printCookies(const Http::Request& req)
{
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c : cookies)
    {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

namespace Generic
{

    void handleReady(const Rest::Request&, Http::ResponseWriter response)
    {
        response.send(Http::Code::Ok, "1");
    }

}

class StatsEndpoint
{
public:
    explicit StatsEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    void init(size_t thr = 2)
    {
        auto opts = Http::Endpoint::options()
                        .maxRequestSize(1500000).maxResponseSize(1500000)
                        .threads(static_cast<int>(10));
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start()
    {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

private:
    void setupRoutes()
    {
        using namespace Rest;
        
        Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
        Routes::Post(router, "/compress", Routes::bind(&StatsEndpoint::doCompress, this));
        Routes::Post(router, "/decompress", Routes::bind(&StatsEndpoint::doDecompress, this));
    }

    void doCompress(const Rest::Request& request, Http::ResponseWriter response) 
    {
        // std::cout << "Data to compress" << std::endl;

        // stores the inputted file as a string
        auto body = request.body();

        auto content_type_header = request.headers().tryGet<Pistache::Http::Header::ContentType>();
        std::cout << content_type_header->mime().toString() << std::endl;

        std::string output;

        std::cout << "Compressing File..." << std::endl;
        Huffman::Compress(body, output);

        std::cout << "Completed Compressing" << std::endl;
        //std::cout << body << std::endl;

        

        response.send(Http::Code::Ok, output);
    }

    void doDecompress(const Rest::Request& request, Http::ResponseWriter response) 
    {
        auto body = request.body();

        std::string output;

        std::cout << "Decompressing File..." << std::endl;
        Huffman::Decompress(body, output);

        std::cout << "Completed Decompressing" << std::endl;

        response.send(Http::Code::Ok, output);
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main(int argc, char* argv[])
{
    Port port(9070);

    int thr = 2;

    if (argc >= 2)
    {
        port = static_cast<uint16_t>(std::stol(argv[1]));

        if (argc == 3)
            thr = std::stoi(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    std::cout << "Cores = " << hardware_concurrency() << std::endl;
    std::cout << "Using " << thr << " threads" << std::endl;

    StatsEndpoint stats(addr);

    stats.init(thr);
    stats.start();
}