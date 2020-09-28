
#pragma once

namespace indicators {
  enum class Color { grey, red, green, yellow, blue, magenta, cyan, white, unspecified };
} // namespace indicators

#pragma once
namespace indicators {
  enum class FontStyle { bold, dark, italic, underline, blink, reverse, concealed, crossed };
} // namespace indicators

#pragma once

namespace indicators {
  enum class ProgressType { incremental, decremental };
} // namespace indicators

//!
//! termcolor
//! ~~~~~~~~~
//!
//! termcolor is a header-only c++ library for printing colored messages
//! to the terminal. Written just for fun with a help of the Force.
//!
//! :copyright: (c) 2013 by Ihor Kalnytskyi
//! :license: BSD, see LICENSE for details
//!

#ifndef TERMCOLOR_HPP_
#define TERMCOLOR_HPP_

// the following snippet of code detects the current OS and
// defines the appropriate macro that is used to wrap some
// platform specific things
#if defined( _WIN32 ) || defined( _WIN64 )
#define TERMCOLOR_OS_WINDOWS
#elif defined( __APPLE__ )
#define TERMCOLOR_OS_MACOS
#elif defined( __unix__ ) || defined( __unix )
#define TERMCOLOR_OS_LINUX
#else
#error unsupported platform
#endif

// This headers provides the `isatty()`/`fileno()` functions,
// which are used for testing whether a standart stream refers
// to the terminal. As for Windows, we also need WinApi funcs
// for changing colors attributes of the terminal.
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
#include <unistd.h>
#elif defined( TERMCOLOR_OS_WINDOWS )
#include <io.h>
#include <windows.h>
#endif

#include <cstdio>
#include <iostream>
#include <optional>

#ifdef __linux__
#include <sys/syscall.h> //Use gettid() syscall under linux to get thread id

#elif defined( _AIX )
#include <pthread.h> // for pthread_getthreadid_np

#elif defined( __DragonFly__ ) || defined( __FreeBSD__ )
#include <pthread_np.h> // for pthread_getthreadid_np

#elif defined( __NetBSD__ )
#include <lwp.h> // for _lwp_self

#elif defined( __sun )
#include <thread.h> // for thr_self
#endif

namespace termcolor {
  // Forward declaration of the `_internal` namespace.
  // All comments are below.
  namespace _internal {
    // An index to be used to access a private storage of I/O streams. See
    // colorize / nocolorize I/O manipulators for details.
    static int colorize_index = std::ios_base::xalloc();

    inline auto get_standard_stream( const std::ostream &stream ) -> FILE *;
    inline auto is_colorized( std::ostream &stream ) -> bool;
    inline auto is_atty( const std::ostream &stream ) -> bool;

#if defined( TERMCOLOR_OS_WINDOWS )
    inline void win_change_attributes( std::ostream &stream, int foreground, int background = -1 );
#endif
  } // namespace _internal

  inline auto colorize( std::ostream &stream ) -> std::ostream & {
    stream.iword( _internal::colorize_index ) = 1L;
    return stream;
  }

  inline auto nocolorize( std::ostream &stream ) -> std::ostream & {
    stream.iword( _internal::colorize_index ) = 0L;
    return stream;
  }

