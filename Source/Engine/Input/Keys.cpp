#include "Keys.hpp"

#include <SDL.h>

Key ConvertSDLKeyCode(int32 sdlKeyCode)
{
  switch (sdlKeyCode)
  {
    case SDLK_q: return Key::Q;
    case SDLK_w: return Key::W;
    case SDLK_e: return Key::E;
    case SDLK_r: return Key::R;
    case SDLK_t: return Key::T;
    case SDLK_y: return Key::Y;
    case SDLK_u: return Key::U;
    case SDLK_i: return Key::I;
    case SDLK_o: return Key::O;
    case SDLK_p: return Key::P;
    case SDLK_LEFTBRACKET: return Key::LeftBracket;
    case SDLK_RIGHTBRACKET: return Key::RightBracket;
    case SDLK_BACKSLASH: return Key::BackSlash;
    case SDLK_a: return Key::A;
    case SDLK_s: return Key::S;
    case SDLK_d: return Key::D;
    case SDLK_f: return Key::F;
    case SDLK_g: return Key::G;
    case SDLK_h: return Key::H;
    case SDLK_j: return Key::J;
    case SDLK_k: return Key::K;
    case SDLK_l: return Key::L;
    case SDLK_SEMICOLON: return Key::SemiColon;
    case SDLK_ALTERASE: return Key::Apostrophe;
    case SDLK_z: return Key::Z;
    case SDLK_x: return Key::X;
    case SDLK_c: return Key::C;
    case SDLK_v: return Key::V;
    case SDLK_b: return Key::B;
    case SDLK_n: return Key::N;
    case SDLK_m: return Key::M;
    case SDLK_COMMA: return Key::Comma;
    case SDLK_PERIOD: return Key::FullStop;
    case SDLK_SLASH: return Key::ForwardSlash;
  }
}