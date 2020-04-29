#include <OCI/Registry/Client.hpp>
#include <sstream>

constexpr std::uint16_t SSL_PORT    = 443;
constexpr std::uint16_t HTTP_PORT   = 80;
constexpr std::uint16_t DOCKER_PORT = 5000;

enum class HTTP_CODE {
  OK                = 200,
  Created           = 201,
  Accepted          = 202,
  No_Content        = 204,
  Moved             = 301,
  Found             = 302,
  Temp_Redirect     = 307,
  Perm_Redirect     = 308,
  Bad_Request       = 400,
  Unauthorized      = 401,
  Forbidden         = 403,
  Not_Found         = 404,
  Too_Many_Requests = 429,
  Inter_Srv_Err     = 500,
  Not_Implemented   = 501,
  Bad_Gateway       = 502,
  Service_Unavail   = 503,
};

auto splitLocation( std::string location ) -> std::tuple< std::string, std::string, std::string > {
  std::string proto;
  std::string domain;
  std::string uri;

  proto    = location.substr( 0, location.find( ':' ) ); // http or https
  location = location.substr( location.find( '/' ) + 2 ); // strip http:// or https://
  domain   = location.substr( 0, location.find( '/' ) );
  uri      = location.substr( location.find( '/' ) );

  return { proto, domain, uri };
}

OCI::Registry::Client::Client() : _cli( nullptr ), _patch_cli( nullptr ) {}
OCI::Registry::Client::Client( std::string const& location ) {
  _resource = location;

  if ( _resource.find( "//" ) != std::string::npos ) {
    _resource = _resource.substr( 2 ); // strip the // if exists
  }

  if ( _resource.find( '/' ) != std::string::npos ) {
    _domain = _resource.substr( 0, _resource.find( '/' ) );
  } else {
    _domain = _resource;
  }

  auto domain = _domain;
  
  // in uri docker will translate to https
  // if docker.io use registry-1.docker.io as the site doesn't redirect
  if ( _domain == "docker.io" ) {
    // either docker.io should work, or they should have a proper redirect implemented
    domain = "registry-1.docker.io";
  }

  _cli  = std::make_shared< httplib::SSLClient >( domain, SSL_PORT );

  if ( not ping() ) {
    _cli  = std::make_shared< httplib::Client >( domain, DOCKER_PORT );

    if ( not ping() ) {
      std::cerr << domain << " does not respond to the V2 API (secure/unsecure)" << std::endl;
    }
  }

  _patch_cli = nullptr;
  _cli->set_follow_location( true );
}

