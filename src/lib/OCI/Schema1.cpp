#include <OCI/Schema1.hpp>
#include <spdlog/spdlog.h>

auto OCI::Schema1::operator==( ImageManifest const &im1, ImageManifest const &im2 ) -> bool {
  return im1.fsLayers == im2.fsLayers;
}

auto OCI::Schema1::operator!=( ImageManifest const &im1, ImageManifest const &im2 ) -> bool {
  return not( im1 == im2 );
}

auto OCI::Schema1::operator==( ImageManifest::fsLayer const &imfsl1, ImageManifest::fsLayer const &imfsl2 ) -> bool {
  return imfsl1.blobSum == imfsl2.blobSum;
}

auto OCI::Schema1::operator!=( ImageManifest::fsLayer const &imfsl1, ImageManifest::fsLayer const &imfsl2 ) -> bool {
  return not( imfsl1.blobSum == imfsl2.blobSum );
}

void OCI::Schema1::from_json( const nlohmann::json &j, OCI::Schema1::ImageManifest &im ) {
  if ( j.find( "schemaVersion" ) == j.end() ) {
    spdlog::info( "OCI::Schema1 no schemaVersion" );
  } else {
    spdlog::info( "OCI::Schema1 found schemaVersion" );
    j.at( "schemaVersion" ).get_to( im.schemaVersion );
    j.at( "name" ).get_to( im.name );
    j.at( "tag" ).get_to( im.tag );
    j.at( "architecture" ).get_to( im.architecture );

    // mediaType introduced in v2
    // config introduced in v2

    if ( j.find( "fsLayers" ) != j.end() ) {
      im.fsLayers = j.at( "fsLayers" ).get< std::vector< ImageManifest::fsLayer >  >();
    }

    if ( j.find( "history" ) != j.end() ) {
      im.history = j.at( "history" ).get< std::vector< ImageManifest::histEntry >  >();
    }
  }
}

void OCI::Schema1::from_json( const nlohmann::json &j, ImageManifest::fsLayer &imfsl ) {
  j.at( "blobSum" ).get_to(imfsl.blobSum);
}

void OCI::Schema1::from_json( const nlohmann::json &j, ImageManifest::histEntry &imh ) {
  j.at( "v1Compatibility" ).get_to(imh.v1Compatibility);
}

void OCI::Schema1::from_json( const nlohmann::json &j, OCI::Schema1::SignedImageManifest &sim ) {
  (void)j;
  (void)sim;

  spdlog::warn( "Construct OCI::Schema1::SignedManifest" );
}

void OCI::Schema1::from_json( const nlohmann::json &j, OCI::Schema1::SignedImageManifest::Signature &sims ) {
  (void)j;
  (void)sims;

  spdlog::warn( "Construct OCI::Schema1::SignedManifest::Signature" );
}

void OCI::Schema1::to_json( nlohmann::json &j, OCI::Schema1::ImageManifest const &im ) {
  // clang-format off
  j = nlohmann::json{
      { "schemaVersion", im.schemaVersion },
      { "architecture", im.architecture },
      { "name", im.name },
      { "fsLayers", im.fsLayers },
      { "history", im.history },
      { "raw_str", im.raw_str }
  };
  // clang-format on
}
void OCI::Schema1::to_json( nlohmann::json &j, ImageManifest::fsLayer const &imfsl ) {
  j = nlohmann::json{ { "blobSum", imfsl.blobSum } };
}

void OCI::Schema1::to_json( nlohmann::json &j, ImageManifest::histEntry const &imh ) {
  j = nlohmann::json{ { "v1Compatibility", imh.v1Compatibility } };
}