  inline auto reset( std::ostream &stream ) -> std::ostream & {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[00m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, -1, -1 );
#endif
    }
    return stream;
  }

  inline auto bold( std::ostream &stream ) -> std::ostream & {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[1m";
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  inline auto dark( std::ostream &stream ) -> std::ostream & {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[2m";
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  inline auto italic( std::ostream &stream ) -> std::ostream & {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[3m";
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  inline auto underline( std::ostream &stream ) -> std::ostream & {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[4m";
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  inline auto blink( std::ostream &stream ) -> std::ostream & {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[5m";
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  inline auto reverse( std::ostream &stream ) -> std::ostream & {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[7m";
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  inline std::ostream &concealed( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[8m";
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  inline std::ostream &crossed( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[9m";
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  template < uint8_t code > inline auto color( std::ostream &stream ) -> std::ostream & {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      char command[ 12 ]; // NOLINT
      std::snprintf( command, sizeof( command ), "\033[38;5;%dm", code ); // NOLINT
      stream << command; // NOLINT
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  template < uint8_t code > inline std::ostream &on_color( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      char command[ 12 ]; // NOLINT
      std::snprintf( command, sizeof( command ), "\033[48;5;%dm", code ); // NOLINT
      stream << command; // NOLINT
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  template < uint8_t r, uint8_t g, uint8_t b > inline std::ostream &color( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      char command[ 20 ]; // NOLINT
      std::snprintf( command, sizeof( command ), "\033[38;2;%d;%d;%dm", r, g, b ); // NOLINT
      stream << command; // NOLINT
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  template < uint8_t r, uint8_t g, uint8_t b > inline std::ostream &on_color( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      char command[ 20 ]; // NOLINT
      std::snprintf( command, sizeof( command ), "\033[48;2;%d;%d;%dm", r, g, b ); // NOLINT
      stream << command; // NOLINT
#elif defined( TERMCOLOR_OS_WINDOWS )
#endif
    }
    return stream;
  }

  inline std::ostream &grey( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[30m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream,
                                        0 // grey (black)
      );
#endif
    }
    return stream;
  }

  inline std::ostream &red( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[31m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, FOREGROUND_RED );
#endif
    }
    return stream;
  }

  inline std::ostream &green( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[32m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, FOREGROUND_GREEN );
#endif
    }
    return stream;
  }

  inline std::ostream &yellow( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[33m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, FOREGROUND_GREEN | FOREGROUND_RED );
#endif
    }
    return stream;
  }

  inline std::ostream &blue( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[34m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, FOREGROUND_BLUE );
#endif
    }
    return stream;
  }

  inline std::ostream &magenta( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[35m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, FOREGROUND_BLUE | FOREGROUND_RED );
#endif
    }
    return stream;
  }

  inline std::ostream &cyan( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[36m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, FOREGROUND_BLUE | FOREGROUND_GREEN );
#endif
    }
    return stream;
  }

  inline std::ostream &white( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[37m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
#endif
    }
    return stream;
  }

  inline std::ostream &on_grey( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[40m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, -1,
                                        0 // grey (black)
      );
#endif
    }
    return stream;
  }

  inline std::ostream &on_red( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[41m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, -1, BACKGROUND_RED );
#endif
    }
    return stream;
  }

  inline std::ostream &on_green( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[42m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, -1, BACKGROUND_GREEN );
#endif
    }
    return stream;
  }

  inline std::ostream &on_yellow( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[43m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, -1, BACKGROUND_GREEN | BACKGROUND_RED );
#endif
    }
    return stream;
  }

  inline std::ostream &on_blue( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[44m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, -1, BACKGROUND_BLUE );
#endif
    }
    return stream;
  }

  inline std::ostream &on_magenta( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[45m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, -1, BACKGROUND_BLUE | BACKGROUND_RED );
#endif
    }
    return stream;
  }

  inline std::ostream &on_cyan( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[46m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, -1, BACKGROUND_GREEN | BACKGROUND_BLUE );
#endif
    }
    return stream;
  }

  inline std::ostream &on_white( std::ostream &stream ) {
    if ( _internal::is_colorized( stream ) ) {
#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      stream << "\033[47m";
#elif defined( TERMCOLOR_OS_WINDOWS )
      _internal::win_change_attributes( stream, -1, BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED );
#endif
    }

    return stream;
  }

  //! Since C++ hasn't a way to hide something in the header from
  //! the outer access, I have to introduce this namespace which
  //! is used for internal purpose and should't be access from
  //! the user code.
  namespace _internal {
    //! Since C++ hasn't a true way to extract stream handler
    //! from the a given `std::ostream` object, I have to write
    //! this kind of hack.
    inline auto get_standard_stream( const std::ostream &stream ) -> FILE * {
      if ( &stream == &std::cout ) {
        return stdout;
      }

      if ( ( &stream == &std::cerr ) || ( &stream == &std::clog ) ) {
        return stderr;
      }

      return nullptr;
    }

    // Say whether a given stream should be colorized or not. It's always
    // true for ATTY streams and may be true for streams marked with
    // colorize flag.
    inline auto is_colorized( std::ostream &stream ) -> bool {
      return is_atty( stream ) || static_cast< bool >( stream.iword( colorize_index ) );
    }

    //! Test whether a given `std::ostream` object refers to
    //! a terminal.
    inline auto is_atty( const std::ostream &stream ) -> bool {
      FILE *std_stream = get_standard_stream( stream );

      // Unfortunately, fileno() ends with segmentation fault
      // if invalid file descriptor is passed. So we need to
      // handle this case gracefully and assume it's not a tty
      // if standard stream is not detected, and 0 is returned.
      if ( std_stream == nullptr ) {
        return false;
      }

#if defined( TERMCOLOR_OS_MACOS ) || defined( TERMCOLOR_OS_LINUX )
      return ::isatty( fileno( std_stream ) );
#elif defined( TERMCOLOR_OS_WINDOWS )
      return ::_isatty( _fileno( std_stream ) );
#endif
    }

#if defined( TERMCOLOR_OS_WINDOWS )
    //! Change Windows Terminal colors attribute. If some
    //! parameter is `-1` then attribute won't changed.
    inline void win_change_attributes( std::ostream &stream, int foreground, int background ) {
      // yeah, i know.. it's ugly, it's windows.
      static WORD defaultAttributes = 0;

      // Windows doesn't have ANSI escape sequences and so we use special
      // API to change Terminal output color. That means we can't
      // manipulate colors by means of "std::stringstream" and hence
      // should do nothing in this case.
      if ( !_internal::is_atty( stream ) )
        return;

      // get terminal handle
      HANDLE hTerminal = INVALID_HANDLE_VALUE;
      if ( &stream == &std::cout )
        hTerminal = GetStdHandle( STD_OUTPUT_HANDLE );
      else if ( &stream == &std::cerr )
        hTerminal = GetStdHandle( STD_ERROR_HANDLE );

      // save default terminal attributes if it unsaved
      if ( !defaultAttributes ) {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if ( !GetConsoleScreenBufferInfo( hTerminal, &info ) )
          return;
        defaultAttributes = info.wAttributes;
      }

      // restore all default settings
      if ( foreground == -1 && background == -1 ) {
        SetConsoleTextAttribute( hTerminal, defaultAttributes );
        return;
      }

      // get current settings
      CONSOLE_SCREEN_BUFFER_INFO info;
      if ( !GetConsoleScreenBufferInfo( hTerminal, &info ) )
        return;

      if ( foreground != -1 ) {
        info.wAttributes &= ~( info.wAttributes & 0x0F );
        info.wAttributes |= static_cast< WORD >( foreground );
      }

      if ( background != -1 ) {
        info.wAttributes &= ~( info.wAttributes & 0xF0 );
        info.wAttributes |= static_cast< WORD >( background );
      }

      SetConsoleTextAttribute( hTerminal, info.wAttributes );
    }
#endif // TERMCOLOR_OS_WINDOWS

  } // namespace _internal

} // namespace termcolor

#undef TERMCOLOR_OS_WINDOWS
#undef TERMCOLOR_OS_MACOS
#undef TERMCOLOR_OS_LINUX

#endif // TERMCOLOR_HPP_

#pragma once
#include <utility>

#if defined( _MSC_VER )
#include <windows.h>

namespace indicators {

  static inline std::pair< size_t, size_t > terminal_size() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int                        cols, rows;
    GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );
    cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return { static_cast< size_t >( rows ), static_cast< size_t >( cols ) };
  }

  size_t terminal_width() { return terminal_size().second; }

} // namespace indicators

#else

#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h>    // for STDOUT_FILENO

namespace indicators {

  static inline auto terminal_size() -> std::pair< size_t, size_t > {
    struct winsize size;
    ioctl( STDOUT_FILENO, TIOCGWINSZ, &size );
    return { static_cast< size_t >( size.ws_row ), static_cast< size_t >( size.ws_col ) };
  }

  static inline auto terminal_width() -> size_t { return terminal_size().second; }
  static inline auto terminal_height() -> size_t { return terminal_size().first; }

} // namespace indicators

#endif

/*
Activity Indicators for Modern C++
https://github.com/p-ranav/indicators

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2019 Dawid Pilarski <dawid.pilarski@panicsoftware.com>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <cstddef>
// #include <indicators/color.hpp>
// #include <indicators/font_style.hpp>
// #include <indicators/progress_type.hpp>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace indicators {

  namespace details {

    template < bool condition > struct if_else;

    template <> struct if_else< true > { using type = std::true_type; };

    template <> struct if_else< false > { using type = std::false_type; };

    template < bool condition, typename True, typename False > struct if_else_type;

    template < typename True, typename False > struct if_else_type< true, True, False > { using type = True; };

    template < typename True, typename False > struct if_else_type< false, True, False > { using type = False; };

    template < typename... Ops > struct conjuction;

    template <> struct conjuction<> : std::true_type {};

    template < typename Op, typename... TailOps >
    struct conjuction< Op, TailOps... > : if_else_type< !Op::value, std::false_type, conjuction< TailOps... > >::type {
    };

    template < typename... Ops > struct disjunction;

    template <> struct disjunction<> : std::false_type {};

    template < typename Op, typename... TailOps >
    struct disjunction< Op, TailOps... > : if_else_type< Op::value, std::true_type, disjunction< TailOps... > >::type {
    };

    enum class ProgressBarOption {
      bar_width = 0,
      prefix_text,
      postfix_text,
      start,
      end,
      fill,
      lead,
      remainder,
      max_postfix_text_len,
      completed,
      show_percentage,
      show_elapsed_time,
      show_remaining_time,
      saved_start_time,
      foreground_color,
      spinner_show,
      spinner_states,
      font_styles,
      hide_bar_when_complete,
      min_progress,
      max_progress,
      progress_type,
      stream
    };

    template < typename T, ProgressBarOption Id > struct Setting {
      template < typename... Args,
                 typename = typename std::enable_if< std::is_constructible< T, Args... >::value >::type >
      explicit Setting( Args &&... args ) : value( std::forward< Args >( args )... ) {}
      Setting( const Setting & ) = default;
      Setting( Setting && )      = default;

      static constexpr auto id = Id;
      using type               = T;

      T value{};
    };

    template < typename T > struct is_setting : std::false_type {};

    template < ProgressBarOption Id, typename T > struct is_setting< Setting< T, Id > > : std::true_type {};

    template < typename... Args > struct are_settings : if_else< conjuction< is_setting< Args >... >::value >::type {};

    template <> struct are_settings<> : std::true_type {};

    template < typename Setting, typename Tuple > struct is_setting_from_tuple;

    template < typename Setting > struct is_setting_from_tuple< Setting, std::tuple<> > : std::true_type {};

    template < typename Setting, typename... TupleTypes >
    struct is_setting_from_tuple< Setting, std::tuple< TupleTypes... > >
        : if_else< disjunction< std::is_same< Setting, TupleTypes >... >::value >::type {};

    template < typename Tuple, typename... Settings >
    struct are_settings_from_tuple : if_else< conjuction< is_setting_from_tuple< Settings, Tuple >... >::value >::type {
    };

    template < ProgressBarOption Id > struct always_true { static constexpr auto value = true; };

    template < ProgressBarOption Id, typename Default > Default &&get_impl( Default &&def ) {
      return std::forward< Default >( def );
    }

    template < ProgressBarOption Id, typename Default, typename T, typename... Args >
    auto get_impl( Default && /*def*/, T &&first, Args &&... /*tail*/ ) ->
        typename std::enable_if< ( std::decay< T >::type::id == Id ), decltype( std::forward< T >( first ) ) >::type {
      return std::forward< T >( first );
    }

    template < ProgressBarOption Id, typename Default, typename T, typename... Args >
    auto get_impl( Default &&def, T && /*first*/, Args &&... tail ) ->
        typename std::enable_if< ( std::decay< T >::type::id != Id ),
                                 decltype( get_impl< Id >( std::forward< Default >( def ),
                                                           std::forward< Args >( tail )... ) ) >::type {
      return get_impl< Id >( std::forward< Default >( def ), std::forward< Args >( tail )... );
    }

    template < ProgressBarOption Id, typename Default, typename... Args,
               typename = typename std::enable_if< are_settings< Args... >::value, void >::type >
    auto get( Default &&def, Args &&... args )
        -> decltype( details::get_impl< Id >( std::forward< Default >( def ), std::forward< Args >( args )... ) ) {
      return details::get_impl< Id >( std::forward< Default >( def ), std::forward< Args >( args )... );
    }

    template < ProgressBarOption Id > using StringSetting = Setting< std::string, Id >;

    template < ProgressBarOption Id > using IntegerSetting = Setting< std::size_t, Id >;

    template < ProgressBarOption Id > using BooleanSetting = Setting< bool, Id >;

    template < ProgressBarOption Id, typename Tuple, std::size_t counter = 0 > struct option_idx;

    template < ProgressBarOption Id, typename T, typename... Settings, std::size_t counter >
    struct option_idx< Id, std::tuple< T, Settings... >, counter >
        : if_else_type< ( Id == T::id ), std::integral_constant< std::size_t, counter >,
                        option_idx< Id, std::tuple< Settings... >, counter + 1 > >::type {};

    template < ProgressBarOption Id, std::size_t counter > struct option_idx< Id, std::tuple<>, counter > {
      static_assert( always_true< (ProgressBarOption)Id >::value, "No such option was found" );
    };

    template < ProgressBarOption Id, typename Settings >
    auto get_value( Settings &&settings )
        -> decltype( ( std::get< option_idx< Id, typename std::decay< Settings >::type >::value >(
            std::declval< Settings && >() ) ) ) {
      return std::get< option_idx< Id, typename std::decay< Settings >::type >::value >(
          std::forward< Settings >( settings ) );
    }

  } // namespace details

  namespace option {
    using BarWidth          = details::IntegerSetting< details::ProgressBarOption::bar_width >;
    using PrefixText        = details::StringSetting< details::ProgressBarOption::prefix_text >;
    using PostfixText       = details::StringSetting< details::ProgressBarOption::postfix_text >;
    using Start             = details::StringSetting< details::ProgressBarOption::start >;
    using End               = details::StringSetting< details::ProgressBarOption::end >;
    using Fill              = details::StringSetting< details::ProgressBarOption::fill >;
    using Lead              = details::StringSetting< details::ProgressBarOption::lead >;
    using Remainder         = details::StringSetting< details::ProgressBarOption::remainder >;
    using MaxPostfixTextLen = details::IntegerSetting< details::ProgressBarOption::max_postfix_text_len >;
    using Completed         = details::BooleanSetting< details::ProgressBarOption::completed >;
    using ShowPercentage    = details::BooleanSetting< details::ProgressBarOption::show_percentage >;
    using ShowElapsedTime   = details::BooleanSetting< details::ProgressBarOption::show_elapsed_time >;
    using ShowRemainingTime = details::BooleanSetting< details::ProgressBarOption::show_remaining_time >;
    using SavedStartTime    = details::BooleanSetting< details::ProgressBarOption::saved_start_time >;
    using ForegroundColor   = details::Setting< Color, details::ProgressBarOption::foreground_color >;
    using ShowSpinner       = details::BooleanSetting< details::ProgressBarOption::spinner_show >;
    using SpinnerStates     = details::Setting< std::vector< std::string >, details::ProgressBarOption::spinner_states >;
    using HideBarWhenComplete = details::BooleanSetting< details::ProgressBarOption::hide_bar_when_complete >;
    using FontStyles          = details::Setting< std::vector< FontStyle >, details::ProgressBarOption::font_styles >;
    using MinProgress         = details::IntegerSetting< details::ProgressBarOption::min_progress >;
    using MaxProgress         = details::IntegerSetting< details::ProgressBarOption::max_progress >;
    using ProgressType        = details::Setting< ProgressType, details::ProgressBarOption::progress_type >;
    using Stream              = details::Setting< std::ostream &, details::ProgressBarOption::stream >;
  } // namespace option
} // namespace indicators

