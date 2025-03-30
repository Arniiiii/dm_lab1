#include "arby/crypto_api/binance/binance.hpp"

#include "arby/boost_beast_related/custom_response.hpp"
static_assert(boost::beast::http::is_body<arby::body_string<simdjson::padded_string>>::value, "It's not body in sense of Boost Beast's library.");
