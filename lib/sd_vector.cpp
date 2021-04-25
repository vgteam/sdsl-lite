#include "sdsl/sd_vector.hpp"
#include <cassert>

//! Namespace for the succinct data structure library
namespace sdsl
{

sd_vector_builder::sd_vector_builder() :
    m_size(0), m_capacity(0),
    m_wl(0),
    m_tail(0), m_tail_inc(0), m_items(0),
    m_last_high(0), m_highpos(0)
{
}

sd_vector_builder::sd_vector_builder(size_type n, size_type m, bool multiset) :
    m_size(n), m_capacity(m),
    m_wl(0),
    m_tail(0), m_tail_inc((multiset ? 0 : 1)), m_items(0),
    m_last_high(0), m_highpos(0)
{
    if (m_capacity > m_size) {
        throw std::runtime_error("sd_vector_builder: requested capacity is larger than vector size.");
    }

    std::pair<size_type, size_type> params = sd_vector<>::get_params(m_size, m_capacity);
    m_wl = params.first;
    m_low = int_vector<>(m_capacity, 0, params.first);
    m_high = bit_vector(params.second, 0);
}

void
sd_vector_builder::swap(sd_vector_builder& sdb)
{
    std::swap(m_size, sdb.m_size);
    std::swap(m_capacity, sdb.m_capacity);
    std::swap(m_wl, sdb.m_wl);
    std::swap(m_tail, sdb.m_tail);
    std::swap(m_tail_inc, sdb.m_tail_inc);
    std::swap(m_items, sdb.m_items);
    std::swap(m_last_high, sdb.m_last_high);
    std::swap(m_highpos, sdb.m_highpos);
    m_low.swap(sdb.m_low);
    m_high.swap(sdb.m_high);
}

template<>
sd_vector<>::sd_vector(builder_type& builder)
{
    if(builder.items() != builder.capacity()) {
      throw std::runtime_error("sd_vector: the builder is not at full capacity.");
    }

    m_size = builder.m_size;
    m_wl = builder.m_wl;
    m_low.swap(builder.m_low);
    m_high.swap(builder.m_high);
    util::init_support(m_high_1_select, &m_high);
    util::init_support(m_high_0_select, &m_high);

    builder = builder_type();
}

} // end namespace