#pragma once

#if defined( _MSC_VER )
#if !defined( NOMINMAX )
#define NOMINMAX
#endif
#include <io.h>
#include <windows.h>
#else
#include <cstdio>
#endif

namespace indicators {

#if defined( _MSC_VER )

  static inline void show_console_cursor( bool const show ) {
    HANDLE out = GetStdHandle( STD_OUTPUT_HANDLE );

    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo( out, &cursorInfo );
    cursorInfo.bVisible = show; // set the cursor visibility
    SetConsoleCursorInfo( out, &cursorInfo );
  }

#else

  static inline void show_console_cursor( bool const show ) { std::fputs( show ? "\033[?25h" : "\033[?25l", stdout ); }

#endif

} // namespace indicators
#pragma once

#if defined( _MSC_VER )
#if !defined( NOMINMAX )
#define NOMINMAX
#endif
#include <io.h>
#include <windows.h>
#else
#include <iostream>
#endif

namespace indicators {

#ifdef _MSC_VER

  static inline void move( int x, int y ) {
    auto hStdout = GetStdHandle( STD_OUTPUT_HANDLE );
    if ( !hStdout )
      return;

    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    GetConsoleScreenBufferInfo( hStdout, &csbiInfo );

    COORD cursor;

    cursor.X = csbiInfo.dwCursorPosition.X + x;
    cursor.Y = csbiInfo.dwCursorPosition.Y + y;
    SetConsoleCursorPosition( hStdout, cursor );
  }

  static inline void move_up( int lines ) { move( 0, -lines ); }
  static inline void move_down( int lines ) { move( 0, -lines ); }
  static inline void move_right( int cols ) { move( cols, 0 ); }
  static inline void move_left( int cols ) { move( -cols, 0 ); }

#else

  static inline void move_up( int lines ) { std::cout << "\033[" << lines << "A"; }
  static inline void move_down( int lines ) { std::cout << "\033[" << lines << "B"; }
  static inline void move_right( int cols ) { std::cout << "\033[" << cols << "C"; }
  static inline void move_left( int cols ) { std::cout << "\033[" << cols << "D"; }

#endif

} // namespace indicators
#pragma once

// #include <indicators/display_width.hpp>

#include <clocale>
#if __has_include( <codecvt>)
#include <codecvt>
#define INDICATORS_HAVE_CODECVT 1 // NOLINT
#endif
#include <cstdlib>
#include <locale>
#include <string>
#include <cwchar>

namespace unicode {

#if INDICATORS_HAVE_CODECVT
  namespace details {

    /*
     * This is an implementation of wcwidth() and wcswidth() (defined in
     * IEEE Std 1002.1-2001) for Unicode.
     *
     * http://www.opengroup.org/onlinepubs/007904975/functions/wcwidth.html
     * http://www.opengroup.org/onlinepubs/007904975/functions/wcswidth.html
     *
     * In fixed-width output devices, Latin characters all occupy a single
     * "cell" position of equal width, whereas ideographic CJK characters
     * occupy two such cells. Interoperability between terminal-line
     * applications and (teletype-style) character terminals using the
     * UTF-8 encoding requires agreement on which character should advance
     * the cursor by how many cell positions. No established formal
     * standards exist at present on which Unicode character shall occupy
     * how many cell positions on character terminals. These routines are
     * a first attempt of defining such behavior based on simple rules
     * applied to data provided by the Unicode Consortium.
     *
     * For some graphical characters, the Unicode standard explicitly
     * defines a character-cell width via the definition of the East Asian
     * FullWidth (F), Wide (W), Half-width (H), and Narrow (Na) classes.
     * In all these cases, there is no ambiguity about which width a
     * terminal shall use. For characters in the East Asian Ambiguous (A)
     * class, the width choice depends purely on a preference of backward
     * compatibility with either historic CJK or Western practice.
     * Choosing single-width for these characters is easy to justify as
     * the appropriate long-term solution, as the CJK practice of
     * displaying these characters as double-width comes from historic
     * implementation simplicity (8-bit encoded characters were displayed
     * single-width and 16-bit ones double-width, even for Greek,
     * Cyrillic, etc.) and not any typographic considerations.
     *
     * Much less clear is the choice of width for the Not East Asian
     * (Neutral) class. Existing practice does not dictate a width for any
     * of these characters. It would nevertheless make sense
     * typographically to allocate two character cells to characters such
     * as for instance EM SPACE or VOLUME INTEGRAL, which cannot be
     * represented adequately with a single-width glyph. The following
     * routines at present merely assign a single-cell width to all
     * neutral characters, in the interest of simplicity. This is not
     * entirely satisfactory and should be reconsidered before
     * establishing a formal standard in this area. At the moment, the
     * decision which Not East Asian (Neutral) characters should be
     * represented by double-width glyphs cannot yet be answered by
     * applying a simple rule from the Unicode database content. Setting
     * up a proper standard for the behavior of UTF-8 character terminals
     * will require a careful analysis not only of each Unicode character,
     * but also of each presentation form, something the author of these
     * routines has avoided to do so far.
     *
     * http://www.unicode.org/unicode/reports/tr11/
     *
     * Markus Kuhn -- 2007-05-26 (Unicode 5.0)
     *
     * Permission to use, copy, modify, and distribute this software
     * for any purpose and without fee is hereby granted. The author
     * disclaims all warranties with regard to this software.
     *
     * Latest version: http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
     */

    struct interval {
      int first;
      int last;
    };

    /* auxiliary function for binary search in interval table */
    static inline auto bisearch( wchar_t ucs, const struct interval *table, int max ) -> int {
      int min = 0;
      int mid = 0;

      if ( ucs < table[ 0 ].first || ucs > table[ max ].last ) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return 0;
      }

