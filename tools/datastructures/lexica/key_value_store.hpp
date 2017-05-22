#ifndef KEY_VALUE_STORE_HDR_HDR
#define KEY_VALUE_STORE_HDR_HDR

#include "perfect_hash_index.hpp"
#include "string_vector.hpp"

#include <boost/optional/optional.hpp>
#include <boost/optional/optional_io.hpp>

class KeyValueStore {
public:
    boost::optional<std::string> get(const std::string& key) const;

    void load(std::FILE* mphf);
    void load(const std::string& filename);
    void save(std::FILE* mphf) const;
    void save(const std::string& filename) const;

    class Builder {
    public:
        Builder();
        ~Builder();

        void add(const std::string& key, const std::string& value);
        KeyValueStore* build();
    private:
        KeyValueStore* store_;
        StringVector<> valuesInOrder_;
    };

private:
    PerfectHashIndex perfectHashIndex_;
    StringVector<> stringVector_;

};

#endif
