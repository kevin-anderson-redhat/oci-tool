#include <OCI/Registry/Client.hpp>
#include <sstream>
constexpr std::uint16_t SSL_PORT = 443;

enum class HTTP_CODE {
  OK              = 200,
  Created         = 201,
  Accepted        = 202,
  Moved           = 301,
  Found           = 302,
  Temp_Redirect   = 307,
  Perm_Redirect   = 308,
  Bad_Request     = 400,
  Unauthorized    = 401,
  Forbidden       = 403,
  Not_Found       = 404,
  Inter_Srv_Err   = 500,
  Not_Implemented = 501,
  Bad_Gateway     = 502,
  Service_Unavail = 503,
};

auto splitLocation( std::string location ) -> std::tuple< std::string, std::string > { // this should have a third, assuming https proto across the board
  std::string domain;
  std::string uri;

  location = location.substr( location.find( '/' ) + 2 ); // strip http:// or https://
  domain   = location.substr( 0, location.find( '/' ) );
  uri      = location.substr( location.find( '/' ) );

  return { domain, uri };
}

OCI::Registry::Client::Client() : _cli( nullptr ) {}
OCI::Registry::Client::Client(  std::string const& domain ) : 
                                _cli( std::make_shared< httplib::SSLClient >( domain, SSL_PORT ) ),
                                _domain( domain ) {
  _cli->set_follow_location( true );
}

OCI::Registry::Client::Client(  std::string const& domain,
                                std::string username,
                                std::string password ) : 
                                _cli( std::make_shared< httplib::SSLClient >( domain, SSL_PORT ) ),
                                _domain( domain ),
                                _username( std::move( username ) ),
                                _password( std::move( password ) ) {
  _cli->set_follow_location( true );
}

// registry.redhat.io does not provide the scope in the Header, so have to generate it and
//   pass it in as an argument
void OCI::Registry::Client::auth( httplib::Headers const& headers, std::string const& scope ) {
  auto www_auth   = headers.find( "Www-Authenticate" );

  if ( www_auth != headers.end() ) {
    auto auth_type  = www_auth->second.substr( 0, www_auth->second.find( ' ' ) );
    auto auth_hint  = www_auth->second.substr( www_auth->second.find( ' ' ) + 1 );
    auto coma       = auth_hint.find( ',' );
    auto ncoma      = auth_hint.find( ',', coma + 1 );

    std::string location;

    //std::cout << "|" << auth_type << "|" << " |" << auth_hint << "|" << std::endl;

    auto realm      = auth_hint.substr( 0, coma );
         realm      = realm.substr( realm.find( '"' ) + 1, realm.length() - ( realm.find( '"' ) + 2 ) );
         realm      = realm.substr( realm.find( '/' ) + 2 ); // remove proto
    auto endpoint   = realm.substr( realm.find( '/' ) );
         realm      = realm.substr( 0, realm.find( '/') );
    auto service    = auth_hint.substr( coma + 1, ncoma - coma - 1 );
         service    = service.substr( service.find( '"' ) + 1, service.length() - ( service.find( '"' ) + 2 ) );

    httplib::SSLClient client( realm, SSL_PORT );

    if ( not _username.empty() and not _password.empty() ) {
      client.set_basic_auth( _username.c_str(), _password.c_str() );
    }

    location  += endpoint + "?service=" + service + "&scope=" + scope;

    auto result   = client.Get( location.c_str() );

    auto j = nlohmann::json::parse( result->body );

    if ( j.find( "token" ) == j.end() ) {
      std::cerr << "Auth Failed: " << j.dump( 2 ) << std::endl;
    } else {
      j.get_to( _ctr );
    }
  } else {
    std::cerr << "OCI::Registry::Client::auth not given header 'Www-Authenticate'" << std::endl;
  }
}

auto OCI::Registry::Client::authHeaders() -> httplib::Headers {
  if ( _ctr.token.empty() ) { // TODO: add test for if valid, based on _ctr.expires_in and _ctr.issued_at
    return {};
  } else {
    return {
      { "Authorization", "Bearer " + _ctr.token } // only return this if token is valid
    };
  }
}