      while ( max >= min ) {
        mid = ( min + max ) / 2;
        if ( ucs > table[ mid ].last ) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
          min = mid + 1;
        } else if ( ucs < table[ mid ].first ) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
          max = mid - 1;
        } else {
          return 1;
        }
      }

      return 0;
    }

    /* The following two functions define the column width of an ISO 10646
     * character as follows:
     *
     *    - The null character (U+0000) has a column width of 0.
     *
     *    - Other C0/C1 control characters and DEL will lead to a return
     *      value of -1.
     *
     *    - Non-spacing and enclosing combining characters (general
     *      category code Mn or Me in the Unicode database) have a
     *      column width of 0.
     *
     *    - SOFT HYPHEN (U+00AD) has a column width of 1.
     *
     *    - Other format characters (general category code Cf in the Unicode
     *      database) and ZERO WIDTH SPACE (U+200B) have a column width of 0.
     *
     *    - Hangul Jamo medial vowels and final consonants (U+1160-U+11FF)
     *      have a column width of 0.
     *
     *    - Spacing characters in the East Asian Wide (W) or East Asian
     *      Full-width (F) category as defined in Unicode Technical
     *      Report #11 have a column width of 2.
     *
     *    - All remaining characters (including all printable
     *      ISO 8859-1 and WGL4 characters, Unicode control characters,
     *      etc.) have a column width of 1.
     *
     * This implementation assumes that wchar_t characters are encoded
     * in ISO 10646.
     */

    static inline int mk_wcwidth( wchar_t ucs ) {
      /* sorted list of non-overlapping intervals of non-spacing characters */
      /* generated by "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c" */
      static const struct interval combining[] = {
          { 0x0300, 0x036F },   { 0x0483, 0x0486 },   { 0x0488, 0x0489 },   { 0x0591, 0x05BD },   { 0x05BF, 0x05BF },
          { 0x05C1, 0x05C2 },   { 0x05C4, 0x05C5 },   { 0x05C7, 0x05C7 },   { 0x0600, 0x0603 },   { 0x0610, 0x0615 },
          { 0x064B, 0x065E },   { 0x0670, 0x0670 },   { 0x06D6, 0x06E4 },   { 0x06E7, 0x06E8 },   { 0x06EA, 0x06ED },
          { 0x070F, 0x070F },   { 0x0711, 0x0711 },   { 0x0730, 0x074A },   { 0x07A6, 0x07B0 },   { 0x07EB, 0x07F3 },
          { 0x0901, 0x0902 },   { 0x093C, 0x093C },   { 0x0941, 0x0948 },   { 0x094D, 0x094D },   { 0x0951, 0x0954 },
          { 0x0962, 0x0963 },   { 0x0981, 0x0981 },   { 0x09BC, 0x09BC },   { 0x09C1, 0x09C4 },   { 0x09CD, 0x09CD },
          { 0x09E2, 0x09E3 },   { 0x0A01, 0x0A02 },   { 0x0A3C, 0x0A3C },   { 0x0A41, 0x0A42 },   { 0x0A47, 0x0A48 },
          { 0x0A4B, 0x0A4D },   { 0x0A70, 0x0A71 },   { 0x0A81, 0x0A82 },   { 0x0ABC, 0x0ABC },   { 0x0AC1, 0x0AC5 },
          { 0x0AC7, 0x0AC8 },   { 0x0ACD, 0x0ACD },   { 0x0AE2, 0x0AE3 },   { 0x0B01, 0x0B01 },   { 0x0B3C, 0x0B3C },
          { 0x0B3F, 0x0B3F },   { 0x0B41, 0x0B43 },   { 0x0B4D, 0x0B4D },   { 0x0B56, 0x0B56 },   { 0x0B82, 0x0B82 },
          { 0x0BC0, 0x0BC0 },   { 0x0BCD, 0x0BCD },   { 0x0C3E, 0x0C40 },   { 0x0C46, 0x0C48 },   { 0x0C4A, 0x0C4D },
          { 0x0C55, 0x0C56 },   { 0x0CBC, 0x0CBC },   { 0x0CBF, 0x0CBF },   { 0x0CC6, 0x0CC6 },   { 0x0CCC, 0x0CCD },
          { 0x0CE2, 0x0CE3 },   { 0x0D41, 0x0D43 },   { 0x0D4D, 0x0D4D },   { 0x0DCA, 0x0DCA },   { 0x0DD2, 0x0DD4 },
          { 0x0DD6, 0x0DD6 },   { 0x0E31, 0x0E31 },   { 0x0E34, 0x0E3A },   { 0x0E47, 0x0E4E },   { 0x0EB1, 0x0EB1 },
          { 0x0EB4, 0x0EB9 },   { 0x0EBB, 0x0EBC },   { 0x0EC8, 0x0ECD },   { 0x0F18, 0x0F19 },   { 0x0F35, 0x0F35 },
          { 0x0F37, 0x0F37 },   { 0x0F39, 0x0F39 },   { 0x0F71, 0x0F7E },   { 0x0F80, 0x0F84 },   { 0x0F86, 0x0F87 },
          { 0x0F90, 0x0F97 },   { 0x0F99, 0x0FBC },   { 0x0FC6, 0x0FC6 },   { 0x102D, 0x1030 },   { 0x1032, 0x1032 },
          { 0x1036, 0x1037 },   { 0x1039, 0x1039 },   { 0x1058, 0x1059 },   { 0x1160, 0x11FF },   { 0x135F, 0x135F },
          { 0x1712, 0x1714 },   { 0x1732, 0x1734 },   { 0x1752, 0x1753 },   { 0x1772, 0x1773 },   { 0x17B4, 0x17B5 },
          { 0x17B7, 0x17BD },   { 0x17C6, 0x17C6 },   { 0x17C9, 0x17D3 },   { 0x17DD, 0x17DD },   { 0x180B, 0x180D },
          { 0x18A9, 0x18A9 },   { 0x1920, 0x1922 },   { 0x1927, 0x1928 },   { 0x1932, 0x1932 },   { 0x1939, 0x193B },
          { 0x1A17, 0x1A18 },   { 0x1B00, 0x1B03 },   { 0x1B34, 0x1B34 },   { 0x1B36, 0x1B3A },   { 0x1B3C, 0x1B3C },
          { 0x1B42, 0x1B42 },   { 0x1B6B, 0x1B73 },   { 0x1DC0, 0x1DCA },   { 0x1DFE, 0x1DFF },   { 0x200B, 0x200F },
          { 0x202A, 0x202E },   { 0x2060, 0x2063 },   { 0x206A, 0x206F },   { 0x20D0, 0x20EF },   { 0x302A, 0x302F },
          { 0x3099, 0x309A },   { 0xA806, 0xA806 },   { 0xA80B, 0xA80B },   { 0xA825, 0xA826 },   { 0xFB1E, 0xFB1E },
          { 0xFE00, 0xFE0F },   { 0xFE20, 0xFE23 },   { 0xFEFF, 0xFEFF },   { 0xFFF9, 0xFFFB },   { 0x10A01, 0x10A03 },
          { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F }, { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
          { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD }, { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 },
          { 0xE0020, 0xE007F }, { 0xE0100, 0xE01EF } };

      /* test for 8-bit control characters */
      if ( ucs == 0 ) {
        return 0;
      }

      if ( ucs < 32 || ( ucs >= 0x7f && ucs < 0xa0 ) ) { // NOLINT
        return -1;
      }

      /* binary search in table of non-spacing characters */
      if ( bisearch( ucs, combining, sizeof( combining ) / sizeof( struct interval ) - 1 ) != 0 ) { // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
        return 0;
      }

      /* if we arrive here, ucs is not a combining or C0/C1 control character */

      return 1 + ( ucs >= 0x1100 && ( ucs <= 0x115f || /* Hangul Jamo init. consonants */ // NOLINT
                                      ucs == 0x2329 || ucs == 0x232a || // NOLINT
                                      ( ucs >= 0x2e80 && ucs <= 0xa4cf && ucs != 0x303f ) || /* CJK ... Yi */ // NOLINT
                                      ( ucs >= 0xac00 && ucs <= 0xd7a3 ) ||                  /* Hangul Syllables */ // NOLINT
                                      ( ucs >= 0xf900 && ucs <= 0xfaff ) || /* CJK Compatibility Ideographs */ // NOLINT
                                      ( ucs >= 0xfe10 && ucs <= 0xfe19 ) || /* Vertical forms */ // NOLINT
                                      ( ucs >= 0xfe30 && ucs <= 0xfe6f ) || /* CJK Compatibility Forms */ // NOLINT
                                      ( ucs >= 0xff00 && ucs <= 0xff60 ) || /* Fullwidth Forms */ // NOLINT
                                      ( ucs >= 0xffe0 && ucs <= 0xffe6 ) || ( ucs >= 0x20000 && ucs <= 0x2fffd ) || // NOLINT
                                      ( ucs >= 0x30000 && ucs <= 0x3fffd ) ) ); // NOLINT
    }

    static inline auto mk_wcswidth( const wchar_t *pwcs, size_t n ) -> int {
      int w = 0;
      int width = 0;

      for ( ; (*pwcs != 0) && n-- > 0; pwcs++ ) {
        if ( ( w = mk_wcwidth( *pwcs ) ) < 0 ) {
          return -1;
        } else {
          width += w;
        }
      }

      return width;
    }

    /*
     * The following functions are the same as mk_wcwidth() and
     * mk_wcswidth(), except that spacing characters in the East Asian
     * Ambiguous (A) category as defined in Unicode Technical Report #11
     * have a column width of 2. This variant might be useful for users of
     * CJK legacy encodings who want to migrate to UCS without changing
     * the traditional terminal character-width behaviour. It is not
     * otherwise recommended for general use.
     */
    static inline int mk_wcwidth_cjk( wchar_t ucs ) {
      /* sorted list of non-overlapping intervals of East Asian Ambiguous
       * characters, generated by "uniset +WIDTH-A -cat=Me -cat=Mn -cat=Cf c" */
      static const struct interval ambiguous[] = {
          { 0x00A1, 0x00A1 },    { 0x00A4, 0x00A4 }, { 0x00A7, 0x00A8 }, { 0x00AA, 0x00AA }, { 0x00AE, 0x00AE },
          { 0x00B0, 0x00B4 },    { 0x00B6, 0x00BA }, { 0x00BC, 0x00BF }, { 0x00C6, 0x00C6 }, { 0x00D0, 0x00D0 },
          { 0x00D7, 0x00D8 },    { 0x00DE, 0x00E1 }, { 0x00E6, 0x00E6 }, { 0x00E8, 0x00EA }, { 0x00EC, 0x00ED },
          { 0x00F0, 0x00F0 },    { 0x00F2, 0x00F3 }, { 0x00F7, 0x00FA }, { 0x00FC, 0x00FC }, { 0x00FE, 0x00FE },
          { 0x0101, 0x0101 },    { 0x0111, 0x0111 }, { 0x0113, 0x0113 }, { 0x011B, 0x011B }, { 0x0126, 0x0127 },
          { 0x012B, 0x012B },    { 0x0131, 0x0133 }, { 0x0138, 0x0138 }, { 0x013F, 0x0142 }, { 0x0144, 0x0144 },
          { 0x0148, 0x014B },    { 0x014D, 0x014D }, { 0x0152, 0x0153 }, { 0x0166, 0x0167 }, { 0x016B, 0x016B },
          { 0x01CE, 0x01CE },    { 0x01D0, 0x01D0 }, { 0x01D2, 0x01D2 }, { 0x01D4, 0x01D4 }, { 0x01D6, 0x01D6 },
          { 0x01D8, 0x01D8 },    { 0x01DA, 0x01DA }, { 0x01DC, 0x01DC }, { 0x0251, 0x0251 }, { 0x0261, 0x0261 },
          { 0x02C4, 0x02C4 },    { 0x02C7, 0x02C7 }, { 0x02C9, 0x02CB }, { 0x02CD, 0x02CD }, { 0x02D0, 0x02D0 },
          { 0x02D8, 0x02DB },    { 0x02DD, 0x02DD }, { 0x02DF, 0x02DF }, { 0x0391, 0x03A1 }, { 0x03A3, 0x03A9 },
          { 0x03B1, 0x03C1 },    { 0x03C3, 0x03C9 }, { 0x0401, 0x0401 }, { 0x0410, 0x044F }, { 0x0451, 0x0451 },
          { 0x2010, 0x2010 },    { 0x2013, 0x2016 }, { 0x2018, 0x2019 }, { 0x201C, 0x201D }, { 0x2020, 0x2022 },
          { 0x2024, 0x2027 },    { 0x2030, 0x2030 }, { 0x2032, 0x2033 }, { 0x2035, 0x2035 }, { 0x203B, 0x203B },
          { 0x203E, 0x203E },    { 0x2074, 0x2074 }, { 0x207F, 0x207F }, { 0x2081, 0x2084 }, { 0x20AC, 0x20AC },
          { 0x2103, 0x2103 },    { 0x2105, 0x2105 }, { 0x2109, 0x2109 }, { 0x2113, 0x2113 }, { 0x2116, 0x2116 },
          { 0x2121, 0x2122 },    { 0x2126, 0x2126 }, { 0x212B, 0x212B }, { 0x2153, 0x2154 }, { 0x215B, 0x215E },
          { 0x2160, 0x216B },    { 0x2170, 0x2179 }, { 0x2190, 0x2199 }, { 0x21B8, 0x21B9 }, { 0x21D2, 0x21D2 },
          { 0x21D4, 0x21D4 },    { 0x21E7, 0x21E7 }, { 0x2200, 0x2200 }, { 0x2202, 0x2203 }, { 0x2207, 0x2208 },
          { 0x220B, 0x220B },    { 0x220F, 0x220F }, { 0x2211, 0x2211 }, { 0x2215, 0x2215 }, { 0x221A, 0x221A },
          { 0x221D, 0x2220 },    { 0x2223, 0x2223 }, { 0x2225, 0x2225 }, { 0x2227, 0x222C }, { 0x222E, 0x222E },
          { 0x2234, 0x2237 },    { 0x223C, 0x223D }, { 0x2248, 0x2248 }, { 0x224C, 0x224C }, { 0x2252, 0x2252 },
          { 0x2260, 0x2261 },    { 0x2264, 0x2267 }, { 0x226A, 0x226B }, { 0x226E, 0x226F }, { 0x2282, 0x2283 },
          { 0x2286, 0x2287 },    { 0x2295, 0x2295 }, { 0x2299, 0x2299 }, { 0x22A5, 0x22A5 }, { 0x22BF, 0x22BF },
          { 0x2312, 0x2312 },    { 0x2460, 0x24E9 }, { 0x24EB, 0x254B }, { 0x2550, 0x2573 }, { 0x2580, 0x258F },
          { 0x2592, 0x2595 },    { 0x25A0, 0x25A1 }, { 0x25A3, 0x25A9 }, { 0x25B2, 0x25B3 }, { 0x25B6, 0x25B7 },
          { 0x25BC, 0x25BD },    { 0x25C0, 0x25C1 }, { 0x25C6, 0x25C8 }, { 0x25CB, 0x25CB }, { 0x25CE, 0x25D1 },
          { 0x25E2, 0x25E5 },    { 0x25EF, 0x25EF }, { 0x2605, 0x2606 }, { 0x2609, 0x2609 }, { 0x260E, 0x260F },
          { 0x2614, 0x2615 },    { 0x261C, 0x261C }, { 0x261E, 0x261E }, { 0x2640, 0x2640 }, { 0x2642, 0x2642 },
          { 0x2660, 0x2661 },    { 0x2663, 0x2665 }, { 0x2667, 0x266A }, { 0x266C, 0x266D }, { 0x266F, 0x266F },
          { 0x273D, 0x273D },    { 0x2776, 0x277F }, { 0xE000, 0xF8FF }, { 0xFFFD, 0xFFFD }, { 0xF0000, 0xFFFFD },
          { 0x100000, 0x10FFFD } };

      /* binary search in table of non-spacing characters */
      if ( bisearch( ucs, ambiguous, sizeof( ambiguous ) / sizeof( struct interval ) - 1 ) )
        return 2;

      return mk_wcwidth( ucs );
    }

    static inline int mk_wcswidth_cjk( const wchar_t *pwcs, size_t n ) {
      int w, width = 0;

      for ( ; *pwcs && n-- > 0; pwcs++ )
        if ( ( w = mk_wcwidth_cjk( *pwcs ) ) < 0 )
          return -1;
        else
          width += w;

      return width;
    }

    // convert UTF-8 string to wstring
    static inline std::wstring utf8_decode( const std::string &str ) {
      std::wstring_convert< std::codecvt_utf8< wchar_t > > myconv;
      return myconv.from_bytes( str );
    }

    // convert wstring to UTF-8 string
    static inline std::string utf8_encode( const std::wstring &str ) {
      std::wstring_convert< std::codecvt_utf8< wchar_t > > myconv;
      return myconv.to_bytes( str );
    }

  } // namespace details

  static inline int display_width( const std::string &input ) {
    using namespace unicode::details;
    return mk_wcswidth( utf8_decode( input ).c_str(), input.size() );
  }

  static inline int display_width( const std::wstring &input ) {
    return details::mk_wcswidth( input.c_str(), input.size() );
  }