OCI::Registry::Client::Client( std::string const& location,
                               std::string username,
                               std::string password ) : Client( location ) {
  _username = std::move( username );
  _password = std::move( password );
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

auto OCI::Registry::Client::authHeaders() const -> httplib::Headers {
  httplib::Headers retVal{};

  if ( _ctr.token.empty() ) { // TODO: add test for if valid, based on _ctr.expires_in and _ctr.issued_at
  } else {
    retVal = httplib::Headers{
      { "Authorization", "Bearer " + _ctr.token } // only return this if token is valid
    };
  }

  return retVal;
}

auto OCI::Registry::Client::catalog() -> OCI::Catalog {
  Catalog retVal;

  std::cerr << "OCI::Registry::Client::catalog Not implemented" << std::endl;

  return retVal;
}

auto OCI::Registry::Client::fetchBlob( const std::string& rsrc, SHA256 sha, std::function< bool(const char *, uint64_t ) >& call_back ) -> bool {
  _cli->set_follow_location( false );
  bool retVal = true;
  auto client = _cli;

  auto location = std::string( "/v2/" + rsrc + "/blobs/" + sha );
  auto res      = client->Head( location.c_str(), authHeaders() );

  if ( HTTP_CODE( res->status ) == HTTP_CODE::Unauthorized ) {
    auth( res->headers, "repository:" + rsrc + ":pull" );

    res = client->Head( location.c_str(), authHeaders() );
  }

  if ( res->has_header( "Location" ) ) {
    std::string proto;
    std::string domain;
    
    std::tie( proto, domain, location ) = splitLocation( res->get_header_value( "Location" ) );

    if ( domain != _domain ) {
      if ( proto == "https" ) {
        client = std::make_shared< httplib::SSLClient >( domain, SSL_PORT );
      } else {
        client = std::make_shared< httplib::Client >( domain, DOCKER_PORT );
      }
    }
  }

  res = client->Get( location.c_str(), authHeaders(), call_back );

  switch( HTTP_CODE( res->status ) ) {
    case HTTP_CODE::OK:
    case HTTP_CODE::Found:
      break;
    default:
      retVal = false;
      std::cerr << "OCI::Registry::Client::fetchBlob " << location << std::endl;
      std::cerr << "  Status: " << res->status << " Body: " << res->body << std::endl;
  }

  _cli->set_follow_location( true );

  return retVal;
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
    std::string proto;
    std::string domain;

    std::tie( proto, domain, location ) = splitLocation( res->get_header_value( "Location" ) );

    if ( domain != _domain ) {
      if ( proto == "https" ) {
        client = std::make_shared< httplib::SSLClient >( domain, SSL_PORT );
      } else {
        client = std::make_shared< httplib::Client >( domain, DOCKER_PORT );
      }
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
  (void)target;

  _cli->set_follow_location( false );
  auto client   = _cli;
  auto location = "/v2/" + im.name + "/blobs/" + sha;
  auto res      = client->Head( location.c_str(), authHeaders() );

  if ( HTTP_CODE( res->status ) == HTTP_CODE::Unauthorized ) {
    auth( res->headers, "repository:" + im.name + ":pull" ); // auth modifies the headers, so should auth return headers???

    res = client->Head( location.c_str(), authHeaders() );
  }

  if ( res->has_header( "Location" ) ) {
    std::string proto;
    std::string domain;

    std::tie( proto, domain, location ) = splitLocation( res->get_header_value( "Location" ) );

    if ( domain != _domain ) {
      if ( proto == "https" ) {
        client = std::make_shared< httplib::SSLClient >( domain, SSL_PORT );
      } else {
        client = std::make_shared< httplib::Client >( domain, DOCKER_PORT );
      }
    }
  }

  res = client->Head( location.c_str(), authHeaders() );
  
  switch ( HTTP_CODE( res->status ) ) {
    case HTTP_CODE::OK:
    case HTTP_CODE::Found:
    case HTTP_CODE::Not_Found: // Common if not yet pushed images so it created a lot of noise
      break;
    default:
      std::cerr << "OCI::Registry::Client::hasBlob " << location << std::endl;
      std::cerr << "  Status: " << res->status << " Body: " << res->body << std::endl;
      for ( auto const& header : res->headers ) {
        std::cerr << header.first << " -> " << header.second << std::endl;
      }
  }

  _cli->set_follow_location( true );

  return HTTP_CODE( res->status ) == HTTP_CODE::OK or HTTP_CODE( res->status ) == HTTP_CODE::Found;
}

auto OCI::Registry::Client::putBlob( const Schema1::ImageManifest& im,
                                     const std::string& target,
                                     std::uintmax_t total_size,
                                     const char * blob_part,
                                     uint64_t blob_part_size ) -> bool {
  (void)im;
  (void)target;
  (void)total_size;
  (void)blob_part;
  (void)blob_part_size;

  std::cout << "OCI::Registry::Client::putBlob Schema1::ImageManifest Not_Implemented" << std::endl;

  return false;
}

auto OCI::Registry::Client::putBlob( Schema2::ImageManifest const& im,
                                     std::string const& target,
                                     SHA256 const& blob_sha,
                                     std::uintmax_t total_size,
                                     const char * blob_part,
                                     uint64_t blob_part_size ) -> bool {
  bool retVal = false;

  // https://docs.docker.com/registry/spec/api/#initiate-blob-upload -- Resumable
  auto headers = authHeaders();
  std::shared_ptr< httplib::Response > res;

  bool has_error  = false;
  bool chunk_sent = false;

  if ( _patch_location.empty() ) {
    std::cerr << "Starting upload for Blob " << blob_sha << std::endl;
    res = _cli->Post( ( "/v2/" + im.name + "/blobs/uploads" ).c_str(), headers, "", "" );
  } else {
    headers.emplace( "Host", _domain );
    res = _patch_cli->Get( ( _patch_location ).c_str(), headers );
  }

  while ( not retVal and not has_error ) {
    switch ( HTTP_CODE( res->status ) ) {
      case HTTP_CODE::Unauthorized:
        if ( _auth_retry ) {
          _auth_retry = false;

          auth( res->headers, "repository:" + im.name + ":push" );

          retVal = putBlob( im, target, blob_sha, total_size, blob_part, blob_part_size );
        } else {
          _auth_retry = true;
        }

        break;
      case HTTP_CODE::Accepted: 
        {
          std::string proto;
          std::string domain;
          std::uint16_t port = 0;

          std::tie( proto, domain, _patch_location ) = splitLocation( res->get_header_value( "Location" ) );

          if ( _patch_cli == nullptr ) {
            if ( proto == "https" ) {
              port = SSL_PORT;
            } else {
              port = HTTP_PORT;
            }

            auto has_alt_port = domain.find( ':' );

            if ( has_alt_port != std::string::npos ) {
              port = std::stoul( domain.substr( has_alt_port + 1 ) ); 
              domain = domain.substr( 0, has_alt_port );
            }

            if ( proto == "https" ) {
              _patch_cli = std::make_unique< httplib::SSLClient >( domain, port );
            } else {
              _patch_cli = std::make_unique< httplib::Client >( domain, port );
            }
          }
        }

        if ( chunk_sent ) {
          retVal = true;
        } else {
          headers.emplace( "Host", _domain );
          res = _patch_cli->Get( ( _patch_location ).c_str(), headers );
        }

        break;
      case HTTP_CODE::No_Content:
        if ( chunk_sent ) {
          has_error = true;
        } else {
          chunk_sent       = true;
          auto range       = res->get_header_value( "Range" );
          auto last_offset = std::stoul( range.substr( range.find( '-' ) + 1 ) ) + 1;

          headers.emplace( "Content-Range", std::to_string( last_offset ) + "-" + std::to_string( last_offset + blob_part_size ) );
          headers.emplace( "Content-Length", std::to_string( blob_part_size ) );

          // Not documented, but is part of the API
          httplib::ContentProvider content_provider = [&]( size_t offset, size_t length, httplib::DataSink &sink ) {
            std::cout << "BPS: " << blob_part_size << " Offset: " << offset << " Length: " << length << std::endl;
            (void)offset;
            (void)length;
            sink.write( blob_part, blob_part_size );
          };

          if ( last_offset + blob_part_size < total_size ) {
            // Patch till close to finish
            res = _patch_cli->Patch( _patch_location.c_str(), headers, blob_part_size, content_provider, "application/octet-stream"  );
          } else {
            // Finalize and label with the digest
            res = _patch_cli->Put( ( _patch_location + "?digest=" + blob_sha ).c_str(), headers, blob_part_size, content_provider, "application/octet-stream" );

            _patch_cli      = nullptr;
            _patch_location = "";
          }
        }

        break;
      default:
        std::cerr << "OCI::Registry::Client::putBlob " << im.name << std::endl;
        std::cerr << "  Status: " << res->status << std::endl;
        for ( auto const& header : res->headers ) {
          std::cerr << header.first << " -> " << header.second << std::endl;
        }
        std::cerr << " Body: " << res->body << std::endl;
        has_error = true;
    }
  }

  return retVal;
}

void OCI::Registry::Client::fetchManifest( Schema1::ImageManifest& im, const std::string& rsrc, const std::string& target ) {
  auto json_body = fetchManifest( im.mediaType, rsrc, target );

  json_body.get_to( im );

  if ( im.name.empty() ) {
    im.name = rsrc;
  }

  im.originDomain = _domain; // This is just for sync from a Registry to a Directory
}

void OCI::Registry::Client::fetchManifest( Schema1::SignedImageManifest& sim, const std::string& rsrc, const std::string& target ) {
  auto json_body = fetchManifest( sim.mediaType, rsrc, target );

  json_body.get_to( sim );

  if ( sim.name.empty() ) {
    sim.name = rsrc;
  }

  sim.originDomain = _domain; // This is just for sync from a Registry to a Directory
}

void OCI::Registry::Client::fetchManifest( Schema2::ManifestList& ml, const std::string& rsrc, const std::string& target ) {
  auto json_body = fetchManifest( ml.mediaType, rsrc, target );
  _requested_target = target;

  json_body.get_to( ml );

  if ( ml.name.empty() ) {
    ml.name = rsrc;
  }

  ml.originDomain    = _domain;
  ml.requestedTarget = _requested_target;
}

void OCI::Registry::Client::fetchManifest( Schema2::ImageManifest& im, const std::string& rsrc, const std::string& target ) {
  auto json_body = fetchManifest( im.mediaType, rsrc, target );

  json_body.get_to( im );

  if ( im.name.empty() ) {
    im.name = rsrc;
  }

  im.originDomain    = _domain; // This is just for sync from a Registry to a Directory
  im.requestedTarget = _requested_target;
}

auto OCI::Registry::Client::fetchManifest( const std::string& mediaType, const std::string& resource, const std::string& target ) -> nlohmann::json {
  auto location = "/v2/" + resource + "/manifests/" + target;
  auto headers  = authHeaders();

  nlohmann::json retVal;

  headers.emplace( "Accept", mediaType );

  auto res = _cli->Get( location.c_str(), headers );

  switch ( HTTP_CODE( res->status ) ) {
    case HTTP_CODE::Unauthorized:
      auth( res->headers, "repository:" + resource + ":pull" ); // auth modifies the headers, so should auth return headers???

      retVal = fetchManifest( mediaType, resource, target ); // Hopefully this doesn't spiral into an infinite auth loop
      break;
    case HTTP_CODE::OK:
      retVal = nlohmann::json::parse( res->body );
    case HTTP_CODE::Not_Found:
      break;
    default:
      std::cerr << "OCI::Registry::Client::fetchManifest " << location << std::endl;
      std::cerr << "  Status: " << res->status << " Body: " << res->body << std::endl;
  }

  return retVal;
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

auto OCI::Registry::Client::ping() -> bool {
  bool retVal = true;

  auto res = _cli->Get( "/v2/" );

  if ( res == nullptr ) { // Most likely an invalid domain or port
    retVal = false;
  } else {
    if ( res->has_header( "Docker-Distribution-Api-Version" ) ) {
      if ( res->get_header_value( "Docker-Distribution-Api-Version" ) != "registry/2.0" ) {
        retVal = false;
      }
    } else {
      retVal = false;
    }
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