void OCI::Registry::Client::fetchBlob( const std::string& rsrc, SHA256 sha, std::function< bool(const char *, uint64_t ) >& call_back ) {
  _cli->set_follow_location( false );
  auto client = _cli;

  auto location = std::string( "/v2/" + rsrc + "/blobs/" + sha );
  auto res      = client->Head( location.c_str(), authHeaders() );

  if ( HTTP_CODE( res->status ) == HTTP_CODE::Unauthorized ) {
    auth( res->headers, "repository:" + rsrc + ":pull" );

    res = client->Head( location.c_str(), authHeaders() );
  }

  if ( res->has_header( "Location" ) ) {
    std::string domain;
    
    std::tie( domain, location ) = splitLocation( res->get_header_value( "Location" ) );

    if ( domain != _domain ) {
      client = std::make_shared< httplib::SSLClient >( domain, SSL_PORT );
    }
  }

  res = client->Get( location.c_str(), authHeaders(), call_back );

  if ( not ( HTTP_CODE( res->status ) == HTTP_CODE::OK or HTTP_CODE( res->status ) == HTTP_CODE::Found ) ) {
    std::cerr << "OCI::Registry::Client::fetchBlob " << location << std::endl;
    std::cerr << "  Status: " << res->status << " Body: " << res->body << std::endl;
  }

  _cli->set_follow_location( true );
}

auto OCI::Registry::Client::hasBlob( const Schema1::ImageManifest& im, SHA256 sha ) -> bool {
  _cli->set_follow_location( false );
  auto client = _cli;

  auto location     = "/v2/" + im.name + "/blobs/" + sha;
  auto res          = _cli->Head( location.c_str(), authHeaders() );

  if ( HTTP_CODE( res->status ) == HTTP_CODE::Unauthorized ) {
    auth( res->headers, "repository:" + im.name + ":pull" ); // auth modifies the headers, so should auth return headers???

    res = client->Head( location.c_str(), authHeaders() );
  }

  if ( res->has_header( "Location" ) ) {
    std::string domain;

    std::tie( domain, location ) = splitLocation( res->get_header_value( "Location" ) );

    if ( domain != _domain ) {
      client = std::make_shared< httplib::SSLClient >( domain, SSL_PORT );
    }

    res = client->Head( location.c_str(), authHeaders() );
  }

  if ( not ( HTTP_CODE( res->status ) == HTTP_CODE::OK or HTTP_CODE( res->status ) == HTTP_CODE::Found ) ) {
    std::cerr << "OCI::Registry::Client::hasBlob " << location << std::endl;
    std::cerr << "  Status: " << res->status << " Body: " << res->body << std::endl;
    for ( auto const& header : res->headers ) {
      std::cerr << header.first << " -> " << header.second << std::endl;
    }
  }
  _cli->set_follow_location( true );

  return HTTP_CODE( res->status ) == HTTP_CODE::OK or HTTP_CODE( res->status ) == HTTP_CODE::Found;
}

auto OCI::Registry::Client::hasBlob( const Schema2::ImageManifest& im, const std::string& target, SHA256 sha ) -> bool {
  _cli->set_follow_location( false );
  auto client   = _cli;
  auto location = "/v2/" + im.name + "/blobs/" + sha;
  auto res      = client->Head( location.c_str(), authHeaders() );

  if ( HTTP_CODE( res->status ) == HTTP_CODE::Unauthorized ) {
    auth( res->headers, "repository:" + im.name + ":pull" ); // auth modifies the headers, so should auth return headers???

    res = client->Head( location.c_str(), authHeaders() );
  }

  if ( res->has_header( "Location" ) ) {
    std::string domain;

    std::tie( domain, location ) = splitLocation( res->get_header_value( "Location" ) );

    if ( domain != _domain ) {
      client = std::make_shared< httplib::SSLClient >( domain, SSL_PORT );
    }
  }

  res = client->Head( location.c_str(), authHeaders() );

  if ( not ( HTTP_CODE( res->status ) == HTTP_CODE::OK or HTTP_CODE( res->status ) == HTTP_CODE::Found ) ) {
    std::cerr << "OCI::Registry::Client::hasBlob " << location << std::endl;
    std::cerr << "  Status: " << res->status << " Body: " << res->body << std::endl;
    for ( auto const& header : res->headers ) {
      std::cerr << header.first << " -> " << header.second << std::endl;
    }
  }
  _cli->set_follow_location( true );

  return HTTP_CODE( res->status ) == HTTP_CODE::OK or HTTP_CODE( res->status ) == HTTP_CODE::Found;
}