#else

  static inline int display_width( const std::string &input ) { return input.length(); }

  static inline int display_width( const std::wstring &input ) { return input.length(); }

#endif

} // namespace unicode

// #include <indicators/setting.hpp>
// #include <indicators/termcolor.hpp>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#include <cassert>
#include <cmath>

namespace indicators::details {
    inline void set_stream_color( std::ostream &os, Color color ) {
      switch ( color ) {
      case Color::grey:
        os << termcolor::grey;
        break;
      case Color::red:
        os << termcolor::red;
        break;
      case Color::green:
        os << termcolor::green;
        break;
      case Color::yellow:
        os << termcolor::yellow;
        break;
      case Color::blue:
        os << termcolor::blue;
        break;
      case Color::magenta:
        os << termcolor::magenta;
        break;
      case Color::cyan:
        os << termcolor::cyan;
        break;
      case Color::white:
        os << termcolor::white;
        break;
      default:
        assert( false ); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
      }
    }

    inline void set_font_style( std::ostream &os, FontStyle style ) {
      switch ( style ) {
      case FontStyle::bold:
        os << termcolor::bold;
        break;
      case FontStyle::dark:
        os << termcolor::dark;
        break;
      case FontStyle::italic:
        os << termcolor::italic;
        break;
      case FontStyle::underline:
        os << termcolor::underline;
        break;
      case FontStyle::blink:
        os << termcolor::blink;
        break;
      case FontStyle::reverse:
        os << termcolor::reverse;
        break;
      case FontStyle::concealed:
        os << termcolor::concealed;
        break;
      case FontStyle::crossed:
        os << termcolor::crossed;
        break;
      default:
        break;
      }
    }

