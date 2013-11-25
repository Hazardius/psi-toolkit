#include "layer_tag_collection.hpp"

bool LayerTagCollection::isEmpty() {
    return v_.none();
}

bool LayerTagCollection::isNonempty() {
    return v_.any();
}

bool LayerTagCollection::operator<(LayerTagCollection other) const {
    return v_.size() < other.v_.size() || (v_.size() == other.v_.size() && (v_ & other.v_) == v_);
}

bool LayerTagCollection::operator==(const LayerTagCollection& other) const {
    return v_ == other.v_;
}

bool LayerTagCollection::operator!=(const LayerTagCollection& other) const {
    return v_ != other.v_;
}

unsigned long LayerTagCollection::getHash() const {
    return v_.to_ulong();
}

boost::dynamic_bitset<>::size_type LayerTagCollection::resize_(
    boost::dynamic_bitset<>::size_type size) {

    if (size > v_.size()) {
        throw std::exception();
    }
    return v_.size();
}

boost::dynamic_bitset<>::size_type LayerTagCollection::resize_(LayerTagCollection& other) {
    return resize_(other.resize_(v_.size()));
}

LayerTagCollection createUnion(
    LayerTagCollection tag_list_a,
    LayerTagCollection tag_list_b
) {
    LayerTagCollection result(tag_list_a.resize_(tag_list_b));
    result.v_ |= tag_list_a.v_;
    result.v_ |= tag_list_b.v_;
    return result;
}

LayerTagCollection createIntersection(
    LayerTagCollection tag_list_a,
    LayerTagCollection tag_list_b
) {
    LayerTagCollection result(tag_list_a.resize_(tag_list_b));
    result.v_ |= tag_list_a.v_;
    result.v_ &= tag_list_b.v_;
    return result;
}

bool isSubset(
    LayerTagCollection tag_list_a,
    LayerTagCollection tag_list_b) {

    tag_list_a.resize_(tag_list_b);

    return (tag_list_a.v_ & tag_list_b.v_) == tag_list_a.v_;
}
