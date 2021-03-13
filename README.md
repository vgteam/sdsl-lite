# SDSL - Succinct Data Structure Library (vgteam fork)

## This fork

This is [vgteam's](https://github.com/vgteam) fork of the Succinct Data Structure Library (SDSL).
As [SDSL 2](https://github.com/simongog/sdsl-lite) is no longer maintained, vgteam tools and libraries will depend on this until SDSL 3 is released.

## Major changes

* Switched from C++11 to C++14.
* `sd_vector` improvements:
  * `sd_vector::one_iterator`: Iterator over set bits.
  * Predecessor and successor queries.
  * Defined semantics for an `sd_vector` encoding a multiset of integers.
* `rle_vector`: A run-length encoded bitvector.

## Tools/libraries using this fork

- [x] [VG](https://github.com/vgteam/vg)
- [x] [GBWT](https://github.com/jltsiren/gbwt)
- [x] [GBWTGraph](https://github.com/jltsiren/gbwtgraph)
- [x] [GCSA2](https://github.com/jltsiren/gcsa2)
- [x] [libbdsg](https://github.com/vgteam/libbdsg)
- [x] [libbdsg-easy](https://github.com/vgteam/libbdsg-easy)
- [x] [mmmulti](https://github.com/ekg/mmmulti)
- [x] [XG](https://github.com/vgteam/xg)