    inline std::ostream &write_duration( std::ostream &os, std::chrono::nanoseconds ns ) {
      using namespace std;
      using namespace std::chrono;
      using days = duration< int, ratio< 86400 > >;
      char fill  = os.fill();
      os.fill( '0' );
      auto d = duration_cast< days >( ns );
      ns -= d;
      auto h = duration_cast< hours >( ns );
      ns -= h;
      auto m = duration_cast< minutes >( ns );
      ns -= m;
      auto s = duration_cast< seconds >( ns );
      if ( d.count() > 0 )
        os << setw( 2 ) << d.count() << "d:";
      if ( h.count() > 0 )
        os << setw( 2 ) << h.count() << "h:";
      os << setw( 2 ) << m.count() << "m:" << setw( 2 ) << s.count() << 's';
      os.fill( fill );
      return os;
    }

    class BlockProgressScaleWriter {
    public:
      BlockProgressScaleWriter( std::ostream &os, size_t bar_width ) : os( os ), bar_width( bar_width ) {}

      std::ostream &write( float progress ) {
        std::string                fill_text{ "█" };
        std::vector< std::string > lead_characters{ " ", "▏", "▎", "▍", "▌", "▋", "▊", "▉" };
        auto                       value           = std::min( 1.0f, std::max( 0.0f, progress / 100.0f ) );
        auto                       whole_width     = std::floor( value * bar_width );
        auto                       remainder_width = fmod( ( value * bar_width ), 1.0f );
        auto                       part_width      = std::floor( remainder_width * lead_characters.size() );
        std::string                lead_text       = lead_characters[ size_t( part_width ) ];
        if ( ( bar_width - whole_width - 1 ) < 0 )
          lead_text = "";
        for ( size_t i = 0; i < whole_width; ++i )
          os << fill_text;
        os << lead_text;
        for ( size_t i = 0; i < ( bar_width - whole_width - 1 ); ++i )
          os << " ";
        return os;
      }

    private:
      std::ostream &os;
      size_t        bar_width = 0;
    };

    class ProgressScaleWriter {
    public:
      ProgressScaleWriter( std::ostream &os, size_t bar_width, std::string fill, std::string lead,
                           std::string remainder )
          : os( os ), bar_width( bar_width ), fill(std::move( fill )), lead(std::move( lead )), remainder(std::move( remainder )) {}

      auto write( float progress ) -> std::ostream & {
        auto pos = static_cast< size_t >( progress * bar_width / 100.0 );
        for ( size_t i = 0, current_display_width = 0; i < bar_width; ) {
          std::string next;

          if ( i < pos ) {
            next                  = fill;
            current_display_width = unicode::display_width( fill );
          } else if ( i == pos ) {
            next                  = lead;
            current_display_width = unicode::display_width( lead );
          } else {
            next                  = remainder;
            current_display_width = unicode::display_width( remainder );
          }

          i += current_display_width;

          if ( i > bar_width ) {
            // `next` is larger than the allowed bar width
            // fill with empty space instead
            os << std::string( ( bar_width - ( i - current_display_width ) ), ' ' );
            break;
          }

          os << next;
        }
        return os;
      }

    private:
      std::ostream &os;
      size_t        bar_width = 0;
      std::string   fill;
      std::string   lead;
      std::string   remainder;
    };

    class IndeterminateProgressScaleWriter {
    public:
      IndeterminateProgressScaleWriter( std::ostream &os, size_t bar_width, const std::string &fill,
                                        const std::string &lead )
          : os( os ), bar_width( bar_width ), fill( fill ), lead( lead ) {}

      std::ostream &write( size_t progress ) {
        for ( size_t i = 0; i < bar_width; ) {
          std::string next;
          size_t      current_display_width = 0;

          if ( i < progress ) {
            next                  = fill;
            current_display_width = unicode::display_width( fill );
          } else if ( i == progress ) {
            next                  = lead;
            current_display_width = unicode::display_width( lead );
          } else {
            next                  = fill;
            current_display_width = unicode::display_width( fill );
          }

          i += current_display_width;

          if ( i > bar_width ) {
            // `next` is larger than the allowed bar width
            // fill with empty space instead
            os << std::string( ( bar_width - ( i - current_display_width ) ), ' ' );
            break;
          }

          os << next;
        }
        return os;
      }

    private:
      std::ostream &os;
      size_t        bar_width = 0;
      std::string   fill;
      std::string   lead;
    };

  } // namespace indicators
#pragma once

// #include <indicators/details/stream_helper.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
// #include <indicators/color.hpp>
// #include <indicators/setting.hpp>
// #include <indicators/terminal_size.hpp>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>

namespace indicators {
  class ProgressBar {
    using Settings =
        std::tuple< option::BarWidth, option::PrefixText, option::PostfixText, option::Start, option::End, option::Fill,
                    option::Lead, option::Remainder, option::MaxPostfixTextLen, option::Completed,
                    option::ShowPercentage, option::ShowElapsedTime, option::ShowRemainingTime, option::SavedStartTime,
                    option::ForegroundColor, option::FontStyles, option::MinProgress, option::MaxProgress,
                    option::ProgressType, option::Stream >;

