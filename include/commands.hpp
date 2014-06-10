#pragma once

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! Game commands
////////////////////////////////////////////////////////////////////////////////
enum class command_type {
    none
  , move_nw, move_n,    move_ne
  , move_w,  move_here, move_e
  , move_sw, move_s,    move_se
  , cancel
  , open, close
  , kick
  , search
  , untrap
};

} //namespace yama
