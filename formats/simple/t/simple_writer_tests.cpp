#include "tests.hpp"

#include <fstream>

#include <boost/scoped_ptr.hpp>

#include "simple_lattice_writer.hpp"
#include "writers_tests_utils.hpp"


BOOST_AUTO_TEST_SUITE( simple_lattice_writer )


BOOST_AUTO_TEST_CASE( simple_lattice_writer ) {

    Lattice lattice;
    writers_tests_utils::prepareSimpleLattice(lattice);

    std::map<std::string, std::string> tagsSeparators;
    tagsSeparators["token"] = ";";

    boost::scoped_ptr<LatticeWriter<std::ostream> > writer(new SimpleLatticeWriter(
                                                false, //linear
                                                false, //no-alts
                                                true, //with-blank
                                                "symbol", //tag
                                                ",", //sep
                                                "|", //alt-sep
                                                tagsSeparators
                                                ));

    std::ostringstream osstr;
    writer->writeLattice(lattice, osstr);

    std::string line;
    std::string contents;
    std::ifstream s(ROOT_DIR "formats/simple/t/files/simple_ala.txt");
    while (getline(s, line)) {
        contents += line;
        contents += "\n";
    }

    BOOST_CHECK_EQUAL(osstr.str(), contents);

}


BOOST_AUTO_TEST_CASE( simple_lattice_writer_linear ) {

    Lattice lattice;
    writers_tests_utils::prepareSimpleLattice(lattice);

    std::map<std::string, std::string> tagsSeparators;
    tagsSeparators["token"] = ";";

    boost::scoped_ptr<LatticeWriter<std::ostream> > writer(new SimpleLatticeWriter(
                                                true, //linear
                                                false, //no-alts
                                                true, //with-blank
                                                "symbol", //tag
                                                ",", //sep
                                                "|", //alt-sep
                                                tagsSeparators
                                                ));

    std::ostringstream osstr;
    writer->writeLattice(lattice, osstr);

    std::string line;
    std::string contents;
    std::ifstream s(ROOT_DIR "formats/simple/t/files/simple_ala.txt");
    while (getline(s, line)) {
        contents += line;
        contents += "\n";
    }

    BOOST_CHECK_EQUAL(osstr.str(), contents);

}


BOOST_AUTO_TEST_CASE( simple_lattice_writer_advanced ) {

    Lattice lattice;
    writers_tests_utils::prepareAdvancedLattice(lattice);

    std::map<std::string, std::string> tagsSeparators;
    // tagsSeparators["splitter"] = "\n";

    boost::scoped_ptr<LatticeWriter<std::ostream> > writer(new SimpleLatticeWriter(
                                                false, //linear
                                                false, //no-alts
                                                true, //with-blank
                                                "token", //tag
                                                ",", //sep
                                                "|", //alt-sep
                                                tagsSeparators
                                                ));

    std::ostringstream osstr;
    writer->writeLattice(lattice, osstr);

    std::string line;
    std::string contents;
    std::ifstream s(ROOT_DIR "formats/simple/t/files/simple_ala_advanced.txt");
    while (getline(s, line)) {
        contents += line;
        contents += "\n";
    }

    BOOST_CHECK_EQUAL(osstr.str(), contents);

}


BOOST_AUTO_TEST_SUITE_END()