void OCI::Registry::Client::putBlob( const Schema1::ImageManifest& im, const std::string& target, std::uintmax_t total_size, const char * blob_part, uint64_t blob_part_size ) {
  (void)im;
  (void)target;
  (void)total_size;
  (void)blob_part;
  (void)blob_part_size;

  std::cout << "OCI::Registry::Client::putBlob Schema1::ImageManifest not Not_Implemented" << std::endl;
}

void OCI::Registry::Client::putBlob( const Schema2::ImageManifest& im,
                                     const std::string& target,
                                     const SHA256& blob_sha,
                                     std::uintmax_t total_size,
                                     const char * blob_part,
                                     uint64_t blob_part_size ) {
  (void)im;
  (void)target;
  (void)blob_sha;
  (void)total_size;
  (void)blob_part;
  (void)blob_part_size;

  std::cout << "OCI::Registry::Client::putBlob Schema2::ImageManifest not Not_Implemented" << std::endl;
}

void OCI::Registry::Client::inspect( std::string const& rsrc, std::string const& target ) {
  using namespace std::string_literals;

  auto tags         = tagList( rsrc );
  auto manifestList = OCI::Manifest< Schema2::ManifestList >( this, rsrc, target );

  if ( manifestList.schemaVersion == 1 ) { // NOLINT Fall back to Schema1
      auto image_manifest = OCI::Manifest< Schema1::ImageManifest >( this, rsrc, target );
  } else if ( manifestList.schemaVersion == 2 ) { // NOLINT
    // need to do more than this, doing this instead of to_json for the moment
    // want a "combined" struct for this data and a to_json for it so the output can be "pretty printed"
    std::cout << "name: " << manifestList.name << "\n";
    std::cout << "schemaVersion: " << manifestList.schemaVersion << "\n";
    std::cout << "mediaType: " << manifestList.mediaType << "\n";
    std::cout << "tags: [" << "\n";

    for ( auto const& tag: tags.tags ) {
      std::cout << "  " << tag << "\n";
    }

    std::cout << "]" << "\n";
    std::cout << "manifests: [\n";

    for ( auto const & manifest_item: manifestList.manifests ) {
      std::cout << "  {\n";
      std::cout << "    mediaType: " << manifest_item.mediaType << "\n";
      std::cout << "    size: " << manifest_item.size << "\n";
      std::cout << "    digest: " << manifest_item.digest << "\n";
      std::cout << "    architecture: " << manifest_item.platform.architecture << "\n";
      std::cout << "    os: " << manifest_item.platform.os << "\n";

      if ( not manifest_item.platform.os_version.empty() ) {
        std::cout << "    os.version: " << manifest_item.platform.os_version << "\n";
      }

      for ( auto const & os_feature: manifest_item.platform.os_features ) {
        std::cout << "    os.feature: " << os_feature << "\n";
      }

      if ( not manifest_item.platform.variant.empty() ) {
        std::cout << "    variant: " << manifest_item.platform.variant << "\n";
      }

      for ( auto const & feature: manifest_item.platform.features ) {
        std::cout << "    feature: " << feature << "\n";
      }

      std::cout << "    ImageManifest: {\n";
      auto image_manifest = OCI::Manifest< Schema2::ImageManifest >( this, rsrc, target );
      std::cout << "      schemaVersion: " << image_manifest.schemaVersion << "\n";
      std::cout << "      mediaType: " << image_manifest.mediaType << "\n";
      std::cout << "      config: { \n";
      std::cout << "        mediaType: " << image_manifest.config.mediaType << "\n";
      std::cout << "        size: " << image_manifest.config.size << "\n";
      std::cout << "        digest: " << image_manifest.config.digest << "\n";
      std::cout << "        layers: {\n";

      for ( auto const& layer: image_manifest.layers ) {
        std::cout << "          mediaType: " << layer.mediaType << "\n";
        std::cout << "          size: " << layer.size << "\n";
        std::cout << "          digest: " << layer.digest << "\n";
        std::cout << "          urls: {\n";

        for ( auto const& url: layer.urls ) {
          std::cout << "            url: " << url << "\n";
        }

        std::cout << "          }\n";
      }

      std::cout << "        }\n";
      std::cout << "      }\n";
      std::cout << "    }\n";
      std::cout << "  },\n";
    }
    std::cout << "]" << std::endl;
  }
}

