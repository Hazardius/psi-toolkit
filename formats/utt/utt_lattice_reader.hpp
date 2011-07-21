#ifndef UTT_LATTICE_READER_HDR
#define UTT_LATTICE_READER_HDR

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi.hpp>

#include "lattice_reader.hpp"
#include "quoter.hpp"
#include "utt_quoter.hpp"
#include "utf8_string.hpp"


using namespace boost::spirit;


struct UTTLRItem {
    int position;
    int length;
    std::string segmentType;
    std::string form;
    std::string annotations;
    std::string unused;

    void unescape(Quoter & quoter) {
        // Quoter * quoter = new UTTQuoter();
        segmentType = quoter.unescape(segmentType);
        form = quoter.unescape(form);
        annotations = quoter.unescape(annotations);
        // delete quoter;
    }
};


BOOST_FUSION_ADAPT_STRUCT(
    UTTLRItem,
    (int, position)
    (std::string, unused)
    (int, length)
    (std::string, unused)
    (std::string, segmentType)
    (std::string, unused)
    (std::string, form)
    (std::string, annotations)
)


struct UTTLRGrammar : public qi::grammar<std::string::const_iterator, UTTLRItem()> {

    UTTLRGrammar() : UTTLRGrammar::base_type(start) {

        start
            %= qi::int_
            >> +(qi::space)
            >> qi::int_
            >> +(qi::space)
            >> +(qi::char_ - ' ')
            >> +(qi::space)
            >> +(qi::char_ - ' ')
            >> -(qi::lexeme[' ' >> +(qi::char_)])
            ;

    }

    qi::rule<std::string::const_iterator, UTTLRItem()> start;

};


class UTTLatticeReader : public LatticeReader {

public:

    /**
     * Gets format name (here: "UTT").
     */
    std::string getFormatName();

private:
    virtual std::string doInfo();

    virtual void doReadIntoLattice(std::istream& inputStream, Lattice& lattice);
};


#endif
