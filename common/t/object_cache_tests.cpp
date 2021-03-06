#include "tests.hpp"

#include "object_cache.hpp"
#include "xml_property_tree.hpp"

#include "config.hpp"

BOOST_AUTO_TEST_SUITE( object_cache )

BOOST_AUTO_TEST_CASE( object_cache_simple ) {
    boost::shared_ptr<XmlPropertyTree> objA1
        = ObjectCache::getInstance().getObject<XmlPropertyTree>(
            ROOT_DIR "common/t/simple.xml");

    boost::shared_ptr<XmlPropertyTree> objA2
        = ObjectCache::getInstance().getObject<XmlPropertyTree>(
            ROOT_DIR "common/t/simple.xml");

    BOOST_CHECK_THROW(
        ObjectCache::getInstance().getObject<XmlPropertyTree>(
            ROOT_DIR "common/t/nonexisting.xml"),
        std::exception);

    BOOST_CHECK(objA1.get() == objA2.get());
}

BOOST_AUTO_TEST_SUITE_END()