void OCI::Registry::Client::fetchManifest( Schema1::ImageManifest& im, const std::string& rsrc, const std::string& target ) {
  auto res = fetchManifest( im.mediaType, rsrc, target );

  if ( res != nullptr ) {
    if ( HTTP_CODE( res->status ) == HTTP_CODE::OK ) {
      nlohmann::json::parse( res->body ).get_to( im );

      if ( im.name.empty() ) {
        im.name = rsrc;
      }

      im.originDomain = _domain; // This is just for sync from a Registry to a Directory
    } else {
      std::cerr << "OCI::Registry::Client::fetchManifest Schema1::ImageManifest " << rsrc << " " << target << " " << res->body << std::endl;
    }
  }
}

void OCI::Registry::Client::fetchManifest( Schema1::SignedImageManifest& sim, const std::string& rsrc, const std::string& target ) {
  auto res = fetchManifest( sim.mediaType, rsrc, target );

  if ( res != nullptr ) {
    if ( HTTP_CODE( res->status ) == HTTP_CODE::OK ) {
      nlohmann::json::parse( res->body ).get_to( sim );

      if ( sim.name.empty() ) {
        sim.name = rsrc;
      }

      sim.originDomain = _domain; // This is just for sync from a Registry to a Directory
    } else {
      std::cerr << "OCI::Registry::Client::fetchManifest Schema1::SignedImageManifest " << rsrc << " " << target << " " << res->body << std::endl;
    }
  }
}

void OCI::Registry::Client::fetchManifest( Schema2::ManifestList& ml, const std::string& rsrc, const std::string& target ) {
  auto res = fetchManifest( ml.mediaType, rsrc, target );
  _requested_target = target;

  if ( res != nullptr ) {
    if ( HTTP_CODE( res->status ) == HTTP_CODE::OK ) {
      nlohmann::json::parse( res->body ).get_to( ml );

      if ( ml.name.empty() ) {
        ml.name = rsrc;
      }

      ml.originDomain    = _domain;
      ml.requestedTarget = _requested_target;
    } else {
      std::cerr << "OCI::Registry::Client::fetchManifest Schema2::ManifestList " << rsrc << " " << target << " " << res->body << std::endl;
    }
  }
}

void OCI::Registry::Client::fetchManifest( Schema2::ImageManifest& im, const std::string& rsrc, const std::string& target ) {
  auto res = fetchManifest( im.mediaType, rsrc, target );

  if ( res != nullptr ) {
    if ( HTTP_CODE( res->status ) == HTTP_CODE::OK ) {
      try {
        nlohmann::json::parse( res->body ).get_to( im );

        if ( im.name.empty() ) {
          im.name = rsrc;
        }

        im.originDomain    = _domain; // This is just for sync from a Registry to a Directory
        im.requestedTarget = _requested_target;
      } catch ( nlohmann::detail::out_of_range & err ) {
        std::cerr << "Status: " << res->status << " Body: " << res->body << std::endl;
        std::cerr << err.what() << std::endl;

        throw;
      }
    } else {
      std::cerr << "OCI::Registry::Client::fetchManifest Schema2::ImageManifest " << rsrc << " " << target << " " << res->body << std::endl;
    }
  }
}

auto OCI::Registry::Client::fetchManifest( const std::string& mediaType, const std::string& rsrc, const std::string& target ) -> std::shared_ptr< httplib::Response> {
  auto location = "/v2/" + rsrc + "/manifests/" + target;
  auto headers  = authHeaders();

   headers.emplace( "Accept", mediaType );

  auto res = _cli->Get( location.c_str(), headers );

  if ( HTTP_CODE( res->status ) == HTTP_CODE::Unauthorized ) {
    auth( res->headers, "repository:" + rsrc + ":pull" ); // auth modifies the headers, so should auth return headers???
    headers = authHeaders();
    headers.emplace( "Accept", mediaType );

    res = _cli->Get( location.c_str(), headers );
  } else if ( HTTP_CODE( res->status ) != HTTP_CODE::OK ) {
    std::cerr << "OCI::Registry::Client::fetchManifest " << location << std::endl;
    std::cerr << "  Status: " << res->status << " Body: " << res->body << std::endl;
  }

  return res;
}

