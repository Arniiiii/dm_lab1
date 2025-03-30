#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <locale>
#include <string>
#include <string_view>
#include <system_error>

#include "fmt/base.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include "argparse/argparse.hpp"
#include "boost/asio/buffer.hpp"
#include "boost/asio/ssl/context.hpp"
#include "boost/beast/core/multi_buffer.hpp"
#include "boost/certify/https_verification.hpp"
#include "magic_enum.hpp"
#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "simdjson.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include "boost/graph/edge_list.hpp"
#include "boost/graph/graph_utility.hpp"

#include "arby/boost_beast_related/custom_response.hpp"
#include "arby/concepts.hpp"
#include "arby/crypto_api/binance/binance.hpp"
// #include "arby/loglevel_map.hpp"
#include "arby/restapipath.hpp"
#include "arby/syncshit.hpp"
#include "arby/version.h"

namespace arby
{
  template <typename CryptoExchangeAPI> int actual_logic()
    requires cryptoexchangeapi::Con_StringView<typename CryptoExchangeAPI::string_view_type>
             && cryptoexchangeapi::Con_String<typename CryptoExchangeAPI::string_type, char>
             && cryptoexchangeapi::Con_VectorLikeT<CryptoExchangeAPI>
             && cryptoexchangeapi::Con_URLDefault<CryptoExchangeAPI>
             && cryptoexchangeapi::Con_HandleTime<CryptoExchangeAPI>
             && cryptoexchangeapi::Con_HandleExchangeInfo<CryptoExchangeAPI>
             && cryptoexchangeapi::Con_ListBadAssets<CryptoExchangeAPI,
                                                     CryptoExchangeAPI::ListBadAssets.size()>
  {
    CryptoExchangeAPI bin_api;

    quill::Logger* logger = quill::Frontend::create_or_get_logger(
        "root", quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_id_1"));

    std::string_view host = CryptoExchangeAPI::scm_getHost();  // "api.binance.com";
    uint16_t port = CryptoExchangeAPI::scm_getPort();          // 443
    try
      {
        boost::asio::io_context ioc;

        boost::asio::ssl::context ssl_ctx(boost::asio::ssl::context::tlsv12_client);

        ssl_ctx.set_verify_mode(boost::asio::ssl::context::verify_peer
                                | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        ssl_ctx.set_default_verify_paths();

        boost::certify::enable_native_https_server_verification(ssl_ctx);

        boost::asio::ip::tcp::resolver resolver(ioc);
        boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ssl_ctx);

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data()))
          {
            boost::beast::error_code errc{static_cast<int>(::ERR_get_error()),
                                          boost::asio::error::get_ssl_category()};
            throw boost::beast::system_error{errc};
          }

        // Look up the domain name
        auto const results = resolver.resolve(host, std::to_string(port));

        // Make the connection on the IP address we get from a lookup
        boost::beast::get_lowest_layer(stream).connect(results);

        // Perform the SSL handshake
        stream.handshake(boost::asio::ssl::stream_base::client);

        // This buffer is used for reading and must be persisted
        boost::beast::flat_static_buffer<1024> buffer;

        // auto res = arby::syncHttpRequest<arby::body_string<simdjson::padded_string>,
        //                                  boost::beast::http::fields>(
        //     stream, buffer, ApiCryptoExchangeAPI::scm_handleTimePath());

        // Write the message to standard out
        // std::cout << res.body() << '\n';

        // fmt::println("time: {}",
        //              bin_api.cm_handleTime(static_cast<simdjson::padded_string_view>(res.body())));

        QUILL_LOG_INFO(logger, "Downloading ExchangeInfo...");

        boost::beast::flat_buffer buffer_dynamic;
        size_t const expected_max_size = 14'000'000;
        buffer_dynamic.reserve(expected_max_size);
        auto res = arby::syncHttpRequestLargeResponse<arby::body_string<simdjson::padded_string>,
                                                      boost::beast::http::fields>(
            stream, buffer_dynamic, CryptoExchangeAPI::scm_handleExchangeInfoPath(),
            expected_max_size);

        QUILL_LOG_INFO(logger, "Finished downloading ExchangeInfo.");

        cryptoexchangeapi::ExchangeInfo<CryptoExchangeAPI::template vector_like_T> exchange_info
            = bin_api.cm_handleExchangeInfo(res.body());

        auto it = std::find_if(
            exchange_info.symbols.cbegin(), exchange_info.symbols.cend(),
            [](const cryptoexchangeapi::Symbol& symbol) { return symbol.symbol == "BTCUSDT"; });

        if (it != exchange_info.symbols.cend())
          {
            QUILL_LOG_INFO(logger, "ExchangeInfo[symbols][BTCUSDT][baseAsset]: {}",
                           (*it).base_asset);
          }

        // filter bad assets
        std::size_t num_of_erased_elements
            = std::erase_if(exchange_info.symbols, [](const cryptoexchangeapi::Symbol& symbol) {
                auto isBadAsset = [](const cryptoexchangeapi::Symbol::asset_type& asset) -> bool {
                  return std::any_of(
                      CryptoExchangeAPI::ListBadAssets.cbegin(),
                      CryptoExchangeAPI::ListBadAssets.cend(),
                      [&asset](std::string_view bad_asset) { return asset == bad_asset; });
                };
                auto isStatusTrading = [](cryptoexchangeapi::Symbol::Status_type status) -> bool {
                  return status == cryptoexchangeapi::Symbol::TRADING;
                };
                auto isTradableAtSpot
                    = [](cryptoexchangeapi::Symbol::is_spot_trading_allowed_type
                             is_spot_trading_allowed) -> bool { return is_spot_trading_allowed; };

                return isBadAsset(symbol.base_asset) || isBadAsset(symbol.quote_asset)
                       || !isStatusTrading(symbol.status)
                       || !isTradableAtSpot(symbol.is_spot_trading_allowed);
              });

        QUILL_LOG_INFO(logger, "Filtered {} symbols.", num_of_erased_elements);


        using GraphWithoutWeights_T = boost::edge_list<decltype(exchange_info.symbols.begin())>;
        
        GraphWithoutWeights_T graph_without_weights(exchange_info.symbols.begin(), exchange_info.symbols.end());
      
        boost::print_graph(graph_without_weights);
        

        // Gracefully close the stream
        boost::beast::error_code errc;
        stream.shutdown(errc);

        if ((errc.value() == boost::asio::error::eof)
            || (errc.value() == boost::asio::ssl::error::stream_errors::stream_truncated))
          {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            // Remote peer failed to send a close_notify message.
            errc = {};
          }

        if (errc)
          {
            QUILL_LOG_ERROR(logger, "Error: {}", boost::beast::system_error{errc}.what());
            return errc.value();
          }
      }
    catch (std::exception& e)
      {
        QUILL_LOG_ERROR(logger, "Got exception in main: {}", e.what());
        return 1;
      }
    return 0;
  }

}  // namespace arby
int main(int argc, char* argv[])
{
  quill::Backend::start();

  quill::Logger* logger = quill::Frontend::create_or_get_logger(
      "root", quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_id_1"));

  argparse::ArgumentParser program("DataMiningLab1", ARBY_VERSION);
  program.add_description(
      "DataMiningLab1 program is expected to do some internal arbytrage on crypto echanges like Binance.");
  program.add_argument("-E", "--exchange")
      .default_value(std::string("binance"))
      .help("Choose what crypto exchange to (ab)use")
      .choices("binance");
  program.add_argument("-V", "--version")
      .help(fmt::format("Get version. Current version: {}", ARBY_VERSION));
  try
    {
      program.parse_args(argc, argv);
    }
  catch (std::exception& e)
    {
      QUILL_LOG_ERROR(logger, "The program couldn't parse your command line arguments: {}",
                      e.what());
      fmt::println("{}", program.help().str());
      return std::make_error_code(std::errc::invalid_argument).value();
    }
  if (program.get<std::string>("exchange") == "binance")
    {
      return arby::actual_logic<arby::cryptoexchangeapi::ConImpl_BinanceAPI>();
    }
  QUILL_LOG_ERROR(logger,
                  "Nothing happens... Why? Like what rudiculious arguments you have typed?...");
  return std::make_error_code(std::errc::not_supported).value();
}
