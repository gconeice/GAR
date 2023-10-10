#ifndef LINEAR_SCAN_H__
#define LINEAR_SCAN_H__


#include <vector>
#include <span>
#include <bitstring.h>

/**
 * Use a selection vector of size n to read from a buffer of size n*w.  The
 * selection vector should be *one-hot*. I.e., it should have exactly one
 * non-zero entry. The buffer's index corresponding to the location of the
 * non-zero value is returned. The content of the buffer is not changed.
 */
template <Mode m>
std::vector<Garbled::Bit<m>> read_selection(
    std::size_t w,
    std::span<const Garbled::Bit<m>> selection,
    std::span<const Garbled::Bit<m>> buff);


/**
 * Use a selection vector of size n to write to a buffer of size n*w.  The
 * selection vector should be *one-hot*. I.e., it should have exactly one
 * non-zero entry. The buffer's index corresponding to the location of the
 * non-zero value is returned. Additionally, the selected buffer index is
 * overwritten by `to_write`.
 */
template <Mode m>
std::vector<Garbled::Bit<m>> write_selection(
    std::span<const Garbled::Bit<m>> to_write,
    std::span<const Garbled::Bit<m>> selection,
    std::span<Garbled::Bit<m>> buff);


/**
 * Helper function for efficiently constructing selection vectors.
 * Given a binary encoding of a number, construct the corresponding one-hot
 * encoding.
 *
 * Outputs the one hot encoding in-place to the target span.
 */
template <Mode m>
void one_hot(std::span<const Garbled::Bit<m>>, std::span<Garbled::Bit<m>>);


/**
 * Helper function for efficiently constructing selection vectors.
 * Given a binary encoding of a number, construct the corresponding one-hot
 * encoding.
 */
template <Mode m>
std::vector<Garbled::Bit<m>> one_hot(std::span<const Garbled::Bit<m>> inp) {
  std::vector<Garbled::Bit<m>> u(1 << inp.size());
  one_hot<m>(inp, u);
  return u;
}


/**
 * Read a particular index from a buffer via linear scan.
 *
 * See read_selection.
 */
template <Mode m>
std::vector<Garbled::Bit<m>> read_scan(
    std::size_t w,
    std::span<const Garbled::Bit<m>> index,
    std::span<const Garbled::Bit<m>> to_read) {
  const auto selection = one_hot(index);
  return read_selection(w, std::span { selection }, to_read);
}


/**
 * Write to a particular index in a buffer via linear scan.
 * Also returns the old content.
 *
 * See write_selection.
 */
template <Mode m>
std::vector<Garbled::Bit<m>> write_scan(
    std::span<const Garbled::Bit<m>> to_write,
    std::span<const Garbled::Bit<m>> index,
    std::span<Garbled::Bit<m>> buff) {
  const auto selection = one_hot(index);
  return write_selection(to_write, std::span { selection }, buff);
}

#endif
