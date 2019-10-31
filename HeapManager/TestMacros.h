#pragma once

#include <stdexcept>
#include <string>

#define T_COND_MSG( expr, msg ) if( !(expr) )throw std::runtime_error( std::string(msg) + " line - " + std::to_string(__LINE__) + " | fun - " + std::string(__FUNCSIG__) );
#define T_COND( expr ) T_COND_MSG( expr, "" )
#define T_COMPARE( v, e ) T_COND_MSG( v == e, "" )
#define T_COMPARE_MSG( v, e, msg ) T_COND_MSG( v == e, msg )