void OCI::Registry::Client::putManifest( const Schema1::ImageManifest& im, const std::string& target ) {
  (void)im;
  (void)target;

  std::cout << "OCI::Registry::Client::putManifest Schema1::ImageManifest is not implemented" << std::endl;
}

void OCI::Registry::Client::putManifest( const Schema1::SignedImageManifest& sim, const std::string& target ) {
  (void)sim;
  (void)target;

  std::cout << "OCI::Registry::Client::putManifest Schema1::SignedImageManifest is not implemented" << std::endl;
}

void OCI::Registry::Client::putManifest( const Schema2::ManifestList& ml, const std::string& target ) {
  (void)ml;
  (void)target;

  std::cout << "OCI::Registry::Client::putManifest Schema2::ManifestList is not implemented" << std::endl;
}

void OCI::Registry::Client::putManifest( const Schema2::ImageManifest& im, const std::string& target ) {
  (void)im;
  (void)target;

  std::cout << "OCI::Registry::Client::putManifest Schema2::ImageManifest is not implemented" << std::endl;
}

auto OCI::Registry::Client::tagList( const std::string& rsrc ) -> OCI::Tags {
  Tags retVal;

  auto location = "/v2/" + rsrc + "/tags/list";
  auto res = _cli->Get( location.c_str(), authHeaders() );

  if ( HTTP_CODE( res->status ) == HTTP_CODE::Unauthorized ) {
    auth( res->headers, "repository:" + rsrc + ":pull" );

    res = _cli->Get( location.c_str(), authHeaders() );
  }

  if ( HTTP_CODE( res->status ) == HTTP_CODE::OK ) {
    retVal = nlohmann::json::parse( res->body ).get< Tags >();
  } else {
    std::cerr << "OCI::Registry::Client::tagList " << location << std::endl;
    std::cerr << "  Status: " << res->status << " Body: " << res->body << std::endl;
  }

  return retVal;
}

auto OCI::Registry::Client::pingResource( std::string const& rsrc ) -> bool {
  auto const location = "/v2/" + rsrc;
  auto res            = _cli->Get( location.c_str(), authHeaders() );

  if ( HTTP_CODE( res->status ) == HTTP_CODE::Unauthorized ) {
    auth( res->headers, "repository:" + rsrc + ":pull" );
    res  = _cli->Get( location.c_str(), authHeaders() );
  }

  return HTTP_CODE( res->status ) == HTTP_CODE::OK;
}


void OCI::Registry::from_json( nlohmann::json const& j, Client::TokenResponse& ctr ) {
  if ( j.find( "token" ) != j.end() ) {
    j.at( "token" ).get_to( ctr.token );
  }

  if ( ctr.token.empty() and j.find( "access_token" ) != j.end() ) {
    j.at( "access_token" ).get_to( ctr.token );
  }

  if ( j.find( "expires_in" ) != j.end() ) {
    ctr.expires_in = std::chrono::seconds( j.at( "expires_in" ).get<int>() );
  } else {
    ctr.expires_in = std::chrono::seconds( 60 ); // NOLINT default value
  }

  if ( j.find( "issued_at" ) != j.end() ) {
    std::tm tm = {};
    std::stringstream ss( j.at( "issued_at" ).get< std::string >() );
    std::get_time( &tm, "%Y-%m-%dT%H:%M:%S" ); // EXAMPLE 2020-04-20T11:52:16.177118311Z
    ctr.issued_at = std::chrono::system_clock::from_time_t( std::mktime( &tm ) );
  } else {
    ctr.issued_at = std::chrono::system_clock::now();
  }

  if ( j.find( "refresh_token" ) != j.end() ) {
    j.at( "refresh_token" ).get_to( ctr.refresh_token );
  }
}
