#ifndef ONE_HOT_H__
#define ONE_HOT_H__


#include <resource.h>
#include <byte.h>

#include <span>


template <Mode m>
std::vector<Garbled::Bit<m>> onehot_op(std::span<const Garbled::Bit<m>>, std::span<const Garbled::Bit<m>>);



#endif