  public:
    template < typename... Args, typename std::enable_if< details::are_settings_from_tuple<
                                                              Settings, typename std::decay< Args >::type... >::value,
                                                          void * >::type = nullptr >
    explicit ProgressBar( Args &&... args )
        : settings_(
              details::get< details::ProgressBarOption::bar_width >( option::BarWidth{ 100 }, std::forward< Args >( args )... ), // NOLINT
              details::get< details::ProgressBarOption::prefix_text >( option::PrefixText{}, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::postfix_text >( option::PostfixText{}, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::start >( option::Start{ "[" }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::end >( option::End{ "]" }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::fill >( option::Fill{ "=" }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::lead >( option::Lead{ ">" }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::remainder >( option::Remainder{ " " }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::max_postfix_text_len >( option::MaxPostfixTextLen{ 0 }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::completed >( option::Completed{ false }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::show_percentage >( option::ShowPercentage{ false }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::show_elapsed_time >( option::ShowElapsedTime{ false }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::show_remaining_time >( option::ShowRemainingTime{ false }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::saved_start_time >( option::SavedStartTime{ false }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::foreground_color >( option::ForegroundColor{ Color::unspecified }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::font_styles >( option::FontStyles{ std::vector< FontStyle >{} }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::min_progress >( option::MinProgress{ 0 }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::max_progress >( option::MaxProgress{ 100 }, std::forward< Args >( args )... ), // NOLINT
              details::get< details::ProgressBarOption::progress_type >( option::ProgressType{ ProgressType::incremental }, std::forward< Args >( args )... ),
              details::get< details::ProgressBarOption::stream >( option::Stream{ std::cout }, std::forward< Args >( args )... ) ) {

      // if progress is incremental, start from min_progress
      // else start from max_progress
      const auto type = get_value< details::ProgressBarOption::progress_type >();
      if ( type == ProgressType::incremental ) {
        progress_ = get_value< details::ProgressBarOption::min_progress >();
      } else {
        progress_ = get_value< details::ProgressBarOption::max_progress >();
      }
    }

    template < typename T, details::ProgressBarOption id > void set_option( details::Setting< T, id > &&setting ) {
      static_assert( !std::is_same< T, typename std::decay< decltype(
                                           details::get_value< id >( std::declval< Settings >() ) ) >::type >::value,
                     "Setting has wrong type!" );
      std::lock_guard< std::mutex > lock( mutex_ );
      get_value< id >() = std::move( setting ).value;
    }

    template < typename T, details::ProgressBarOption id > void set_option( const details::Setting< T, id > &setting ) {
      static_assert( !std::is_same< T, typename std::decay< decltype(
                                           details::get_value< id >( std::declval< Settings >() ) ) >::type >::value,
                     "Setting has wrong type!" );
      std::lock_guard< std::mutex > lock( mutex_ );
      get_value< id >() = setting.value;
    }

    void set_option( const details::Setting< std::string, details::ProgressBarOption::postfix_text > &setting ) {
      std::lock_guard< std::mutex > lock( mutex_ );
      get_value< details::ProgressBarOption::postfix_text >() = setting.value;
      if ( setting.value.length() > get_value< details::ProgressBarOption::max_postfix_text_len >() ) {
        get_value< details::ProgressBarOption::max_postfix_text_len >() = setting.value.length();
      }
    }

    void set_option( details::Setting< std::string, details::ProgressBarOption::postfix_text > &&setting ) {
      std::lock_guard< std::mutex > lock( mutex_ );
      get_value< details::ProgressBarOption::postfix_text >() = std::move( setting ).value;
      auto &new_value                                         = get_value< details::ProgressBarOption::postfix_text >();
      if ( new_value.length() > get_value< details::ProgressBarOption::max_postfix_text_len >() ) {
        get_value< details::ProgressBarOption::max_postfix_text_len >() = new_value.length();
      }
    }

    void set_progress( size_t new_progress ) {
      {
        std::lock_guard< std::mutex > lock( mutex_ );
        progress_ = new_progress;
      }

      save_start_time();
      print_progress();
    }

    void tick() {
      {
        std::lock_guard< std::mutex > lock{ mutex_ };
        const auto                    type = get_value< details::ProgressBarOption::progress_type >();
        if ( type == ProgressType::incremental ) {
          progress_ += 1;
        } else {
          progress_ -= 1;
        }
      }
      save_start_time();
      print_progress();
    }

    auto current() -> size_t {
      std::lock_guard< std::mutex > lock{ mutex_ };
      return std::min( progress_, size_t( get_value< details::ProgressBarOption::max_progress >() ) );
    }

    [[nodiscard]] auto is_completed() const -> bool {
      std::lock_guard< std::mutex > lock{ mutex_ };
      return get_value< details::ProgressBarOption::completed >();
    }

    void mark_as_completed() {
      {
        std::lock_guard< std::mutex > lock{ mutex_ };
        get_value< details::ProgressBarOption::completed >() = true;
      }
      print_progress();
    }

  private:
    template < details::ProgressBarOption id >
    auto get_value() -> decltype( ( details::get_value< id >( std::declval< Settings & >() ).value ) ) {
      return details::get_value< id >( settings_ ).value;
    }

    template < details::ProgressBarOption id >
    [[nodiscard]] auto get_value() const -> decltype( ( details::get_value< id >( std::declval< const Settings & >() ).value ) ) {
      return details::get_value< id >( settings_ ).value;
    }

    size_t                                                        progress_{ 0 };
    Settings                                                      settings_;
    std::chrono::nanoseconds                                      elapsed_{};
    std::chrono::time_point< std::chrono::high_resolution_clock > start_time_point_;
    mutable std::mutex                                            mutex_;

    template < typename Indicator, size_t count > friend class MultiProgress;
    template < typename Indicator > friend class DynamicProgress;
    std::atomic< bool >   multi_progress_mode_{ false };
    std::atomic< size_t > dynamic_index_{ 0 };

    void save_start_time() {
      auto &show_elapsed_time   = get_value< details::ProgressBarOption::show_elapsed_time >();
      auto &saved_start_time    = get_value< details::ProgressBarOption::saved_start_time >();
      auto &show_remaining_time = get_value< details::ProgressBarOption::show_remaining_time >();
      if ( ( show_elapsed_time || show_remaining_time ) && !saved_start_time ) {
        start_time_point_ = std::chrono::high_resolution_clock::now();
        saved_start_time  = true;
      }
    }

    auto get_prefix_text() -> std::pair< std::string, size_t > {
      std::stringstream os;
      os << get_value< details::ProgressBarOption::prefix_text >();
      const auto result      = os.str();
      const auto result_size = unicode::display_width( result );
      return { result, result_size };
    }

    auto get_postfix_text() -> std::pair< std::string, size_t > {
      std::stringstream os;
      constexpr auto    percent      = 100;
      const auto        max_progress = get_value< details::ProgressBarOption::max_progress >();

      if ( get_value< details::ProgressBarOption::show_percentage >() ) {
        os << " "
           << std::min( static_cast< size_t >( static_cast< float >( progress_ ) / max_progress * percent ),
                        size_t( percent ) )
           << "%";
      }

      auto &saved_start_time = get_value< details::ProgressBarOption::saved_start_time >();

      if ( get_value< details::ProgressBarOption::show_elapsed_time >() ) {
        os << " [";
        if ( saved_start_time ) {
          details::write_duration( os, elapsed_ );
        } else {
          os << "00:00s";
        }
      }

      if ( get_value< details::ProgressBarOption::show_remaining_time >() ) {
        if ( get_value< details::ProgressBarOption::show_elapsed_time >() ) {
          os << "<";
        } else {
          os << " [";
        }

        if ( saved_start_time ) {
          auto eta = std::chrono::nanoseconds(
              progress_ > 0 ? static_cast< long long >( elapsed_.count() * max_progress / progress_ ) : 0 );
          auto remaining = eta > elapsed_ ? ( eta - elapsed_ ) : ( elapsed_ - eta );
          details::write_duration( os, remaining );
        } else {
          os << "00:00s";
        }

        os << "]";
      } else {
        if ( get_value< details::ProgressBarOption::show_elapsed_time >() ) {
          os << "]";
        }
      }

      os << " " << get_value< details::ProgressBarOption::postfix_text >();

      const auto result      = os.str();
      const auto result_size = unicode::display_width( result );
      return { result, result_size };
    }

  public:
    void print_progress( bool from_multi_progress = false ) {
      std::lock_guard< std::mutex > lock{ mutex_ };
      constexpr auto MIN_TERM_WIDTH  = 80;
      constexpr auto BARWITH_MAX_PER = 0.6;

      auto &os        = get_value< details::ProgressBarOption::stream >();
      auto bar_width  = get_value< details::ProgressBarOption::bar_width >();
      auto term_width = terminal_width() < MIN_TERM_WIDTH ? MIN_TERM_WIDTH : terminal_width();

      if ( bar_width > ( term_width * BARWITH_MAX_PER ) ) {
        bar_width = ( term_width * BARWITH_MAX_PER );
      }

      const auto max_str_len = ( term_width - bar_width ) / 2;

      auto prefix_pair   = get_prefix_text();
      auto prefix_text   = prefix_pair.first;
      auto prefix_length = prefix_pair.second;

      if ( prefix_length > max_str_len ) {
        prefix_text = prefix_text.substr( 0, max_str_len - 3 );
        prefix_text += "...";
        prefix_length = prefix_text.length();
      }

      auto postfix_pair   = get_postfix_text();
      auto postfix_text   = postfix_pair.first;
      auto postfix_length = postfix_pair.second;

      if ( postfix_length > max_str_len ) {
        postfix_text = postfix_text.substr( 0, max_str_len - 3 );
        postfix_text += "...";
        postfix_length = postfix_text.length();
      }

      const auto type         = get_value< details::ProgressBarOption::progress_type >();
      const auto min_progress = get_value< details::ProgressBarOption::min_progress >();
      const auto max_progress = get_value< details::ProgressBarOption::max_progress >();
      if ( multi_progress_mode_ && !from_multi_progress ) {
        if ( ( type == ProgressType::incremental && progress_ >= max_progress ) ||
             ( type == ProgressType::decremental && progress_ <= min_progress ) ) {
          get_value< details::ProgressBarOption::completed >() = true;
        }
        return;
      }
      auto now = std::chrono::high_resolution_clock::now();
      if ( !get_value< details::ProgressBarOption::completed >() ) {
        elapsed_ = std::chrono::duration_cast< std::chrono::nanoseconds >( now - start_time_point_ );
      }

      if ( get_value< details::ProgressBarOption::foreground_color >() != Color::unspecified ) {
        details::set_stream_color( os, get_value< details::ProgressBarOption::foreground_color >() );
      }

      for ( auto &style : get_value< details::ProgressBarOption::font_styles >() ) {
        details::set_font_style( os, style );
      }

      // PREFIX TEXT
      os << prefix_text;

      os << get_value< details::ProgressBarOption::start >();

      // BAR
      details::ProgressScaleWriter writer{
          os, bar_width, get_value< details::ProgressBarOption::fill >(),
          get_value< details::ProgressBarOption::lead >(), get_value< details::ProgressBarOption::remainder >() };
      writer.write( double( progress_ ) / double( max_progress ) * 100.0F ); // NOLINT

      os << get_value< details::ProgressBarOption::end >();

      // POSTFIX TEXT
      os << postfix_text;

      // Get length of prefix text and postfix text
      const auto start_length   = get_value< details::ProgressBarOption::start >().size();
      //const auto bar_width      = get_value< details::ProgressBarOption::bar_width >();
      const auto end_length     = get_value< details::ProgressBarOption::end >().size();
      const auto terminal_width = terminal_size().second;
      // prefix + bar_width + postfix should be <= terminal_width
      const auto remaining = terminal_width - ( prefix_length + start_length + bar_width + end_length + postfix_length );

      if ( remaining > 0 and remaining < terminal_width ) {
        os << std::string( remaining, ' ' ) << "\r";
//      } else if ( remaining < 0 ) {
        // Do nothing. Maybe in the future truncate postfix with ...
      }
      os.flush();

      if ( ( type == ProgressType::incremental && progress_ >= max_progress ) ||
           ( type == ProgressType::decremental && progress_ <= min_progress ) ) {
        get_value< details::ProgressBarOption::completed >() = true;
      }
      if ( get_value< details::ProgressBarOption::completed >() &&
           !from_multi_progress ) {// Don't std::endl if calling from MultiProgress
        os << termcolor::reset << std::endl;
      }
    }
  };

} // namespace indicators

#pragma once
#include <atomic>
#include <functional>
// #include <indicators/color.hpp>
// #include <indicators/setting.hpp>
// #include <indicators/details/stream_helper.hpp>
#include <iostream>
#include <mutex>
#include <vector>
#include <chrono>
#include <condition_variable>

namespace indicators {
  // It exists because the std::this_thread::get_id() is much slower(especially
  // under VS 2013) - Taken from spdlog
  inline auto thread_id() noexcept -> size_t {
#ifdef _WIN32
    return static_cast< size_t >( ::GetCurrentThreadId() );
#elif defined( __linux__ )
    return static_cast< size_t >( ::syscall( SYS_gettid ) );
#elif defined( _AIX ) || defined( __DragonFly__ ) || defined( __FreeBSD__ )
    return static_cast< size_t >( ::pthread_getthreadid_np() );
#elif defined( __NetBSD__ )
    return static_cast< size_t >( ::_lwp_self() );
#elif defined( __OpenBSD__ )
    return static_cast< size_t >( ::getthrid() );
#elif defined( __sun )
    return static_cast< size_t >( ::thr_self() );
#elif __APPLE__
    uint64_t tid;
    pthread_threadid_np( nullptr, &tid );
    return static_cast< size_t >( tid );
#else // Default to standard C++11 (other Unix)
    return static_cast< size_t >( std::hash< std::thread::id >()( std::this_thread::get_id() ) );
#endif
  }

  template < typename Indicator > class DynamicProgress {
    using Settings = std::tuple< option::HideBarWhenComplete >;

  public:
    class BarGuard {
    public:
      BarGuard( size_t bar_index, DynamicProgress * dyn_pro ) : bar_index_( bar_index ), dyn_pro_( dyn_pro ) {}

      BarGuard( BarGuard const& ) = delete;
      BarGuard( BarGuard&& other ) noexcept : bar_index_( other.bar_index_ ), dyn_pro_( std::move( other.dyn_pro_ ) ) {
        other.value_held_ = false;
      }
      ~BarGuard() {
        if ( value_held_ ) {
          dyn_pro_->erase( bar_index_ );
        }
      }

      auto operator=( BarGuard const& ) = delete;
      auto operator=( BarGuard&& other ) noexcept -> BarGuard& {
        bar_index_ = other.bar_index_;
        dyn_pro_   = std::move( other.dyn_pro_ );

        other.value_held_ = false;

        return *this;
      }

      auto get() -> Indicator& {
        return dyn_pro_->at( bar_index_ );
      }
    private:
      bool   value_held_{true};
      size_t bar_index_;
      DynamicProgress * dyn_pro_;
    };

    DynamicProgress() = default;

    DynamicProgress( DynamicProgress const& ) = delete;
    DynamicProgress( DynamicProgress && /*other*/ ) = delete;

    ~DynamicProgress() {
      if ( started_ ) {
        run_thr_ = false;

        prt_thr_.join();
      }
    }

    auto operator=( DynamicProgress const& ) -> DynamicProgress& = delete;
    auto operator=( DynamicProgress && ) -> DynamicProgress& = delete;

    // Return a lifetime guard instead of an index into the underlining structure
    auto push_back( Indicator &bar ) -> BarGuard {
      auto thr_id = thread_id();
      {
        std::lock_guard< std::mutex > lock{ mutex_ };

        if ( not started_ ) {
          startManager();
        }

        while ( bars_.find( thr_id ) != bars_.end() ) {
          ++thr_id;
        } // Still allow more then one bar per thread

        bars_.insert( { thr_id, bar } );
      }

      bar.multi_progress_mode_ = true;
      bar.dynamic_index_       = thr_id;
      cv_.notify_all();

      return std::move( BarGuard{ thr_id, this } );
    }

    void erase( size_t index ) {
      std::lock_guard< std::mutex > lock{ mutex_ };

      bars_.erase( bars_.find( index ) );
      cv_.notify_all();
    }

    template < typename T, details::ProgressBarOption id > void set_option( details::Setting< T, id > &&setting ) {
      static_assert( !std::is_same< T, typename std::decay< decltype(
                                           details::get_value< id >( std::declval< Settings >() ) ) >::type >::value,
                     "Setting has wrong type!" );
      std::lock_guard< std::mutex > lock( mutex_ );
      get_value< id >() = std::move( setting ).value;
    }

    template < typename T, details::ProgressBarOption id > void set_option( const details::Setting< T, id > &setting ) {
      static_assert( !std::is_same< T, typename std::decay< decltype(
                                           details::get_value< id >( std::declval< Settings >() ) ) >::type >::value,
                     "Setting has wrong type!" );
      std::lock_guard< std::mutex > lock( mutex_ );
      get_value< id >() = setting.value;
    }

    friend class BarGuard;
  private:
    Settings                settings_;
    bool                    started_{ false };
    std::atomic< bool >     run_thr_{ true };
    std::thread             prt_thr_;
    std::mutex              mutex_;
    std::condition_variable cv_;
    std::map< size_t, std::reference_wrapper< Indicator > >
                          bars_; // Threads close and references get invalidated, need to be able remove references of completed bars
    std::atomic< size_t > last_draw_height_{ 0 };

    template < details::ProgressBarOption id >
    auto get_value() -> decltype( ( details::get_value< id >( std::declval< Settings & >() ).value ) ) {
      return details::get_value< id >( settings_ ).value;
    }

    template < details::ProgressBarOption id >
    auto get_value() const -> decltype( ( details::get_value< id >( std::declval< const Settings & >() ).value ) ) {
      return details::get_value< id >( settings_ ).value;
    }

    auto operator[]( size_t index ) -> Indicator & {
      std::lock_guard< std::mutex > lock{ mutex_ };
      return bars_.at( index ).get();
    }

    auto at( size_t index ) -> Indicator & {
      std::lock_guard< std::mutex > lock{ mutex_ };
      return bars_.at( index );
    }

    void print_progress() {
      std::lock_guard< std::mutex > lock{ mutex_ };
      // Clear previous screen
      for ( size_t i = 0; i != last_draw_height_; ++i ) {
        std::cout << "\r\033[K\033[A"; // clear current line first, then move cursor up one line
      }

      last_draw_height_ = 0;
      for ( auto &bar : bars_ ) {
        // Only print upto the height of the current display
        if ( last_draw_height_++ < terminal_height() - 1 ) { // Increment once
          bar.second.get().print_progress( true );
          std::cout << "\n";
        } else {
          std::cout << termcolor::reset;
          std::cout << last_draw_height_ - 1 << "/" << bars_.size() << " Displayed.\r";

          break;
        }
      }

      std::cout << std::flush;
    }

    auto startManager() -> void {
      using namespace std::chrono_literals;
      started_ = true;
      prt_thr_ = std::thread([&](){
          size_t bar_count = 0;
          while( run_thr_ ) {
            { // Wake on a change or timeout
              std::unique_lock< std::mutex > ul( mutex_ );
              cv_.wait_for( ul, 250ms, [&]() { return bars_.size() != bar_count; } );
              bar_count = bars_.size();
            }
            print_progress();
          }
        });
    }
  };

} // namespace indicators
