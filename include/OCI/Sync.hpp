#pragma once
#include <OCI/Base/Client.hpp>
#include <OCI/Copy.hpp>
#include <OCI/Extensions/Yaml.hpp>

namespace OCI {
  void Sync( OCI::Extensions::Yaml *src, OCI::Base::Client *dest, ProgressBars &progress_bars );

  void Sync( OCI::Base::Client *src, OCI::Base::Client *dest, ProgressBars &progress_bars );

  void Sync( std::string const &rsrc, OCI::Base::Client *src, OCI::Base::Client *dest, ProgressBars &progress_bars );

  void Sync( std::string const &rsrc, std::vector< std::string > const &tags, OCI::Base::Client *src,
             OCI::Base::Client *dest, ProgressBars &progress_bars );
} // namespace OCI
